#include "stdafx.h"
#include "Gpib.h"
#include "ni488.h"
#include <array>
#include "constants.h"


std::string Gpib::query( std::string query )
{
	send( query );
	return receive();
}


Gpib::Gpib(short device, bool safemode)
{
	deviceSafemode = safemode;
	deviceID = device;
	int hpone, agilentTwo, srsTwo, srsThree, pulseGen, pulseGen2, microHP, powerHP, agilent;
	// I think that a lot of these aren't actually doing anything... I think these are supposed to open devices.
	try
	{
		hpone = ibdev(17);
		agilentTwo = ibdev(12);
		srsTwo = ibdev(6);
		srsThree = ibdev(19);
		pulseGen = ibdev(7);
		pulseGen2 = ibdev(4);
		microHP = ibdev(10);
		powerHP = ibdev(5);
		agilent = ibdev(11);
	}
	catch (Error& err)
	{
		errBox("GPIB Initialization failed!: " + err.whatStr());
	}
}


// send message to address.
void Gpib::send( std::string message)
{
	if ( !deviceSafemode)
	{
		Send( 0, deviceID, (void*) cstr(message), message.size(), NLend );
		if ( ibsta == ERR )
		{
			thrower( "GPIB ERROR: " + getErrMessage( iberr ) );
		}
	}
}


std::string Gpib::receive()
{
	//char msg[256] = "";
	char msg[256];
	if (!deviceSafemode)
	{
		Receive( 0, deviceID, (void*)msg, 256, STOPend );
		// this error handling doesn't seem to work with send / receive... prob this only works with 488, not 488.2
		if (ibcntl == 0)
		{
			thrower( "GPIB ERROR: " + getErrMessage( iberr ) );
		}
	}
	std::string msgStr( msg );
	return msgStr.substr(0, ibcntl);
}


// "gets the device descriptor ud."
int Gpib::ibdev(int pad)
{
	// board address
	int bdindx = 0;   
	// see gpib dos manual for the rest
	int sad = 0; 
	int tmo = 13;
	int eot = 1;
	int eos = 0;
	if (!deviceSafemode)
	{
		int id = ::ibdev(bdindx, pad, sad, tmo, eot, eos);
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



std::string Gpib::queryIdentity()
{
	try
	{
		std::string result = query( "*IDN?" );
		if (result != "")
		{
			return result;
		}
		else
		{
			return "Disconnected...";
		}

	}
	catch ( Error& exception )
	{
		return exception.what();
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
