// created by Mark O. Brown
#pragma once
#include <atomic>
#include <vector>
#include <string>
#include "NIAWG/NiawgStructures.h"
#include "GeneralObjects/coordinate.h"
#include "directions.h"


// used for pre-writing rearrangement moves...
struct rerngMove{
	unsigned row;
	unsigned col;
	dir direction;
	double initFreq = 0;
	double finFreq = 0;
	/// all of the following can (in principle) be individually calibrated.
	double moveBias;
	// in ms
	double moveTime = 60e-3;
	// in MHz
	double flashingFreq = 1e6;
	// in ns
	double deadTime = 0;
	// the static / moving ratio.
	double staticMovingRatio = 1;
	// the actual wave, ready for pushing to the niawg.
	std::vector<double> waveVals;
};


// abstracted moves used by the min cost matching algorithm and the 
struct complexMove{
	complexMove( ) {}
	complexMove( dir direction ){
		moveDir = direction;
		locationsToMove.clear( );
	}
	bool isInlineParallel = false;
	dir moveDir;
	std::vector<int_coordinate> locationsToMove;
	bool needsFlash;
	int dirInt( ){
		if ( moveDir == dir::right || moveDir == dir::left ){
			return (moveDir == dir::right) ? 1 : -1;
		}
		else{
			return (moveDir == dir::up) ? 1 : -1;
		}
	}
};;


// should be a one-dimensional move, only change in row or column. Could probably improve the struct to reflect that.
struct simpleMove{
	simpleMove( int irow, int icol, int frow, int fcol ){
		initRow = irow;
		initCol = icol;
		finRow = frow;
		finCol = fcol;
		distanceToTarget = -1;
	}
	simpleMove ( int irow, int icol, int frow, int fcol, double d )	{
		initRow = irow;
		initCol = icol;
		finRow = frow;
		finCol = fcol;
		distanceToTarget = d;
	}
	bool operator == ( const simpleMove & other ) const	{
		// two moves are equal if all members are equal.
		return( initRow == other.initRow &&
				initCol == other.initCol &&
				finRow  == other.finRow  &&
				finCol  == other.finCol);
	}
	unsigned long initRow;
	unsigned long initCol;
	unsigned long finRow;
	unsigned long finCol;
	double distanceToTarget;
	dir dir( )	{
		if ( finCol != initCol ){
			return (finCol > initCol) ? dir::right : dir::left;
		}
		else{
			return (finRow > initRow) ? dir::up : dir::down;
		}
	}
	int dirInt( ){
		if ( finCol != initCol ){
			return (finCol > initCol) ? 1 : -1;
		}
		else{
			return (finRow > initRow) ? 1 : -1;
		}
	}
	
	int movingIndex(){
		if ( dir( ) == dir::up || dir( ) == dir::down )	{
			return initRow;
		}
		else{
			return initCol;
		}
	}
	int staticIndex( ){
		if ( dir( ) == dir::up || dir( ) == dir::down ){
			return initCol;
		}
		else{
			return initRow;
		}
	}
};
