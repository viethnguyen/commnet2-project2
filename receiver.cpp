#include "common.h"
#include "newport.h"
#include "message.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

//state of the receiver
const int WAITING_STATE = 0;
const int RECEIVING_STATE = 1;

/**
 * @param argv[1] receiving port number
 * 			argv[2] destination address
 * 			argv[3] destination port number
 * 			argv[4] filename to save
 */
int main(int argc, const char * argv[])
{

	int state = WAITING_STATE;

try {
	  ofstream logfile;
	  logfile.open("receiver_log.txt");
	  logfile << " --- \n";

  const char* hname = "localhost";  

  //configure receiving port     
  Address * my_addr = new Address(hname, (short)(atoi(argv[1])));
  
  LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
  my_port->setAddress(my_addr);
  my_port->init();

  //configure a sending port to send ACK
  Address * my_tx_addr = new Address(hname, 3005);
  Address * dst_addr =  new Address(argv[2], (short)(atoi(argv[3])));
  mySendingPort *my_tx_port = new mySendingPort();
  my_tx_port->setAddress(my_tx_addr);
  my_tx_port->setRemoteAddress(dst_addr);
  my_tx_port->init();
  
  Message *m = new Message();
  char *filename;
  char *filename_to_save = (char *) argv[4];
  
  cout << "begin receiving..." <<endl;
  Packet *p;
  int ackedSeqNum = 0;
  while (1)
  {
    p = my_port->receivePacket(); 
    if (p !=NULL)
    {  
    	switch(state){
    	case WAITING_STATE:{
    		int seqNum = p->accessHeader()->getIntegerInfo(m->SEQNUM_POS);
    		filename = p->getPayload();
    		Packet * ack_packet = m->make_ack_packet(seqNum);
    		cout << "receiving a packet of seq num " << seqNum << " and filename is " << filename << endl;
    		cout << "Beginning receiving file..." << endl;
    		my_tx_port->sendPacket(ack_packet);
    		remove("out.txt");
    		state = RECEIVING_STATE;
    		break;

    	}
    	case RECEIVING_STATE:{
    		// Stop-and-wait scheme
    		int seqNum = p->accessHeader()->getIntegerInfo(m->SEQNUM_POS);
    		cout << "receiving a packet of seq num " << seqNum << endl;
    		logfile << "Receive " << seqNum << "\n";
    		// special case, if being sent a seqnum = 0, which means the sender does not know
    		// receiver DID receive it, so we send ack again
    		if(seqNum != ackedSeqNum + 1){
    			Packet * ack_packet = m->make_ack_packet(seqNum);
    			cout << "The sender does not know we DID receive packet! Resend ACK for him... " << endl;
    			my_tx_port->sendPacket(ack_packet);
    			break;
    		}

    		char *payload = p->getPayload();
    		if(m->append_data_to_file(filename_to_save,payload,p->getPayloadSize())){//only when write to file success...
    			ackedSeqNum = seqNum;
    			Packet *ack_packet = m->make_ack_packet(seqNum);

        		my_tx_port->sendPacket(ack_packet); //... we send ACK
    		}

    		bool isEOF = p->accessHeader()->getShortIntegerInfo(m->EOF_POS) != 0 ? true : false;
    		cout << "isEOF? " << isEOF << endl;
    		if(isEOF){
    			cout << "Done receive file" <<endl;
    			logfile.close();
    		}
    		break;
    	}
    	default:
    		break;
    	}
    }
  } 
} catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
}  

return 0;
}
