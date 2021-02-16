// created by Mark O. Brown
#include "stdafx.h"
#include "ftdiFlume.h"
#include <fstream>

ftdiFlume::ftdiFlume( bool safemode_option ) : safemode(safemode_option )
{}

unsigned ftdiFlume::getNumDevices( )
{

	unsigned long numDevs=1;
	if ( !safemode )
	{
//#ifdef _WIN64
		numDevs = 0;
		FT_STATUS ftStatus;
		ftStatus = FT_ListDevices( &numDevs, nullptr, FT_LIST_NUMBER_ONLY );
		if ( ftStatus != FT_OK )
		{
			thrower ( "ERROR: Error listing devices ftdi using FT_ListDevices! Status was \"" + getErrorText( ftStatus ) 
					 + "\"" );
		}
//#endif
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
	return "ERROR CODE NOT RECOGNIZED!";
}

void ftdiFlume::open( const char devSerial[] )
{
	if ( !safemode )
	{
//#ifdef _WIN64
		FT_STATUS ftStatus = FT_OpenEx( (PVOID)devSerial, FT_OPEN_BY_SERIAL_NUMBER, &ftAsyncHandle );
		if ( ftStatus != FT_OK )
		{
			thrower ( "Error opening device! Status was \"" + getErrorText(ftStatus) + "\"" );
		}
//#endif
	}
}


std::string ftdiFlume::getDeviceInfoList ( )
{
	std::string msg = "";
	if ( !safemode )
	{
		FT_STATUS ftStatus;
		unsigned long numDevs;
		// create the device information list
		ftStatus = FT_CreateDeviceInfoList ( &numDevs );
		if ( ftStatus != FT_OK )
		{
			thrower ( "Error creating device info list! " + getErrorText ( ftStatus ) );
		}
		if ( numDevs > 0)
		{
			std::vector<FT_DEVICE_LIST_INFO_NODE> devInfo ( numDevs );
			ftStatus = FT_GetDeviceInfoList ( &devInfo[ 0 ], &numDevs );
			if ( ftStatus != FT_OK )
			{
				thrower ( "Error during FT_GetDeviceInfoList! " + getErrorText(ftStatus)  );
			}
			for ( auto deviceInc : range(numDevs) )
			{
				auto dev = devInfo[ deviceInc ];
				msg += "\tDev " + str(deviceInc) + ":\n";
				msg += "\t\tFlags = " + str(dev.Flags) + "\n";
				msg += "\t\tType = " + str ( dev.Type ) + "\n";
				msg += "\t\tID = " + str ( dev.ID ) + "\n";
				msg += "\t\tLocId = " + str ( dev.LocId ) + "\n";
				msg += "\t\tSerial Number = " + str ( dev.SerialNumber ) + "\n";
				msg += "\t\tDescription = " + str ( dev.Description ) + "\n";
				msg += "\t\tftHandle = " + str ( dev.ftHandle ) + "\n\n";
				msg += "\t\tftHandle = " + str ( dev.ftHandle ) + "\n";
			}
		}
		else
		{
			msg += "No Devices found.";
		}
	}
	return msg;
}


void ftdiFlume::setUsbParams( )
{
	if ( !safemode )
	{
//#ifdef _WIN64
		FT_STATUS ftStatus = FT_SetUSBParameters( ftAsyncHandle, 65536, 65536 );
		if ( ftStatus != FT_OK )
		{
			thrower ( "Error Setting usb parameters. Status was \"" + getErrorText( ftStatus ) + "\"" );
		}
//#endif
	}
}

unsigned long ftdiFlume::trigger() {
	
		FT_STATUS ftStatus;
		std::vector<unsigned char> dataBuffer(7);
		unsigned long dwNumberOfBytesSent = 0;
		unsigned long BytesWritten = dataBuffer.size();
		
		dataBuffer[0] = 161;
		dataBuffer[1] = 0;
		dataBuffer[2] = 0;
		dataBuffer[3] = 0;
		dataBuffer[4] = 0;
		dataBuffer[5] = 0;
		dataBuffer[6] = 1;
		write ( dataBuffer, 7 );
		/*
		ftStatus = FT_Write(ftAsyncHandle, dataBuffer.data(), dataBuffer.size(), &BytesWritten);
		if (ftStatus != FT_OK)
		{
			thrower("error writing; FT_Write failed! Status was \"" + getErrorText(ftStatus) + "\"");
		}
		*/
		return BytesWritten;
}

unsigned long ftdiFlume::write( std::vector<unsigned char> dataBuffer, unsigned long amountToWrite )
{
	unsigned long BytesWritten=dataBuffer.size();

	if ( !safemode )
	{
//#ifdef _WIN64
		FT_STATUS ftStatus;
		if ( amountToWrite == 0 )
		{
			amountToWrite = sizeof( dataBuffer );
		}

		
		ftStatus = FT_Write( ftAsyncHandle, dataBuffer.data(), amountToWrite, &BytesWritten );

		if ( ftStatus != FT_OK )
		{
			thrower ( "error writing; FT_Write failed! Status was \"" + getErrorText( ftStatus ) + "\"" );
		}
//#endif
	}
	return BytesWritten;
}

std::vector<UINT8> ftdiFlume::read ( unsigned long readSize )
{
	std::vector<UINT8> readData(readSize);
	unsigned long amountRead;
	if ( !safemode )
	{
		auto status = FT_Read ( ftAsyncHandle, &readData[ 0 ], readSize, &amountRead );
		if ( status != FT_OK )
		{
			thrower ( "Error Reading! Status was " + getErrorText ( status ) );
		}
	}
	return readData;
}
void ftdiFlume::close( )
{
	if ( !safemode )
	{
//#ifdef _WIN64
		FT_STATUS ftStatus = FT_Close( ftAsyncHandle );
		if ( ftStatus != FT_OK )
		{
			thrower ( "Error closing async connection; FT_Write failed?  Status was \"" + getErrorText( ftStatus ) + "\"" );
		}
//#endif
	}
}
bool ftdiFlume::getSafemodeSetting() {
	return safemode;
}
