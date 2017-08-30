#include "stdafx.h"

#include "openWithExplorer.h"
#include <string>

/*
 * This function gets the name of a file to open, using the fancy windows dialog box.
 * It returns the chosen filename if successful, it returns "" if the user canceled.
 */
std::string openWithExplorer(CWnd* parent)
{
	OPENFILENAME dialogInfo;
	// buffer for file name
	char fileName[260];       	
	// Initialize OPENFILENAME
	ZeroMemory(&dialogInfo, sizeof(dialogInfo));
	dialogInfo.lStructSize = sizeof(dialogInfo);
	dialogInfo.hwndOwner = parent->GetSafeHwnd();
	dialogInfo.lpstrFile = fileName;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	dialogInfo.lpstrFile[0] = '\0';
	dialogInfo.nMaxFile = sizeof(fileName);
	dialogInfo.lpstrFilter = NULL;//"All\0*.*\0Text\0*.TXT\0";
	dialogInfo.nFilterIndex = 1;
	dialogInfo.lpstrFileTitle = NULL;
	dialogInfo.nMaxFileTitle = 0;
	dialogInfo.lpstrInitialDir = NULL;
	dialogInfo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&dialogInfo))
	{
		return dialogInfo.lpstrFile;
	}
	else 
	{
		return "";
	}
}
