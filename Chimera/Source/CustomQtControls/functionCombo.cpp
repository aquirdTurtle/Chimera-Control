// created by Mark O. Brown
#include "stdafx.h"
#include "functionCombo.h"
#include "ConfigurationSystems/ProfileSystem.h"

void functionCombo::loadFunctions( )
{
	ProfileSystem::reloadCombo( combo, functionLocation, str( "*." )
								+ FUNCTION_EXTENSION, "__NONE__" );
	combo->insertItem (0, "Parent Script");
}

