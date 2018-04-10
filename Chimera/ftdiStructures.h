#pragma once
#include <array>
#include "range.h"

enum class ftdiConnectionOption
{
	None,
	Serial,
	Async
};


struct ftdiPt
{
	unsigned int time;
	std::array<unsigned char, 8> pts;
	bool operator==( const ftdiPt& p )
	{
		bool pts_match = true;
		for ( auto pt_count : range( pts.size( ) ) )
		{
			if ( pts[pt_count] != p.pts[pt_count] )
			{
				pts_match = false;
			}
		}
		return (time == p.time && pts_match);
	}
};

struct finBufInfo
{
	unsigned int bytesToWrite;
	std::vector<unsigned char> pts;
};