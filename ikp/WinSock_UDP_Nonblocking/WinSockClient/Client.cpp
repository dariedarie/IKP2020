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

	// funkcija koja obavlja svu klijentsku logiku
	startSending(outgoingBuffer, clientSocket, &serverAddress, &sockAddrLen);


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