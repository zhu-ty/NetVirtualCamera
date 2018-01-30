#include <stdio.h>

#ifndef __WIN_CPP_SOCKET_H__
#define __WIN_CPP_SOCKET_H__

#pragma comment(lib, "Ws2_32.lib")

class Socket {
private:
	void* socketdata;
	bool status;
public:

private:

public:
	Socket();
	~Socket();

	bool state();
	bool abort();
	bool connectToHost(unsigned char *ip, int ip_len, int port, int exceed_time = 10000);
	bool write(unsigned char *data, int data_len);
	bool waitForReadyRead(int exceed_time = 10000);
	int read(unsigned char* data, int len);
};

#endif