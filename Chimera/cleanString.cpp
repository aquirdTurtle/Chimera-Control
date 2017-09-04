#include "stdafx.h"
#include "cleanString.h"
#include <string>

/*
 * This function deals with the trailing \r\n on each line required for edit controls to make sure everything is consistent. It also makes sure that there is 
 * no crap at the beginning of the string, which happens sommetimes. str is the string which it does this to.
 */
void cleanString(std::string &str)
{
	// make sure that the end of the line has the proper "\r\n" newline structure.
	if (str.length() == 0)
	{
		str += "\r\n";
	}
	else if (str[str.length() - 1] == '\r')
	{
		str.append("\n");
	}
	else if (str[str.length() - 1] == '\n' && str[str.length() - 2] != '\r')
	{
		str[str.length() - 1] = '\n';
		str.append("\n");
	}
	else 
	{
		// no such characters at the end
		str.append("\r\n");
	}

	// make sure the beginning of the line doesn't start with crap.
	// should always be at least one character long based on previous looping.
	bool erasingFlag;
	do 
	{
		erasingFlag = false;
		if (!isalpha(str[0]) && !isdigit(str[0]) && str[0] != ' ' && str[0] != '\t' && !iscntrl(str[0]) && str[0] != '%' 
			&& str[0] != '{' && str[0] != '}')
		{
			// kill it!
			str.erase(0);
			erasingFlag = true;
		}
		else if (iscntrl(str[0]) && str[0] != '\0' && str[0] != '\t')
		{
			// if the line is just a blank line, it should only be two characters long and be "\r\n"
			if (str != "\r\n")
			{
				str.erase(0, 1);
				erasingFlag = true;
			}
		}

	} while (erasingFlag == true);
	return;
}
