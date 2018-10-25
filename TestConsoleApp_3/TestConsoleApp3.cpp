// TestConsoleApp3.cpp : Defines the entry point for the console application.
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
#include "../Chimera/Thrower.h"
#include "../Chimera/NiawgStructures.h"

int increment ( std::vector<UINT>& ind, UINT currentLevel, UINT maxVal, bool reversed )
{
	// if level below
	int res = 0;
	if ( currentLevel != 0 )
	{
		// try iterate below.
		res = increment ( ind, currentLevel - 1, maxVal, reversed );
		if ( res != -1 )
		{
			// lower level succeeded. success.
			return res;
		}
	}

	// if reach this point, either lower levels failed or no levels below.
	bool condition;
	auto isFin = currentLevel == ind.size ( ) - 1;
	if ( reversed )
	{
		condition = ( isFin && ind[ currentLevel ] != 0 ) || ( !isFin && ind[ currentLevel ] != ind[ currentLevel + 1 ] + 1 );
	}
	else
	{
		condition = ( isFin && ind[ currentLevel ] != maxVal ) || ( !isFin && ind[ currentLevel ] != ind[ currentLevel + 1 ] - 1 );
	}
	if ( condition )
	{
		// possible to increment.
		ind[ currentLevel ] += reversed ? -1 : 1;
		return currentLevel;
	}

	// reset.
	ind[ currentLevel ] = reversed ? maxVal - currentLevel : currentLevel;
	return -1;
}


niawgPair<std::vector<UINT>> findLazyPosition ( Matrix<bool> source, UINT targetDim )
{
	if ( source.getRows ( ) != source.getCols ( ) )
	{
		thrower ( "ERROR! Can't do oddly sized matrixes" );
	}
	UINT sourceDim = source.getRows ( );
	// initialize indexes to 0,1,2,3,...
	niawgPair<std::vector<UINT>> indexes;
	indexes[ Axes::Horizontal ].resize ( targetDim );
	indexes[ Axes::Vertical ].resize ( targetDim );
	auto inc = 0;
	for ( auto inc : range ( indexes[ Axes::Horizontal ].size ( ) ) )
	{
		auto num = sourceDim - inc - 1;
		indexes[ Axes::Horizontal ][ inc ] = indexes[ Axes::Vertical ][ inc ] = num;
		//indexes[ Axes::Horizontal ][ inc ] = indexes[ Axes::Vertical ][ inc ] = inc;
	}
	UINT res, count = 0, matchCount = 0;
	while ( true )
	{
		// fill test;
		Matrix<bool> testArray ( sourceDim, sourceDim, 0 );
		for ( auto xi : indexes[ Axes::Horizontal ] )
		{
			for ( auto yi : indexes[ Axes::Vertical ] )
			{
				testArray ( yi, xi ) = true;
			}
		}
		UINT match = 0;
		for ( auto i : range ( testArray.size ( ) ) )
		{
			match += testArray.data[ i ] * source.data[ i ];
		}
		if ( match == targetDim*targetDim )
		{
			// found a match, horray.
			break;
		}
		// get next
		res = increment ( indexes[ Axes::Horizontal ], targetDim - 1, sourceDim - 1, true );
		count++;
		if ( res == -1 )
		{
			res = increment ( indexes[ Axes::Vertical ], targetDim - 1, sourceDim - 1, true );
			if ( res == -1 )
			{
				thrower("No-Match");
			}
		}
	}
	return indexes;
}


int main ( )
{
	std::random_device rd;
	std::mt19937 e2 ( rd ( ) );
	std::uniform_real_distribution<double> dist ( 0, 1 );
	UINT statNum = 100;
	UINT n = 10;
	CodeTimer timer;
	timer.tick ( "Begin" );
	std::ofstream outFile ( "Fast_Lazy_Rerng_Sim.csv" );
	std::vector<UINT> sizes = { 3,4,5,6,7 };
	for ( auto n : sizes )
	{
		outFile << n << ",";
		std::cout << "Size: " << n << "\n";
		for ( auto lr_ : range ( 100 ) )
		{
			double resNum = 0;
			std::cout << "\b\b" << lr_;
			double lr = double ( lr_ ) / 100.0;
			for ( auto i : range ( statNum ) )
			{
				Matrix<bool> init ( 10, 10, 0 );
				for ( auto& p : init )
				{
					p = dist ( e2 ) > 1 - lr;
				}
				try
				{
					auto res = findLazyPosition ( init, n );
					resNum += 1;
				}
				catch ( Error& )
				{ // failed
				}
			}
			outFile << resNum / statNum << ",";
		}
		outFile << "\n";
		std::cout << "\n";
	}	
	timer.tick ( "Fin" );
	std::cout << timer.getTimingMessage ( );
	std::cin.get ( );
}


