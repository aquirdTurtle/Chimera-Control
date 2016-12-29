#include "stdafx.h"
#include "commonMessages.h"
#include "Windows.h"
#include "textPromptDialogProcedure.h"
#include "myAgilent.h"
#include "NiawgController.h"
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
#include "CameraWindow.h"

// Functions called by all windows to do the same thing, mostly things that happen on menu presses.
namespace commonMessages
{
	// this function handles messages that all windows can recieve, e.g. accelerator keys and menu messages. It redirects
	// everything to all of the other functions below, for the most part.
	bool handleCommonMessage(int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin, 
							  CameraWindow* camWin)
	{
		switch (msgID)
		{
			case ID_FILE_RUN_EVERYTHING:
			case ID_ACCELERATOR_F5:
			case ID_FILE_MY_WRITE_WAVEFORMS:
			{
				camWin->redrawPictures();
				
				try
				{
					commonMessages::startCamera( scriptWin, mainWin, camWin );
					commonMessages::startNiawg(msgID, scriptWin, mainWin, camWin);
				}
				catch (my_exception& except)
				{
					colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.whatStr());
					mainWin->getComm()->sendColorBox( colors );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer( "ERROR!", "Red" );
				}
				break;
			}
			case WM_CLOSE:
			case ID_ACCELERATOR_ESC:
			case ID_FILE_ABORT_GENERATION:
			{
				// try to abort.
				try
				{
					commonMessages::abortSystem(scriptWin, mainWin);
				}				
				catch (my_exception& except)
				{
					colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.whatStr());
					mainWin->getComm()->sendColorBox( colors );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer( "ERROR!", "Red" );
				}
				break;
			}
			case ID_RUNMENU_RUNCAMERA:
			{
				colorBoxes<char> colors = { /*niawg*/'-', /*camera*/'Y', /*intensity*/'-' };
				mainWin->getComm()->sendColorBox( colors );
				mainWin->getComm()->sendStatus( "Starting Camera..." );
				try
				{
					commonMessages::startCamera( scriptWin, mainWin, camWin );
				}
				catch ( my_exception& exception )
				{
					colorBoxes<char> colors = { /*niawg*/'-', /*camera*/'R', /*intensity*/'-' };
					mainWin->getComm()->sendColorBox( colors );
					mainWin->getComm()->sendError( "EXITED WITH ERROR! " + exception.whatStr() );
					mainWin->getComm()->sendStatus( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					mainWin->getComm()->sendTimer( "ERROR!", "Red" );
				}
				//mainWin->
				break;
			}
			case ID_RUNMENU_RUNNIAWG:
			{
				try
				{
					commonMessages::startNiawg(msgID, scriptWin, mainWin, camWin);
				}
				catch (my_exception& except)
				{
					colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
					mainWin->getComm()->sendColorBox( colors );
					mainWin->getComm()->sendError( "EXITED WITH ERROR! " + except.whatStr() );
					mainWin->getComm()->sendStatus( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					return true;
				}
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
				commonMessages::exitProgram(scriptWin, mainWin);
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
				commonMessages::helpWindow();
				break;
			}
			case ID_HELP_GENERALINFORMATION:
			{
				break;
			}
			case ID_NIAWG_RELOADDEFAULTWAVEFORMS:
			{
				commonMessages::reloadNIAWGDefaults(mainWin);
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

	void startCamera(ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin)
	{
		camWin->redrawPictures();
		mainWin->getComm()->sendTimer( "Starting...", "Blue" );
		camWin->prepareCamera();

		std::string msg = camWin->getStartMessage();
		int answer = MessageBox( 0, msg.c_str(), "Start Info", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			// user doesn't want to start the camera.
			return;
		}
		// tells system an acq has taken place

		// Set the running version to whatever is selected at the beginning of this function.		
		//eCurrentlyRunningCameraMode = eCurrentlySelectedCameraMode;
		eExperimentIsRunning = true;
		/*
		time_t time_obj = time(0);   // get time now
		struct tm currentTime;
		localtime_s(&currentTime, &time_obj);
		std::string timeStr = "(" + std::to_string(currentTime.tm_year + 1900) + ":" + std::to_string(currentTime.tm_mon + 1) + ":"
			+ std::to_string(currentTime.tm_mday) + ")" + std::to_string(currentTime.tm_hour) + ":"
			+ std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
		appendText("\r\n**********" + timeStr + "**********\r\nSystem is Running.\r\n", IDC_STATUS_EDIT);
		appendText("\r\n******" + timeStr + "******\r\n", IDC_ERROR_EDIT);
		*/
		// Set hardware and start acquisition
		camWin->startCamera();
		/*
			eExperimentIsRunning = false;
			eCameraWindowExperimentTimer.setColorID(ID_RED);
			eCameraWindowExperimentTimer.setTimerDisplay("ERROR");
			// stop the plotting thread if it started.
			ePlotThreadExitIndicator = false;
			//eThreadExitIndicator = false;
			// Wait until plotting thread is complete.
			WaitForSingleObject(ePlottingThreadHandle, INFINITE);
			if (ANDOR_SAFEMODE)
			{
				eExperimentIsRunning = false;
			}
			appendText("Failed to start camera aquisition.\r\n", IDC_STATUS_EDIT);
			*/
		return;
	}

	/* 
	 *  Can throw my_exception.
	 */ 
	int startNiawg(int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, CameraWindow* camWin)
	{
		Communicator* comm = mainWin->getComm();
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
				mainWin->getComm()->sendStatus(msgString);
				colorBoxes<char> colors = { /*niawg*/'B', /*camera*/'-', /*intensity*/'-' };
				mainWin->getComm()->sendColorBox( colors );

				try
				{
					mainWin->restartNiawgDefaults();
				}
				catch (my_exception& except)
				{
					colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
					mainWin->getComm()->sendColorBox( colors );
					mainWin->getComm()->sendFatalError("Failed to restart the NIAWG default during the script restart procedure! Error reported is" + except.whatStr(), "", colors);
					eExperimentIsRunning = false;
					return -1;
				}								
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
			colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
			mainWin->getComm()->sendColorBox( colors );
			mainWin->getComm()->sendError("ERROR: No configurations in current sequence! Please set some configurations to run in this sequence or set the null sequence.\r\n", "", colors);
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
			mainWin->getComm()->sendStatus(msgString);
			colorBoxes<char> colors = { /*niawg*/'Y', /*camera*/'-', /*intensity*/'-' };
			mainWin->getComm()->sendColorBox( colors );
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
			inputParams->comm = mainWin->getComm();
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
	
	/*
	 *  Can throw my_exception.
	 */
	int abortSystem(ScriptingWindow* scriptWin, MainWindow* mainWin)
	{
		Communicator* comm = mainWin->getComm();
		std::string orientation = scriptWin->getCurrentProfileSettings().orientation;
		if (!eExperimentIsRunning)
		{
			std::string msgString = "Passively Outputting Default Waveform.";
			colorBoxes<char> colors = { /*niawg*/'B', /*camera*/'-', /*intensity*/'-' };
			mainWin->getComm()->sendColorBox( colors );
			comm->sendError("System was not running. Can't Abort.\r\n");
			return -1;
		}
		// set reset flag
		eAbortSystemFlag = true;
		// wait for reset to occur
		int result = 1;
		result = WaitForSingleObject(eNIAWGWaitThreadHandle, 0);
		if (result == WAIT_TIMEOUT)
		{
			mainWin->PostMessageA(WM_COMMAND, MAKEWPARAM(ID_FILE_ABORT_GENERATION, 0));
			return -1;
		}
		result = WaitForSingleObject(eExperimentThreadHandle, 0);
		if (result == WAIT_TIMEOUT)
		{
			// try again. this will put this message at the back of the message queue so waiting messages will get handled first.
			mainWin->PostMessageA(WM_COMMAND, MAKEWPARAM(ID_FILE_ABORT_GENERATION, 0));	
			return -1;
		}
		eAbortSystemFlag = false;
		// abort the generation on the NIAWG.
		myAgilent::agilentDefault();
		
		std::string msgString = "Passively Outputting Default Waveform.";
		colorBoxes<char> colors = { /*niawg*/'B', /*camera*/'-', /*intensity*/'-' };
		comm->sendStatus( msgString );
		comm->sendColorBox( colors );
		mainWin->restartNiawgDefaults();
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
	
	
	int exitProgram(ScriptingWindow* scriptWindow, MainWindow* mainWin)
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
				/// Exiting
				// Close the NIAWG normally.
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					try
					{
						mainWin->stopNiawg();
					}
					catch (my_exception& except)
					{
						errBox("ERROR: The NIAWG did not exit smoothly. : " + except.whatStr());
					}
				}
				PostQuitMessage(1);
				break;
			case IDCANCEL:
				break;
			default:
				break;
		}
		return 0;
	}

	void commonMessages::helpWindow()
	{
		/*
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
		*/
		return;
	}

	void commonMessages::reloadNIAWGDefaults( MainWindow* mainWin )
	{
		profileSettings profileInfo = mainWin->getCurentProfileSettings();
		if (eExperimentIsRunning)
		{
			MessageBox(0, "The system is currently running. You cannot reload the default waveforms while the system is running. Please restart "
				"the system before attempting to reload default waveforms.", 0, 0);
			return;
		}
		int choice = MessageBox(0, "Reload the default waveforms from (presumably) updated files? Please make sure that the updated files are "
			"syntactically correct, or else the program will crash.", 0, MB_OKCANCEL);
		if (choice == IDCANCEL)
		{
			return;
		}
		try
		{
			mainWin->setNiawgDefaults(false);
			mainWin->restartNiawgDefaults();
		}
		catch (my_exception& exception)
		{
			errBox("ERROR: failed to reload the niawg default waveforms! Error message: " + exception.whatStr());
			mainWin->restartNiawgDefaults();
			return;
		}
		mainWin->getComm()->sendStatus("Reloaded Default Waveforms.\r\nInitialized Default Waveform.\r\n");
		return;
	}
};
