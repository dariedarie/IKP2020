#ifndef _HELPER_H_
#define _HELPER_H_
//MODULE AND DEFINE
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <winsock2.h>
#include "Frame.h"

#define SIZE_OF_SEGMENT 4
#define IP_ADDRESS_LEN 16
#define START_WINDOW_SIZE 10
#define IP_ADDRESS_LEN 16
#define ADVERTISED_WINDOW_SIZE 256
#define OUTGOING_BUFFER_SIZE 1024
#define SHUT_DOWN_BUFFER 1024
#define INITIAL_NEXT_SEQUENCE_NUM -111
#define _WINSOCK_DEPRECATED_NO_WARNINGS


//STRUCTS

typedef struct {
	char EOM;
	int SequenceNumber;
	char Data[SIZE_OF_SEGMENT];
	int Length;
} Segment;

typedef struct {
	char ACK;
	int NextSequenceNumber;
} PacketACK;

typedef struct {
	int windowSize;
	int SSTresh;
	bool permission;
} Control;


//DEFINITION
#define DefaultEOM 0x1;
#define DefaultACK 0x6;

//SELECTOR 
#define EOM(S) ((S).EOM)
#define SequenceNumber(S) ((S).SequenceNumber)
#define Data(S) ((S).Data)
#define ACK(P) ((P).ACK)
#define NextSequenceNumber(P) ((P).NextSequenceNumber)
#define SIZE_OF_SEGMENT 4
#define PARAM_X 12
#define Length(S) ((S).Length)

//CALL OF FUNCTIONS

Segment CreateSegment(int inputSequenceNumber, char inputData[SIZE_OF_SEGMENT], char inputChecksum);

PacketACK CreatePacketACK(int inputNextSequenceNumber);

Control CalculateWinSize(int windowSize, int SSTresh, bool dozvola);

void printSegment(Segment paket, sockaddr_in clientAddress);

int CalculateSegmentsCount(int len);

int NumOfBytesToTransfer(int num_of_bytes_left);

char* getBytesForSegment(int lfs, char* buffer, int bytes_to_transfer, int num_of_bytes_left);

int repeatSending(int param_send, int repeatStart, int repeatEnd, char *outgoingBuffer, int lfs, SOCKET clientSocket, sockaddr_in *serverAddress, int sockAddrLen);

void printAck(sockaddr_in serverAddress, int ack);

int receiveAcks(int paramSend, PacketACK ack, SOCKET clientSocket, sockaddr_in *serverAddress, int *sockAddrLen);

int sendSegment(Segment *paket, SOCKET clientSocket, sockaddr_in *serverAddress, int sockAddrLen);

void sendFinalSegment(SOCKET clientSocket, sockaddr_in *serverAddress, int *sockAddrLen);

int startSending(char* outgoingBuffer, SOCKET clientSocket, sockaddr_in *serverAddress, int *sockAddrLen);

PacketACK sendFinalAck(PacketACK ack, SOCKET serverSocket, sockaddr_in* clientAddress, int sockAddrLen);
#endif