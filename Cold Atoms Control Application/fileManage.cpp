
#include "stdafx.h"
#include "fileManage.h"
#include "saveTextFileFromEdit.h"
#include "getFileName.h"
#include "cleanString.h"
#include "constants.h"
#include "externals.h"
#include <algorithm>
#include "appendText.h"
#include <sstream>

#include "colorScript.h"

/**
 * This namespace contains all major functions for handling script and configuration save files. It includes:
 * 
 * int checkExperimentSave(HWND parWin)
 * int openExperimentConfig(HWND parWin, std::string comboBoxItem)
 * int saveConfig()
 * int saveConfigAs(HWND hostWin)
 * int newScript(std::string defaultFileName, HWND& scriptEdit, std::string &fileAddr, char(&name)[_MAX_FNAME], HWND &nameHandle, bool &saveVar, HWND &saveInd)
 * int openScript(HWND parent, std::string &filePathway, char(&name)[_MAX_FNAME], HWND &relevantEdit, HWND &savedInd, HWND &nameHandle, bool &savedVar,
				bool promptForFile, bool is_NIAWG_Script)
 * int saveScript(HWND editToSave, std::string& filePath, HWND& savedIndicator, bool& savedVariable)
 * int saveScriptAs(HWND editToSave, HWND hostWindow, std::string &filePath, char(&currentName)[_MAX_FNAME], HWND &nameDisplay, HWND &savedIndicator,
					bool &savedVariable)
 * int checkSaveScript(std::string x_or_y_or_I, HWND editOfInterest, HWND parent, char(&name)[_MAX_FNAME], HWND &savedIndicator, bool &savedVariable, std::string &filePathway,
					   HWND &nameDisplayHandle)
 */
namespace fileManage
{

	/*
	 * This function checks whether the user wants to save an experiment configuration or not. It returns 1 if successful, it returns 0 if the user canceled.
	 */
	int checkExperimentSave(HWND parWin)
	{
		int msgboxID = MessageBox(NULL, "Save Current Profile?", "Save Prompt", MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3);
		switch (msgboxID)
		{
			case IDYES:
			{
				fileManage::saveConfig();
				return 1;
				break;
			}
			case IDNO:
			{
				return 1;
				// just leave
				break;
			}
			case IDCANCEL:
			{
				// Leave returning signal for winproc to quit
				return 0;
				break;
			}
		}
		return -1;
	}

	int checkConfigurationSave(HWND parentWindow)
	{
		int msgboxID = MessageBox(NULL, "Save Current Profile?", "Save Prompt", MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3);
		switch (msgboxID)
		{
			case IDYES:
			{
				fileManage::saveConfig();
				return 1;
				break;
			}
			case IDNO:
			{
				return 1;
				// just leave
				break;
			}
			case IDCANCEL:
			{
				// Leave returning signal for winproc to quit
				return 0;
				break;
			}
		}
		return 0;
	}

	/*
	 * This function opens an experiment configuration file and loads all of it's recorded settings into the current settings. comboBoxItem is the experiment configuration name if 
	 * the user selected an experiment configuration from the combobox, else it's "".
	 * The function returns 0 if canceled for some reason, -1 if error, 1 if successful load.
	 */
	int openExperimentConfig(HWND parWin, std::string comboBoxItem)
	{
		bool reverseOrder = false;
		// this gets called from the file menu.
		// Assign based on the comboBox Item entry.
		eExperimentConfigPathString = EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH + "\\" + comboBoxItem + "\\" + comboBoxItem + ".experimentConfig";
		std::vector<char> vecChar_CurrentConfigurationName(eCurrentCategoryName.c_str(), eCurrentCategoryName.c_str() + eCurrentCategoryName.size() + 1u);
		char nameStr[_MAX_FNAME];
		char extChars[_MAX_EXT];
		// do stuff
		int myError = _splitpath_s(eExperimentConfigPathString.c_str(), NULL, 0, NULL, 0, nameStr, _MAX_FNAME, extChars, _MAX_EXT);
		std::string extStr(extChars);
		if (extStr != ".experimentConfig")
		{
			MessageBox(0, "Please open a .experimentConfig file.", 0, 0);
			return 0;
			// user tried to open a not-conifg file.
		}
		eCurrentExperimentName = std::string(nameStr);

		std::ifstream experimentConfigOpenFile(eExperimentConfigPathString.c_str());

		// check if opened correctly.
		if (!experimentConfigOpenFile)
		{
			MessageBox(0, "Opening of Experiment Configuration File Failed!", 0, 0);
			return -1;
		}

		/// Set the Configuration combobox.
		// Get all files in the relevant directory.
		std::vector<std::string> configurationNames;
		configurationNames = fileManage::searchForFiles(eCurrentExperimentFolder, "*");

		// Make the final vector out of the unique objects left.
		// Send list to object
		for (int comboInc = 0; comboInc < configurationNames.size(); comboInc++)
		{
			SendMessage(eCategoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(configurationNames[comboInc].c_str())));
		}
		
		/// Load Values from the experiment config file.
		// Dummy Variable Option
		experimentConfigOpenFile >> eUseDummyVariables;
		if (eUseDummyVariables) 
		{
			CheckDlgButton(parWin, IDC_DUMMY_BUTTON, BST_CHECKED);
			// You can't use normal variables if you use a dummy variable.
			eVariableNames.resize(0);
			eVerticalVarFileNames.resize(0);
			char tempChar2 = '\0';
			SetWindowText(eVar1NameTextHandle, &tempChar2);
			// If these weren't already empty, they are now, so...
			SetWindowText(eVar2NameTextHandle, &tempChar2);
			SetWindowText(eVar3NameTextHandle, &tempChar2);
			SetWindowText(eVar4NameTextHandle, &tempChar2);
			SetWindowText(eVar5NameTextHandle, &tempChar2);
			SetWindowText(eVar6NameTextHandle, &tempChar2);
		}
		else 
		{
			CheckDlgButton(parWin, IDC_DUMMY_BUTTON, BST_UNCHECKED);
		}
		// Dummy Variable Number
		experimentConfigOpenFile >> eDummyNum;
		std::string dummyNumString = std::to_string(eDummyNum);
		SetWindowText(eDummyNumTextHandle, (LPCSTR)dummyNumString.c_str());

		// Accumulations Number
		experimentConfigOpenFile >> eAccumulations;
		std::string accumString = std::to_string(eAccumulations);
		SetWindowText(eAccumulationsTextHandle, (LPCSTR)accumString.c_str());

		// get var files from master option
		experimentConfigOpenFile >> eGetVarFilesFromMaster;
		if (eGetVarFilesFromMaster)
		{
			CheckDlgButton(parWin, IDC_RECEIVE_VAR_FILES_BUTTON, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_RECEIVE_VAR_FILES_BUTTON, BST_UNCHECKED);
		}

		// output waveform read progress option
		experimentConfigOpenFile >> eOutputReadStatus;
		if (eOutputReadStatus)
		{
			CheckDlgButton(parWin, IDC_OUTPUT_READ_STATUS, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_OUTPUT_READ_STATUS, BST_UNCHECKED);
		}
		// output waveform write progress option
		experimentConfigOpenFile >> eOutputWriteStatus;
		if (eOutputWriteStatus)
		{
			CheckDlgButton(parWin, IDC_OUTPUT_WRITE_STATUS, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_OUTPUT_WRITE_STATUS, BST_UNCHECKED);
		}
		// log current script option
		experimentConfigOpenFile >> eLogScriptAndParams;
		if (eLogScriptAndParams)
		{
			CheckDlgButton(parWin, IDC_LOG_SCRIPT_PARAMS, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_LOG_SCRIPT_PARAMS, BST_UNCHECKED);
		}

		// connect to master option
		experimentConfigOpenFile >> eConnectToMaster;
		if (eConnectToMaster)
		{
			CheckDlgButton(parWin, IDC_CONNECT_TO_MASTER_BUTTON, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_CONNECT_TO_MASTER_BUTTON, BST_UNCHECKED);
		}

		// output correction waveform time option
		experimentConfigOpenFile >> eOutputCorrTime;
		if (eOutputCorrTime)
		{
			CheckDlgButton(parWin, IDC_OUTPUT_CORR_TIME_BUTTON, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_OUTPUT_CORR_TIME_BUTTON, BST_UNCHECKED);
		}
		// program the agilent intensity functino generator option
		experimentConfigOpenFile >> eProgramIntensityOption;
		if (eProgramIntensityOption)
		{
			CheckDlgButton(parWin, IDC_PROGRAM_INTENSITY_BOX, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_PROGRAM_INTENSITY_BOX, BST_UNCHECKED);
		}

		experimentConfigOpenFile >> eOutputRunInfo;
		if (eOutputRunInfo)
		{
			CheckDlgButton(parWin, IDC_OUTPUT_MORE_RUN_INFO, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(parWin, IDC_OUTPUT_MORE_RUN_INFO, BST_UNCHECKED);
		}

		std::string notes;
		std::string tempNote;
		std::getline(experimentConfigOpenFile, tempNote);
		while (experimentConfigOpenFile)
		{
			notes += tempNote + "\r\n";
			std::getline(experimentConfigOpenFile, tempNote);
		} 
		SendMessage(eExperimentConfigurationNotesEditHandle, WM_SETTEXT, 0, (LPARAM)notes.c_str());

		// And done. update the indicators:
		SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
		eExperimentSaved = true;

		SetWindowText(eConfigurationDisplayInScripting, "");
		// close.
		experimentConfigOpenFile.close();
		return 1;
	}

	/*
	 * This function opens the scripts from the subconfig file
	 */
	int openSubConfig(HWND parWin, std::string subConfigFileName)
	{
		/// Open file
		std::ifstream subConfigFile(subConfigFileName.c_str());
		if (!subConfigFile.is_open())
		{
			MessageBox(0, "Couldn't open Subconfig File!", 0, 0);
			return 0;
		}
		///
		// Open Vertical Script
		getline(subConfigFile, eVerticalParentScriptPathString);
		fileManage::openScript(parWin, eVerticalParentScriptPathString, eVerticalCurrentParentScriptName, eVerticalScriptEditHandle, eVerticalScriptSavedIndicatorHandle, eVerticalScriptNameTextHandle,
							   eVerticalScriptSaved, false, true, true);
		// Open Horizontal Script
		getline(subConfigFile, eHorizontalParentScriptPathString);
		fileManage::openScript(parWin, eHorizontalParentScriptPathString, eHorizontalCurrentParentScriptName, eHorizontalScriptEditHandle, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptNameTextHandle,
							   eHorizontalScriptSaved, false, true, true);
		// Open Intensity Script
		getline(subConfigFile, eIntensityParentScriptPathString);
		fileManage::openScript(parWin, eIntensityParentScriptPathString, eIntensityCurrentParentScriptName, eIntensityScriptEditHandle, eIntensityScriptSavedIndicatorHandle,
							   eIntensityNameHandle, eIntensityScriptSaved, false, false, true);
		int position;
		std::string scriptLocation;
		if (eVerticalParentScriptPathString.size() > 0)
		{
			// Check location of vertical script.
			position = eVerticalParentScriptPathString.find_last_of('\\');
			scriptLocation = eVerticalParentScriptPathString.substr(0, position);
			if (scriptLocation != eCurrentCategoryFolder)
			{
				int answer = MessageBox(0, "The requested vertical script is not currently located in the current configuration folder. This is recommended so that scripts related to a"
					" particular configuration are reserved to that configuration folder. Copy script to current configuration folder?", 0, MB_YESNO);
				if (answer == IDYES)
				{
					std::string scriptName = eVerticalParentScriptPathString.substr(position, eVerticalParentScriptPathString.size());
					eVerticalParentScriptPathString = eCurrentCategoryFolder + scriptName;
					fileManage::saveScript(eVerticalScriptEditHandle, eVerticalParentScriptPathString, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved);
				}
			}
		}
		if (eHorizontalParentScriptPathString.size() > 0)
		{
			// Check location of horizontal script.
			position = eHorizontalParentScriptPathString.find_last_of('\\');
			scriptLocation = eHorizontalParentScriptPathString.substr(0, position);
			if (scriptLocation != eCurrentCategoryFolder)
			{
				int answer = MessageBox(0, "The requested horizontal script is not currently located in the current configuration folder. This is recommended so that scripts related to a"
					" particular configuration are reserved to that configuration folder. Copy script to current configuration folder?", 0, MB_YESNO);
				if (answer == IDYES)
				{
					std::string scriptName = eHorizontalParentScriptPathString.substr(position, eHorizontalParentScriptPathString.size());
					eHorizontalParentScriptPathString = eCurrentCategoryFolder + scriptName;
					fileManage::saveScript(eHorizontalScriptEditHandle, eHorizontalParentScriptPathString, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved);
				}
			}
		}

		if (eIntensityParentScriptPathString.size() > 0)
		{
			// Check location of Intensity script.
			position = eIntensityParentScriptPathString.find_last_of('\\');
			scriptLocation = eIntensityParentScriptPathString.substr(0, position);
			if (scriptLocation != eCurrentCategoryFolder)
			{
				int answer = MessageBox(0, "The requested intensity script is not currently located in the current configuration folder. This is recommended so that scripts related to a"
					" particular configuration are reserved to that configuration folder. Copy script to current configuration folder?", 0, MB_YESNO);
				if (answer == IDYES)
				{
					std::string scriptName = eIntensityParentScriptPathString.substr(position, eIntensityParentScriptPathString.size());
					eIntensityParentScriptPathString = eCurrentCategoryFolder + scriptName;
					fileManage::saveScript(eIntensityScriptEditHandle, eIntensityParentScriptPathString, eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved);
				}
			}
		}
		/// Get Variables
		// Number of Variables
		int varNum;
		subConfigFile >> varNum;
		if (varNum < 0 || varNum > 6)
		{
			// nothing in the file...?
			eVariableNames.resize(0);
		}
		else
		{
			eVariableNames.resize(varNum);
		}
		// Handle Variable Characters
		if (eVariableNames.size() < 6)
		{
			SetWindowText(eVar6NameTextHandle, '\0');
		}
		else
		{
			subConfigFile >> eVariableNames[5];
			SetWindowText(eVar6NameTextHandle, eVariableNames[5].c_str());
		}
		if (eVariableNames.size() < 5)
		{
			SetWindowText(eVar5NameTextHandle, '\0');
		}
		else
		{
			subConfigFile >> eVariableNames[4];
			SetWindowText(eVar5NameTextHandle, eVariableNames[4].c_str());
		}
		if (eVariableNames.size() < 4)
		{
			SetWindowText(eVar4NameTextHandle, '\0');
		}
		else
		{
			subConfigFile >> eVariableNames[3];
			SetWindowText(eVar4NameTextHandle, eVariableNames[3].c_str());
		}
		if (eVariableNames.size() < 3)
		{
			SetWindowText(eVar3NameTextHandle, '\0');
		}
		else
		{
			subConfigFile >> eVariableNames[2];
			SetWindowText(eVar3NameTextHandle, eVariableNames[2].c_str());
		}
		if (eVariableNames.size() < 2)
		{
			SetWindowText(eVar2NameTextHandle, '\0');
		}
		else
		{
			subConfigFile >> eVariableNames[1];
			SetWindowText(eVar2NameTextHandle, eVariableNames[1].c_str());
		}
		if (eVariableNames.size() < 1)
		{
			SetWindowText(eVar1NameTextHandle, '\0');
		}
		else
		{
			subConfigFile >> eVariableNames[0];
			SetWindowText(eVar1NameTextHandle, eVariableNames[0].c_str());
			// Can't use adummy variable if you use real variables.
			CheckDlgButton(parWin, IDC_DUMMY_BUTTON, BST_UNCHECKED);
			eUseDummyVariables = false;
		}

		fileManage::saveConfig();

		return 0;
	}
	
	/*
	 * Saves the configuration based on the current configuration name. Retruns 0 if no current name, 1 if successful.
	 */
	int saveConfig()
	{
		// check if name exists.
		if (eExperimentConfigPathString == "")
		{
			MessageBox(0, "No experiment configuration has been set yet.", 0, 0);
			// name doesn't exist
			return 0;
		}

		// If it does, open it.
		std::fstream experimentConfigSaveFile(eExperimentConfigPathString, std::fstream::out);
		if (!experimentConfigSaveFile.is_open())
		{
			MessageBox(0, "Couldn't open the experiment configuration file!", 0, 0);
			return -1;
		}
		// Dummy Variable Option
		experimentConfigSaveFile << eUseDummyVariables << "\n";
		// Dummy Variable Number
		experimentConfigSaveFile << eDummyNum << "\n";
		// Accumulations Number
		experimentConfigSaveFile << eAccumulations << "\n";
		// get var files from master option
		experimentConfigSaveFile << eGetVarFilesFromMaster << "\n";
		// output waveform read progress option
		experimentConfigSaveFile << eOutputReadStatus << "\n";
		// output waveform write progress option
		experimentConfigSaveFile << eOutputWriteStatus << "\n";
		// log current script option
		experimentConfigSaveFile << eLogScriptAndParams << "\n";
		// connect to master option
		experimentConfigSaveFile << eConnectToMaster << "\n";
		// output correction waveform time option
		experimentConfigSaveFile << eOutputCorrTime << "\n";		
		// Output intensity programming option.
		experimentConfigSaveFile << eProgramIntensityOption << "\n";
		// Output more run info option.
		experimentConfigSaveFile << eOutputRunInfo << "\n";
		// notes.
		LRESULT notesLength = SendMessage(eExperimentConfigurationNotesEditHandle, WM_GETTEXTLENGTH, 0, 0);
		char* buffer = new char[notesLength + 1];
		SendMessage(eExperimentConfigurationNotesEditHandle, WM_GETTEXT, notesLength + 1, (LPARAM)buffer);
		std::string notes(buffer);
		delete buffer;
		experimentConfigSaveFile << notes << "\n";
		// And done.
		experimentConfigSaveFile.close();
		
		SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
		eExperimentSaved = true;

		if (eCurrentConfigurationLocation == "")
		{
			MessageBox(0, "No subconfiguration has been set yet.", 0, 0);
			// name doesn't exist
			return 0;
		}
		
		std::fstream subConfigSaveFile(eCurrentConfigurationLocation.c_str(), std::fstream::out);
		//
		if (!subConfigSaveFile.is_open())
		{
			MessageBox(0, "Subconfiguration failed to open!", 0, 0);
		}
		// order matters!
		// vertical Script File Address
		subConfigSaveFile << eVerticalParentScriptPathString << "\n";
		// horizontal Script File Address
		subConfigSaveFile << eHorizontalParentScriptPathString << "\n";
		// Intensity Script File Address
		subConfigSaveFile << eIntensityParentScriptPathString << "\n";
		// Number of Variables
		subConfigSaveFile << eVariableNames.size() << "\n";
		// Variable Characters
		for (int i = 0; i < eVariableNames.size(); i++)
		{
			subConfigSaveFile << eVariableNames[i] << "\n";
		}
		return 1;
	}

	/*
	 * This file clears a script window, clears the script name, and opens the default script as a template. It always returns 1.
	 */
	int newScript(std::string defaultFileName, HWND& scriptEdit, std::string& fileAddr, char(&name)[_MAX_FNAME], HWND& nameHandle, bool& saveVar, 
				  HWND& saveInd)
	{
		std::string tempName;
		tempName = DEFAULT_SCRIPT_FOLDER_PATH + defaultFileName;
		
		std::ifstream openFile(tempName.c_str());
		std::string tempLine;
		std::string fileText;
		while (std::getline(openFile, tempLine)){
			cleanString(tempLine);

			fileText += tempLine;
			// Append the line to the edit control here (use c_str() ).
		}
		// put the default into the new control.
		SendMessage(scriptEdit, WM_SETTEXT, NULL, (LPARAM)fileText.c_str());
		// This is a new file. I don't want to accidentally overwrite the default file.
		SendMessage(saveInd, BM_SETCHECK, BST_UNCHECKED, NULL);
		saveVar = false;
		// Clear these variables.
		sprintf_s(name, "");
		SetWindowText(nameHandle, "");
		fileAddr = "";

		openFile.close();
		return 1;
	}

	/*
	 * This prompts the user for a script to open and then opens it. It returns 0 if the user cancels or if the user doesn't select a script file. It returns
	 * 1 if successful.
	 */
	int openScript(HWND parent, std::string& filePathway, char(&name)[_MAX_FNAME], HWND& relevantEdit, HWND& savedInd, HWND& nameHandle, bool& savedVar,
				   bool promptForFile, bool is_NIAWG_Script, bool isParentScript)
	{
		std::string tempName;
		if (promptForFile == true)
		{
			tempName = getFileNameDialog(parent);
			// user didn't enter a name, pressed cancel.
			if (tempName == "")
			{
				return 0;
			}
			char extChars[_MAX_EXT];
			int myError = _splitpath_s(tempName.c_str(), NULL, 0, NULL, 0, name, _MAX_FNAME, extChars, _MAX_EXT);
			std::string extStr(extChars);
			if (extStr != ".script")
			{
				MessageBox(0, "Please open a .script file.", 0, 0);
				return 0;
			}
			filePathway = tempName;
		}
		if (filePathway == "")
		{
			SendMessage(relevantEdit, WM_SETTEXT, NULL, (LPARAM)"");
			SendMessage(savedInd, BM_SETCHECK, BST_CHECKED, NULL);
			savedVar = true;
			return 0;
		}
		char extChars[_MAX_EXT];
		int myError = _splitpath_s(filePathway.c_str(), NULL, 0, NULL, 0, name, _MAX_FNAME, extChars, _MAX_EXT);
		std::string extStr(extChars);
		if (extStr != ".script")
		{
			MessageBox(0, "Please open a .script file.", 0, 0);
			return 0;
		}
		std::string nameString(name);
		// the file should already exist if called via the open function.
		if (!fileExists(filePathway))
		{
			MessageBox(0, ("ERROR: The file located at: " + filePathway 
						   + " could not be opened. Check to make sure this file exists in this location.").c_str(), 0, 0);
			return 0;
		}
		SendMessage(relevantEdit, WM_SETTEXT, NULL, (LPARAM)"");
		std::ifstream openFile(filePathway.c_str());
		std::string tempLine;
		std::string fileText;
		std::string coloring;
		//std::string word;
		std::string word;
		std::string appendingText;
		CHARFORMAT syntaxFormat;
		memset(&syntaxFormat, 0, sizeof(CHARFORMAT));
		syntaxFormat.cbSize = sizeof(CHARFORMAT);
		syntaxFormat.dwMask = CFM_COLOR;
		int relevantID = GetDlgCtrlID(relevantEdit);
		while (std::getline(openFile, tempLine))
		{
			// clean up the string
			cleanString(tempLine);
			fileText += tempLine;
		}
		SendMessage(relevantEdit, WM_SETTEXT, 0, (LPARAM)fileText.c_str());
		RedrawWindow(relevantEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		if (relevantEdit == eIntensityScriptEditHandle) 
		{
			colorScript(relevantEdit, "AGILENT", 0, ULONG_MAX, eIntensityViewCombo.hwnd, eCurrentIntensityViewIsParent);
		}
		else if (relevantEdit == eVerticalScriptEditHandle)
		{
			colorScript(relevantEdit, "NIAWG", 0, ULONG_MAX, eVerticalViewCombo.hwnd, eCurrentVerticalViewIsParent);
		}
		else
		{
			colorScript(relevantEdit, "NIAWG", 0, ULONG_MAX, eHorizontalViewCombo.hwnd, eCurrentHorizontalViewIsParent);
		}

		int position;
		std::string scriptLocation;
		if (filePathway.size() > 0)
		{
			// Check location of vertical script.
			position = filePathway.find_last_of('\\');
			scriptLocation = filePathway.substr(0, position);
			if (scriptLocation != eCurrentCategoryFolder)
			{
				int answer = MessageBox(0, "The requested script is not currently located in the current configuration folder. This is recommended so that scripts related to a"
					" particular configuration are reserved to that configuration folder. Copy script to current configuration folder?", 0, MB_YESNO);
				if (answer == IDYES)
				{
					std::string scriptName = filePathway.substr(position, filePathway.size());
					filePathway = eCurrentCategoryFolder + scriptName;
					fileManage::saveScript(relevantEdit, filePathway, savedInd, savedVar);
				}
			}
		}
		// Since opening a saved file, this file is the most recent version of things. Set this to be saved.
		SendMessage(savedInd, BM_SETCHECK, BST_CHECKED, NULL);
		if (isParentScript)
		{
			SetWindowText(nameHandle, name);
		}
		openFile.close();
		savedVar = true;
		return 1;
	}

	/*
	 * This saves a script based on the script's current name. If no current name, it returns zero. If the user tries to save a file currently being used, it
	 * returns -1. If successful, it returns 1.
	 */
	int saveScript(HWND editToSave, std::string& filePath, HWND& savedIndicator, bool& savedVariable)
	{
		// check if name exists.
		if (filePath == "")
		{
			return 0;
		}
		if (eSystemIsRunning)
		{
			if (filePath == eMostRecentHorizontalScriptNames || filePath == eMostRecentIntensityScriptNames || filePath == eMostRecentVerticalScriptNames)
			{
				MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
					"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
				return -1;
			}
		}
		// If it does...
		char tempChar[10000];
		int myError = GetWindowText(editToSave, &tempChar[0], 10000);
		std::fstream saveFile(filePath, std::fstream::out);
		int test = saveFile.is_open();
		saveFile << tempChar;

		saveFile.close();

		SendMessage(savedIndicator, BM_SETCHECK, BST_CHECKED, NULL);
		savedVariable = true;
		return 1;
	}

	/*
	 * This prompts the user for a file name, and then saves a script to that name. It returns 0 if user canceled or if the script is empty or if the user 
	 * tries to save over a file currently being used. It returns 1 if successful.
	 */
	int saveScriptAs(HWND editToSave, HWND hostWindow, std::string &filePath, char(&currentName)[_MAX_FNAME], HWND &nameDisplay, HWND &savedIndicator,
					 bool& savedVariable)
	{
		char tempChar[10000];
		// get the text of the script
		int myError = GetWindowText(editToSave, &tempChar[0], 10000);

		if (myError == 0)
		{
			if (tempChar[0] != '\0')
			{
				return 0;
			}
		}
		filePath = saveTextFileFromEdit(hostWindow, "script");
		if (filePath == "")
		{
			// user canceled.
			return 0;
		}
		myError = _splitpath_s(filePath.c_str(), NULL, 0, NULL, 0, currentName, _MAX_FNAME, NULL, 0);
		if (eSystemIsRunning) 
		{
			if (currentName == eMostRecentHorizontalScriptNames || currentName == eMostRecentIntensityScriptNames || currentName == eMostRecentVerticalScriptNames)
			{
				MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
							  "horizontal and vertical AOM scripts and the intensity script.", 0, 0);
				return 0;
			}
		}

		std::fstream saveFile(filePath, std::fstream::out);

		saveFile << tempChar;

		saveFile.close();

		SendMessage(savedIndicator, BM_SETCHECK, BST_CHECKED, NULL);

		savedVariable = true;

		SetWindowText(nameDisplay, &currentName[0]);
		return 1;
	}
	
	/*
	 * This checks if the user wants to save a script. it returns 1 unless the user cancels, in which case it returns 0.
	 */
	int checkSaveScript(std::string scriptType, HWND editOfInterest, HWND parent, char(&name)[_MAX_FNAME], HWND &savedIndicator, bool &savedVariable, 
						std::string &filePathway, HWND &nameDisplayHandle)
	{

		char prompt[100];
		sprintf_s(prompt, "Save Current %s Script?", scriptType.c_str());
		int msgboxID = MessageBox(NULL, prompt, "Save Prompt", MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3);
		switch (msgboxID)
		{
			case IDYES:
			{
				// if there is a name, then just save
				if (strcmp(name, "") != 0)
				{
					fileManage::saveScript(editOfInterest, filePathway, savedIndicator, savedVariable);
				}
				// else no name -> save as
				else
				{
					fileManage::saveScriptAs(editOfInterest, parent, filePathway, name, nameDisplayHandle, savedIndicator, savedVariable);
				}
				return 1;
				break;
			}
			case IDNO:
			{
				return 1;
				// just leave
				break;
			}
			case IDCANCEL:
			{
				// Leave returning signal for winproc to quit
				return 0;
				break;
			}
		}
		return 1;
	}
	/*
	 * This function searches a specific location for a certain type of file and removes redundancy if necessary. It returns a vector of the names of the files
	 * found that meet the given criteria.
	 */
	std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions)
	{
		// Re-add the entries back in and figure out which one is the current one.
		std::vector<std::string> names;
		std::string search_path = locationToSearch + "\\" + extensions;
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		if (extensions == "*")
		{
			hFind = FindFirstFileEx(search_path.c_str(), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0);
		}
		else
		{
			hFind = FindFirstFile(search_path.c_str(), &fd);
		}	
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				// if looking for folders
				if (extensions == "*")
				{
					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
					{
						if (std::string(fd.cFileName) != "." && std::string(fd.cFileName) != "..")
						{
							names.push_back(fd.cFileName);
						}
					}
				}
				else
				{
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
							names.push_back(fd.cFileName);
					}
				}
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}

		// Remove suffix from file names and...
		for (int configListInc = 0; configListInc < names.size(); configListInc++)
		{
			if (extensions == "*" || extensions == "*.*" || extensions == "*.hSubConfig" || extensions == "*.vSubConfig" || extensions == "*.seq")
			{
				names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - (extensions.size() - 1));
			}
			else
			{
				names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - extensions.size());
			}
		}
		// Make the final vector out of the unique objects left.
		return names;
	}

	int lookForPredefinedScripts(HWND editToSearch, HWND comboToUpdate)
	{
		std::vector<std::string> predefinedScripts;
		TCHAR editText[10192];
		SendMessage(editToSearch, WM_GETTEXT, 10192, (LPARAM)editText);
		std::stringstream editTextStream;
		editTextStream << editText;
		std::string line;
		//
		getline(editTextStream, line);
		while (editTextStream)
		{
			if (line.size() > 7)
			{
				if (line.substr(line.size() - 8, 7) == ".script")
				{
					bool newScript = true;
					for (int predefinedInc = 0; predefinedInc < predefinedScripts.size(); predefinedInc++)
					{
						if (predefinedScripts[predefinedInc] == line.substr(0, line.size() - 7))
						{
							newScript = false;
						}
					}
					if (newScript)
					{
						predefinedScripts.push_back(line.substr(0, line.size() - 8));
					}
				}
			}
			getline(editTextStream, line);
		}
		// add these to the combo
		// eVerticalViewCombo.hwnd
		SendMessage(comboToUpdate, CB_RESETCONTENT, 0, 0);
		SendMessage(comboToUpdate, CB_ADDSTRING, 0, (LPARAM)"Parent Script");
		for (int predefinedInc = 0; predefinedInc < predefinedScripts.size(); predefinedInc++)
		{
			SendMessage(comboToUpdate, CB_ADDSTRING, 0, (LPARAM)predefinedScripts[predefinedInc].c_str());
		}
		return 0;
	}

	int reloadCombo(HWND comboToReload, std::string serachLocation, std::string extension, std::string nameToLoad)
	{

		std::vector<std::string> names;
		// search for folders
		names = fileManage::searchForFiles(serachLocation, extension);

		/// Get current selection
		long long itemIndex = SendMessage(comboToReload, CB_GETCURSEL, 0, 0);
		TCHAR experimentConfigToOpen[256];
		std::string currentSelection;
		int currentInc = -1;
		if (itemIndex != -1)
		{
			// Send CB_GETLBTEXT message to get the item.
			SendMessage(comboToReload, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
			currentSelection = experimentConfigToOpen;
		}
		/// Reset stuffs
		SendMessage(comboToReload, CB_RESETCONTENT, 0, 0);
		// Send list to object
		for (int comboInc = 0; comboInc < names.size(); comboInc++)
		{
			if (nameToLoad == names[comboInc])
			{
				currentInc = comboInc;
			}
			SendMessage(comboToReload, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(names[comboInc].c_str())));
		}
		// Set initial value
		SendMessage(comboToReload, CB_SETCURSEL, currentInc, 0);
		return 0;
	}

	bool fileExists(std::string filePathway)
	{
		// got this from stack exchange. dunno how it works but it should be fast.
		struct stat buffer;
		return (stat(filePathway.c_str(), &buffer) == 0);
	}
}