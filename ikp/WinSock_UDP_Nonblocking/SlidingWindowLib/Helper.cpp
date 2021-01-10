#include <stdio.h>
#include "Helper.h"
#include <stdlib.h>
#include <time.h>



//						redni broj segmenta      podatak(2 bajta??)
Segment CreateSegment( int inputSequenceNumber,  char inputData,  char inputChecksum) {
	Segment S;
	EOM(S) = DefaultEOM;	// nepotrebno
	SequenceNumber(S) = inputSequenceNumber;
	Data(S) = inputData;	
	return S;
}
//						r.b. sledeceg segmenta koji se ocekuje
PacketACK CreatePacketACK( int inputNextSequenceNumber) {
	PacketACK P;
	ACK(P) = DefaultACK;
	NextSequenceNumber(P) = inputNextSequenceNumber;
	return P;
}