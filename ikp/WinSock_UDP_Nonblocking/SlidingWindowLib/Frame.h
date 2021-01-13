#ifndef FRAME_H
#define FRAME_H
#define NONE 0
#define SWS(S) (S).sendingWindowSize 
#define LAR(S) (S).lastAckReceived	
#define LFS(S) (S).lastFrameSent	

typedef struct {
	int sendingWindowSize;
	int lastAckReceived;
	int lastFrameSent;
} SendingWindow;


void createNew(SendingWindow *s, int windowSize);
#endif