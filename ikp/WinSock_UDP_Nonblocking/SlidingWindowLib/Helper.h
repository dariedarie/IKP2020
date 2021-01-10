#ifndef _HELPER_H_
#define _HELPER_H_

/* *** MODULE *** */
#include <string>
#include <stdint.h>

//using namespace std;

// deo poruke koji se salje
// unutar jednog sliding windowa imamo nekoliko segmenata
typedef struct {
	 char EOM;	
	 int SequenceNumber;
	 char Data;
} Segment;

// struktura ack poruke koju klijent prima od servera
typedef struct {
	 char ACK;
	 int NextSequenceNumber;
} PacketACK;

/* Definition */
#define DefaultEOM 0x1;
#define DefaultACK 0x6;

/* Selector */
#define EOM(S) ((S).EOM)
#define SequenceNumber(S) ((S).SequenceNumber)
#define Data(S) ((S).Data)
#define ACK(P) ((P).ACK)
#define NextSequenceNumber(P) ((P).NextSequenceNumber)

Segment CreateSegment( int inputSequenceNumber,  char inputData,  char inputChecksum);

PacketACK CreatePacketACK( int inputNextSequenceNumber,  int inputAdvertisedWindowSize,  char inputChecksum);

#endif