// ConsoleApplication1.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
//#include "boost/algorithm/string/replace.hpp"
//#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <afxsock.h>

int main()
{
	AfxSocketInit();
	CSocket test;
	int result;
	result = test.Create();
	int a = GetLastError();
	std::cout << result << "\t" << a;
	std::cin.get();
	return -1;
}
