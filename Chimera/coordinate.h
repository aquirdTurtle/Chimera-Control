#pragma once
#include "windows.h"

struct coordinate
{
	coordinate::coordinate( ULONG r, ULONG c )
	{
		row = r;
		column = c;
	}

	coordinate::coordinate()
	{
		row = 0;
		column = 0;
	}

	bool operator==( const coordinate& coordinate2 )
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	ULONG row;
	ULONG column;
};
