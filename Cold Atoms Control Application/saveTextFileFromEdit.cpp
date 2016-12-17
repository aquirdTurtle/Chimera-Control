#include "stdafx.h"

#include <string>

/*
 * This function opens up the dialog for selecting an existing or new file name for saving scripts and configurations. It uses GetSaveFileName.
 * hwndOwner is the handle for the owner window of the new dialog
 * extension is the text of the default extension for saving this time.
 * returns the name that was selected. If no name is selected, this returns "".
 */
std::string saveTextFileFromEdit(HWND hwndOwner, std::string extension, profileSettings location)
{
	//Save Dialog
	OPENFILENAME saveFileDialogInfoObj;
	char szSaveFileName[MAX_PATH];
	szSaveFileName[0] = '\0';
	ZeroMemory(&saveFileDialogInfoObj, sizeof(saveFileDialogInfoObj));
	
	saveFileDialogInfoObj.lpstrInitialDir = (LPTSTR)(location.pathIncludingCategory).c_str();
	saveFileDialogInfoObj.lStructSize = sizeof(saveFileDialogInfoObj);
	saveFileDialogInfoObj.hwndOwner = hwndOwner;
	saveFileDialogInfoObj.lpstrFilter = NULL;
	saveFileDialogInfoObj.lpstrFile = szSaveFileName;
	saveFileDialogInfoObj.nMaxFile = MAX_PATH;
	saveFileDialogInfoObj.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	saveFileDialogInfoObj.lpstrDefExt = extension.c_str();
	if (GetSaveFileName(&saveFileDialogInfoObj) == 0)
	{
		MessageBox(NULL, "No save file name selected", NULL, MB_OK);
		return "";
	}
	return szSaveFileName;
}

