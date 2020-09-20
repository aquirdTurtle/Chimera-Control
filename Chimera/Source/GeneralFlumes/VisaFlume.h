// created by Mark O. Brown
#pragma once
#include <string>
#include "visa.h"

/// 
/* 
What is VISA (Virtual Instrument Software Architecture)?
VISA is a standardized I/O API for communicating with test and measurement devices, such as our waveform generators,
oscilloscopes. It is very general, and has been implemented over a variety of serialized (like USB) and parallel 
(like GPIB) communication busses.  It typically makes use of the "Standard Commands for Programmable Instruments" 
syntax.

https://en.wikipedia.org/wiki/Virtual_instrument_software_architecture
https://en.wikipedia.org/wiki/Standard_Commands_for_Programmable_Instruments

A couple key intro paragraphs from the VISA wiki:

"Virtual instrument software architecture, commonly known as VISA, is a widely used I/O API in the test and measurement 
(T&M) industry for communicating with instruments from a computer. VISA is an industry standard implemented by several 
T&M companies, such as, Anritsu, Bustec, Keysight Technologies, Kikusui, National Instruments, Rohde & Schwarz, 
and Tektronix."

"The VISA library has standardized the presentation of its operations over several software reuse mechanisms, including
through a C API exposed from Windows DLL, visa32.dll, over the Microsoft COM technology, and through a .NET API. 
Although there are several VISA vendors and implementations, applications written against VISA are (nominally) 
vendor-interchangeable thanks to the standardization of VISA's presentation and operations/capabilities. 
Implementations from specific vendors[specify] are also available for less common programming languages[specify] and 
software reuse technologies."
*/


// this is my wrapper around the visa protocol. There are two types of visa on my system, "NI"-Visa and "Keysight"-Visa.
// These in principle are to be used for different, but I believe that this is all low level stuff under the hood. 

class VisaFlume{
	public:
		// THIS CLASS IS NOT COPYABLE.
		VisaFlume& operator=(const VisaFlume&) = delete;
		VisaFlume (const VisaFlume&) = delete;

	    VisaFlume(bool safemode, std::string address);
		void write( std::string message );
		void close( );
		void open( );
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
		std::string openErrMsg ( long status );
		std::string identityQuery();
		const bool deviceSafemode;
		const std::string usbAddress;
	private:
		unsigned long instrument, defaultResourceManager;

};