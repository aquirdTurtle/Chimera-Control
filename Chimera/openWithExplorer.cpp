#include "stdafx.h"

#include "openWithExplorer.h"
#include <string>

/*
 * This function gets the name of a file to open, using the fancy windows dialog box.
 * It returns the chosen filename if successful, it returns "" if the user canceled.
 */
std::string openWithExplorer(CWnd* parent, std::string extension)
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

	dialogInfo.lpstrFilter = txt.c_str( );
	dialogInfo.nFilterIndex = 1;
	dialogInfo.lpstrFileTitle = NULL;
	dialogInfo.nMaxFileTitle = 0;
	dialogInfo.lpstrInitialDir = NULL;
	dialogInfo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;

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
