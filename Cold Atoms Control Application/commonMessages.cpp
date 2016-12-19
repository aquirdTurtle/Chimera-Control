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
#include <array>
#include "MainWindow.h"
#include "postMyString.h"

// Functions called by all windows to do the same thing, mostly things that happen on menu presses.
namespace commonMessages
{
	// this function handles messages that all windows can recieve, e.g. accelerator keys and menu messages.
	bool handleCommonMessage(int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin)
	{
		switch (msgID)
		{
			case ID_FILE_MY_RUN:
			case ID_ACCELERATOR_F5:
			case ID_FILE_MY_WRITE_WAVEFORMS:
			{
				commonMessages::startSystem(parent->GetSafeHwnd(), msgID, scriptWin, mainWin);
				break;
			}
			case WM_CLOSE:
			case ID_ACCELERATOR_ESC:
			case ID_FILE_ABORT_GENERATION:
			{
				// finish the abort.
				commonMessages::abortSystem(parent->GetSafeHwnd(), scriptWin, mainWin);
				break;
			}
			/// File Management 
			case ID_FILE_SAVEALL:
			{
				// save the scripts
				scriptWin->saveHorizontalScript();
				scriptWin->saveVerticalScript();
				scriptWin->saveIntensityScript();
				// save the profile
				mainWin->profile.saveEntireProfile(scriptWin, mainWin);
				break;
			}
			case ID_PROFILE_SAVE_PROFILE:
			{
				mainWin->profile.saveEntireProfile(scriptWin, mainWin);
				break;
			}
			case ID_FILE_MY_EXIT:
			{
				commonMessages::exitProgram(parent->GetSafeHwnd(), scriptWin, mainWin);
				break;
			}
			case ID_FILE_MY_INTENSITY_NEW:
			{
				scriptWin->newIntensityScript();
				break;
			}
			case ID_FILE_MY_INTENSITY_OPEN:
			{
				scriptWin->openIntensityScript(parent->GetSafeHwnd());
				break;
			}
			case ID_FILE_MY_INTENSITY_SAVE:
			{
				scriptWin->saveIntensityScript();
				break;
			}
			case ID_FILE_MY_INTENSITY_SAVEAS:
			{
				scriptWin->saveIntensityScriptAs(parent->GetSafeHwnd());
				break;
			}
			case ID_FILE_MY_VERTICAL_NEW:
			{
				scriptWin->newVerticalScript();
				break;
			}
			case ID_FILE_MY_VERTICAL_OPEN:
			{
				scriptWin->openVerticalScript(parent->GetSafeHwnd());
				break;
			}
			case ID_FILE_MY_VERTICAL_SAVE:
			{
				scriptWin->saveVerticalScript();
				break;
			}
			case ID_FILE_MY_VERTICAL_SAVEAS:
			{
				scriptWin->saveVerticalScriptAs(parent->GetSafeHwnd());
				break;
			}
			case ID_FILE_MY_HORIZONTAL_NEW:
			{
				scriptWin->newHorizontalScript();
				break;
			}
			case ID_FILE_MY_HORIZONTAL_OPEN:
			{
				scriptWin->openHorizontalScript(parent->GetSafeHwnd());
				break;
			}
			case ID_FILE_MY_HORIZONTAL_SAVE:
			{
				scriptWin->saveHorizontalScript();
				break;
			}
			case ID_FILE_MY_HORIZONTAL_SAVEAS:
			{
				scriptWin->saveHorizontalScriptAs(parent->GetSafeHwnd());
				break;
			}
			case ID_SEQUENCE_RENAMESEQUENCE:
			{
				mainWin->profile.renameSequence();
				break;
			}
			case ID_SEQUENCE_ADD_TO_SEQUENCE:
			{
				mainWin->profile.addToSequence(parent);
				break;
			}
			case ID_SEQUENCE_SAVE_SEQUENCE:
			{
				mainWin->profile.saveSequence();
				break;
			}
			case ID_SEQUENCE_NEW_SEQUENCE:
			{
				mainWin->profile.newSequence(parent);
				break;
			}
			case ID_SEQUENCE_RESET_SEQUENCE:
			{
				mainWin->profile.loadNullSequence();
				break;
			}
			case ID_SEQUENCE_DELETE_SEQUENCE:
			{
				mainWin->profile.deleteSequence();
				break;
			}
			case ID_HELP_SCRIPT:
			{
				commonMessages::helpWindow(parent->GetSafeHwnd());
				break;
			}
			case ID_HELP_GENERALINFORMATION:
			{
				break;
			}
			case ID_NIAWG_RELOADDEFAULTWAVEFORMS:
			{
				//commonMessages::reloadNIAWGDefaults(parent->GetSafeHwnd());
				break;
			}
			case ID_EXPERIMENT_NEW_EXPERIMENT_TYPE:
			{
				mainWin->profile.newExperiment();
				break;
			}
			case ID_EXPERIMENT_SAVEEXPERIMENTSETTINGS:
			{
				mainWin->profile.saveExperimentOnly(mainWin);
				break;
			}
			case ID_EXPERIMENT_SAVEEXPERIMENTSETTINGSAS:
			{
				mainWin->profile.saveExperimentAs(mainWin);
				break;
			}

			case ID_EXPERIMENT_RENAME_CURRENT_EXPERIMENT:
			{
				mainWin->profile.renameExperiment(mainWin);
				break;
			}
			case ID_EXPERIMENT_DELETE_CURRENT_EXPERIMENT:
			{
				mainWin->profile.deleteExperiment();
				//commonMessages::deleteCurrentExperimentType(window);
				break;
			}
			case ID_CATEGORY_NEW_CATEGORY:
			{
				mainWin->profile.newCategory();
				break;
			}
			case ID_CATEGORY_RENAME_CURRENT_CATEGORY:
			{
				mainWin->profile.renameCategory();
				break;
			}
			case ID_CATEGORY_DELETE_CURRENT_CATEGORY:
			{
				mainWin->profile.deleteCategory();
				break;
			}
			case ID_CATEGORY_SAVECATEGORYSETTINGS:
			{
				mainWin->profile.saveCategoryOnly(mainWin);
				break;
			}
			case ID_CATEGORY_SAVECATEGORYSETTINGSAS:
			{
				mainWin->profile.saveCategoryAs(mainWin);
				break;
			}
			case ID_CONFIGURATION_NEW_CONFIGURATION:
			{
				mainWin->profile.newConfiguration(mainWin);
				break;
			}
			case ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION:
			{
				mainWin->profile.renameConfiguration();
				break;
			}
			case ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION:
			{
				mainWin->profile.deleteConfiguration();
				break;
			}
			case ID_CONFIGURATION_SAVE_CONFIGURATION_AS:
			{
				mainWin->profile.saveConfigurationAs(scriptWin, mainWin);
				break;
			}
			case ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS:
			{
				mainWin->profile.saveConfigurationOnly(scriptWin, mainWin);
				break;
			}
		}
		return false;
	}


	int startSystem(HWND parentWindow, int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin)
	{
		profileSettings profileInfo = mainWin->getCurentProfileSettings();
		if (eExperimentIsRunning)
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
				mainWin->setShortStatus(msgString);
				mainWin->changeShortStatusColor("B");
				scriptWin->changeBoxColor("B");

				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE), mainWin->getCurentProfileSettings().orientation, mainWin))
				{
					return -3;
				}
				// Officially stop trying to generate anything.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle), mainWin->getCurentProfileSettings().orientation, mainWin))
				{
					return -3;
				}
				// clear the memory
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle), mainWin->getCurentProfileSettings().orientation, mainWin))
				{
					return -3;
				}
				ViInt32 waveID;
				if (profileInfo.orientation == "Horizontal")
				{
					// create waveform (necessary?)
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// allocate waveform into the device memory
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// rewrite the script. default_hConfigScript should still be valid.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;

					}
					// start generic waveform to maintain power output to AOM.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript"), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					eCurrentScript = "DefaultHConfigScript";

				}
				else if (profileInfo.orientation == "Vertical")
				{
					// create waveform (necessary?)
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// allocate waveform into the device memory
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// rewrite the script. default_hConfigScript should still be valid.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					// start generic waveform to maintain power output to AOM.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript"), mainWin->getCurentProfileSettings().orientation, mainWin))
					{
						return -3;
					}
					eCurrentScript = "DefaultVConfigScript";
				}
				// Initiate Generation.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle), mainWin->getCurentProfileSettings().orientation, mainWin))
				{
					return -3;
				}
				eExperimentIsRunning = false;
				postMyString(mainWin, eErrorTextMessageID, "Restarted Generation.\r\n");
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
		if (profileInfo.sequenceConfigurationNames.size() == 0)
		{
			mainWin->updateStatusText("error", "ERROR: No configurations in current sequence! Please set some configurations to run in this sequence or set the null sequence.\r\n");
			mainWin->changeShortStatusColor("R");
			scriptWin->changeBoxColor("R");
			errBox("Error!");
			return -9;
		}
		// check config settings
		
		if (mainWin->checkProfileReady()) //eProfile.allSettingsReadyCheck(scriptWin))
		{
			return -1;
		}
		if (scriptWin->checkScriptSaves())
		{
			return -1;
		}
		scriptInfo<std::string> scriptNames = scriptWin->getScriptNames();
		// ordering matters here, make sure you get the correct script name.
		std::string horizontalNameString(scriptNames.horizontalNIAWG);
		std::string verticalNameString(scriptNames.verticalNIAWG);
		std::string intensityNameString(scriptNames.intensityAgilent);
		std::string beginInfo = "Current Settings:\r\n=============================\r\n\r\n";
		std::string sequenceInfo = "";//eProfile.getSequenceNamesString();
		if (sequenceInfo != "")
		{
			beginInfo += sequenceInfo;
		}
		else
		{
			scriptInfo<bool> scriptSavedStatus = scriptWin->getScriptSavedStatuses();
			beginInfo += "Vertical Script Name:............. " + std::string(verticalNameString);
			if (scriptSavedStatus.verticalNIAWG)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Horizontal Script Name:........... " + std::string(horizontalNameString);
			if (scriptSavedStatus.horizontalNIAWG)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Intensity Script Name:............ " + std::string(intensityNameString);
			if (scriptSavedStatus.intensityAgilent)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
		}
		beginInfo += "\r\n";
		std::vector<variable> vars = mainWin->getAllVariables();
		if (vars.size() == 0)
		{
			beginInfo += "Variable Names:................... NO VARIABLES\r\n";
		}
		else
		{
			beginInfo += "Variable Names:................... ";
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				beginInfo += vars[varInc].name + " ";
			}
			beginInfo += "\r\n";
		}
		mainOptions settings = mainWin->getMainOptions();
		if (settings.connectToMaster == true)
		{
			beginInfo += "Connecting To Master.............. TRUE\r\n";
			if (settings.getVariables== true)
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
			if (settings.getVariables == true)
			{
				beginInfo += "Getting Variables from Master:.... TRUE ??????\r\n";
			}
		}
		if (settings.programIntensity == true)
		{
			beginInfo += "Programming Intensity:............ TRUE\r\n";
		}
		else
		{
			beginInfo += "Programming Intensity:............ FALSE\r\n";
		}
		beginInfo += "\r\n";
		std::string beginQuestion = "\r\n\r\nBegin Waveform Generation with these Settings?";
		INT_PTR areYouSure = DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_BEGINNING_SETTINGS), 0, beginningSettingsDialogProc, (LPARAM)(beginInfo + beginQuestion).c_str());
		if (areYouSure == 0)
		{
			std::string msgString = "Performing Initial Analysis and Writing and Loading Non-Varying Waveforms...";
			mainWin->setShortStatus(msgString);
			mainWin->changeShortStatusColor("Y");
			scriptWin->changeBoxColor("Y");
			// Set the thread structure.
			experimentThreadInputStructure* inputParams = new experimentThreadInputStructure();
			// force accumulations to zero. This shouldn't affect anything, this should always get set by the master or be infinite.
			inputParams->threadRepetitions = 0;
			inputParams->settings = settings;
			inputParams->threadCurrentScript = eCurrentScript;
			if (msgID == ID_FILE_MY_WRITE_WAVEFORMS)
			{
				inputParams->threadDontActuallyGenerate = true;
			}
			else
			{
				inputParams->threadDontActuallyGenerate = false;
			}
			inputParams->threadSequenceFileNames = profileInfo.sequenceConfigurationNames;
			inputParams->currentFolderLocation = (profileInfo.pathIncludingCategory);
			inputParams->debugOptions = mainWin->getDebuggingOptions();
			inputParams->numberOfVariables = mainWin->getAllVariables().size();
			inputParams->mainWin = mainWin;
			scriptInfo<std::string> addresses = scriptWin->getScriptAddresses();
			eMostRecentVerticalScriptNames = addresses.verticalNIAWG;
			eMostRecentHorizontalScriptNames = addresses.horizontalNIAWG;
			eMostRecentIntensityScriptNames = addresses.intensityAgilent;
			// Start the programming thread.
			unsigned int experimentThreadID;
			eExperimentThreadHandle = (HANDLE)_beginthreadex(0, 0, experimentProgrammingThread, (LPVOID *)inputParams, 0, &experimentThreadID);
			eExperimentIsRunning = true;
			mainWin->addTimebar("main");
			mainWin->addTimebar("error");
			mainWin->addTimebar("debug");
			mainWin->updateStatusText("debug", beginInfo);
		}
		return 0;
	}
	
	
	int abortSystem(HWND parentWindow, ScriptingWindow* scriptWin, MainWindow* mainWin)
	{
		std::string orientation = scriptWin->getCurrentProfileSettings().orientation;
		if (!eExperimentIsRunning)
		{
			postMyString(mainWin, eErrorTextMessageID, "System was not running. Can't Abort.\r\n");
			// change the color of the colored status window.
			std::string msgString = "Passively Outputting Default Waveform.";
			mainWin->setShortStatus(msgString);
			mainWin->changeShortStatusColor("B");
			scriptWin->changeBoxColor("B");
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
		mainWin->setShortStatus(msgString);
		mainWin->changeShortStatusColor("B");
		scriptWin->changeBoxColor("B");
		//std::string currentOrientation = scriptWin->getCurrentProfileSettings().orientation;
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE), orientation, mainWin))
			{
				return -2;
			}
			// Officially stop trying to generate anything.
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle), orientation, mainWin))
			{
				return -2;
			}
			// clear the memory
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle), orientation, mainWin))
			{
				return -2;
			}
		}
		ViInt32 waveID;

		if (orientation == "Horizontal")
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// create waveform (necessary?)
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID), orientation, mainWin))
				{
					return -2;
				}
				// allocate waveform into the device memory
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2), orientation, mainWin))
				{
					return -2;
				}
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform), orientation, mainWin))
				{
					return -2;
				}
				// rewrite the script. default_hConfigScript should still be valid.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript), orientation, mainWin))
				{
					return -2;
				}
				// start generic waveform to maintain power output to AOM.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE), orientation, mainWin))
				{
					return -2;
				}
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript"), orientation, mainWin))
				{
					return -2;
				}
			}
			eCurrentScript = "DefaultHConfigScript";
		}
		else if (orientation == "Vertical")
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// create waveform (necessary?)
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID), orientation, mainWin))
				{
					return -2;

				}
				// allocate waveform into the device memory
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2), orientation, mainWin))
				{
					return -2;
				}
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform), orientation, mainWin))
				{
					return -2;
				}
				// rewrite the script. default_hConfigScript should still be valid.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript), orientation, mainWin))
				{
					return -2;
				}
				// start generic waveform to maintain power output to AOM.
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE), orientation, mainWin))
				{
					return -2;
				}
				if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript"), orientation, mainWin))
				{
					return -2;
				}
			}
			eCurrentScript = "DefaultVConfigScript";
		}
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			// Initiate Generation.
			if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle), orientation, mainWin))
			{
				return -2;
			}
		}
		eExperimentIsRunning = false;
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
	
	
	int exitProgram(HWND parentWindow, ScriptingWindow* scriptWindow, MainWindow* mainWin)
	{
		if (eExperimentIsRunning)
		{
			MessageBox(0, "Experiment is Currently Running. Please stop the system before exiting so that devices devices can stop normally.", 0, 0);
			return -1;
		}
		if (scriptWindow->checkScriptSaves())
		{
			return true;
		}
		if (mainWin->checkProfileSave())
			//eProfile.checkSaveEntireProfile(scriptWindow))
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


	int reloadNIAWGDefaults(HWND parentWindow, MainWindow* mainWin)
	{
		if (eExperimentIsRunning)
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
		std::string orientation = mainWin->getCurentProfileSettings().orientation;
		std::string tempOrientation = orientation;

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
		mainWin->setOrientation(HORIZONTAL_ORIENTATION);
		if (myErrorHandler(myNIAWG::analyzeNIAWGScripts(default_hConfigVerticalScriptFile[0], default_hConfigHorizontalScriptFile[0], default_hConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
			eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
			fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, "", noSingletons, orientation, mainWin->getDebuggingOptions(), mainWin),
			"", ConnectSocket, default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, false, eError, eSessionHandle, false, "", false, false, false, mainWin)
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
		mainWin->setOrientation(VERTICAL_ORIENTATION);
		if (myErrorHandler(myNIAWG::analyzeNIAWGScripts(default_vConfigVerticalScriptFile[0], default_vConfigHorizontalScriptFile[0],
												   default_vConfigScriptString, TRIGGER_NAME, waveformCount, eSessionHandle, SESSION_CHANNELS,
												   eError, defXPredWaveformNames, defYPredWaveformNames, defPredWaveformCount, defPredWaveLocs, libWaveformArray,
												   fileOpenedStatus, allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, true, false, "", 
													noSingletons, orientation, mainWin->getDebuggingOptions(), mainWin),
			"", ConnectSocket, default_vConfigVerticalScriptFile, default_vConfigHorizontalScriptFile, false, eError, eSessionHandle, userScriptIsWritten, "", false, false, false, mainWin)
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
		mainWin->setOrientation(tempOrientation);

		// clear NIAWG memory.
		myNIAWG::NIAWG_CheckDefaultError(niFgen_ClearArbMemory(eSessionHandle));

		ViInt32 waveID;
		if (orientation == HORIZONTAL_ORIENTATION)
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
		else if (orientation == VERTICAL_ORIENTATION)
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
		postMyString(mainWin, eStatusTextMessageID, "Reloaded Default Waveforms.\r\n");
		postMyString(mainWin, eStatusTextMessageID, "Initialized Default Waveform.\r\n");
		return 0;
	}


};
