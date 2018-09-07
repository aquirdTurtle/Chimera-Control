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
#include "../Chimera/NiawgController.h"
#include "../Chimera/externals.h"
#include "../Chimera/CodeTimer.h"


ULONGLONG calcExpected ( UINT m, UINT n )
{
	/*m larger*/
	ULONGLONG tterm = 1;
	for ( auto i : range ( n ) )
	{
		tterm *= m - i;
	}
	for ( auto i : range ( n ) )
	{
		tterm /= ( i + 1 );
	}
	return tterm;
}


int main ( )
{
	CodeTimer timer;
	std::random_device rd;
	std::mt19937 mt ( rd ( ) );
	std::uniform_real_distribution<double> atomDist ( 0, 1 );
	UINT repNum = 1;
	UINT atomArraySize = 10;
	UINT targetDim = 9;
	std::ofstream resultsFile ( "Rearrange_Results_" + str ( targetDim ) + "x" + str ( targetDim ) + ".csv" );
	std::vector<std::vector<UINT>> results;
	Matrix<bool> atomArray ( atomArraySize, atomArraySize );
	bool loadRateScan = false;
	bool singleMatch = false;
	UINT attemptCount = 0;
	for ( auto loadInc : range ( loadRateScan ? 101 : 1 ) )
	{
		double loadRate = loadRateScan ? double ( loadInc ) / 100 : 0.8;
		if ( loadRateScan )
		{
			std::cout << "\b\b\b\b\b     \b\b\b\b\b" << loadRate;
		}
		results.push_back ( std::vector<UINT> ( ) );
		for ( auto repInc : range ( repNum ) )
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
				auto num = atomArraySize - inc - 1;
				std::cout << num << ", ";
				xindexes[ inc ] = yindexes[ inc ] = num;
				//xindexes[ inc ] = yindexes[ inc ] = inc;
			}
			std::cout << "\n";
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
					if ( singleMatch )
					{
						std::cout << "\n" << atomArray.print ( ) << "\n" << testArray.print ( ) << "\n";
						break;
					}
				}
				// get next
				attemptCount++;
				res = NiawgController::increment ( xindexes, targetDim - 1, atomArraySize - 1, true );
				count++;
				if ( res == -1 )
				{
					res = NiawgController::increment ( yindexes, targetDim - 1, atomArraySize - 1, true );
					if ( res == -1 )
					{
						break;
					}
				}
			}
			results[ loadInc ].push_back ( matchCount );
		}
		resultsFile << "\n" << loadRate << ", ";
		for ( auto res : results[ loadInc ] )
		{
			resultsFile << res << ", ";
		}
	}
	auto exp = calcExpected ( atomArraySize, targetDim );
	exp = exp*exp;
	if ( attemptCount != exp && !singleMatch )
	{
		std::cout << "\nERROR!!!! expected different number of attempts... saw " << attemptCount << " but expected " << exp << "\n";
	}
	std::cout << "\nfin.\n";
	std::cin.get ( );
}


/// niawg programming timing testing...
/*
int main ( )
{
	CodeTimer timer;
	niawgPair<std::vector<UINT>> lazyPositions;
	lazyPositions[ Axes::Vertical ] = lazyPositions[ Axes::Horizontal ] = { 1,2,3,4,5,6 };
	niawgPair<std::vector<UINT>> finalLazyPositions;
	std::ofstream outFile("Results.csv");
	for ( auto ax : AXES )
	{
		auto& flp = finalLazyPositions[ ax ];
		auto& lp = lazyPositions[ ax ];
		flp.resize ( lp.size ( ) );
		for ( auto posInc : range ( lp.size ( ) ) )
		{
			flp[ posInc ] = posInc + 2;
		}
	}
	std::vector<double> vals;
	if ( false )
	{
		std::vector<double> freqs, phases, initB, finB;
		for ( auto i : range ( 10 ) )
		{
			freqs.push_back ( 11.5*i + 29 );
			phases.push_back ( 0 );
			initB.push_back ( 0.1 );
			finB.push_back ( 0.1 + 0.1*i );
		}
		timer.tick ( "Pre-Precalculate" );
		auto res = NiawgController::precalcSingleDimMoves ( freqs, phases );
		timer.tick ( "Pre-Combine" );
		for ( auto i : range ( 100 ) )
		{
			vals = NiawgController::combineIndvMoves ( lazyPositions[ Axes::Vertical ], finalLazyPositions[ Axes::Vertical ],
														initB, finB, res );
		}
		timer.tick("Post-Combine");
	}
	if ( true )
	{
		std::vector<double> times = { 1e-6,2e-6,5e-6,10e-6,20e-6, 50e-6, 100e-6, 200e-6, 500e-6, 1e-3, 2e-3 };
		for ( auto wavetime : times )
		{
			simpleWave moveWave;
			moveWave.varies = false;
			moveWave.name = "NA";
			moveWave.chan[ Axes::Vertical ].signals.resize ( 4 );
			moveWave.chan[ Axes::Horizontal ].signals.resize ( 4 );
			moveWave.time = wavetime;
			moveWave.sampleNum = NiawgController::waveformSizeCalc ( moveWave.time );
			for ( auto axis : AXES )
			{
				UINT count = 0;
				for ( auto sigInc : range ( moveWave.chan[ axis ].signals.size ( ) ) )
				{
					auto lp = lazyPositions[ axis ][ sigInc ];
					auto pos = axis == Axes::Horizontal ? lp : 10 - lp - 1;
					auto flp = finalLazyPositions[ axis ][ sigInc ];
					auto finPos = axis == Axes::Horizontal ? flp : 10 - flp - 1;
					auto& sig = moveWave.chan[ axis ].signals[ sigInc ];
					sig.freqInit = ( pos * 11.5 + 29 ) * 1e6;
					sig.freqFin = ( finPos * 11.5 + 29 ) * 1e6;
					sig.freqRampType = "lin";
					sig.initPower = 0.1;
					sig.finPower = 0.2;
					sig.powerRampType = "nr";
					sig.initPhase = 0;
					count++;
				}
			}
			timer.tick ( "Before_Generate" );
			for ( auto i : range ( 100 ) )
			{
				NiawgController::generateWaveform ( moveWave.chan[ Axes::Horizontal ], debugInfo ( ), moveWave.sampleNum,
													moveWave.time, std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4> ( ),
													false, false, false );
			}
			timer.tick ( "After_Generate" );
			std::cout << wavetime << " " << timer.getTime ( ) / 100 << "\n";
			outFile << wavetime << " " << timer.getTime ( ) / 100 << "\n";
		}
	}
	std::cout << timer.getTimingMessage ( );
	std::cin.get ( );
}
// 0.1476s for standard method
*/

