
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
#include "Rearrangement.h"
#include "windows.h"

// should be a one-dimensional move, only change in row or column. Could probably improve the struct to reflect that.
struct simpleMove
{
	bool operator == ( const simpleMove & other ) const
	{
		// two moves are equal if all members are equal.
		return(initRow == other.initRow &&
				initCol == other.initCol &&
				finRow == other.finRow  &&
				finCol == other.finCol);
	}
	int initRow;
	int initCol;
	int finRow;
	int finCol;
};


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
#ifdef _DEBUG
	currMatrix = print( );
#endif
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
		throw;
		//thrower( "ERROR: row index out of range during Matrix access!" );
	}
	if ( col >= cols )
	{
		throw;
		//thrower( "ERROR: col index out of range during Matrix access!" );
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

struct complexMove
{
	complexMove( ) {}
	complexMove( std::string rowColumn, int dimNum, int dir )
	{
		rowOrColumn = rowColumn;
		whichRowOrColumn = dimNum;
		direction = dir;
	}
	std::string rowOrColumn;
	int whichRowOrColumn;
	int direction;
	bool needsFlash;
	std::vector<int> whichAtoms;
};;



enum class smartRerngOption
{
	// none places target in center.
	none,
	// find the optimal convolution match and use that to do the rearrangement algorithm
	convolution,
	// do the full optimization algorithm for all possibilities and use the one with the least moves
	full
};


enum class nonFlashingOption
{
	// no non-flashing moves
	none,
	// non-flashing moves only if no atoms in new row.
	cautious,
	full
};


// the reason to do partial is that I may run into power issues where I don't have enough power in the moving tweezers
// to spread over more than a certain number of atoms.
const UINT PARTIAL_PARALLEL_LIMIT = 2;
enum class parallelMoveOption
{
	// no parallel moves
	none,
	// parallel moves, but only two allowed to be combined at once.
	partial,
	// as many as possible.
	full
};


struct rerngOptions
{
	bool active = false;
	double moveSpeed = 60e-6;
	double flashingRate = 1e6;
	double moveBias = 1;
	// 
	double deadTime = 0;
	// the static / moving time ratio, i.e. if 2 this might imply out of a total 60us move, the static part takes up
	// 40us and the moving part takes up 20us.
	double staticMovingRatio = 1;
	bool outputInfo = false;
	bool outputIndv = false;
	bool preprogram = false;
	bool useCalibration = false;
	double finalMoveTime = 1e-3;
	// hard set right now.
	nonFlashingOption noFlashOption = nonFlashingOption::none;
	parallelMoveOption parallel = parallelMoveOption::none;
	smartRerngOption smartOption = smartRerngOption::full;
};


void optimizeMoves( std::vector<simpleMove> singleMoves, Matrix<bool> origSource, std::vector<complexMove> &moves, 
					rerngOptions options )
{
	if ( options.parallel == parallelMoveOption::none && options.noFlashOption == nonFlashingOption::none )
	{
		// no optimizations.
		return;
	}
	Matrix<bool> runningSource = origSource;
	while ( singleMoves.size( ) != 0 )
	{
		std::vector<std::vector<int>> dimLoops = { range( (int)origSource.getRows( ) ), range( (int)origSource.getRows( ), 0, -1 ),
			range( (int)origSource.getCols( ) ), range( (int)origSource.getCols( ), 0, -1 ) };
		std::vector<UINT> altSize = { origSource.getCols( ), origSource.getCols( ), origSource.getRows( ), origSource.getRows( ) };
		std::vector<std::string> directions = { "row", "row", "column", "column" };
		std::vector<int> offsets = { 1, -1, 1, -1 };
		for ( auto dim : range( dimLoops.size( ) ) )
		{
			for ( auto dimInc : dimLoops[dim] )
			{
				std::vector<int> moveIndexes;
				std::vector<simpleMove> moveList;
				for ( auto moveInc : range( singleMoves.size( ) ) )
				{
					if ( (options.parallel == parallelMoveOption::partial && moveList.size( ) == PARTIAL_PARALLEL_LIMIT)
						 || (options.parallel == parallelMoveOption::none && moveList.size( ) == 1) )
					{
						// already have all the moves we want for combining.
						break;
					}
					simpleMove& move = singleMoves[moveInc];
					int init = directions[dim] == "row" ? move.initRow : move.initCol;
					int fin = directions[dim] == "row" ? move.finRow : move.finCol;
					if ( init == dimInc && fin == dimInc + offsets[dim] )
					{
						// avoid repeats by checking iff singleMoves is in moveList first
						if ( std::find( moveList.begin( ), moveList.end( ), move ) == moveList.end( ) )
						{
							moveIndexes.push_back( moveInc );
							moveList.push_back( move );
						}
					}
				}
				if ( moveIndexes.size( ) == 0 )
				{
					// no moves in this row in this direction.
					continue;
				}
				// From the moves that go from dim to dim+offset, get which have atom at initial position and have no 
				// atom at the final position
				for ( unsigned k = moveIndexes.size( ); k-- > 0; )
				{
					auto& move = singleMoves[moveIndexes[k]];
					// check that initial spot has atom & final spot is free
					if ( !(runningSource( move.initRow, move.initCol ) && !runningSource( move.finRow, move.finCol )) )
					{
						// can't move this one, remove from list.
						moveIndexes.erase( moveIndexes.begin( ) + k );
						moveList.erase( moveList.begin( ) + k );
					}
				}
				if ( moveList.size( ) == 0 )
				{
					// couldn't move any atoms.
					continue;
				}
				moves.push_back( complexMove( directions[dim], dimInc, offsets[dim] ) );
				/// create complex move objs
				Matrix<bool> tmpSource = runningSource;
				for (auto indexNumber : range(moveIndexes.size()))
				{
					// offset from moveIndexes is the # of moves already erased.
					UINT moveIndex = moveIndexes[indexNumber] - indexNumber;
					auto& move = singleMoves[moveIndex];
					moves.back( ).whichAtoms.push_back( directions[dim] == "row" ? move.initCol : move.initRow );
					// update source image with new configuration.
					tmpSource( move.initRow, move.initCol ) = false;
					tmpSource( move.finRow, move.finCol ) = true;
					singleMoves.erase( singleMoves.begin() + moveIndex);
				}
				moves.back( ).needsFlash = false;
				/// determine if flashing is needed for this move.
				// loop through all locations in the row/collumn
				for ( auto location : range( altSize[dim] ) )
				{
					UINT initRow, initCol, finRow, finCol, which;
					bool isRow = directions[dim] == "row";
					initRow = isRow ? dimInc : location;
					initCol = isRow ? location : dimInc;
					finRow = initRow + isRow*offsets[dim];
					finCol = initCol + (!isRow)*offsets[dim];
					// if atom in location and location not being moved, always need to flash to not move this atom.
					if ( runningSource( initRow, initCol ) && std::find( moves.back( ).whichAtoms.begin( ), 
																		 moves.back( ).whichAtoms.end( ), location )
						 == moves.back( ).whichAtoms.end( ) )
					{
						moves.back( ).needsFlash = true;
					}
					// if being cautious...
					if ( runningSource( finRow, finCol ) )
					{
						moves.back( ).needsFlash = true;
					}
				}
				//
				runningSource = tmpSource;
			}
		}
	}
}

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


// for visualization purposes. note that the returned vector will be one longer than the number of moves because it
// includes the original image.
std::vector<std::string> evolveSource( Matrix<bool> source, std::vector<complexMove> moves )
{
	std::vector<std::string> images;
	images.push_back( source.print( ) );
	for ( auto move : moves )
	{
		for ( auto loc : move.whichAtoms )
		{
			UINT initRow, initCol, finRow, finCol;
			initRow = move.rowOrColumn == "row" ? move.whichRowOrColumn : loc;
			initCol = move.rowOrColumn == "row" ? loc : move.whichRowOrColumn;
			finRow = initRow + (move.rowOrColumn == "row") * move.direction;
			finCol = initCol + !(move.rowOrColumn == "row") * move.direction;
			if ( !source( initRow, initCol ) )
			{
				throw;
			}
			// potentially could move a blank...
			source( finRow, finCol ) = source( initRow, initCol );
			source( initRow, initCol ) = false;
		}
		images.push_back( source.print( ) );
	}
	return images;
}


int main( )
{
	try {
		//First, make Source and Target Matrix. In experiment the source will come from the evaluation of the photos, 
		// but the target has still to be made.
		//Matrix size
		const int N = 10; 
		//loading probability
		double p = 0.1; 
		//Source Matrix
		std::vector<std::vector<int> > source( N, std::vector<int>( N, 0 ) ); 
		//Number source atoms
		int numberSource = 0; 
		//initialize source matrix
		srand( 15 );
		Matrix<bool> sourceM( N, N );
		for ( int i = 0; i < N; i++ ) 
		{
			for ( int j = 0; j < N; j++ ) 
			{
				double random = double( rand( ) ) / RAND_MAX;
				if ( random < p ) 
				{
					source[i][j] = 1;
					sourceM( i, j ) = true;
				}
				else 
				{
					source[i][j] = 0;
					sourceM( i, j ) = false;
				}
			}
		}

		//initialize Target matrix
		std::vector<std::vector<int> > target( N, std::vector<int>( N, 0 ) ); 
		Matrix<bool> targetM( N, N );
		//Square
		for ( int i = ceil( N*4.0 / 10.0 ) - 1; i < floor( N*6.0 / 10.0 ); i++ ) 
		{
			for ( int j = ceil( N*4.0 / 10.0 ) - 1; j < floor( N*6.0 / 10.0 ); j++ ) 
			{
				target[i][j] = 1;
				targetM( i, j ) = 1;
			}
		}

		//Create an instance of Class Rearrangement
		Rearrangement *rearrangementobject = new Rearrangement( );

		//Get the maximal number of moves which equals to the maximal travelled
		//distance, only given the Target matrix!
		int maxmoves = rearrangementobject->nofmovesmax( target );
		std::cout << "The maximal Number of moves or the maximal travelled distance given the Target Matrix is:  " 
			<< maxmoves << std::endl;

		//Use the rearrangement algorithm. You need to make an empty operationsmatrix first, in which the algorithm will write
		//All the single moves. Stop here, if only single moves of interest
		//The returned value is the travelled distance
		std::vector<std::vector<int>> operationsmatrix;
		rearrangementobject->rearrangement( source, target, operationsmatrix );
		std::vector<simpleMove> moves( operationsmatrix.size( ) );
		UINT moveInc = 0;
		for ( auto& move : moves )
		{
			move.initRow = operationsmatrix[moveInc][0];
			move.initCol = operationsmatrix[moveInc][1];
			move.finRow = operationsmatrix[moveInc][2];
			move.finCol = operationsmatrix[moveInc][3];
			moveInc++;
		}
		// You're done if only considering single moves. Operationsmatrix is your list of moves. 
		// Now for considering parallel movements, that means more than one atom in each row and column, use the parallelmoves
		// function after the rearrangement function
		std::vector<parallelMove> giveMark; 
		rearrangementobject->parallelmoves( operationsmatrix, source, N, giveMark );
		std::vector<complexMove> movesNew;
		rerngOptions op;
		op.parallel = parallelMoveOption::partial;
		op.noFlashOption = nonFlashingOption::full;
		optimizeMoves( moves, sourceM, movesNew, op );
		std::cout << "--------------Parallel Moves--------------------" << std::endl;
		std::cout << "Target:\n" << targetM.print( );
		std::cout << "Source: \n" << sourceM.print();
		std::cout << "Kai's original algorithm: " << giveMark.size() << "moves. " << std::endl;
		for ( int i = 0; i < giveMark.size( ); i++ )
		{
			std::cout << giveMark[i].rowOrColumn << ", " << giveMark[i].whichRowOrColumn << " direction: "
				<< giveMark[i].upOrDown << ", atoms:";
			std::cout << "{ ";
			for ( int j = 0; j < giveMark[i].whichAtoms.size( ); j++ ) 
			{
				std::cout << giveMark[i].whichAtoms[j] << ", ";
			}
			std::cout << "} " << std::endl;
		}
		std::vector<std::string> states = evolveSource( sourceM, movesNew );
		std::cout << "\n\nComplex Moves: " << movesNew.size() << " moves.\n";
		std::cout << states[0];
		moveInc = 0;
		for ( auto move : movesNew)
		{
			std::cout << move.rowOrColumn << ", " << move.whichRowOrColumn << " direction: "
					  << move.direction << ", needs flash: " << str(move.needsFlash) << ", atoms:";
			std::cout << "{ ";
			for ( int j = 0; j < move.whichAtoms.size( ); j++ )
			{
				std::cout << move.whichAtoms[j] << ", ";
			}
			std::cout << "} " << std::endl;
			std::cout << states[moveInc+1];
			moveInc++;
		}
	}
	catch ( std::exception& e )
	{
		std::cout << e.what( ) << '\n';
	}
	std::cin.get( );
	return 0;
}


