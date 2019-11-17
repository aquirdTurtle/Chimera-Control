// created by Mark O. Brown
#include "stdafx.h"
#include "DioRows.h"

/*
A constant array which can be used to iterate through all values of DioRows. For example:
for (auto row : DioRows::allRows)
{
	// ...
}
*/
const std::array<DioRows::which, 4> DioRows::allRows = {which::A,which::B,which::C,which::D};


/*
Note that this function is not case-sensitive - it will always convert the input string to lowercase before testing.
*/
DioRows::which DioRows::fromStr ( std::string rowStr )
{
	for ( auto w : allRows )
	{
		if ( str(rowStr,13,false,true) == toStr ( w ) )
		{
			return w;
		}
	}
 	thrower  ( "Failed to convert string to diorow!" );
	return which::A;
}


std::string DioRows::toStr ( which m )
{
	switch ( m )
	{
		case which::A:
			return "a";
		case which::B:
			return "b";
		case which::C:
			return "c";
		case which::D:
			return "d";
	}
	thrower  ( "Faied to convert dio row to string!" );
	return "";
}
