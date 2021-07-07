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

ConfigStream::ConfigStream () {
	this->precision (std::numeric_limits<double>::max_digits10-1);
	setCase (false);
}

ConfigStream::ConfigStream(std::ifstream& file){
	this->precision (std::numeric_limits<double>::max_digits10-1); 
	ScriptStream::operator<<(file.rdbuf ());
	// config streams are case-sensitive.
	setCase (false);
	streamText = this->str();
};

ConfigStream::ConfigStream (std::string txt, bool isAddr){
	this->precision (std::numeric_limits<double>::max_digits10-1);
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
		ScriptStream::operator<<(txt.c_str());
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

