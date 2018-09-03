#pragma once
/*
* A nice custom class and #define that makes my custom throws have file & code line information. Very nice.
* stolen From http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused
* Slightly modified.
*/

#include <sstream>
#include <stdexcept>


class Error : public std::runtime_error
{
	public:
	Error( const std::string &arg, const char *file, int line ) : std::runtime_error( arg )
	{
		std::ostringstream out;
		out << file << ":" << line << ": " << arg;
		msg = out.str( );
		bareMsg = arg;
	}

	~Error( ) throw() {}

	const char *what( ) const throw()
	{
		return msg.c_str( );
	}
	std::string whatStr( ) const throw()
	{
		return msg;
	}
	std::string whatBare( ) const throw()
	{
		return bareMsg;
	}
	private:
	std::string msg;
	std::string bareMsg;
};


// the following gives any throw call file and line information.
#define thrower(arg) throw Error(arg, __FILE__, __LINE__)
