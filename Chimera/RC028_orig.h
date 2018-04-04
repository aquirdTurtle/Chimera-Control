#pragma once
#include <windows.h>
#include "ftd2xx.h"

#define NUMPOINTS         2048
#define MSGLENGTH         7
#define TIMEOFFS          0x0800
#define BANKAOFFS         0x1000
#define BANKBOFFS         0x1800
#define WBWRITE           (unsigned char)161
#define WRITESPERDATAPT   3
#define BUFFERSIZESER     100
#define BUFFERSIZEASYNC   2048

#define NONE            0
#define SERIAL          1
#define ASYNC           2

struct RC028_POINT {
	unsigned int Time;
	unsigned char P1;
	unsigned char P2;
	unsigned char P3;
	unsigned char P4;
	unsigned char P5;
	unsigned char P6;
	unsigned char P7;
	unsigned char P8;
};

class RC028
{
private:
	HANDLE m_hSerialComm;
	FT_HANDLE ftHandle;
	int connType;
public:
	//int connectRS232(LPCWSTR Port);
	int disconnect();
	int connectasync(const char devSerial[]);
	unsigned long write();
	int trigger();
	int setPoint(short number, unsigned int Time, unsigned char P1, unsigned char P2, unsigned char P3, unsigned char P4, unsigned char P5, unsigned char P6, unsigned char P7, unsigned char P8);
	struct mem {
		RC028_POINT Points[NUMPOINTS];
	};
	mem mem;
	RC028();
};



//RC028_POINT genPoint(short Time, unsigned char P1, unsigned char P2, unsigned char P3, unsigned char P4, unsigned char P5, unsigned char P6, unsigned char P7, unsigned char P8);