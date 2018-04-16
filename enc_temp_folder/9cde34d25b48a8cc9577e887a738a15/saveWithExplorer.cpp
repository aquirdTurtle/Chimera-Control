#include "stdafx.h"
#include "saveWithExplorer.h"
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
	std::string initDir( location.categoryPath );
	saveFileDialogInfoObj.lpstrInitialDir = (LPTSTR)initDir.c_str();
	saveFileDialogInfoObj.lStructSize = sizeof(saveFileDialogInfoObj);
	saveFileDialogInfoObj.hwndOwner = owner->GetSafeHwnd();
	
	// using string literals because of the embedded \0 character.
	using namespace std::string_literals;
	std::string txt;
	if ( extension == "aScript" )
	{
		txt = "Agilent Script\0*.aScript\0\0"s;
	}
	else if ( extension == "nScript" )
	{
		txt = "Niawg Script\0*.nScript\0\0"s;
	}
	else if ( extension == "mScript" )
	{
		txt = "Master Script\0*.mScript\0"s;
	}
	else if ( extension == "Config" )
	{
		txt = "Configuraiton\0*.Config\0"s;
	}
	else if ( extension == "catConfig" )
	{
		txt = "Category Config File\0*.catConfig\0"s;
	}
	else if ( extension == "eConfig" )
	{
		txt = "Experiment Config File\0*.eConfig\0"s;
	}

	saveFileDialogInfoObj.lpstrFilter = txt.c_str();
	saveFileDialogInfoObj.lpstrFile = szSaveFileName;
	saveFileDialogInfoObj.nMaxFile = MAX_PATH;
	saveFileDialogInfoObj.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	saveFileDialogInfoObj.lpstrDefExt = extension.c_str();
	if (GetSaveFileName(&saveFileDialogInfoObj) == 0)
	{
		errBox("No save file name selected");
		return "";
	}
	return szSaveFileName;
}

