#include "stdafx.h"
#include "ConfigStream.h"

std::ostream& operator<<(std::ostream& os, const Expression& expr)
{
	if (expr.expressionStr == "")
	{
		os << "\"!#EMPTY_STRING#!\"";
	}
	else
	{
		os << expr.expressionStr.c_str ();
	}
	return os;
}

ConfigStream::ConfigStream(std::ifstream& file)
{
	std::stringstream tempStream;
	tempStream << file.rdbuf ();
	this->ScriptStream::ScriptStream (tempStream.str ());
	// config streams are case-sensitive.
	setCase (false);
};


ConfigStream& ConfigStream::operator>>(Expression& expression)
{
	ScriptStream::operator>>(expression.expressionStr);
	if (expression.expressionStr == "\"!#EMPTY_STRING#!\"")
	{
		expression.expressionStr = "";
	}
	return *this;
}

ConfigStream& ConfigStream::operator>>(std::string& txt)
{
	ScriptStream::operator>>(txt);
	if (txt == "\"!#EMPTY_STRING#!\"")
	{
		txt = "";
	}
	return *this;
}
/*
ConfigStream& ConfigStream::operator<<(Expression& expr)
{
	std::stringstream::operator<<(expr.expressionStr == "" ? "\"!#EMPTY_STRING#!\"" : expr.expressionStr.c_str());
	return *this;
}

ConfigStream& ConfigStream::operator<<(std::string& txt)
{
	std::stringstream::operator<<( txt == "" ? "\"!#EMPTY_STRING#!\"" : txt.c_str() );
	return *this;
}
*/