
#include "stdafx.h" //for precompiled header
#include <vector>
#include <iostream>
#include <ctime>
#include <chrono>

int main( )
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end1, end2, end3, end4, end5;
	start = std::chrono::high_resolution_clock::now( );
	std::cout << "hi!";
	end1 = std::chrono::high_resolution_clock::now( );
	std::cout << "hi!!!!";
	end2 = std::chrono::high_resolution_clock::now( );
	std::cout << "hi!";
	end3 = std::chrono::high_resolution_clock::now( );
	std::cout << "hi!";
	end4 = std::chrono::high_resolution_clock::now( );
	std::cout << "hi!";
	end5 = std::chrono::high_resolution_clock::now( );

	std::chrono::duration<double> e1, e2, e3, e4, e5;
	e1 = end1 - start;
	e2 = end2 - end1;
	e3 = end3 - end2;
	e4 = end4 - end3;
	e5 = end5 - end4;
		std::cout << "elapsed time: \n" << e1.count( ) << "s\n"
		<< e2.count( ) << "s\n"
		<< e3.count( ) << "s\n"
		<< e4.count( ) << "s\n"
		<< e5.count( ) << "s\n";
	std::cin.get( );
	return 0;
}
