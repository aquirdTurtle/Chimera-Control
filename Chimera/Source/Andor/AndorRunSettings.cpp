// created by Mark O. Brown
#include "stdafx.h"
#include "AndorRunSettings.h"

unsigned __int64 AndorRunSettings::totalPicsInVariation ( ){
	return repetitionsPerVariation * picsPerRepetition;
}

int AndorRunSettings::totalPicsInExperiment ( ){
	return int ( totalPicsInVariation ( ) * totalVariations );
}
