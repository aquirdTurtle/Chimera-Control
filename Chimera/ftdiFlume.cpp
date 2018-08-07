#include "stdafx.h"
#include "ftdiFlume.h"

ftdiFlume::ftdiFlume( bool safemode_option ) : safemode(safemode_option )
{}


UINT ftdiFlume::getNumDevices( )
{
	FT_STATUS ftStatus;
	DWORD numDevs=1;
	if ( !safemode )
	{
		numDevs = 0;
#ifdef _WIN64
		ftStatus = FT_ListDevices( &numDevs, NULL, FT_LIST_NUMBER_ONLY );
		if ( ftStatus != FT_OK )
		{
			thrower( "ERROR: Error listing devices ftdi using FT_ListDevices! Status was \"" + getErrorText( ftStatus ) 
					 + "\"" );
		}
#endif
	}
	return numDevs;
}



std::string ftdiFlume::getErrorText( int errCode )
{
	switch ( errCode )
	{
	case 0: return "FT_OK";
	case 1: return "FT_INVALID_HANDLE";
	case 2: return "FT_DEVICE_NOT_FOUND";
	case 3: return "FT_DEVICE_NOT_OPENED";
	case 4: return "FT_IO_ERROR";
	case 5: return "FT_INSUFFICIENT_RESOURCES";
	case 6: return "FT_INVALID_PARAMETER";
	case 7: return "FT_INVALID_BAUD_RATE";
	case 8: return "FT_DEVICE_NOT_OPENED_FOR_ERASE";
	case 9: return "FT_DEVICE_NOT_OPENED_FOR_WRITE";
	case 10: return "FT_FAILED_TO_WRITE_DEVICE";
	case 11: return "FT_EEPROM_READ_FAILED";
	case 12: return "FT_EEPROM_WRITE_FAILED";
	case 13: return "FT_EEPROM_ERASE_FAILED";
	case 14: return "FT_EEPROM_NOT_PRESENT";
	case 15: return "FT_EEPROM_NOT_PROGRAMMED";
	case 16: return "FT_INVALID_ARGS";
	case 17: return "FT_NOT_SUPPORTED";
	case 18: return "FT_OTHER_ERROR";
	case 19: return "FT_DEVICE_LIST_NOT_READY";
	}
}


void ftdiFlume::open( const char devSerial[] )
{
	if ( !safemode )
	{
#ifdef _WIN64
		FT_STATUS ftStatus = FT_OpenEx( (PVOID)devSerial, FT_OPEN_BY_SERIAL_NUMBER, &ftAsyncHandle );
		if ( ftStatus != FT_OK )
		{
			thrower( "Error opening device! Status was \"" + getErrorText(ftStatus) + "\"" );
		}
#endif
	}
}


void ftdiFlume::setUsbParams( )
{
	if ( !safemode )
	{
#ifdef _WIN64
		FT_STATUS ftStatus = FT_SetUSBParameters( ftAsyncHandle, 65536, 65536 );
		if ( ftStatus != FT_OK )
		{
			thrower( "Error Setting usb parameters. Status was \"" + getErrorText( ftStatus ) + "\"" );
		}
#endif
	}
}


DWORD ftdiFlume::write( std::vector<unsigned char> dataBuffer, DWORD amountToWrite )
{
	FT_STATUS ftStatus;
	DWORD BytesWritten=dataBuffer.size();
	if ( !safemode )
	{
#ifdef _WIN64
		if ( amountToWrite == NULL )
		{
			amountToWrite = sizeof( dataBuffer );
		}
		ftStatus = FT_Write( ftAsyncHandle, dataBuffer.data(), amountToWrite, &BytesWritten );
		if ( ftStatus != FT_OK )
		{
			thrower( "error writing; FT_Write failed! Status was \"" + getErrorText( ftStatus ) + "\"" );
		}
#endif
	}
	return BytesWritten;
}


void ftdiFlume::close( )
{
	if ( !safemode )
	{
#ifdef _WIN64
		FT_STATUS ftStatus = FT_Close( ftAsyncHandle );
		if ( ftStatus != FT_OK )
		{
			thrower( "Error closing async connection; FT_Write failed?  Status was \"" + getErrorText( ftStatus ) + "\"" );
		}
#endif
	}
}