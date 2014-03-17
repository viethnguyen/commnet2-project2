/*
 * message.h
 *
 *  Created on: Mar 2, 2014
 *      Author: vietnguyen
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_



class Message {
protected:
	char * filename_;
	int number_of_packets_;
	size_t length_;
public:
	static const int MAX_FILE_NAME_LEN = 80;
	Message();
	//define message format - byte position in the header of each field
	static const int ID_POS = 0;
	static const int TYPE_POS = 4;
	static const int SEQNUM_POS = 6;
	static const int EOF_POS = 10;

	//'make' functions
	int get_number_of_packets(){return number_of_packets_;}
	int get_file_length(){return length_;}
	Packet *make_init_packet(char* filename);
	Packet *make_ack_packet(int seqNum);
	bool prepare_file_to_send(char *filename);
	Packet *make_data_packet(int seqNum);
	bool append_data_to_file(char *filename, char *datachunk, int len);

};

#endif /* MESSAGE_H_ */
