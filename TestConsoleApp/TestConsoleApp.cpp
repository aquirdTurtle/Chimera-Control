
#include "stdafx.h" 
#include <vector>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <Windows.h>
//#include <boost/lexical_cast.hpp>

// this can replace str() and str(), as well as providing functionality to set the precision of
// to_string() conversions.
template <typename T> std::string str( T input, const int precision = 6, bool eatZeros = false, bool toLower = false )
{
	std::ostringstream out;
	out << std::setprecision( precision ) << input;
	std::string outStr = out.str( );
	if ( eatZeros )
	{
		if ( outStr.find( "." ) != std::string::npos )
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
// the following trick was taken from 
// https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
/// DONT CALL THESE DIRECTLY.
#define GET_MACRO(arg1,arg2,FUNCTION,...) FUNCTION
#define cstr1(input)				str(input).c_str()
#define cstr2(input, precision)		str(input, precision).c_str()
// if the user enters 1 argument then  GET_MACRO calls FOO2, else it calls FOO3 and discards FOO2. Either way, it calls
// the function with __VA_ARGS__ which was the original "..." arguments.
#define cstr(...) GET_MACRO(__VA_ARGS__, cstr2, cstr1)(__VA_ARGS__)

int main( )
{
	int num = 5;
	const char * p = std::to_string( num ).c_str( );
	std::cout << p;
	std::cin.get( );
	return 0;
}
