// created by Mark O. Brown
#include "stdafx.h"
#include "DoRows.h"

/*
A constant array which can be used to iterate through all values of DoRows. For example:
for (auto row : DoRows::allRows)
{
	// ...
}
*/
const std::array<DoRows::which, 4> DoRows::allRows = {which::A,which::B,which::C,which::D};


/*
Note that this function is not case-sensitive - it will always convert the input string to lowercase before testing.
*/
DoRows::which DoRows::fromStr ( std::string rowStr )
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


std::string DoRows::toStr ( which m )
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
