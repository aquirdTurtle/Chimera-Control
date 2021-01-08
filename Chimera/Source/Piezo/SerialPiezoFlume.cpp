#include "stdafx.h"
#include "SerialPiezoFlume.h"
#include <boost/lexical_cast.hpp>

SerialPiezoFlume::SerialPiezoFlume ( bool sMode, std::string addr ) : safemode(sMode), flumeCore( sMode, addr ){
}

std::string SerialPiezoFlume::getDeviceInfo ( ){
	return flumeCore.query ( "i\r" );
}

void SerialPiezoFlume::open ( ) {
}

void SerialPiezoFlume::close (){
}

void SerialPiezoFlume::setXAxisVoltage (double val){
	auto echo = flumeCore.query("XV" + str (val) + "\r");
}

double SerialPiezoFlume::getXAxisVoltage (){
	auto echo = flumeCore.query ("XR?\r");
	auto ans = flumeCore.read ();
	double dres = 0;
	try{
		if (ans.find ('[') == std::string::npos || ans.find (']') == std::string::npos) {
			thrower ("ERROR! Answer from serial flume trying to read y-voltage not formatted as expected! Answer was: " + ans);
		}
		else { // annoying formatting issues
			auto numStr = ans.substr (ans.find ('[')+1, ans.find (']') - ans.find ('[')-1);
			numStr = numStr.substr (numStr.find_first_not_of (' '));
			dres = boost::lexical_cast<double>(numStr);
		}
	}
	catch (boost::bad_lexical_cast){
		throwNested ("ERROR: Failed to read x-axis voltage from serial piezo driver? Text was " + ans);
	}
	return dres;
}

void SerialPiezoFlume::setYAxisVoltage (double val) {
	auto echo = flumeCore.query ("YV" + str (val) + "\r");
}

double SerialPiezoFlume::getYAxisVoltage () {
	auto echo = flumeCore.query ("YR?\r");
	auto ans = flumeCore.read ();
	double dres = 0;
	try {
		if (ans.find ('[') == std::string::npos || ans.find (']') == std::string::npos) {
			thrower ("ERROR! Answer from serial flume trying to read x-voltage not formatted as expected! Answer was: " + ans);
		}
		else {
			auto numStr = ans.substr (ans.find ('[') + 1, ans.find (']') - ans.find ('[') - 1);
			numStr = numStr.substr (numStr.find_first_not_of (' '));
			dres = boost::lexical_cast<double>(numStr);
		}
	}
	catch (boost::bad_lexical_cast) {
		throwNested ("ERROR: Failed to read Y-axis voltage from serial piezo driver? Text was " + ans);
	}
	return dres;
}

void SerialPiezoFlume::setZAxisVoltage (double val){
	auto echo = flumeCore.query ("ZV" + str (val) + "\r");
}

double SerialPiezoFlume::getZAxisVoltage (){
	auto echo = flumeCore.query ("ZR?\r");
	auto ans = flumeCore.read ();
	double dres = 0;
	try {
		if (ans.find ('[') == std::string::npos || ans.find (']') == std::string::npos) {
			thrower ("ERROR! Answer from serial flume not formatted as expected! Answer was: " + ans);
		}
		else {
			auto numStr = ans.substr (ans.find ('[') + 1, ans.find (']') - ans.find ('[') - 1);
			numStr = numStr.substr (numStr.find_first_not_of (' '));
			dres = boost::lexical_cast<double>(numStr);
		}
	}
	catch (boost::bad_lexical_cast) {
		throwNested ("ERROR: Failed to read Z-axis voltage from serial piezo driver? Text was " + ans);
	}
	return dres;
}
