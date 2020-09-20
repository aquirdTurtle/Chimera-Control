// created by Mark O. Brown
#pragma once
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <QString>
#include "range.h"

template <typename T> QString qstr (T input, const int precision = 13, bool eatZeros = false, bool toLower = false,
	bool zeroPad = false, bool useScientificNotation = false) {
	return QString(str (input, precision, eatZeros, toLower, zeroPad, useScientificNotation).c_str ());
}

// this can replace str() and str(), as well as providing functionality to set the precision of
// to_string() conversions.
template <typename T> std::string str( T input, const int precision = 13, bool eatZeros = false, bool toLower = false,
									   bool zeroPad = false, bool useScientificNotation=false ){
	std::ostringstream out;
	if (useScientificNotation) {
		out << std::scientific;
	}
	else {
		out << std::fixed;
	}
	out << std::setprecision(precision) << input;
	std::string outStr = out.str();
	if ( zeroPad ){
		if ( outStr.find_first_not_of( "-0." ) != std::string::npos || outStr == "0" ){
			int fpos = int ( outStr.find_first_not_of ( "-0." ) );
			if ( int(outStr.size ( )) - fpos - precision < 0 ){
				if ( outStr.find_first_of ( "." ) == std::string::npos ){
					// was int with no decimal, make float so that can add zeros. 
					outStr += ".";
					if ( fpos == -1 ){
						fpos = int(outStr.size());
					}
				}
				// then needs zeros
				for ( auto zero : range ( -(int(outStr.size ( )) - fpos - precision) ) ){
					outStr += "0";
				}
			}
		}
	}
	if (eatZeros){	// this only makes sense if input was a double.
		if (outStr.find(".") != std::string::npos){
			outStr.erase(outStr.find_last_not_of('0') + 1, std::string::npos);
		}
	}
	if (toLower){
		std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::tolower);
	}
	return outStr;
}

template <> std::string str<QString> (QString input, const int precision, bool eatZeros, bool toLower, bool zeroPad, 
	bool useScientificNotation);

template <typename T> std::wstring w_str( T input, const int precision = 13, bool eatZeros = false, bool toLower = false )
{
	std::wostringstream out;
	out << std::setprecision( precision ) << input;
	std::wstring outStr = out.str( );
	if ( eatZeros )
	{
		if ( outStr.find( L"." ) != std::string::npos )
		{
			outStr.erase( outStr.find_last_not_of( '0' ) + 1, std::string::npos );
		}
	}
	if ( toLower )
	{
		std::transform( outStr.begin( ), outStr.end( ), outStr.begin( ), ::tolower );
	}
	return outStr;
}
// overloaded defines are tricky in c++. This is effectively just an overloaded define for 
//#define cstr(input) str(input).c_str()
//#define cstr(input, precision) str(input, precision).c_str()
// the following trick was taken from https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
/// DONT CALL THESE DIRECTLY.
#define GET_MACRO(arg1,arg2,FUNCTION,...) FUNCTION
#define cstr1(input)				str(input).c_str()
#define cstr2(input, precision)		str(input, precision).c_str()
/// JUST CALL THIS
// if the user enters 1 argument then  GET_MACRO calls FOO2, else it calls FOO3 and discards FOO2. Either way, it calls
// the function with __VA_ARGS__ which was the original "..." arguments.
#define cstr(...) GET_MACRO(__VA_ARGS__, cstr2, cstr1)(__VA_ARGS__)