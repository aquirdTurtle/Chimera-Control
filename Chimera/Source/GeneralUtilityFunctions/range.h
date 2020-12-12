// created by Mark O. Brown
#pragma once
#include <vector>
#include <boost\range\irange.hpp>

boost::integer_range<unsigned __int64> range ( unsigned __int64 stop );
boost::integer_range<unsigned long> range( unsigned long stop );
boost::integer_range<unsigned> range( unsigned stop );
boost::integer_range<int> range( int stop );


