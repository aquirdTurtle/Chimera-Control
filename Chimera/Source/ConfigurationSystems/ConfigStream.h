#pragma once
#include <iostream>
#include "ParameterSystem/Expression.h"
#include "Version.h"
#include "Scripts/ScriptStream.h"
#include <type_traits>
#include <typeinfo>

/* a small wrapper to handle some special things for configuration files, like handling empty strings.*/
class ConfigStream : public ScriptStream{
	public:
		ConfigStream ();
		explicit ConfigStream(std::ifstream& file);
		explicit ConfigStream (std::string, bool isAddress=false);
		ConfigStream& operator>>(std::string& outputString);
		ConfigStream& operator>>(Expression& expression);
		template<typename type>
		ConfigStream& operator>> (type& output);
		std::string getline ();
		std::string getline (char delim);
		static std::string emptyStringTxt;
		Version ver;
		std::string streamText;
private:

};

template<typename type>
ConfigStream& ConfigStream::operator>> (type& output){
	std::string tempString;
	ScriptStream::operator>>(tempString);
	try{
		output = boost::lexical_cast<type>(tempString);
	}
	catch (boost::bad_lexical_cast){
		throwNested ("Scriptstream Failed to convert the text \"" + tempString + "\" to the requested type! Requested "
			"type was \"" + typeid(output).name () + "\".");
	}
	return *this;
}

std::ostream& operator<<(std::ostream& os, const Expression& expr);
