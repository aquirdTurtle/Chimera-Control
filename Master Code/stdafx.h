// stdafx.h
// include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "errBox.h"

/// MFC Stuff
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
/// 
#include "afxcoll.h"
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include "resource.h"
#include <string>
#include <vector>
#include "Control.h"
#include "KeyHandler.h"
#include "VariableSystem.h"
#include <algorithm>

// this can replace std::string() and std::to_string(), as well as providing functionality to set the precision of
// to_string() conversions.
template <typename T> std::string str(T input, const int precision = 6, bool eatZeros = false, bool toLower = false)
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


// first part of pair is the variable component of the time, second part is the "raw" or constant part of the time.
// this structure will be evaluated to determine the real time (given a variation # and variable values) that an 
// operation will take place at.
typedef std::pair<std::vector<std::string>, double> timeType;

/// MY MACROS
// these are all things that I use to make my code a tad simpler.
// simple macro to condense this style of error checking. I might change my usage of errors later.
#define ERRCHK(functionCall) if (!functionCall){return false;}
// verbose must be defined. This is meant to be used in the experiment thread.
#define VERBOSE(text) (if (verbose){input->status->appendText(text, 0);})

/*
* A nice custom class and #define that makes my custom throws have file & code line information. Very nice.
* stolen From http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused
* Slightly modified.
*/
class Error : public std::runtime_error
{
	public:
		Error( const std::string &arg, const char *file, int line ) : std::runtime_error( arg )
		{
			std::ostringstream out;
			out << file << ", Line " << line << ": " << arg;
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

#include <stdexcept>

template <typename IntType>
std::vector<IntType> range( IntType start, IntType stop, IntType step )
{
	if (step == IntType( 0 ))
	{
		throw std::invalid_argument( "step for range must be non-zero" );
	}

	std::vector<IntType> result;
	IntType increment = start;
	while ((step > 0) ? (increment < stop) : (increment > stop))
	{
		result.push_back( increment );
		increment += step;
	}

	return result;
}


template <typename IntType>
std::vector<IntType> range( IntType start, IntType stop )
{
	return range( start, stop, IntType( 1 ) );
}


template <typename IntType>
std::vector<IntType> range( IntType stop )
{
	return range( IntType( 0 ), stop, IntType( 1 ) );
}

void appendText(std::string newText, CEdit& edit);
void appendText(std::string newText, Control<CRichEditCtrl>& edit);

double reduce(std::string expression, key variationKey = {}, UINT variation = -1, 
			  std::vector<variable>& vars = std::vector<variable>());

#define idVerify(idSet, idDefined)	verifyIdsMatch(idSet, idDefined, __FILE__, __LINE__)

template <typename ControlType> void verifyIdsMatch(Control<ControlType>& control, UINT idDefined, const char *file, int line)
{
	if (control.GetDlgCtrlID() != idDefined)
	{
		errBox("ERROR: these two IDs should match: " + str(control.GetDlgCtrlID()) + " and " + str(idDefined) + ". Please "
			   "change the second ID in order to make them match. This occured at file \"" + str(file) + "\" "
			   "line " + str(line));
		throw;
	}
}
