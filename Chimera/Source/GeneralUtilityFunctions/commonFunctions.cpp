// created by Mark O. Brown
#include "stdafx.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "NIAWG/NiawgCore.h"
#include "ExcessDialogs/TextPromptDialog.h"
#include "ExcessDialogs/StartDialog.h"
#include "ExcessDialogs/openWithExplorer.h"
#include "ExcessDialogs/saveWithExplorer.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/MainWindow.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "PrimaryWindows/ScriptingWindow.h"
#include "PrimaryWindows/BaslerWindow.h"
#include "LowLevel/externals.h"
#include "ExperimentThread/ExperimentType.h"
#include "ExperimentThread/autoCalConfigInfo.h"

// Functions called by all windows to do the same thing, mostly things that happen on menu presses or hotkeys
namespace commonFunctions
{
	// this function handles messages that all windows can recieve, e.g. accelerator keys and menu messages. It 
	// redirects everything to all of the other functions below, for the most part.
	void handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin, 
							  AndorWindow* andorWin, AuxiliaryWindow* auxWin, BaslerWindow* basWin )
	{
		switch (msgID)
		{
			case ID_MACHINE_OPTIMIZATION:
			{
				// this is mostly prepared like F5.
				if ( andorWin->wantsAutoCal ( ) && !andorWin->wasJustCalibrated ( ) )
				{
					andorWin->PostMessageA ( WM_COMMAND, MAKEWPARAM ( IDC_CAMERA_CALIBRATION_BUTTON, 0 ) );
					return;
				}
				AllExperimentInput input;
				andorWin->redrawPictures ( false );
				try
				{
					mainWin->getComm ( )->sendStatus( "Starting Automatic Optimization...\r\n" );
					mainWin->getComm ( )->sendTimer ( "Starting..." );
					prepareMasterThread ( msgID, scriptWin, mainWin, andorWin, auxWin, basWin, input, 
										  true, true, true, true, true );
					andorWin->preparePlotter ( input );
					andorWin->prepareAtomCruncher ( input );
					input.masterInput->quiet = true;
					logStandard ( input, andorWin->getLogger ( ), mainWin->getServoinfo() );

					auxWin->updateOptimization ( input );
					input.masterInput->expType = ExperimentType::MachineOptimization;
					andorWin->startPlotterThread ( input );
					andorWin->startAtomCruncher ( input );
					startExperimentThread ( mainWin, input );
				}
				catch ( Error& err )
				{
					if ( err.whatBare ( ) == "CANCEL" )
					{
						mainWin->getComm ( )->sendStatus ( "Canceled camera initialization.\r\n" );
						mainWin->getComm ( )->sendColorBox ( System::Niawg, 'B' );
						break;
					}
					mainWin->getComm ( )->sendError ( "EXITED WITH ERROR! " + err.trace ( ) );
					mainWin->getComm ( )->sendColorBox ( System::Camera, 'R' );
					mainWin->getComm ( )->sendStatus ( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					mainWin->getComm ( )->sendTimer ( "ERROR!" );
					andorWin->assertOff ( );
					break;
				}
				break;
			}
			case ID_RUNMENU_RUNBASLERANDMASTER:
			{
				AllExperimentInput input;
				try
				{
					mainWin->getComm ( )->sendTimer ( "Starting..." );
					prepareMasterThread ( msgID, scriptWin, mainWin, andorWin, auxWin, basWin, input, false, true, false, 
										  true, false );
					commonFunctions::getPermissionToStart ( andorWin, mainWin, scriptWin, auxWin, false, true, input );
					input.masterInput->expType = ExperimentType::Normal;
					logStandard ( input, andorWin->getLogger ( ), mainWin->getServoinfo (), "", false );
					startExperimentThread ( mainWin, input );
				}
				catch ( Error& err )
				{
					mainWin->getComm ( )->sendError ( "EXITED WITH ERROR!\n " + err.trace ( ) );
					mainWin->getComm ( )->sendColorBox ( System::Basler, 'R' );
					mainWin->getComm ( )->sendColorBox ( System::Master, 'R' );
					mainWin->getComm ( )->sendStatus ( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					mainWin->getComm ( )->sendTimer ( "ERROR!" );
					andorWin->assertOff ( );
					break;
				}
				break;
			}
			case ID_FILE_RUN_EVERYTHING:
			case ID_ACCELERATOR_F5:
			case ID_FILE_MY_WRITE_WAVEFORMS:
			{
				if ( andorWin->wantsAutoCal( ) && !andorWin->wasJustCalibrated( ) )
				{
					andorWin->PostMessageA( WM_COMMAND, MAKEWPARAM( IDC_CAMERA_CALIBRATION_BUTTON, 0 ) );
					return;
				}
				AllExperimentInput input;
				try
				{
					if ( mainWin->masterIsRunning() )
					{
						auto response = promptBox ( "The Master system is already running. Would you like to run the "
													"current configuration when master finishes? This effectively "
													"auto-presses F5 when complete and skips confirmation.", MB_YESNO );
						if ( response == IDYES )
						{
							mainWin->autoF5_AfterFinish = true;
						}
						break;
					}
					mainWin->getComm ( )->sendTimer ( "Starting..." );
					prepareMasterThread( msgID, scriptWin, mainWin, andorWin, auxWin, basWin, input, true, true, 
										 true, true, true );
					input.masterInput->expType = ExperimentType::Normal;
					if ( !mainWin->autoF5_AfterFinish )
					{
						commonFunctions::getPermissionToStart ( andorWin, mainWin, scriptWin, auxWin, true, true, input );
					}
					mainWin->autoF5_AfterFinish = false;
					andorWin->preparePlotter( input );
					andorWin->prepareAtomCruncher( input );
					logStandard( input, andorWin->getLogger ( ), mainWin->getServoinfo ());
					andorWin->startAtomCruncher(input);
					andorWin->startPlotterThread ( input );
					startExperimentThread( mainWin, input );
				}
				catch (Error& err)
				{
					if (err.whatBare() == "CANCEL")
					{
						mainWin->getComm()->sendStatus("Canceled camera initialization.\r\n");
						mainWin->getComm()->sendColorBox( System::Niawg, 'B' );
						break;
					}
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + err.trace());
					mainWin->getComm()->sendColorBox( System::Camera, 'R' );
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
					andorWin->assertOff();
					break;
				}
				break;
			}
			case WM_CLOSE:
			case ID_ACCELERATOR_ESC:
			case ID_FILE_ABORT_GENERATION:
			{
				std::string status;
				bool niawgAborted = false, andorAborted = false, masterAborted = false, baslerAborted = false;

				scriptWin->stopRearranger( );
				andorWin->wakeRearranger( );
				try
				{
					if (basWin->baslerCameraIsRunning ())
					{
						status = "Basler";
						basWin->handleDisarmPress ();
						baslerAborted = true;
					}
					mainWin->getComm ()->sendColorBox (System::Basler, 'B');
				}
				catch (Error & err)
				{
					mainWin->getComm ()->sendError ("error while aborting basler! Error Message: " + err.trace ());
					if (status == "Basler")
					{
						mainWin->getComm ()->sendColorBox (System::Basler, 'R');
					}
					mainWin->getComm ()->sendStatus ("EXITED WITH ERROR!\r\n");
					mainWin->getComm ()->sendTimer ("ERROR!");
				}


				try
				{
					if ( mainWin->expThreadManager.runningStatus( ) )
					{
						status = "MASTER";
						commonFunctions::abortMaster( mainWin, auxWin );
						masterAborted = true;
					}
					mainWin->getComm( )->sendColorBox( System::Master, 'B' );
					andorWin->assertOff( );
				}
				catch ( Error& err )
				{
					mainWin->getComm( )->sendError( "Abort Master thread exited with Error! Error Message: " 
													+ err.trace( ) );
					mainWin->getComm( )->sendColorBox( System::Master, 'R' );
					mainWin->getComm( )->sendStatus( "Abort Master thread exited with Error!\r\n" );
					mainWin->getComm( )->sendTimer( "ERROR!" );
				}
				try
				{
					if ( andorWin->andor.isRunning( ) )
					{
						status = "ANDOR";
						commonFunctions::abortCamera( andorWin, mainWin );
						andorAborted = true;
					}
					mainWin->getComm( )->sendColorBox( System::Camera, 'B' );
				}
				catch ( Error& err )
				{
					mainWin->getComm( )->sendError( "Andor Camera threw error while aborting! Error: " + err.trace( ) );
					mainWin->getComm( )->sendColorBox( System::Camera, 'R' );
					mainWin->getComm( )->sendStatus( "Abort camera threw error\r\n" );
					mainWin->getComm( )->sendTimer( "ERROR!" );
				}
				//
				scriptWin->waitForRearranger( );

				try
				{
					if ( scriptWin->niawgIsRunning() )
					{
						status = "NIAWG";
						abortNiawg( scriptWin, mainWin );
						niawgAborted = true;
					}
					mainWin->getComm( )->sendColorBox( System::Niawg, 'B' );
				}
				catch ( Error& err )
				{
					mainWin->getComm( )->sendError( "Abor NIAWG exited with Error! Error Message: " + err.trace( ) );
					if ( status == "NIAWG" )
					{
						mainWin->getComm( )->sendColorBox( System::Niawg, 'R' );
					}
					mainWin->getComm( )->sendStatus( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					mainWin->getComm( )->sendTimer( "ERROR!" );
				}
				if (!niawgAborted && !andorAborted && !masterAborted && !baslerAborted)
				{
					mainWin->getComm ( )->sendError ( "Andor camera, NIAWG, Master, and Basler camera were not running. "
													  "Can't Abort.\r\n" );
				}
				break;
			}
			case ID_RUNMENU_RUNCAMERA:
			{
				AllExperimentInput input;
				try
				{
					mainWin->getComm ( )->sendColorBox ( System::Camera, 'Y' );
					mainWin->getComm ( )->sendTimer ( "Starting..." );
					commonFunctions::prepareMasterThread ( ID_RUNMENU_RUNCAMERA, scriptWin, mainWin, andorWin, auxWin,
														   basWin, input, false, false, true, false, true );
					input.masterInput->expType = ExperimentType::Normal;
					andorWin->preparePlotter ( input );
					andorWin->prepareAtomCruncher ( input );
					commonFunctions::getPermissionToStart ( andorWin, mainWin, scriptWin, auxWin, false, false, input );
					andorWin->startAtomCruncher ( input );
					andorWin->startPlotterThread ( input );
					logStandard ( input, andorWin->getLogger ( ), mainWin->getServoinfo (), "", false );
					commonFunctions::startExperimentThread ( mainWin, input );
					mainWin->getComm()->sendColorBox( System::Camera, 'G' );
					mainWin->getComm()->sendStatus("Camera is Running.\r\n");
				}
				catch (Error& exception)
				{
					if (exception.whatBare() == "CANCEL")
					{
						mainWin->getComm()->sendColorBox( System::Camera, 'B' );
						mainWin->getComm()->sendStatus("Camera is Not Running, User Canceled.\r\n");
						break;
					}
					mainWin->getComm()->sendColorBox( System::Camera, 'R' );
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + exception.trace());
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
					mainWin->getComm()->sendTimer("ERROR!");
					andorWin->assertOff();
				}
				break;
			}
			case ID_RUNMENU_RUNNIAWG:
			{
				AllExperimentInput input;
				try
				{
					commonFunctions::prepareMasterThread( ID_RUNMENU_RUNNIAWG, scriptWin, mainWin, andorWin, auxWin, 
														  basWin,	input, true, false, false, false, false );
					input.masterInput->expType = ExperimentType::Normal;
					commonFunctions::getPermissionToStart( andorWin, mainWin, scriptWin, auxWin, true, false, input );
					commonFunctions::logStandard( input, andorWin->getLogger ( ), mainWin->getServoinfo (), "", false );
					commonFunctions::startExperimentThread( mainWin, input );
				}
				catch (Error& except)
				{
					mainWin->getComm()->sendColorBox( System::Niawg, 'R' );
					mainWin->getComm()->sendError("EXITED WITH ERROR! " + except.trace());
					mainWin->getComm()->sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
				}
				break;
			}
			case ID_RUNMENU_RUNMASTER:
			{
				AllExperimentInput input;
				try
				{
					commonFunctions::prepareMasterThread( ID_RUNMENU_RUNMASTER, scriptWin, mainWin, andorWin, auxWin, 
														  basWin, input, false, true, false, false, false );
					input.masterInput->expType = ExperimentType::Normal;
					commonFunctions::getPermissionToStart( andorWin, mainWin, scriptWin, auxWin, false, true, input );
					commonFunctions::logStandard ( input, andorWin->getLogger ( ), mainWin->getServoinfo (), "", false );
					commonFunctions::startExperimentThread( mainWin, input );
				}
				catch (Error& err)
				{
					if (err.whatBare() == "Canceled!")
					{
						break;
					}
					mainWin->getComm()->sendColorBox( System::Master, 'R' );
					mainWin->getComm()->sendError( "EXITED WITH ERROR! " + err.trace() );
					mainWin->getComm()->sendStatus( "EXITED WITH ERROR!\r\n" );
				}
				break;
			}
			case ID_RUNMENU_ABORTMASTER:
			{
				if ( mainWin->experimentIsPaused( ) )
				{
					mainWin->getComm( )->sendError( "Experiment is paused. Please unpause before aborting.\r\n" );
					break;
				}
				commonFunctions::abortMaster(mainWin, auxWin);
				break;
			}
			/// File Management 
			case ID_ACCELERATOR40121:
			case ID_FILE_SAVEALL:
			{
				try
				{
					scriptWin->saveNiawgScript( );
					scriptWin->saveIntensityScript( );
					scriptWin->saveMasterScript( );
					auxWin->updateAgilent( whichAg::TopBottom );
					auxWin->updateAgilent( whichAg::Axial );
					auxWin->updateAgilent( whichAg::Flashing );
					auxWin->updateAgilent( whichAg::Microwave );
					mainWin->profile.saveEntireProfile( scriptWin, mainWin, auxWin, andorWin, basWin );
					mainWin->masterConfig.save( mainWin, auxWin, andorWin );
					
				}
				catch ( Error& err )
				{
					mainWin->getComm( )->sendError( err.trace( ) );
				}
				break;
			}
			case ID_FILE_MY_EXIT:
			{
				try
				{
					commonFunctions::exitProgram(scriptWin, mainWin, andorWin, auxWin);
				}
				catch (Error& err)
				{
					mainWin->getComm()->sendError("ERROR! " + err.trace());
				}
				break;
			}
			case ID_RUNMENU_RUNBASLER:
			{
				// the basler is the only large part of the code which doesn't go through the main experiment thread.
				// will probably change that at some point.
				AllExperimentInput input;
				errBox ("Run Basler (only) not functioning right now! needs code attention!");
				/*
				auto& logger = andorWin->getLogger ( );
				logger.initializeDataFiles ( "", true );
				logger.logAndorSettings ( input.AndorSettings, false );
				logger.logMasterInput ( NULL );
				logger.logMiscellaneousStart ( );
				logger.logBaslerSettings ( input.baslerRunSettings, true );
				break;*/
				// TODO: fix this to work with expthread
			}
			case ID_RUNMENU_ABORTCAMERA:
			{
				try
				{
					if ( andorWin->andor.isRunning ( ) )
					{
						commonFunctions::abortCamera ( andorWin, mainWin );
					}
					else
					{
						mainWin->getComm ( )->sendError ( "Camera was not running. Can't Abort.\r\n" );
					}
					mainWin->getComm ( )->sendColorBox ( System::Camera, 'B' );
					andorWin->assertOff ( );
				}
				catch ( Error& except )
				{
					mainWin->getComm ( )->sendError ( "EXITED WITH ERROR! " + except.trace ( ) );
					mainWin->getComm ( )->sendColorBox ( System::Camera, 'R' );
					mainWin->getComm ( )->sendStatus ( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					mainWin->getComm ( )->sendTimer ( "ERROR!" );
				}
				break;
			}
			case ID_RUNMENU_ABORTNIAWG:
			{
				try
				{
					if ( scriptWin->niawgIsRunning() )
					{
						commonFunctions::abortNiawg ( scriptWin, mainWin );
					}
					else
					{
						mainWin->getComm ( )->sendError ( "NIAWG was not running. Can't Abort.\r\n" );
					}
					mainWin->getComm ( )->sendColorBox ( System::Niawg, 'B' );
				}
				catch ( Error& except )
				{
					mainWin->getComm ( )->sendError ( "EXITED WITH ERROR! " + except.trace ( ) );
					mainWin->getComm ( )->sendColorBox ( System::Niawg, 'R' );
					mainWin->getComm ( )->sendStatus ( "EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n" );
					mainWin->getComm ( )->sendTimer ( "ERROR!" );
				}
				break;
			}
			case ID_ACCELERATOR_F1:
			{
				mainWin->autoServo ( 0, 0 );
				AllExperimentInput input;
				input.masterInput = new ExperimentThreadInput ( auxWin, mainWin, NULL, basWin, scriptWin);
				input.masterInput->runList.andor = false;
				input.masterInput->updatePlotterXVals = false;
				auxWin->fillMasterThreadInput ( input.masterInput );
				mainWin->fillMotInput ( input.masterInput );
				scriptWin->fillMotInput (input.masterInput);
				input.masterInput->expType = ExperimentType::LoadMot;
				mainWin->startExperimentThread ( input.masterInput );
				break;
			}
			case ID_ACCELERATOR_F11:
			{
				// F11 is the set of calibrations.
				AllExperimentInput input;
				input.masterInput = new ExperimentThreadInput ( auxWin, mainWin, andorWin, basWin, scriptWin );
				input.masterInput->quiet = true;
				try
				{
					auxWin->fillMasterThreadInput (input.masterInput);
					andorWin->fillMasterThreadInput (input.masterInput);
					scriptWin->fillMasterThreadInput (input.masterInput);
					auto calNum = mainWin->getAutoCalNumber ();
					if (calNum == 0)
					{
						mainWin->autoServo (0, 0);
					}
					auto& calInfo = AUTO_CAL_LIST[calNum];
					mainWin->getComm ()->sendStatus (calInfo.infoStr);
					input.masterInput->profile = calInfo.prof;
					input.masterInput->runList = calInfo.runList;
					input.masterInput->seq.name = calInfo.fileName;
					input.masterInput->seq.sequence.resize (1);
					input.masterInput->seq.sequence[0] = input.masterInput->profile;
					input.masterInput->expType = ExperimentType::AutoCal;
					logStandard (input, andorWin->getLogger (), mainWin->getServoinfo (), calInfo.fileName, false);
					startExperimentThread (mainWin, input);
				}
				catch (Error & err)
				{
					errBox ("Failed to start auto calibration experiment: " + err.trace());
				}
				break;
			}
			// the rest of these are all one-liners. 
			case ID_PROFILE_SAVE_PROFILE: { mainWin->profile.saveEntireProfile ( scriptWin, mainWin, auxWin, andorWin, basWin ); break; }
			case ID_PLOTTING_STOPPLOTTER: { andorWin->stopPlotter( ); break; }
			case ID_FILE_MY_INTENSITY_NEW: { scriptWin->newIntensityScript(); break; }
			case ID_FILE_MY_INTENSITY_OPEN: { scriptWin->openIntensityScript(parent); break; }
			case ID_FILE_MY_INTENSITY_SAVE: { scriptWin->saveIntensityScript(); break; }
			case ID_FILE_MY_INTENSITY_SAVEAS: { scriptWin->saveIntensityScriptAs(parent); break; }
			case ID_TOP_BOTTOM_NEW_SCRIPT: { auxWin->newAgilentScript( whichAg::TopBottom); break; }
			case ID_TOP_BOTTOM_OPEN_SCRIPT: { auxWin->openAgilentScript( whichAg::TopBottom, parent ); break; }
			case ID_TOP_BOTTOM_SAVE_SCRIPT: { auxWin->saveAgilentScript( whichAg::TopBottom); break; }
			case ID_TOP_BOTTOM_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs( whichAg::TopBottom, parent ); break; }
			case ID_AXIAL_NEW_SCRIPT: { auxWin->newAgilentScript( whichAg::Axial); break; }
			case ID_AXIAL_OPEN_SCRIPT: { auxWin->openAgilentScript( whichAg::Axial, parent ); break; }
			case ID_AXIAL_SAVE_SCRIPT: { auxWin->saveAgilentScript( whichAg::Axial); break; }
			case ID_AXIAL_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs( whichAg::Axial, parent ); break; }
			case ID_FLASHING_NEW_SCRIPT: { auxWin->newAgilentScript( whichAg::Flashing ); break; }
			case ID_FLASHING_OPEN_SCRIPT: { auxWin->openAgilentScript( whichAg::Flashing, parent ); break; }
			case ID_FLASHING_SAVE_SCRIPT: { auxWin->saveAgilentScript( whichAg::Flashing ); break; }
			case ID_FLASHING_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs( whichAg::Flashing, parent ); break; }
			case ID_UWAVE_NEW_SCRIPT: { auxWin->newAgilentScript( whichAg::Microwave ); break; }
			case ID_UWAVE_OPEN_SCRIPT: { auxWin->openAgilentScript( whichAg::Microwave, parent ); break; }
			case ID_UWAVE_SAVE_SCRIPT: { auxWin->saveAgilentScript( whichAg::Microwave ); break; }
			case ID_UWAVE_SAVE_SCRIPT_AS: { auxWin->saveAgilentScriptAs( whichAg::Microwave, parent ); break; }
			case ID_FILE_MY_NIAWG_NEW: { scriptWin->newNiawgScript(); break; }
			case ID_FILE_MY_NIAWG_OPEN: { scriptWin->openNiawgScript(parent); break; }
			case ID_FILE_MY_NIAWG_SAVE: { scriptWin->saveNiawgScript(); break; }
			case ID_FILE_MY_NIAWG_SAVEAS: { scriptWin->saveNiawgScriptAs(parent); break; }
			case ID_MASTERSCRIPT_NEW: { scriptWin->newMasterScript(); break; }
			case ID_MASTERSCRIPT_SAVE: { scriptWin->saveMasterScript(); break; }
			case ID_MASTERSCRIPT_SAVEAS: { scriptWin->saveMasterScriptAs(parent); break; }
			case ID_MASTERSCRIPT_OPENSCRIPT: { scriptWin->openMasterScript(parent); break; }
			case ID_MASTERSCRIPT_NEWFUNCTION: { scriptWin->newMasterFunction();	break; }
			case ID_MASTERSCRIPT_SAVEFUNCTION: { scriptWin->saveMasterFunction(); break; }
			case ID_SEQUENCE_RENAMESEQUENCE: { mainWin->profile.renameSequence(); break; }
			case ID_SEQUENCE_ADD_TO_SEQUENCE: { mainWin->profile.addToSequence(parent); break; }
			case ID_SEQUENCE_SAVE_SEQUENCE: { mainWin->profile.saveSequence(); break; }
			case ID_SEQUENCE_NEW_SEQUENCE: { mainWin->profile.newSequence(parent); break; }
			case ID_SEQUENCE_RESET_SEQUENCE: { mainWin->profile.loadNullSequence(); break; }
			case ID_SEQUENCE_DELETE_SEQUENCE: { mainWin->profile.deleteSequence(); break; }
			case ID_NIAWG_RELOADDEFAULTWAVEFORMS: { commonFunctions::reloadNIAWGDefaults(mainWin, scriptWin); break; }
			case ID_CONFIGURATION_NEW_CONFIGURATION: { mainWin->profile.newConfiguration(mainWin, auxWin, andorWin, scriptWin); break; }
			case ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION: { mainWin->profile.renameConfiguration(); break; }
			case ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION: { mainWin->profile.deleteConfiguration(); break; }
			case ID_CONFIGURATION_SAVE_CONFIGURATION_AS: { mainWin->profile.saveConfigurationAs(scriptWin, mainWin, auxWin, andorWin, basWin); break; }
			case ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS: { mainWin->profile.saveConfigurationOnly(scriptWin, mainWin, auxWin, andorWin, basWin); break; }
			case ID_NIAWG_SENDSOFTWARETRIGGER: { scriptWin->sendNiawgSoftwareTrig(); break; }
			case ID_NIAWG_STREAMWAVEFORM: { scriptWin->streamNiawgWaveform(); break; }
			case ID_NIAWG_GETNIAWGERROR: { errBox(scriptWin->getNiawgErr()); break; }
			case ID_PICTURES_AUTOSCALEPICTURES: { andorWin->handleAutoscaleSelection(); break; }
			case ID_BASLER_AUTOSCALE: { basWin->handleBaslerAutoscaleSelection ( ); break; }
			case ID_PICTURES_GREATER_THAN_MAX_SPECIAL: { andorWin->handleSpecialGreaterThanMaxSelection(); break; }
			case ID_PICTURES_LESS_THAN_MIN_SPECIAL: { andorWin->handleSpecialLessThanMinSelection(); break; }
			case ID_PICTURES_ALWAYSSHOWGRID: { andorWin->passAlwaysShowGrid(); break; }
			case ID_NIAWG_NIAWGISON: { scriptWin->passNiawgIsOnPress( ); break; }
			case ID_DATATYPE_PHOTONS_COLLECTED: { andorWin->setDataType( CAMERA_PHOTONS ); break; }
			case ID_DATATYPE_PHOTONS_SCATTERED: { andorWin->setDataType( ATOM_PHOTONS ); break; }
			case ID_DATATYPE_RAW_COUNTS: { andorWin->setDataType( RAW_COUNTS ); break; }
			case ID_RUNMENU_ABORTBASLER: { basWin->handleDisarmPress ( ); break; }
			case ID_MASTERCONFIG_SAVEMASTERCONFIGURATION: { mainWin->masterConfig.save(mainWin, auxWin, andorWin); break; }
			case ID_MASTERCONFIGURATION_RELOAD_MASTER_CONFIG: { mainWin->masterConfig.load(mainWin, auxWin, andorWin); break; }
			case ID_MASTER_VIEWORCHANGEINDIVIDUALDACSETTINGS: { auxWin->ViewOrChangeDACNames(); break; }
			case ID_MASTER_VIEWORCHANGETTLNAMES: { auxWin->ViewOrChangeTTLNames(); break; }
			case ID_ACCELERATOR_F2: case ID_RUNMENU_PAUSE: { mainWin->handlePause(); break; }
			case ID_HELP_HARDWARESTATUS: { mainWin->showHardwareStatus ( ); break; }
			case ID_FORCE_EXIT:	{ forceExit ( scriptWin, mainWin, andorWin, auxWin, scriptWin); break; }
			default:
				errBox("Common message passed but not handled! The feature you're trying to use"\
						" feature likely needs re-implementation / new handling.");
		}
	}

	void calibrateCameraBackground( ScriptingWindow* scriptWin, MainWindow* mainWin, AndorWindow* andorWin,
									AuxiliaryWindow* auxWin, BaslerWindow* basWin )
	{
		try 
		{
			AllExperimentInput input;
			input.masterInput = new ExperimentThreadInput ( auxWin, mainWin, andorWin, basWin, scriptWin);
			input.masterInput->runList = { true, true, false };
			auxWin->fillMasterThreadInput (input.masterInput);
			andorWin->loadCameraCalSettings( input );
			mainWin->loadCameraCalSettings( input.masterInput );
			scriptWin->loadCameraCalSettings (input.masterInput);
			mainWin->startExperimentThread( input.masterInput );
		}
		catch ( Error& err )
		{
			errBox( err.trace( ) );
		}
	}	 

	void prepareMasterThread( int msgID, ScriptingWindow* scriptWin, MainWindow* mainWin, AndorWindow* andorWin,
							  AuxiliaryWindow* auxWin, BaslerWindow* basWin, AllExperimentInput& input, bool runNiawg,
							  bool runMaster, bool runAndor, bool runBasler, bool updatePlotXVals )
	{
		seqSettings seq = mainWin->getSeqSettings( );
		if (scriptWin->niawgIsRunning())
		{
			mainWin->getComm( )->sendColorBox( System::Niawg, 'R' );
			thrower ( "NIAWG is already running! Please Restart the niawg before running an experiment.\r\n" );
		}
		if (seq.sequence.size() == 0)
		{
			mainWin->getComm()->sendColorBox( System::Niawg, 'R' );
			thrower ( "No configurations in current sequence! Please set some configurations to run in this "
					  "sequence or set the null sequence.\r\n" );
		}
		mainWin->checkProfileReady();
		scriptWin->checkScriptSaves( );
		// Set the thread structure.
		input.masterInput = new ExperimentThreadInput ( auxWin, mainWin, andorWin, basWin, scriptWin);
		input.masterInput->runList = {runMaster, runAndor, runBasler};
		input.masterInput->updatePlotterXVals = updatePlotXVals;
		input.masterInput->skipNext = andorWin->getSkipNextAtomic( );
		input.masterInput->numVariations = auxWin->getTotalVariationNumber ( );
		input.masterInput->dontActuallyGenerate = ( msgID == ID_FILE_MY_WRITE_WAVEFORMS );
		input.masterInput->debugOptions = mainWin->getDebuggingOptions();
		input.masterInput->profile = mainWin->getProfileSettings ();;
		if (runNiawg)
		{
			scriptInfo<std::string> addresses = scriptWin->getScriptAddresses();
			scriptWin->setNiawgRunningState( true );
		}
		// Start the programming thread. order is important.
		mainWin->fillMasterThreadSequence( input.masterInput );
		auxWin->fillMasterThreadInput( input.masterInput );
		mainWin->fillMasterThreadInput( input.masterInput );
		andorWin->fillMasterThreadInput( input.masterInput );
		scriptWin->fillMasterThreadInput( input.masterInput );
	}


	void startExperimentThread(MainWindow* mainWin, AllExperimentInput& input)
	{
		mainWin->addTimebar( "main" );
		mainWin->addTimebar( "error" );
		mainWin->addTimebar( "debug" );
		mainWin->startExperimentThread( input.masterInput );
	}

	void abortCamera( AndorWindow* camWin, MainWindow* mainWin )
	{
		if (!camWin->cameraIsRunning())
		{
			mainWin->getComm()->sendColorBox( System::Niawg, 'B' );
			mainWin->getComm()->sendError( "System was not running. Can't Abort.\r\n" );
			return;
		}
		std::string errorMessage;
		// abort acquisition if in progress
		camWin->abortCameraRun();
		mainWin->getComm()->sendStatus( "Aborted Camera Operation.\r\n" );
	}


	void abortNiawg( ScriptingWindow* scriptWin, MainWindow* mainWin )
	{
		Communicator* comm = mainWin->getComm();
		// set reset flag
		eAbortNiawgFlag = true;
		if (!scriptWin->niawgIsRunning())
		{
			std::string msgString = "Passively Outputting Default Waveform.";
			comm->sendColorBox( System::Niawg, 'B' );
			comm->sendError( "System was not running. Can't Abort.\r\n" );
			return;
		}
		// wait for reset to occur
		int result = 1;
		result = WaitForSingleObject( eNIAWGWaitThreadHandle, 0 );
		if (result == WAIT_TIMEOUT)
		{
			// try again. Hopefully gives the main thread to handle other messages first if this happens.
			mainWin->PostMessageA( WM_COMMAND, MAKEWPARAM( ID_FILE_ABORT_GENERATION, 0 ) );
			return;
		}
		eAbortNiawgFlag = false;
		// abort the generation on the NIAWG.
		scriptWin->setIntensityDefault();
		comm->sendStatus( "Aborted NIAWG Operation. Passively Outputting Default Waveform.\r\n" );
		comm->sendColorBox( System::Niawg, 'B' );
		scriptWin->restartNiawgDefaults();
		scriptWin->setNiawgRunningState( false );
	}


	void abortMaster( MainWindow* mainWin, AuxiliaryWindow* auxWin )
	{
		mainWin->abortMasterThread();
		auxWin->handleAbort();
	}


	void forceExit ( ScriptingWindow* scriptWindow, MainWindow* mainWin, AndorWindow* camWin, AuxiliaryWindow* auxWin, ScriptingWindow* scriptWin)
	{
		/// Exiting. Close the NIAWG normally.
		try
		{
			scriptWin->stopNiawg ( );
		}
		catch ( Error& except )
		{
			errBox ( "The NIAWG did not exit smoothly: " + except.trace ( ) );
		}
		auxWin->EndDialog ( 0 );
		camWin->EndDialog ( 0 );
		scriptWindow->EndDialog ( 0 );
		mainWin->EndDialog ( 0 );
		PostQuitMessage ( 1 );
	}


	void exitProgram( ScriptingWindow* scriptWindow, MainWindow* mainWin, AndorWindow* camWin, AuxiliaryWindow* auxWin)
	{
		if (scriptWindow->niawgIsRunning())
		{
			thrower ( "The NIAWG is Currently Running. Please stop the system before exiting so that devices devices "
					  "can stop normally." );
		}
		if (camWin->cameraIsRunning())
		{
			thrower ( "The Camera is Currently Running. Please stop the system before exiting so that devices devices "
					  "can stop normally." );
		}
		if (mainWin->masterIsRunning())
		{
			thrower ( "The Master system (ttls & aoSys) is currently running. Please stop the system before exiting so "
					  "that devices can stop normally." );
		}
		scriptWindow->checkScriptSaves( );
		mainWin->checkProfileSave();
		std::string exitQuestion = "Are you sure you want to exit?\n\nThis will stop all output of the NI arbitrary "
			"waveform generator. The Andor camera temperature control will also stop, causing the Andor camera to "
			"return to room temperature.";
		int areYouSure = promptBox(exitQuestion, MB_OKCANCEL);
		if (areYouSure == IDOK)
		{
			forceExit ( scriptWindow, mainWin, camWin, auxWin, scriptWindow);
		}
	}


	void reloadNIAWGDefaults( MainWindow* mainWin, ScriptingWindow* scriptWin )
	{
		// this hasn't actually been used or tested in a long time... aug 29th 2019
		profileSettings profile = mainWin->getProfileSettings();
		if (scriptWin->niawgIsRunning())
		{
			thrower ( "The system is currently running. You cannot reload the default waveforms while the system is "
					  "running. Please restart the system before attempting to reload default waveforms." );
		}
		int choice = promptBox("Reload the default waveforms from (presumably) updated files? Please make sure that "
								"the updated files are syntactically correct, or else the program will crash.",
								MB_OKCANCEL );
		if (choice == IDCANCEL)
		{
			return;
		}
		try
		{
			scriptWin->setNiawgDefaults();
			scriptWin->restartNiawgDefaults();
		}
		catch (Error&)
		{
			scriptWin->restartNiawgDefaults();
			throwNested( "failed to reload the niawg default waveforms!" );
		}
		mainWin->getComm()->sendStatus( "Reloaded Default Waveforms.\r\nInitialized Default Waveform.\r\n" );
	}


	void logStandard( AllExperimentInput input, DataLogger& logger, std::vector<servoInfo> servos, 
					  std::string specialName, bool needsCal )
	{
		logger.initializeDataFiles( specialName, needsCal );
		logger.logMasterInput( input.masterInput );
		logger.logServoInfo (servos);
		logger.logMiscellaneousStart();
		logger.initializeAiLogging( input.masterInput->numAiMeasurements );		
	}


	bool getPermissionToStart( AndorWindow* camWin, MainWindow* mainWin, ScriptingWindow* scriptWin,
							   AuxiliaryWindow* auxWin, bool runNiawg, bool runMaster, AllExperimentInput& input )
	{
		std::string startMsg = "Current Settings:\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n";
		startMsg = camWin->getStartMessage( );
		if ( runNiawg )
		{
			scriptInfo<std::string> scriptNames = scriptWin->getScriptNames( );
			// ordering matters here, make sure you get the correct script name.
			std::string niawgNameString( scriptNames.niawg );
			std::string intensityNameString( scriptNames.intensityAgilent );
			std::string sequenceInfo = "";
			if ( sequenceInfo != "" )
			{
				startMsg += sequenceInfo;
			}
			else
			{
				scriptInfo<bool> scriptSavedStatus = scriptWin->getScriptSavedStatuses( );
				startMsg += "NIAWG Script Name:...... " + str( niawgNameString );
				if ( scriptSavedStatus.niawg )
				{
					startMsg += " SAVED\r\n";
				}
				else
				{
					startMsg += " NOT SAVED\r\n";
				}
				startMsg += "Intensity Script Name:....... " + str( intensityNameString );
				if ( scriptSavedStatus.intensityAgilent )
				{
					startMsg += " SAVED\r\n";
				}
				else
				{
					startMsg += " NOT SAVED\r\n";
				}
			}
			startMsg += "\r\n";
		}
		startMsg += "\r\n\r\nBegin Experiment with these Settings?";
		StartDialog dlg( startMsg, IDD_BEGINNING_SETTINGS );
		bool areYouSure = dlg.DoModal( );
		if ( !areYouSure )
		{
			thrower ( "CANCEL!" );
		}
		return areYouSure;
	}
};
