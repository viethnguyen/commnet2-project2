/*
 * forwarder.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: vietnguyen
 */

#include "common.h"
#include "newport.h"
#include "message.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

//state of the forwarder
const int WAITING_STATE = 0;
const int RECEIVING_STATE = 1;

struct cShared{
	LossyReceivingPort *my_recv_port;
	mySendingPort *my_send_port;
};


void *forwardProc1(void *arg){
	printf("Started receiving thread\n");
	ofstream log12;
	log12.open("forward_1_2.txt");
	log12 << " --- \n";

	struct cShared *sh = (struct cShared *)arg;
	Packet *recvPacket;
	Message *m = new Message();
	while(1){

		recvPacket = sh->my_recv_port->receivePacket();
		if(recvPacket!=NULL){
			int seqNum = recvPacket->accessHeader()->getIntegerInfo(m->SEQNUM_POS);
			sh->my_send_port->sendPacket(recvPacket);
			cout << "forward from node 1 to node 2 packet : " << seqNum << "\n";
			log12 << "forward from node 1 to node 2 packet : " << seqNum << "\n";
		}
	}
	return NULL;
}

void *forwardProc2(void *arg){
	printf("Started receiving thread\n");
	ofstream log21;
	log21.open("forward_2_1.txt");
	log21 << " --- \n";
	struct cShared *sh = (struct cShared *)arg;
	Packet *recvPacket;
	Message *m = new Message();
	while(1){

		recvPacket = sh->my_recv_port->receivePacket();
		if(recvPacket!=NULL){
			int seqNum = recvPacket->accessHeader()->getIntegerInfo(m->SEQNUM_POS);
			sh->my_send_port->sendPacket(recvPacket);
			cout << "forward from node 2 to node 1 packet : " << seqNum << "\n";
			log21 << "forward from node 2 to node 1 packet : " << seqNum << "\n";
		}
	}
	return NULL;
}

/**
 * default param
 * 			hostname = "localhost"
 *			port1_send number = 4000
 *			port2_send number = 4500
 * @param argv[1] node1_address
 * 			argv[2] node1 port number
 * 			argv[3] node2 address
 * 			argv[4] node2 port number
 * 			argv[5] port1_recv number
 * 			argv[6] port2_recv number
 */
int main(int argc, const char * argv[])
{


try {
  const char* hname = "localhost";

  /* For node 1 */

  //configure receiving port
  Address * rx_addr_node1 = new Address(hname, (short)(atoi(argv[5])));

  LossyReceivingPort *rx_port_node1 = new LossyReceivingPort(0.2);
  rx_port_node1->setAddress(rx_addr_node1);
  rx_port_node1->init();

  //configure a sending port
  Address * tx_addr_node1 = new Address(hname, 4000);
  Address * node1_addr =  new Address(argv[1], (short)(atoi(argv[2])));
  mySendingPort *tx_port_node1 = new mySendingPort();
  tx_port_node1->setAddress(tx_addr_node1);
  tx_port_node1->setRemoteAddress(node1_addr);
  tx_port_node1->init();

  /* For node 2 */

  //configure receiving port
  Address * rx_addr_node2 = new Address(hname, (short)(atoi(argv[6])));

  LossyReceivingPort *rx_port_node2 = new LossyReceivingPort(0.2);
  rx_port_node2->setAddress(rx_addr_node2);
  rx_port_node2->init();

  //configure a sending port
  Address * tx_addr_node2 = new Address(hname, 4500);
  Address * node2_addr =  new Address(argv[3], (short)(atoi(argv[4])));
  mySendingPort *tx_port_node2 = new mySendingPort();
  tx_port_node2->setAddress(tx_addr_node2);
  tx_port_node2->setRemoteAddress(node2_addr);
  tx_port_node2->init();

  /* End configure */

  cout << " Start forwarding ..." << endl;

  // spawn 2 new threads for both ways of forwarding

  struct cShared *sh1;	//Forwarding node1->node2
  sh1 = (struct cShared*)malloc(sizeof(struct cShared));
  sh1->my_recv_port = rx_port_node1;
  sh1->my_send_port = tx_port_node2;
  pthread_t thread1;
  printf("Creating receiving thread\n");
  pthread_create(&(thread1), NULL, &forwardProc1, sh1);

  struct cShared *sh2;  //Forwarding node2->node1
  sh2 = (struct cShared*)malloc(sizeof(struct cShared));
  sh2->my_recv_port = rx_port_node2;
  sh2->my_send_port = tx_port_node1;
  pthread_t thread2;
  printf("Creating receiving thread\n");
  pthread_create(&(thread2), NULL, &forwardProc2, sh2);

  while(1){
	  //loop infinitely
  }

} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
}

return 0;
}
