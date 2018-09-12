#include "stdafx.h"
#include "WinSerialFlume.h"

WinSerialFlume::WinSerialFlume( bool safemode_option ) : safemode(safemode_option )
{}

unsigned long WinSerialFlume::writeFile( unsigned long index, std::vector<unsigned char> dataBuffer )
{
	unsigned long numberOfBytesWritten;
	if ( m_hSerialComm == INVALID_HANDLE_VALUE )
	{
		thrower ( "ERROR: tried to write serial without a valid handle! did flume initailize properly?" );
	}
	if ( WriteFile( m_hSerialComm, &dataBuffer[index], 1, &numberOfBytesWritten, NULL ) != 0 )
	{
		if ( numberOfBytesWritten == 0)
		{
			thrower ( "ERROR: bad value for numberOfBytesWritten: " + str( numberOfBytesWritten ) );
		}
		return numberOfBytesWritten;
	}
	else
	{
		thrower ( "ERROR: WriteFile" );
	}
}


void WinSerialFlume::close( )
{
	if ( m_hSerialComm == INVALID_HANDLE_VALUE )
	{
		thrower ( "ERROR: tried to disconnect but handle was invalid?" );
	}
	if ( !safemode )
	{
		if ( !CloseHandle( m_hSerialComm ) )
		{
			thrower ( "ERROR: Error closing windows serial handle? (Check error codes...)" );
		}
	}
	m_hSerialComm = INVALID_HANDLE_VALUE;
}

