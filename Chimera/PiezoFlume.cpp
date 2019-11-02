#include "stdafx.h"
#include <boost/algorithm/string/replace.hpp>
#include "PiezoFlume.h"

PiezoFlume::PiezoFlume ( bool sMode, std::string sn ) : safemode(sMode), comPortNumber(sn.begin(), sn.end())
{
	comPortNumber.push_back ( '\0' );
	/*
	HINSTANCE hdll = LoadLibrary ( TEXT ( "MDT_COMMAND_LIB_win32.dll" ) );
	if ( hdll == NULL )
	{
		thrower ( "Failed to load thorlabs piezo driver dll! Make sure that MDT_COMMAND_LIB_win32.dll is in the "
				  "application folder!" );
	}

	List_raw = (ftype_List) GetProcAddress ( hdll, "List" );
	Open_raw = (ftype_Open) GetProcAddress ( hdll, "Open" );
	Close_raw = (ftype_Close) GetProcAddress ( hdll, "Close" );
	GetId_raw = (ftype_GetId) GetProcAddress ( hdll, "GetId" );
	IsOpen_raw = (ftype_IsOpen) GetProcAddress ( hdll, "IsOpen" );

	GetFriendlyName_raw = (ftype_GetFriendlyName) GetProcAddress ( hdll, "GetFriendlyName" );
	GetSerialNumber_raw = (ftype_GetSerialNumber) GetProcAddress ( hdll, "GetSerialNumber" );

	SetXAxisVoltage_raw = (ftype_SetXAxisVoltage) GetProcAddress ( hdll, "SetXAxisVoltage" );
	SetXAxisMinVoltage_raw = (ftype_SetXAxisMinVoltage) GetProcAddress ( hdll, "SetXAxisMinVoltage" );
	SetXAxisMaxVoltage_raw = (ftype_SetXAxisMaxVoltage) GetProcAddress ( hdll, "SetXAxisMaxVoltage" );
	GetXAxisVoltage_raw = (ftype_GetXAxisVoltage) GetProcAddress ( hdll, "GetXAxisVoltage" );
	GetXAxisMinVoltage_raw = (ftype_GetXAxisMinVoltage) GetProcAddress ( hdll, "GetXAxisMinVoltage" );
	GetXAxisMaxVoltage_raw = (ftype_GetXAxisMaxVoltage) GetProcAddress ( hdll, "GetXAxisMaxVoltage" );

	SetYAxisVoltage_raw = (ftype_SetYAxisVoltage) GetProcAddress ( hdll, "SetYAxisVoltage" );
	SetYAxisMinVoltage_raw = (ftype_SetYAxisMinVoltage) GetProcAddress ( hdll, "SetYAxisMinVoltage" );
	SetYAxisMaxVoltage_raw = (ftype_SetYAxisMaxVoltage) GetProcAddress ( hdll, "SetYAxisMaxVoltage" );
	GetYAxisVoltage_raw = (ftype_GetYAxisVoltage) GetProcAddress ( hdll, "GetYAxisVoltage" );
	GetYAxisMinVoltage_raw = (ftype_GetYAxisMinVoltage) GetProcAddress ( hdll, "GetYAxisMinVoltage" );
	GetYAxisMaxVoltage_raw = (ftype_GetYAxisMaxVoltage) GetProcAddress ( hdll, "GetYAxisMaxVoltage" );

	SetZAxisVoltage_raw = (ftype_SetZAxisVoltage) GetProcAddress ( hdll, "SetZAxisVoltage" );
	SetZAxisMinVoltage_raw = (ftype_SetZAxisMinVoltage) GetProcAddress ( hdll, "SetZAxisMinVoltage" );
	SetZAxisMaxVoltage_raw = (ftype_SetZAxisMaxVoltage) GetProcAddress ( hdll, "SetZAxisMaxVoltage" );
	GetZAxisVoltage_raw = (ftype_GetZAxisVoltage) GetProcAddress ( hdll, "GetZAxisVoltage" );
	GetZAxisMinVoltage_raw = (ftype_GetZAxisMinVoltage) GetProcAddress ( hdll, "GetZAxisMinVoltage" );
	GetZAxisMaxVoltage_raw = (ftype_GetZAxisMaxVoltage) GetProcAddress ( hdll, "GetZAxisMaxVoltage" );

	SetXYZAxisVoltage_raw = (ftype_SetXYZAxisVoltage) GetProcAddress ( hdll, "SetXYZAxisVoltage" );
	GetXYZAxisVoltage_raw = (ftype_GetXYZAxisVoltage) GetProcAddress ( hdll, "GetXYZAxisVoltage" );
	SetMasterScanEnable_raw = (ftype_SetMasterScanEnable) GetProcAddress ( hdll, "SetMasterScanEnable" );
	GetMasterScanEnable_raw = (ftype_GetMasterScanEnable) GetProcAddress ( hdll, "GetMasterScanEnable" );

	SetMasterScanVoltage_raw = (ftype_SetMasterScanVoltage) GetProcAddress ( hdll, "SetMasterScanVoltage" );
	GetMasterScanVoltage_raw = (ftype_GetMasterScanVoltage) GetProcAddress ( hdll, "GetMasterScanVoltage" );


	GetLimitVoltage_raw = (ftype_GetLimitVoltage) GetProcAddress ( hdll, "GetLimitVoltage" );
	SetAllVoltage_raw = (ftype_SetAllVoltage) GetProcAddress ( hdll, "SetAllVoltage" );

	if ( !(List_raw != NULL && Open_raw != NULL && Close_raw != NULL && GetId_raw != NULL && GetFriendlyName_raw != NULL &&
		 SetXAxisVoltage_raw != NULL && SetXAxisMinVoltage_raw != NULL && SetXAxisMaxVoltage_raw != NULL &&
		 GetXAxisVoltage_raw != NULL && GetXAxisMinVoltage_raw != NULL  && GetXAxisMaxVoltage_raw != NULL &&
		 SetYAxisVoltage_raw != NULL && SetYAxisMinVoltage_raw != NULL && SetYAxisMaxVoltage_raw != NULL &&
		 GetYAxisVoltage_raw != NULL && GetYAxisMinVoltage_raw != NULL && GetYAxisMaxVoltage_raw != NULL &&
		 SetZAxisVoltage_raw != NULL && SetZAxisMinVoltage_raw != NULL && SetZAxisMaxVoltage_raw != NULL &&
		 GetZAxisVoltage_raw != NULL && GetZAxisMinVoltage_raw != NULL && GetZAxisMaxVoltage_raw != NULL &&
		 SetAllVoltage_raw != NULL && SetXYZAxisVoltage_raw != NULL && GetXYZAxisVoltage_raw != NULL &&
		 SetMasterScanEnable_raw != NULL && GetMasterScanEnable_raw != NULL && GetLimitVoltage_raw != NULL && 
		 IsOpen_raw != NULL && GetSerialNumber_raw != NULL))
	{
		thrower ( "ERROR: Opened thorlabs piezo driver dll but failed to load one of the procedures?!?" );
	}
	*/
}


std::string PiezoFlume::list ( )
{	
	if ( !safemode )
	{		
		std::vector<char> sNum ( bufferSize );
		auto res = List_raw ( &sNum[ 0 ] );
		if ( res < 0 )
		{
			thrower ( "ERROR: raw \"List\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( res ) + "." );
		}
		for ( auto character : sNum )
		{
			if ( character == '\0' )
			{

			}
		}
		std::string tmpStr = std::string ( sNum.begin ( ), sNum.end ( ) ).c_str();
		std::string listStr = str ( res ) + " Device(s): " + tmpStr;
		return listStr;
	}
	else
	{
		return "SAFEMODE - NO DEVICES";
	}
}


void PiezoFlume::open (  )
{
	if ( !safemode )
	{
		// nBaud and timeout taken from example thorlabs program. Not sure if nBaud is changable without causing probs.
		if ( isOpen ( ) )
		{
			close ( );
		}
		deviceHandle = Open_raw ( &comPortNumber[ 0 ], 115200, 10 );
		if ( deviceHandle < 0 )
		{
			thrower ( "ERROR: raw \"Open\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( deviceHandle ) + "." );
		}
	}
}

bool PiezoFlume::isOpen ( )
{
	if ( !safemode )
	{
		return IsOpen_raw ( &comPortNumber[0] );
	}
	else
	{
		return true;
	}
}


void PiezoFlume::close ( )
{
	if ( !safemode )
	{
		auto res = Close_raw ( deviceHandle );
		if ( res < 0 )
		{
			thrower ( "ERROR: Failed to close!" );
		}
	}
}


double PiezoFlume::getXAxisVoltage ( )
{
	if ( !safemode )
	{
		double val;
		auto res = GetXAxisVoltage_raw ( deviceHandle, val );
		if ( res < 0 )
		{
			thrower ( "GetXAxisVoltage Failed! Error was: " + str ( res ) + "." );
		}
		return val;
	}
	else
	{
		return 0;
	}
}

double PiezoFlume::getYAxisVoltage ( )
{
	if ( !safemode )
	{
		double val;
		auto res = GetYAxisVoltage_raw ( deviceHandle, val );
		if ( res < 0 )
		{
			thrower ( "GetYAxisVoltage Failed! Error was: " + str ( res ) + "." );
		}
		return val;
	}
	else
	{
		return 0;
	}
}

double PiezoFlume::getZAxisVoltage ( )
{
	if ( !safemode )
	{
		double val;
		auto res = GetZAxisVoltage_raw ( deviceHandle, val );
		if ( res < 0 )
		{
			thrower ( "GetZAxisVoltage Failed! Error was: " + str ( res ) + "." );
		}
		return val;
	}
	else
	{
		return 0;
	}
}


std::string PiezoFlume::getDeviceInfo ( )
{
	std::string devInfo = getID ( ).c_str();
	boost::replace_all ( devInfo, "\r\r\r", "\r" );
	boost::replace_all ( devInfo, "\r", "\n\t\t" );
	return devInfo;
}


std::string PiezoFlume::getID ( )
{
	if ( !safemode )
	{
		if ( !isOpen ( ) )
		{
			thrower ( "Piezo Driver Com Port was not open!" );
		}
		std::vector<char> txt ( bufferSize, '\0' );
		auto res = GetId_raw ( deviceHandle, &txt[0] );
		if ( res < 0 )
		{
			thrower ( "ERROR: raw \"GetId\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( res ) + "." );
		}
		return std::string(txt.begin(), txt.end());
	}
	else
	{
		return "SAFEMODE";
	}
}

double PiezoFlume::getLimitVoltage ( )
{
	if ( !safemode )
	{
		if ( !isOpen ( ) )
		{
			thrower ( "Piezo Driver Com Port was not open!" );
		}
		double voltLimit;
		auto res = GetLimitVoltage_raw ( deviceHandle, voltLimit );
		if ( res < 0 )
		{
			thrower ( "ERROR: raw \"GetLimitVoltage\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( res ) + "." );
		}
		return voltLimit;
	}
	else
	{
		return 0;
	}
}

std::string PiezoFlume::getSerialNumber ( )
{
	if ( !safemode )
	{
		if ( !isOpen ( ) )
		{
			thrower ( "Piezo Driver Com Port was not open!" );
		}
		std::vector<char> txt ( bufferSize, '\0' );
		auto res = GetSerialNumber_raw ( deviceHandle, &txt[ 0 ] );
		if ( res < 0 )
		{
			thrower ( "ERROR: raw \"GetSerialNumber\" function from the thorlabs piezo driver dll returned an error! "
					  "Error code: " + str ( res ) + "." );
		}
		return std::string ( txt.begin ( ), txt.end ( ) );
	}
	else
	{
		return "SAFEMODE";
	}
}



void PiezoFlume::setXAxisVoltage ( double val )
{
	if ( !safemode )
	{
		auto res = SetXAxisVoltage_raw ( deviceHandle, val );
		if ( res != 0 )
		{
			thrower ( "Piezo Controller Set X Axis Voltage Failed! Error code was: " + str ( res ) + "." );
		}
	}
}

void PiezoFlume::setYAxisVoltage ( double val )
{
	if ( !safemode )
	{
		auto res = SetYAxisVoltage_raw ( deviceHandle, val );
		if ( res != 0 )
		{
			thrower ( "Piezo Controller Set X Axis Voltage Failed! Error code was: " + str ( res ) + "." );
		}
	}
}

void PiezoFlume::setZAxisVoltage ( double val )
{
	if ( !safemode )
	{
		auto res = SetZAxisVoltage_raw ( deviceHandle, val );
		if ( res != 0 )
		{
			thrower ( "Piezo Controller Set X Axis Voltage Failed! Error code was: " + str ( res ) + "." );
		}
	}
}
