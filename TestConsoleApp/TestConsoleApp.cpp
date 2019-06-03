#include "stdafx.h"  
#include <iostream> 
#include <vector> 
#include <string> 
#include "afxwin.h"


int main( )
{
	auto res = system ( "python C:\\Users\\Regal-Lab\\Code\\Data_Analysis_Control\\ThresholdAnalysis.py" );
	std::cout << res;
	std::cin.get ( );

	return 0;
}
 
