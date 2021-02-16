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
		// THIS CLASS IS NOT COPYABLE.
		ftdiFlume& operator=(const ftdiFlume&) = delete;
		ftdiFlume (const ftdiFlume&) = delete;

		ftdiFlume( bool safemode_option );
		unsigned long write( std::vector<unsigned char> dataBuffer, unsigned long amountToWrite=0 );
		unsigned getNumDevices( );
		std::vector<UINT8> read ( unsigned long readSize );
		void close( );
		unsigned long trigger();
		std::string getErrorText( int errCode );
		std::string getDeviceInfoList ( );
		bool getSafemodeSetting();
		void open(const char devSerial[]);
		void setUsbParams();
	private:
		const bool safemode;
		FT_HANDLE ftAsyncHandle;

};


