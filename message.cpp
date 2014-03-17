/*
 * message.cpp
 *
 *  Created on: Mar 2, 2014
 *      Author: vietnguyen
 *
 *      Define message types used
 */

#include "common.h"
#include "newport.h"
#include "message.h"
#include <cstring>
#include <iostream>
#include <fstream>

#define INIT_SIZE 500
#define DATA_SIZE 1200
#define SENDER_ID 15
#define RECEIVER_ID 20

#define INIT_MESSAGE_TYPE 0
#define ACK_MESSAGE_TYPE 1
#define DATA_MESSAGE_TYPE 2
#define NACK_MESSAGE_TYPE 3

/**
 * Constructor
 */

Message::Message(){
	filename_ =  new char[MAX_FILE_NAME_LEN];
	number_of_packets_ = 0;
	length_ = 0;
}

/**
 * Create a new init message, with the name of the file being transfered
 */
Packet *Message::make_init_packet(char* filename){
	  Packet * init_packet;
	  init_packet = new Packet();
	  init_packet->setPayloadSize(INIT_SIZE);

	  //Header
	  PacketHdr *hdr = init_packet->accessHeader();
	  hdr->setIntegerInfo(SENDER_ID, ID_POS);
	  hdr->setShortIntegerInfo(INIT_MESSAGE_TYPE, TYPE_POS);
	  hdr->setIntegerInfo(0,SEQNUM_POS); //sequence number

	  //Payload
	  int len = strlen(filename);
	  init_packet->fillPayload(len, filename);
	  return init_packet;
}

/**
 * Create an ACK message
 */
Packet *Message::make_ack_packet(int seqNum){
	Packet *ack_packet;
	ack_packet = new Packet();
	ack_packet->setPayloadSize(0);

	//Header
	PacketHdr *hdr = ack_packet->accessHeader();
	hdr->setIntegerInfo(RECEIVER_ID, ID_POS);
	hdr->setShortIntegerInfo(ACK_MESSAGE_TYPE, TYPE_POS);
	hdr->setIntegerInfo(seqNum, SEQNUM_POS); // sequence number being ACKED

	return ack_packet;
}

/**
 * Get some information of the file to send
 */
bool Message::prepare_file_to_send(char *filename){
	//read file
	ifstream f1(filename);
	f1.seekg(0, ios::end);
	size_t len = f1.tellg();
	strcpy(filename_,filename);
	length_ = len;
	 number_of_packets_ = len/DATA_SIZE + 1;
	 return true;
}

/**
 * Create a DATA message. seqNum = sequence number of the packet
 */
Packet *Message::make_data_packet(int seqNum){
	Packet *data_packet;
	data_packet = new Packet();

	//Header
	PacketHdr *hdr = data_packet->accessHeader();
	hdr->setIntegerInfo(SENDER_ID, ID_POS);
	hdr->setShortIntegerInfo(DATA_MESSAGE_TYPE, TYPE_POS);
	hdr->setIntegerInfo(seqNum,SEQNUM_POS); //sequence number
	if(seqNum == number_of_packets_){
		hdr->setShortIntegerInfo(1,EOF_POS);
	}else{
		hdr->setShortIntegerInfo(0,EOF_POS);
	}

	//Payload
	int payload_size;
	if(seqNum == number_of_packets_){
		payload_size = length_ - (number_of_packets_ - 1) * DATA_SIZE;
	}
	else{
		payload_size = DATA_SIZE;
	}
	char *data_chunk = new char[payload_size];
	//read the file
	ifstream f1(filename_);
	f1.seekg(0, ios::end);
	int pos = (seqNum - 1) * DATA_SIZE;
	f1.seekg(pos);
	f1.read(data_chunk, payload_size);
	f1.close();
	data_packet->fillPayload(payload_size, data_chunk);
	return data_packet;
}

/**
 * append a data chunk to a file. Used for receiver.
 */
bool Message::append_data_to_file(char *filename, char* datachunk, int len){
	try{
		ofstream outfile;
		outfile.open(filename, std::ofstream::out | std::ofstream::app);
		outfile.write(datachunk, len);
		outfile.close();
		return true;
	} catch (const char *reason ) {
	    cerr << "Exception:" << reason << endl;
	    return false;
	  }

}

