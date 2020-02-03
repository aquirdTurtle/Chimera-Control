// created by Mark O. Brown
#pragma once
#include "GeneralUtilityFunctions/range.h"
#include "afxwin.h"
#include <vector>



/*
 * This object is a wrapper around a 2D vector that facilitates generating, storing, and handling multi-dimensional
 * key data for experiment runs with more than one parameter being varyied.
 */
template <class TYPE>
class multiDimensionalKey
{
	public:
		multiDimensionalKey( UINT nDimensions=1 );
		void resize( std::vector<std::vector<UINT>> dimSizes );
		void setValue( std::vector<UINT> indecies, UINT seqNum, double value );
		std::vector<std::vector<TYPE>> values;
	private:
		const UINT nDims;
		std::vector<UINT> dimensionSizes;
};

template <class TYPE> 
multiDimensionalKey<TYPE>::multiDimensionalKey( UINT nDimensions ) : nDims( nDimensions )
{
	dimensionSizes.resize( nDims );
}

template <class TYPE>
void multiDimensionalKey<TYPE>::resize( std::vector<std::vector<UINT>> allSizes )
{
	if ( nDims == 0 )
	{
		thrower ( "Tried to resize key to have zero dimensions!" );
	}
	values.clear( );
	values.resize( allSizes.size( ) );
	UINT seqInc = 0;
	for ( auto& seqSizes : allSizes )
	{
		if ( seqSizes.size( ) != nDims )
		{
			thrower ( "Tried to resize with size vector of the wrong size!" );
		}
		dimensionSizes = seqSizes;
		// since the dimensions are varied independently, I need to multiply the individual numbers together not add 
		// them together to get the total number.
		UINT totalVariationNumber = 1;
		for ( auto dimInc : range( dimensionSizes.size( ) ) )
		{
			totalVariationNumber *= dimensionSizes[dimInc];
		}
		values[seqInc].clear( );
		values[seqInc].resize( totalVariationNumber );
		seqInc++;
	}
}


template <class TYPE>
void multiDimensionalKey<TYPE>::setValue( std::vector<UINT> indecies, UINT seqNum, double value )
{
	if ( indecies.size( ) != nDims )
	{
		thrower ( "tried to set value in multidimensional key with dimension index vector of the wrong size" );
	}	
	for ( auto inc : range( indecies.size( ) ) )
	{
		if ( indecies[inc] >= dimensionSizes[inc] )
		{
			thrower ( "Multidimensional-key access-index out of range." );
		}
	}
	UINT overallIndex;
	overallIndex = indecies[0];
	for ( auto dimInc : range( dimensionSizes.size( ) - 1 ) )
	{
		overallIndex *= dimensionSizes[dimInc + 1];
		overallIndex += indecies[dimInc + 1];
	}

	if ( overallIndex >= values[seqNum].size( ) )
	{
		thrower ( "Overall multidimensional key vector access index out of range!!! This shouldn't happen. The"
				 " code should catch this earlier as one of the individual indecies must have been out of range or "
				 "something got set incorrectly." );
	}
	values[seqNum][overallIndex] = value;
}

