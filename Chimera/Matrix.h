#pragma once
#include <boost/container/vector.hpp>
#include "windows.h"

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
		Matrix<type> submatrix( UINT rowOffset, UINT rowSubSpan, UINT colOffset, UINT colSubSpan );
		std::string print( );
		void updateString( );
		// typename tells the compiler that std::vector<type>::iterator will be a type.
		typename boost::container::vector<type>::iterator begin( ) { return data.begin( ); }
		typename boost::container::vector<type>::iterator end( ) { return data.end( ); }
	private:
		// need to use the boost version because the std version doesn't do std::vector<bool> properly.
		boost::container::vector<type> data;
		UINT rows, cols;
		// the following string is only updated if in debug mode.
		std::string currMatrix;
};

template <class type>
void Matrix<type>::updateString( )
{
//#ifdef _DEBUG
	currMatrix = print( );
//#endif
	return;
}


template <class type>
Matrix<type> Matrix<type>::submatrix( UINT rowOffset, UINT rowSubSpan, UINT colOffset, UINT colSubSpan )
{
	if ( rowOffset + rowSubSpan > rows || colOffset + colSubSpan > cols )
	{
		thrower( "ERROR: submatrix extends beyond matrix bounds!" );
	}
	Matrix<type> subM( 0, 0 );
	// might be faster to use insert.
	for ( auto rowInc : range( rowSubSpan ) )
	{
		subM.data.insert( subM.data.end( ), data.begin( ) + (rowOffset + rowInc) * cols + colOffset,
						  data.begin( ) + (rowOffset + rowInc) * cols + colOffset + colSubSpan );
	}
	subM.rows = rowSubSpan;
	subM.cols = colSubSpan;
	updateString( );
	return subM;
}


template <class type>
std::string Matrix<type>::print( )
{
	std::string printStr;
	UINT counter = 0;
	for ( auto elem : *this )
	{
		printStr += str( elem ) + ", ";
		if ( ++counter % cols == 0 )
		{
			printStr += ";\n";
		}
	}
	return printStr;
}


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
		thrower( "ERROR: row index out of range during Matrix access!" );
	}
	if ( col > cols )
	{
		thrower( "ERROR: col index out of range during Matrix access!" );
	}
	UINT rowOffset( row * cols );
	UINT index = rowOffset + col;
	updateString( );
	return data[index];
}

template<class type>
type & Matrix<type>::operator()( UINT row, UINT col )
{
	if ( row >= rows )
	{
		thrower( "ERROR: row index out of range during Matrix access!" );
	}
	if ( col >= cols )
	{
		thrower( "ERROR: col index out of range during Matrix access!" );
	}
	UINT rowOffset( row * cols );
	UINT index = rowOffset + col;
	updateString( );
	return data[index];
}


template <class type>
UINT Matrix<type>::getCols( )
{
	updateString( );
	return cols;
}


template <class type>
UINT Matrix<type>::getRows( )
{
	updateString( );
	return rows;
}

