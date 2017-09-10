#pragma once
#include "VariableSystem.h"
#include "Expression.h"
#include <sstream>
#include <vector>

/*
 This class is designed to hold the text for a script that the code reads from.
*/
class ScriptStream : public std::stringstream
{
	public:
		ScriptStream::ScriptStream( std::string buf ) : std::stringstream( buf ) {}
		ScriptStream::ScriptStream() : std::stringstream() {}
		
		/* 
		 The main purpose of this class is really to simplify my repeated use of >> 
		 using the following overload.
		 */
		ScriptStream & operator>>( std::string& outputString );
		ScriptStream & operator>>( Expression& expression );

		void loadReplacements( std::vector<std::pair<std::string, std::string>> args );
		void clearReplacements();
		std::string getline();
		std::string getline(char delim);
	private:
		void eatComments();
		bool isNotPartOfName( char test );
		std::vector<std::pair<std::string, std::string>> replacements;
};
