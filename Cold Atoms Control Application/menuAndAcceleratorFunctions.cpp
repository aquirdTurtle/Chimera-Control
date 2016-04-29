#include "stdafx.h"
#include "menuAndAcceleratorFunctions.h"
#include "Windows.h"
#include "namePromptDialogProc.h"
#include "fileManage.h"
#include "appendText.h"
#include "myAgilent.h"
#include "myNIAWG.h"
#include "experimentThreadInputStructure.h"
#include "experimentProgrammingThread.h"
#include "scriptWriteHelpProc.h"
#include "myErrorHandler.h"
#include "beginningSettingsDialogProc.h"
#include "selectInCombo.h"

namespace menuAndAcceleratorFunctions
{
	int startSystem(HWND parentWindow, WPARAM wParam)
	{
		if (eCurrentSequenceName == "NO SEQUENCE")
		{
			if (eHorizontalParentScriptPathString == "")
			{
				MessageBox(0, "Horizontal script hasn't been loaded! (aborting attempt to start)", 0, 0);
				return -1;
			}
			if (eVerticalParentScriptPathString == "")
			{
				MessageBox(0, "Vertical script hasn't been loaded! (aborting attempt to start)", 0, 0);
				return -1;
			}
			if (eIntensityParentScriptPathString == "")
			{
				MessageBox(0, "Intensity script hasn't been loaded! (aborting attempt to start)", 0, 0);
				return -1;
			}
		}
		if (eSystemIsRunning)
		{
			int restart = MessageBox(0, "Restart Generation?", 0, MB_OKCANCEL);
			if (restart == IDOK)
			{
				if (!eSystemIsRunning)
				{
					appendText("System was not running. Can't Abort.\r\n", IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
					return -2;
				}
				// reset flag
				eAbortSystemFlag = true;
				// wait for reset to occur
				int result = WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);
				result = WaitForSingleObject(eExperimentThreadHandle, INFINITE);
				eAbortSystemFlag = false;
				// abort the generation on the NIAWG.
				myAgilent::agilentDefault();
				std::string msgString = "Passively Outputting Default Waveform";
				SetWindowText(eColoredStatusEdit, msgString.c_str());
				eGenStatusColor = "B";
				RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
				{
					return -3;
				}
				// Officially stop trying to generate anything.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle)))
				{
					return -3;
				}
				// clear the memory
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle)))
				{
					return -3;
				}
				ViInt32 waveID;
				if (eCurrentOrientation == "Horizontal")
				{
					// create waveform (necessary?)
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID)))
					{
						return -3;
					}
					// allocate waveform into the device memory
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2)))
					{
						return -3;
					}
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform)))
					{
						return -3;
					}
					// rewrite the script. default_hConfigScript should still be valid.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript)))
					{
						return -3;

					}
					// start generic waveform to maintain power output to AOM.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
					{
						return -3;
					}
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")))
					{
						return -3;
					}
					eCurrentScript = "DefaultHConfigScript";

				}
				else if (eCurrentOrientation == "Vertical")
				{
					// create waveform (necessary?)
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID)))
					{
						return -3;
					}
					// allocate waveform into the device memory
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2)))
					{
						return -3;
					}
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform)))
					{
						return -3;
					}
					// rewrite the script. default_hConfigScript should still be valid.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript)))
					{
						return -3;
					}
					// start generic waveform to maintain power output to AOM.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
					{
						return -3;
					}
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript")))
					{
						return -3;
					}
					eCurrentScript = "DefaultVConfigScript";
				}
				// Initiate Generation.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
				{
					return -3;
				}
				eSystemIsRunning = false;
				appendText("Restarted Generation.\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
			}
			else if (restart == IDCANCEL)
			{
				return -4;
			}
			else
			{
				return -4;
			}
		}
		if (eHorizontalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("horizontal", eHorizontalScriptEditHandle, parentWindow, eHorizontalCurrentParentScriptName, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved,
				eHorizontalParentScriptPathString, eHorizontalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -4;
			}
		}
		if (eVerticalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("vertical", eVerticalScriptEditHandle, parentWindow, eVerticalCurrentParentScriptName, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved,
				eVerticalParentScriptPathString, eVerticalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -4;
			}
		}
		if (eIntensityScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("intensity", eIntensityScriptEditHandle, parentWindow, eIntensityCurrentParentScriptName, eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved,
				eIntensityParentScriptPathString, eIntensityNameHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -4;
			}
		}
		std::string verticalNameString(eVerticalCurrentParentScriptName);
		std::string horizontalNameString(eHorizontalCurrentParentScriptName);		
		std::string beginInfo = "Current Settings:\r\n=============================\r\n\r\n";
		if (eCurrentSequenceName != "NO SEQUENCE")
		{
			beginInfo += "Sequence:\r\n";
			for (int sequenceInc = 0; sequenceInc < eSequenceFileNames.size(); sequenceInc++)
			{
				beginInfo += std::to_string(sequenceInc) + ". " + eSequenceFileNames[sequenceInc] + "\r\n";
			}
		}
		else
		{
			beginInfo += "Vertical Script Name:............. " + std::string(eVerticalCurrentParentScriptName);
			if (eVerticalScriptSaved)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Horizontal Script Name:........... " + std::string(eHorizontalCurrentParentScriptName);
			if (eHorizontalScriptSaved)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Intensity Script Name:............ " + std::string(eIntensityCurrentParentScriptName);
			if (eIntensityScriptSaved)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
		}
		beginInfo += "\r\n";
		if (eVariableNames.size() == 0)
		{
			beginInfo += "Variable Names:................... NO VARIABLES\r\n";
		}
		else
		{
			beginInfo += "Variable Names:................... ";
			for (int varInc = 0; varInc < eVariableNames.size(); varInc++)
			{
				beginInfo += eVariableNames[varInc] + " ";
			}
			beginInfo += "\r\n";
		}

		if (eConnectToMaster == true)
		{
			beginInfo += "Connecting To Master.............. TRUE\r\n";
			if (eGetVarFilesFromMaster == true)
			{
				beginInfo += "Getting Variables from Master:.... TRUE\r\n";
			}
			else
			{
				beginInfo += "Getting Variables from Master:.... FALSE\r\n";
			}
		}
		else
		{
			beginInfo += "Connecting To Master:............. FALSE\r\n";
			if (eGetVarFilesFromMaster == true)
			{
				beginInfo += "Getting Variables from Master:.... TRUE ??????\r\n";
			}
		}
		if (eProgramIntensityOption == true)
		{
			beginInfo += "Programming Intensity:............ TRUE\r\n";
		}
		else
		{
			beginInfo += "Programming Intensity:............ FALSE\r\n";
		}

		if (eLogScriptAndParams == true)
		{
			beginInfo += "Logging Script and Parameters:.... TRUE\r\n";
		}
		else
		{
			beginInfo += "Logging Script and Parameters:.... FALSE\r\n";
		}
		if (eUseDummyVariables)
		{
			beginInfo += "Using Dummy Variables:............ TRUE\r\n";
		}
		else
		{
			beginInfo += "Using Dummy Variables:............ FALSE\r\n";
		}
		beginInfo += "\r\n";
		beginInfo += "Number of Dummy Variables:........ " + std::to_string(eDummyNum) + "\r\n=============================\r\n";

		std::string beginQuestion = "\r\n\r\nBegin Waveform Generation with these Settings?";
		INT_PTR areYouSure = DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_BEGINNING_SETTINGS), 0, beginningSettingsDialogProc, (LPARAM)(beginInfo + beginQuestion).c_str());
		if (areYouSure == 0)
		{
			std::string msgString = "Performing Initial Analysis and Writing and Loading Non-Varying Waveforms...";
			SetWindowText(eColoredStatusEdit, msgString.c_str());
			eGenStatusColor = "Y";
			RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

			// Set the thread structure.
			experimentThreadInputStructure* inputParams = new experimentThreadInputStructure();
			// force accumulations to zero. This shouldn't affect anything, this should always get set by the master or be infinite.
			(*inputParams).threadAccumulations = 0;
			(*inputParams).threadConnectToMaster = eConnectToMaster;
			(*inputParams).threadCurrentScript = eCurrentScript;
			if (LOWORD(wParam) == ID_FILE_MY_WRITE_WAVEFORMS)
			{
				(*inputParams).threadDontActuallyGenerate = true;
			}
			else
			{
				(*inputParams).threadDontActuallyGenerate = false;
			}
			(*inputParams).threadDummyNum = eDummyNum;
			(*inputParams).threadGetVarFilesFromMaster = eGetVarFilesFromMaster;
			(*inputParams).threadSequenceFileNames = eSequenceFileNames;
			(*inputParams).threadLogScriptAndParams = eLogScriptAndParams;
			(*inputParams).threadProgramIntensityOption = eProgramIntensityOption;
			(*inputParams).threadUseDummyVariables = eUseDummyVariables;
			(*inputParams).threadVariableNames = eVariableNames;
			(*inputParams).threadXScriptSaved = eVerticalScriptSaved;
			(*inputParams).currentFolderLocation = eCurrentCategoryFolder;
			eMostRecentVerticalScriptNames = eVerticalParentScriptPathString;
			eMostRecentHorizontalScriptNames = eHorizontalParentScriptPathString;
			eMostRecentIntensityScriptNames = eIntensityParentScriptPathString;
			// Start the programming thread.
			unsigned int experimentThreadID;
			eExperimentThreadHandle = (HANDLE)_beginthreadex(0, 0, experimentProgrammingThread, (LPVOID *)inputParams, 0, &experimentThreadID);
			eSystemIsRunning = true;
			time_t time_obj = time(0);   // get time now
			struct tm currentTime;
			localtime_s(&currentTime, &time_obj);
			std::string timeStr = "(" + std::to_string(currentTime.tm_year + 1900) + ":" + std::to_string(currentTime.tm_mon + 1) + ":"
				+ std::to_string(currentTime.tm_mday) + ")" + std::to_string(currentTime.tm_hour) + ":"
				+ std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
			CHARFORMAT currentFormat, tempFormat, getFormat;
			memset(&getFormat, 0, sizeof(CHARFORMAT));
			getFormat.cbSize = sizeof(CHARFORMAT);
			memset(&tempFormat, 0, sizeof(CHARFORMAT));
			tempFormat.cbSize = sizeof(CHARFORMAT);

			tempFormat.dwMask = CFM_COLOR;
			tempFormat.crTextColor = RGB(255, 255, 255);
			memset(&currentFormat, 0, sizeof(CHARFORMAT));
			currentFormat.cbSize = sizeof(CHARFORMAT);
			currentFormat.dwMask = CFM_COLOR;

			// make sure selection is at end of edit.
			SendMessage(eSystemStatusTextHandle, EM_SETSEL, (WPARAM)(GetWindowTextLength(eSystemStatusTextHandle)), (LPARAM)GetWindowTextLength(eSystemStatusTextHandle));
			LRESULT result = SendMessage(eSystemStatusTextHandle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&getFormat);
			if (getFormat.crTextColor != tempFormat.crTextColor)
			{
				result = SendMessage(eSystemStatusTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&tempFormat);
			}
			appendText("\r\n**********" + timeStr + "**********\r\nSystem is Running.\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
			currentFormat.crTextColor = RGB(50, 50, 250);
			result = SendMessage(eSystemStatusTextHandle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&getFormat);
			if (getFormat.crTextColor != currentFormat.crTextColor)
			{
				result = SendMessage(eSystemStatusTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&currentFormat);
			}

			// make sure selection is at end of edit.
			SendMessage(eSystemErrorTextHandle, EM_SETSEL, (WPARAM)(GetWindowTextLength(eSystemErrorTextHandle)), (LPARAM)GetWindowTextLength(eSystemErrorTextHandle));
			result = SendMessage(eSystemErrorTextHandle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&getFormat);
			if (getFormat.crTextColor != tempFormat.crTextColor)
			{
				result = SendMessage(eSystemErrorTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&tempFormat);
			}
			appendText("\r\n**********" + timeStr + "**********\r\n", IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
			currentFormat.crTextColor = RGB(200, 0, 0);
			result = SendMessage(eSystemErrorTextHandle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&getFormat);
			if (getFormat.crTextColor != currentFormat.crTextColor)
			{
				result = SendMessage(eSystemErrorTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&currentFormat);
			}

			// make sure selection is at end of edit.
			SendMessage(eSystemDebugTextHandle, EM_SETSEL, (WPARAM)(GetWindowTextLength(eSystemDebugTextHandle)), (LPARAM)GetWindowTextLength(eSystemDebugTextHandle));
			result = SendMessage(eSystemDebugTextHandle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&getFormat);
			if (getFormat.crTextColor != tempFormat.crTextColor)
			{
				result = SendMessage(eSystemDebugTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&tempFormat);
			}
			appendText("\r\n**********" + timeStr + "**********\r\n", IDC_SYSTEM_DEBUG_TEXT, eMainWindowHandle);
			currentFormat.crTextColor = RGB(13, 152, 186);
			result = SendMessage(eSystemDebugTextHandle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&getFormat);
			if (getFormat.crTextColor != currentFormat.crTextColor)
			{
				result = SendMessage(eSystemDebugTextHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&currentFormat);
			}
			if (eOutputRunInfo)
			{
				appendText(beginInfo, IDC_SYSTEM_DEBUG_TEXT, eMainWindowHandle);
			}
		}
		return 0;
	}
	int abortSystem(HWND parentWindow)
	{
		if (!eSystemIsRunning)
		{
			appendText("System was not running. Can't Abort.\r\n", IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
			// change the color of the colored status window.
			std::string msgString = "Passively Outputting Default Waveform.";
			SetWindowText(eColoredStatusEdit, msgString.c_str());
			eGenStatusColor = "B";
			RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			return -1;
		}
		// set reset flag
		eAbortSystemFlag = true;
		// wait for reset to occur
		int result = 1;
		result = WaitForSingleObject(eNIAWGWaitThreadHandle, 0);
		if (result == WAIT_TIMEOUT)
		{
			// try again. this will put this message at the back of the message queue so waiting messages will get handled first.
			PostMessage(parentWindow, WM_COMMAND, MAKEWPARAM(ID_FILE_ABORT_GENERATION, 0), 0);
			return -1;
		}
		result = WaitForSingleObject(eExperimentThreadHandle, 0);
		if (result == WAIT_TIMEOUT)
		{
			// try again. this will put this message at the back of the message queue so waiting messages will get handled first.
			PostMessage(parentWindow, WM_COMMAND, MAKEWPARAM(ID_FILE_ABORT_GENERATION, 0), 0);
			return -1;
		}
		eAbortSystemFlag = false;
		// abort the generation on the NIAWG.
		myAgilent::agilentDefault();
		std::string msgString = "Passively Outputting Default Waveform.";
		SetWindowText(eColoredStatusEdit, msgString.c_str());
		eGenStatusColor = "B";
		RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		if (!SAFEMODE)
		{
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
			{
				return -2;
			}
			// Officially stop trying to generate anything.
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle)))
			{
				return -2;
			}
			// clear the memory
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle)))
			{
				return -2;
			}
		}
		ViInt32 waveID;

		if (eCurrentOrientation == "Horizontal")
		{
			if (!SAFEMODE)
			{
				// create waveform (necessary?)
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID)))
				{
					return -2;
				}
				// allocate waveform into the device memory
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2)))
				{
					return -2;
				}
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform)))
				{
					return -2;
				}
				// rewrite the script. default_hConfigScript should still be valid.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript)))
				{
					return -2;
				}
				// start generic waveform to maintain power output to AOM.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
				{
					return -2;
				}
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")))
				{
					return -2;
				}
			}
			eCurrentScript = "DefaultHConfigScript";
		}
		else if (eCurrentOrientation == "Vertical")
		{
			if (!SAFEMODE)
			{
				// create waveform (necessary?)
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID)))
				{
					return -2;

				}
				// allocate waveform into the device memory
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2)))
				{
					return -2;
				}
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform)))
				{
					return -2;
				}
				// rewrite the script. default_hConfigScript should still be valid.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript)))
				{
					return -2;
				}
				// start generic waveform to maintain power output to AOM.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
				{
					return -2;
				}
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript")))
				{
					return -2;
				}
			}
			eCurrentScript = "DefaultVConfigScript";
		}
		if (!SAFEMODE)
		{
			// Initiate Generation.
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
			{
				return -2;
			}
		}
		eSystemIsRunning = false;
		return 0;
	}
	int saveAll(HWND parentWindow)
	{
		// Just use the procedures that already exist. No need to rewrite the handling.
		PostMessage(parentWindow, WM_COMMAND, MAKEWPARAM(ID_FILE_MY_INTENSITY_SAVE, 0), 0);
		PostMessage(parentWindow, WM_COMMAND, MAKEWPARAM(ID_FILE_MY_VERTICAL_SAVE, 0), 0);
		PostMessage(parentWindow, WM_COMMAND, MAKEWPARAM(ID_FILE_MY_HORIZONTAL_SAVE, 0), 0);
		PostMessage(parentWindow, WM_COMMAND, MAKEWPARAM(ID_PROFILE_SAVE_PROFILE, 0), 0);
		return 0;
	}
	int exitProgram(HWND parentWindow)
	{
		if (eSystemIsRunning)
		{
			MessageBox(0, "System is Currently Running. Please stop the system before exiting so that devices devices can stop normally.", 0, 0);
			return -1;
		}

		if (eHorizontalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("horizontal", eHorizontalScriptEditHandle, parentWindow, eHorizontalCurrentParentScriptName,
				eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved, eHorizontalParentScriptPathString,
				eHorizontalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -2;
			}
		}
		if (eVerticalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("vertical", eVerticalScriptEditHandle, parentWindow, eVerticalCurrentParentScriptName, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved,
				eVerticalParentScriptPathString, eVerticalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -2;
			}
		}
		if (eIntensityScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("intensity", eIntensityScriptEditHandle, parentWindow, eIntensityCurrentParentScriptName, eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved,
				eIntensityParentScriptPathString, eIntensityNameHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -2;
			}
		}

		if (eExperimentSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkExperimentSave(parentWindow);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -2;
			}
		}
		std::string exitQuestion = "Are you sure you want to exit?\n\nThis will stop all output of the arbitrary waveform generator.";
		int areYouSure = MessageBox(NULL, exitQuestion.c_str(), "Exit", MB_OKCANCEL | MB_ICONWARNING);
		switch (areYouSure)
		{
			case IDOK:
				PostQuitMessage(1);
				break;
			case IDCANCEL:
				break;
			default:
				break;
		}
		return 0;
	}

	int newIntensityScript(HWND parentWindow)
	{
		if (eIntensityScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("intensity", eIntensityScriptEditHandle, parentWindow, eIntensityCurrentParentScriptName,
				eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved, eIntensityParentScriptPathString,
				eIntensityNameHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont = 0) 
			{
				return -1;
			}
		}
		fileManage::newScript("DEFAULT_INTENSITY_SCRIPT.script", eIntensityScriptEditHandle, eIntensityParentScriptPathString, eIntensityCurrentParentScriptName,
			eIntensityNameHandle, eIntensityScriptSaved, eIntensityScriptSavedIndicatorHandle);
		eIntensityViewScriptPathString = eIntensityParentScriptPathString;
		strcpy_s(eIntensityCurrentViewScriptName, eIntensityCurrentParentScriptName);
		selectInCombo(eIntensityViewCombo.hwnd, "Parent Script");
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
		return 0;
	}
	int openIntensityScript(HWND parentWindow)
	{
		if (eIntensityScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("intensity", eIntensityScriptEditHandle, parentWindow, eIntensityCurrentParentScriptName,
				eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved, eIntensityParentScriptPathString,
				eIntensityNameHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -1;
			}
		}
		fileManage::openScript(parentWindow, eIntensityParentScriptPathString, eIntensityCurrentParentScriptName, eIntensityScriptEditHandle,
							   eIntensityScriptSavedIndicatorHandle, eIntensityNameHandle, eIntensityScriptSaved, true, false, eCurrentIntensityViewIsParent);
		//fileManage::lookForPredefinedScripts(eIntensityScriptEditHandle, eIntensityViewCombo.hwnd);
		eIntensityViewScriptPathString = eIntensityParentScriptPathString;
		strcpy_s(eIntensityCurrentViewScriptName, eIntensityCurrentParentScriptName);
		selectInCombo(eIntensityViewCombo.hwnd, "Parent Script");
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
		return 0;
	}
	int saveIntensityScript(HWND parentWindow)
	{
		// try to save
		int success = fileManage::saveScript(eIntensityScriptEditHandle, eIntensityParentScriptPathString, eIntensityScriptSavedIndicatorHandle,
			eIntensityScriptSaved);
		// this occurs if save failed because there was no previous name to save to.
		if (success == 0)
		{
			fileManage::saveScriptAs(eIntensityScriptEditHandle, parentWindow, eIntensityParentScriptPathString, eIntensityCurrentParentScriptName, eIntensityNameHandle,
				eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved);
		}

		return 0;
	}
	int saveIntensityScriptAs(HWND parentWindow)
	{
		fileManage::saveScriptAs(eIntensityScriptEditHandle, parentWindow, eIntensityParentScriptPathString, eIntensityCurrentParentScriptName, eIntensityNameHandle,
			eIntensityScriptSavedIndicatorHandle, eIntensityScriptSaved);
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
		return 0;
	}

	int newVerticalScript(HWND parentWindow)
	{
		if (eVerticalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("vertical", eVerticalScriptEditHandle, parentWindow, eVerticalCurrentParentScriptName, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved,
				eVerticalParentScriptPathString, eVerticalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont = 0)
			{
				return -1;
			}
		}
		fileManage::newScript("DEFAULT_VERTICAL_SCRIPT.script", eVerticalScriptEditHandle, eVerticalParentScriptPathString, eVerticalCurrentParentScriptName, eVerticalScriptNameTextHandle,
			eVerticalScriptSaved, eVerticalScriptSavedIndicatorHandle);
		fileManage::lookForPredefinedScripts(eVerticalScriptEditHandle, eVerticalViewCombo.hwnd);
		eVerticalViewScriptPathString = eVerticalParentScriptPathString;
		strcpy_s(eVerticalCurrentViewScriptName, eVerticalCurrentParentScriptName);
		selectInCombo(eVerticalViewCombo.hwnd, "Parent Script");
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);

		return 0;
	}
	int openVerticalScript(HWND parentWindow)
	{
		if (eVerticalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("vertical", eVerticalScriptEditHandle, parentWindow, eVerticalCurrentParentScriptName, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved,
				eVerticalParentScriptPathString, eVerticalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -1;
			}
		}
		fileManage::openScript(parentWindow, eVerticalParentScriptPathString, eVerticalCurrentParentScriptName, eVerticalScriptEditHandle, eVerticalScriptSavedIndicatorHandle,
			eVerticalScriptNameTextHandle, eVerticalScriptSaved, true, true, eCurrentVerticalViewIsParent);
		fileManage::lookForPredefinedScripts(eVerticalScriptEditHandle, eVerticalViewCombo.hwnd);
		eVerticalViewScriptPathString = eVerticalParentScriptPathString;
		strcpy_s(eVerticalCurrentViewScriptName, eVerticalCurrentParentScriptName);
		selectInCombo(eVerticalViewCombo.hwnd, "Parent Script");
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);

		return 0;
	}
	int saveVerticalScript(HWND parentWindow)
	{
		// try to save
		int success = fileManage::saveScript(eVerticalScriptEditHandle, eVerticalParentScriptPathString, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved);
		// this occurs if save failed because there was no previous name to save to.
		if (success == 0)
		{
			fileManage::saveScriptAs(eVerticalScriptEditHandle, parentWindow, eVerticalParentScriptPathString, eVerticalCurrentParentScriptName, eVerticalScriptNameTextHandle,
				eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved);
		}
		fileManage::lookForPredefinedScripts(eVerticalScriptEditHandle, eVerticalViewCombo.hwnd);
		return 0;
	}
	int saveVerticalScriptAs(HWND parentWindow)
	{
		fileManage::saveScriptAs(eVerticalScriptEditHandle, parentWindow, eVerticalParentScriptPathString, eVerticalCurrentParentScriptName, eVerticalScriptNameTextHandle,
			eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved);
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
		fileManage::lookForPredefinedScripts(eVerticalScriptEditHandle, eVerticalViewCombo.hwnd);
		return 0;
	}

	int newHorizontalScript(HWND parentWindow)
	{
		if (eHorizontalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("horizontal", eHorizontalScriptEditHandle, parentWindow, eHorizontalCurrentParentScriptName, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved,
				eHorizontalParentScriptPathString, eHorizontalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont = 0)
			{
				return -1;
			}
		}
		fileManage::newScript("DEFAULT_HORIZONTAL_SCRIPT.script", eHorizontalScriptEditHandle, eHorizontalParentScriptPathString, eHorizontalCurrentParentScriptName, eHorizontalScriptNameTextHandle,
							  eHorizontalScriptSaved, eHorizontalScriptSavedIndicatorHandle);
		fileManage::lookForPredefinedScripts(eHorizontalScriptEditHandle, eHorizontalViewCombo.hwnd);
		eHorizontalViewScriptPathString = eHorizontalParentScriptPathString;
		strcpy_s(eHorizontalCurrentViewScriptName, eHorizontalCurrentParentScriptName);
		selectInCombo(eHorizontalViewCombo.hwnd, "Parent Script");
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
		return 0;
	}
	int openHorizontalScript(HWND parentWindow)
	{
		if (eVerticalScriptSaved == false)
		{
			// check if the user wants to save
			int cont = fileManage::checkSaveScript("horizontal", eHorizontalScriptEditHandle, parentWindow, eHorizontalCurrentParentScriptName, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved,
				eHorizontalParentScriptPathString, eHorizontalScriptNameTextHandle);
			// this occurs if the user presses cancel. Just break, don't open.
			if (cont == 0)
			{
				return -1;
			}
		}
		fileManage::openScript(parentWindow, eHorizontalParentScriptPathString, eHorizontalCurrentParentScriptName, eHorizontalScriptEditHandle, eHorizontalScriptSavedIndicatorHandle,
			eHorizontalScriptNameTextHandle, eHorizontalScriptSaved, true, true, eCurrentHorizontalViewIsParent);
		fileManage::lookForPredefinedScripts(eHorizontalScriptEditHandle, eHorizontalViewCombo.hwnd);
		eHorizontalViewScriptPathString = eHorizontalParentScriptPathString;
		strcpy_s(eHorizontalCurrentViewScriptName, eHorizontalCurrentParentScriptName);

		selectInCombo(eHorizontalViewCombo.hwnd, "Parent Script");

		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);

		return 0;
	}

	int saveHorizontalScript(HWND parentWindow)
	{
		// try to save
		int success = fileManage::saveScript(eHorizontalScriptEditHandle, eHorizontalParentScriptPathString, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved);
		// this occurs if save failed because there was no previous name to save to.
		if (success == 0)
		{
			fileManage::saveScriptAs(eHorizontalScriptEditHandle, parentWindow, eHorizontalParentScriptPathString, eHorizontalCurrentParentScriptName, eHorizontalScriptNameTextHandle,
				eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved);
		}
		// refresh this
		fileManage::lookForPredefinedScripts(eHorizontalScriptEditHandle, eHorizontalViewCombo.hwnd);
		return 0;
	}
	int saveHorizontalScriptAs(HWND parentWindow)
	{
		fileManage::saveScriptAs(eHorizontalScriptEditHandle, parentWindow, eHorizontalParentScriptPathString, eHorizontalCurrentParentScriptName, eHorizontalScriptNameTextHandle,
			eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved);
		eConfigurationSaved = false;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
		fileManage::lookForPredefinedScripts(eHorizontalScriptEditHandle, eHorizontalViewCombo.hwnd);
		return 0;
	}

	int newExperimentType(HWND parentWindow)
	{
		TCHAR* result = 0;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -1;
		}
		std::string tempExpName(result);
		delete[] result;
		std::string dirName = EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH + "\\" + tempExpName;
		if (dirName == "")
		{
			// user entered nothing.
			return -2;
		}
		eCurrentExperimentFolder = dirName;
		// Create the new file directory
		CreateDirectory(eCurrentExperimentFolder.c_str(), 0);
		// create new experiment configuration file and save it.
		std::string tempExperimentConfigPathString(eExperimentConfigPathString);
		std::string tempExperimentName(eCurrentExperimentName);
		eCurrentExperimentName = tempExpName;
		eExperimentConfigPathString = eCurrentExperimentFolder + "\\" + tempExpName + ".experimentConfig";
		fileManage::saveConfig();
		eCurrentExperimentName = tempExperimentName;
		eExperimentConfigPathString = tempExperimentConfigPathString;
		/// Reset Stuffs
		fileManage::reloadCombo(eExperimentTypeCombo, EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH, "*", eCurrentExperimentName);
		return 0;
	}
	int newCategory(HWND parentWindow)
	{

		if (eCurrentExperimentFolder == "")
		{
			MessageBox(0, "Please select the experiment before adding a new configuration.", 0, 0);
			return -1;
		}
		TCHAR* result = 0;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -2;
		}
		std::string categoryName(result);
		delete[] result;
		std::string dirName = eCurrentExperimentFolder + "\\" + categoryName;
		if (dirName == "")
		{
			// user entered nothing.
			return -3;
		}
		// Create the new file directory
		CreateDirectory(dirName.c_str(), 0);

		/// Reset the combo
		fileManage::reloadCombo(eCategoryCombo, eCurrentExperimentFolder, "*", eCurrentCategoryName);
		SendMessage(eConfigurationDisplayInScripting, WM_SETTEXT, 0, (LPARAM)"");
		return 0;
	}
	int saveConfigurationAs(HWND parentWindow)
	{
		if (eCurrentCategoryFolder == "" || eCurrentExperimentFolder == "")
		{
			MessageBox(0, "Please Select an Experiment and Configuration before creating a new SubConfiguration", 0, 0);
		}
		TCHAR* result = NULL;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -1;
		}
		std::string subConfigName(result);
		delete[] result;
		if (subConfigName == "")
		{
			// user entered nothing.
			return -2;
		}
		eCurrentConfigurationName = subConfigName;
		if (eCurrentOrientation == "Horizontal")
		{
			eCurrentConfigurationLocation = eCurrentCategoryFolder + "\\" + std::string(subConfigName) + ".hSubConfig";
		}
		else if (eCurrentOrientation == "Vertical")
		{
			eCurrentConfigurationLocation = eCurrentCategoryFolder + "\\" + std::string(subConfigName) + ".vSubConfig";
		}
		fileManage::saveConfig();

		/// Experiment Type Combo Box
		// Get all files in the relevant directory.
		std::vector<std::string> subconfigNames;
		if (eCurrentOrientation == "Horizontal")
		{
			subconfigNames = fileManage::searchForFiles(eCurrentCategoryFolder, "*.hSubConfig");
		}
		else if (eCurrentOrientation == "Vertical")
		{
			subconfigNames = fileManage::searchForFiles(eCurrentCategoryFolder, "*.vSubConfig");
		}
		/// Reset stuffs
		SendMessage(eConfigurationCombo, CB_RESETCONTENT, 0, 0);
		// Send list to object
		int currentInc = -1;
		for (int comboInc = 0; comboInc < subconfigNames.size(); comboInc++)
		{
			if (subConfigName == subconfigNames[comboInc])
			{
				currentInc = comboInc;
			}
			SendMessage(eConfigurationCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(subconfigNames[comboInc].c_str())));
		}
		// Set initial value
		SendMessage(eConfigurationCombo, CB_SETCURSEL, currentInc, 0);

		return 0;
	}

	int renameCurrentExperimentType(HWND parentWindow)
	{
		/// check that there IS a current experiment type.
		// Send CB_GETCURSEL message to get the index of the selected list item.
		long long itemIndex = SendMessage(eExperimentTypeCombo, CB_GETCURSEL, 0, 0);
		if (itemIndex == -1)
		{
			MessageBox(0, "Please select an experiment to rename", 0, 0);
			// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
			// is blank. just break out, this is fine.
			return -1;
		}
		TCHAR experimentConfigToOpen[256];
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(eExperimentTypeCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);

		eCurrentExperimentFolder = EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH + "\\" + std::string(experimentConfigToOpen);
		std::string currentExperimentConfigName = eCurrentExperimentFolder + "\\" + std::string(experimentConfigToOpen) + ".experimentConfig";
		TCHAR* result = 0;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -1;
		}
		// first rename the config file inside the folder.
		eCurrentExperimentName = std::string(result);
		delete[] result;
		std::string newConfigName = eCurrentExperimentFolder + "\\" + eCurrentExperimentName + ".experimentConfig";
		MoveFile(currentExperimentConfigName.c_str(), newConfigName.c_str());

		std::string newExperimentPath = EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH + "\\" + eCurrentExperimentName;
		// move the folder.
		MoveFile(eCurrentExperimentFolder.c_str(), newExperimentPath.c_str());
		eCurrentExperimentFolder = newExperimentPath;
		// Reload Files
		fileManage::reloadCombo(eExperimentTypeCombo, EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH, "*", eCurrentExperimentName);

		eCurrentCategoryFolder = eCurrentExperimentFolder + "\\" + eCurrentCategoryName;
		eIntensityParentScriptPathString = eCurrentCategoryFolder + "\\" + eIntensityCurrentParentScriptName + ".script";
		eHorizontalParentScriptPathString = eCurrentCategoryFolder + "\\" + eHorizontalCurrentParentScriptName + ".script";
		eVerticalParentScriptPathString = eCurrentCategoryFolder + "\\" + eVerticalCurrentParentScriptName + ".script";
		return 0;
	}
	int deleteCurrentExperimentType(HWND parentWindow)
	{
		/// get the current name
		long long itemIndex = SendMessage(eExperimentTypeCombo, CB_GETCURSEL, 0, 0);
		if (itemIndex == -1)
		{
			MessageBox(0, "Please Select an Experiment to Delete.", 0, 0);
			// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
			// is blank. just break out, this is fine.
			return -1;
		}
		TCHAR experimentConfigToOpen[256];
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(eExperimentTypeCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);

		eCurrentExperimentFolder = EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH + "\\" + std::string(experimentConfigToOpen);
		std::string msgString = "Warning! You are about to delete the entire experiment folder: " + eCurrentExperimentFolder + ", and all configurations therein. Are you sure you want to continue?";
		int answer = MessageBox(0, msgString.c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			return -2;
		}
		
		int len = strlen(eCurrentExperimentFolder.c_str()) + 2; // required to set 2 nulls at end of argument to SHFileOperation.
		char* tempdir = (char*)malloc(len);
		memset(tempdir, 0, len);
		strcpy_s(tempdir, len, eCurrentExperimentFolder.c_str());
		SHFILEOPSTRUCT file_op = {NULL, FO_DELETE, tempdir, "", FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, false, 0,	""};
		
		if (SHFileOperation(&file_op) != 0)
		{
			std::string errMsg = "Delete Failed!";
			MessageBox(0, errMsg.c_str(), 0, 0);
		}
		free(tempdir);
		// set this variable to blank.
		eCurrentExperimentFolder = "";
		eCurrentExperimentName = "";
		// reload combo
		fileManage::reloadCombo(eExperimentTypeCombo, EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH, "*", "__none__");

		return 0;
	}
	int renameCurrentCategory(HWND parentWindow)
	{

		/// check that there IS a current experiment type.
		// Send CB_GETCURSEL message to get the index of the selected list item.
		long long itemIndex = SendMessage(eCategoryCombo, CB_GETCURSEL, 0, 0);
		if (itemIndex == -1)
		{
			MessageBox(0, "Please select a configuration to to rename", 0, 0);
			// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
			// is blank. just break out, this is fine.
			return -1;
		}
		TCHAR configurationToOpen[256];
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(eCategoryCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)configurationToOpen);

		eCurrentCategoryFolder = eCurrentExperimentFolder + "\\" + std::string(configurationToOpen);
		TCHAR* result = 0;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -1;
		}
		// first rename the config file inside the folder.the f
		std::string newCategoryName(result);
		delete[] result;
		std::string newConfigurationPath = eCurrentExperimentFolder + "\\" + newCategoryName;
		
		// move the folder.
		if (MoveFile(eCurrentCategoryFolder.c_str(), newConfigurationPath.c_str()) == 0)
		{
			int a = GetLastError();
			std::string errMsg = "ERROR: Renaming of File Failed: " + std::to_string(a);
			MessageBox(0, errMsg.c_str(), 0, 0);
			return -1;
		}
		eCurrentCategoryFolder = newConfigurationPath;
		// Reload Files
		fileManage::reloadCombo(eCategoryCombo, eCurrentExperimentFolder, "*", newCategoryName);

		eIntensityParentScriptPathString = eCurrentCategoryFolder + "\\" + eIntensityCurrentParentScriptName + ".script";
		eHorizontalParentScriptPathString = eCurrentCategoryFolder + "\\" + eHorizontalCurrentParentScriptName + ".script";
		eVerticalParentScriptPathString = eCurrentCategoryFolder + "\\" + eVerticalCurrentParentScriptName + ".script";

		return 0;
	}
	int deleteCurrentCategory(HWND parentWindow)
	{
		/// get the current name
		long long itemIndex = SendMessage(eCategoryCombo, CB_GETCURSEL, 0, 0);
		if (itemIndex == -1)
		{
			MessageBox(0, "Please select a configuration to delete.", 0, 0);
			// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
			// is blank. just break out, this is fine.
			return -1;
		}
		TCHAR conifgurationName[256];
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(eCategoryCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)conifgurationName);

		eCurrentCategoryFolder = eCurrentExperimentFolder + "\\" + std::string(conifgurationName);
		std::string msgString = "Warning! You are about to delete the entire configuration folder: " + eCurrentCategoryFolder + ", and all specifics therein. Are you sure you want to continue?";
		int answer = MessageBox(0, msgString.c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			return -2;
		}
		/// Delete the stuff
		int len = strlen(eCurrentCategoryFolder.c_str()) + 2; // required to set 2 nulls at end of argument to SHFileOperation.
		char* tempdir = (char*)malloc(len);
		memset(tempdir, 0, len);
		strcpy_s(tempdir, len, eCurrentCategoryFolder.c_str());
		
		SHFILEOPSTRUCT file_op = { NULL, FO_DELETE, tempdir, "", FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, false, 0,	"" };
		
		if (SHFileOperation(&file_op) != 0)
		{
			std::string errMsg = "Delete Failed!";
			MessageBox(0, errMsg.c_str(), 0, 0);
		}
		free(tempdir);
		eCurrentCategoryFolder == "";
		// reload combo
		fileManage::reloadCombo(eCategoryCombo, eCurrentExperimentFolder, "*", "__none__");

		return 0;
	}
	int newConfiguration(HWND parentWindow)
	{
		if (eCurrentCategoryFolder == "" || eCurrentExperimentFolder == "")
		{
			MessageBox(0, "Please Select an Experiment and Configuration before creating a new SubConfiguration", 0, 0);
		}
		TCHAR* result = NULL;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -1;
		}
		std::string subConfigName(result);
		delete[] result;
		if (subConfigName == "")
		{
			// user entered nothing.
			return -1;
		}
		std::string newConfiguration;
		if (eCurrentOrientation == "Horizontal")
		{
			newConfiguration = eCurrentCategoryFolder + "\\" + std::string(subConfigName) + ".hSubConfig";
		}
		else if (eCurrentOrientation == "Vertical")
		{
			newConfiguration = eCurrentCategoryFolder + "\\" + std::string(subConfigName) + ".vSubConfig";
		}
		std::fstream subConfigSaveFile(newConfiguration.c_str(), std::fstream::out);
		//
		if (!subConfigSaveFile.is_open())
		{
			MessageBox(0, "Details File failed to open!", 0, 0);
		}

		// vertical Script File Address
		subConfigSaveFile << "\n";
		// horizontal Script File Address
		subConfigSaveFile << "\n";
		// Intensity Script File Address
		subConfigSaveFile << "\n";

		/// Experiment Type Combo Box
		
		// reload.
		if (eCurrentOrientation == "Horizontal")
		{
			fileManage::reloadCombo(eConfigurationCombo, eCurrentCategoryFolder, "*.hSubConfig", eCurrentConfigurationName);
			
		}
		else if (eCurrentOrientation == "Vertical")
		{
			fileManage::reloadCombo(eConfigurationCombo, eCurrentCategoryFolder, "*.vSubConfig", eCurrentConfigurationName);
		}

		return 0;
	}
	int renameCurrentConfiguration(HWND parentWindow)
	{
		/// check that there IS a current experiment type.
		// Send CB_GETCURSEL message to get the index of the selected list item.
		long long itemIndex = SendMessage(eConfigurationCombo, CB_GETCURSEL, 0, 0);
		if (itemIndex == -1)
		{
			MessageBox(0, "Please select a details option to rename.", 0, 0);
			// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
			// is blank. just break out, this is fine.
			return -1;
		}
		TCHAR detailsName[256];
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(eConfigurationCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)detailsName);
		eCurrentConfigurationLocation = eCurrentCategoryFolder + "\\" + std::string(detailsName);
		eCurrentConfigurationName = detailsName;
		if (eCurrentOrientation == "Horizontal")
		{
			eCurrentConfigurationLocation += ".hSubConfig";
		}
		else if (eCurrentOrientation == "Vertical")
		{
			eCurrentConfigurationLocation += ".vSubConfig";
		}
		
		TCHAR* result = 0;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		if (result == NULL)
		{
			// canceled
			return -1;
		}
		// first rename the config file inside the folder.
		eCurrentConfigurationName = std::string(result);
		delete[] result;
		std::string newDetailsLocation;
		if (eCurrentOrientation == "Horizontal")
		{
			newDetailsLocation = eCurrentCategoryFolder + "\\" + eCurrentConfigurationName + ".hSubConfig";
		}
		else if (eCurrentOrientation == "Vertical")
		{
			newDetailsLocation = eCurrentCategoryFolder + "\\" + eCurrentConfigurationName + ".vSubConfig";
		}
		MoveFile(eCurrentConfigurationLocation.c_str(), newDetailsLocation.c_str());
		eCurrentConfigurationLocation = newDetailsLocation;
		// Reload Files
		if (eCurrentOrientation == "Horizontal")
		{
			fileManage::reloadCombo(eConfigurationCombo, eCurrentCategoryFolder, "*.hSubConfig", eCurrentConfigurationName);
		}
		else if (eCurrentOrientation == "Vertical")
		{
			fileManage::reloadCombo(eConfigurationCombo, eCurrentCategoryFolder, "*.vSubConfig", eCurrentConfigurationName);
		}

		return 0;
	}
	int deleteCurrentConfiguration(HWND parentWindow)
	{
		/// get the current name
		long long itemIndex = SendMessage(eConfigurationCombo, CB_GETCURSEL, 0, 0);
		if (itemIndex == -1)
		{
			MessageBox(0, "Please select a configuration to delete.", 0, 0);
			// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
			// is blank. just break out, this is fine.
			return -1;
		}
		TCHAR conifgurationName[256];
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(eConfigurationCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)conifgurationName);

		std::string fileToDelete = eCurrentCategoryFolder + "\\" + std::string(conifgurationName);
		if (eCurrentOrientation == "Horizontal")
		{
			fileToDelete += ".hSubConfig";
		}
		else if (eCurrentOrientation == "Vertical")
		{
			fileToDelete += ".vSubConfig";
		}
		std::string msgString = "Warning! You are about to delete the configuration file: " + fileToDelete + ", and all specifics therein. Are you sure you want to continue?";
		int answer = MessageBox(0, msgString.c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			return -2;
		}
		/// Delete the stuff
		int len = strlen(fileToDelete.c_str()) + 2; // required to set 2 nulls at end of argument to SHFileOperation.
		char* tempdir = (char*)malloc(len);
		memset(tempdir, 0, len);
		strcpy_s(tempdir, len, fileToDelete.c_str());

		SHFILEOPSTRUCT file_op = { NULL, FO_DELETE, tempdir, "", FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, false, 0,	"" };

		if (SHFileOperation(&file_op) != 0)
		{
			std::string errMsg = "Delete Failed!";
			MessageBox(0, errMsg.c_str(), 0, 0);
		}
		free(tempdir);
		eCurrentCategoryFolder == "";
		if (eCurrentOrientation == "Horizontal")
		{
			// reload combo
			fileManage::reloadCombo(eConfigurationCombo, eCurrentCategoryFolder, "*.hSubConfig", "__none__");

		}
		else if (eCurrentOrientation == "Vertical")
		{
			fileManage::reloadCombo(eConfigurationCombo, eCurrentCategoryFolder, "*.vSubConfig", "__none__");
		}
		return 0;
	}
	//
	int newSequence(HWND parentWindow)
	{
		// TODO:::
		// check requirements
		
		// prompt for save

		// prompt for name
		TCHAR* result = NULL;
		result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
		//
		if (result == NULL || std::string(result) == "")
		{
			// user canceled or entered nothing
			return -1;
		}
		// try to open the file.
		std::fstream sequenceFile(eCurrentCategoryFolder + "\\" + result + ".seq", std::fstream::out);
		if (!sequenceFile.is_open())
		{
			MessageBox(0, "Couldn't create a file with this sequence name! Make sure there are no forbidden characters in your name.", 0, 0);
			return -1;
		}
		eCurrentSequenceName = std::string(result);
		sequenceFile << eCurrentSequenceName + "\n";
		// output current configuration
		eSequenceFileNames.clear();
		if (eCurrentConfigurationName != "")
		{
			if (eCurrentOrientation == "Horizontal")
			{
				eSequenceFileNames.push_back(eCurrentConfigurationName + ".hSubConfig");
			}
			else if (eCurrentOrientation == "Vertical")
			{
				eSequenceFileNames.push_back(eCurrentConfigurationName + ".vSubConfig");
			}
		}
		// reset the display and combo
		SendMessage(eSequenceDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
		if (eSequenceFileNames.size() != 0)
		{
			appendText("1. " + eSequenceFileNames[0] + "\r\n", IDC_SEQUENCE_DISPLAY, eMainWindowHandle);
		}
		// add to the comboBox.
		SendMessage(eSequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)eCurrentSequenceName.c_str());
		// find that one and select it
		int count = SendMessage(eSequenceCombo.hwnd, CB_GETCOUNT, 0, 0);
		TCHAR comboText[256];
		for (int comboInc = 0; comboInc < count; comboInc++)
		{
			SendMessage(eSequenceCombo.hwnd, CB_GETLBTEXT, comboInc, (LPARAM)comboText);
			if (std::string(comboText) == eCurrentSequenceName)
			{
				SendMessage(eSequenceCombo.hwnd, CB_SETCURSEL, comboInc, 0);
				break;
			}
		}

		return 0;
	}
	//
	int deleteSequence(HWND parentWindow)
	{
		int answer = MessageBox(0, ("Are you sure you want to delete the sequence named " + eCurrentSequenceName + "?").c_str(), 0, MB_OKCANCEL);
		// 
		if (answer == IDOK)
		{
			if (DeleteFile((eCurrentCategoryFolder + "\\" + eCurrentSequenceName + ".seq").c_str()) == 0)
			{
				MessageBox(0, "Delete Failed!", 0, 0);
			}
		}
		return 0;
	}
	int addToSequence(HWND parentWindow)
	{
		if (eCurrentOrientation == "Horizontal")
		{
			eSequenceFileNames.push_back(eCurrentConfigurationName + ".hSubConfig");
		}
		else if (eCurrentOrientation == "Vertical")
		{
			eSequenceFileNames.push_back(eCurrentConfigurationName + ".vSubConfig");
		}
		// add text to display.
		appendText(std::to_string(eSequenceFileNames.size()) + ". " + eSequenceFileNames.back() + "\r\n", IDC_SEQUENCE_DISPLAY, parentWindow);
		return 0;
	}
	int resetSequence(HWND parentWindow)
	{
		eSequenceFileNames.clear();
		// re-ad the current one if one is selected. Which it should be.
		if (eCurrentConfigurationName != "")
		{
			if (eCurrentOrientation == "Horizontal")
			{
				eSequenceFileNames.push_back(eCurrentConfigurationName + ".hSubConfig");
			}
			else if (eCurrentOrientation == "Vertical")
			{
				eSequenceFileNames.push_back(eCurrentConfigurationName + ".vSubConfig");
			}
		}
		// change the edit control
		SendMessage(eSequenceDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
		appendText("1. " + eSequenceFileNames[0] + "\r\n", IDC_SEQUENCE_DISPLAY, eMainWindowHandle);
		return 0;
	}
	int saveSequence(HWND parentWindow)
	{
		if (eCurrentSequenceName == "NO SEQUENCE")
		{
			// nothing to save;
			return 0;
		}
		// if not saved...
		if (eCurrentSequenceName == "")
		{
			TCHAR* result = NULL;
			result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
			if (result == NULL)
			{
				return -1;
			}
			eCurrentSequenceName = std::string(result);
			// prompt for name.
		}
		std::fstream sequenceSaveFile(eCurrentCategoryFolder + "\\" + eCurrentSequenceName + ".seq", std::fstream::out);
		if (!sequenceSaveFile.is_open())
		{
			MessageBox(0, "ERROR: Couldn't open sequence file for saving!", 0, 0);
			return -1;
		}
		for (int sequenceInc = 0; sequenceInc < eSequenceFileNames.size(); sequenceInc++)
		{
			sequenceSaveFile << eSequenceFileNames[sequenceInc] + "\n";
		}
		sequenceSaveFile.close();
		return 0;
	}
	
	int saveProfile(HWND parentWindow)
	{
		// try to save
		fileManage::saveConfig();
		//... If failed message pops up.
		menuAndAcceleratorFunctions::saveSequence(parentWindow);
		eConfigurationSaved = true;
		SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
		eExperimentSaved = true;
		SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
		return 0;
	}

	int helpWindow(HWND parentWindow)
	{
		HWND infoRet = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SCRIPT_HELP_DIALOG), parentWindow, (DLGPROC)scriptWriteHelpProc);
		if (infoRet != NULL)
		{
			ShowWindow(infoRet, SW_SHOW);
			SetDlgItemText(infoRet, IDC_HELP_EDIT, SCRIPT_INFO_TEXT);
		}
		else
		{
			MessageBox(parentWindow, "CreateDialog returned NULL", "Warning!", MB_OK | MB_ICONINFORMATION);
		}
		return 0;
	}

	int reloadNIAWGDefaults(HWND parentWindow)
	{

		if (eSystemIsRunning)
		{
			MessageBox(0, "The system is currently running. You cannot reload the default waveforms while the system is running. Please restart "
				"the system before attempting to reload default waveforms.", 0, 0);
			return -1;
		}
		int choice = MessageBox(0, "Reload the default waveforms from (presumably) updated files? Please make sure that the updated files are "
			"syntactically correct, or else the program will crash.", 0, MB_OKCANCEL);
		if (choice == IDCANCEL)
		{
			return -2;
		}
		// delete the old stuff.
		delete eDefault_hConfigMixedWaveform;
		delete eDefault_vConfigMixedWaveform;
		delete eDefault_hConfigScript;
		delete eDefault_vConfigScript;

		std::string tempOrientation = eCurrentOrientation;

		int waveformCount = 0;
		std::vector<int> defPredWaveLocs;
		std::vector<std::string> libWaveformArray[20];
		bool fileOpenedStatus[20] = { 0 };
		SOCKET ConnectSocket = INVALID_SOCKET;


		// Contains the names of predefined x waveforms
		std::vector<std::string> defXPredWaveformNames;
		// Contains the names of predefined y waveforms
		std::vector<std::string> defYPredWaveformNames;

		std::string default_hConfigScriptString;
		std::string default_vConfigScriptString;
		// first line of every script is script "name".
		default_hConfigScriptString = "script DefaultHConfigScript\n";
		default_vConfigScriptString = "script DefaultVConfigScript\n";
		// Vectors of structures that each contain all the basic information about a single waveform. Most of this (pardon the default waveforms) gets erased after 
		// an experiment.
		std::vector<waveData> allXWaveformParameters, allYWaveformParameters;
		// Vectors of flags that signify whether a given waveform is being varied or not.
		std::vector<bool> xWaveformIsVaried, yWaveformIsVaried, intensityIsVaried;
		bool userScriptIsWritten = false;
		int defPredWaveformCount = 0;

		// initialize default open the default files.

		std::vector<std::fstream> default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, default_vConfigVerticalScriptFile, default_vConfigHorizontalScriptFile;
		default_hConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.script"));
		default_hConfigHorizontalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.script"));
		default_vConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.script"));
		default_vConfigHorizontalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.script"));

		// check errors
		if (!default_hConfigVerticalScriptFile[0].is_open())
		{
			MessageBox(NULL, "ERROR: Couldn't open default file. The previous default waveforms are still running, "
				"but you need to correct the mistake and reload the default waveforms before another run.", "ERROR", MB_OK);
			return -1;
		}
		if (!default_hConfigHorizontalScriptFile[0].is_open())
		{
			MessageBox(NULL, "FATAL ERROR: Couldn't open default file. The previous default waveforms are still running, "
				"but you need to correct the mistake and reload the default waveforms before another run.", "ERROR", MB_OK);
			return -1;
		}
		// check errors
		if (!default_vConfigVerticalScriptFile[0].is_open())
		{
			MessageBox(NULL, "ERROR: Couldn't open default file. The previous default waveforms are still running, "
				"but you need to correct the mistake and reload the default waveforms before another run.", "ERROR", MB_OK);
			return -1;
		}
		if (!default_vConfigHorizontalScriptFile[0].is_open())
		{
			MessageBox(NULL, "ERROR: Couldn't open default file. The previous default waveforms are still running, "
				"but you need to correct the mistake and reload the default waveforms before another run.", "ERROR", MB_OK);
			return -1;
		}

		// analyze the input files and create the xy-script. Originally, I thought I'd write the script in two parts, the x and y parts, but it turns out not to 
		// work like I thought it did. If  I'd known this from the start, I probably wouldn't have created this subroutine, except perhaps for the fact that it get 
		// called recursively by predefined scripts in the instructions file.
		/// Create Horizontal Configuration
		eCurrentOrientation = "Horizontal";
		if (myErrorHandler(myNIAWG::createXYScript(default_hConfigVerticalScriptFile[0], default_hConfigHorizontalScriptFile[0], default_hConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
			eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
			fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, ""),
			"", ConnectSocket, default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, false, eError, eSessionHandle, false, "", false, false)
			== true)
		{
			MessageBox(0, "ERROR: Creation of Default Waveforms and Default Script Has Failed! The previous default waveforms are still running, "
				"but you need to correct the mistake and reload the default waveforms before another run.", 0, MB_OK);
			return -3;
		}
		// the script file must end with "end script".
		default_hConfigScriptString += "end Script";

		// Convert script string to ViConstString. +1 for a null character on the end.
		eDefault_hConfigScript = new ViChar[default_hConfigScriptString.length() + 1];
		sprintf_s(eDefault_hConfigScript, default_hConfigScriptString.length() + 1, "%s", default_hConfigScriptString.c_str());
		strcpy_s(eDefault_hConfigScript, default_hConfigScriptString.length() + 1, default_hConfigScriptString.c_str());
		// now do the vertical one.
		eCurrentOrientation = "Vertical";

		if (myErrorHandler(myNIAWG::createXYScript(default_vConfigVerticalScriptFile[0], default_vConfigHorizontalScriptFile[0],
												   default_vConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
												   eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
												   fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, ""),
			"", ConnectSocket, default_vConfigVerticalScriptFile, default_vConfigHorizontalScriptFile, false, eError, eSessionHandle, userScriptIsWritten, "", false, false)
			== true)
		{
			MessageBox(0, "ERROR: Creation of Default Waveforms and Default Script Has Failed! The previous default waveforms are still running, "
				"but you need to correct the mistake and reload the default waveforms before another run.", 0, MB_OK);
			return -4;
		}
		// the script file must end with "end script".
		default_vConfigScriptString += "end Script";
		// Convert script string to ViConstString. +1 for a null character on the end.
		eDefault_vConfigScript = new ViChar[default_vConfigScriptString.length() + 1];
		sprintf_s(eDefault_vConfigScript, default_vConfigScriptString.length() + 1, "%s", default_vConfigScriptString.c_str());
		strcpy_s(eDefault_vConfigScript, default_vConfigScriptString.length() + 1, default_vConfigScriptString.c_str());

		// go back to whatever configuration you were in.
		eCurrentOrientation = tempOrientation;

		// clear NIAWG memory.
		myNIAWG::NIAWG_CheckDefaultError(niFgen_ClearArbMemory(eSessionHandle));

		ViInt32 waveID;
		if (eCurrentOrientation == "Horizontal")
		{
			// create waveform (necessary?)
			myNIAWG::NIAWG_CheckDefaultError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID));
			// allocate waveform into the device memory
			myNIAWG::NIAWG_CheckDefaultError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2));
			// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
			myNIAWG::NIAWG_CheckDefaultError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform));
			// rewrite the script. default_hConfigScript should still be valid.
			myNIAWG::NIAWG_CheckDefaultError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript));
			// start generic waveform to maintain power output to AOM.
			myNIAWG::NIAWG_CheckDefaultError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE));
			myNIAWG::NIAWG_CheckDefaultError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript"));
			eCurrentScript = "DefaultHConfigScript";

		}
		else if (eCurrentOrientation == "Vertical")
		{
			// create waveform (necessary?)
			myNIAWG::NIAWG_CheckDefaultError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID));
			// allocate waveform into the device memory
			myNIAWG::NIAWG_CheckDefaultError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2));
			// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
			myNIAWG::NIAWG_CheckDefaultError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform));
			// rewrite the script. default_hConfigScript should still be valid.
			myNIAWG::NIAWG_CheckDefaultError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript));
			// start generic waveform to maintain power output to AOM.
			myNIAWG::NIAWG_CheckDefaultError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE));
			myNIAWG::NIAWG_CheckDefaultError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript"));
			eCurrentScript = "DefaultVConfigScript";
		}
		// Initiate Generation.
		myNIAWG::NIAWG_CheckDefaultError(niFgen_InitiateGeneration(eSessionHandle));
		appendText("Reloaded Default Waveforms.\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
		appendText("Initialized Default Waveform.\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
		return 0;
	}
};
