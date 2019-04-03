// created by Mark O. Brown
#include "stdafx.h"
#include "range.h"

boost::integer_range<ULONGLONG> range ( ULONGLONG stop )
{
	return boost::irange<ULONGLONG> ( ULONGLONG ( 0 ), stop );
}
boost::integer_range<ULONG> range( ULONG stop )
{
	return boost::irange<ULONG>( ULONG( 0 ), stop );
}

boost::integer_range<UINT> range( UINT stop )
{
	return boost::irange<UINT>( UINT( 0 ), stop );
}

boost::integer_range<int> range( int stop )
{
	return boost::irange<int>( 0, stop );
}

