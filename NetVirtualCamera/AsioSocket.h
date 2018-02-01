#ifndef __GIGA_RENDER_ASIO_SOCKET_H__
#define __GIGA_RENDER_ASIO_SOCKET_H__

#define _WIN32_WINNT 0x0A00
#include <asio.hpp>  
#include <iostream>  
#include <functional>  

class AsioSocket {
private:
	asio::io_service ios;
	std::shared_ptr<asio::ip::tcp::socket> socketPtr;
	std::vector<char> readbuf;
	bool status;
public:

private:

public:
	AsioSocket();
	~AsioSocket();

	bool state();
	bool abort();
	bool connectToHost(unsigned char *ip, int ip_len, int port, int exceed_time = 10000);
	bool write(unsigned char *data, int data_len);
	bool waitForReadyRead(int exceed_time = 10000);
	int read(unsigned char* data, int len);
};

#endif