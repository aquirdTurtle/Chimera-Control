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

struct double_coordinate
{
	double_coordinate::double_coordinate ( double r, double c )
	{
		row = r;
		column = c;
	}

	double_coordinate::double_coordinate ( )
	{
		row = 0;
		column = 0;
	}

	bool operator==( const double_coordinate& coordinate2 )
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	double row;
	double column;
};

