// created by Mark O. Brown
#include "stdafx.h"
#include "ScriptStream.h"
#include <algorithm>

/*
Notes on function args problem

See tests for examples
Possible solutions:
- Decoration method
	- need to parse expressions in analyzing function args to replace parts of expressions
		- shouldn't need to change >> operator functionality
	- replace variable names with decorated names which indicate scope
	- effectively deals with name conflict issue
- change scoping of expression
	- add exceptions to expression, keeping existing structure.
		- doesn't deal with name conflict issue
	- expression core structure is vector of strings instead of single string
		- add vector of scopes
*/


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
	long long pos = tellg();
	temp.seekg( pos );
	// get the word.
	std::string tempStr;
	
	bool isExpression = false;
	outputString = "";
	int unclosedParentheses = 0;
	do
	{
		temp >> tempStr;
		if ( tempStr == "" )
		{
			break;
		}
		std::vector<std::string> tempTerms = Expression::splitString( tempStr );
		for ( auto& term : tempTerms )
		{
			if ( term == "(" )
			{
				unclosedParentheses++;
			}
			else if ( term == ")" )
			{
				if ( unclosedParentheses == 0 )
				{
					thrower ( "ERROR: excessive right parenthesis in input! \")\" was seen when there were no \"(\" to "
							 "close." );
				}
				unclosedParentheses--;
			}
			// convert to lower-case
			std::transform( term.begin( ), term.end( ), term.begin( ), ::tolower );
			// replace any keywords
			for ( auto repl : replacements )
			{
				if ( term == repl.first )
				{
					// auto-replace before the user even needs to deal with this.
					term = repl.second;
				}
			}
			outputString += term;
		}
	} while ( unclosedParentheses > 0 );

	seekg( temp.tellg() );
	return *this;
}


ScriptStream & ScriptStream::operator>>( Expression& expression )
{
	return operator>>( expression.expressionStr );
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

std::string ScriptStream::getline()
{
	return getline( '\n' );
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
void ScriptStream::loadReplacements( std::vector<std::pair<std::string, std::string>> args, std::vector<parameterType>& params, 
									 std::string paramDecoration, std::string replCallScope, std::string funcScope )
{
	// need to parse each replacement string and decorate parameters to give proper scoping
	for (auto replNum : range(args.size()))
	{
		auto& repl = args[replNum];
		auto replSplit = Expression::splitString(repl.second);
		for (auto& replTerm : replSplit)
		{
			for (auto& param : params)
			{
				if (replTerm == param.name && (param.parameterScope == replCallScope
					|| param.parameterScope == GLOBAL_PARAMETER_SCOPE))
				{
					// is a variable with the right scope for the replacement. Make a copy of the parameter, 
					// decorate the name, and change the scope.
					auto replParam = param;
					// make it hard to accidentally cause conflicts with the new name. paramDecoration should be the
					// name of the calling function.
					replParam.name += "#!@#" + paramDecoration + "#!@#";
					replParam.parameterScope = funcScope;
					params.push_back (replParam);
					replTerm = replParam.name;
				}
			}
		}
		std::string finReplString = "";
		for (auto replTerm : replSplit)
		{
			finReplString += replTerm;
		}
		args[replNum].second = finReplString;
	}
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
				thrower ( "!" );
			}
		}
		return;
	}
	std::streamoff position = tellg();
	// when it exits the loop, it will just have moved passed the first non-whitespace character. I want that character. Go back.
	if (position == 0)
	{
		seekg( 0, std::ios::beg );
	}
	else
	{
		seekg( -1, SEEK_CUR );
	}
}
