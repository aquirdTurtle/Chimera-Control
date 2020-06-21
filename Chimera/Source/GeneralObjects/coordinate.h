// created by Mark O. Brown
#pragma once

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
	coordinate::coordinate( unsigned long r, unsigned long c )
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

	unsigned long row;
	unsigned long column;
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

