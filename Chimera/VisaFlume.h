#pragma once
#include <string>
#include "visa.h"
// this is my wrapper around the visa protocol. There are two types of visa on my system, "NI"-Visa and "Agilent"-Visa.
// These in principle are to be used for different, but I believe that this is all low level stuff under the hood. 
class VisaFlume
{
	public:
	    VisaFlume(bool safemode, std::string address);
		void write( std::string message );		
		void close();
		void open();
		void errCheck( long status );
		void setAttribute( ViAttr attributeName, ViAttrState value );
		void printf( std::string msg );
		void errQuery( std::string& errMsg, long& errCode );
		std::string identityQuery();
	private:
	    const bool deviceSafemode;
		std::string usbAddress;
		std::string deviceName;
		ULONG instrument, defaultResourceManager;

};