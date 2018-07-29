#include "stdafx.h"
#include "Thrower.h"
#include "AndorTriggerModes.h"

std::string AndorTriggerModeText ( AndorTriggerMode mode )
{
	if ( mode == AndorTriggerMode::External )
	{
		return "External-Trigger";
	}
	else if ( mode == AndorTriggerMode::Internal )
	{
		return "Internal-Trigger";
	}
	else if ( mode == AndorTriggerMode::StartOnTrigger )
	{
		return "Start-On-Trigger";
	}
	else
	{
		thrower("ERROR: AndorTriggerMode not recognized?!");
	}
}

AndorTriggerMode AndorTriggerModeFromText ( std::string txt )
{
	if ( txt == "External-Trigger" )
	{
		return AndorTriggerMode::External;
	}
	else if ( txt == "Internal-Trigger" )
	{
		return AndorTriggerMode::Internal;
	}
	else if ( txt == "Start-On-Trigger" )
	{
		return AndorTriggerMode::StartOnTrigger;
	}
	else
	{
		thrower("ERROR: andortriggermode not recognized?!?");
	}
}
