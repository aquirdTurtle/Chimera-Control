#include "stdafx.h"
#include <boost/algorithm/string/replace.hpp>
#include "PiezoFlume.h"
#include "Piezo/CmdLibrary.h"

PiezoFlume::PiezoFlume ( bool sMode, std::string sn ) : safemode(sMode), comPortNumber(sn.begin(), sn.end()){
	comPortNumber.push_back ( '\0' );
}


std::string PiezoFlume::list ( ){	
	if ( !safemode ){		
		std::vector<unsigned char> sNum ( bufferSize );
		auto res = List( &sNum[ 0 ] );
		if ( res < 0 ){
			thrower ( "ERROR: raw \"List\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( res ) + "." );
		}
		for ( auto character : sNum ){
			if ( character == '\0' ){
			}
		}
		std::string tmpStr = std::string ( sNum.begin ( ), sNum.end ( ) ).c_str();
		std::string listStr = str ( res ) + " Device(s): " + tmpStr;
		return listStr;
	}
	else{
		return "SAFEMODE - NO DEVICES";
	}
}


void PiezoFlume::open (  ){
	if ( !safemode ){
		// nBaud and timeout taken from example thorlabs program. Not sure if nBaud is changable without causing probs.
		// for some reason seems that the api wants you to use list before opening a com port. fails otherwise.
		// - MOB Dec 3rd 2019
		//unsigned char buffer[512];
		//memset (buffer, 0, sizeof (buffer));
		std::vector<unsigned char> buf(512,0);
		auto numDev = List (buf.data());
		deviceHandle = Open (comPortNumber.data(), 115200, 10);
		if ( deviceHandle < 0 )	{
			thrower ( "ERROR: raw \"Open\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( deviceHandle ) + "." );
		}
	}
}

bool PiezoFlume::isOpen ( ){
	if ( !safemode ){
		return IsOpen( &comPortNumber[0] );
	}
	else{
		return true;
	}
}


void PiezoFlume::close ( ){
	if ( !safemode ){
		auto res = Close( deviceHandle );
		if ( res < 0 ){
			thrower ( "ERROR: Failed to close!" );
		}
	}
}


double PiezoFlume::getXAxisVoltage ( ){
	if ( !safemode ){
		double val;
		auto res = GetXAxisVoltage( deviceHandle, &val );
		if ( res < 0 ){
			thrower ( "GetXAxisVoltage Failed! Error was: " + str ( res ) + "." );
		}
		return val;
	}
	else{
		return 0;
	}
}

double PiezoFlume::getYAxisVoltage ( ){
	if ( !safemode ){
		double val;
		auto res = GetYAxisVoltage( deviceHandle, &val );
		if ( res < 0 ){
			thrower ( "GetYAxisVoltage Failed! Error was: " + str ( res ) + "." );
		}
		return val;
	}
	else{
		return 0;
	}
}

double PiezoFlume::getZAxisVoltage ( ){
	if ( !safemode ){
		double val;
		auto res = GetZAxisVoltage( deviceHandle, &val );
		if ( res < 0 ){
			thrower ( "GetZAxisVoltage Failed! Error was: " + str ( res ) + "." );
		}
		return val;
	}
	else{
		return 0;
	}
}


std::string PiezoFlume::getDeviceInfo ( ){
	std::string devInfo = getID ( ).c_str();
	boost::replace_all ( devInfo, "\r\r\r", "\r" );
	boost::replace_all ( devInfo, "\r", "\n\t\t" );
	return devInfo;
}


std::string PiezoFlume::getID ( ){
	if ( !safemode ){
		try {
			if (!isOpen ()) {
				thrower ("Piezo Driver Com Port \"" + std::string (comPortNumber.begin (), comPortNumber.end ())
					+ "\" was not open!");
			}
			std::vector<unsigned char> txt (bufferSize, '\0');
			auto res = GetId (deviceHandle, &txt[0]);
			if (res < 0) {
				thrower ("ERROR: raw \"GetId\" function from the thorlabs piezo driver dll returned an error! Error code: "
					+ str (res) + ".");
			}
			return std::string (txt.begin (), txt.end ());
		}
		catch (ChimeraError & err) {
			return err.trace ();
		}
	}
	else{
		return "SAFEMODE";
	}
}

double PiezoFlume::getLimitVoltage ( ){
	if ( !safemode ){
		if ( !isOpen ( ) ){
			thrower ( "Piezo Driver Com Port was not open!" );
		}
		double voltLimit;
		auto res = GetLimitVoltage( deviceHandle, &voltLimit );
		if ( res < 0 ){
			thrower ( "ERROR: raw \"GetLimitVoltage\" function from the thorlabs piezo driver dll returned an error! Error code: "
					  + str ( res ) + "." );
		}
		return voltLimit;
	}
	else{
		return 0;
	}
}

std::string PiezoFlume::getSerialNumber ( ){
	if ( !safemode ){
		if ( !isOpen ( ) ){
			thrower ( "Piezo Driver Com Port was not open!" );
		}
		std::vector<unsigned char> txt ( bufferSize, '\0' );
		auto res = GetSerialNumber( deviceHandle, &txt[ 0 ] );
		if ( res < 0 ){
			thrower ( "ERROR: raw \"GetSerialNumber\" function from the thorlabs piezo driver dll returned an error! "
					  "Error code: " + str ( res ) + "." );
		}
		return std::string ( txt.begin ( ), txt.end ( ) );
	}
	else{
		return "SAFEMODE";
	}
}



void PiezoFlume::setXAxisVoltage ( double val ){
	if ( !safemode ){
		auto res = SetXAxisVoltage( deviceHandle, val );
		if ( res != 0 ){
			thrower ( "Piezo Controller Set X Axis Voltage Failed! (Is voltage in range?) Error code was: " + str ( res ) + "." );
		}
	}
}

void PiezoFlume::setYAxisVoltage ( double val ){
	if ( !safemode ){
		auto res = SetYAxisVoltage( deviceHandle, val );
		if ( res != 0 ){
			thrower ( "Piezo Controller Set Y Axis Voltage Failed! (Is voltage in range?) Error code was: " + str ( res ) + "." );
		}
	}
}

void PiezoFlume::setZAxisVoltage ( double val ){
	if ( !safemode ){
		auto res = SetZAxisVoltage( deviceHandle, val );
		if ( res != 0 ){
			thrower ( "Piezo Controller Set Z Axis Voltage Failed! (Is voltage in range?) Error code was: " + str ( res ) + "." );
		}
	}
}
