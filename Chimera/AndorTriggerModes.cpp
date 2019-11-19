// created by Mark O. Brown
#include "stdafx.h"
#include "AndorTriggerModes.h"
const std::array<AndorTriggerMode::mode, 3> AndorTriggerMode::allModes = { AndorTriggerMode::mode::External,
AndorTriggerMode::mode::Internal,
AndorTriggerMode::mode::StartOnTrigger };

std::string AndorTriggerMode::toStr( AndorTriggerMode::mode m )
{
	if ( m == AndorTriggerMode::mode::External )
	{
		return "External-Trigger";
	}
	else if ( m == AndorTriggerMode::mode::Internal )
	{
		return "Internal-Trigger";
	}
	else if ( m == AndorTriggerMode::mode::StartOnTrigger )
	{
		return "Start-On-Trigger";
	}
	else
	{
		thrower ("AndorTriggerMode not recognized?!");
	}
}


AndorTriggerMode::mode AndorTriggerMode::fromStr ( std::string txt )
{
	for ( auto m : allModes )
	{
		if ( txt == toStr ( m ) )
		{
			return m;
		}
	}
	thrower ( "AndorTriggerMode string not recognized?!" );
}

