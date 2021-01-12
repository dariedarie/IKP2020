#ifndef FRAME_H
#define FRAME_H

typedef struct {
	int sendingWindowSize;
	int lastAckReceived;
	int lastFrameSent;
} SendingWindow;

#define NONE 0

#define SWS(S) (S).sendingWindowSize 
#define LAR(S) (S).lastAckReceived	
#define LFS(S) (S).lastFrameSent	

void createNew(SendingWindow *s, int windowSize) {
	s->sendingWindowSize = windowSize;
	s->lastAckReceived = NONE;
	s->lastFrameSent = NONE;
}
/*
void createNewSendingWindow(SendingWindow *s, int windowSize, int lastAckReceived, int lastFrameSent) {
	SWS(*s) = windowSize;
	LAR(*s) = lastAckReceived;
	LFS(*s) = lastFrameSent;
}


void receiveAcks(SendingWindow *s, int ack) {
	LAR(*s) = (LAR(*s) < ack) ? ack : LAR(*s);
}


void sendFrames(SendingWindow s, int frameCount) {
	
	frameCount = (frameCount < SWS(s)) ? frameCount : SWS(s);

	if (LFS(s) + frameCount > SWS(s) + LAR(s)) {
		frameCount = SWS(s) + LAR(s) - LFS(s);
	}
	LFS(s) += frameCount;
}*/

#endif