// created by Mark O. Brown
#pragma once
/*
* A nice custom class and #define that makes my custom throws have file & code line information. Very nice.
* stolen From http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused
* Slightly modified, especially with the std rethrowing, which I also got from somewhere, but I forget where. 
*/

#include <sstream>
#include <stdexcept>
#include <regex>
#include <qstring.h>
#include <GeneralUtilityFunctions/my_str.h>

class ChimeraError : public std::runtime_error{
	public:
	ChimeraError ( const std::string &arg, const char *file, int line ) : std::runtime_error ( arg )	{
		loc = std::string ( file ) + "; line " + std::to_string ( line );
		std::ostringstream out;
		out << arg << "\n{@ Location:" << loc << "}";
		msg = out.str ( );
		bareMsg = arg;
	}
	~ChimeraError ( ) throw( ) { }

	const char * what ( ) const throw( ){
		return msg.c_str ( );
	}
	std::string whatBare ( ) const throw( ){
		return bareMsg;
	}
	std::string whatLoc ( ) const throw( ){
		return loc;
	}
	std::string trace ( ){
		return getErrorStackTrace ( *this );
	}
	QString qtrace () {
		return qstr (this->trace ());
	}
	static std::string getErrorStackTrace ( const std::exception& e, unsigned int level = 0 ){
		std::string msg = "Begin ChimeraError Trace:\nERROR: " + std::string ( e.what ( ) );
		std::regex reg ( "\n" );
		msg = std::regex_replace ( msg, reg, "\n" + std::string ( level, '\t' ) );
		std::string stackMsg = std::string ( level, '\t' ) + msg + "\n";
		try	{
			std::rethrow_if_nested ( e );
		}
		catch ( const std::exception& e ){
			stackMsg += getErrorStackTrace ( e, level + 1 );
		}
		catch(...) // I forget why this is here. MOB Aug 1 2020
		{ }
		return stackMsg;
	}
	private:
		std::string msg;
		std::string bareMsg;
		std::string loc;
};

// the following gives any throw call file and line information.
// throw_with_nested makes it possible to chain thrower calls and get a full error stack traceback

// use this if not throwing from inside a catch().
#define thrower(arg) throw ChimeraError(arg, __FILE__, __LINE__) 

// use this ONLY if throwing inside a catch. If throwing outside a catch the program might crash. 
#define throwNested(arg) std::throw_with_nested( ChimeraError(arg, __FILE__, __LINE__))
