#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iomanip>

#include "afxwin.h"
#include "Control.h"

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

/// a set of functions that take more arbitrary things to strings that std::to_string (which is also rather wordy for such a simple 
/// function...

#define cstr(input) str(input).c_str()

std::string str(std::string string)
{
	return string;
}

std::string str( const char * text )
{
	return std::string(text);
}

std::string str( char * text )
{
	return std::string( text );
}

template <typename type> std::string str( type quantity )
{
	return std::to_string( quantity );
}

std::string doubleToString( double number, long precision );

void inline errBox(std::string msg)
{
	MessageBox(0, msg.c_str(), "ERROR!", MB_ICONERROR);
}

void inline idVerify(int idSet, int idDefined)
{
	if (idSet != idDefined)
	{
		errBox("ERROR: these two IDs should match: " + str(idSet) + " and " + str(idDefined) + ". Please change the second ID in order to make them match. Throwing to show the location of this error...");
		throw;
	}
}

