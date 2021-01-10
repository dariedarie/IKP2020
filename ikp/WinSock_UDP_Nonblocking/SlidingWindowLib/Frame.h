#ifndef FRAME_H
#define FRAME_H
// poslednji frejm koji smo poslali je uvek ispred (ili na istom mestu) kao poslednji frejm za koji imamo ack
// kada nam stizu ack-ovi od servera pomera se LAR unapred
// kada saljemo frejmove pomera se LFS unapred

typedef struct {
	 int sendingWindowSize;
	 int lastAckReceived;
	 int lastFrameSent;
} SendingWindow;

#define NONE 0

#define SWS(S) (S).sendingWindowSize // sirina prozora koji se salje
#define LAR(S) (S).lastAckReceived	// redni broj poslednjeg acka koji je stigao
#define LFS(S) (S).lastFrameSent	//redni broj poslednjeg oslatog segmenta

void createNew(SendingWindow *s,  int windowSize) {
	s->sendingWindowSize = windowSize;
	s->lastAckReceived = NONE;
	s->lastFrameSent = NONE;
}

// ne znam sta ce nam ovo
void createNewSendingWindow(SendingWindow *s,  int windowSize,  int lastAckReceived,  int lastFrameSent) {
	SWS(*s) = windowSize;
	LAR(*s) = lastAckReceived;
	LFS(*s) = lastFrameSent;
}


void receiveAcks(SendingWindow *s, int ack) {
	// ako je ack veci od poslednjeg prethodnog upisi ga, ako nije ostavi stari 
	LAR(*s) = (LAR(*s) < ack) ? ack : LAR(*s);
}

							//broj frejmova koje saljemo
void sendFrames(SendingWindow s, int frameCount) {
	// ako je broj frejmova koji saljemo veci od sirine prozora smanji ga
	frameCount = (frameCount < SWS(s)) ? frameCount : SWS(s);

	// ako je poslednji frejm koji saljemo za CELU SIRINU PROZORA ispred poslednjeg frejma za koji smo dobili ack  
	if (LFS(s) + frameCount > SWS(s) + LAR(s)) {
		// smanjimo ga tako da bude jednak broju frejmova za koje jos nemamo ack + sirina prozora
		frameCount = SWS(s) + LAR(s) - LFS(s);
	}

	// povecavamo vrednost poslednjeg frejma koji smo poslali
	LFS(s) += frameCount;
}

#endif