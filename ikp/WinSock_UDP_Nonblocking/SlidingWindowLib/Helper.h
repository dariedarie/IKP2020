#ifndef _HELPER_H_
#define _HELPER_H_
#define SIZE_OF_SEGMENT 4

/* *** MODULE *** */
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>


typedef struct {
	char EOM;
	int SequenceNumber;
	char Data[SIZE_OF_SEGMENT];
} Segment;

typedef struct {
	char ACK;
	int NextSequenceNumber;
} PacketACK;

typedef struct {
	int windowSize;
	int SSTresh;
	bool dozvola;
} Control;


/* Definition */
#define DefaultEOM 0x1;
#define DefaultACK 0x6;

/* Selector */
#define EOM(S) ((S).EOM)
#define SequenceNumber(S) ((S).SequenceNumber)
#define Data(S) ((S).Data)
#define ACK(P) ((P).ACK)
#define NextSequenceNumber(P) ((P).NextSequenceNumber)
#define SIZE_OF_SEGMENT 4
#define PARAM_X 12

Segment CreateSegment(int inputSequenceNumber, char inputData[SIZE_OF_SEGMENT], char inputChecksum);

PacketACK CreatePacketACK(int inputNextSequenceNumber, int inputAdvertisedWindowSize, char inputChecksum);

Control CalculateWinSize(int windowSize, int SSTresh,bool dozvola);

#endif