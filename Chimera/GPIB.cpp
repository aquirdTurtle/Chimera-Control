#include "stdafx.h"
#include "Gpib.h"
#include "ni488.h"
#include <array>
#include "constants.h"

void Gpib::gpibWrite( int deviceID, std::string msg )
{
	int size = msg.size();
	int result = ibwrt(deviceID, (void*)cstr(msg), size);
	if ( result == ERR )
	{
		thrower( "gpib write failed! " + getErrMessage(iberr ));
	}
}


std::string Gpib::gpibRead( int deviceID )
{
	char result[256];
	int code = ibrd( deviceID, result, 256 );
	if ( code == ERR )
	{
		thrower( "gpib read failed!" + getErrMessage( iberr ) );
	}
	return str( result );
}


std::string Gpib::gpibQuery( int deviceID, std::string query )
{
	gpibWrite( deviceID, query );
	return gpibRead( deviceID );
}


Gpib::Gpib()
{
	int hpone, agilentTwo, srsTwo, srsThree, pulseGen, pulseGen2, microHP, powerHP, agilent;
	// I think that a lot of these aren't actually doing anything...
	try
	{
		hpone = gpibIbdev(17);
		agilentTwo = gpibIbdev(12);
		srsTwo = gpibIbdev(6);
		srsThree = gpibIbdev(19);
		pulseGen = gpibIbdev(7);
		pulseGen2 = gpibIbdev(4);
		microHP = gpibIbdev(10);
		powerHP = gpibIbdev(5);
		agilent = gpibIbdev(11);
	}
	catch (Error& err)
	{
		errBox("GPIB Initialization failed!: " + err.whatStr());
	}
}


// send message to address.
void Gpib::gpibSend(int address, std::string message)
{
	if ( !GPIB_SAFEMODE )
	{
		//errBox( "attempting to send message: " + message );
		Send( 0, address, (void*) cstr(message), message.size(), NLend );
		if ( ibsta == ERR )
		{
			thrower( "GPIB ERROR: " + getErrMessage( iberr ) );
		}
		else
		{
			// errBox( "success??" );
		}
	}
}



// gets the device descriptor ud. Input the device address.
int Gpib::gpibIbdev(int pad)
{
	// board address
	int bdindx = 0;   
	// see gpib dos manual for the rest
	int sad = 0; 
	int tmo = 13;
	int eot = 1;
	int eos = 0;
	if (!GPIB_SAFEMODE)
	{
		int id = ibdev(bdindx, pad, sad, tmo, eot, eos);
		if (id == -1)
		{
			thrower("ibdev failed!");
		}
		return id;
	}
	else
	{
		return 0;
	}
}



std::string Gpib::queryIdentity( int deviceAddress )
{
	try
	{
		return gpibQuery( deviceAddress, "*IDN?" );
	}
	catch ( Error& exception )
	{
		if ( exception.whatBare() == "gpib write failed! Code 0: System Error" )
		{
			return "Disconnected...";
		}
		else
		{
			return exception.what();
		}
	}
}


std::string Gpib::getErrMessage( long errCode )
{
	switch ( errCode )
	{
		case EDVR:
			return "Code 0: System Error";
		case ECIC:
			return "Code 1: Function requires GPIB interface to be CIC";
		case ENOL:
			return "Code 2: No listeners on the GPIB";
		case EADR:
			return "Code 3: GPIB interface not addressed correctly";
		case EARG:
			return "Code 4: Invalid argument to function call";
		case ESAC:
			return "Code 5: GPIB interface not System controller as required";
		case EABO:
			return "Code 6: I/O operation aborted (timeout)";
		case ENEB:
			return "Code 7: Nonexistent GPIB interface";
		case EDMA:
			return "Code 8: DMA error";
		case EOIP:
			return "Code 10: Asynchronous I/O in progress";
		case ECAP:
			return "Code 11: No capability for operation";
		case EFSO:
			return "Code 12: File system error";
		case EBUS:
			return "Code 14: GPIB bus error";
		case ESTB:
			return "Code 15: Serial poll status byte queue overflow";
		case ESRQ:
			return "Code 16: SRQ stuck in ON position";
		case ETAB:
			return "Code 20: Table Problem";
		default:
			return "Unrecognized Error Code! Code was " + str( errCode );
	}
}
