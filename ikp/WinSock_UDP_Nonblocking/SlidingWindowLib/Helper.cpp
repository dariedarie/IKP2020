#include "Helper.h"
#include "Frame.h"

  
Segment CreateSegment(int inputSequenceNumber, char inputData[SIZE_OF_SEGMENT], char inputChecksum) {
	Segment S;
	EOM(S) = DefaultEOM;
	SequenceNumber(S) = inputSequenceNumber;

	memcpy(Data(S), inputData, SIZE_OF_SEGMENT);
	return S;
}

PacketACK CreatePacketACK(int inputNextSequenceNumber) {
	PacketACK P;
	ACK(P) = DefaultACK;
	NextSequenceNumber(P) = inputNextSequenceNumber;
	return P;
}

void createNew(SendingWindow *s, int windowSize) {
	s->sendingWindowSize = windowSize;
	s->lastAckReceived = NONE;
	s->lastFrameSent = NONE;
}

Control CalculateWinSize(int windowSize,int SSTresh, bool dozvola)
{
	float temp = 0;
	if (SSTresh != windowSize)
	{
		if (dozvola == false)
		{
			if (windowSize == PARAM_X)
			{
				SSTresh += windowSize / 2;
				printf("\nSSTresh: %d\n", SSTresh);
				windowSize = 10;

			}
			else
			{
				windowSize++;
			}

		}
		else
		{
			temp = (float)(SSTresh)+((float)SSTresh / (float)windowSize);
			int temp3 = round(temp);
			windowSize += temp3;
			dozvola = true;
		}
	}
	else
	{
		temp = (float)(SSTresh)+((float)SSTresh / (float)windowSize);
		int temp2 = round(temp);
		windowSize += temp2;
		dozvola = true;
	}

	Control c;
	c.SSTresh = SSTresh;
	c.windowSize = windowSize;
	c.dozvola = dozvola;

	return c;
}