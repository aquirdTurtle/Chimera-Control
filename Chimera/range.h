// created by Mark O. Brown
#pragma once
#include "afxwin.h"
#include <vector>
#include <boost\range\irange.hpp>

boost::integer_range<ULONGLONG> range ( ULONGLONG stop );
boost::integer_range<ULONG> range( ULONG stop );
boost::integer_range<UINT> range( UINT stop );
boost::integer_range<int> range( int stop );
