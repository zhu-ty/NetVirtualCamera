#include "AsioSocket.h"
#ifndef CSharp
#include <thread>



struct Socketdata {
	
};

AsioSocket::AsioSocket() {}
AsioSocket::~AsioSocket() {}


bool AsioSocket::connectToHost(unsigned char *ip, int ip_len, int port,
	int exceed_time) {
	if (status == true) {
		std::cout << "Asio socket is already initialized. " << std::endl;
		return false;
	}
	ip[ip_len] = 0;
	asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(std::string((const char*)ip)), 
		port);
	socketPtr = std::make_shared<asio::ip::tcp::socket>(ios);
	size_t largestBufSize = 4096 * 4096 * 8;
	asio::socket_base::receive_buffer_size opt1(largestBufSize);
	asio::socket_base::send_buffer_size opt2(largestBufSize);
	socketPtr->connect(endpoint);
	socketPtr->set_option(opt1);
	socketPtr->set_option(opt2);
	status = true;
	return true;
}

bool AsioSocket::write(unsigned char *data, int data_len) {
	// Send an initial buffer
	asio::error_code error;
	std::vector<char> data_vec(data, data + data_len);
	socketPtr->write_some(asio::buffer(data_vec), error);
	if (error) {
		std::cerr << error.message() << std::endl;
	}
	return true;
}

bool AsioSocket::state() {
	return status;
}

bool AsioSocket::waitForReadyRead(int exceed_time) {
	size_t argp = 0;
	asio::error_code error;
	for (int i = 0; i < exceed_time; i += 5) {
		argp = socketPtr->available(error);
		if (error) {
			std::cerr << error.message() << std::endl;
		}
		if (argp > 0) {
			//printf("Available data buffer size %ld\n", argp);
			return true;
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	printf("Over time!\n");
	return false;
}

int AsioSocket::read(unsigned char* data, int len) {
	// Receive data until the server closes the connection
	asio::error_code error;
	std::vector<char> readbuf(len);
	int len_out = socketPtr->read_some(asio::buffer(readbuf), error);
	if (error) {
		std::cerr << error.message() << std::endl;
	}
	memcpy(data, asio::buffer_cast<unsigned char*>(asio::buffer(readbuf)), len);
	return len_out;
}

bool AsioSocket::abort() {
	socketPtr->close();
	status = false;
	return true;
}
#endif