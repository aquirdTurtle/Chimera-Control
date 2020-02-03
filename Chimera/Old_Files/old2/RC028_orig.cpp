#include "stdafx.h"
#include <windows.h>
#include "RC028.h"
#include <iostream>
#include <sstream>

using namespace std;

RC028::RC028() {
	this->connType = NONE;
}

int RC028::setPoint(short number, unsigned int Time, unsigned char P1, unsigned char P2, unsigned char P3, unsigned char P4, unsigned char P5, unsigned char P6, unsigned char P7, unsigned char P8)
{
	RC028_POINT point;
	point.Time = Time;
	point.P1 = P1;
	point.P2 = P2;
	point.P3 = P3;
	point.P4 = P4;
	point.P5 = P5;
	point.P6 = P6;
	point.P7 = P7;
	point.P8 = P8;
	this->mem.Points[number] = point;
	return 0;
}

int RC028::connectasync(const char devSerial[])
{
	FT_STATUS ftStatus;
	DWORD numDevs;
	ftStatus = FT_ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);

	if (numDevs > 0)
	{
		ftStatus = FT_OpenEx((PVOID)devSerial, FT_OPEN_BY_SERIAL_NUMBER, &this->ftHandle);
		if (ftStatus != FT_OK) {
			cout << "Error opening device" << endl;
			return 1;
		}
		ftStatus = FT_SetUSBParameters(this->ftHandle, 65536, 65536);
		if (ftStatus != FT_OK) {
			cout << "Error opening device" << endl;
			return 1;
		}
		cout << "Connected..." << endl;
		this->connType = ASYNC;
		return 0;
	}
	else
	{
		cout << "No devices found." << endl;
		return 1;
	}

	
}

//int RC028::connectRS232(LPCWSTR Port) //Removed this function since it appears to be unused; confirmed demo runs without it. 


int RC028::trigger()
{
	if (this->connType == SERIAL)
	{
		if (this->m_hSerialComm != INVALID_HANDLE_VALUE)
		{
			unsigned char dataBuffer[7];
			unsigned long dwNumberOfBytesSent = 0;

			dataBuffer[0] = 161;
			dataBuffer[1] = 0;
			dataBuffer[2] = 0;
			dataBuffer[3] = 0;
			dataBuffer[4] = 0;
			dataBuffer[5] = 0;
			dataBuffer[6] = 1;

			while (dwNumberOfBytesSent < 7)
			{
				unsigned long dwNumberOfBytesWritten;

				if (WriteFile(m_hSerialComm, &dataBuffer[dwNumberOfBytesSent], 1,
					&dwNumberOfBytesWritten, NULL) != 0)
				{
					if (dwNumberOfBytesWritten > 0)
						++dwNumberOfBytesSent;
					else
					{
						//Handle Error Condition
						std::cout << "ERROR: dwNumberOfBytesWritten " << dwNumberOfBytesWritten << endl;
						return 1;
					}
				}
				else
				{
					//Handle Error Condition
					std::cout << "ERROR: WriteFile" << endl;
					return 1;
				}
			}

			std::cout << "Bytes sent for trigger: " << dwNumberOfBytesSent << endl;
		}
	}
	else if (this->connType == ASYNC)
	{
		FT_STATUS ftStatus;
		DWORD BytesWritten;
		unsigned char dataBuffer[7];
		unsigned long dwNumberOfBytesSent = 0;

		dataBuffer[0] = 161;
		dataBuffer[1] = 0;
		dataBuffer[2] = 0;
		dataBuffer[3] = 0;
		dataBuffer[4] = 0;
		dataBuffer[5] = 0;
		dataBuffer[6] = 1;

		ftStatus = FT_Write(this->ftHandle, dataBuffer, sizeof(dataBuffer), &BytesWritten);
		if (ftStatus == FT_OK) {
			// FT_Write OK
			std::cout << "Bytes sent for trigger: " << dwNumberOfBytesSent << endl;
			return 0;
		}
		else {
			// FT_Write Failed
			cout << "error writing" << endl;
			return 1;
		}
	}

	return 1;
}

int RC028::disconnect()
{
	if (this->connType == SERIAL)
	{
		if (this->m_hSerialComm != INVALID_HANDLE_VALUE)
		{
			CloseHandle(this->m_hSerialComm);
			m_hSerialComm = INVALID_HANDLE_VALUE;
			cout << "Serial connection closed..." << endl;
			this->connType = NONE;
			return 0;
		}
		return 1;
	}
	else if (this->connType == ASYNC)
	{
		FT_STATUS ftStatus;
		ftStatus = FT_Close(this->ftHandle);
		if (ftStatus == FT_OK) {
			// FT_Write OK
			cout << "Async connection closed" << endl;
			return 0;
		}
		else {
			// FT_Write Failed
			cout << "Error closing async connection" << endl;
			return 1;
		}
	}
	else
	{
		cout << "No connection to close..." << endl;
		return 1;
	}
	
}

unsigned long RC028::write()
{
	if (this->connType == SERIAL)
	{
		if (this->m_hSerialComm != INVALID_HANDLE_VALUE)
		{
			unsigned char dataBuffer[BUFFERSIZESER*MSGLENGTH*WRITESPERDATAPT];
			//we have MSG_LENGTH bytes per message
			//how many entries do we need to submit?
			bool proceed = true;
			int count = 0;
			int idx = 0;
			unsigned int totalBytes = 0;

			while (proceed)
			{
				unsigned int bytesToWrite = 0;
				unsigned int number = 0;

				while ((number < BUFFERSIZESER) && proceed)
				{
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH] = WBWRITE;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 1] = ((TIMEOFFS + count) >> 8) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 2] = (TIMEOFFS + count) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 3] = ((this->mem.Points[count].Time) >> 24) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 4] = ((this->mem.Points[count].Time) >> 16) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 5] = ((this->mem.Points[count].Time) >> 8) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 6] = (this->mem.Points[count].Time) & 0xFF;

					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 7] = WBWRITE;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 8] = ((BANKAOFFS + count) >> 8) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 9] = (BANKAOFFS + count) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 10] = this->mem.Points[count].P1;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 11] = this->mem.Points[count].P2;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 12] = this->mem.Points[count].P3;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 13] = this->mem.Points[count].P4;

					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 14] = WBWRITE;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 15] = ((BANKBOFFS + count) >> 8) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 16] = (BANKBOFFS + count) & 0xFF;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 17] = this->mem.Points[count].P5;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 18] = this->mem.Points[count].P6;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 19] = this->mem.Points[count].P7;
					dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 20] = this->mem.Points[count].P8;

					if ((this->mem.Points[count].Time == 0) && (this->mem.Points[count].P1 == 0) && (this->mem.Points[count].P2 == 0) && (this->mem.Points[count].P3 == 0) && (this->mem.Points[count].P4 == 0) && (this->mem.Points[count].P5 == 0) && (this->mem.Points[count].P6 == 0) && (this->mem.Points[count].P7 == 0) && (this->mem.Points[count].P8 == 0))
					{
						proceed = false;
						std::cout << "Termination of sequence found..." << endl;
					}
					if (count == NUMPOINTS)
					{
						std::cout << "RC028.h: Non-Terminated table... Unit will not work right..." << endl;
						proceed = false;
					}

					number++;
					count++;

					bytesToWrite += WRITESPERDATAPT * MSGLENGTH;
				}

				unsigned long dwNumberOfBytesSent = 0;

				while (dwNumberOfBytesSent < bytesToWrite)
				{
					unsigned long dwNumberOfBytesWritten;

					if (WriteFile(m_hSerialComm, &dataBuffer[dwNumberOfBytesSent], 1,
						&dwNumberOfBytesWritten, NULL) != 0)
					{
						if (dwNumberOfBytesWritten > 0)
							++dwNumberOfBytesSent;
						else
						{
							//Handle Error Condition
							std::cout << "ERROR: dwNumberOfBytesWritten " << dwNumberOfBytesWritten << endl;
							return 1;
						}
					}
					else
					{
						//Handle Error Condition
						std::cout << "ERROR: WriteFile" << endl;
						return 1;
					}
				}
				std::cout << "Bytes sent in iteration: " << dwNumberOfBytesSent << endl;
				totalBytes += dwNumberOfBytesSent;

			}
			std::cout << "Total bytes sent: " << totalBytes << endl;
		}
		return 0;
	}
	else if(this->connType == ASYNC)
	{
		FT_STATUS ftStatus;
		DWORD BytesWritten;
		unsigned char dataBuffer[BUFFERSIZEASYNC*MSGLENGTH*WRITESPERDATAPT];
		//we have MSG_LENGTH bytes per message
		//how many entries do we need to submit?
		bool proceed = true;
		int count = 0;
		int idx = 0;
		unsigned int totalBytes = 0;

		while (proceed)
		{
			unsigned int bytesToWrite = 0;
			unsigned int number = 0;

			while ((number < BUFFERSIZEASYNC) && proceed)
			{
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH] = WBWRITE;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 1] = ((TIMEOFFS + count) >> 8) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 2] = (TIMEOFFS + count) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 3] = ((this->mem.Points[count].Time) >> 24) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 4] = ((this->mem.Points[count].Time) >> 16) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 5] = ((this->mem.Points[count].Time) >> 8) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 6] = (this->mem.Points[count].Time) & 0xFF;

				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 7] = WBWRITE;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 8] = ((BANKAOFFS + count) >> 8) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 9] = (BANKAOFFS + count) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 10] = this->mem.Points[count].P1;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 11] = this->mem.Points[count].P2;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 12] = this->mem.Points[count].P3;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 13] = this->mem.Points[count].P4;

				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 14] = WBWRITE;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 15] = ((BANKBOFFS + count) >> 8) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 16] = (BANKBOFFS + count) & 0xFF;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 17] = this->mem.Points[count].P5;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 18] = this->mem.Points[count].P6;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 19] = this->mem.Points[count].P7;
				dataBuffer[WRITESPERDATAPT * number * MSGLENGTH + 20] = this->mem.Points[count].P8;

				if ((this->mem.Points[count].Time == 0) && (this->mem.Points[count].P1 == 0) && (this->mem.Points[count].P2 == 0) && (this->mem.Points[count].P3 == 0) && (this->mem.Points[count].P4 == 0) && (this->mem.Points[count].P5 == 0) && (this->mem.Points[count].P6 == 0) && (this->mem.Points[count].P7 == 0) && (this->mem.Points[count].P8 == 0))
				{
					proceed = false;
					std::cout << "Termination of sequence found..." << endl;
				}
				if (count == NUMPOINTS)
				{
					std::cout << "RC028.h: Non-Terminated table... Unit will not work right..." << endl;
					proceed = false;
				}

				number++;
				count++;

				bytesToWrite += WRITESPERDATAPT * MSGLENGTH;
			}

			ftStatus = FT_Write(this->ftHandle, dataBuffer, bytesToWrite, &BytesWritten);
			if (ftStatus == FT_OK) {
				// FT_Write OK
				cout << "data written" << endl;
			}
			else {
				// FT_Write Failed
				cout << "error writing" << endl;
			}

			std::cout << "Bytes sent in iteration: " << BytesWritten << endl;
			totalBytes += BytesWritten;

		}
		std::cout << "Total bytes sent: " << totalBytes << endl;
		return 0;
	}
	else
	{
		cout << "No connection exists..." << endl;
		return 1;
	}
	
	
}
