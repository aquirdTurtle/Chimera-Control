// created by Mark O. Brown
#include "stdafx.h"
#include "rerngOptionStructures.h"

const std::array<rerngMode::mode, 4> rerngMode::allModes = { mode::StandardFlashing, mode::Ultrafast, mode::Antoine, mode::Lazy };

std::string rerngMode::toStr ( mode m )
{
	switch ( m )
	{
		case mode::StandardFlashing:
			return "StandardFlashing";
		case mode::Ultrafast:
			return "Ultrafast";
		case mode::Antoine:
			return "Antoine";
		case mode::Lazy:
			return "Lazy";
	}
	return "";
}


rerngMode::mode rerngMode::fromStr ( std::string txt )
{
	for ( auto m : allModes )
	{
		if ( toStr ( m ) == txt )
		{
			return m;
		}
	}
	return mode::Lazy;
}


