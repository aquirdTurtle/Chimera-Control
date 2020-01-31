// created by Mark O. Brown
#include "stdafx.h"
#include "AndorRunSettings.h"

ULONGLONG AndorRunSettings::totalPicsInVariation ( )
{
	return repetitionsPerVariation * picsPerRepetition;
}

int AndorRunSettings::totalPicsInExperiment ( )
{
	return int ( totalPicsInVariation ( ) * totalVariations );
}
