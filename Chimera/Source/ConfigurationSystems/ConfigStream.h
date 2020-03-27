#pragma once
#include <iostream>
#include "ParameterSystem/Expression.h"
#include "Scripts/ScriptStream.h"

/* a small wrapper to handle some special things for configuration files, like handling empty strings.*/
class ConfigStream : public ScriptStream
{
	public:
		ConfigStream () {};
		explicit ConfigStream(std::ifstream& file);
		~ConfigStream () {};
		ConfigStream& operator>>(std::string& outputString);
		ConfigStream& operator>>(Expression& expression);
		template<typename type>
		ConfigStream& operator>> (type& output);
		/*
		ConfigStream& operator<<(std::string& outputString);
		ConfigStream& operator<<(Expression& expression);
		template<typename type>
		ConfigStream& operator<< (type& output);
		*/
		/*
		ConfigStream& operator>>(Expression& expr);
		ConfigStream& operator<<(Expression& expr);
		ConfigStream& operator>>(std::string& txt);
		ConfigStream& operator<<(std::string& txt);
		template<typename type>
		ConfigStream& operator>> (type& output);
		//template<typename type>
		//ConfigStream& operator<< (type& input);
		using std::stringstream::operator<<;
		*/
	private:
		//std::string emptyStringText = "\"!#EMPTY_STRING#!\"";
};

template<typename type>
ConfigStream& ConfigStream::operator>> (type& output)
{
	std::string tempString;
	ScriptStream::operator>>(tempString);
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

std::ostream& operator<<(std::ostream& os, const Expression& expr);