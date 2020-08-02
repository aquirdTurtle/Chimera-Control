// created by Mark O. Brown
#include "stdafx.h"
#include "GeneralUtilityFunctions/range.h"

/* this whole little system was designed for for each loops, so that you can do, e.g.
for (auto increment : range(5)) {}
in the same way you might in python.
*/

boost::integer_range<unsigned __int64> range ( unsigned __int64 stop )
{
	return boost::irange<unsigned __int64> ( unsigned __int64 ( 0 ), stop );
}
boost::integer_range<unsigned long> range( unsigned long stop )
{
	return boost::irange<unsigned long>( unsigned long( 0 ), stop );
}

boost::integer_range<unsigned> range( unsigned stop )
{
	return boost::irange<unsigned>( unsigned( 0 ), stop );
}

boost::integer_range<int> range( int stop )
{
	return boost::irange<int>( 0, stop );
}

