#include "stdafx.h"
#include "getSyntaxColor.h"
#include <string>
#include "boost/lexical_cast.hpp"
#include "externals.h"
#include <algorithm>

/*
 * Simple function that checks word for different types of syntax and returns words corresponding to what type of syntax this corresponds to.
 * Return: the function returns one of 
 * "-1" (word size was zero)
 * ...
 */
std::string getSyntaxColor(std::string word, std::string editType)
{
	// check special cases
	if (word.size() == 0)
	{
		return "-1";
	}
	else if (word[0] == '%')
	{
		if (word.size() > 1)
		{
			if (word[1] == '%')
			{
				return "comment2";
			}
		}
		return "comment1";
	}
	// Check NIAWG-specific commands
	if (editType == "NIAWG")
	{
		if (word == "gen" || word == "1," || word == "2," || word == "3," || word == "4," || word == "5," || word == "freq" || word == "amp" || word == "const"
			|| word == "&" || word == "ramp")
		{
			return "command";
		}
		// check logic
		if (word == "repeat" || word == "until" || word == "trigger" || word == "end" || word == "forever")
		{
			return "logic";
		}
		// check options
		if (word == "lin" || word == "nr" || word == "tanh")
		{
			return "option";
		}
		// check variable
		for (int varInc = 0; varInc < eVariables.getCurrentNumberOfVariables(); varInc++)
		{
			if (word == eVariables.getVariableInfo(varInc).name)
			{
				return "variable";
			}
		}
		if (word == "predefined" || word == "script")
		{
			return "script";
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".nScript")
			{
				return "script file";
			}
		}
	}
	// check Agilent-specific commands
	else if (editType == "AGILENT")
	{
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);
		if (word == "agilent" || word == "ramp" || word == "hold" || word == "predefined" || word == "script")
		{
			return "command";
		}
		if (word == "once" || word == "oncewaittrig" || word == "lin" || word == "tanh" || word == "repeatuntiltrig")
		{
			return "option";
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".aScript")
			{
				return "script file";
			}
		}
	}

	// check delimiter
	if (word == "#")
	{
		return "delimiter";
	}
	// see if it's a double.
	try
	{
		boost::lexical_cast<double>(word);
		return "number";
	}
	catch (boost::bad_lexical_cast &)
	{
		return "unrecognized";
	}
}
