// TestConsoleApp2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "CodeTimer.h"
#include "range.h"
#include "afxwin.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include "Matrix.h"
#include <fstream>


int increment ( std::vector<UINT>& ind, UINT currentLevel, UINT maxVal )
{
	// if level below
	int res = 0;
	if ( currentLevel != 0 )
	{
		// try iterate below.
		res = increment ( ind, currentLevel - 1, maxVal );
		if ( res != -1 )
		{
			// lower level succeeded. success.
			return res;
		}
	}
	// if reach this point, either lower levels failed or no levels below.
	auto isFin = currentLevel == ind.size ( ) - 1;
	if ( ( isFin && ind[ currentLevel ] != maxVal ) || (!isFin && ind[ currentLevel ] != ind[ currentLevel + 1 ] - 1 ))
	{
		// possible to increment.
		ind[ currentLevel ]++;
		return currentLevel;
	}

	// reset.
	ind[ currentLevel ] = currentLevel;
	return -1;
}


ULONGLONG calcExpected ( UINT m, UINT n )
{
	/*m larger*/
	ULONGLONG tterm = 1;
	for ( auto i : range ( n ) )
	{
		tterm *= m - i;
	}
	std::cout << tterm << "\n";
	for ( auto i : range ( n ) )
	{
		tterm /= (i+1);
	}
	std::cout << tterm << "\n";
	return tterm;
}


int main ( )
{
	std::random_device rd;
	std::mt19937 mt ( rd ( ) );
	std::uniform_real_distribution<double> atomDist ( 0, 1 );
	
	UINT atomArraySize = 10;
	UINT targetDim = 8;
	std::ofstream resultsFile ( "Rearrange_Results_" + str(targetDim) + "x" + str(targetDim) + ".csv" );
	std::vector<std::vector<UINT>> results;
	Matrix<bool> atomArray ( atomArraySize, atomArraySize );

	for ( auto loadInc : range ( 101 ) )
	{
		double loadRate = double(loadInc)/100;
		std::cout << "\b\b\b\b\b     \b\b\b\b\b" << loadRate;
		results.push_back ( std::vector<UINT> ( ) );
		for ( auto repInc : range ( 5000 ) )
		{
			for ( auto& atom : atomArray )
			{
				atom = ( atomDist ( mt ) < loadRate );
			}
			// initialize indexes to 0,1,2,3,...
			std::vector<UINT> xindexes ( targetDim );
			std::vector<UINT> yindexes ( targetDim );
			auto inc = 0;
			for ( auto inc : range ( xindexes.size ( ) ) )
			{
				xindexes[ inc ] = yindexes[ inc ] = inc;
			}
			UINT res;
			UINT count = 0;
			UINT matchCount = 0;
			while ( true )
			{
				// fill test;
				Matrix<bool> testArray ( atomArraySize, atomArraySize, 0 );
				for ( auto xi : xindexes )
				{
					for ( auto yi : yindexes )
					{
						testArray ( xi, yi ) = true;
					}
				}
				UINT match = 0;
				for ( auto i : range ( testArray.size ( ) ) )
				{
					match += testArray.data[ i ] * atomArray.data[ i ];
				}
				if ( match == targetDim*targetDim )
				{
					matchCount++;
				}
				// get next
				res = increment ( xindexes, targetDim - 1, atomArraySize - 1 );
				count++;
				if ( res == -1 )
				{
					res = increment ( yindexes, targetDim - 1, atomArraySize - 1 );
					if ( res == -1 )
					{
						break;
					}
				}
			}
			results[loadInc].push_back ( matchCount );
		}
		resultsFile << "\n" << loadRate << ", ";
		for ( auto res : results[loadInc] )
		{
			resultsFile << res << ", ";
		}
	}
	std::cout << "\nfin.\n";
	std::cin.get ( );
}


