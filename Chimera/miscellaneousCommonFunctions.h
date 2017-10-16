#pragma once
#include "Control.h"
#include "afxwin.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iomanip>

/*
 * A nice custom class and #define that makes my custom throws have file & code line information. Very nice.
 * stolen From http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused
 * Slightly modified.
 */


class Error : public std::runtime_error
{
	public:
		Error(const std::string &arg, const char *file, int line) : std::runtime_error(arg)
		{
			std::ostringstream out;
			out << file << ":" << line << ": " << arg;
			msg = out.str();
			bareMsg = arg;
		}

		~Error() throw() {}

		const char *what() const throw() 
		{
			return msg.c_str();
		}
		std::string whatStr() const throw()
		{
			return msg;
		}
		std::string whatBare() const throw()
		{
			return bareMsg;
		}
	private:
		std::string msg;
		std::string bareMsg;
};


// the following gives any throw call file and line information.
#define thrower(arg) throw Error(arg, __FILE__, __LINE__)


// shows error message if it exists. Could be function but for consistency with other ERR_X Macros...
#define ERR_POP(string) {if (string != ""){errBox(string);}}
// shows error message and exits given function with error.
#define ERR_POP_RETURN(string) {if (string != ""){errBox(string); return;}}

/*
 * This functions appends the text "newText" to the edit control corresponding to textIDC.
 */
//void appendText(std::string newText, int textIDC, HWND parentWindow);
void appendText(std::string newText, CEdit& edit);
void appendText(std::string newText, Control<CRichEditCtrl>& edit);

template <typename IntType> std::vector<IntType> range( IntType start, IntType stop, IntType step )
{
	if ( step == IntType( 0 ) )
	{
		throw std::invalid_argument( "step for range must be non-zero" );
	}

	std::vector<IntType> result;
	IntType inc = start;
	while ( (step > 0) ? (inc < stop) : (inc > stop) )
	{
		result.push_back( inc );
		inc += step;
	}

	return result;
}


template <typename IntType> std::vector<IntType> range( IntType start, IntType stop )
{
	return range( start, stop, IntType( 1 ) );
}

template <typename IntType> std::vector<IntType> range( IntType stop )
{
	return range( IntType( 0 ), stop, IntType( 1 ) );
}

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
//
#define idVerify(idSet, ...)	verifyIdsMatch(idSet, {__VA_ARGS__}, __FILE__, __LINE__)

/// a set of functions that take more arbitrary things to strings that str (which is also rather wordy for such a simple 
/// function...

// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void errBox( T msg )
{
	MessageBox( eMainWindowHwnd, cstr( msg ), "ERROR!", MB_ICONERROR | MB_SYSTEMMODAL );
}


// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void infoBox( T msg )
{
	MessageBox( eMainWindowHwnd, cstr( msg ), "Info", MB_ICONWARNING );
}

template <typename T> int promptBox( T msg, UINT promptStyle )
{
	return MessageBox( eMainWindowHwnd, cstr( msg ), "Prompt", promptStyle | MB_SYSTEMMODAL );
}

template <typename ControlType> void verifyIdsMatch(Control<ControlType>& control, std::vector<UINT> ids,
													 const char *file, int line)
{
	std::string idString;
	for (auto id : ids)
	{
		if (control.GetDlgCtrlID() == id)
		{
			return;
		}
		idString += str(id) + ", ";
	}

	// if it reaches here, it didn't
	errBox("ERROR: the following ID: " + str(control.GetDlgCtrlID()) + " should match one of \"" + idString + "\". "
		   "Please change the second ID in order to make them match. This occured at file \"" + str(file) + "\" "
		   "line " + str(line));

	throw;
}


