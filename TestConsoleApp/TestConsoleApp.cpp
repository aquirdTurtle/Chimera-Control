
#include "stdafx.h" 
#include <vector>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>
#include <functional>
#include <Windows.h>
#include <chrono>
#include <numeric>
#include <valarray>

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


std::vector<std::vector<long>> convolveDumb( std::vector<std::vector<bool>> atoms,
														  std::vector<std::vector<bool>> target )
{
	std::vector<std::vector<long>> result( atoms.size( ) - target.size( )+1,
										   std::vector<long>( atoms[0].size( ) - target[0].size( )+1 ) );
	for ( auto startRowInc : range( result.size( ) ) )
	{
		for ( auto startColInc : range( result[0].size( ) ) )
		{
			// calcualte product
			long conv = 0;
			for ( auto rowInc : range( target.size( ) ) )
			{
				for ( auto colInc : range( target[0].size( ) ) )
				{
					conv += atoms[rowInc + startRowInc][colInc + startColInc] * target[rowInc][colInc];
				}
			}
			result[startRowInc][startColInc] = conv;
			if ( conv == target.size( ) * target[0].size( ) )
			{
				// perfect match was found. 
				return result;
			}
		}
	}
	return result;
}



std::vector<std::vector<long>> convolve( std::vector<std::vector<bool>> atoms,
															std::vector<std::vector<bool>> target )
{
	std::vector<std::vector<long>> result( atoms.size( ) - target.size( ) + 1,
										   std::vector<long>( atoms[0].size( ) - target[0].size( ) + 1 ) );
	for ( auto startRowInc : range( result.size( ) ) )
	{
		for ( auto startColInc : range( result[0].size( ) ) )
		{
			// calcualte product
			long conv = 0;
			for ( auto rowInc : range( target.size( ) ) )
			{
				std::vector<long> tmp( target[0].size( ) );
				std::transform( atoms[rowInc + startRowInc].begin( ) + startColInc,
								atoms[rowInc + startRowInc].begin( ) + startColInc + target.size( ),
								target[rowInc].begin( ), tmp.begin( ), []( auto& i, auto& j ) {return long( i*j ); } );
				conv += std::accumulate( tmp.begin( ), tmp.end( ), 0 );
			}
			result[startRowInc][startColInc] = conv;
			if ( conv == target.size( ) * target[0].size( ) )
			{
				// perfect match was found. 
				return result;
			}
		}
	}
	return result;
}



template <class type> void printVec( std::vector<std::vector<type>> vecToPrint )
{
	for ( auto row : vecToPrint )
	{
		for ( auto i : row )
		{
			std::cout << i << " ";
		}
		std::cout << "\n";
	}
}




int main( )
{
	// initialize stuff
	std::uniform_int_distribution<int> dist( 0, 100 );	
	UINT iterations = 100000;
	UINT dimAtoms = 7;
	UINT dimTarget = 5;
	UINT loadingRate = 55;
	std::vector<std::vector<std::vector<bool>>> atomArrays( iterations );
	for ( auto& atoms : atomArrays )
	{
		atoms.resize( dimAtoms );
		std::vector<std::vector<int>> tmpArray( dimAtoms );
		for ( auto& rowInc : range(tmpArray.size()))
		{
			std::random_device rnd_device;
			std::mt19937 mersenne_engine( rnd_device( ) );
			auto gen = std::bind( dist, mersenne_engine );
			tmpArray[rowInc].resize( dimAtoms );
			atoms[rowInc].resize( dimAtoms );
			std::generate( tmpArray[rowInc].begin( ), tmpArray[rowInc].end( ), gen );
			std::transform( tmpArray[rowInc].begin( ), tmpArray[rowInc].end( ), atoms[rowInc].begin( ),
							[loadingRate]( int& i ) {return i / loadingRate; } );
		}
	}
	std::vector<std::vector<bool>> target( dimTarget, std::vector<bool>( dimTarget, true));
	auto start = std::chrono::steady_clock::now( );
	for ( auto array : atomArrays )
	{
		std::vector<std::vector<long>> result = convolveDumb( array, target );
	}
	auto end = std::chrono::steady_clock::now( );
	auto diff = end - start;
	std::cout << std::chrono::duration <double, std::milli>( diff ).count( ) / iterations << " ms per dumb convolution\n";
	start = std::chrono::steady_clock::now( );
	for ( auto array : atomArrays )
	{
		std::vector<std::vector<long>> result = convolve( array, target );
	}
	end = std::chrono::steady_clock::now( );
	diff = end - start;
	std::cout << std::chrono::duration <double, std::milli>( diff ).count( ) / iterations << " ms per transform / accumulat convolution\n";
	/*
	printVec( atomArrays[0] );
	std::cout << "convolved with\n";
	printVec( target );
	std::cout << "equals\n";
	printVec( result );
	*/
	std::cin.get( );
	return 0;
}
