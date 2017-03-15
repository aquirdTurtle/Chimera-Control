#include "stdafx.h"
#include "ScriptStream.h"
#include <algorithm>

ScriptStream & ScriptStream::operator>>( std::string& outputString )
{
	eatComments();
	// there might be a better way to do this. I'm really just creating a 
	// stringstream object in order to access the stringstream >> operator
	// directly. I was having trouble calling the parent class version, not
	// really sure why.
	std::string text = str();
	std::stringstream temp( text );
	// make sure they are at the same place...
	int pos = tellg();
	temp.seekg( pos );
	// get the word.
	temp >> outputString;
	// convert to lower-case
	std::transform( outputString.begin(), outputString.end(), outputString.begin(), ::tolower );
	// replace any keywords
	for (auto repl : this->replacements)
	{
		if (outputString == repl.first)
		{
			// auto-replace before the user even needs to deal with this.
			outputString = repl.second;
		}
	}
	this->seekg( temp.tellg() );
	return *this;
}


bool ScriptStream::isNotPartOfName( char test)
{
	std::vector<char> addendums = { '\t', '\r', '\n', ' ', ')', '(', ',' };
	for (auto elem : addendums)
	{
		if (test == elem)
		{
			return true;
		}
	}
	return false;
}

/*
 * A Wrapper around standard getline that ignores comments before this line.
 */
std::string ScriptStream::getline(char delim)
{
	eatComments();
	std::string line;
	std::getline( *this, line, delim );
	// convert to lower-case
	std::transform( line.begin(), line.end(), line.begin(), ::tolower );
	// look for arg words. This is mostly important for replacements inside function definitions.
	for (auto arg : replacements)
	{
		int pos = line.find( arg.first );
		int start = 0;
		while (pos != std::string::npos)
		{
			if (pos != 0 && !isNotPartOfName( line[pos - 1] ))
			{
				// then it was a part of the name, don't use this again...
				start = pos;
				continue;
			}

			if ( pos + arg.first.size() > line.size() && !isNotPartOfName(line[pos + arg.first.size()]))
			{
				// then it was a part of the name, don't use this again...
				start = pos;
				continue;
			}

			// Made it this far, so replace it.
			line.replace( line.begin() + pos, line.begin() + pos + arg.first.size(), line );
			pos = line.find( arg.first );
		}
	}
	return line;
}

/*
 if you load a vector of function arguments into this object, then the object will
 automatically replace the argument name with the value as the stream dumps contents
 via >>. Arguments must all be singletons; this variable doesn't keep track of variations
 or anything, although perhaps it could, if the design of this system was different. Right
 now I don't think it suits the design.
 */
void ScriptStream::loadReplacements( std::vector<std::pair<std::string, std::string>> args )
{
	this->replacements = args;
}


void ScriptStream::clearReplacements()
{
	this->replacements.clear();
}


void ScriptStream::eatComments()
{
	// Grab the first character
	std::string comment;
	char currentChar = this->get();
	// including the !file.eof() to avoid grabbing the null character at the end. 
	while ((currentChar == ' ' && !this->eof()) || (currentChar == '\n' && !this->eof()) || (currentChar == '\r' && !this->eof())
			|| (currentChar == '\t' && !this->eof()) || currentChar == '%' || (currentChar == ';' && !this->eof()))
	{
		// remove entire comments from the input
		if (currentChar == '%')
		{
			std::getline( *this, comment , '\n' );
		}
		// get the next char
		currentChar = this->get();
	}
	char next = this->peek();
	if (next == EOF)
	{
		if (this->eof())
		{
			this->clear();
			this->seekg( -1, SEEK_CUR );
			if (this->eof())
			{
				errBox( "!" );
			}
		}
		return;
	}
	std::streamoff position = this->tellg();
	// when it exits the loop, it will just have moved passed the first non-whitespace character. I want that character. Go back.
	if (position == 0)
	{
		this->seekg( 0, std::ios::beg );
	}
	else
	{
		this->seekg( -1, SEEK_CUR );
	}
}
