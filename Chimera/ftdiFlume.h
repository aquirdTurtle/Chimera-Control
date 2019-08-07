// created by Mark O. Brown
#pragma once
#include "ftd2xx.h"
/*
 * An interface for using the ft api for programming ftdi chips. ftdi stands for future technology devices 
 * international, and is the name of the manufacturer for one of the chips on the dio system. I believe that this chip
 * is primarily just facilitates the usb interface for programming the dio.
 */
class ftdiFlume
{
	public:
		ftdiFlume( bool safemode_option );
		DWORD write( std::vector<unsigned char> dataBuffer, DWORD amountToWrite=NULL );
		UINT getNumDevices( );
		std::vector<UINT8> read ( DWORD readSize );
		void close( );
		DWORD trigger();
		std::string getErrorText( int errCode );
		std::string getDeviceInfoList ( );
		bool getSafemodeSetting();
		void open(const char devSerial[]);
		void setUsbParams();
	private:
		const bool safemode;
		FT_HANDLE ftAsyncHandle;

};


