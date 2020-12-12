// created by Mark O. Brown
#pragma once
#include "GeneralUtilityFunctions/range.h"
#include <vector>

/*
 * This object is a wrapper around a 2D vector that facilitates generating, storing, and handling multi-dimensional
 * key data for experiment runs with more than one parameter being varyied.
 */
template <class TYPE>
class multiDimensionalKey{
	public:
		multiDimensionalKey( unsigned nDimensions=1 );
		void resize( std::vector<unsigned> dimSizes );
		void setValue( std::vector<unsigned> indecies, double value );
		std::vector<TYPE> values;
	private:
		const unsigned nDims;
		std::vector<unsigned> dimensionSizes;
};

template <class TYPE> 
multiDimensionalKey<TYPE>::multiDimensionalKey( unsigned nDimensions ) : nDims( nDimensions ){
	dimensionSizes.resize( nDims );
}

template <class TYPE>
void multiDimensionalKey<TYPE>::resize( std::vector<unsigned> allSizes ){
	if ( nDims == 0 ){
		thrower ( "Tried to resize key to have zero dimensions!" );
	}
	values.clear( );
	values.resize( allSizes.size( ) );
	if ( allSizes.size( ) != nDims ){
		thrower ( "Tried to resize with size vector of the wrong size!" );
	}
	dimensionSizes = allSizes;
	// since the dimensions are varied independently, I need to multiply the individual numbers together not add 
	// them together to get the total number.
	unsigned totalVariationNumber = 1;
	for ( auto dimInc : range( dimensionSizes.size( ) ) ){
		totalVariationNumber *= dimensionSizes[dimInc];
	}
	values.clear( );
	values.resize( totalVariationNumber );
}


template <class TYPE>
void multiDimensionalKey<TYPE>::setValue( std::vector<unsigned> indecies, double value ){
	if ( indecies.size( ) != nDims ){
		thrower ( "tried to set value in multidimensional key with dimension index vector of the wrong size" );
	}	
	for ( auto inc : range( indecies.size( ) ) ){
		if ( indecies[inc] >= dimensionSizes[inc] )	{
			thrower ( "Multidimensional-key access-index out of range." );
		}
	}
	unsigned overallIndex;
	overallIndex = indecies[0];
	for ( auto dimInc : range( dimensionSizes.size( ) - 1 ) ){
		overallIndex *= dimensionSizes[dimInc + 1];
		overallIndex += indecies[dimInc + 1];
	}

	if ( overallIndex >= values.size( ) ){
		thrower ( "Overall multidimensional key vector access index out of range!!! This shouldn't happen. The"
				 " code should catch this earlier as one of the individual indecies must have been out of range or "
				 "something got set incorrectly." );
	}
	values[overallIndex] = value;
}

