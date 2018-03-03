#pragma once

#include <vector>

/// these range functions are designed to mimic the python "range" function. Stolen from some stack overflow question.
template <typename IntType> std::vector<IntType> range( IntType start, IntType stop, IntType step )
{
	if ( step == IntType( 0 ) )
	{
		throw std::invalid_argument( "step for range must be non-zero" );
	}

	std::vector<IntType> result;
	IntType inc = start;
	while ( (step > 0) ? (inc < stop) : (inc > stop) )
	{
		result.push_back( inc );
		inc += step;
	}

	return result;
}


template <typename IntType> std::vector<IntType> range( IntType start, IntType stop )
{
	return range( start, stop, IntType( 1 ) );
}

template <typename IntType> std::vector<IntType> range( IntType stop )
{
	return range( IntType( 0 ), stop, IntType( 1 ) );
}