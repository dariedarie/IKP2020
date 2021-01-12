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

void createNew(SendingWindow *s, int windowSize);

/*void createNew(SendingWindow *s, int windowSize) {
	s->sendingWindowSize = windowSize;
	s->lastAckReceived = NONE;
	s->lastFrameSent = NONE;
}*/
#endif