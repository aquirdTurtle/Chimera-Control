#include "stdafx.h"
#include "ConfigStream.h"

std::string ConfigStream::emptyStringTxt= "\"!#EMPTY_STRING#!\"";

std::ostream& operator<<(std::ostream& os, const Expression& expr){
	if (expr.expressionStr == ""){
		os << ConfigStream::emptyStringTxt;
	}
	else{
		os << expr.expressionStr.c_str ();
	}
	return os;
}

ConfigStream::ConfigStream(std::ifstream& file){
	ScriptStream::operator<<(file.rdbuf ());
	// config streams are case-sensitive.
	setCase (false);
};

ConfigStream::ConfigStream (std::string txt, bool isAddr){
	if (isAddr){
		std::ifstream cFile (txt);
		if (!cFile.is_open ()){
			thrower ("Failed to open file for initializing config stream!");
		}
		ScriptStream::operator<<(cFile.rdbuf ());
		// config streams are case-sensitive.
		setCase (false);
		cFile.close ();
		streamText = this->str ();
	}
	else{
		ScriptStream::ScriptStream (txt);
		streamText = this->str ();
	}
};

ConfigStream& ConfigStream::operator>>(Expression& expression){
	ScriptStream::operator>>(expression.expressionStr);
	if (expression.expressionStr == emptyStringTxt){
		expression.expressionStr = "";
	}
	return *this;
}

ConfigStream& ConfigStream::operator>>(std::string& txt){
	ScriptStream::operator>>(txt);
	if (txt == emptyStringTxt){
		txt = "";
	}
	return *this;
}

std::string ConfigStream::getline (){
	return ConfigStream::getline ('\n');
}

std::string ConfigStream::getline (char delim){
	auto txt = ScriptStream::getline (delim);
	return txt == ConfigStream::emptyStringTxt ? "" : txt;
}

