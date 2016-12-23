#pragma once
#include "stdafx.h"
#include <string>
/**
 * This namespace contains all major functions for handling script and configuration save files. It includes:
 X~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~X
 * int checkExperimentSave(HWND parWin);
 * int openExperimentConfig(HWND parWin, std::string comboBoxItem);
 * int saveConfig();
 * int saveConfigAs(HWND hostWin);
 * int newScript(std::string defaultFileName, HWND &scriptEdit, std::string &fileAddr, char(&name)[_MAX_FNAME], HWND &nameHandle, bool &saveVar,
 * 				 HWND &saveInd);
 * int openParentScript(HWND parent, std::string &filePathway, char(&name)[_MAX_FNAME], HWND &relevantEdit, HWND &savedInd, HWND &nameHandle, bool &savedVar,
 *				  bool promptForFile, bool is_NIAWG_Script = true);
 * int checkSaveScript(std::string x_or_y_or_I, HWND editOfInterest, HWND parent, char(&name)[_MAX_FNAME], HWND &savedIndicator, bool &savedVariable,
 *					   std::string &filePathway, HWND &nameDisplayHandle);
 * int saveScript(HWND editToSave, std::string &filePath, HWND &savedIndicator, bool &savedVariable);
 * int saveScriptAs(HWND editToSave, HWND hostWindow, std::string &filePath, char(&currentName)[_MAX_FNAME], HWND &nameDisplay, HWND &savedIndicator,
 *				    bool &savedVariable);
 */
namespace fileManage
{
	/*
	int checkExperimentSave(HWND parWin);
	int openExperimentConfig(HWND parWin, std::string comboBoxItem);
	int openConfiguration(HWND parWin, std::string subConfigFile);
	int saveConfig();
	//int saveConfigAs(HWND hostWin);
	int newScript(std::string defaultFileName, HWND &scriptEdit, std::string &fileAddr, char(&name)[_MAX_FNAME], HWND &nameHandle, bool &saveVar, 
				  HWND &saveInd);
	int openParentScript(HWND parent, std::string &filePathway, char(&name)[_MAX_FNAME], HWND &relevantEdit, HWND &savedInd, HWND &nameHandle, bool &savedVar, 
				   bool promptForFile, bool is_NIAWG_Script, bool isParentScript);
	int checkSaveScript(std::string x_or_y_or_I, HWND editOfInterest, HWND parent, char(&name)[_MAX_FNAME], HWND &savedIndicator, bool &savedVariable, 
						std::string &filePathway, HWND &nameDisplayHandle, std::string scriptType);
	int saveScript(HWND editToSave, std::string &filePath, HWND &savedIndicator, bool &savedVariable);
	int saveScriptAs(HWND editToSave, HWND hostWindow, std::string &filePath, char(&currentName)[_MAX_FNAME], HWND &nameDisplay, HWND &savedIndicator, 
					 bool &savedVariable, std::string scriptType);
	std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);
	int reloadCombo(HWND comboToReload, std::string serachLocation, std::string extension, std::string nameToLoad);
	int lookForPredefinedScripts(HWND editToSearch, HWND comboToUpdate);
	bool fileOrFolderExists(std::string filePathway);
	*/
}
