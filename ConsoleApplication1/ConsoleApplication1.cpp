// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Windows.h"

int main()
{
	std::cout << "begin...\n";
	HMODULE dio = LoadLibrary( L"DIO64_Visa32.dll" );
	std::cout << dio << std::endl;
	std::cout << "fin\n";
	std::cin.get();
	return 0;
}

