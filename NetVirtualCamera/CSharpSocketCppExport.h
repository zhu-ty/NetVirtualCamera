#pragma once


#ifdef SK_EXPORTS
#define SK_API __declspec(dllexport)
#else
#define SK_API __declspec(dllimport)
#endif

#include<iostream>
using namespace std;

// This class is exported from the CSharpSocketCppExport.dll
class SK_API SKSocket {
private:
	int usage_i = 0;
public:
	SKSocket(void);
	bool state();
	bool abort();
	bool connectToHost(unsigned char * ip, int ip_len, int port, int exceed_time = 10000);
	bool write(unsigned char * data, int data_len);
	bool waitForReadyRead(int exceed_time = 10000);
	int read(unsigned char * data, int len);
};