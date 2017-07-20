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
	std::stringstream tempStream( text );
	// make sure they are at the same place...
	int pos = tellg();
	tempStream.seekg( pos );
	// get the word.
	tempStream >> outputString;
	// convert to lower-case
	std::transform( outputString.begin(), outputString.end(), outputString.begin(), ::tolower );
	if (outputString.find("(") != std::string::npos)
	{
		// continue getting characters until the ) is found. 
		while (outputString.find(")") == std::string::npos)
		{
			if (tempStream && tempStream.peek() == EOF)
			{
				thrower("ERROR: Unmatched left parenthesis \"(\"");
			}
			std::string tempStr;
			tempStream >> tempStr;
			outputString.append(tempStr);
		}
	}
	// replace any keywords. This doesn't catch it when the keywords are in (), only when they are isolated. Need to 
	// check if this is an issue with using constants with expressions or not.
	for (auto repl : replacements)
	{
		if (outputString == repl.first)
		{
			// auto-replace before the user even needs to deal with this.
			outputString = repl.second;
		}
	}
	try
	{
		outputString = std::to_string(reduce(outputString));
	}
	catch (Error& err)
	{
		// failed to reduce, that's fine, will try again later.
		// this can happen if it's not a reducable quantity (i.e. not enclosed in (), or if there are variables in the
		// string, for example.
	}
	// update the actual stream with the position that the temporary stream ended at.
	seekg( tempStream.tellg() );
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
 via >>. Arguments must all be constants; this variable doesn't keep track of variations
 or anything, although perhaps it could, if the design of this system was different. Right
 now I don't think it suits the design.
 */
void ScriptStream::loadReplacements( std::vector<std::pair<std::string, std::string>> args )
{
	replacements = args;
}


void ScriptStream::clearReplacements()
{
	replacements.clear();
}


void ScriptStream::eatComments()
{
	// Grab the first character
	std::string comment;
	char currentChar = get();
	// including the !file.eof() to avoid grabbing the null character at the end. 
	while ((currentChar == ' ' && !eof()) || (currentChar == '\n' && !eof()) || (currentChar == '\r' && !eof())
			|| (currentChar == '\t' && !eof()) || currentChar == '%' || (currentChar == ';' && !eof()))
	{
		// remove entire comments from the input
		if (currentChar == '%')
		{
			std::getline( *this, comment , '\n' );
		}
		// get the next char
		currentChar = get();
	}
	char next = peek();
	if (next == EOF)
	{
		if (eof())
		{
			clear();
			seekg( -1, SEEK_CUR );
			if (eof())
			{
				// shouldn't happen.
				errBox( "!" );
			}
		}
		return;
	}
	std::streamoff sPos = tellg();
	// when it exits the loop, it will just have moved passed the first non-whitespace character. I want that character. Go back.
	if (sPos == 0)
	{
		seekg( 0, std::ios::beg );
	}
	else
	{
		seekg( -1, SEEK_CUR );
	}
}
