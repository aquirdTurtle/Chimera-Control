// created by Mark O. Brown
#include "stdafx.h"
#include "SegmentEnd.h"

const std::array<SegmentEnd::type, 5> SegmentEnd::allTypes = { type::repeat, type::repeatTilTrig, type::once, 
															   type::repeatInf, type::onceWaitTrig };

bool SegmentEnd::invovlesTrig ( SegmentEnd::type t )
{
	return t == type::repeatTilTrig || t == type::onceWaitTrig;
}

std::string SegmentEnd::toStr (SegmentEnd::type t )
{
	switch ( t )
	{
		case type::repeat:
			return "repeat";
		case type::repeatTilTrig:
			return "repeattiltrig";
		case type::once:
			return "once";
		case type::repeatInf:
			return "repeatinf";
		case type::onceWaitTrig:
			return "oncewaittrig";
		default:
			thrower ( "Failed to convert SegmentEnd to string???" );
	}
}

SegmentEnd::type SegmentEnd::fromStr ( std::string txt )
{
	for ( auto t : SegmentEnd::allTypes )
	{
		if ( txt == SegmentEnd::toStr ( t ) )
		{
			return t;
		}
	}
	thrower ( "Failed to convert string to SegmentEnd for agilent!" );
}

