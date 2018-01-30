#include <winsock2.h>
#include <ws2tcpip.h>
#include "Socket.h"
#include <thread>
#include <chrono>
#include <iostream>

struct Socketdata {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	addrinfo *result = NULL;
	addrinfo *ptr = NULL;
	addrinfo hints;
};

Socket::Socket() {
	socketdata = new Socketdata;
	
	status = false;
}

Socket::~Socket() {
	delete socketdata;
}

bool Socket::connectToHost(unsigned char *ip, int ip_len, int port,
	int exceed_time) {
	int iResult;
	// Initialize Winsock

	iResult = WSAStartup(MAKEWORD(2, 2), &(((Socketdata*)socketdata)->wsaData));
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		exit(-1);
	}
	memset(&(((Socketdata*)socketdata)->hints), 0, sizeof(addrinfo));
	((Socketdata*)socketdata)->hints.ai_family = AF_UNSPEC;
	((Socketdata*)socketdata)->hints.ai_socktype = SOCK_STREAM;
	((Socketdata*)socketdata)->hints.ai_protocol = IPPROTO_TCP;
	// Connect to server.
	char port_str[10];
	ip[ip_len] = 0;
	itoa(port, port_str, 10);
	iResult = getaddrinfo((PCSTR)ip, (PCSTR)port_str, &(((Socketdata*)socketdata)->hints),
		&(((Socketdata*)socketdata)->result));
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return false;
	}
	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	((Socketdata*)socketdata)->ptr = ((Socketdata*)socketdata)->result;
	// Create a SOCKET for connecting to server
	((Socketdata*)socketdata)->ConnectSocket = socket(((Socketdata*)socketdata)->ptr->ai_family, 
		((Socketdata*)socketdata)->ptr->ai_socktype,
		((Socketdata*)socketdata)->ptr->ai_protocol);
	if (((Socketdata*)socketdata)->ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(((Socketdata*)socketdata)->result);
		WSACleanup();
		return false;
	}
	// Connect to server.
	iResult = connect(((Socketdata*)socketdata)->ConnectSocket, ((Socketdata*)socketdata)->ptr->ai_addr, (int)((Socketdata*)socketdata)->ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(((Socketdata*)socketdata)->ConnectSocket);
		((Socketdata*)socketdata)->ConnectSocket = INVALID_SOCKET;
	}
	int a = 4096*4096*8;

	if (setsockopt(((Socketdata*)socketdata)->ConnectSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&a, sizeof(int)) == -1) {
		fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
	}
	if (setsockopt(((Socketdata*)socketdata)->ConnectSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&a, sizeof(int)) == -1) {
		fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
	}

	int nNetTimeout = 3000;//3秒
						   //发送时限
	setsockopt(((Socketdata*)socketdata)->ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)nNetTimeout, sizeof(int));
	//接收时限
	setsockopt(((Socketdata*)socketdata)->ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)nNetTimeout, sizeof(int));

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message
	freeaddrinfo(((Socketdata*)socketdata)->result);
	if (((Socketdata*)socketdata)->ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return false;
	}
	status = true;
	return true;
}

bool Socket::state() {
	return status;
}

bool Socket::write(unsigned char *data, int data_len) {
	int iResult;
	// Send an initial buffer
	iResult = send(((Socketdata*)socketdata)->ConnectSocket, (const char*)data, data_len, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(((Socketdata*)socketdata)->ConnectSocket);
		WSACleanup();
		return false;
	}
	return true;
}

bool Socket::waitForReadyRead(int exceed_time) {
	u_long argp = 0;
	for (int i = 0; i < exceed_time; i += 5) {
		ioctlsocket(((Socketdata*)socketdata)->ConnectSocket, FIONREAD, &argp);
		if (argp > 0)
		{
			std::cout << argp << std::endl;
			return true;
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	printf("Over time!\n");
	return false;
}

int Socket::read(unsigned char* data, int len) {
	// Receive data until the server closes the connection
	int iResult;
	iResult = recv(((Socketdata*)socketdata)->ConnectSocket, (char*)data, len, 0);
	return len;
}

bool Socket::abort() {
	int iResult;
	iResult = shutdown(((Socketdata*)socketdata)->ConnectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(((Socketdata*)socketdata)->ConnectSocket);
		WSACleanup();
		return false;
	}
	// cleanup
	closesocket(((Socketdata*)socketdata)->ConnectSocket);
	WSACleanup();
	return true;
}