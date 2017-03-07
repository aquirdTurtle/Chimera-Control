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
//#include "Windows.h"
#include "resource.h"
#include <string>
#include <vector>

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
class myException : public std::runtime_error
{
	public:
		myException( const std::string &arg, const char *file, int line ) : std::runtime_error( arg )
		{
			std::ostringstream out;
			out << file << ", Line " << line << ": " << arg;
			msg = out.str();
			bareMsg = arg;
		}

		~myException() throw() {}

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
#define thrower(arg) throw myException(arg, __FILE__, __LINE__)

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

