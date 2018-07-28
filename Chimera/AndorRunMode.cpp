#include "stdafx.h"
#include "AndorRunMode.h"

std::string AndorRunModeText ( AndorRunModes mode )
{
	if ( mode == AndorRunModes::Kinetic )
	{
		return "Kinetic-Series-Mode";
	}
	else if ( mode == AndorRunModes::Video )
	{
		return "Video-Mode";
	}
	else if ( mode == AndorRunModes::Accumulate )
	{
		return "Accumulate-Mode";
	}
	else
	{
		throw;
	}
}
