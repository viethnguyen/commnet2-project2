#include "common.h"
#include "newport.h"
#include "message.h"
#include <iostream>
#include <fstream>
using namespace std;

//state of the sender
const int IDLE_STATE = 0;
const int SENDING_STATE = 1;

char* read_all_bytes(const char *filename){

	ifstream f1(filename);
	f1.seekg(0, ios::end);
	size_t len = f1.tellg();
	char *ret = new char[len];
	f1.seekg(0, ios::beg);
	f1.read(ret, len);
	f1.close();
	return ret;
}

/**
 * @param: argv[1] destination host address
 * 			argv[2] destination port number
 * 			argv[3] listening port number
 * 			argv[4] filename
 */
int main(int argc, const char * argv[])
{
 try {

  ofstream logfile;
  logfile.open("sender_log.txt");
  logfile << " --- \n";


  const char* hname = "localhost";       
  Address * my_tx_addr = new Address(hname, 3000);

  //configure sending port
  Address * dst_addr =  new Address(argv[1], (short)(atoi(argv[2])));
  mySendingPort *my_port = new mySendingPort();
  my_port->setAddress(my_tx_addr);
  my_port->setRemoteAddress(dst_addr);
  my_port->init();

  //configure receiving port to listen to ACK frames
  Address * my_rx_addr = new Address(hname, (short)(atoi(argv[3])));
  LossyReceivingPort *my_rx_port = new LossyReceivingPort(0.2);
  my_rx_port->setAddress(my_rx_addr);
  my_rx_port->init();

  //create a single packet
  Packet * my_packet;
  my_packet = new Packet();
  my_packet->setPayloadSize(100);
  PacketHdr *hdr = my_packet->accessHeader();
  hdr->setOctet('D',0);
  hdr->setOctet('A',1);
  hdr->setOctet('T',2);
  hdr->setIntegerInfo(1,3);

  //init a file transfer session
  Message *m = new Message();
  char *filename = (char *) argv[4];
  Packet *init_packet = m->make_init_packet(filename);
 

  my_port->sendPacket(init_packet);
  my_port->lastPkt_ = init_packet;
  cout << "Init packet is sent!" <<endl;
  my_port->setACKflag(false);
  //schedule retransmit
  my_port->timer_.startTimer(2.5);  

  cout << "begin waiting for ACK..." <<endl;
  Packet *pAck;
  while (!my_port->isACKed()){
        pAck = my_rx_port->receivePacket();

        if (pAck!= NULL)
        {
	     my_port->setACKflag(true);
	     my_port->timer_.stopTimer();
         cout << "OK. Now begin to transfer file..." <<endl;
        }
  };  
    
  //begining transfer
   m->prepare_file_to_send(filename);
   int np = m->get_number_of_packets();

   //stop-and-wait scheme
   for(int i = 1; i <= np; i++){
	   Packet *data_packet = m->make_data_packet(i);
	   my_port->sendPacket(data_packet);
	   my_port->lastPkt_ = data_packet;
	   cout << "Data packet seqnum " << i << " is sent!" << endl;
	   logfile << "Send " << i << "\n";
	   my_port->setACKflag(false);
	   //schedule retransmit
	   my_port->timer_.startTimer(2.5);

	   cout << "begin waiting for ACK ... " << endl;

	   while(!my_port->isACKed()){
		   pAck = my_rx_port->receivePacket();
		   if(pAck !=  NULL)
		   {
			   int seqNum = pAck->accessHeader()->getIntegerInfo(m->SEQNUM_POS);
			   if(seqNum == i) {
				   my_port->setACKflag(true);
				   my_port->timer_.stopTimer();
				   cout << "Data packet seqnum " << i << "/ " << np << " is received successfully!" << endl;
			   }
		   }
	   }
   }
   logfile.close();

 } catch (const char *reason ) {
    cerr << "Exception:" << reason << endl;
    exit(-1);
  }  

 return 0;
}


