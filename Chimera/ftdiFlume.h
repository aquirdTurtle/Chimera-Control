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
		void open( const char devSerial[] );
		void setUsbParams( );
		DWORD ftdiFlume::write( std::vector<unsigned char> dataBuffer, DWORD amountToWrite=NULL );
		UINT getNumDevices( );
		void close( );
		std::string getErrorText( int errCode );
	private:
		const bool safemode;
		FT_HANDLE ftAsyncHandle;
};


