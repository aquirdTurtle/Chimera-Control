#pragma once

#include <string>

enum class AndorTriggerMode
{
	External,
	Internal,
	StartOnTrigger,
	None=-1
};

std::string AndorTriggerModeText ( AndorTriggerMode mode );
AndorTriggerMode AndorTriggerModeFromText ( std::string txt );

