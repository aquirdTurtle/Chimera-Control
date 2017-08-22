#include "stdafx.h"

#include <string>

/*
 * This function opens up the dialog for selecting an existing or new file name for saving scripts and configurations. It uses GetSaveFileName.
 * hwndOwner is the handle for the owner window of the new dialog
 * extension is the text of the default extension for saving this time.
 * returns the name that was selected. If no name is selected, this returns "".
 */
std::string saveWithExplorer(CWnd* owner, std::string extension, profileSettings location)
{
	//Save Dialog
	OPENFILENAME saveFileDialogInfoObj;
	char szSaveFileName[MAX_PATH];
	szSaveFileName[0] = '\0';
	ZeroMemory(&saveFileDialogInfoObj, sizeof(saveFileDialogInfoObj));
	
	saveFileDialogInfoObj.lpstrInitialDir = (LPTSTR)cstr(location.categoryPath);
	saveFileDialogInfoObj.lStructSize = sizeof(saveFileDialogInfoObj);
	saveFileDialogInfoObj.hwndOwner = owner->GetSafeHwnd();
	saveFileDialogInfoObj.lpstrFilter = NULL;
	saveFileDialogInfoObj.lpstrFile = szSaveFileName;
	saveFileDialogInfoObj.nMaxFile = MAX_PATH;
	saveFileDialogInfoObj.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	saveFileDialogInfoObj.lpstrDefExt = cstr(extension);
	if (GetSaveFileName(&saveFileDialogInfoObj) == 0)
	{
		errBox("No save file name selected");
		return "";
	}
	return szSaveFileName;
}

