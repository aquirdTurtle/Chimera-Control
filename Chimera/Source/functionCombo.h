// created by Mark O. Brown
#pragma once
#include "constants.h"
#include "Control.h"
#include "afxWin.h"

/*
 * A small (at the moment) wrapper for combos meant to list all the functions. This exists because there are several 
 * different places where this combo is wanted.
 */
class functionCombo : public Control<CComboBox>
{
	public:
		void loadFunctions( );
	private:
		std::string functionLocation = FUNCTIONS_FOLDER_LOCATION;
};

