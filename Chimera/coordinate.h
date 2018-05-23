#pragma once
#include "windows.h"


struct int_coordinate
{
	int_coordinate::int_coordinate(int r, int c)
	{
		row = r;
		column = c;
	}

	int_coordinate::int_coordinate()
	{
		row = 0;
		column = 0;
	}

	bool int_coordinate::operator==(const int_coordinate& coordinate2)
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	int row;
	int column;
};


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

	bool coordinate::operator==( const coordinate& coordinate2 )
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	ULONG row;
	ULONG column;
};
