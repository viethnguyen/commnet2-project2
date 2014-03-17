
# Specify the compiler
CC = g++

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -g -Wall
LIBS = -pthread

# Make the source
all:	sender receiver forwarder

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp
 
message.o: message.h message.cpp
	$(CC) $(CCOPTS) -c message.cpp
     
sender:  sender.cpp common.o message.o 
	$(CC) $(CCOPTS) $(LIBS) common.o message.o sender.cpp -o sender

receiver: receiver.cpp common.o message.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o receiver.cpp -o receiver

forwarder: forwarder.cpp common.o message.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o forwarder.cpp -o forwarder
	
clean :
	rm -f common.o message.o sender receiver forwarder
