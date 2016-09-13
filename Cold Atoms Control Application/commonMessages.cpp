#include "stdafx.h"
#include "commonMessages.h"
#include "Windows.h"
#include "textPromptDialogProcedure.h"
#include "appendText.h"
#include "myAgilent.h"
#include "myNIAWG.h"
#include "experimentThreadInputStructure.h"
#include "experimentProgrammingThread.h"
#include "scriptWriteHelpProc.h"
#include "myErrorHandler.h"
#include "beginningSettingsDialogProc.h"
#include "selectInCombo.h"
#include "getFileName.h"
#include "saveTextFileFromEdit.h"

namespace commonMessages
{
	// this functino handles messages that all windows can recieve, e.g. accelerator keys and menu messages.
	bool handleCommonMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
					case ID_FILE_MY_RUN:
					case ID_ACCELERATOR_F5:
					case ID_FILE_MY_WRITE_WAVEFORMS:
					{
						commonMessages::startSystem(window, wParam);
						break;
					}
					case ID_ACCELERATOR_ESC:
					case ID_FILE_ABORT_GENERATION:
					{
						// finish the abort.
						commonMessages::abortSystem(window);
						break;
					}
					/// File Management 
					case ID_FILE_SAVEALL:
					{
						// save the scripts
						commonMessages::saveHorizontalScript(window);
						commonMessages::saveVerticalScript(window);
						commonMessages::saveIntensityScript(window);
						// save the profile
						eProfile.saveEntireProfile();
						break;
					}
					case ID_PROFILE_SAVE_PROFILE:
					{
						eProfile.saveEntireProfile();
						break;
					}
					case ID_FILE_MY_EXIT:
					{
						commonMessages::exitProgram(window);
						break;
					}
					case ID_FILE_MY_INTENSITY_NEW:
					{
						commonMessages::newIntensityScript(window);
						break;
					}
					case ID_FILE_MY_INTENSITY_OPEN:
					{
						commonMessages::openIntensityScript(window);
						break;
					}
					case ID_FILE_MY_INTENSITY_SAVE:
					{
						commonMessages::saveIntensityScript(window);
						break;
					}
					case ID_FILE_MY_INTENSITY_SAVEAS:
					{
						commonMessages::saveIntensityScriptAs(window);
						break;
					}
					case ID_FILE_MY_VERTICAL_NEW:
					{
						commonMessages::newVerticalScript(window);
						break;
					}
					case ID_FILE_MY_VERTICAL_OPEN:
					{
						commonMessages::openVerticalScript(window);
						break;
					}
					case ID_FILE_MY_VERTICAL_SAVE:
					{
						commonMessages::saveVerticalScript(window);
						break;
					}
					case ID_FILE_MY_VERTICAL_SAVEAS:
					{
						commonMessages::saveVerticalScriptAs(window);
						break;
					}
					case ID_FILE_MY_HORIZONTAL_NEW:
					{
						commonMessages::newHorizontalScript(window);
						break;
					}
					case ID_FILE_MY_HORIZONTAL_OPEN:
					{
						commonMessages::openHorizontalScript(window);
						break;
					}
					case ID_FILE_MY_HORIZONTAL_SAVE:
					{
						commonMessages::saveHorizontalScript(window);
						break;
					}
					case ID_FILE_MY_HORIZONTAL_SAVEAS:
					{
						commonMessages::saveHorizontalScriptAs(window);
						break;
					}
					case ID_SEQUENCE_RENAMESEQUENCE:
					{
						eProfile.renameSequence();
						break;
					}
					case ID_SEQUENCE_ADD_TO_SEQUENCE:
					{
						eProfile.addToSequence(window);
						break;
					}
					case ID_SEQUENCE_SAVE_SEQUENCE:
					{
						eProfile.saveSequence();
						break;
					}
					case ID_SEQUENCE_NEW_SEQUENCE:
					{
						eProfile.newSequence(window);
						break;
					}
					case ID_SEQUENCE_RESET_SEQUENCE:
					{
						eProfile.loadNullSequence();
						break;
					}
					case ID_SEQUENCE_DELETE_SEQUENCE:
					{
						eProfile.deleteSequence();
						break;
					}
					case ID_HELP_SCRIPT:
					{
						commonMessages::helpWindow(window);
						break;
					}
					case ID_HELP_GENERALINFORMATION:
					{
						break;
					}
					case ID_NIAWG_RELOADDEFAULTWAVEFORMS:
					{
						commonMessages::reloadNIAWGDefaults(window);
						break;
					}
					case ID_EXPERIMENT_NEW_EXPERIMENT_TYPE:
					{
						eProfile.newExperiment();
						break;
					}
					case ID_EXPERIMENT_SAVEEXPERIMENTSETTINGS:
					{
						eProfile.saveExperimentOnly();
						break;
					}
					case ID_EXPERIMENT_SAVEEXPERIMENTSETTINGSAS:
					{
						eProfile.saveExperimentAs();
						break;
					}

					case ID_EXPERIMENT_RENAME_CURRENT_EXPERIMENT:
					{
						eProfile.renameExperiment();
						break;
					}
					case ID_EXPERIMENT_DELETE_CURRENT_EXPERIMENT:
					{
						eProfile.deleteExperiment();
						//commonMessages::deleteCurrentExperimentType(window);
						break;
					}
					case ID_CATEGORY_NEW_CATEGORY:
					{
						eProfile.newCategory();
						break;
					}
					case ID_CATEGORY_RENAME_CURRENT_CATEGORY:
					{
						eProfile.renameCategory();
						break;
					}
					case ID_CATEGORY_DELETE_CURRENT_CATEGORY:
					{
						eProfile.deleteCategory();
						break;
					}
					case ID_CATEGORY_SAVECATEGORYSETTINGS:
					{
						eProfile.saveCategoryOnly();
						break;
					}
					case ID_CATEGORY_SAVECATEGORYSETTINGSAS:
					{
						eProfile.saveCategoryAs();
						break;
					}
					case ID_CONFIGURATION_NEW_CONFIGURATION:
					{
						eProfile.newConfiguration();
						break;
					}
					case ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION:
					{
						eProfile.renameConfiguration();
						break;
					}
					case ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION:
					{
						eProfile.deleteConfiguration();
						break;
					}
					case ID_CONFIGURATION_SAVE_CONFIGURATION_AS:
					{
						eProfile.saveConfigurationAs();
						break;
					}
					case ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS:
					{
						eProfile.saveConfigurationOnly();
						break;
					}
				}
			}
		}
		return false;
	}
	int startSystem(HWND parentWindow, WPARAM wParam)
	{
		if (eSystemIsRunning)
		{
			int restart = MessageBox(0, "Restart Generation?", 0, MB_OKCANCEL);
			if (restart == IDOK)
			{

				
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
				if (eProfile.getOrientation() == "Horizontal")
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
				else if (eProfile.getOrientation() == "Vertical")
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
		if (eProfile.getSequenceNames().size() == 0)
		{
			appendText("ERROR: No configurations in current sequence! Please set some configurations to run in this sequence or set the null sequence.\r\n", 
						IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
			return -9;
		}
		// check config settings
		if (eProfile.allSettingsReadyCheck())
		{
			return -1;
		}
		if (eHorizontalNIAWGScript.checkSave())
		{
			return -1;
		}
		if (eVerticalNIAWGScript.checkSave())
		{
			return -1;
		}
		if (eIntensityAgilentScript.checkSave())
		{
			return -1;
		}

		std::string verticalNameString(eVerticalNIAWGScript.getScriptName());
		std::string horizontalNameString(eHorizontalNIAWGScript.getScriptName());
		std::string intensityNameString(eIntensityAgilentScript.getScriptName());
		std::string beginInfo = "Current Settings:\r\n=============================\r\n\r\n";
		std::string sequenceInfo = eProfile.getSequenceNamesString();
		if (sequenceInfo != "")
		{
			beginInfo += sequenceInfo;
		}
		else
		{
			beginInfo += "Vertical Script Name:............. " + std::string(verticalNameString);
			if (eVerticalNIAWGScript.savedStatus())
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Horizontal Script Name:........... " + std::string(horizontalNameString);
			if (eHorizontalNIAWGScript.savedStatus())
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Intensity Script Name:............ " + std::string(intensityNameString);
			if (eIntensityAgilentScript.savedStatus())
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
		}
		beginInfo += "\r\n";
		if (eVariables.getCurrentNumberOfVariables() == 0)
		{
			beginInfo += "Variable Names:................... NO VARIABLES\r\n";
		}
		else
		{
			beginInfo += "Variable Names:................... ";
			for (int varInc = 0; varInc < eVariables.getCurrentNumberOfVariables(); varInc++)
			{
				beginInfo += eVariables.getVariableInfo(varInc).name + " ";
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
		beginInfo += "\r\n";
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
			(*inputParams).threadGetVarFilesFromMaster = eGetVarFilesFromMaster;
			(*inputParams).threadSequenceFileNames = eProfile.getSequenceNames();
			(*inputParams).threadLogScriptAndParams = eLogScriptAndParams;
			(*inputParams).threadProgramIntensityOption = eProgramIntensityOption;
			(*inputParams).currentFolderLocation = (eProfile.getCurrentPathIncludingCategory());
			eMostRecentVerticalScriptNames = eVerticalNIAWGScript.getScriptPathAndName();
			eMostRecentHorizontalScriptNames = eHorizontalNIAWGScript.getScriptPathAndName();
			eMostRecentIntensityScriptNames = eIntensityAgilentScript.getScriptPathAndName();
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
		if (!TWEEZER_COMPUTER_SAFEMODE)
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

		if (eProfile.getOrientation() == "Horizontal")
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
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
		else if (eProfile.getOrientation() == "Vertical")
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
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
		if (!TWEEZER_COMPUTER_SAFEMODE)
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
		if (eHorizontalNIAWGScript.checkSave())
		{
			return true;
		}
		if (eVerticalNIAWGScript.checkSave())
		{
			return true;
		}
		if (eIntensityAgilentScript.checkSave())
		{
			return true;
		}
		if (eProfile.checkSaveEntireProfile())
		{
			return true;
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
		if (eIntensityAgilentScript.checkSave())
		{
			return true;
		}
		eIntensityAgilentScript.newScript();
		eProfile.updateConfigurationSavedStatus(false);
		eIntensityAgilentScript.updateScriptNameText();
		return 0;
	}
	int openIntensityScript(HWND parentWindow)
	{
		if (eIntensityAgilentScript.checkSave())
		{
			return true;
		}

		std::string intensityOpenName = getFileNameDialog(parentWindow);
		eIntensityAgilentScript.openParentScript(intensityOpenName);
		eProfile.updateConfigurationSavedStatus(false);
		eIntensityAgilentScript.updateScriptNameText();
		return 0;
	}
	int saveIntensityScript(HWND parentWindow)
	{
		eIntensityAgilentScript.saveScript();
		eIntensityAgilentScript.updateScriptNameText();
		return 0;
	}
	int saveIntensityScriptAs(HWND parentWindow)
	{
		std::string extensionNoPeriod = eVerticalNIAWGScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return -1;
		}
		extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
		std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod);
		eIntensityAgilentScript.saveScriptAs(newScriptAddress);
		eProfile.updateConfigurationSavedStatus(false);
		eIntensityAgilentScript.updateScriptNameText();
		return 0;
	}
	int newVerticalScript(HWND parentWindow)
	{
		if (eVerticalNIAWGScript.checkSave())
		{
			return true;
		}
		eVerticalNIAWGScript.newScript();
		eProfile.updateConfigurationSavedStatus(false);
		eVerticalNIAWGScript.updateScriptNameText();
		return 0;
	}
	int openVerticalScript(HWND parentWindow)
	{
		if (eVerticalNIAWGScript.checkSave())
		{
			return true;
		}
		std::string intensityOpenName = getFileNameDialog(parentWindow);
		eVerticalNIAWGScript.openParentScript(intensityOpenName);
		eProfile.updateConfigurationSavedStatus(false);
		eVerticalNIAWGScript.updateScriptNameText();
		return 0;
	}
	int saveVerticalScript(HWND parentWindow)
	{
		eVerticalNIAWGScript.saveScript();
		eVerticalNIAWGScript.updateScriptNameText();
		return 0;
	}
	int saveVerticalScriptAs(HWND parentWindow)
	{
		std::string extensionNoPeriod = eVerticalNIAWGScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return -1;
		}
		extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
		std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod);
		eVerticalNIAWGScript.saveScriptAs(newScriptAddress);
		eProfile.updateConfigurationSavedStatus(false);
		eVerticalNIAWGScript.updateScriptNameText();
		return 0;
	}	
	int newHorizontalScript(HWND parentWindow)
	{
		if (eHorizontalNIAWGScript.checkSave())
		{
			return true;
		}
		eHorizontalNIAWGScript.newScript();
		eProfile.updateConfigurationSavedStatus(false);
		eHorizontalNIAWGScript.updateScriptNameText();
		return 0;
	}
	int openHorizontalScript(HWND parentWindow)
	{
		if (eHorizontalNIAWGScript.checkSave())
		{
			return true;
		}
		std::string intensityOpenName = getFileNameDialog(parentWindow);
		eHorizontalNIAWGScript.openParentScript(intensityOpenName);
		eProfile.updateConfigurationSavedStatus(false);
		eHorizontalNIAWGScript.updateScriptNameText();
		return 0;
	}
	int saveHorizontalScript(HWND parentWindow)
	{
		eHorizontalNIAWGScript.saveScript();
		eHorizontalNIAWGScript.updateScriptNameText();
		return 0;
	}
	int saveHorizontalScriptAs(HWND parentWindow)
	{
		std::string extensionNoPeriod = eVerticalNIAWGScript.getExtension();
		if (extensionNoPeriod.size() == 0)
		{
			return -1;
		}
		extensionNoPeriod = extensionNoPeriod.substr(1, extensionNoPeriod.size());
		std::string newScriptAddress = saveTextFileFromEdit(parentWindow, extensionNoPeriod);
		eHorizontalNIAWGScript.saveScriptAs(newScriptAddress);
		eProfile.updateConfigurationSavedStatus(false);
		eHorizontalNIAWGScript.updateScriptNameText();
		return 0;
	}
		
	int saveProfile(HWND parentWindow)
	{
		//... If failed message pops up.
		eProfile.saveEntireProfile();
		eProfile.updateConfigurationSavedStatus(true);
		eHorizontalNIAWGScript.updateScriptNameText();
		eVerticalNIAWGScript.updateScriptNameText();
		eIntensityAgilentScript.updateScriptNameText();
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

		std::string tempOrientation = eProfile.getOrientation();

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
		std::vector<variable> noSingletons;
		/// Create Horizontal Configuration
		eProfile.setOrientation(HORIZONTAL_ORIENTATION);
		if (myErrorHandler(myNIAWG::analyzeNIAWGScripts(default_hConfigVerticalScriptFile[0], default_hConfigHorizontalScriptFile[0], default_hConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
			eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
			fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, "", noSingletons),
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
		eProfile.setOrientation(VERTICAL_ORIENTATION);

		if (myErrorHandler(myNIAWG::analyzeNIAWGScripts(default_vConfigVerticalScriptFile[0], default_vConfigHorizontalScriptFile[0],
												   default_vConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
												   eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
												   fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, "", noSingletons),
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
		eProfile.setOrientation(tempOrientation);

		// clear NIAWG memory.
		myNIAWG::NIAWG_CheckDefaultError(niFgen_ClearArbMemory(eSessionHandle));

		ViInt32 waveID;
		if (eProfile.getOrientation() == HORIZONTAL_ORIENTATION)
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
		else if (eProfile.getOrientation() == VERTICAL_ORIENTATION)
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
