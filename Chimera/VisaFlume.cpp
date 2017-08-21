#include "stdafx.h"
#include "VisaFlume.h"

VisaFlume::VisaFlume() : deviceSafemode(false){}

/// 
void VisaFlume::init( std::string address )
{
	usbAddress = address;
	deviceSafemode = false;
}
/// 

void VisaFlume::write( std::string message )
{
	// not sure what this is for.
	ULONG actual;
	if (!deviceSafemode)
	{
		errCheck( viWrite( instrument, (unsigned char*)cstr( message ), (ViUInt32)message.size(), &actual ) );
	}
}


void VisaFlume::close()
{
	if (!deviceSafemode)
	{
		errCheck( viClose( defaultResourceManager ) );
	}
}

// open the default resource manager resource. From ni.com:
// " The viOpenDefaultRM() function must be called before any VISA operations can be invoked.The first call to this function 
// initializes the VISA system, including the Default Resource Manager resource, and also returns a session to that resource. 
// Subsequent calls to this function return unique sessions to the same Default Resource Manager resource.
// When a Resource Manager session is passed to viClose(), not only is that session closed, but also all find lists and device 
// sessions( which that Resource Manager session was used to create ) are closed.
void VisaFlume::open()
{
	if (!deviceSafemode)
	{
		errCheck( viOpenDefaultRM( &defaultResourceManager ) );
		errCheck( viOpen( defaultResourceManager, (char *)cstr( usbAddress ), VI_NULL, VI_NULL, &instrument ) );
	}
}


void VisaFlume::setAttribute( ULONG attributeName, ULONG value )
{
	if (!deviceSafemode)
	{
		errCheck( viSetAttribute( instrument, attributeName, value ) );
	}
}


std::string VisaFlume::identityQuery()
{
	char buf[256] = { 0 };
	if (!deviceSafemode)
	{
		open();
		viQueryf( instrument, (ViString)"*IDN?\n", "%t", buf );
		close();
	}
	else
	{
		return "Device is in Safemode. Change this in the constants.h file.\r\n";
	}
	return buf;
}


void VisaFlume::errQuery( std::string& errMsg, long& errCode )
{
	char buf[256] = { 0 };
	if (!deviceSafemode)
	{
		viQueryf( instrument, (ViString)"SYST:ERR?\n", "%ld,%t", &errCode, buf );
	}
	else
	{
		return;
	}
	errMsg = str( buf );
}

/*
* This function checks if the agilent throws an error or if there is an error communicating with the agilent.
*/
void VisaFlume::errCheck( long status )
{
	long errorCode = 0;
	// Check comm status
	if (status < 0)
	{
		// Error detected.
		thrower( "ERROR: VisaFlume Communication error! Error Code: " + str( status ) + "\r\n" );
	}
	// Query the agilent for errors.
	std::string errMessage;
	errQuery( errMessage, errorCode );
	if (errorCode != 0)
	{
		// Agilent error
		thrower( "ERROR: agilent returned error message: " + str( errorCode ) + ":" + errMessage );
	}
}



void VisaFlume::printf( std::string msg )
{
	if (!deviceSafemode)
	{
		errCheck( viPrintf( instrument, (ViString)cstr( msg ) ) );
	}
}

