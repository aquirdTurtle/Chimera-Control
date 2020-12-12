// created by Mark O. Brown
#pragma once
#include "LowLevel/constants.h"
#include "Control.h"
#include <CustomQtControls/AutoNotifyCtrls.h>

/*
 * A small (at the moment) wrapper for combos meant to list all the functions. This exists because there are several 
 * different places where this combo is wanted.
 */
class functionCombo{
	public:
		void loadFunctions( );
		CQComboBox* combo;
	private:
		std::string functionLocation = FUNCTIONS_FOLDER_LOCATION;
};

