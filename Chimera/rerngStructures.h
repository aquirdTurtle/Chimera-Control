#pragma once
#include <atomic>
#include <vector>
#include <string>
#include "windows.h"
#include "NiawgController.h"
#include "NiawgStructures.h"



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

// should be a one-dimensional move, only change in row or column. Could probably improve the struct to reflect that.
struct simpleMove
{
	bool operator == ( const simpleMove & other ) const
	{
		// two moves are equal if all members are equal.
		return( initRow == other.initRow &&
				initCol == other.initCol &&
				finRow  == other.finRow  &&
				finCol  == other.finCol);
	}
	int initRow;
	int initCol;
	int finRow;
	int finCol;
};