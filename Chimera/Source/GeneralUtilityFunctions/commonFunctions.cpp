// created by Mark O. Brown
#include "stdafx.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "NIAWG/NiawgCore.h"
#include "ExcessDialogs/openWithExplorer.h"
#include "ExcessDialogs/saveWithExplorer.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtDeformableMirrorWindow.h"
#include "PrimaryWindows/QtScriptWindow.h"
#include "PrimaryWindows/QtBaslerWindow.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include "ExperimentThread/ExperimentType.h"
#include "ExperimentThread/autoCalConfigInfo.h"
#include <QDateTime.h>

// Functions called by all windows to do the same thing, mostly things that happen on menu presses or hotkeys
namespace commonFunctions
{
	// this function handles messages that all windows can recieve, e.g. accelerator keys and menu messages. It 
	// redirects everything to all of the other functions below, for the most part.
	void handleCommonMessage( int msgID, IChimeraQtWindow* win ){
		auto* mainWin = win->mainWin; 
		auto* andorWin = win->andorWin;
		auto* scriptWin = win->scriptWin;
		auto* dmWin = win->dmWin;
		auto* basWin = win->basWin;
		auto* auxWin = win->auxWin;
		try {
			switch (msgID) {
				//case ID_MACHINE_OPTIMIZATION:{
				//	// this is mostly prepared like F5.
				//	if ( andorWin->wantsAutoCal ( ) && !andorWin->wasJustCalibrated ( ) ){
				//		return;
				//	}
				//	AllExperimentInput input;
				//	andorWin->redrawPictures ( false );
				//	try	{
				//		mainWin->reportStatus ( "Starting Automatic Optimization...\r\n" );
				//		andorWin->setTimerText ( "Starting..." );
				//		prepareMasterThread ( msgID, win, input, true, true, true, true, true );
				//		input.masterInput->quiet = true;
				//		logStandard ( input, andorWin->getLogger ( ), mainWin->getServoinfo(), andorWin->getAlignmentVals ());
				//		auxWin->updateOptimization ( input );
				//		input.masterInput->expType = ExperimentType::MachineOptimization;
				//		startExperimentThread ( mainWin, input );
				//	}
				//	catch ( ChimeraError& err ){
				//		if ( err.whatBare ( ) == "CANCEL" )	{
				//			mainWin->reportStatus ( "Canceled camera initialization.\r\n" );
				//			break;
				//		}
				//		mainWin->reportErr ( "Exited with Error!\n" + err.qtrace ( ) );
				//		mainWin->reportStatus ( "EXITED WITH ERROR!\nInitialized Default Waveform\r\n" );
				//		andorWin->setTimerText ( "ERROR!" );
				//		andorWin->assertOff ( );
				//		break;
				//	}
				//	break;
				//}
			case ID_RUNMENU_RUNBASLERANDMASTER:
			{
				AllExperimentInput input;
				try {
					andorWin->setTimerText ("Starting...");
					prepareMasterThread (msgID, win, input, false, true, false, true, false);
					commonFunctions::getPermissionToStart (win, false, true, input);
					input.masterInput->expType = ExperimentType::Normal;
					logStandard (input, andorWin->getLogger (), andorWin->getAlignmentVals (),
						"", false);
					startExperimentThread (mainWin, input);
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("EXITED WITH ERROR!\n " + err.qtrace ());
					mainWin->reportStatus ("EXITED WITH ERROR!\nInitialized Default Waveform\r\n");
					andorWin->setTimerText ("ERROR!");
					andorWin->assertOff ();
					break;
				}
				break;
			}
			case ID_FILE_RUN_EVERYTHING:
			case ID_ACCELERATOR_F5:
			case ID_FILE_MY_WRITE_WAVEFORMS: {
				AllExperimentInput input;
				try {
					if (mainWin->masterIsRunning ()) {
						auto response = QMessageBox::question (mainWin, "Auto-F5?",
							"The Master system is already running. Would you like to run the "
							"current configuration when master finishes? This effectively "
							"auto-presses F5 when complete and skips confirmation.");
						if (response == QMessageBox::Yes) {
							mainWin->autoF5_AfterFinish = true;
						}
						break;
					}
					andorWin->setTimerText ("Starting...");
					// automatically save; this is important to handle changes like the auto servo and auto carrier
					commonFunctions::handleCommonMessage (ID_FILE_SAVEALL, win);
					prepareMasterThread (msgID, win, input, true, true, true, true, true);
					input.masterInput->expType = ExperimentType::Normal;
					if (!mainWin->autoF5_AfterFinish) {
						commonFunctions::getPermissionToStart (win, true, true, input);
					}
					mainWin->autoF5_AfterFinish = false;
					logStandard (input, andorWin->getLogger (), andorWin->getAlignmentVals ());
					startExperimentThread (mainWin, input);
				}
				catch (ChimeraError & err) {
					if (err.whatBare () == "CANCEL") {
						mainWin->reportStatus ("Canceled camera initialization.\r\n");
						break;
					}
					mainWin->reportErr ("EXITED WITH ERROR!\n " + err.qtrace ());
					mainWin->reportStatus ("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					andorWin->setTimerText ("ERROR!");
					andorWin->assertOff ();
					break;
				}
				break;
			}
			case ID_ACCELERATOR_ESC:
			case ID_FILE_ABORT_GENERATION: {
				std::string status;
				bool niawgAborted = false, andorAborted = false, masterAborted = false, baslerAborted = false;
				try {
					if (basWin->baslerCameraIsRunning ()) {
						status = "Basler";
						basWin->handleDisarmPress ();
						baslerAborted = true;
					}
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("error while aborting basler! Error Message: " + err.qtrace ());
					if (status == "Basler") {
					}
					mainWin->reportStatus ("EXITED WITH ERROR!\r\n");
					andorWin->setTimerText ("ERROR!");
				}
				try {
					if (mainWin->expIsRunning ()) {
						status = "MASTER";
						commonFunctions::abortMaster (win);
						masterAborted = true;
					}
					andorWin->assertOff ();
					andorWin->assertDataFileClosed ();
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("Abort Master thread exited with Error! Error Message: "
						+ err.qtrace ());
					mainWin->reportStatus ("Abort Master thread exited with Error!\r\n");
					andorWin->setTimerText ("ERROR!");
				}
				try {
					if (andorWin->andor.isRunning ()) {
						status = "ANDOR";
						commonFunctions::abortCamera (win);
						andorAborted = true;
					}
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("Andor Camera threw error while aborting! Error: " + err.qtrace ());
					mainWin->reportStatus ("Abort camera threw error\r\n");
					andorWin->setTimerText ("ERROR!");
				}
				try {
					if (scriptWin->niawgIsRunning ()) {
						status = "NIAWG";
						abortNiawg (win);
						niawgAborted = true;
					}
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("Abor NIAWG exited with Error! Error Message: " + err.qtrace ());
					if (status == "NIAWG") { //?
					}
					mainWin->reportStatus ("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					andorWin->setTimerText ("ERROR!");
				}
				if (!niawgAborted && !andorAborted && !masterAborted && !baslerAborted) {
					for (auto& dev : mainWin->getDevices ().list) {
						mainWin->handleColorboxUpdate ("Black", qstr (dev.get ().getDelim ()));
					}
					mainWin->reportErr ("Andor camera, NIAWG, Master, and Basler camera were not running. "
						"Can't Abort.\r\n");
				}
				break;
			}
			case ID_RUNMENU_ABORTMASTER: {
				if (mainWin->experimentIsPaused ()) {
					mainWin->reportErr ("Experiment is paused. Please unpause before aborting.\r\n");
					break;
				}
				commonFunctions::abortMaster (win);
				break;
			}
									   /// File Management 
			case ID_ACCELERATOR40121:
			case ID_FILE_SAVEALL: {
				try {
					scriptWin->saveNiawgScript ();
					scriptWin->saveIntensityScript ();
					scriptWin->saveMasterScript ();
					auxWin->updateAgilent (AgilentEnum::name::TopBottom);
					auxWin->updateAgilent (AgilentEnum::name::Axial);
					auxWin->updateAgilent (AgilentEnum::name::Flashing);
					auxWin->updateAgilent (AgilentEnum::name::Microwave);
					mainWin->profile.saveConfiguration (win);
					mainWin->masterConfig.save (mainWin, auxWin, andorWin);
				}
				catch (ChimeraError & err) {
					mainWin->reportErr (err.qtrace ());
				}
				break;
			}
			case ID_FILE_MY_EXIT: {
				try {
					commonFunctions::exitProgram (win);
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("ERROR! " + err.qtrace ());
				}
				break;
			}
			case ID_RUNMENU_ABORTCAMERA: {
				try {
					if (andorWin->andor.isRunning ()) {
						commonFunctions::abortCamera (win);
					}
					else {
						mainWin->reportErr ("Camera was not running. Can't Abort.\r\n");
					}
					andorWin->assertOff ();
				}
				catch (ChimeraError & except) {
					mainWin->reportErr ("EXITED WITH ERROR!\n" + except.qtrace ());
					mainWin->reportStatus ("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					andorWin->setTimerText ("ERROR!");
				}
				break;
			}
			case ID_RUNMENU_ABORTNIAWG: {
				try {
					if (scriptWin->niawgIsRunning ()) {
						commonFunctions::abortNiawg (win);
					}
					else {
						mainWin->reportErr ("NIAWG was not running. Can't Abort.\r\n");
					}
				}
				catch (ChimeraError & except) {
					mainWin->reportErr ("EXITED WITH ERROR!" + except.qtrace ());
					mainWin->reportStatus ("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					andorWin->setTimerText ("ERROR!");
				}
				break;
			}
			case ID_ACCELERATOR_F1: {
				try {
					AllExperimentInput input;
					input.masterInput = new ExperimentThreadInput (win);
					input.masterInput->updatePlotterXVals = false;
					auxWin->fillMasterThreadInput (input.masterInput);
					mainWin->fillMotInput (input.masterInput);
					input.masterInput->expType = ExperimentType::LoadMot;
					mainWin->startExperimentThread (input.masterInput);
				}
				catch (ChimeraError & err) {
					mainWin->reportErr (err.qtrace ());
				}
				break;
			}
			case ID_ACCELERATOR_F11: {
				if (mainWin->experimentIsRunning) {
					return;
				}
				if (mainWin->getMainOptions ().delayAutoCal) {
					mainWin->handleNotification ("Delaying Auto-Calibration!\n");
					return;
				}
				if (QDateTime::currentDateTime ().time ().hour () < 4) {
					// This should set up the calibration to run at 4AM. 
					return;
				};
				// F11 is the set of calibrations.
				AllExperimentInput input;
				input.masterInput = new ExperimentThreadInput (win);
				input.masterInput->quiet = true;
				try {
					auxWin->fillMasterThreadInput (input.masterInput);
					andorWin->fillMasterThreadInput (input.masterInput);
					auto calNum = andorWin->getDataCalNum ();
					if (calNum == -1) {
						return;
					}
					input.masterInput->calibrations = mainWin->getCalInfo ();
					// automatically save; this is important to handle changes like the auto servo and auto carrier
					commonFunctions::handleCommonMessage (ID_FILE_SAVEALL, win);
					auto& calInfo = AUTO_CAL_LIST[calNum];
					mainWin->reportStatus (qstr (calInfo.infoStr));
					input.masterInput->profile = calInfo.prof;
					input.masterInput->expType = ExperimentType::AutoCal;
					logStandard (input, andorWin->getLogger (), andorWin->getAlignmentVals (), calInfo.fileName, false);
					startExperimentThread (mainWin, input);
				}
				catch (ChimeraError & err) {
					mainWin->reportErr ("Failed to start auto calibration experiment: " + err.qtrace ());
				}
				break;
			}
								   // the rest of these are all one-liners. 			
			case ID_PROFILE_SAVE_PROFILE: { mainWin->profile.saveConfiguration (win); break; }
			case ID_PLOTTING_STOPPLOTTER: { andorWin->stopPlotter (); break; }
			case ID_FILE_MY_INTENSITY_NEW: { scriptWin->newIntensityScript (); break; }
			case ID_FILE_MY_INTENSITY_OPEN: { scriptWin->openIntensityScript (win); break; }
			case ID_FILE_MY_INTENSITY_SAVE: { scriptWin->saveIntensityScript (); break; }
			case ID_FILE_MY_INTENSITY_SAVEAS: { scriptWin->saveIntensityScriptAs (win); break; }
			case ID_ACCELERATOR_F2: case ID_RUNMENU_PAUSE: { mainWin->handlePauseToggle (); break; }
			case ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION: { mainWin->profile.renameConfiguration (); break; }
			case ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION: { mainWin->profile.deleteConfiguration (); break; }
			case ID_CONFIGURATION_SAVE_CONFIGURATION_AS: { mainWin->profile.saveConfigurationAs (win); break; }
			case ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS: { mainWin->profile.saveConfiguration (win); break; }
			case ID_MASTERSCRIPT_NEW: { scriptWin->newMasterScript (); break; }
			case ID_MASTERSCRIPT_SAVE: { scriptWin->saveMasterScript (); break; }
			case ID_MASTERSCRIPT_SAVEAS: { scriptWin->saveMasterScriptAs (win); break; }
			case ID_MASTERSCRIPT_OPENSCRIPT: { scriptWin->openMasterScript (win); break; }
			case ID_MASTERSCRIPT_NEWFUNCTION: { scriptWin->newMasterFunction ();	break; }
			case ID_MASTERSCRIPT_SAVEFUNCTION: { scriptWin->saveMasterFunction (); break; }
			case ID_FILE_MY_NIAWG_NEW: { scriptWin->newNiawgScript (); break; }
			case ID_FILE_MY_NIAWG_OPEN: { scriptWin->openNiawgScript (win); break; }
			case ID_FILE_MY_NIAWG_SAVE: { scriptWin->saveNiawgScript (); break; }
			case ID_FILE_MY_NIAWG_SAVEAS: { scriptWin->saveNiawgScriptAs (win); break; }
			case ID_MASTERCONFIG_SAVEMASTERCONFIGURATION: { mainWin->masterConfig.save (mainWin, auxWin, andorWin); break; }
			case ID_MASTERCONFIGURATION_RELOAD_MASTER_CONFIG: { mainWin->masterConfig.load (mainWin, auxWin, andorWin); break; }

			case ID_TOP_BOTTOM_NEW_SCRIPT: { auxWin->newAgilentScript (AgilentEnum::name::TopBottom); break; }
			case ID_TOP_BOTTOM_OPEN_SCRIPT: { auxWin->openAgilentScript (AgilentEnum::name::TopBottom, win); break; }
			case ID_TOP_BOTTOM_SAVE_SCRIPT: { auxWin->saveAgilentScript (AgilentEnum::name::TopBottom); break; }
			case ID_TOP_BOTTOM_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs (AgilentEnum::name::TopBottom, win); break; }

			case ID_AXIAL_NEW_SCRIPT: { auxWin->newAgilentScript (AgilentEnum::name::Axial); break; }
			case ID_AXIAL_OPEN_SCRIPT: { auxWin->openAgilentScript (AgilentEnum::name::Axial, win); break; }
			case ID_AXIAL_SAVE_SCRIPT: { auxWin->saveAgilentScript (AgilentEnum::name::Axial); break; }
			case ID_AXIAL_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs (AgilentEnum::name::Axial, win); break; }
										/*
										case ID_FLASHING_NEW_SCRIPT: { auxWin->newAgilentScript( whichAg::Flashing ); break; }
										case ID_FLASHING_OPEN_SCRIPT: { auxWin->openAgilentScript( whichAg::Flashing, win ); break; }
										case ID_FLASHING_SAVE_SCRIPT: { auxWin->saveAgilentScript( whichAg::Flashing ); break; }
										case ID_FLASHING_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs( whichAg::Flashing, win); break; }
										case ID_UWAVE_NEW_SCRIPT: { auxWin->newAgilentScript( whichAg::Microwave ); break; }
										case ID_UWAVE_OPEN_SCRIPT: { auxWin->openAgilentScript( whichAg::Microwave, win ); break; }
										case ID_UWAVE_SAVE_SCRIPT: { auxWin->saveAgilentScript( whichAg::Microwave ); break; }
										case ID_UWAVE_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs( whichAg::Microwave, win); break; }

										case ID_NIAWG_RELOADDEFAULTWAVEFORMS: { commonFunctions::reloadNIAWGDefaults(mainWin, scriptWin); break; }

										case ID_NIAWG_SENDSOFTWARETRIGGER: { scriptWin->sendNiawgSoftwareTrig(); break; }
										case ID_NIAWG_STREAMWAVEFORM: { scriptWin->streamNiawgWaveform(); break; }
										case ID_NIAWG_GETNIAWGERROR: { errBox(scriptWin->getNiawgErr()); break; }
										case ID_BASLER_AUTOSCALE: { basWin->handleBaslerAutoscaleSelection ( ); break; }
										case ID_PICTURES_GREATER_THAN_MAX_SPECIAL: { andorWin->handleSpecialGreaterThanMaxSelection(); break; }
										case ID_PICTURES_LESS_THAN_MIN_SPECIAL: { andorWin->handleSpecialLessThanMinSelection(); break; }
										case ID_PICTURES_ALWAYSSHOWGRID: { andorWin->passAlwaysShowGrid(); break; }
										case ID_NIAWG_NIAWGISON: { scriptWin->passNiawgIsOnPress( ); break; }
										case ID_DATATYPE_PHOTONS_COLLECTED: { andorWin->setDataType( CAMERA_PHOTONS ); break; }
										case ID_DATATYPE_PHOTONS_SCATTERED: { andorWin->setDataType( ATOM_PHOTONS ); break; }
										case ID_DATATYPE_RAW_COUNTS: { andorWin->setDataType( RAW_COUNTS ); break; }
										case ID_RUNMENU_ABORTBASLER: { basWin->handleDisarmPress ( ); break; }


										case ID_MASTER_VIEWORCHANGEINDIVIDUALDACSETTINGS: { auxWin->ViewOrChangeDACNames(); break; }
										case ID_MASTER_VIEWORCHANGETTLNAMES: { auxWin->ViewOrChangeTTLNames(); break; }
										case ID_HELP_HARDWARESTATUS: { mainWin->showHardwareStatus ( ); break; }
										*/
			default:
				errBox ("Common message passed but not handled! The feature you're trying to use"\
					" feature likely needs re-implementation / new handling.");
			}
		}
		catch (ChimeraError & err) {
			mainWin->reportErr (err.qtrace ());
		}
	}

	void calibrateCameraBackground(IChimeraQtWindow* win){
		try {
			AllExperimentInput input;
			input.masterInput = new ExperimentThreadInput ( win );
			win->auxWin->fillMasterThreadInput (input.masterInput);
			win->andorWin->loadCameraCalSettings( input );
			win->mainWin->loadCameraCalSettings( input.masterInput );
			win->mainWin->startExperimentThread( input.masterInput );
		}
		catch ( ChimeraError& err ){
			errBox( err.trace( ) );
		}
	}

	void prepareMasterThread( int msgID, IChimeraQtWindow* win, AllExperimentInput& input, bool runNiawg,
							  bool runMaster, bool runAndor, bool runBasler, bool updatePlotXVals )	{
		if (win->scriptWin->niawgIsRunning()){
			thrower ( "NIAWG is already running! Please Restart the niawg before running an experiment.\r\n" );
		}
		win->mainWin->checkProfileSave();
		win->scriptWin->checkScriptSaves( );
		// Set the thread structure.
		input.masterInput = new ExperimentThreadInput ( win );
		input.masterInput->updatePlotterXVals = updatePlotXVals;
		input.masterInput->skipNext = win->andorWin->getSkipNextAtomic( );
		input.masterInput->numVariations = win->auxWin->getTotalVariationNumber ( );
		input.masterInput->sleepTime = win->mainWin->getDebuggingOptions ().sleepTime;
		input.masterInput->profile = win->mainWin->getProfileSettings ();
		// Start the programming thread. order is important.
		win->auxWin->fillMasterThreadInput( input.masterInput );
		win->mainWin->fillMasterThreadInput( input.masterInput );
		win->andorWin->fillMasterThreadInput( input.masterInput );
	}

	void startExperimentThread(IChimeraQtWindow* win, AllExperimentInput& input){
		win->mainWin->addTimebar( "main" );
		win->mainWin->addTimebar( "error" );
		win->mainWin->startExperimentThread( input.masterInput );
	}

	void abortCamera(IChimeraQtWindow* win){
		if (!win->andorWin->cameraIsRunning()){
			win->mainWin->reportErr ( "System was not running. Can't Abort.\r\n" );
			return;
		}
		std::string errorMessage;
		// abort acquisition if in progress
		win->andorWin->abortCameraRun();
		win->mainWin->reportStatus( "Aborted Camera Operation.\r\n" );
	}


	void abortNiawg(IChimeraQtWindow* win){
		// set reset flag
		if (!win->scriptWin->niawgIsRunning()){
			std::string msgString = "Passively Outputting Default Waveform.";
			win->mainWin->onErrorMessage( "System was not running. Can't Abort.\r\n" );
			return;
		}
		// abort the generation on the NIAWG.
		win->scriptWin->setIntensityDefault();
		win->mainWin->reportStatus ( "Aborted NIAWG Operation. Passively Outputting Default Waveform.\r\n" );
		win->scriptWin->restartNiawgDefaults();
		win->scriptWin->setNiawgRunningState( false );
	}


	void abortMaster( IChimeraQtWindow* win ){
		win->mainWin->abortMasterThread();
		win->auxWin->handleAbort();
	}


	void forceExit (IChimeraQtWindow* win){
		/// Exiting. Close the NIAWG normally.
		try	{
			win->scriptWin->stopNiawg ( );
		}
		catch ( ChimeraError& except ){
			errBox ( "The NIAWG did not exit smoothly: " + except.trace ( ) );
		}
		//PostQuitMessage ( 1 );
	}


	void exitProgram(IChimeraQtWindow* win)	{
		if (win->scriptWin->niawgIsRunning()){
			thrower ( "The NIAWG is Currently Running. Please stop the system before exiting so that devices devices "
					  "can stop normally." );
		}
		if (win->andorWin->cameraIsRunning()){
			thrower ( "The Camera is Currently Running. Please stop the system before exiting so that devices devices "
					  "can stop normally." );
		}
		if (win->mainWin->masterIsRunning()){
			thrower ( "The Master system (ttls & aoSys) is currently running. Please stop the system before exiting so "
					  "that devices can stop normally." );
		}
		win->scriptWin->checkScriptSaves( );
		win->mainWin->checkProfileSave();
		std::string exitQuestion = "Are you sure you want to exit?\n\nThis will stop all output of the NI arbitrary "
			"waveform generator. The Andor camera temperature control will also stop, causing the Andor camera to "
			"return to room temperature.";
		auto areYouSure = QMessageBox::question (win, "Exit?", qstr(exitQuestion));
		if (areYouSure == QMessageBox::Yes){
			forceExit ( win );
		}
	}


	void reloadNIAWGDefaults( QtMainWindow* mainWin, QtScriptWindow* scriptWin ){
		// this hasn't actually been used or tested in a long time... aug 29th 2019
		profileSettings profile = mainWin->getProfileSettings();
		if (scriptWin->niawgIsRunning()){
			thrower ( "The system is currently running. You cannot reload the default waveforms while the system is "
					  "running. Please restart the system before attempting to reload default waveforms." );
		}
		auto yesno = QMessageBox::question (mainWin, "Reload Niawg Waveforms?",
			"Reload the default waveforms from (presumably) updated files? Please make sure that "
			"the updated files are syntactically correct, or else the program will crash.");
		if (yesno == QMessageBox::No){
			return;
		}
		try{
			scriptWin->setNiawgDefaults();
			scriptWin->restartNiawgDefaults();
		}
		catch (ChimeraError&){
			scriptWin->restartNiawgDefaults();
			throwNested( "failed to reload the niawg default waveforms!" );
		}
		mainWin->reportStatus ( "Reloaded Default Waveforms.\r\nInitialized Default Waveform.\r\n" );
	}


	void logStandard( AllExperimentInput input, DataLogger& logger, piezoChan<double> cameraPiezoVals,
					  std::string specialName, bool needsCal ){
		logger.initializeDataFiles( specialName, needsCal );
		logger.logMasterInput( input.masterInput );
		logger.logMiscellaneousStart();
		logger.initializeAiLogging( input.masterInput->numAiMeasurements );
		logger.logAndorPiezos (cameraPiezoVals);
	}


	bool getPermissionToStart( IChimeraQtWindow* win, bool runNiawg, bool runMaster, AllExperimentInput& input ){
		std::string startMsg = "Current Settings:\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n";
		startMsg = win->andorWin->getStartMessage( );
		if ( runNiawg ){
			scriptInfo<std::string> scriptNames = win->scriptWin->getScriptNames( );
			// ordering matters here, make sure you get the correct script name.
			std::string niawgNameString( scriptNames.niawg );
			std::string intensityNameString( scriptNames.intensityAgilent );
			std::string sequenceInfo = "";
			if ( sequenceInfo != "" ){
				startMsg += sequenceInfo;
			}
			else{
				scriptInfo<bool> scriptSavedStatus = win->scriptWin->getScriptSavedStatuses( );
				startMsg += "NIAWG Script Name:...... " + str( niawgNameString );
				if ( scriptSavedStatus.niawg ){
					startMsg += " SAVED\r\n";
				}
				else{
					startMsg += " NOT SAVED\r\n";
				}
				startMsg += "Intensity Script Name:....... " + str( intensityNameString );
				if ( scriptSavedStatus.intensityAgilent ){
					startMsg += " SAVED\r\n";
				}
				else{
					startMsg += " NOT SAVED\r\n";
				}
			}
			startMsg += "\r\n";
		}
		startMsg += "\r\n\r\nBegin Experiment with these Settings?";
		//StartDialog dlg( startMsg, IDD_BEGINNING_SETTINGS );
		//bool areYouSure = dlg.DoModal( );
		//if ( !areYouSure )
		//{
		//	thrower ( "CANCEL!" );
		//}
		return true;
	}
};
