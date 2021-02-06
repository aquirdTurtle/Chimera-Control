// created by Mark O. Brown
#pragma once

#include "boost/container/vector.hpp"
#include <vector>
#include "GeneralUtilityFunctions/range.h"
#include "GeneralUtilityFunctions/my_str.h"
#include <qmetatype.h>
#include <qpoint.h>

// a wrapper around a 1D matrix that allows for 2D (row/collum) access styles. Mostly used to represent images.
template <class type>
class Matrix {
	public:
		Matrix( unsigned rowsInGrid=0, unsigned colsInGrid=0 );
		Matrix( unsigned rowsInGrid, unsigned colsInGrid, type initValue );
		Matrix( unsigned rowsInGrid, unsigned colsInGrid, std::vector<type> init1D );
		Matrix (unsigned rowsInGrid, unsigned colsInGrid, boost::container::vector<type> init1D);

		type operator()( unsigned row, unsigned col ) const;
		type & operator()( unsigned row, unsigned col );
		type operator()( QPoint p ) const;
		type & operator()( QPoint p );
		unsigned getRows () const;
		unsigned getCols () const;
		size_t size ( ) const;
		Matrix<type> submatrix( unsigned rowOffset, unsigned rowSubSpan, unsigned colOffset, unsigned colSubSpan );
		std::string print( );
		void updateString( );
		// typename tells the compiler that std::vector<type>::iterator will be a type.
		typename boost::container::vector<type>::iterator begin( ) { return data.begin( ); }
		typename boost::container::vector<type>::iterator end( ) { return data.end( ); }
		// need to use the boost version because the std vector notoriously doesn't do std::vector<bool> properly. 
		// vector<bool> was specialized in the standard library, a decision I think most consider to be a mistake.
		boost::container::vector<type> data;
	private:
		unsigned rows, cols;
		// the following string is only updated if in debug mode.
		std::string currMatrix;
};

template <class type>
void loadBools (Matrix<type>& matrix, std::vector<bool> init) {
	if (matrix.data.size () != init.size ()) {
		thrower ("ERROR: loadBools: bool vector not same size as underlying matrix data!");
	}
	unsigned count = 0;
	for (auto val : init) {
		matrix.data[count++] = val;
	}
};


template <class type>
void Matrix<type>::updateString () {
	currMatrix = print ();
	return;
};


template <class type>
Matrix<type> Matrix<type>::submatrix (unsigned rowOffset, unsigned rowSubSpan, unsigned colOffset, unsigned colSubSpan) {
	if (rowOffset + rowSubSpan > rows || colOffset + colSubSpan > cols) {
		thrower ("ERROR: submatrix extends beyond matrix bounds!");
	}
	Matrix<type> subM (0, 0);
	// might be faster to use insert.
	for (auto rowInc : range (rowSubSpan)) {
		subM.data.insert (subM.data.end (), data.begin () + (rowOffset + rowInc) * cols + colOffset,
			data.begin () + (rowOffset + rowInc) * cols + colOffset + colSubSpan);
	}
	subM.rows = rowSubSpan;
	subM.cols = colSubSpan;
	return subM;
};


template <class type>
std::string Matrix<type>::print () {
	std::string printStr;
	unsigned counter = 0;
	for (auto rowInc : range (getRows ())) {
		for (auto colInc : range (getCols ())) {
			printStr += str ((*this)(getRows () - rowInc - 1, colInc)) + ", ";
		}
		printStr += ";\n";
	}
	return printStr;
};


// the array gets sized only once in the constructor.
template<class type>
Matrix<type>::Matrix( unsigned rowsInGrid, unsigned colsInGrid ) :
	rows( rowsInGrid ),
	cols( colsInGrid ),
	data( rowsInGrid*colsInGrid )
{}


template<class type>
Matrix<type>::Matrix( unsigned rowsInGrid, unsigned colsInGrid, type initValue ) :
	rows( rowsInGrid ),
	cols( colsInGrid ),
	data( rowsInGrid*colsInGrid, initValue )
{}


template<class type>
Matrix<type>::Matrix( unsigned rowsInGrid, unsigned colsInGrid, std::vector<type> init1D ) :
	rows( rowsInGrid ),
	cols( colsInGrid ),
	data( init1D.begin(), init1D.end() ){
	if ( data.size( ) != rows * cols ){
		thrower ( "ERROR: Initialized matrix with 1d vector whose size did not match the initializing row and column #."
				 "Lengths were: " + str( data.size( ) ) + ", " + str( rows ) + ", and " + str( cols )
				 + " respectively." );
	}
}

template<class type>
Matrix<type>::Matrix (unsigned rowsInGrid, unsigned colsInGrid, boost::container::vector<type> init1D) :
	rows (rowsInGrid),
	cols (colsInGrid),
	data (init1D.begin (), init1D.end ()) {
	if (data.size () != rows * cols) {
		thrower ("ERROR: Initialized matrix with 1d vector whose size did not match the initializing row and column #."
			"Lengths were: " + str (data.size ()) + ", " + str (rows) + ", and " + str (cols)
			+ " respectively.");
	}
}

template <class type>
unsigned Matrix<type>::getCols( ) const{
	return cols;
}


template <class type>
unsigned Matrix<type>::getRows( ) const{
	return rows;
}

template <class type>
size_t Matrix<type>::size ( ) const{
	return data.size ( );
}


template<class type>
type Matrix<type>::operator()( QPoint p ) const{
	if ( p.y () >= rows ){
		thrower ( "ERROR: row index out of range during Matrix access! row was " + str ( p.y ()) );
	}
	if ( p.x () >= cols ){
		thrower ( "ERROR: col index out of range during Matrix access! col was " + str ( p.x ()) );
	}

	unsigned rowOffset ( p.y () * cols );
	unsigned index = rowOffset + p.x ();
	return data[ index ];
}

template<class type>
type & Matrix<type>::operator()( QPoint p ){
	if ( p.y() >= rows ){
		thrower ( "ERROR: row index out of range during Matrix access! row was " + str ( p.y() ) );
	}
	if ( p.x() >= cols ){
		thrower ( "ERROR: col index out of range during Matrix access! col was " + str ( p.x ()) );
	}
	unsigned rowOffset ( p.y () * cols );
	unsigned index = rowOffset + p.x ();
	return data[ index ];
}

template<class type>
type Matrix<type>::operator()( unsigned row, unsigned col ) const{
	if ( row >= rows ){
		thrower ( "ERROR: row index out of range during Matrix access! row was " + str ( row ) );
	}
	if ( col >= cols ){
		thrower ( "ERROR: col index out of range during Matrix access! col was " + str ( col ) );
	}
	unsigned rowOffset ( row * cols );
	unsigned index = rowOffset + col;
	return data[ index ];
}

template<class type>
type & Matrix<type>::operator()( unsigned row, unsigned col ){
	if ( row >= rows ){
		thrower ( "ERROR: row index out of range during Matrix access! row was " + str ( row ) );
	}
	if ( col >= cols ){
		thrower ( "ERROR: col index out of range during Matrix access! col was " + str ( col ) );
	}
	unsigned rowOffset ( row * cols );
	unsigned index = rowOffset + col;
	return data[ index ];
}

Q_DECLARE_METATYPE(Matrix<long>)
