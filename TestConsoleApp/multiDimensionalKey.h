// created by Mark O. Brown
#pragma once
#include "range.h"
#include "Thrower.h"
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
		multiDimensionalKey( UINT nDimensions );
		multiDimensionalKey( );
		void resize( std::vector<std::vector<UINT>> dimSizes );
		double getValue( std::vector<UINT> indecies );
		void setValue( std::vector<UINT> indecies, UINT seqNum, double value );
		std::vector<std::vector<TYPE>> values;
	private:
		const UINT nDims;
		std::vector<UINT> dimensions;
};


template <class TYPE>
multiDimensionalKey<TYPE>::multiDimensionalKey( ) : nDims( 1 )
{
	dimensions.resize( nDims );
}

template <class TYPE> 
multiDimensionalKey<TYPE>::multiDimensionalKey( UINT nDimensions ) : nDims( nDimensions )
{
	dimensions.resize( nDimensions );
}

template <class TYPE>
void multiDimensionalKey<TYPE>::resize( std::vector<std::vector<UINT>> dimSizes )
{
	UINT seqInc = 0;
	if ( nDims == 0 )
	{
		thrower ( "ERROR: tried to resize key to have zero dimensions!" );
	}
	values.clear( );
	values.resize( dimSizes.size( ) );
	for ( auto& seqSize : dimSizes )
	{
		if ( seqSize.size( ) != nDims )
		{
			thrower ( "ERROR: tried to resize with size vector of the wrong size!" );
		}
		dimensions = seqSize;
		UINT totalSize = dimensions[0];
		for ( auto dimInc : range( dimensions.size( ) - 1 ) )
		{
			totalSize *= dimensions[dimInc + 1];
		}
		values[seqInc].clear( );
		values[seqInc].resize( totalSize );
		seqInc++;
	}
}

template <class TYPE>
double multiDimensionalKey<TYPE>::getValue( std::vector<UINT> indecies )
{
	if ( indecies.size( ) != nDims )
	{
		thrower ( "tried to get value from multidimensional key with size vector of the wrong size" );
	}
	UINT overallIndex;
	for ( auto inc : range( indecies.size( ) ) )
	{
		if ( indecies[inc] >= dimensions[inc] )
		{
			thrower ( "ERROR: multidimensional key vector access index out of range." );
		}
	}
	overallIndex = indecies[0];
	for ( auto dimInc : range( dimensions.size( ) - 1 ) )
	{
		overallIndex *= dimensions[dimInc + 1];
		overallIndex += indecies[dimInc + 1];
	}

	if ( overallIndex >= values.size( ) )
	{
		thrower ( "ERROR: overall multidimensional key vector access index out of range!!! This shouldn't happen. The"
				 " code should catch this earlier as one of the individual indecies must have been out of range or "
				 "something got set incorrectly." );
	}
	return values[overallIndex];
}

template <class TYPE>
void multiDimensionalKey<TYPE>::setValue( std::vector<UINT> indecies, UINT seqNum, double value )
{
	if ( indecies.size( ) != nDims )
	{
		thrower ( "tried to get value from multidimensional key with size vector of the wrong size" );
	}
	UINT overallIndex;
	for ( auto inc : range( indecies.size( ) ) )
	{
		if ( indecies[inc] >= dimensions[inc] )
		{
			thrower ( "ERROR: multidimensional key vector access index out of range." );
		}
	}
	overallIndex = indecies[0];
	for ( auto dimInc : range( dimensions.size( ) - 1 ) )
	{
		overallIndex *= dimensions[dimInc + 1];
		overallIndex += indecies[dimInc + 1];
	}

	if ( overallIndex >= values[seqNum].size( ) )
	{
		thrower ( "ERROR: overall multidimensional key vector access index out of range!!! This shouldn't happen. The"
				 " code should catch this earlier as one of the individual indecies must have been out of range or "
				 "something got set incorrectly." );
	}
	values[seqNum][overallIndex] = value;
}

