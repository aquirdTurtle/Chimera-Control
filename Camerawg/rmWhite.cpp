#include "stdafx.h"

#include "rmWhite.h"

#include <iostream>

/**
 * The purpose of this function is to look for any whitespace (tabs, spaces, or newlines) just past where the file cursor currently is, and remove said white-
 * space. This is to make it easier to read in data without worrying extensively about how the user formatted their instructions.
 * @param file This is the file whose white-space is about to be removed. 
 */
void rmWhite(std::fstream &file)
{
	// Grab the first character
	std::string comment;
	char currentChar = file.get();
	// including the !file.eof() to avoid grabbing the null character at the end. 
	while ((currentChar == ' ' && !file.eof()) || (currentChar == '\n' && !file.eof()) || (currentChar == '\r' && !file.eof()) 
		|| (currentChar == '\t' && !file.eof()) || currentChar == '%' || (currentChar == ';' && !file.eof()))
	{
		// remove entire comments from the input
		if (currentChar == '%')
		{
			getline(file, comment, '\r');
		}
		// get the next char
		currentChar = file.get();
	}
	char next = file.peek();
	if (next == EOF)
	{
		return;
	}

	std::streamoff pos = file.tellg();
	// when it exits the loop, it will just have moved passed the first non-whitespace character. I want that character. Go back.
	if (pos == 0)
	{
		file.seekg(0, std::ios::beg);
	}
	else
	{
		file.seekg(-1, SEEK_CUR);
	}

	return;
}