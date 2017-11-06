
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
#include <boost/container/vector.hpp>

// this can replace str() and str(), as well as providing functionality to set the precision of
// to_string() conversions.
template <typename T> std::string str( T input, const int precision = 13, bool eatZeros = false, bool toLower = false )
{
	std::ostringstream out;
	out << std::setprecision( precision ) << input;
	std::string outStr = out.str( );
	if ( eatZeros )
	{
		if ( outStr.find( "." ) != std::string::npos )
		{
			outStr.erase( outStr.find_last_not_of( '0' ) + 1, std::string::npos );
		}
	}
	if ( toLower )
	{
		std::transform( outStr.begin( ), outStr.end( ), outStr.begin( ), ::tolower );
	}
	return outStr;
}



// a wrapper around a 1D matrix that allows for 2D (row/collum) access styles. Mostly used to represetn images.
template <class type>
class Matrix
{
	public:
		Matrix( UINT rowsInGrid, UINT colsInGrid );
		Matrix( UINT rowsInGrid, UINT colsInGrid, type initValue );
		type operator()( UINT row, UINT col ) const;
		type & operator()( UINT row, UINT col );
		UINT getRows( );
		UINT getCols( );
		std::string print( );
		// typename tells the compiler that std::vector<type>::iterator will be a type.
		typename boost::container::vector<type>::iterator begin( ) { return data.begin( ); }
		typename boost::container::vector<type>::iterator end( ) { return data.end( ); }
		Matrix<type> submatrixStupid( UINT rowOffset, UINT rowSubSpan, UINT colOffset, UINT colSubSpan );
		Matrix<type> submatrixSmart( UINT rowOffset, UINT rowSubSpan, UINT colOffset, UINT colSubSpan );
	private:
		boost::container::vector<type> data;
		UINT rows, cols;
};


// the array gets sized only once in the constructor.
template<class type>
Matrix<type>::Matrix( UINT rowsInGrid, UINT colsInGrid ) :
	rows( rowsInGrid ),
	cols( colsInGrid ),
	data( rowsInGrid*colsInGrid )
{}


template<class type>
Matrix<type>::Matrix( UINT rowsInGrid, UINT colsInGrid, type initValue ) :
	rows( rowsInGrid ),
	cols( colsInGrid ),
	data( rowsInGrid*colsInGrid, initValue )
{}


template<class type>
type Matrix<type>::operator()( UINT row, UINT col ) const
{
if ( row > rows )
{
	std::cout << "ERROR: row index out of range during rearrangementMoveContainer access!";
}
if ( col > cols )
{
	std::cout << "ERROR: col index out of range during rearrangementMoveContainer access!";
}
UINT rowOffset( row * cols );
UINT index = rowOffset + col;
return data[index];
}

template<class type>
type & Matrix<type>::operator()( UINT row, UINT col )
{
	if ( row > rows )
	{
		std::cout << "ERROR: row index out of range during rearrangementMoveContainer access!";
	}
	if ( col > cols )
	{
		std::cout << "ERROR: col index out of range during rearrangementMoveContainer access!";
	}
	UINT rowOffset( row * cols );
	UINT index = rowOffset + col;
	return data[index];
}


template <class type>
UINT Matrix<type>::getCols( )
{
	return cols;
}


template <class type>
UINT Matrix<type>::getRows( )
{
	return rows;
}


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

template <class type>
Matrix<type> Matrix<type>::submatrixSmart( UINT rowOffset, UINT rowSubSpan, UINT colOffset, UINT colSubSpan )
{
	if ( rowOffset + rowSubSpan > rows || colOffset + colSubSpan > cols)
	{
		//thrower( "ERROR: submatrix extends beyond matrix bounds!" );
	}
	Matrix<type> subM( 0, 0 );
	// might be faster to use insert.
	for ( auto rowInc : range( rowSubSpan ) )
	{
		subM.data.insert( subM.data.end(), data.begin() + (rowOffset + rowInc) * cols + colOffset,
						  data.begin( ) + (rowOffset + rowInc) * cols + colOffset + colSubSpan);
	}
	subM.rows = rowSubSpan;
	subM.cols = colSubSpan;
	return subM;
}

template <class type>
Matrix<type> Matrix<type>::submatrixStupid( UINT rowOffset, UINT rowSubSpan, UINT colOffset, UINT colSubSpan )
{
	if ( rowOffset + rowSubSpan > rows || colOffset + colSubSpan > cols )
	{
		//thrower( "ERROR: submatrix extends beyond matrix bounds!" );
	}
	Matrix<type> subM( rowSubSpan, colSubSpan );
	// might be faster to use insert.
	for ( auto rowInc : range( subM.getRows( ) ) )
	{
		for ( auto colInc : range( subM.getCols( ) ) )
		{
			subM( rowInc, colInc ) = (*this)(rowOffset + rowInc, colOffset + colInc);
		}
	}
	return subM;
}


template <class type>
std::string Matrix<type>::print( )
{
	std::string printStr;
	UINT counter = 0;
	for ( auto elem : *this )
	{
		printStr += str(elem) + ", ";
		if ( ++counter % rows == 0 )
		{
			printStr += ";\n";
		}
	}
	return printStr;
}


int main( )
{
	Matrix<unsigned long> atoms( 4, 4, 0 );
	UINT counter = 0;
	for ( auto& atom : atoms )
	{
		atom = counter++;
	}
	UINT count = 1e6;
	std::chrono::time_point<std::chrono::steady_clock> begin = std::chrono::steady_clock::now( );
	for ( auto i : range( count ) )
	{
		Matrix<unsigned long> subAtoms = atoms.submatrixSmart( 1, 2, 1, 2 );
	}
	std::chrono::time_point<std::chrono::steady_clock> afterSmart = std::chrono::steady_clock::now( );
	for ( auto i : range( count ) )
	{
		Matrix<unsigned long> subAtoms2 = atoms.submatrixStupid( 1, 2, 1, 2 );
	}
	std::chrono::time_point<std::chrono::steady_clock> afterStupid = std::chrono::steady_clock::now( );

	std::cout << "dumb: " << std::chrono::duration<double>( afterStupid - afterSmart ).count( ) << "us\n";
	std::cout << "smart: " << std::chrono::duration<double>( afterSmart - begin ).count( ) << "us\n";
	std::cout << "\n\n\n";
	//std::cout << subAtoms.print( );
	std::cout << "\n\n\n";
	//std::cout << subAtoms2.print( );
	std::cin.get( );
	return 0;
}

