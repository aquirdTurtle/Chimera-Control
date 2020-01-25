// created by Mark O. Brown
#pragma once
/*
* A nice custom class and #define that makes my custom throws have file & code line information. Very nice.
* stolen From http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused
* Slightly modified.
*/

#include <sstream>
#include <stdexcept>
#include <regex>

class Error : public std::runtime_error
{
	public:
	Error ( const std::string &arg, const char *file, int line ) : std::runtime_error ( arg )
	{
		loc = std::string ( file ) + "; line " + std::to_string ( line );
		std::ostringstream out;
		out << arg << "\n@ Location:" << loc;
		msg = out.str ( );
		bareMsg = arg;
	}
	~Error ( ) throw( ) { }

	const char * what ( ) const throw( )
	{
		return msg.c_str ( );
	}
	std::string whatBare ( ) const throw( )
	{
		return bareMsg;
	}
	std::string whatLoc ( ) const throw( )
	{
		return loc;
	}
	std::string trace ( )
	{
		return getErrorStackTrace ( *this );
	}
	static std::string getErrorStackTrace ( const std::exception& e, unsigned int level = 0 )
	{
		std::string msg = "ERROR: " + std::string ( e.what ( ) );
		std::regex r ( "\n" );
		msg = std::regex_replace ( msg, r, "\n" + std::string ( 2*level, ' ' ) );
		std::string stackMsg = std::string ( 2*level, ' ' ) + msg + "\n";
		try
		{
			std::rethrow_if_nested ( e );
		}
		catch ( const std::exception& e )
		{
			stackMsg += getErrorStackTrace ( e, level + 1 );
		}
		catch(...)
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
#define thrower(arg) throw Error(arg, __FILE__, __LINE__) 

// use this if throwing inside a catch.
#define throwNested(arg) std::throw_with_nested( Error(arg, __FILE__, __LINE__))