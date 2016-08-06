// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <bitset>
#include <iostream>
#include "Windows.h"

int main()
{
	WORD stuff;
	std::bitset<16> bits("1111111111111111");
	unsigned short tst = static_cast<unsigned short>(bits.to_ulong());
	std::cout << tst;
	tst += 1;
	std::cout << "\n" << tst;
	std::cout << "\n" << sizeof(tst);
	std::cin.get();
    return 0;
}

