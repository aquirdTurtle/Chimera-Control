#include "stdafx.h"
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
		throw;
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
		throw;
	}
}
