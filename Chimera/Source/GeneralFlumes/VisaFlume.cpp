// created by Mark O. Brown
#include "stdafx.h"
#include "VisaFlume.h"
#include <qdebug.h>

VisaFlume::VisaFlume(bool safemode, std::string address) : deviceSafemode(safemode), usbAddress(address)
{ }

/// 
void VisaFlume::write( std::string message ) {
	// not sure what this is for. Perhaps actual number of characters sent, or something like that.
	unsigned long actual;
	if (!deviceSafemode) {
		errCheck( viWrite( instrument, (unsigned char*)message.c_str(), (ViUInt32)message.size(), &actual ), message );
	}
}

char VisaFlume::readchar( ){
	unsigned char msg[1024];
	unsigned long numRead;
	errCheck( viRead( instrument, msg, 1, &numRead ) );
	return msg[0];
}


void VisaFlume::close(){
	if (!deviceSafemode){
		errCheck( viClose( defaultResourceManager ) );
	}
}

// open the default resource manager resource. From ni.com:
// "The viOpenDefaultRM() function must be called before any VISA operations can be invoked.The first call to this 
// function initializes the VISA system, including the Default Resource Manager resource, and also returns a session 
// to that resource.  Subsequent calls to this function return unique sessions to the same Default Resource Manager 
// resource. When a Resource Manager session is passed to viClose(), not only is that session closed, but also all 
// find lists and device sessions( which that Resource Manager session was used to create ) are closed.
void VisaFlume::open(){
	if (!deviceSafemode){
		errCheck( viOpenDefaultRM( &defaultResourceManager ) );
		errCheck( viOpen( defaultResourceManager, (char *)cstr( usbAddress ), VI_NULL, VI_NULL, &instrument ) );
		errCheck ( viClear (instrument) );
	}
}


void VisaFlume::setAttribute( ViAttr attributeName, ViAttrState value ){
	if (!deviceSafemode){
		errCheck( viSetAttribute( instrument, attributeName, value ) );
	}
}


std::string VisaFlume::identityQuery(){
	char buf[1024] = { 0 };
	if (!deviceSafemode){
		errCheck( viQueryf( instrument, (ViString)"*IDN?\n", "%1024t", buf ), "*IDN?\n" );
	}
	else{
		return "Device is in Safemode. Change this in the constants.h file.";
	}
	return buf;
}

char VisaFlume::scan( ){
	ViChar c[1024];
	errCheck(viScanf( instrument, "%5t", c ));
	return c[0];
}


void VisaFlume::flush( ){
	errCheck(viFlush( instrument, VI_WRITE_BUF | VI_READ_BUF_DISCARD ));
}

void VisaFlume::query( std::string msg, long& data ){
	if ( !deviceSafemode ){
		errCheck( viQueryf( instrument, (ViString)msg.c_str( ), "%ld", &data ), msg );
	}
	else{
		return;
	}
}

void VisaFlume::query( std::string msg ){
	ViChar data[5000];
	if ( !deviceSafemode ){
		errCheck( viQueryf( instrument, (ViString)msg.c_str( ), "%5000t", data ), msg );
	}
	else {
		return;
	}
}

void VisaFlume::query( std::string msg, float& data ){
	if ( !deviceSafemode ){
		errCheck ( viQueryf( instrument, (ViString)msg.c_str(), "%f", &data ), msg );
		//errCheck (viClear (instrument), msg);
	}
	else{
		return;
	}
}

void VisaFlume::query( std::string msg, std::string& data ){
	char datac[10000];
	ViInt32 totalPoints = 10000;
	if ( !deviceSafemode ){
		errCheck( viQueryf( instrument, (ViString)msg.c_str( ), "%#b", &totalPoints, datac ), msg);
		//errCheck( viQueryf( instrument, (ViString)msg.c_str( ), "%100000T", datac ) );
	}
	else {
		return;
	}
	data = std::string( datac, totalPoints );
}

void VisaFlume::errQuery( std::string& errMsg, long& errCode ){
	char buf[1024] = { 0 };
	if (!deviceSafemode){
		viQueryf( instrument, (ViString)"SYST:ERR?\n", "%1024t", buf );
	}
	else{
		return;
	}
	qDebug () << "Visa Error Msg: " << buf;
	errMsg = str( buf );
}

/*
* This function checks if the agilent throws an error or if there is an error communicating with the agilent.
*/
void VisaFlume::errCheck( long status ){
	errCheck( status, "" );
}

std::string VisaFlume::openErrMsg ( long status ){
	switch ( status ){
		case VI_SUCCESS:
			return "VI_SUCCESS: Session opened successfully.";
		case VI_SUCCESS_DEV_NPRESENT:
			return "VI_SUCCESS_DEV_NPRESENT: Session opened successfully, but the device at the specified address is "
				"not responding.";
		case VI_WARN_CONFIG_NLOADED:
			return "VI_WARN_CONFIG_NLOADED: The specified configuration either does not exist or could not be loaded; "
				"using VISA-specified defaults.";
		case VI_ERROR_INV_OBJECT:
			return "VI_ERROR_INV_OBJECT: The given session reference is invalid.";
		case VI_ERROR_NSUP_OPER:
			return "VI_ERROR_NSUP_OPER: The given sesn does not support this operation. This operation is supported "
				"only by a Resource Manager session.";
		case VI_ERROR_INV_RSRC_NAME:
			return "VI_ERROR_INV_RSRC_NAME: Invalid resource reference specified. Parsing error.";
		case VI_ERROR_INV_ACC_MODE:
			return "VI_ERROR_INV_ACC_MODE: Invalid access mode.";
		case VI_ERROR_RSRC_NFOUND:
			return "VI_ERROR_RSRC_NFOUND: Insufficient location information or resource not present in the system.";
		case VI_ERROR_ALLOC:
			return "VI_ERROR_ALLOC: Insufficient system resources to open a session.";
		case VI_ERROR_RSRC_BUSY:
			return "VI_ERROR_RSRC_BUSY: The resource is valid, but VISA cannot currently access it.";
		case VI_ERROR_RSRC_LOCKED:
			return "VI_ERROR_RSRC_LOCKED: Specified type of lock cannot be obtained because the resource is already "
				"locked with a lock type incompatible with the lock requested.";
		case VI_ERROR_TMO:
			return "VI_ERROR_TMO: A session to the resource could not be obtained within the specified openTimeout period.";
		case VI_ERROR_LIBRARY_NFOUND:
			return "VI_ERROR_LIBRARY_NFOUND: A code library required by VISA could not be located or loaded.";
		case VI_ERROR_INTF_NUM_NCONFIG:
			return "VI_ERROR_INTF_NUM_NCONFIG: The interface type is valid, but the specified interface number is not"
				" configured.";
//		case VI_ERROR_MACHINE_NAVAIL:
//			return "VI_ERROR_MACHINE_NAVAIL: The remote machine does not exist or is not accepting any connections. "
//				"If the NI-VISA server is installed and running on the remote machine, it may have an incompatible "
//				"version or may be listening on a different port.";
		case VI_ERROR_NPERMISSION:
			return "VI_ERROR_NPERMISSION: Access to the remote machine is denied.";
		default:
			return "Error Code Not found???";
	}
}

void VisaFlume::errCheck( long status, std::string msg ){
	long errorCode = 0;
	// Check comm status
	if (status < 0){
		std::string throwerMsg = "VisaFlume Communication error! (Is the device connected?) Error Code: " 
			+ str( status ) + ": " + openErrMsg(status) + "\r\n";
		if ( msg != "" ){
			// Error detected.
			throwerMsg += "Error seen while writing the following message: " + msg + "\r\n";
		}
		// Error detected.
		thrower ( throwerMsg );
	}
	// Query the agilent for errors.
	std::string errMessage;
	errQuery( errMessage, errorCode );
	if (errorCode != 0){
		std::string throwerMsg = "Visa returned error message: " + str( errorCode ) + ":" + errMessage;
		if ( msg != "" ){
			// Error detected.
			throwerMsg += "Error seen while writing the following message: " + msg + "\r\n";
		}
		// Agilent error
		thrower ( throwerMsg );
	}
}

void VisaFlume::printf( std::string msg ){
	if (!deviceSafemode){
		errCheck( viPrintf( instrument, (ViString)cstr( msg ) ) );
	}
}

