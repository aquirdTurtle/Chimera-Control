#include "stdafx.h" 
#include <iostream>
#include <chrono>
#include <vector>
#include "afxwin.h"
#include <fstream>
#include <string>

using namespace std;
using namespace chrono;


int main( )
{
	std::ifstream thresholdFile;
	std::vector<double> thresholds;
	thresholdFile.open ("Threshold-File.txt");
	while ( true )
	{
		double threshold;
		thresholdFile >> threshold;
		if ( thresholdFile.eof ( ) )
		{     
			break;
		}
		thresholds.push_back ( threshold );
	}
	for ( auto thresh : thresholds )
	{
		cout << thresh << ", ";
	}
	cout << endl;
	cout << thresholds.size ( ) << endl;
	cout << thresholds.back ( );
	cin.get ( );
	return 0;
}

