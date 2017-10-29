#pragma once
#include <atomic>
#include <vector>
#include <string>
#include "windows.h"
#include "NiawgController.h"
#include "NiawgStructures.h"



struct parallelMovesContainer
{
	std::string rowOrColumn;
	int which_rowOrColumn;
	int upOrDown;
	std::vector<int> whichAtoms;
};

// should be a one-dimensional move, only change in row or column. Could probably improve the struct to reflect that.
struct simpleMove
{
	int initRow;
	int initCol;
	int finRow;
	int finCol;
};