// created by Mark O. Brown
#include "stdafx.h"
#include "WinSerialFlume.h"

WinSerialFlume::WinSerialFlume( bool safemode_option, std::string portAddress ) : safemode(safemode_option )
{
	open ( portAddress );
}

void WinSerialFlume::open ( std::string fileAddr )
{
	if ( !safemode )
	{
		serialPortHandle = CreateFile ( fileAddr.c_str ( ), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 
										NULL );
		if ( serialPortHandle == INVALID_HANDLE_VALUE )
		{
			thrower ( "Failed to initialize open serial port!" );
		}
		DCB dcbSettings = { 0 };
		dcbSettings.DCBlength = sizeof ( dcbSettings );
		// currently configured for piezo controller, since this is the only raw serial communication I'm doing at the 
		// moment. may need to make these constructor input parameters if these change for new devices.
		dcbSettings.BaudRate = CBR_115200;
		dcbSettings.ByteSize = 8;
		dcbSettings.StopBits = ONESTOPBIT;
		dcbSettings.Parity = NOPARITY;
		auto res = SetCommState ( serialPortHandle, &dcbSettings );
		if ( res == 0 )
		{
			thrower ( "Failed to initialize port: Failed to set comm state!" );
		}
		COMMTIMEOUTS timeouts = { 0 };
		// all times in ms.
		timeouts.ReadIntervalTimeout = 50; 
		timeouts.ReadTotalTimeoutConstant = 50; 
		timeouts.ReadTotalTimeoutMultiplier = 10; 
		timeouts.WriteTotalTimeoutConstant = 50; 
		timeouts.WriteTotalTimeoutMultiplier = 10; 
		res = SetCommTimeouts ( serialPortHandle, &timeouts );
		if ( res == 0 )
		{
			thrower ( "Failed to initialize port: Failed to set comm timeouts!" );
		}
	}
}

void WinSerialFlume::write( std::string msg )
{
	unsigned long numberOfBytesWritten;
	std::vector<unsigned char> buffer ( msg.begin ( ), msg.end ( ) );
	if ( buffer.size ( ) == 0 )
	{
		thrower ( "Trying to write empty serial message?" );
	}
	if ( !safemode )
	{
		if ( serialPortHandle == INVALID_HANDLE_VALUE )
		{
			thrower ( "Tried to write serial without a valid handle! did flume initailize properly?" );
		}
		if ( WriteFile ( serialPortHandle, &buffer[ 0 ], buffer.size(), &numberOfBytesWritten, NULL ) != 0 )
		{
			if ( numberOfBytesWritten != buffer.size() )
			{
				thrower ( "Bad value for numberOfBytesWritten: " + str ( numberOfBytesWritten ) );
			}
		}
		else
		{
			thrower ( "WriteFile failed inside WinSerialFlume! (failed to write command on serial port)" );
		}
	}
}

std::string WinSerialFlume::read ( )
{
	char buf[ 256 ] = { 0 };
	DWORD numBytesRead;
	if ( !safemode )
	{
		/*
		auto err = SetCommMask ( serialPortHandle, EV_RXCHAR );
		if ( err == 0 )
		{
			thrower ( "Failed to set comm mask?!?" );
		}
		DWORD dwEventMask; 
		err = WaitCommEvent ( serialPortHandle, &dwEventMask, NULL );
		if ( err == 0 )
		{
			thrower ( "WaitCommEvent Failed?!" );
		}
		*/
		int err;
		char readChar;
		int totalCount = 0;
		do
		{
			err = ReadFile ( serialPortHandle, &readChar, sizeof ( readChar ), &numBytesRead, 0 );
			if ( err == 0 )
			{
				thrower ( "ReadFile Failed?!" );
			}
			buf[ totalCount++ ] = readChar;
		} while ( numBytesRead > 0 );
	}
	return str ( buf );
}


std::string WinSerialFlume::query ( std::string msg )
{
	write ( msg );
	return read ( );
}


void WinSerialFlume::close( )
{
	if ( serialPortHandle == INVALID_HANDLE_VALUE )
	{
		thrower ( "ERROR: tried to disconnect but handle was invalid?" );
	}
	if ( !safemode )
	{
		if ( !CloseHandle( serialPortHandle ) )
		{
			thrower ( "ERROR: Error closing windows serial handle? (Check error codes...)" );
		}
	}
	serialPortHandle = INVALID_HANDLE_VALUE;
}

