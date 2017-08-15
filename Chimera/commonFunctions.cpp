#include "stdafx.h"
#include <array>

#include "commonFunctions.h"
#include "TextPromptDialog.h"
#include "myAgilent.h"
#include "NiawgController.h"
#include "experimentThreadInputStructure.h"
#include "scriptWriteHelpProc.h"
#include "beginningSettingsDialogProc.h"
#include "getFileName.h"
#include "saveTextFileFromEdit.h"

#include "MainWindow.h"
#include "CameraWindow.h"
#include "DeviceWindow.h"

// Functions called by all windows to do the same thing, mostly things that happen on menu presses.
namespace commonFunctions
{
	// this function handles messages that all windows can recieve, e.g. accelerator keys and menu messages. It 
	// redirects everything to all of the other functions below, for the most part.
	void handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin, 
							  CameraWindow* camWin, DeviceWindow* deviceWin )
	{
		switch (msgID)
		{
			case ID_FILE_RUN_EVERYTHING:
			case ID_ACCELERATOR_F5:
			case ID_FILE_MY_WRITE_WAVEFORMS:
			{
				camWin->redrawPictures(false);
				try
				{
					commonFunctions::startCamera(scriptWin, mainWin, camWin);
				}
				catch (Error& err)
				{
					if (err.whatBare() == "CANCEL")
					{
						mainWin->getComm()->sendStatus("Canceled camera initialization.\r\n");
						mainWin->getComm()->sendColorBox( Niawg, 'B' );
						break;
					}
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + err.whatStr());
					mainWin->getComm()->sendColorBox( Camera, 'R' );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
					camWin->assertOff();
					break;
				}
				try
				{
					//commonFunctions::startNiawg(msgID, scriptWin, mainWin, camWin, deviceWin);
				}
				catch (Error& err)
				{
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + err.whatStr());
					mainWin->getComm()->sendColorBox( Niawg, 'R' );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
				}
				try
				{
					commonFunctions::startMasterOnly(mainWin, deviceWin, scriptWin);
				}
				catch (Error& err)
				{
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + err.whatStr());
					mainWin->getComm()->sendColorBox( Master, 'R' );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
				}
				break;
			}
			case WM_CLOSE:
			case ID_ACCELERATOR_ESC:
			case ID_FILE_ABORT_GENERATION:
			{
				std::string status;
				try
				{
					bool niawgAborted = false, andorAborted = false, masterAborted = false;
					if (mainWin->niawg.isRunning())
					{
						status = "NIAWG";
						commonFunctions::abortNiawg(scriptWin, mainWin);
						niawgAborted = true;
					}
					mainWin->getComm()->sendColorBox( Niawg, 'B' );
					if (camWin->Andor.isRunning())
					{
						status = "ANDOR";
						commonFunctions::abortCamera(camWin, mainWin);
						andorAborted = true;
					}
					if (mainWin->masterThreadManager.runningStatus())
					{
						status = "MASTER";
						commonFunctions::abortMaster(mainWin, deviceWin);
						masterAborted = true;
					}
					mainWin->getComm()->sendColorBox( Camera, 'B' );
					camWin->assertOff();
					// todo... intensity

					if (!niawgAborted && !andorAborted && !masterAborted)
					{					
						mainWin->getComm()->sendError("Neither Camera nor NIAWG was not running. Can't Abort.\r\n");
					}
				}
				catch (Error& except)
				{
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.whatStr());
					if (status == "NIAWG")
					{
						mainWin->getComm()->sendColorBox( Niawg, 'R' );
					}
					else if (status == "ANDOR")
					{
						mainWin->getComm()->sendColorBox( Camera, 'R' );
					}
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
				}
				break;
			}
			case ID_RUNMENU_RUNCAMERA:
			{
				mainWin->getComm()->sendColorBox( Camera, 'Y' );
				mainWin->getComm()->sendStatus("Starting Camera...\r\n");
				try
				{
					commonFunctions::startCamera(scriptWin, mainWin, camWin);
					mainWin->getComm()->sendColorBox( Camera, 'G' );
					mainWin->getComm()->sendStatus("Camera is Running.\r\n");
				}
				catch (Error& exception)
				{
					if (exception.whatBare() == "CANCEL")
					{
						mainWin->getComm()->sendColorBox( Camera, 'B' );
						mainWin->getComm()->sendStatus("Camera is Not Running, User Canceled.\r\n");
						break;
					}
					mainWin->getComm()->sendColorBox( Camera, 'R' );
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + exception.whatStr());
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
					camWin->assertOff();
					break;
				}
				break;
			}
			case ID_RUNMENU_RUNNIAWG:
			{
				try
				{
					commonFunctions::startNiawgOnly(msgID, scriptWin, mainWin, camWin, deviceWin);
				}
				catch (Error& except)
				{
					mainWin->getComm()->sendColorBox( Niawg, 'R' );
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.whatStr());
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
				}
				break;
			}
			case ID_RUNMENU_RUNMASTER:
			{
				try
				{
					commonFunctions::startMasterOnly( mainWin, deviceWin, scriptWin );
				}
				catch (Error& err)
				{
					mainWin->getComm()->sendColorBox( Master, 'R' );
					mainWin->getComm()->sendError( "EXITED WITH ERROR! " + err.whatStr() );
					mainWin->getComm()->sendStatus( "EXITED WITH ERROR!\r\n" );
				}
				break;
			}
			case ID_RUNMENU_ABORTMASTER:
			{
				commonFunctions::abortMaster(mainWin, deviceWin);
				break;
			}
			/// File Management 
			case ID_FILE_SAVEALL:
			{
				scriptWin->saveHorizontalScript();
				scriptWin->saveVerticalScript();
				scriptWin->saveIntensityScript();
				mainWin->profile.saveEntireProfile(scriptWin, mainWin, deviceWin, camWin);
				break;
			}
			case ID_PROFILE_SAVE_PROFILE:
			{
				mainWin->profile.saveEntireProfile(scriptWin, mainWin, deviceWin, camWin);
				break;
			}
			case ID_FILE_MY_EXIT:
			{
				commonFunctions::exitProgram(scriptWin, mainWin, camWin);
				break;
			}
			case ID_FILE_MY_INTENSITY_NEW:
			{
				scriptWin->newIntensityScript();
				break;
			}
			case ID_FILE_MY_INTENSITY_OPEN:
			{
				scriptWin->openIntensityScript(parent);
				break;
			}
			case ID_FILE_MY_INTENSITY_SAVE:
			{
				scriptWin->saveIntensityScript();
				break;
			}
			case ID_FILE_MY_INTENSITY_SAVEAS:
			{
				scriptWin->saveIntensityScriptAs(parent);
				break;
			}
			case ID_FILE_MY_VERTICAL_NEW:
			{
				scriptWin->newVerticalScript();
				break;
			}
			case ID_FILE_MY_VERTICAL_OPEN:
			{
				scriptWin->openVerticalScript(parent);
				break;
			}
			case ID_FILE_MY_VERTICAL_SAVE:
			{
				scriptWin->saveVerticalScript();
				break;
			}
			case ID_FILE_MY_VERTICAL_SAVEAS:
			{
				scriptWin->saveVerticalScriptAs(parent);
				break;
			}
			case ID_FILE_MY_HORIZONTAL_NEW:
			{
				scriptWin->newHorizontalScript();
				break;
			}
			case ID_FILE_MY_HORIZONTAL_OPEN:
			{
				scriptWin->openHorizontalScript(parent);
				break;
			}
			case ID_FILE_MY_HORIZONTAL_SAVE:
			{
				scriptWin->saveHorizontalScript();
				break;
			}
			case ID_FILE_MY_HORIZONTAL_SAVEAS:
			{
				scriptWin->saveHorizontalScriptAs(parent);
				break;
			}
			case ID_MASTERSCRIPT_NEW:
			{
				scriptWin->newMasterScript();
				break;
			}
			case ID_MASTERSCRIPT_SAVE:
			{
				scriptWin->saveMasterScript();
				break;
			}
			case ID_MASTERSCRIPT_SAVEAS:
			{
				scriptWin->saveMasterScriptAs(parent);
				break;
			}
			case ID_MASTERSCRIPT_NEWFUNCTION:
			{
				scriptWin->newMasterFunction();
				break;
			}
			case ID_MASTERSCRIPT_SAVEFUNCTION:
			{
				scriptWin->saveMasterFunction();
				break;
			}

			/*			
			ID_MASTERSCRIPT_RENAME
			ID_MASTERSCRIPT_DELETEFUNCTION	
			*/

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
				commonFunctions::helpWindow();
				break;
			}
			case ID_HELP_GENERALINFORMATION:
			{
				break;
			}
			case ID_NIAWG_RELOADDEFAULTWAVEFORMS:
			{
				commonFunctions::reloadNIAWGDefaults(mainWin);
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
				mainWin->profile.saveConfigurationAs(scriptWin, mainWin, deviceWin);
				break;
			}
			case ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS:
			{
				mainWin->profile.saveConfigurationOnly(scriptWin, mainWin, deviceWin, camWin);
				break;
			}
			case ID_NIAWG_SENDSOFTWARETRIGGER:
			{
				mainWin->niawg.sendSoftwareTrigger();
				break;
			}
			case ID_NIAWG_STREAMWAVEFORM:
			{
				mainWin->niawg.streamWaveform();
				break;
			}
			case ID_NIAWG_GETNIAWGERROR:
			{
				errBox(mainWin->niawg.getErrorMsg());
				break;
			}
			case ID_PICTURES_AUTOSCALEPICTURES:
			{
				camWin->handleAutoscaleSelection();
				break;
			}
			case ID_PICTURES_GREATER_THAN_MAX_SPECIAL:
			{
				camWin->handleSpecialGreaterThanMaxSelection();
				break;
			}
			case ID_PICTURES_LESS_THAN_MIN_SPECIAL:
			{
				camWin->handleSpecialLessThanMinSelection();
				break;
			}
			case ID_PICTURES_ALWAYSSHOWGRID:
			{
				camWin->passAlwaysShowGrid();
				break;
			}
			case ID_RUNMENU_ABORTCAMERA:
			{
				try
				{
					if (camWin->Andor.isRunning())
					{
						commonFunctions::abortCamera(camWin, mainWin);
					}
					else
					{
						mainWin->getComm()->sendError("Camera was not running. Can't Abort.\r\n");
					}
					mainWin->getComm()->sendColorBox( Camera, 'B' );
					camWin->assertOff();
				}
				catch (Error& except)
				{
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.whatStr());
					mainWin->getComm()->sendColorBox( Camera, 'R' );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
				}
				break;
			}
			case ID_RUNMENU_ABORTNIAWG:
			{
				try
				{
					if (mainWin->niawg.isRunning())
					{
						commonFunctions::abortNiawg(scriptWin, mainWin);
					}
					else
					{
						mainWin->getComm()->sendError("NIAWG was not running. Can't Abort.\r\n");
					}
					mainWin->getComm()->sendColorBox( Niawg, 'B' );
				}
				catch (Error& except)
				{
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.whatStr());
					mainWin->getComm()->sendColorBox( Niawg, 'R' );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
				}
				break;
			}
			case ID_MASTERCONFIG_SAVEMASTERCONFIGURATION:
			{

				mainWin->masterConfig.save(mainWin, deviceWin, camWin);
				break;
			}
			case ID_MASTERCONFIGURATION_RELOAD_MASTER_CONFIG:
			{
				mainWin->masterConfig.load(mainWin, deviceWin, camWin);
				break;
			}
			case ID_MASTER_VIEWORCHANGEINDIVIDUALDACSETTINGS:
			{
				deviceWin->ViewOrChangeDACNames();
				break;
			}
			case ID_MASTER_VIEWORCHANGETTLNAMES:
			{
				deviceWin->ViewOrChangeTTLNames();
				break;
			}
			case ID_ACCELERATOR_F2:
			case ID_RUNMENU_PAUSE:
			{
				mainWin->handlePause();
				break;
			}
			case ID_ACCELERATOR_F1:
			{
				deviceWin->loadMotSettings();
				break;
			}

			default:
				errBox("ERROR: Common message passed but not handled!");
		}
	}

	void startCamera( ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin )
	{
		camWin->redrawPictures( false );
		mainWin->getComm()->sendTimer( "Starting..." );
		camWin->prepareCamera();

		std::string msg = camWin->getStartMessage();
		int answer = MessageBox( 0, cstr(msg), "Start Info", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			// user doesn't want to start the camera.
			thrower( "CANCEL" );
		}
		// tells system an acq has taken place

		// Set the running version to whatever is selected at the beginning of this function.		
		//eCurrentlyRunningCameraMode = eCurrentlySelectedCameraMode;
		/*
		time_t time_obj = time(0);   // get time now
		struct tm currentTime;
		localtime_s(&currentTime, &time_obj);
		std::string timeStr = "(" + str(currentTime.tm_year + 1900) + ":" + str(currentTime.tm_mon + 1) + ":"
			+ str(currentTime.tm_mday) + ")" + str(currentTime.tm_hour) + ":"
			+ str(currentTime.tm_min) + ":" + str(currentTime.tm_sec);
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
	}

	/*
	 * 
	 */
	void startNiawgOnly( int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, CameraWindow* camWin,
						 DeviceWindow* deviceWin )
	{
		Communicator* comm = mainWin->getComm();
		profileSettings profile = mainWin->getCurrentProfileSettings();
		if (mainWin->niawgIsRunning())
		{
			int restart = MessageBox( 0, "Restart Generation?", 0, MB_OKCANCEL );
			if (restart == IDOK)
			{
				// reset flag
				eAbortNiawgFlag = true;
				// wait for reset to occur
				int result = WaitForSingleObject( eNIAWGWaitThreadHandle, INFINITE );
				result = WaitForSingleObject( eExperimentThreadHandle, INFINITE );
				eAbortNiawgFlag = false;
				// abort the generation on the NIAWG.
				Agilent::agilentDefault();
				std::string msgString = "Passively Outputting Default Waveform";
				mainWin->getComm()->sendStatus( msgString );
				mainWin->getComm()->sendColorBox( Niawg, 'B' );

				try
				{
					mainWin->restartNiawgDefaults();
				}
				catch (Error& except)
				{
					mainWin->getComm()->sendColorBox( Niawg, 'R' );
					mainWin->getComm()->sendFatalError( "Failed to restart the NIAWG default during the script restart procedure! Error "
														"reported is" + except.whatStr() );
					mainWin->setNiawgRunningState( false );
					return;
				}
			}
			else
			{
				return;
			}
		}
		if (profile.sequenceConfigNames.size() == 0)
		{
			mainWin->getComm()->sendColorBox( Niawg, 'R' );
			mainWin->getComm()->sendError( "ERROR: No configurations in current sequence! Please set some configurations to run in this "
										   "sequence or set the null sequence.\r\n" );
			return;
		}
		// check config settings
		mainWin->checkProfileReady();
		if (scriptWin->checkScriptSaves())
		{
			return;
		}
		scriptInfo<std::string> scriptNames = scriptWin->getScriptNames();
		// ordering matters here, make sure you get the correct script name.
		std::string horizontalNameString( scriptNames.horizontalNIAWG );
		std::string verticalNameString( scriptNames.verticalNIAWG );
		std::string intensityNameString( scriptNames.intensityAgilent );
		std::string beginInfo = "Current Settings:\r\n=============================\r\n\r\n";
		std::string sequenceInfo = "";
		if (sequenceInfo != "")
		{
			beginInfo += sequenceInfo;
		}
		else
		{
			scriptInfo<bool> scriptSavedStatus = scriptWin->getScriptSavedStatuses();
			beginInfo += "Vertical Script Name:........ " + str( verticalNameString );
			if (scriptSavedStatus.verticalNIAWG)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Horizontal Script Name:...... " + str( horizontalNameString );
			if (scriptSavedStatus.horizontalNIAWG)
			{
				beginInfo += " SAVED\r\n";
			}
			else
			{
				beginInfo += " NOT SAVED\r\n";
			}
			beginInfo += "Intensity Script Name:....... " + str( intensityNameString );
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
		std::vector<variable> vars = deviceWin->getAllVariables();
		if (vars.size() == 0)
		{
			beginInfo += "Variable Names:.............. NO VARIABLES\r\n";
		}
		else
		{
			beginInfo += "Variable Names:.............. ";
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				beginInfo += vars[varInc].name + " ";
			}
			beginInfo += "\r\n";
		}
		mainOptions settings = mainWin->getMainOptions();
		if (settings.programIntensity)
		{
			beginInfo += "Programming Intensity:....... TRUE\r\n";
		}
		else
		{
			beginInfo += "Programming Intensity:....... FALSE\r\n";
		}
		beginInfo += "\r\n";
		std::string beginQuestion = "\r\n\r\nBegin Waveform Generation with these Settings?";
		INT_PTR areYouSure = DialogBoxParam( NULL, MAKEINTRESOURCE( IDD_BEGINNING_SETTINGS ), 0, 
											 beginningSettingsDialogProc, (LPARAM)cstr(beginInfo + beginQuestion));
		if (areYouSure == 0)
		{
			mainWin->getComm()->sendStatus( "Performing Initial Analysis and Writing and Loading Non-Varying Waveforms...\r\n" );
			mainWin->getComm()->sendColorBox( Niawg, 'Y' );
			// Set the thread structure.
			MasterThreadInput* inputParams = new MasterThreadInput();
			// force accumulations to zero. This shouldn't affect anything, this should always get set by the master or be infinite.

			//inputParams->settings = settings;
			if (msgID == ID_FILE_MY_WRITE_WAVEFORMS)
			{
				inputParams->dontActuallyGenerate = true;
			}
			else
			{
				inputParams->dontActuallyGenerate = false;
			}
			inputParams->debugOptions = mainWin->getDebuggingOptions();
			inputParams->comm = mainWin->getComm();
			inputParams->profile = profile;
			scriptInfo<std::string> addresses = scriptWin->getScriptAddresses();
			eMostRecentVerticalScriptNames = addresses.verticalNIAWG;
			eMostRecentHorizontalScriptNames = addresses.horizontalNIAWG;
			eMostRecentIntensityScriptNames = addresses.intensityAgilent;

			inputParams->runNiawg = true;
			inputParams->runMaster = false;

			// Start the programming thread.
			deviceWin->fillMasterThreadInput( inputParams );
			mainWin->fillMasterThreadInput( inputParams );
			mainWin->startMaster( inputParams );
			mainWin->setNiawgRunningState( true );
			mainWin->addTimebar( "main" );
			mainWin->addTimebar( "error" );
			mainWin->addTimebar( "debug" );
			mainWin->updateStatusText( "debug", beginInfo );
		}
	}

	void abortCamera( CameraWindow* camWin, MainWindow* mainWin )
	{
		if (!camWin->cameraIsRunning())
		{
			mainWin->getComm()->sendColorBox( Niawg, 'B' );
			mainWin->getComm()->sendError( "System was not running. Can't Abort.\r\n" );
			return;
		}
		int fitsStatus = 0;
		std::string errorMessage;
		// abort acquisition if in progress
		camWin->abortCameraRun();
		mainWin->getComm()->sendStatus( "Aborted Camera Operation.\r\n" );
		// todo: here handle data closing as well....? 
	}


	void abortNiawg( ScriptingWindow* scriptWin, MainWindow* mainWin )
	{
		Communicator* comm = mainWin->getComm();
		// set reset flag
		eAbortNiawgFlag = true;
		if (!mainWin->niawgIsRunning())
		{
			std::string msgString = "Passively Outputting Default Waveform.";
			comm->sendColorBox( Niawg, 'B' );
			comm->sendError( "System was not running. Can't Abort.\r\n" );
			return;
		}
		std::string orientation = scriptWin->getCurrentProfileSettings().orientation;
		// wait for reset to occur
		int result = 1;
		result = WaitForSingleObject( eNIAWGWaitThreadHandle, 0 );
		if (result == WAIT_TIMEOUT)
		{
			// try again. Hopefully gives the main thread to handle other messages first if this happens.
			mainWin->PostMessageA( WM_COMMAND, MAKEWPARAM( ID_FILE_ABORT_GENERATION, 0 ) );
			return;
		}
		result = WaitForSingleObject( eExperimentThreadHandle, 0 );
		if (result == WAIT_TIMEOUT)
		{
			// try again. Hopefully gives the main thread to handle other messages first if this happens.
			mainWin->PostMessageA( WM_COMMAND, MAKEWPARAM( ID_FILE_ABORT_GENERATION, 0 ) );
			return;
		}
		eAbortNiawgFlag = false;
		// abort the generation on the NIAWG.
		myAgilent::agilentDefault();
		comm->sendStatus( "Aborted NIAWG Operation. Passively Outputting Default Waveform.\r\n" );
		comm->sendColorBox( Niawg, 'B' );
		mainWin->restartNiawgDefaults();
		mainWin->setNiawgRunningState( false );
	}


	void exitProgram( ScriptingWindow* scriptWindow, MainWindow* mainWin, CameraWindow* camWin )
	{
		if (mainWin->niawgIsRunning())
		{
			thrower( "The NIAWG is Currently Running. Please stop the system before exiting so that devices devices "
					 "can stop normally." );
		}
		if (camWin->cameraIsRunning())
		{
			thrower( "The Camera is Currently Running. Please stop the system before exiting so that devices devices "
					 "can stop normally." );
		}
		if (mainWin->masterIsRunning())
		{
			thrower( "The Master system (ttls & dacs) is currently running. Please stop the system before exiting so "
					 "that devices can stop normally." );
		}
		if (scriptWindow->checkScriptSaves())
		{
			return;
		}
		mainWin->checkProfileSave();
		std::string exitQuestion = "Are you sure you want to exit?\n\nThis will stop all output of the arbitrary waveform generator.";
		int areYouSure = MessageBox( NULL, cstr(exitQuestion), "Exit", MB_OKCANCEL | MB_ICONWARNING );
		if (areYouSure == IDOK)
		{
			/// Exiting
			// Close the NIAWG normally.
			if (!NIAWG_SAFEMODE)
			{
				try
				{
					mainWin->stopNiawg();
				}
				catch (Error& except)
				{
					errBox( "ERROR: The NIAWG did not exit smoothly. : " + except.whatStr() );
				}
			}
			PostQuitMessage( 1 );
		}
	}

	void commonFunctions::helpWindow()
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
	}

	void commonFunctions::reloadNIAWGDefaults( MainWindow* mainWin )
	{
		profileSettings profile = mainWin->getCurrentProfileSettings();
		if (mainWin->niawgIsRunning())
		{
			thrower( "The system is currently running. You cannot reload the default waveforms while the system is running. Please restart "
					 "the system before attempting to reload default waveforms." );
		}
		int choice = MessageBox( 0, "Reload the default waveforms from (presumably) updated files? Please make sure that the updated files are "
								 "syntactically correct, or else the program will crash.", 0, MB_OKCANCEL );
		if (choice == IDCANCEL)
		{
			return;
		}
		try
		{
			mainWin->setNiawgDefaults();
			mainWin->restartNiawgDefaults();
		}
		catch (Error& exception)
		{
			mainWin->restartNiawgDefaults();
			thrower( "ERROR: failed to reload the niawg default waveforms! Error message: " + exception.whatStr() );
		}
		mainWin->getComm()->sendStatus( "Reloaded Default Waveforms.\r\nInitialized Default Waveform.\r\n" );
	}

	void commonFunctions::setMot(MainWindow* mainWin)
	{
		MasterThreadInput* input = new MasterThreadInput;
		input->quiet = true;
		
	}


	void commonFunctions::startMasterOnly(MainWindow* mainWin, DeviceWindow* deviceWin, ScriptingWindow* scriptWin)
	{
		Communicator* comm = mainWin->getComm();
		comm->sendStatus("............................\r\n");
		comm->sendStatus("Checking if Ready...\n");
		// profile.allSettingsReadyCheck( this );
		// masterScript.checkSave( this );
		comm->sendStatus("Starting Experiment Thread...\r\n");
		comm->sendColorBox( Master, 'Y' );
		MasterThreadInput* input = new MasterThreadInput;
		input->quiet = false;
		input->runNiawg = false;
		input->runMaster = true;
		scriptWin->checkMasterSave();
		deviceWin->fillMasterThreadInput(input);
		mainWin->fillMasterThreadInput(input);
		mainWin->startMaster(input);
	}

	void startFullMasterThread( MainWindow* mainWin, DeviceWindow* deviceWin, ScriptingWindow* scriptWin )
	{
		Communicator* comm = mainWin->getComm();
		comm->sendStatus( "............................\r\n" );
		comm->sendStatus( "Checking if Ready...\n" );
		// profile.allSettingsReadyCheck( this );
		// masterScript.checkSave( this );
		comm->sendStatus( "Starting Experiment Thread...\r\n" );
		comm->sendColorBox( Master, 'Y' );
		MasterThreadInput* input = new MasterThreadInput;
		input->quiet = false;
		scriptWin->checkMasterSave();
		input->runNiawg = true;
		input->runMaster = true;
		deviceWin->fillMasterThreadInput( input );
		mainWin->fillMasterThreadInput( input );
		mainWin->startMaster( input );
	}

	void commonFunctions::abortMaster(MainWindow* mainWin, DeviceWindow* deviceWin)
	{
		mainWin->abortMasterThread();
		deviceWin->handleAbort();
	}
};

