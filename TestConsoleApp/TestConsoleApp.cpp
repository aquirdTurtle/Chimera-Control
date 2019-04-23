#include "stdafx.h"  
#include <iostream> 
#include <chrono> 
#include <vector> 
#include "afxwin.h" 
#include <fstream> 
#include <string> 
#include "Expression.h"
#include "codeTimer.h"
#include "thrower.h"
#include <random>
#include "multiDimensionalKey.h"
/*
#include "dlpc350_common.h"
#include "dlpc350_api.h"
#include "dlpc350_usb.h"
*/
//using namespace std;
void generateKey ( std::vector<std::vector<parameterType>>& variables, bool randomizeVariablesOption,
					std::vector<variationRangeInfo> inputRangeInfo )
{
	// get information from variables.
	for ( auto& seqVariables : variables )
	{
		for ( auto& variable : seqVariables )
		{
			variable.keyValues.clear ( );
		}
	}
	// get maximum dimension.
	UINT maxDim = 0;
	for ( auto seqVariables : variables )
	{
		for ( auto variable : seqVariables )
		{
			if ( variable.scanDimension > maxDim )
			{
				maxDim = variable.scanDimension;
			}
		}
	}
	// each element of the vector refers to the number of variations within a given variation range.
	// variations[seqNumber][dimNumber][rangeNumber]
	std::vector<std::vector<std::vector<int>>> variations ( variables.size ( ), std::vector<std::vector<int>> ( maxDim ) );
	std::vector<std::vector<int>> variableIndexes ( variables.size ( ) );
	for ( auto seqInc : range ( variables.size ( ) ) )
	{
		for ( UINT dimInc : range ( maxDim ) )
		{
			variations[ seqInc ][ dimInc ].resize ( variables[ seqInc ].front ( ).ranges.size ( ) );
			for ( UINT varInc = 0; varInc < variables[ seqInc ].size ( ); varInc++ )
			{
				auto& variable = variables[ seqInc ][ varInc ];
				if ( variable.scanDimension != dimInc + 1 )
				{
					continue;
				}
				// find a varying parameter.
				if ( variable.constant )
				{
					continue;
				}
				// then this variable varies in this dimension. 
				variableIndexes[ seqInc ].push_back ( varInc );
				// variations.size is the number of ranges currently.
				if ( variations[ seqInc ][ dimInc ].size ( ) != variable.ranges.size ( ) )
				{
					// if its zero its just the initial size on the initial variable.
					if ( variations[ seqInc ].size ( ) != 0 )
					{
						thrower ( "Not all variables seem to have the same number of ranges for their parameters!" );
					}
					variations[ seqInc ][ dimInc ].resize ( variable.ranges.size ( ) );
				}
				// make sure the variations number is consistent between
				for ( auto rangeInc : range ( variations[ seqInc ][ dimInc ].size ( ) ) )
				{
					auto& variationNum = variations[ seqInc ][ dimInc ][ rangeInc ];
					if ( variable.scanDimension != dimInc + 1 )
					{
						continue;
					}
					// can break here...?
					variationNum = inputRangeInfo[ rangeInc ].variations;
				}
			}
		}
	}
	std::vector<std::vector<UINT>> totalVariations ( variations.size ( ), std::vector<UINT> ( maxDim ) );
	for ( auto seqInc : range ( variations.size ( ) ) )
	{
		for ( auto dimInc : range ( variations[ seqInc ].size ( ) ) )
		{
			totalVariations[ seqInc ][ dimInc ] = 0;
			for ( auto variationsInRange : variations[ seqInc ][ dimInc ] )
			{
				totalVariations[ seqInc ][ dimInc ] += variationsInRange;
			}
		}
	}
	// create a key which will be randomized and then used to randomize other things the same way.
	multiDimensionalKey<int> randomizerMultiKey ( maxDim );
	randomizerMultiKey.resize ( totalVariations );
	UINT count = 0;
	for ( auto& keyElem : randomizerMultiKey.values[ 0 ] )
	{
		keyElem = count++;
	}
	if ( randomizeVariablesOption )
	{
		std::random_device rng;
		std::mt19937 twister ( rng ( ) );
		std::shuffle ( randomizerMultiKey.values[ 0 ].begin ( ), randomizerMultiKey.values[ 0 ].end ( ), twister );
		// we now have a random key for the shuffling which every variable will follow
		// initialize this to one so that constants always get at least one value.
	}
	int totalSize = 1;
	for ( auto seqInc : range ( variableIndexes.size ( ) ) )
	{
		if ( variableIndexes[ seqInc ].size ( ) == 0 && inputRangeInfo[ seqInc ].variations != 1 )
		{
			//thrower  ( "Key generator thinks that there are variations but no variables vary?!?! Low level bug"
			//		  ", this shouldn't happen." );
		}
		for ( auto variableInc : range ( variableIndexes[ seqInc ].size ( ) ) )
		{
			int varIndex = variableIndexes[ seqInc ][ variableInc ];
			auto& variable = variables[ seqInc ][ varIndex ];
			// calculate all values for a given variable
			multiDimensionalKey<double> tempKey ( maxDim ), tempKeyRandomized ( maxDim );
			tempKey.resize ( totalVariations );
			tempKeyRandomized.resize ( totalVariations );
			std::vector<UINT> rangeOffset ( totalVariations.size ( ), 0 );
			std::vector<UINT> indexes ( maxDim );
			while ( true )
			{
				UINT rangeIndex = 0, variationInc = 0;
				UINT varDim = variable.scanDimension - 1;
				UINT relevantIndex = indexes[ varDim ];
				UINT tempShrinkingIndex = relevantIndex;
				UINT rangeCount = 0, rangeOffset = 0;
				// calculate which range it is and what the index offset should be as a result.
				for ( auto range : inputRangeInfo )
				{
					if ( tempShrinkingIndex >= range.variations )
					{
						tempShrinkingIndex -= range.variations;
						rangeOffset += range.variations;
					}
					else
					{
						rangeIndex = rangeCount;
						break;
					}
					rangeCount++;
				}

				auto& currRange = variable.ranges[ rangeIndex ];
				if ( variations[ seqInc ][ varDim ][ rangeIndex ] <= 1 )
				{
					thrower ( "You need more than one variation in every range." );
				}
				// calculate the parameters for the variation range
				double valueRange = ( currRange.finalValue - currRange.initialValue );
				int spacings;

				if ( inputRangeInfo[ rangeIndex ].leftInclusive && inputRangeInfo[ rangeIndex ].rightInclusive )
				{
					spacings = variations[ seqInc ][ variables[ seqInc ][ varIndex ].scanDimension - 1 ][ rangeIndex ] - 1;
				}
				else if ( !inputRangeInfo[ rangeIndex ].leftInclusive && !inputRangeInfo[ rangeIndex ].rightInclusive )
				{
					spacings = variations[ seqInc ][ varDim ][ rangeIndex ] + 1;
				}
				else
				{
					spacings = variations[ seqInc ][ varDim ][ rangeIndex ];
				}

				double initVal;
				if ( inputRangeInfo[ rangeIndex ].leftInclusive )
				{
					initVal = currRange.initialValue;
				}
				else
				{
					initVal = currRange.initialValue + valueRange / spacings;
				}
				// calculate values.
				variationInc = indexes[ varDim ];
				double value = valueRange * ( variationInc - rangeOffset ) / spacings + initVal;
				tempKey.setValue ( indexes, seqInc, value );
				// increment. This part effectively makes this infinite while an arbitrary-dimensional loop.
				bool isAtEnd = true;
				for ( auto indexInc : range ( indexes.size ( ) ) )
				{
					// if at end of cycle for this index in this range
					if ( indexes[ indexInc ] == totalVariations[ seqInc ][ indexInc ] - 1 )
					{
						indexes[ indexInc ] = 0;
						continue;
					}
					else
					{
						indexes[ indexInc ]++;
						isAtEnd = false;
						break;
					}
				}
				if ( isAtEnd )
				{
					break;
				}
			}
			for ( auto keyInc : range ( randomizerMultiKey.values[ seqInc ].size ( ) ) )
			{
				tempKeyRandomized.values[ seqInc ][ keyInc ] = tempKey.values[ seqInc ][ randomizerMultiKey.values[ seqInc ][ keyInc ] ];
			}
			variable.keyValues = tempKeyRandomized.values[ seqInc ];
			variable.valuesVary = true;
			totalSize = tempKeyRandomized.values[ seqInc ].size ( );
		}
	}
	// now add all constant objects.
	for ( auto& seqVariables : variables )
	{
		for ( parameterType& variable : seqVariables )
		{
			if ( variable.constant )
			{
				variable.keyValues.clear ( );
				variable.keyValues.resize ( totalSize );
				for ( auto& val : variable.keyValues )
				{
					// the only constant value is stored as the initial value here.
					val = variable.constantValue;
				}
				variable.valuesVary = false;
			}
		}
	}
}

int main( )
{
	/*
	cout << DLPC350_USB_Init ( ) << endl;
	unsigned char HWStatus, SysStatus, MainStatus;
	cout << DLPC350_USB_Open ( ) << endl;
	cout << DLPC350_GetStatus ( &HWStatus, &SysStatus, &MainStatus ) << endl;
	cout << int(HWStatus) << " " << int(SysStatus) << " " << int(MainStatus) << endl;
	cout << "hello, world!";
	cin.get ( );
	*/
	CodeTimer timer;
	
	Expression expr ( "4" );
	timer.tick ( "" );
	long numReps = 1e5;
	if ( true )
	{
		for ( int inc = 0; inc < numReps; inc++ )
		{
			expr.evaluate ( );
		}
	}
	timer.tick ( "Simple-Eval" );
	std::cout << timer.getTime ( ) / numReps << "ms Per Simple Rep\n";
	
	std::vector<std::vector<parameterType>> params(1);
	parameterType param;
	param.name = "testVar";
	param.constant = true;
	param.constantValue = 5;
	params[0].push_back ( param );
	
	variationRangeInfo rInfo;
	rInfo.leftInclusive = false;
	rInfo.variations = 10;
	rInfo.rightInclusive = false;
	std::vector<variationRangeInfo> inputRangeInfo;
	inputRangeInfo.push_back ( rInfo );

	generateKey ( params, false, inputRangeInfo );
	Expression expr2 ( "testVar" );
	numReps = 1e4;
	try
	{
		for ( int inc = 0; inc < numReps; inc++ )
		{
			expr2.evaluate ( params[0], 0 );
		}
	}
	catch ( Error& err )
	{
		std::cout << err.trace ( );
	}
	timer.tick ( "1-Var" );
	std::cout << timer.getTime ( ) / numReps << "ms Per 1-Var Rep\n";
	std::cin.get ( );
	return 0;
}
 

