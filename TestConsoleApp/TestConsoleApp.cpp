#include "stdafx.h"  
#include <iostream> 
#include <vector> 
#include <string> 
#include "afxwin.h"


int main( )
{
	auto res = system ( "C:\\ProgramData\\Anaconda3\\python C:\\Users\\Mark-Brown\\Documents\\Analysis-Code\\ThresholdAnalysis.py" );
	std::cout << res;
	std::cin.get ( );

	return 0;
}
 
