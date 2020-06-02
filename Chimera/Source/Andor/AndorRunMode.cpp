// created by Mark O. Brown
#include "stdafx.h"
#include "AndorRunMode.h"

const std::array<AndorRunModes::mode, 3> AndorRunModes::allModes = { mode::Video,	
mode::Kinetic,	mode::Accumulate};

std::string AndorRunModes::toStr ( AndorRunModes::mode mode )
{
	if ( mode == AndorRunModes::mode::Kinetic )
	{
		return "Kinetic-Series-Mode";
	}
	else if ( mode == AndorRunModes::mode::Video )
	{
		return "Video-Mode";
	}
	else if ( mode == AndorRunModes::mode::Accumulate )
	{
		return "Accumulation-Mode";
	}
	else
	{
		throw;
	}
}

AndorRunModes::mode AndorRunModes::fromStr ( std::string txt )
{
	for ( auto m : AndorRunModes::allModes )
	{
		if ( txt == toStr ( m ) )
		{
			return m;
		}
	}
	thrower ("Failed to convert to andor mode from string!");
	return mode::Kinetic;
}

