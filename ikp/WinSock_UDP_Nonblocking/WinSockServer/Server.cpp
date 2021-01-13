#include <winsock2.h>
#include <stdio.h>
#include "MemoryPool.cpp"
#include "Helper.cpp"

//#include "ServerFunctions.cpp"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 50
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16
#define BUFFER_POOL_SIZE 10000
#define START_WINDOW_SIZE 10
#define SEGMENT_SIZE 4
#define INITIAL_NSN -1111


// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();

int main(int argc, char* argv[])
{
	// Server address
	sockaddr_in serverAddress;
	// Server's socket
	int serverPort = SERVER_PORT;
	// size of sockaddr structure
	int sockAddrLen = sizeof(struct sockaddr);
	// buffer we will use to receive client message
	char accessBuffer[SEGMENT_SIZE];
	bool alreadyReceived[ACCESS_BUFFER_SIZE];

	char sendBuffer[ACCESS_BUFFER_SIZE];
	// variable used to store function return value
	int iResult;

	POOL *buffer_pool = pool_create(BUFFER_POOL_SIZE);

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Initialize serverAddress structure used by bind
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET; /*set server address protocol family*/
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(serverPort);

	// create a socket
	SOCKET serverSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram supporting socket
		IPPROTO_UDP); // UDP

// check if socket creation succeeded
	if (serverSocket == INVALID_SOCKET)
	{
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind port number and local address to socket
	iResult = bind(serverSocket, (LPSOCKADDR)&serverAddress, sizeof(serverAddress));

	if (iResult == SOCKET_ERROR)
	{
		printf("Socket bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// Set socket to nonblocking mode
	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	printf("Simple UDP server started and waiting clients.\n");



	// Main server loop
	while (1)
	{

		// clientAddress will be populated from recvfrom
		sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof(sockaddr_in));

		// set whole buffer to zero
		memset(accessBuffer, 0, SEGMENT_SIZE);

		// Initialize select parameters
		FD_SET set;
		timeval timeVal;

		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(serverSocket, &set);

		// Set timeouts to zero since we want select to return
		// instantaneously
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(SERVER_SLEEP_TIME);
			continue;
		}

		int windowSize = START_WINDOW_SIZE;
		int recv_len;
		Segment paket;
		PacketACK ack = CreatePacketACK(INITIAL_NSN);
		int lastFrameCorrect = 0;
		// poslednji primljen frejm
		int LFR = 0;
		bool first = true;
		// poslednji frejm za koji je poslat ack
		int LAF = windowSize;
		int counterBuffer = 0;
		int finish = 0;

		int novi_ack = INITIAL_NSN / 2;
		int stari_ack = INITIAL_NSN;

		while (1) {

			if (NextSequenceNumber(ack) == -1) {
				break;
			}

			memset(Data(paket), 0, SEGMENT_SIZE);
			char * recvBuf = (char *)&paket;
			if (recvfrom(serverSocket, recvBuf, sizeof(paket), 0, (LPSOCKADDR)&clientAddress, &sockAddrLen) >= 0) {
				if (EOM(paket) == 0x02) {
					ack = sendFinalAck(ack, serverSocket, &clientAddress, sockAddrLen);
					break;
				}
				else if(strcmp(recvBuf,"Y")==0)
				{
					pool_destroy(buffer_pool);
					// if we are here, it means that server is shutting down
					// close socket and unintialize WinSock2 library
					iResult = closesocket(serverSocket);
					if (iResult == SOCKET_ERROR)
					{
						printf("closesocket failed with error: %d\n", WSAGetLastError());
						return 1;
					}

					iResult = WSACleanup();
					if (iResult == SOCKET_ERROR)
					{
						printf("WSACleanup failed with error: %d\n", WSAGetLastError());
						return 1;
					}

					printf("Server successfully shut down.\n");


					return 0;
				
				}
				

				// ako je pristigli paket unutar prozora	
				if (SequenceNumber(paket) >= LFR && SequenceNumber(paket) <= LAF) {
					if (SequenceNumber(paket) == LFR) {
						LFR++;

						char *mem = (char*)pool_alloc(buffer_pool, Length(paket));

						for (int i = 0; i < Length(paket); i++) {
							mem[i] = Data(paket)[i];
						}

						printSegment(paket, clientAddress);

						print_pool(buffer_pool);

					}
				}
				LAF = LFR + windowSize;

			}


			ack = CreatePacketACK(LFR);
			char *sendBuf = (char *)&ack;

			// da ne bi vise puta poslao ack od prosle poruke
			novi_ack = NextSequenceNumber(ack);
			if (novi_ack != stari_ack) {
				iResult = sendto(serverSocket, sendBuf, sizeof(ack), 0, (LPSOCKADDR)&clientAddress, sockAddrLen);
			}
			stari_ack = NextSequenceNumber(ack);

		}

		if (iResult == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}

	}


	pool_destroy(buffer_pool);
	// if we are here, it means that server is shutting down
	// close socket and unintialize WinSock2 library
	iResult = closesocket(serverSocket);
	if (iResult == SOCKET_ERROR)
	{
		printf("closesocket failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printf("WSACleanup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	printf("Server successfully shut down.\n");


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
