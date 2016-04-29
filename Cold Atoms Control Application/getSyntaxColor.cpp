#include "stdafx.h"
#include "getSyntaxColor.h"
#include <string>
#include "boost/lexical_cast.hpp"
#include "externals.h"

/*
 * Simple function that checks word for different types of syntax and returns words corresponding to what type of syntax this corresponds to.
 * Return: the function returns one of 
 * "-1" (word size was zero)
 * "com
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
		for (int varInc = 0; varInc < eVariableNames.size(); varInc++)
		{
			if (word == eVariableNames[varInc])
			{
				return "variable";
			}
		}
		if (word == "predefined" || word == "script")
		{
			return "script";
		}
		if (word.size() > 7)
		{
			if (word.substr(word.size() - 7, 7) == ".script")
			{
				return "script file";
			}
		}
	}
	// check Agilent-specific commands
	else if (editType == "AGILENT")
	{
		if (word == "Intensity" || word == "Ramp" || word == "Hold")
		{
			return "command";
		}
		if (word == "Once" || word == "OnceWaitTrig" || word == "lin" || word == "tanh")
		{
			return "option";
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
