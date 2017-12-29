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
		char scan( );
		void flush( );
		void query( std::string msg, long& data );
		void query( std::string msg, float& data );
		void query( std::string msg, std::string& data );
		char readchar( );
		void errCheck( long status );		
		void errCheck( long status, std::string msg );
		void setAttribute( ViAttr attributeName, ViAttrState value );
		void printf( std::string msg );
		void errQuery( std::string& errMsg, long& errCode );
		void query( std::string msg );
		std::string identityQuery();
	private:
	    const bool deviceSafemode;
		std::string usbAddress;
		ULONG instrument, defaultResourceManager;

};