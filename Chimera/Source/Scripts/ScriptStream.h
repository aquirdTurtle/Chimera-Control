// created by Mark O. Brown
#pragma once
#include "ParameterSystem/Expression.h"
#include <sstream>
#include <vector>
#include <string>

/*
 This class is designed to hold the text for a script that the code reads from.
*/
class ScriptStream : public std::stringstream
{
	public:
		ScriptStream::ScriptStream( std::string buf ) : std::stringstream( buf ) {}
		ScriptStream::ScriptStream() : std::stringstream() {}
		ScriptStream::ScriptStream (std::ifstream& file);
		void setCase (bool alwaysLowerCase);
		/* 
		 The main purpose of this class is really to simplify my repeated use of >> 
		 using the following overload.
		 */
		ScriptStream & operator>>( std::string& outputString );
		ScriptStream & operator>>( Expression& expression );
		template<typename type>
		ScriptStream& operator>> (type& output);

		void loadReplacements (std::vector<std::pair<std::string, std::string>> args, std::vector<parameterType>& params,
			std::string paramDecoration, std::string replCallScope, std::string funcScope);
		void clearReplacements();
		std::string getline();
		std::string getline(char delim);
	private:
		void eatComments();
		bool isNotPartOfName( char test );
		std::vector<std::pair<std::string, std::string>> replacements;
		bool alwaysLowerCase=true;
};

template<typename type>
ScriptStream& ScriptStream::operator>> (type& output)
{
	std::string tempString;
	*this >> tempString;
	try
	{
		output = boost::lexical_cast<type>(tempString);
	}
	catch (boost::bad_lexical_cast)
	{
		throwNested ("Scriptstream Failed to convert the text\"" + tempString + "\" to the requested type!");
	}
	return *this;
}