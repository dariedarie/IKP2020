#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include "Helper.cpp"

#define SERVER_PORT 15000
#define OUTGOING_BUFFER_SIZE 1024
#define ACCESS_BUFFER_SIZE 1024
// for demonstration purposes we will hard code
// local host ip adderss
#define SERVER_IP_ADDERESS "127.0.0.1"
#define IP_ADDRESS_LEN 16
#define START_WINDOW_SIZE 10
#define ADVERTISED_WINDOW_SIZE 256
#define SEGMENT_SIZE 4

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();

int main(int argc, char* argv[])
{
	// Server address
	sockaddr_in serverAddress;
	// size of sockaddr structure    
	int sockAddrLen = sizeof(struct sockaddr);
	// buffer we will use to store message
	char outgoingBuffer[OUTGOING_BUFFER_SIZE];
	char rcvBuffer[ACCESS_BUFFER_SIZE];
	// port used for communication with server
	int serverPort = SERVER_PORT;
	// variable used to store function return value
	int iResult;

	// Initialize windows sockets for this process
	InitializeWindowsSockets();

	// Initialize serverAddress structure
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
	serverAddress.sin_port = htons((u_short)serverPort);

	// create a socket
	SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

// check if socket creation succeeded
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// inicijalna sirina prozora
	int windowSize = START_WINDOW_SIZE;
	Segment paket;
	PacketACK ack;
	int advertisedWindowSize = 256;

	int bufferSizeOffset = 0;

	char c;


	SendingWindow sendingWindow;
	// inicijalizacija prozora
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

		printf("Enter message to server:\n");
		printf("Size of window: %d\n", windowSize);
		// Read string from user into outgoing buffer
		gets_s(outgoingBuffer, OUTGOING_BUFFER_SIZE);
		while (1) {


			
			repeatStart = LAR(sendingWindow);
			repeatEnd = LFS(sendingWindow);
			paramSend = 0;

			int num_of_segments = 0;
			if (strlen(outgoingBuffer) < SEGMENT_SIZE) {
				num_of_segments = 1;
			}
			if (strlen(outgoingBuffer) % 4 == 0) {
				num_of_segments = strlen(outgoingBuffer) / 4;
			}
			else {
				num_of_segments = strlen(outgoingBuffer) / 4 + 1;
			}

			printf("Number of segments to send: %d\n", num_of_segments);

			int num_of_bytes_left = strlen(outgoingBuffer);

			
			while (LFS(sendingWindow) < LAR(sendingWindow) + windowSize && LFS(sendingWindow) < num_of_segments && paramSend < advertisedWindowSize) {

				int bytes_to_transfer;
				if (num_of_bytes_left >= SEGMENT_SIZE) {
					bytes_to_transfer = SEGMENT_SIZE;
				}
				else {
					bytes_to_transfer = num_of_bytes_left;
				}


				num_of_bytes_left -= bytes_to_transfer;

				char* temp = (char*)malloc(bytes_to_transfer * sizeof(char));

				if (num_of_bytes_left >= 0) {
					memcpy(temp, outgoingBuffer + LFS(sendingWindow) * SEGMENT_SIZE, bytes_to_transfer);
				}


				memset(Data(paket), 0, SEGMENT_SIZE);
				printf("Saljem na server: ");


				for (int j = 0; j < bytes_to_transfer; j++) {
					printf("%c", temp[j]);
				}
				printf("\n");
				paket = CreateSegment(LFS(sendingWindow), temp, 0);
				
				LFS(sendingWindow) = LFS(sendingWindow) + 1;
				char* segment = (char *)&paket;

				iResult = sendto(clientSocket,
					segment,
					sizeof(paket),
					0,
					(LPSOCKADDR)&serverAddress,
					sockAddrLen);

				if (iResult == SOCKET_ERROR)
				{
					printf("sendto failed with error: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					WSACleanup();
					return 1;
				}

				paramSend++;
			}



			int i;
			for (i = repeatStart; i < repeatEnd; i++) {
				char temp[SEGMENT_SIZE];
				for (int i = 0; i < SEGMENT_SIZE; i++) {
					temp[i] = outgoingBuffer[LFS(sendingWindow)*SEGMENT_SIZE + i];
				}
				paket = CreateSegment(i, temp, 0);
				char* segment = (char *)&paket;

				iResult = sendto(clientSocket,
					segment,
					sizeof(paket),
					0,
					(LPSOCKADDR)&serverAddress,
					sockAddrLen);

				printf("Poslao sam opet\n");

				if (iResult == SOCKET_ERROR)
				{
					printf("sendto failed with error: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					WSACleanup();
					return 1;
				}

				paramSend++;
			}



			for (i = 0; i < paramSend; i++) {
				char* acksegment = (char *)&ack;
				if (recvfrom(clientSocket, acksegment, sizeof(ack), 0, (LPSOCKADDR)&serverAddress, &sockAddrLen) >= 0) {
					LAR(sendingWindow) = NextSequenceNumber(ack);

					char ipAddress[IP_ADDRESS_LEN];
					strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(serverAddress.sin_addr));
					int serverPort = ntohs((u_short)serverAddress.sin_port);
					printf("Server connected from ip: %s, port: %d, sent: %d.\n", ipAddress, serverPort, LAR(sendingWindow));
				}
				else {
					printf("Nije primljen ack   i = %d\n", i);
				}
			}


		


			if (LAR(sendingWindow) > num_of_segments - 1) {
				printf("Poslao sam celu poruku\n");
				strcpy(outgoingBuffer, "");
				Control c;
				c = CalculateWinSize(windowSize, SSTresh,dozvola);
				windowSize = c.windowSize;
				SSTresh = c.SSTresh;
				dozvola = c.dozvola;

				break;
			}
		}


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

			sendto(clientSocket, fsegment, sizeof(finalSegment), 0, (struct sockaddr *) &serverAddress, sockAddrLen);

			printf("Saljem final segment\n");

			char* acksegment = (char *)&finalACK;
			recvfrom(clientSocket, acksegment, sizeof(finalACK), 0, (struct sockaddr*) &serverAddress, &sockAddrLen);
		}
	}

	//printf("Message sent to server, press any key to exit.\n");
	//_getch();

	iResult = closesocket(clientSocket);
	if (iResult == SOCKET_ERROR)
	{
		printf("closesocket failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printf("WSACleanup failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	return true;
}
