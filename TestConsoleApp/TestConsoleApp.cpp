#include "stdafx.h" 
#include <iostream>
#include <chrono>
#include "../Chimera/range.h"
#include "Windows.h"
#include <vector>
#include <array>
#include <valarray>
#include <boost/range/irange.hpp>

using namespace std;
using namespace chrono;

int main( )
{
	size_t numReps = 2000000;
	typedef steady_clock::time_point time;
	vector<long long> image( 21 * 21, 100 );
	time begin = steady_clock::now( );
	for ( auto j : boost::irange( size_t( 0 ), size_t(10) ) )
	{
		cout << j << " ";
	}
	cout << endl;
	for ( auto j : boost::irange( 0, 10 ) )
	{
		cout << j << " ";
	}


	for ( size_t i = 0; i < numReps; i++ )
	{
		for ( auto j : boost::irange(size_t(0), image.size() ))
		{
			image[j] += i;
		}
	}
	time end = steady_clock::now( );
	image = vector<long long>( 21 * 21, 100 );
	time begin2 = steady_clock::now( );
	for ( size_t i = 0; i < numReps; i++ )
	{
		for ( size_t j = 0; j < image.size( ); j++ )
		{
			image[j] += i;
		}
	}
	time end2 = steady_clock::now( );
	cout << image[10] << endl;
	cout << "Average (" << numReps << " runs) Execution Time (ms): " 
		<< duration_cast<nanoseconds>(end - begin).count( ) / (1e6 * numReps) << endl;
	cout << "Average (" << numReps << " runs) Execution Time (ms): "
		<< duration_cast<nanoseconds>(end2 - begin2).count( ) / (1e6 * numReps) << endl;

	cin.get( );
	return 0;
}

