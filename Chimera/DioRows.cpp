#include "stdafx.h"
#include "DioRows.h"

DioRows::which DioRows::fromStr ( std::string rowStr )
{
	for ( auto w : { which::A, which::B, which::C, which::D } )
	{
		if ( rowStr == toStr ( w ) )
		{
			return w;
		}
	}
}

std::string DioRows::toStr ( which m )
{
	switch ( m )
	{
		case which::A:
			return "A";
		case which::B:
			return "B";
		case which::C:
			return "C";
		case which::D:
			return "D";
	}
}