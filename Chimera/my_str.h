#pragma once
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>

// this can replace str() and str(), as well as providing functionality to set the precision of
// to_string() conversions.
template <typename T> std::string str(T input, const int precision = 13, bool eatZeros = false, bool toLower = false)
{
	std::ostringstream out;
	out << std::setprecision(precision) << input;
	std::string outStr = out.str();
	if (eatZeros)
	{
		if (outStr.find(".") != std::string::npos)
		{
			outStr.erase(outStr.find_last_not_of('0') + 1, std::string::npos);
		}
	}
	if (toLower)
	{
		std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::tolower);
	}
	return outStr;
}

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

/// Usage note
// the cstr is returning a pointer to an object being created in the macro. That object never gets assigned to anything,
// so after line calling cstr finishes, that object gets destroyed, and the pointer points to nothing. Therefore,
// cstr can not be used if the pointer created is not immediately used to instantiate another object. For example:
//		MessageBox( eMainWindowHwnd, cstr( msg ), "ERROR!", MB_ICONERROR | MB_SYSTEMMODAL );
// works because MessageBox immediately makes a copy of the msg based on it's pointer.
//		const char * tempStr = cstr(msg);
//		MessageBox( eMainWindowHwnd, tempStr, "ERROR!", MB_ICONERROR | MB_SYSTEMMODAL );
// doesn't work because by the time MessageBox is called, the string cstr created has been destroyed and tempStr 
// points to nothing.

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