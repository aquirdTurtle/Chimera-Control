#include "stdafx.h"

#include "explorerOpen.h"

#include <string>

/*
* This function gets the name of a file to open, using the fancy windows dialog box.
* It returns the chosen filename if successful, it returns "" if the user canceled.
*/
std::string explorerOpen(CWnd* parentWindow, std::string extensions, std::string directory)
{
	// common dialog box structure
	OPENFILENAME ofn;       
	// buffer for file name
	char szFile[260];       

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = *parentWindow;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = NULL;// extensions.c_str();//"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = directory.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn))
	{
		return ofn.lpstrFile;
	}
	else
	{
		return "";
	}
}
