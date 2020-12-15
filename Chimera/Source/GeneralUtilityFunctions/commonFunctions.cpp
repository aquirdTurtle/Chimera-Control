// created by Mark O. Brown
#include "stdafx.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
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
					prepareMasterThread (msgID, win, input, true, true, true, true);
					input.masterInput->expType = ExperimentType::Normal;
					if (!mainWin->autoF5_AfterFinish) {
						commonFunctions::getPermissionToStart (win, true, input);
					}
					mainWin->autoF5_AfterFinish = false;
					logStandard (input, andorWin->getLogger ());
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
				bool andorAborted = false, masterAborted = false, baslerAborted = false;
				andorWin->wakeRearranger ();
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
				//
				if ( !andorAborted && !masterAborted && !baslerAborted) {
					for (auto& dev : mainWin->getDevices ().list) {
						mainWin->handleColorboxUpdate ("Black", qstr (dev.get ().getDelim ()));
					}
					mainWin->reportErr ("Andor camera, Master, and Basler camera were not running. "
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
					logStandard (input, andorWin->getLogger (), calInfo.fileName, false);
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
										
										case ID_PICTURES_AUTOSCALEPICTURES: { andorWin->handleAutoscaleSelection(); break; }
										case ID_BASLER_AUTOSCALE: { basWin->handleBaslerAutoscaleSelection ( ); break; }
										case ID_PICTURES_GREATER_THAN_MAX_SPECIAL: { andorWin->handleSpecialGreaterThanMaxSelection(); break; }
										case ID_PICTURES_LESS_THAN_MIN_SPECIAL: { andorWin->handleSpecialLessThanMinSelection(); break; }
										case ID_PICTURES_ALWAYSSHOWGRID: { andorWin->passAlwaysShowGrid(); break; }
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

	void prepareMasterThread( int msgID, IChimeraQtWindow* win, AllExperimentInput& input, 
							  bool runMaster, bool runAndor, bool runBasler, bool updatePlotXVals )	{
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

	void abortMaster( IChimeraQtWindow* win ){
		win->mainWin->abortMasterThread();
		win->auxWin->handleAbort();
	}

	void forceExit (IChimeraQtWindow* win){
		PostQuitMessage ( 1 );
	}


	void exitProgram(IChimeraQtWindow* win)	{
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

	void logStandard( AllExperimentInput input, DataLogger& logger, 
					  std::string specialName, bool needsCal ){
		logger.initializeDataFiles( specialName, needsCal );
		logger.logMasterInput( input.masterInput );
		logger.logMiscellaneousStart();
		logger.initializeAiLogging( input.masterInput->numAiMeasurements );
	}

	bool getPermissionToStart( IChimeraQtWindow* win, bool runMaster, AllExperimentInput& input ){
		std::string startMsg = "Current Settings:\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n";
		startMsg = win->andorWin->getStartMessage( );
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
