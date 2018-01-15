#include "stdafx.h" 
#include <iostream>
#include "nifgen.h"


int main( )
{
	std::cout << "...";
	ViSession vi;
	int result = niFgen_init( "Dev6", 0, 1, &vi );
	std::cout << "!  " << result << ", " << vi;
	std::cin.get( );
	return 0;
}

