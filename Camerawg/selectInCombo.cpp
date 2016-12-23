#include "stdafx.h"
#include "selectInCombo.h"

/**
 * Find the string "stringToFind" in the combo "comboHWND", and select it. This function returns the zero-indexed location of this entry, or -1 if 
 * not found.
 */
int selectInCombo(HWND comboHWND, std::string stringToFind)
{
	// get number of entries in combo.
	int count = SendMessage(comboHWND, CB_GETCOUNT, 0, 0);
	TCHAR comboText[256];
	// loop through all entries in combo
	for (int comboInc = 0; comboInc < count; comboInc++)
	{
		// get a combo entry
		SendMessage(comboHWND, CB_GETLBTEXT, comboInc, (LPARAM)comboText);
		// check if the requested one
		if (std::string(comboText) == stringToFind)
		{
			// select it if yes
			SendMessage(comboHWND, CB_SETCURSEL, comboInc, 0);
			return comboInc;
		}
	}
	// shouldn't reach here if the string was fonud.
	return -1;
}
