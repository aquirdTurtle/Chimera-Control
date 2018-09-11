// TestConsoleApp2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "afxwin.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
#include "../Chimera/CodeTimer.h"
#include "../Chimera/range.h"


class sineLookupTable
{
	public:
		sineLookupTable ( ULONGLONG len_ )
		{
			len = len_;
		}
		void init ( )
		{
			// calculate from 0 to 2pi
			for ( auto i : range ( len ) )
			{
				double phase = double ( i ) / len * 2 * std::_Pi;
				table.push_back ( std::sin ( phase ) );
			}
		}
		double get ( double phase )
		{
			// wrap
			phase = std::fmod(phase, 2 * std::_Pi );
			// convert to index;
			UINT index = std::round(phase * len / (2 * std::_Pi));
			return table[ index ];
		}
	private:
		ULONGLONG len;
		std::vector<double> table;
};


int main ( )
{
	CodeTimer timer;
	UINT len=1000000;
	sineLookupTable table( 1000000 );
	table.init ( );
	UINT testLen = 1000;

	std::uniform_real_distribution<double> unif( 0, 1e6 );
	std::default_random_engine re;
	std::vector<double> resLookup( testLen );
	std::vector<double> resStd( testLen );
	
	std::vector<double> testVals;
	for ( auto r : range ( testLen ) )
	{
		testVals.push_back ( unif ( re ) );
	}
	timer.tick ( "Before table" );
	for ( auto r : range ( testLen ) )
	{		
		resLookup[ r ] = table.get ( testVals[r] );
	}
	timer.tick ( "Mid" );
	for ( auto r : range ( testLen ) )
	{
		resStd[r] =  std::sin ( testVals[ r ] );
	}
	timer.tick ( "After std" );
	double rms = 0, sum = 0;
	for ( auto i : range ( testLen ) )
	{
		auto t = resLookup[ i ] - resStd[ i ];
		sum += t;
		rms += t*t;
	}
	rms = std::sqrt ( rms );
	std::cout << timer.getTimingMessage ( );
	std::cout << "sum of errors:" << sum << "\n";
	std::cout << "rms of errors:" << rms << "\n";
	std::cin.get ( );
}


