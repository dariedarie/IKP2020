#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include "Frame.h"
#include "Helper.h"
#include "Helper.cpp"

#define SERVER_PORT 15000
#define OUTGOING_BUFFER_SIZE 1024
#define ACCESS_BUFFER_SIZE 1024
// for demonstration purposes we will hard code
// local host ip adderss
#define SERVER_IP_ADDERESS "127.0.0.1"
#define IP_ADDRESS_LEN 16

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
	int windowSize = 10;
	Segment paket;
	PacketACK ack;
	// valjda da se odredi sirina buffera dok se cita fajl
	int counterBuffer = 0;
	// ovo valjda broji segmente
	int counterSequence = 0;
	// isto nesto za fajl
	int alreadyReadAll = 0;
	// nepotrebno
	int advertisedWindowSize = 256;

	int bufferSizeOffset = 0;

	char c;


	SendingWindow sendingWindow;
	// inicijalizacija prozora
	createNew(&sendingWindow, windowSize);


	int repeatEnd, repeatStart;
	int paramSend = 0;
	int iterator = 0;
	int SSTresh = 0;
	bool dozvola = false;



	while (1) {

		LAR(sendingWindow) = 0;
		LFS(sendingWindow) = 0;
		paramSend = 0;
		repeatStart = LAR(sendingWindow);
		repeatEnd = LFS(sendingWindow);

		printf("Enter message to server:\n");
		printf("Size window:%d\n", windowSize);
		// Read string from user into outgoing buffer
		gets_s(outgoingBuffer, OUTGOING_BUFFER_SIZE);
		while (1) {


			//				zadnji segment za koji je stigao ack (trenutno 0)
			repeatStart = LAR(sendingWindow);
			//			zadnji segment koji smo poslali (trenutno 0)
			repeatEnd = LFS(sendingWindow);

			// povecava se za 1 nakon svakog slanja segmenta
			// kad se prodje ceo while vraca se na 0
			paramSend = 0;

			//dok je zadnji potvrdjen segment blizu zadnjeg poslatog (razlika manja od sirine prozora)
			//dok nismo dosli do kraja buffera 
			while (LFS(sendingWindow) < LAR(sendingWindow) + windowSize && LFS(sendingWindow) < strlen(outgoingBuffer) && paramSend < advertisedWindowSize) {

				// paket ce imati podatke duzine 1 char
				paket = CreateSegment(LFS(sendingWindow), outgoingBuffer[LFS(sendingWindow)], 0);
				// ovo nam nece trebati
				// pomeri LFS za jedno mesto u napred (jer frejm koji smo napravili postaje lfs)
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
			// posalji sve pakete za koje nije stigao ack
			// iterira se od LAR do LFS 
			for (i = repeatStart; i < repeatEnd; i++) {
				paket = CreateSegment(i, outgoingBuffer[i], 0);
				char* segment = (char *)&paket;

				iResult = sendto(clientSocket,
					segment,
					sizeof(paket),
					0,
					(LPSOCKADDR)&serverAddress,
					sockAddrLen);

				printf("Poslao sam opet");

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
				//printf("FOR PETLJA\n PARAM SEND: %d\n", paramSend);
				char* acksegment = (char *)&ack;
				if (recvfrom(clientSocket, acksegment, sizeof(ack), 0, (LPSOCKADDR)&serverAddress, &sockAddrLen) >= 0) {
					// pomeramo sending window tako da prvi sledeci segment koji se salje bude onaj koji nema potvrdu da je stigao na server
					/*if (NextSequenceNumber(ack) == -1) {
						continue;
					}*/
					LAR(sendingWindow) = NextSequenceNumber(ack);

					printf("VRACAS:%d\n", NextSequenceNumber(ack));
					char ipAddress[IP_ADDRESS_LEN];
					strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(serverAddress.sin_addr));
					int serverPort = ntohs((u_short)serverAddress.sin_port);
					printf("Server connected from ip: %s, port: %d, sent: %d.\n", ipAddress, serverPort, LAR(sendingWindow));
				}
				else {
					printf("Nije primljen ack   i = %d\n", i);
				}
			}


			int paramX = 12;
			float temp = 0;


			if (LAR(sendingWindow) > strlen(outgoingBuffer) - 1) {
				printf("Poslao sam celu poruku\n");
				strcpy(outgoingBuffer, "");
				if (SSTresh != windowSize)
				{
					if (dozvola == false)
					{
						if (windowSize == paramX)
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

				break;
			}
		}

		//SERVER NE MOZE UNAPRED DA ZNA KOLIKU MU PORUKU KLIJENT SALJE
		//KADA SE IZADJE IZ OVE WHILE PETLJE POSLACEMO PORUKU SERVERU KOJA CE DA OZNACAVA KRAJ SLANJA TRENUTNE PORUKE
		//TA ZADNJA PORUKA IMACE DRUGACIJI FLAG 'SOH' OD NORMALNIH PORUKA
		//PO TOME CE SERVER ZNATI DA TREBA DA OCEKUJE NOVU PORUKU (URADICE 'BRAKE')

		PacketACK finalACK;
		NextSequenceNumber(finalACK) = 0;
		Segment finalSegment;
		finalSegment = CreateSegment(-1, 0, 0);
		EOM(finalSegment) = 0x2;
		while (NextSequenceNumber(finalACK) == 0) {

			if (NextSequenceNumber(finalACK) == -1) {
				break;
			}

			// salji poruku dok ne dobijes potvrdu da je stigla na server

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
