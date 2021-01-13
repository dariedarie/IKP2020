#include "Helper.h"
#include "Frame.h"

Segment CreateSegment(int inputSequenceNumber, char inputData[SIZE_OF_SEGMENT], char inputChecksum) {
	Segment S;
	EOM(S) = DefaultEOM;
	SequenceNumber(S) = inputSequenceNumber;

	memcpy(Data(S), inputData, SIZE_OF_SEGMENT);
	Length(S) = SIZE_OF_SEGMENT;
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


Control CalculateWinSize(int windowSize, int SSTresh, bool dozvola)
{
	float temp = 0;
	if (SSTresh != windowSize)
	{
		if (dozvola == false)
		{
			if (windowSize == PARAM_X)
			{
				SSTresh += windowSize / 2;
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

void printSegment(Segment paket, sockaddr_in clientAddress) {
	char ipAddress[IP_ADDRESS_LEN];
	strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(clientAddress.sin_addr));
	int clientPort = ntohs((u_short)clientAddress.sin_port);

	printf("Client connected from ip: %s, port: %d, sent: ", ipAddress, clientPort);
	for (int i = 0; i < Length(paket); i++) {
		printf("%c", Data(paket)[i]);
	}
	printf("\n");
}

int CalculateSegmentsCount(int len) {

	int num_of_segments = 0;
	if (len < SIZE_OF_SEGMENT) {
		num_of_segments = 1;
	}
	if (len % 4 == 0) {
		num_of_segments = len / 4;
	}
	else {
		num_of_segments = len / 4 + 1;
	}

	printf("Number of segments to send: %d\n", num_of_segments);

	return num_of_segments;

}

int NumOfBytesToTransfer(int num_of_bytes_left) {

	int bytes_to_transfer;

	if (num_of_bytes_left >= SIZE_OF_SEGMENT) {
		bytes_to_transfer = SIZE_OF_SEGMENT;
	}
	else {
		bytes_to_transfer = num_of_bytes_left;
	}

	return bytes_to_transfer;
}

char* getBytesForSegment(int lfs, char* buffer, int bytes_to_transfer, int num_of_bytes_left) {
	char* temp = (char*)malloc(bytes_to_transfer * sizeof(char));

	if (num_of_bytes_left >= 0) {
		memcpy(temp, buffer + lfs * SIZE_OF_SEGMENT, bytes_to_transfer);
	}


	printf("\nSaljem na server: ");

	for (int j = 0; j < bytes_to_transfer; j++) {
		printf("%c", temp[j]);
	}
	printf("\n");

	return temp;
}

int repeatSending(int param_send, int repeatStart, int repeatEnd, char *outgoingBuffer, int lfs, SOCKET clientSocket, sockaddr_in *serverAddress, int sockAddrLen) {
	int sent = param_send;
	for (int i = repeatStart; i < repeatEnd; i++) {
		char temp[SIZE_OF_SEGMENT];
		for (int i = 0; i < SIZE_OF_SEGMENT; i++) {
			temp[i] = outgoingBuffer[lfs *SIZE_OF_SEGMENT + i];
		}
		Segment paket = CreateSegment(i, temp, 0);
		char* segment = (char *)&paket;

		int iResult = sendto(clientSocket,
			segment,
			sizeof(paket),
			0,
			(LPSOCKADDR)serverAddress,
			sockAddrLen);

		printf("\n\t\tPoslao sam opet\n");

		if (iResult == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		sent++;
	}
	return sent;
}

void printAck(sockaddr_in serverAddress, int ack) {

	char ipAddress[IP_ADDRESS_LEN];
	strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(serverAddress.sin_addr));
	int serverPort = ntohs((u_short)serverAddress.sin_port);
	printf("Server connected from ip: %s, port: %d, sent: %d.\n", ipAddress, serverPort, ack);
}

int receiveAcks(int paramSend, PacketACK ack, SOCKET clientSocket, sockaddr_in *serverAddress, int *sockAddrLen) {
	int lar = 0;

	for (int i = 0; i < paramSend; i++) {
		char* acksegment = (char *)&ack;
		if (recvfrom(clientSocket, acksegment, sizeof(ack), 0, (LPSOCKADDR)serverAddress, sockAddrLen) >= 0) {

			lar = NextSequenceNumber(ack);

			printAck(*serverAddress, NextSequenceNumber(ack));
		}
		else {
			printf("Nije primljen ack   i = %d\n", i);
		}
	}

	return lar;
}

int sendSegment(Segment *paket, SOCKET clientSocket, sockaddr_in *serverAddress, int sockAddrLen) {
	char* segment = (char *)paket;

	int iResult = sendto(clientSocket,
		segment,
		sizeof(*paket),
		0,
		(LPSOCKADDR)serverAddress,
		sockAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("sendto failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

}

void sendFinalSegment(SOCKET clientSocket, sockaddr_in *serverAddress, int *sockAddrLen) {
	PacketACK finalACK;
	NextSequenceNumber(finalACK) = 0;
	Segment finalSegment;
	char final_message[4] = { 0,0,0,0 };
	finalSegment = CreateSegment(-1, final_message, 0);
	EOM(finalSegment) = 0x2;
	while (NextSequenceNumber(finalACK) == 0) {

		if (NextSequenceNumber(finalACK) == -1) {
			break;
		}


		char* fsegment = (char *)&finalSegment;

		sendto(clientSocket, fsegment, sizeof(finalSegment), 0, (struct sockaddr *) serverAddress, *sockAddrLen);

		printf("Saljem final segment\n");

		char* acksegment = (char *)&finalACK;
		recvfrom(clientSocket, acksegment, sizeof(finalACK), 0, (struct sockaddr*) serverAddress, sockAddrLen);
	}
}

int startSending(char* outgoingBuffer, SOCKET clientSocket, sockaddr_in *serverAddress, int *sockAddrLen) {
	int windowSize = START_WINDOW_SIZE;
	Segment paket;
	PacketACK ack = CreatePacketACK(INITIAL_NEXT_SEQUENCE_NUM);
	int advertisedWindowSize = ADVERTISED_WINDOW_SIZE;

	SendingWindow sendingWindow;

	createNew(&sendingWindow, windowSize);

	int repeatEnd, repeatStart;

	int paramSend = 0;
	int SSTresh = 0;
	bool dozvola = false;


	while (1) {

		LAR(sendingWindow) = 0;
		LFS(sendingWindow) = 0;
		paramSend = 0;
		repeatStart = LAR(sendingWindow);
		repeatEnd = LFS(sendingWindow);

		printf("Size of window: %d\n", windowSize);

		printf("Enter message to server:\n");

		gets_s(outgoingBuffer, OUTGOING_BUFFER_SIZE);
		while (1) {


			repeatStart = LAR(sendingWindow);
		
			repeatEnd = LFS(sendingWindow);

			paramSend = 0;

			int num_of_segments = CalculateSegmentsCount(strlen(outgoingBuffer));

			int num_of_bytes_left = strlen(outgoingBuffer) - LAR(sendingWindow) * SIZE_OF_SEGMENT;

			
			while (LFS(sendingWindow) < LAR(sendingWindow) + windowSize && LFS(sendingWindow) < num_of_segments) {

				int length_of_segment = SIZE_OF_SEGMENT;

				int bytes_to_transfer = NumOfBytesToTransfer(num_of_bytes_left);



				char* temp = getBytesForSegment(LFS(sendingWindow), outgoingBuffer, bytes_to_transfer, num_of_bytes_left);
				num_of_bytes_left -= bytes_to_transfer;
				paket = CreateSegment(LFS(sendingWindow), temp, 0);


				Length(paket) = bytes_to_transfer;

				LFS(sendingWindow) = LFS(sendingWindow) + 1;


				if (sendSegment(&paket, clientSocket, serverAddress, *sockAddrLen) == 1) {
					return 1;
				}

				paramSend++;
			}


			paramSend = repeatSending(paramSend, repeatStart, repeatEnd, outgoingBuffer, LFS(sendingWindow), clientSocket, serverAddress, *sockAddrLen);

			LAR(sendingWindow) = receiveAcks(paramSend, ack, clientSocket, serverAddress, sockAddrLen);


			if (LAR(sendingWindow) > num_of_segments - 1) {
				printf("Poslao sam celu poruku\n");
				strcpy(outgoingBuffer, "");
				Control c;
				c = CalculateWinSize(windowSize, SSTresh, dozvola);
				windowSize = c.windowSize;
				SSTresh = c.SSTresh;
				dozvola = c.dozvola;

				break;
			}
		}


		sendFinalSegment(clientSocket, serverAddress, sockAddrLen);
		char negative[SHUT_DOWN_BUFFER];
		printf("Zelite li da ugasite server(Y/N)?\n");
		gets_s(negative, SHUT_DOWN_BUFFER);
		if(strcmp(negative,"Y")==0)
		{
			sendto(clientSocket, negative, sizeof(SHUT_DOWN_BUFFER), 0, (struct sockaddr *) serverAddress, *sockAddrLen);
			return 200;
		}

	}
}

PacketACK sendFinalAck(PacketACK ack, SOCKET serverSocket, sockaddr_in* clientAddress, int sockAddrLen) {
	ack = CreatePacketACK(-1);
	char *sendBuf = (char *)&ack;

	int iResult = sendto(serverSocket, sendBuf, sizeof(ack), 0, (LPSOCKADDR)clientAddress, sockAddrLen);
	printf("Klijent je zavrsio slanje svoje poruke\n");

	return ack;
}