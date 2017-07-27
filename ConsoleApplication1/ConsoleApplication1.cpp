// ConsoleApplication1.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "boost/algorithm/string/replace.hpp"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <boost/tokenizer.hpp>

int main()
{
	size_t num;
	std::string text = "1Freq";
	std::cout << std::stod(text, &num);
	std::cout << " " << num << "\n";
	if (num == text.size())
	{
		std::cout << "y\n";
	}
	text = "12345";
	std::cout << std::stoi(text, &num);
	if (num == text.size())
	{
		std::cout << "y\n";
	}
	std::cout << " " << num << "\n";

	std::cin.get();
	return -1;
}
