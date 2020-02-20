// created by Mark O. Brown
#include "stdafx.h"
#include "ExperimentThread/ExperimentThreadManager.h"
#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "GeneralObjects/CodeTimer.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "NIAWG/NiawgWaiter.h"
#include "ParameterSystem/Expression.h"
#include "PrimaryWindows/MainWindow.h"
#include "nidaqmx2.h"
#include "Andor/CameraSettingsControl.h"
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>


ExperimentThreadManager::ExperimentThreadManager() {}

/*
 * The workhorse of actually running experiments. This thread procedure analyzes all of the GUI settings and current 
 * configuration settings to determine how to program and run the experiment.
 * @param voidInput: This is the only input to the proxcedure. It MUST be a pointer to a ExperimentThreadInput structure.
 * @return UINT: The return value is not used, i just return TRUE.
 */
unsigned int __stdcall ExperimentThreadManager::experimentThreadProcedure( void* voidInput )
{
	/// initialize various structures
	// convert the input to the correct structure.
	ExperimentThreadInput* input = (ExperimentThreadInput*)voidInput;
	// change the status of the parent object to reflect that the thread is running.
	input->thisObj->experimentIsRunning = true;
	seqInfo expSeq( input->seq.sequence.size( ) );
	UINT seqNum = 0, repetitions = 1;
	std::vector<std::vector<ddsIndvRampListInfo>> ddsRampList;
	ddsRampList.resize( input->seq.sequence.size ( ) );
	// outermost level is for each controller, 2nd level is for sequence number
	std::vector<std::vector<piezoChan<Expression>>> piezoExpressions(input->piezoCores.size(), 
												std::vector<piezoChan<Expression>>( input->seq.sequence.size ( )) );
	std::vector<std::vector<bool>> ctrlPztOptions( input->piezoCores.size ( ),
												   std::vector<bool> ( input->seq.sequence.size ( ) ));
	std::vector<AndorRunSettings> andorRunsettings ( input->seq.sequence.size ( ) );
	microwaveSettings uwSettings;
	// a couple aliases.
	auto& ttls = input->ttls;
	auto& aoSys = input->aoSys;
	auto& comm = input->comm;
	auto& dds = input->dds;
	auto quiet = input->quiet;
	const auto& runMaster = input->runList.master;
	const auto& runAndor = input->runList.andor;
	const auto& runNiawg = input->runList.niawg;
	const auto& runBasler = input->runList.basler;
	const auto& piezos = input->piezoCores;
	const UINT tbTek = 0;
	const UINT aeTek = 1;

	mainOptions mainOpts;
	baslerSettings baslerCamSettings;
	std::vector<std::vector<parameterType>> expParams;
	ScanRangeInfo varRangeInfo = ParameterSystem::getRangeInfoFromFile (input->seq.sequence[0].configFilePath ());
	std::vector<deviceOutputInfo> agilentRunInfo(input->agilents.size());
	std::vector<tektronixInfo> tekInfo (2);
	UINT variations;
	try
	{
		for ( auto& configInfo : input->seq.sequence )
		{
			auto& seq = expSeq.sequence[seqNum];
			expParams.push_back (
				ParameterSystem::combineParamsForExpThread ( 
					ParameterSystem::getConfigParamsFromFile (configInfo.configFilePath ()),
					input->globalParameters));
			std::ifstream configFile (configInfo.configFilePath ());
			if ( runMaster )
			{
				seq.masterScript = ProfileSystem::getMasterAddressFromConfig( configInfo );
				input->thisObj->loadMasterScript( seq.masterScript, seq.masterStream );
				ddsRampList[ seqNum ] = ProfileSystem::stdGetFromConfig ( configFile, dds.configDelim,
																			   DdsCore::getRampListFromConfig );
				for ( auto piezoInc : range(input->piezoCores.size()))
				{
					auto res = ProfileSystem::stdGetFromConfig (
						configFile, input->piezoCores[ piezoInc ].get().configDelim, 
						PiezoCore::getPiezoSettingsFromConfig );
					piezoExpressions[ piezoInc ][ seqNum ] 
						= { Expression ( res.first.x ), Expression ( res.first.y ), Expression ( res.first.z ) };
					ctrlPztOptions[ piezoInc ][ seqNum ] = res.second;
				}
				mainOpts = ProfileSystem::stdGetFromConfig ( configFile, "MAIN_OPTIONS", 
																  MainOptionsControl::getMainOptionsFromConfig );
				repetitions = ProfileSystem::stdGetFromConfig ( configFile, "REPETITIONS",
																Repetitions::getRepsFromConfig );
				uwSettings = ProfileSystem::stdGetFromConfig ( configFile, MicrowaveSystem::delim,
															   MicrowaveSystem::getMicrowaveSettingsFromConfig);
				tekInfo[tbTek] = ProfileSystem::stdGetFromConfig (configFile, input->topBottomTek.configDelim,
																  TektronixAfgControl::getTekInfo);
				tekInfo[aeTek] = ProfileSystem::stdGetFromConfig (configFile, input->eoAxialTek.configDelim,
																  TektronixAfgControl::getTekInfo);
				ParameterSystem::generateKey (expParams, mainOpts.randomizeVariations, varRangeInfo);
				variations = determineVariationNumber (expParams[ seqNum ] );
				for (auto agInc : range(input->agilents.size()))
				{
					agilentRunInfo[agInc] = ProfileSystem::stdGetFromConfig(configFile, 
						input->agilents[agInc].get().configDelim, Agilent::getOutputSettingsFromConfigFile);
				}
			}
			if (runAndor) 
			{
				andorRunsettings[seqNum] = ProfileSystem::stdGetFromConfig (configFile, "CAMERA_SETTINGS",
					AndorCameraSettingsControl::getRunSettingsFromConfig);
				andorRunsettings[seqNum].imageSettings = ProfileSystem::stdGetFromConfig (configFile,
					"CAMERA_IMAGE_DIMENSIONS", AndorCameraSettingsControl::getImageDimSettingsFromConfig);
				andorRunsettings[seqNum].repetitionsPerVariation = repetitions;
				andorRunsettings[seqNum].totalVariations = variations;
			}
			if (runBasler)
			{
				baslerCamSettings = ProfileSystem::stdGetFromConfig (configFile, "BASLER_CAMERA_SETTINGS",
					&BaslerSettingsControl::getSettingsFromConfig, Version ("4.0"));
			}
			if ( runNiawg )
			{
				seq.niawgScript = ProfileSystem::getNiawgScriptAddrFromConfig( configInfo );
				input->thisObj->loadNiawgScript ( seq.niawgScript, seq.niawgStream );
				if ( input->debugOptions.outputNiawgHumanScript )
				{
					std::string debugStr = "Human Script: " + seq.niawgStream.str ( ) + "\n\n";
					debugStr.erase ( std::remove ( debugStr.begin ( ), debugStr.end ( ), '\r' ), debugStr.end ( ) );
					boost::replace_all ( debugStr, "\n", "\r\n" );
					input->comm.sendDebug ( debugStr );
				}
			}

			seqNum++;
		}
	}
	catch ( Error& err)
	{
		errBox( "Failed to load experiment sequence / configuration files!  \n" + err.trace() );
		input->thisObj->experimentIsRunning = false;
		delete voidInput;
		return -1;
	}
	// warnings will be passed by reference to a series of function calls which can append warnings to the string.
	// at a certain point the string will get outputted to the error console. Remember, errors themselves are handled 
	// by thrower () calls.
	std::string warnings, abortString = "\r\nABORTED!\r\n";
	std::chrono::time_point<chronoClock> startTime( chronoClock::now( ) );
	std::vector<long> variedMixedSize;
	NiawgOutput output;
	std::vector<ViChar> niawgMachineScript;
	std::vector<std::string> workingNiawgScripts( input->seq.sequence.size( ) );
	output.isDefault = false;
	// initialize to 2 because of default waveforms. This can probably be changed to 1, since only one default waveform
	// now, but might cause slight breakages...
	output.waves.resize( 2 );
	std::vector<std::pair<UINT, UINT>> ttlShadeLocs;
	std::vector<UINT> dacShadeLocs;
	bool foundRearrangement = false;
	/// ////////////////////////////
	/// start analysis & experiment
	try
	{
		if ( input->expType != ExperimentType::LoadMot )
		{
			input->logger.logMasterRuntime ( repetitions, expParams);
			input->logger.logBaslerSettings ( baslerCamSettings, runBasler );
			input->logger.logAndorSettings ( andorRunsettings[0], runAndor );
			input->logger.logAgilentSettings (input->agilents, agilentRunInfo);
			input->logger.logTektronicsSettings (tekInfo[0], input->topBottomTek.configDelim);
			input->logger.logTektronicsSettings (tekInfo[1], input->eoAxialTek.configDelim);
		}
		for ( auto piezoInc : range ( piezos.size ( ) ) )
		{
			piezos[ piezoInc ].get().updateExprVals ( piezoExpressions[ piezoInc ] );
		}
		dds.updateRampLists ( ddsRampList );
		if ( input->updatePlotterXVals ) updatePlotX_vals ( input, expParams );
		if ( runMaster )
		{
			aoSys.resetDacEvents ( );
			ttls.resetTtlEvents ( );
			aoSys.initializeDataObjects ( input->seq.sequence.size ( ), 0 );
			ttls.initializeDataObjects ( input->seq.sequence.size ( ), 0 );
			input->thisObj->loadSkipTimes.clear ( );
			input->thisObj->loadSkipTimes.resize ( input->seq.sequence.size ( ) );
			input->thisObj->loadSkipTime.resize ( input->seq.sequence.size ( ) );
		}
		if ( runNiawg )
		{
			input->niawg.initForExperiment ( );
		}
		UINT variations;
		for ( auto seqNum : range( input->seq.sequence.size() ) )
		{
			auto& seqVariables = expParams[seqNum];
			auto& seq = expSeq.sequence[ seqNum ];
			if ( seqNum == 0 )
			{
				variations = determineVariationNumber( seqVariables );
			}
			else
			{
				if ( variations != determineVariationNumber( seqVariables ) )
				{
					// the sequences are always interwoven sequence right now, so variations need to match or else
					// this doesn't make sense.
					thrower ( "Variation number changes between sequences! the number of variations must match"
							  " between sequences.  (A low level bug, this shouldn't happen)" );
				}
			}
			/// Prep agilents
			expUpdate( "Loading Agilent Info...", comm, quiet );
			for ( auto agInc: range(input->agilents.size()) )
			{
				auto& agilent = input->agilents[agInc].get();
				for ( auto channelInc : range ( 2 ) )
				{
					if (agilentRunInfo[agInc].channel[channelInc].scriptedArb.fileAddress != "")
					{
						agilent.analyzeAgilentScript (agilentRunInfo[agInc].channel[channelInc].scriptedArb,
							seqVariables);
					}
				}
			}
			expUpdate( "Analyzing Master Script...", comm, quiet );
			if ( runMaster ) 
			{
				comm.sendColorBox ( System::Master, 'Y' );
				input->thisObj->analyzeMasterScript( ttls, aoSys, ttlShadeLocs, dacShadeLocs,
													 seqVariables, seq.masterStream, seqNum,
													 mainOpts.atomThresholdForSkip != UINT_MAX, warnings, 
													 input->thisObj->operationTime, input->thisObj->loadSkipTime );
			}
			if ( input->thisObj->isAborting ) thrower ( abortString );
			if ( runNiawg )	
			{
				comm.sendColorBox ( System::Niawg, 'Y' );
				input->niawg.analyzeNiawgScript ( seq.niawgStream, output, input->profile, input->debugOptions, 
												   warnings, input->rerngGuiForm, seqVariables );
				workingNiawgScripts[ seqNum ] = output.niawgLanguageScript;
			}
			if ( input->thisObj->isAborting ) thrower ( abortString );
			if ( runMaster )
			{
				input->thisObj->loadSkipTimes[ seqNum ].resize ( variations );
			}
		}
		if ( runNiawg )
		{
			input->niawg.finalizeScript ( repetitions, "experimentScript", workingNiawgScripts, niawgMachineScript,
										   !input->niawg.outputVaries ( output ) );
			if ( input->debugOptions.outputNiawgMachineScript )
			{
				std::string debugStr = "NIAWG Machine Script:\n"
					+ std::string ( niawgMachineScript.begin ( ), niawgMachineScript.end ( ) ) + "\n\n";
				debugStr.erase ( std::remove ( debugStr.begin ( ), debugStr.end ( ), '\r' ), debugStr.end ( ) );
				boost::replace_all ( debugStr, "\n", "\r\n" );
				comm.sendDebug ( debugStr );
			}
			for ( auto& seqVariables : expParams)
			{
				input->niawg.writeStaticNiawg ( output, input->debugOptions, seqVariables );
			}
			expUpdate("Constant NIAWG Waveform Preparation Completed...\r\n", comm, input->quiet );
		}
		if ( input->thisObj->isAborting ) { thrower ( abortString ); }
		/// The Key Interpretation step.
		// at this point, all scripts have been analyzed, and each system takes the key and generates all of the data
		// it needs for each variation of the experiment. All these calculations happen at this step.
		expUpdate( "Programming All Variation Data...\r\n", comm, quiet );
		if ( runMaster )
		{
			ttls.shadeTTLs ( ttlShadeLocs );
			aoSys.shadeDacs ( dacShadeLocs );
			ttls.interpretKey(expParams);
			ttls.restructureCommands ( );
			aoSys.interpretKey(expParams, warnings );
		}
		for ( auto piezoInc : range(piezos.size()) )
		{
			if ( ctrlPztOptions[ piezoInc ][ 0 ] )
			{
				piezos[piezoInc].get().evaluateVariations (expParams, variations );
			}
		}
		dds.evaluateDdsInfo (expParams);
		dds.generateFullExpInfo ( variations );
		input->topBottomTek.interpretKey (expParams, tekInfo[0]);
		input->eoAxialTek.interpretKey (expParams, tekInfo[1]);
		for (auto agInc : range (input->agilents.size ()))
		{
			input->agilents[agInc].get().convertInputToFinalSettings (variations, 0, agilentRunInfo[agInc], expParams[0]);
			input->agilents[agInc].get().convertInputToFinalSettings (variations, 1, agilentRunInfo[agInc], expParams[0]);
		}
		input->rsg.interpretKey (expParams, uwSettings);
		/// organize commands, prepping final forms of the data for each repetition.
		// This must be done after the "interpret key" step; before that commands don't have times attached to them.
		for ( auto seqInc : range( input->seq.sequence.size( ) ) )
		{
			auto& seqVariables = expParams[seqInc];
			for (auto variationInc : range(variations))
			{
				if ( input->thisObj->isAborting ) { thrower ( abortString ); }
				if ( runMaster )
				{
					double& currLoadSkipTime = input->thisObj->loadSkipTimes[seqInc][variationInc];
					currLoadSkipTime = ExperimentThreadManager::convertToTime( input->thisObj->loadSkipTime[seqInc], 
																		   seqVariables, variationInc );
				    // organize & format the ttl and dac commands
					aoSys.organizeDacCommands( variationInc, seqInc );
					aoSys.setDacTriggerEvents( ttls, variationInc, seqInc );
					aoSys.findLoadSkipSnapshots( currLoadSkipTime, seqVariables, variationInc, seqInc );
					aoSys.makeFinalDataFormat( variationInc, seqInc ); 
					ttls.organizeTtlCommands ( variationInc, seqInc );
					ttls.findLoadSkipSnapshots( currLoadSkipTime, seqVariables, variationInc, seqInc );
					ttls.convertToFtdiSnaps(variationInc, seqInc);
					ttls.convertToFinalFtdiFormat( variationInc, seqInc );
					// run a couple checks.
					ttls.checkNotTooManyTimes( variationInc, seqInc );
					ttls.checkFinalFormatTimes( variationInc, seqInc );
					aoSys.checkTimingsWork( variationInc, seqInc );
				}
			}
		}
		/// output some timing information 
		if (runMaster)
		{
			expUpdate( "Programmed time per repetition: " + str( ttls.getTotalTime( 0, 0 ) ) + "\r\n", comm, quiet );
			ULONGLONG totalTime = 0;
			for ( auto seqInc : range( input->seq.sequence.size()) )
			{
				for ( auto variationNumber : range(variations) )
				{
					totalTime += ULONGLONG( ttls.getTotalTime( variationNumber, seqInc ) * repetitions );
				}
			}
			expUpdate( "Programmed Total Experiment time: " + str( totalTime ) + "\r\n", comm, quiet );
			expUpdate( "Number of TTL Events in experiment: " + str( ttls.getNumberEvents( 0, 0 ) ) + "\r\n", comm, quiet );
			expUpdate( "Number of DAC Events in experiment: " + str( aoSys.getNumberEvents( 0, 0 ) ) + "\r\n", comm, quiet );
		}
		input->globalControl.setUsages(expParams);
		for ( auto& seqvars : expParams)
		{
			for ( auto& var : seqvars )
			{
				if ( !var.constant && !var.active )
				{
					warnings += "WARNING: Variable " + var.name + " is varied, but not being used?!?\r\n";
				}
			}
		}
		ExperimentThreadManager::checkTriggerNumbers ( input, warnings, variations, uwSettings, 
													   expParams, agilentRunInfo );
		/// finish up
		if ( runMaster )
		{
			handleDebugPlots ( input->debugOptions, comm, ttls, aoSys, input->ttlData, input->dacData );
		}
		if ( warnings != "" )
		{
			comm.sendError ( warnings );
			auto response = promptBox ( "WARNING: The following warnings were reported while preparing the experiment:\r\n"
								   + warnings + "\r\nIs this acceptable? (press no to abort)", MB_YESNO );
			if ( response == IDNO )
			{
				thrower ( abortString );
			}
		}
		warnings = ""; // then reset so as to not mindlessly repeat warnings from the experiment loop.
		/// /////////////////////////////
		/// Begin experiment loop
		if (runMaster)
		{
			comm.sendColorBox( System::Master, 'G' );
		}
		expUpdate ("Starting Basler Camera...", comm, quiet);
		if (runBasler)
		{
			comm.sendPrepareBasler (baslerCamSettings);
			input->basCamera.setBaslserAcqParameters (baslerCamSettings);
			input->basCamera.armCamera (baslerCamSettings.frameRate);
		}
		if (runAndor)
		{
			input->andorCamera.setSettings (andorRunsettings[0]);
			comm.sendPrepareAndor (andorRunsettings[0]);
		}
		// shouldn't there be a sequence loop here?
		// TODO: Handle randomizing repetitions. The thread will need to split into separate if/else statements here.
		// shouldn't there be a sequence loop here?
		for (const UINT& variationInc : range( variations ))
		{
			expUpdate( "Variation #" + str( variationInc + 1 ) + "/" + str(variations) + ": ", comm, quiet );
			if ( input->aiSys.wantsQueryBetweenVariations( ) )
			{
				// I'm writing data which is handled by the main gui thread, so I just let the main gui thread do the 
				// whole measurement here. 
				expUpdate( "Querying Voltages...\r\n", comm, quiet );
				comm.sendLogVoltsMessage ( variationInc );
			}
			if ( input->debugOptions.sleepTime != 0 )
			{
				expUpdate ( "PAUSED!\r\n", comm, quiet );
				Sleep ( input->debugOptions.sleepTime );
				expUpdate ( "UNPAUSED!\r\n", comm, quiet );
			}
			for ( auto seqInc : range(input->seq.sequence.size( ) ) )
			{
				for (auto tempVariable : expParams[seqInc])
				{
					if (tempVariable.valuesVary)
					{
						if (tempVariable.keyValues.size() == 0)
						{
							thrower ( "Variable " + tempVariable.name + " varies, but has no values assigned to "
									  "it! (This shouldn't happen, it's a low-level bug...)" );
						}
						expUpdate( tempVariable.name + ": " + str( tempVariable.keyValues[variationInc], 12) + "\r\n", 
								   comm, quiet );
					}
				}
			}
			expUpdate ( "Starting Andor Camera...", comm, quiet );
			if ( runAndor )
			{
				andorRunsettings[ 0 ].repetitionsPerVariation = repetitions;
				andorRunsettings[ 0 ].totalVariations = variations;
				int stat = 0;
				while ( true )
				{
					stat = input->andorCamera.queryStatus ( );
					if ( stat == DRV_ACQUIRING )
					{
						Sleep ( 1000 );
						expUpdate ( "Waiting for Andor camera to finish acquisition...", comm, quiet );
						if (input->thisObj->isAborting) { thrower (abortString); }
					}
					else
					{
						break;
					}
				} 
				double kinTime;
				input->andorCamera.armCamera ( kinTime );
			}
			expUpdate( "Programming Devices... ", comm, quiet );
			input->rsg.programRsg (variationInc, uwSettings);
			// program devices
			for ( auto agInc : range(input->agilents.size()) )
			{
				input->agilents[agInc].get().setAgilent ( variationInc, expParams[ 0 ], agilentRunInfo[agInc] );
			}
			for ( auto piezoInc : range(piezos.size()) )
			{
				if ( ctrlPztOptions[ piezoInc ][ 0 ] )
				{
					piezos[piezoInc].get().exprProgramPiezo ( 0, variationInc );
				}
			}
			dds.writeExperiment ( 0, variationInc );
			if (runNiawg)
			{
				input->niawg.programNiawg( input, output, warnings, variationInc, variations, variedMixedSize,
										   niawgMachineScript, input->rerngGuiForm, input->rerngGui, expParams );
				if (input->rerngGui.active)
				{
					input->niawg.turnOffRerng ();
					input->conditionVariableForRerng->notify_all ();
					input->niawg.waitForRerng (false);
					input->niawg.handleStartingRerng (input, output);
				}
			}
			input->topBottomTek.programMachine (variationInc, tekInfo[0]);
			input->eoAxialTek.programMachine (variationInc, tekInfo[1]);
			comm.sendError( warnings );
			comm.sendRepProgress( 0 );
			expUpdate( "Running Experiment.\r\n", comm, quiet );

			bool skipOption = input->skipNext == NULL ? false : input->skipNext->load();
			if (runMaster)
			{
				ttls.ftdi_write (0, variationInc, skipOption);
			}
			for (auto repInc : range(repetitions))
			{
				
				/*for (auto seqInc : range(input->seq.sequence.size()))
				{*///this was the original structure for handleing diffferent sequences across repetitions
				if (input->seq.sequence.size() > 1) 
				{
					thrower("error the number of sequences should be zero");
				}
				else 
				{
					UINT seqInc = 0;
					if (input->thisObj->isAborting) { thrower(abortString); }
					else if (input->thisObj->isPaused)
					{
						expUpdate("Paused\r\n!", comm, quiet);
						while (input->thisObj->isPaused)
						{
							// this could be changed to be a bit smarter using a std::condition_variable
							Sleep(100);
							if (input->thisObj->isAborting) { thrower(abortString); }
						}
						expUpdate("Un-Paused!\r\n", comm, quiet);
					}
					comm.sendRepProgress(repInc + 1);
					if (runMaster)
					{
						aoSys.stopDacs();
						// it's important to grab the skipoption from input->skipNext only once because in principle
						// if the cruncher thread was running behind, it could change between writing and configuring the 
						// aoSys and configuring the TTLs, resulting in some funny behavior;
						aoSys.configureClocks(variationInc, seqInc, skipOption);
						aoSys.writeDacs(variationInc, seqInc, skipOption);
						aoSys.startDacs();
						ttls.ftdi_trigger();
						ttls.FtdiWaitTillFinished(variationInc, seqInc);
					}
				}
			}
		}
		/// conclude.
		expUpdate( "\r\nExperiment Finished Normally.\r\n", comm, quiet );
		comm.sendColorBox( System::Master, 'B' );
		if (runMaster)
		{
			// stop is necessary else the dac system will still be running and won't allow updates through normal means.
			aoSys.stopDacs();
			aoSys.unshadeDacs();
			try
			{
				// make sure the display accurately displays the state that the experiment finished at.
				aoSys.setDacStatusNoForceOut( aoSys.getFinalSnapshot( ) );
				ttls.unshadeTtls( );
				ttls.setTtlStatusNoForceOut( ttls.getFinalSnapshot( ) );
			}
			catch ( Error& ) { /* this gets thrown if no dac events. just continue.*/ }
		}
		if (runNiawg)
		{
			input->niawg.cleanupNiawg( input->profile, runMaster, output, comm, mainOpts.dontActuallyGenerate );
		}
		input->thisObj->experimentIsRunning = false;
		switch ( input->expType )
		{
			case ExperimentType::CameraCal:
				comm.sendCameraCalFin ( );
				break;
			case ExperimentType::LoadMot:
			case ExperimentType::MachineOptimization:
			case ExperimentType::AutoCal:
				comm.sendFinish ( input->expType );
				break;
			default:
				comm.sendFinish ( ExperimentType::Normal );
		}
	}
	catch (Error& exception)
	{
		if (runNiawg)
		{
			for (auto& wave : output.waves)
			{
				wave.core.waveVals.clear();
				wave.core.waveVals.shrink_to_fit();
			}
		}
		input->thisObj->experimentIsRunning = false;
		if (runMaster)
		{
			input->ttls.unshadeTtls();
			input->aoSys.unshadeDacs();
		}	
		if ( input->thisObj->isAborting )
		{
			expUpdate( abortString, comm, quiet );
			comm.sendColorBox( System::Master, 'B' );
		}
		else
		{
			comm.sendColorBox( System::Master, 'R' );
			comm.sendStatus( "Bad Exit!\r\n" );
			auto txt = "Exited main experiment thread abnormally." + exception.trace ( );
			comm.sendFatalError( txt );
		}	
		try
		{
			input->andorCamera.abortAcquisition ();
		}
		catch (Error & err) 
		{ /*Probably just idle.*/ }
		{
			std::lock_guard<std::mutex> locker ( input->thisObj->abortLock );
			input->thisObj->isAborting = false;
		}
	}
	// finish up.
	auto exp_t = std::chrono::duration_cast<std::chrono::seconds>( ( chronoClock::now ( ) - startTime ) ).count ( );
	expUpdate( "Experiment took " + str( int(exp_t) / 3600 )  + " hours, " + str(int(exp_t) % 3600 / 60) + " minutes, "
			   + str( int ( exp_t ) % 60) +  " seconds.\r\n", comm, quiet );
	input->thisObj->experimentIsRunning = false;
	
	delete input;
	return false;
}


void ExperimentThreadManager::analyzeMasterScript ( DoSystem& ttls, AoSystem& aoSys,
												std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades,
												std::vector<parameterType>& vars, ScriptStream& currentMasterScript, 
												UINT seqNum, bool expectsLoadSkip, std::string& warnings, 
												timeType& operationTime, std::vector<timeType>& loadSkipTime )
{
	std::string currentMasterScriptText = currentMasterScript.str ( );
	loadSkipTime[ seqNum ].first.clear ( );
	loadSkipTime[ seqNum ].second = 0;
	// starts at 0.1 if not initialized by the user.
	operationTime.second = 0.1;
	operationTime.first.clear ( );
	if ( currentMasterScript.str ( ) == "" )
	{
		thrower ( "Master script is empty! (A low level bug, this shouldn't happen)" );
	}
	std::string word;
	currentMasterScript >> word;
	std::vector<UINT> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	// the analysis loop.
	bool loadSkipFound = false;
	std::string scope = PARENT_PARAMETER_SCOPE;                                                                                              
	while ( !( currentMasterScript.peek ( ) == EOF ) || word != "__end__" )
	{
 		if ( handleTimeCommands ( word, currentMasterScript, vars, scope, operationTime ) )
		{
			// got handled, so break out of the if-else by entering this scope.
		}
		else if ( handleVariableDeclaration ( word, currentMasterScript, vars, scope, warnings ) )
		{}
		else if ( handleDoCommands ( word, currentMasterScript, vars, ttls, ttlShades, seqNum, scope, operationTime) )
		{}
		else if ( handleAoCommands ( word, currentMasterScript, vars, aoSys, dacShades, ttls, seqNum, scope, operationTime) )
		{}
		/// callcppcode function
		else if ( word == "callcppcode" )
		{
			// and that's it... 
			callCppCodeFunction ( );
		}
		/// deal with ttl commands
		else if ( word == "loadskipentrypoint!" )
		{
			loadSkipFound = true;
			loadSkipTime[ seqNum ] = operationTime;
		}
		/// Deal with RSG calls
		else if ( word == "rsg:" )
		{
			thrower ("\"rsg:\" command is deprecated! Please use the microwave system listview instead.");
		}
		/// deal with function calls.
		else if ( handleFunctionCall ( word, currentMasterScript, vars, ttls, aoSys, ttlShades, dacShades, seqNum,
									   warnings, PARENT_PARAMETER_SCOPE, operationTime ) )
		{ }
		else if ( word == "repeat:" )
		{
			Expression repeatStr;
			currentMasterScript >> repeatStr;
			try
			{
				totalRepeatNum.push_back ( repeatStr.evaluate ( ) );
			}
			catch ( Error& )
			{
				throwNested ( "the repeat number failed to convert to an integer! Note that the repeat number can not"
						  " currently be a variable." );
			}
			repeatPos.push_back ( currentMasterScript.tellg ( ) );
			currentRepeatNum.push_back ( 1 );
		}
		else if ( word == "end" )
		{
			// handle end of repeat
			if ( currentRepeatNum.size ( ) == 0 )
			{
				thrower ( "ERROR! Tried to end repeat structure in master script, but you weren't repeating!" );
			}
			if ( currentRepeatNum.back ( ) < totalRepeatNum.back ( ) )
			{
				currentMasterScript.seekg ( repeatPos.back ( ) );
				currentRepeatNum.back ( )++;
			}
			else
			{
				currentRepeatNum.pop_back ( );
				repeatPos.pop_back ( );
				totalRepeatNum.pop_back ( );
			}
		}
		else
		{
			word = ( word == "" ) ? "[EMPTY-STRING]" : word;
			thrower ( "unrecognized master script command: \"" + word + "\"" );
		}
		word = "";
		currentMasterScript >> word;
	}
	if ( expectsLoadSkip && !loadSkipFound )
	{
		thrower ( "Expected load skip in script, but the load skip command was not found during script analysis!" );
	}
}


void ExperimentThreadManager::analyzeFunction ( std::string function, std::vector<std::string> args, DoSystem& ttls,
											AoSystem& aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades,
											std::vector<UINT>& dacShades, 
											std::vector<parameterType>& params, UINT seqNum, std::string& warnings, 
											timeType& operationTime, std::string callingScope )
{	
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	FILE *file;
	fopen_s ( &file, cstr ( FUNCTIONS_FOLDER_LOCATION + function + "." + FUNCTION_EXTENSION ), "r" );
	if ( !file )
	{
		thrower ( "Function " + function + " does not exist! The master script tried to open this function, it"
				  " tried and failed to open the location " + FUNCTIONS_FOLDER_LOCATION + function + "."
				  + FUNCTION_EXTENSION + "." );
	}
	else
	{
		fclose ( file );
	}
	functionFile.open ( FUNCTIONS_FOLDER_LOCATION + function + "." + FUNCTION_EXTENSION, std::ios::in );
	// check opened correctly
	if ( !functionFile.is_open ( ) )
	{
		thrower ( "Function file " + function + "File passed test making sure the file exists, but it still "
				  "failed to open! (A low level bug, this shouldn't happen.)" );
	}
	// append __END__ to the end of the file for analysis purposes.
	std::stringstream buf;
	ScriptStream functionStream;
	buf << functionFile.rdbuf ( );
	functionStream << buf.str ( );
	functionStream << "\r\n\r\n__END__";
	functionFile.close ( );
	if ( functionStream.str ( ) == "" )
	{
		thrower ( "Function File for " + function + " function was empty! (A low level bug, this shouldn't happen" );
	}
	std::string word;
	// the following are used for repeat: functionality
	std::vector<ULONG> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	std::string scope = function;
	/// get the function arguments.
	std::string defLine, name;
	defLine = functionStream.getline ( ':' );
	std::vector<std::string> functionArgs;
	analyzeFunctionDefinition ( defLine, name, functionArgs );
	if ( functionArgs.size ( ) != args.size ( ) )
	{
		std::string functionArgsString;
		for ( auto elem : args )
		{
			functionArgsString += elem + ",";
		}
		thrower ( "incorrect number of arguments in the call for function " + function + ". Number in call was: "
				  + str ( args.size ( ) ) + ", number expected was " + str ( functionArgs.size ( ) ) 
				  + ". Function arguments were:" + functionArgsString + "." );
	}
	std::vector<std::pair<std::string, std::string>> replacements;
	for (auto replacementInc : range(args.size()))
	{
		replacements.push_back ( { functionArgs[ replacementInc ], args[ replacementInc ] } );
	}
	functionStream.loadReplacements ( replacements, params, function, callingScope, function );
	std::string currentFunctionText = functionStream.str ( );
	///
	functionStream >> word;
	while ( !( functionStream.peek ( ) == EOF ) || word != "__end__" )
	{
		if (handleTimeCommands (word, functionStream, params, scope, operationTime)){ /* got handled*/ }
		else if ( handleVariableDeclaration ( word, functionStream, params, scope, warnings ) ){}
		else if ( handleDoCommands ( word, functionStream, params, ttls, ttlShades, seqNum, scope, operationTime) ){}
		else if ( handleAoCommands ( word, functionStream, params, aoSys, dacShades, ttls, seqNum, scope, operationTime) ){}
		else if ( word == "callcppcode" )
		{
			// and that's it... 
			callCppCodeFunction ( );
		}
		/// Handle RSG calls.
		else if ( word == "rsg:" )
		{
			thrower ("\"rsg:\" command is deprecated! Please use the microwave system listview instead.");
		}
		/// deal with function calls.
		else if ( handleFunctionCall ( word, functionStream, params, ttls, aoSys, ttlShades, dacShades, seqNum,
									   warnings, function, operationTime ) ) {}
		else if ( word == "repeat:" )
		{
			std::string repeatStr;
			functionStream >> repeatStr;
			try
			{
				totalRepeatNum.push_back ( boost::lexical_cast<int> ( repeatStr ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "the repeat number for a repeat structure inside the master script failed to convert "
						  "to an integer! Note that the repeat number can not currently be a variable." );
			}
			repeatPos.push_back ( functionStream.tellg ( ) );
			currentRepeatNum.push_back ( 1 );
		}
		else if ( word == "end" )
		{
			if ( currentRepeatNum.size ( ) == 0 )
			{
				thrower ( "mismatched \"end\" command for repeat structure in master script! there were more "
						  "\"end\" commands than \"repeat\" commands." );
			}
			if ( currentRepeatNum.back ( ) < totalRepeatNum.back ( ) )
			{
				functionStream.seekg ( repeatPos.back ( ) );
				currentRepeatNum.back ( )++;
			}
			else
			{
				// remove the entries corresponding to this repeat loop.
				currentRepeatNum.pop_back ( );
				repeatPos.pop_back ( );
				totalRepeatNum.pop_back ( );
				// and continue (no seekg)
			}
		}
		else
		{
			thrower ( "unrecognized master script command inside function analysis: " + word );
		}
		functionStream >> word;
	}
}


bool ExperimentThreadManager::getAbortStatus ( )
{
	return isAborting;
}

double ExperimentThreadManager::convertToTime( timeType time, std::vector<parameterType> variables, UINT variation )
{
	double variableTime = 0;
	// add together current values for all variable times.
	if ( time.first.size( ) != 0 )
	{
		for ( auto varTime : time.first )
		{
			variableTime += varTime.evaluate( variables, variation );
		}
	}
	return variableTime + time.second;
}


void ExperimentThreadManager::handleDebugPlots( debugInfo debugOptions, Communicator& comm, DoSystem& ttls, AoSystem& aoSys,
									  std::vector<std::vector<pPlotDataVec>> ttlData, 
									  std::vector<std::vector<pPlotDataVec>> dacData )
{
	// handle on-screen plots.
	ttls.fillPlotData( 0, ttlData );
	aoSys.fillPlotData( 0, dacData, ttls.getFinalTimes() );
	if ( debugOptions.showTtls )
	{
		comm.sendDebug( ttls.getTtlSequenceMessage( 0, 0 ) );
	}
	if ( debugOptions.showDacs )
	{
		comm.sendDebug( aoSys.getDacSequenceMessage( 0, 0 ) );
	}
	// no quiet on warnings or debug messages.
	comm.sendDebug( debugOptions.message );
}


bool ExperimentThreadManager::runningStatus()
{
	return experimentIsRunning;
}


/***
 * this function is very similar to startExperimentThread but instead of getting anything from the current profile, it
 * knows exactly where to look for the MOT profile. This is currently hard-coded.
 */
void ExperimentThreadManager::loadMotSettings(ExperimentThreadInput* input)
{	
	if ( experimentIsRunning )
	{
		delete input;
		thrower ( "Experiment is Running! Please abort the current run before setting the MOT settings." );
	}
	input->thisObj = this;
	//ParameterSystem::generateKey( input->parameters, false, input->variableRangeInfo );
	runningThread = (HANDLE)_beginthreadex( NULL, NULL, &ExperimentThreadManager::experimentThreadProcedure, input, NULL, NULL );
}


HANDLE ExperimentThreadManager::startExperimentThread(ExperimentThreadInput* input)
{
	if ( !input )
	{
		thrower ( "Input to start experiment thread was null?!?!? (a Low level bug, this shouldn't happen)." );
	}
	if ( experimentIsRunning )
	{
		delete input;
		thrower ( "Experiment is already Running! You can only run one experiment at a time! Please abort before "
				 "running again." );
	}
	input->thisObj = this;
	runningThread = (HANDLE)_beginthreadex( NULL, NULL, &ExperimentThreadManager::experimentThreadProcedure, input, NULL, NULL );
	SetThreadPriority( runningThread, THREAD_PRIORITY_HIGHEST );
	return runningThread;
}


bool ExperimentThreadManager::getIsPaused()
{
	return isPaused;
}


void ExperimentThreadManager::pause()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't pause the experiment if the experiment isn't running!" );
	}
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = true;
}


void ExperimentThreadManager::unPause()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't unpause the experiment if the experiment isn't running!" );
	}
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = false;
}


void ExperimentThreadManager::abort()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't abort the experiment if the experiment isn't running!" );
	}
	std::lock_guard<std::mutex> locker( abortLock );
	isAborting = true;
}

void ExperimentThreadManager::loadAgilentScript ( std::string scriptAddress, ScriptStream& agilentScript )
{
	std::ifstream scriptFile ( scriptAddress );
	if ( !scriptFile.is_open ( ) )
	{
		thrower ( "Scripted Agilent File \"" + scriptAddress + "\" failed to open!" );
	}
	agilentScript << scriptFile.rdbuf ( );
	agilentScript.seekg ( 0 );
	scriptFile.close ( );
}


void ExperimentThreadManager::loadNiawgScript ( std::string scriptAddress, ScriptStream& niawgScript )
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s ( &file, cstr ( scriptAddress ), "r" );
	if ( !file )
	{
		thrower ( "The Niawg Script File " + scriptAddress + " does not exist! The Master-Manager tried to "
				  "open this file before starting the script analysis." );
	}
	else
	{
		fclose ( file );
	}
	scriptFile.open ( cstr ( scriptAddress ) );
	// check opened correctly
	if ( !scriptFile.is_open ( ) )
	{
		thrower ( "File passed test making sure the file exists, but it still failed to open?!?! "
				  "(A low level-bug, this shouldn't happen.)" );
	}
	// dump the file into the stringstream.
	niawgScript << scriptFile.rdbuf ( );
	scriptFile.close ( );
}


void ExperimentThreadManager::loadMasterScript(std::string scriptAddress, ScriptStream& currentMasterScript )
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, cstr(scriptAddress), "r" );
	if ( !file )
	{
		thrower ("The Master Script File " + scriptAddress + " does not exist! The Master-Manager tried to "
				 "open this file before starting the script analysis.");
	}
	else
	{
		fclose( file );
	}
	scriptFile.open(cstr(scriptAddress));
	// check opened correctly
	if (!scriptFile.is_open())
	{
		thrower ("File passed test making sure the file exists, but it still failed to open?!?! "
				 "(A low level-bug, this shouldn't happen.)");
	}
	// dump the file into the stringstream.
	std::stringstream buf( std::ios_base::app | std::ios_base::out | std::ios_base::in );
	// IMPORTANT!
	// always pulses the oscilloscope trigger at the end!
	buf << "\r\n t = 0.01 \r\n pulseon: " + str( OSCILLOSCOPE_TRIGGER ) + " 0.02\r\n t += 0.1\r\n";
	buf << scriptFile.rdbuf();
	// this is used to more easily deal some of the analysis of the script.
	buf << "\r\n\r\n__END__";
	// for whatever reason, after loading rdbuf into a stringstream, the stream seems to not 
	// want to >> into a string. tried resetting too using seekg, but whatever, this works.
	currentMasterScript.str("");
	currentMasterScript.str( buf.str());
	currentMasterScript.clear();
	currentMasterScript.seekg(0);
	scriptFile.close();
}


// makes sure formatting is correct, returns the arguments and the function name from reading the firs real line of a function file.
void ExperimentThreadManager::analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args)
{
	args.clear();
	ScriptStream defStream(defLine);
	std::string word;
	defStream >> word;
	if (word == "")
	{
		defStream >> word;
	}
	if (word != "def")
	{
		thrower ("Function file (extenion \".func\") in functions folder was not a function because it did not"
				 " start with \"def\"! Functions must start with this. Instead it started with \"" + word + "\".");
	}
	std::string functionDeclaration, functionArgumentList;
	functionDeclaration = defStream.getline( ':' );
	int initNamePos = defLine.find_first_not_of(" \t");
	functionName = functionDeclaration.substr(initNamePos, functionDeclaration.find_first_of("(") - initNamePos);

	if (functionName.find_first_of(" ") != std::string::npos)
	{
		thrower ("Function name included a space!");
	}
	int initPos = functionDeclaration.find_first_of("(");
	if (initPos == std::string::npos)
	{
		thrower ("No starting parenthesis \"(\" in function definition. Use \"()\" if no arguments.");
	}
	initPos++;
	int endPos = functionDeclaration.find_last_of(")");
	if (endPos == std::string::npos)
	{
		thrower ("No ending parenthesis \")\" in function definition. Use \"()\" if no arguments.");
	}
	functionArgumentList = functionDeclaration.substr(initPos, endPos - initPos);
	endPos = functionArgumentList.find_first_of(",");
	initPos = functionArgumentList.find_first_not_of(" \t");
	bool good = true;
	// fill out args.
	while (initPos != std::string::npos)
	{
		// get initial argument
		std::string tempArg = functionArgumentList.substr(initPos, endPos - initPos);
		if (endPos == std::string::npos)
		{
			functionArgumentList = "";
		}
		else
		{
			functionArgumentList.erase(0, endPos + 1);
		}
		// clean up any spaces on beginning and end.
		int lastChar = tempArg.find_last_not_of(" \t");
		int lastSpace = tempArg.find_last_of(" \t");
		if (lastSpace > lastChar)
		{
			tempArg = tempArg.substr(0, lastChar + 1);
		}
		// now it should be clean. Check if there are spaces in the middle.
		if (tempArg.find_first_of(" \t") != std::string::npos)
		{
			thrower ( "bad argument list in function. It looks like there might have been a space or tab inside "
					 "the function argument? (A low level bug, this shouldn't happen.)" );
		}
		if (tempArg == "")
		{
			thrower ("bad argument list in function. It looks like there might have been a stray \",\"?");
		}
		args.push_back(tempArg);
		endPos = functionArgumentList.find_first_of(",");
		initPos = functionArgumentList.find_first_not_of(" \t");
	}
}

// at least right now, this doesn't support varying any of the values of the constant vector. this could probably
// be sensibly changed at some point.
bool ExperimentThreadManager::handleVectorizedValsDeclaration ( std::string word, ScriptStream& stream, 
															std::vector<vectorizedNiawgVals>& constVecs, std::string& warnings )
{
	if ( word != "var_v" )
	{
		return false;
	}
	std::string vecLength;
	vectorizedNiawgVals tmpVec;
	stream >> vecLength >> tmpVec.name;
	for ( auto& cv : constVecs )
	{
		if ( tmpVec.name == cv.name )
		{
			thrower ( "Constant Vector name \"" + tmpVec.name + "\"being re-used! You may only declare one constant "
					  "vector with this name." );
		}
	}
	UINT vecLength_ui = 0;
	try
	{
		vecLength_ui = boost::lexical_cast<UINT>( vecLength );
	}
	catch ( boost::bad_lexical_cast )
	{
		thrower ( "Failed to convert constant vector length to an unsigned int!" );
	}
	if ( vecLength_ui == 0 || vecLength_ui > MAX_NIAWG_SIGNALS)
	{
		thrower ( "Invalid constant vector length: " + str ( vecLength_ui ) + ", must be greater than 0 and less than " 
				  + str ( MAX_NIAWG_SIGNALS ) );
	}
	std::string bracketDelims;
	stream >> bracketDelims;
	if ( bracketDelims != "[" )
	{
		thrower ( "Expected \"[\" after constant vector size and name." );
	}
	tmpVec.vals.resize ( vecLength_ui );
	for ( auto& val : tmpVec.vals )
	{
		stream >> val;
	}
	stream >> bracketDelims;
	if ( bracketDelims != "]" )
	{
		thrower ( "Expected \"]\" after constant vector values. Is the vector size right?" );
	}
	constVecs.push_back ( tmpVec );
	return true;
}


bool ExperimentThreadManager::handleVariableDeclaration( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
													 std::string scope, std::string& warnings )
{
	if ( word != "var" )
	{
		return false;
	}
	// add to variables!
	std::string name, valStr;
	stream >> name >> valStr;
	parameterType tmpVariable;
	tmpVariable.constant = true;
	std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
	tmpVariable.name = name;
	for ( auto var : vars )
	{
		if ( var.name == tmpVariable.name )
		{
			if ( var.parameterScope == GLOBAL_PARAMETER_SCOPE )
			{
				warnings += "Warning: local variable \"" + tmpVariable.name + "\" with scope \"" 
					+ scope + "\"is being overwritten by a parameter with the same name and "
					"global parameter scope. It is generally recommended to use the appropriate local scope when "
					"possible.\r\n";
				// this variable is being overwritten, so don't add this variable vector
				return true;
			}
			else if ( str( var.parameterScope, 13, false, true ) == str( scope, 13, false, true ) )
			{
				// being overwritten so don't add, but the variable was specific, so this must be fine.
				return true;
			}
		}
	}
	bool found = false;
	double val;
	try
	{
		val = boost::lexical_cast<double>( valStr );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "Bad string for value of local variable " + str( name ) );
	}
	tmpVariable.constantValue = val;
	tmpVariable.scanDimension = 1;
	tmpVariable.parameterScope = str(scope, 13, false, true);
	tmpVariable.ranges.push_back ( { val, val } );
	// these are always constants, so just go ahead and set the keyvalue for use manually. 
	if ( vars.size( ) == 0 )
	{
		tmpVariable.keyValues = std::vector<double>( 1, val );
	}
	else
	{
		tmpVariable.keyValues = std::vector<double>( vars.front( ).keyValues.size( ), val );
	}	
	vars.push_back( tmpVariable );
	return true;
}


// if it handled it, returns true, else returns false.
bool ExperimentThreadManager::handleTimeCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars, 
											  std::string scope, timeType& operationTime )
{
	try
	{
		if (word == "t")
		{
			std::string command;
			stream >> command;
			word += command;
		}
		//
		if (word == "t++")
		{
			operationTime.second++;
		}
		else if (word == "t+=")
		{
			Expression time;
			stream >> time;
			try
			{
				operationTime.second += time.evaluate ();
			}
			catch (Error&)
			{
				time.assertValid (vars, scope);
				operationTime.first.push_back (time);
			}
		}
		else if (word == "t=")
		{
			Expression time;
			stream >> time;
			try
			{
				operationTime.second = time.evaluate ();
			}
			catch (Error&)
			{
				time.assertValid (vars, scope);
				operationTime.first.push_back (time);
				// because it's equals. There shouldn't be any extra terms added to this now.
				operationTime.second = 0;
			}
		}
		else
		{
			return false;
		}
		return true;
	}
	catch (Error & err)
	{
		throwNested ("Error seen while handling time commands. Word was \"" + word + "\"");
	}
}

/* returns true if handles word, false otherwise. */
bool ExperimentThreadManager::handleDoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
									   DoSystem& ttls, std::vector<std::pair<UINT, UINT>>& ttlShades, UINT seqNum, 
									   std::string scope, timeType& operationTime )
{
	if ( word == "on:" || word == "off:" )
	{
		std::string name;
		stream >> name;
		ttls.handleTtlScriptCommand( word, operationTime, name, ttlShades, vars, seqNum, scope );
	}
	else if ( word == "pulseon:" || word == "pulseoff:" )
	{
		// this requires handling time as it is handled above.
		std::string name;
		Expression pulseLength;
		stream >> name >> pulseLength;
		ttls.handleTtlScriptCommand( word, operationTime, name, pulseLength, ttlShades, vars, seqNum, scope );
	}
	else
	{
		return false;
	}
	return true;
}

/* returns true if handles word, false otherwise. */
bool ExperimentThreadManager::handleAoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
											AoSystem& aoSys, std::vector<UINT>& dacShades, DoSystem& ttls, UINT seqNum, 
											std::string scope, timeType& operationTime )
{
	if ( word == "dac:" )
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.finalVal;
		command.finalVal.assertValid( vars, scope );
		command.time = operationTime;
		command.commandName = "dac:";
		command.numSteps.expressionStr = command.initVal.expressionStr = "__NONE__";
		command.rampTime.expressionStr = command.rampInc.expressionStr = "__NONE__";
		try
		{
			aoSys.handleDacScriptCommand( command, name, dacShades, vars, ttls, seqNum );
		}
		catch ( Error&  )
		{
			throwNested( "Error handling \"dac:\" command." );
		}
	}
	else if ( word == "daclinspace:" )
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.initVal >> command.finalVal >> command.rampTime >> command.numSteps;
		command.initVal.assertValid( vars, scope );
		command.finalVal.assertValid( vars, scope );
		command.rampTime.assertValid( vars, scope );
		command.numSteps.assertValid( vars, scope );
		command.time = operationTime;
		command.commandName = "daclinspace:";
		// not used here.
		command.rampInc.expressionStr = "__NONE__";
		//
		try
		{
			aoSys.handleDacScriptCommand( command, name, dacShades, vars, ttls, seqNum );
		}
		catch ( Error& )
		{
			throwNested(  "Error handling \"dacLinSpace:\" command." );
		}
	}
	else if ( word == "dacarange:" )
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.initVal >> command.finalVal >> command.rampTime >> command.rampInc;
		command.initVal.assertValid( vars, scope );
		command.finalVal.assertValid( vars, scope );
		command.rampTime.assertValid( vars, scope );
		command.rampInc.assertValid( vars, scope );
		command.time = operationTime;
		command.commandName = "dacarange:";
		// not used here.
		command.numSteps.expressionStr = "__NONE__";
		try
		{
			aoSys.handleDacScriptCommand( command, name, dacShades, vars, ttls, seqNum );
		}
		catch ( Error& )
		{
			throwNested("Error handling \"dacArange:\" command." );
		}
	}
	else
	{
		return false;
	}
	return true;
}


/*
	this function can be called directly from scripts. Insert things inside the function to make it do something
	custom that's not possible inside the scripting language.
*/
void ExperimentThreadManager::callCppCodeFunction()
{
	// not used at the moment
}


bool ExperimentThreadManager::isValidWord( std::string word )
{
	if (word == "t" || word == "t++" || word == "t+=" || word == "t=" || word == "on:" || word == "off:"
		 || word == "dac:" || word == "dacarange:" || word == "daclinspace:" || word == "call" 
		 || word == "repeat:" || word == "end" || word == "pulseon:" || word == "pulseoff:" || word == "callcppcode")
	{
		return true;
	}
	return false;
}
 
// just a simple wrapper so that I don't have if (!quiet){ everywhere in the main thread.
void ExperimentThreadManager::expUpdate(std::string text, Communicator& comm, bool quiet)
{
	if (!quiet)
	{
		comm.sendStatus(text);
	}
}

UINT ExperimentThreadManager::determineVariationNumber( std::vector<parameterType> variables )
{
	int variationNumber;
	if ( variables.size() == 0)
	{
		variationNumber = 1;
	}
	else
	{
		variationNumber = variables.front().keyValues.size();
		if (variationNumber == 0)
		{
			variationNumber = 1;
		}
	}
	return variationNumber;
}


void ExperimentThreadManager::checkTriggerNumbers ( ExperimentThreadInput* input, std::string& warnings,
												UINT variations, microwaveSettings settings, 
												std::vector<std::vector<parameterType>>& expParams, 
												std::vector<deviceOutputInfo>& agRunInfo)
{
	/// check all trigger numbers between the DO system and the individual subsystems. These should almost always match,
	/// a mismatch is usually user error in writing the script.
	for ( auto seqInc : range ( input->seq.sequence.size ( ) ) )
	{
		bool niawgMismatch = false, rsgMismatch=false;
		std::vector<std::array<bool, 2>> agMismatchVec ( input->agilents.size ( ), { false,false } );
		auto& seqVariables = expParams[ seqInc ];
		for ( auto variationInc : range ( variations ) )
		{
			if ( input->runList.master)
			{
				UINT actualTrigs = input->ttls.countTriggers ( { DoRows::which::D,15 }, variationInc, seqInc );
				UINT dacExpectedTrigs = input->aoSys.getNumberSnapshots ( variationInc, seqInc );
				std::string infoString = "Actual/Expected DAC Triggers: " + str ( actualTrigs ) + "/" 
					+ str ( dacExpectedTrigs ) + ".";
				if ( actualTrigs != dacExpectedTrigs )
				{
					// this is a serious low level error. throw, don't warn.
					thrower ( "the number of dac triggers that the ttl system sends to the dac line does not "
								"match the number of dac snapshots! " + infoString + ", seen in sequence #"
								+ str ( seqInc ) + " variation #" + str ( variationInc ) + "\r\n" );
				}
				if ( seqInc == 0 && variationInc == 0 && input->debugOptions.outputExcessInfo )
				{
					input->debugOptions.message += infoString + "\n";
				}
			}
			if ( input->runList.niawg && !niawgMismatch )
			{
				auto actualTrigs = input->ttls.countTriggers ( input->niawg.getTrigLines ( ), variationInc, seqInc );
				auto niawgExpectedTrigs = input->niawg.getNumberTrigsInScript ( );
				std::string infoString = "Actual/Expected NIAWG Triggers: " + str ( actualTrigs ) + "/" 
					+ str ( niawgExpectedTrigs ) + ".";
				if ( actualTrigs != niawgExpectedTrigs )
				{
					warnings += "WARNING: the NIAWG is not getting triggered by the ttl system the same number"
						" of times a trigger command appears in the NIAWG script. " + infoString + " First "
						"instance seen sequence number " + str ( seqInc ) + " variation " + str ( variationInc ) 
						+ ".\r\n";
					niawgMismatch = true;
				}
				if ( seqInc == 0 && variationInc == 0 && input->debugOptions.outputExcessInfo )
				{
					input->debugOptions.message += infoString + "\n";
				}
			}
			/// check RSG
			if ( !rsgMismatch )
			{
				auto actualTrigs = input->ttls.countTriggers ( input->rsg.getRsgTriggerLine ( ), variationInc, seqInc );
				auto rsgExpectedTrigs = input->rsg.getNumTriggers ( variationInc, settings );
				std::string infoString = "Actual/Expected RSG Triggers: " + str ( actualTrigs ) + "/"
					+ str ( rsgExpectedTrigs ) + ".";
				if ( actualTrigs != rsgExpectedTrigs && rsgExpectedTrigs != 0 && rsgExpectedTrigs != 1 )
				{
					warnings += "WARNING: the RSG is not getting triggered by the ttl system the same number"
						" of times a trigger command appears in the master script. " + infoString + " First "
						"instance seen sequence number " + str ( seqInc ) + " variation " + str ( variationInc )
						+ ".\r\n";
					rsgMismatch = true;
				}
				if ( seqInc == 0 && variationInc == 0 && input->debugOptions.outputExcessInfo )
				{
					input->debugOptions.message += infoString + "\n";
				}
				/// check Agilents
				for ( auto agInc : range ( input->agilents.size ( ) ) )
				{
					auto& agilent = input->agilents[ agInc ].get();
					for ( auto chan : range ( 2 ) )
					{
						auto& agChan = agRunInfo[agInc].channel[ chan ];
						if ( agChan.option != AgilentChannelMode::which::Script || agMismatchVec[ agInc ][ chan ] )
						{
							continue;
						}
						UINT actualTrigs = input->runList.master ? input->ttls.countTriggers ( agilent.getTriggerLine ( ),
																				variationInc, seqInc ) : 0;
						UINT agilentExpectedTrigs = agChan.scriptedArb.wave.getNumTrigs ( );
						std::string infoString = "Actual/Expected " + agilent.configDelim + " Triggers: "
							+ str ( actualTrigs ) + "/" + str ( agilentExpectedTrigs ) + ".";
						if ( actualTrigs != agilentExpectedTrigs )
						{
							warnings += "WARNING: Agilent " + agilent.configDelim + " is not getting "
								"triggered by the ttl system the same number of times a trigger command "
								"appears in the agilent channel " + str ( chan + 1 ) + " script. " + infoString 
								+ " First seen in sequence #" + str ( seqInc ) + ", variation #" + str ( variationInc ) 
								+ ".\r\n";
							agMismatchVec[ agInc ][ chan ] = true;
						}
						if ( seqInc == 0 && variationInc == 0 && input->debugOptions.outputExcessInfo )
						{
							input->debugOptions.message += infoString + "\n";
						}
					}
				}
			}
		}
	}
}


bool ExperimentThreadManager::handleFunctionCall( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
											  DoSystem& ttls, AoSystem& aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades, 
											  std::vector<UINT>& dacShades, UINT seqNum, std::string& warnings,
											  std::string callingFunction, timeType& operationTime )
{
	if ( word != "call" )
	{
		return false;
	}
	// calling a user-defined function. Get the name and the arguments to pass to the function handler.
	std::string functionCall, functionName, functionArgs;
	functionCall = stream.getline( '\n' );
	boost::erase_all ( functionCall, "\r" );
	int pos = functionCall.find_first_of( "(" ) + 1;
	int finalpos = functionCall.find_last_of( ")" );

	functionName = functionCall.substr( 0, pos - 1 );
	functionArgs = functionCall.substr( pos, finalpos - pos );
	std::string arg;
	std::vector<std::string> args;
	while ( true )
	{
		pos = functionArgs.find_first_of( ',' );
		if ( pos == std::string::npos )
		{
			arg = functionArgs.substr( 0, functionArgs.size( ) );
			if ( arg != "" )
			{
				args.push_back( arg );
			}
			break;
		}
		arg = functionArgs.substr( 0, pos );
		args.push_back( arg );
		// cut oinputut that argument off the string.
		functionArgs = functionArgs.substr( pos + 1, functionArgs.size( ) );
	}
	if ( functionName == callingFunction )
	{
		thrower ( "Recursive function call detected! " + callingFunction + " called itself! This is not allowed."
				 " There is no way to end a function call conditionally so this will necessarily result in an"
				 " infinite recursion\r\n" );
	}
	try
	{
		analyzeFunction( functionName, args, ttls, aoSys, ttlShades, dacShades, vars, seqNum, warnings, 
			operationTime, callingFunction);
	}
	catch ( Error& )
	{
		throwNested( "Error handling Function call to function " + functionName + "." );
	}
	return true;
}


void ExperimentThreadManager::updatePlotX_vals ( ExperimentThreadInput* input, 
											 std::vector<std::vector<parameterType>>& expParams)
{
	// remove old plots that aren't trying to sustain.
	input->plotterInput->key = ParameterSystem::getKeyValues ( expParams[ 0 ] );
	auto& pltInput = input->plotterInput;
	auto plotInc = 0;
	for ( auto plotParams : pltInput->plotInfo )
	{
		int which = pltInput->dataArrays.size () - pltInput->plotInfo.size () + plotInc;
		if (which < 0 || which >= pltInput->dataArrays.size ())
		{
			thrower ("Plotter data array access out of range?!");
		}
		auto& data = pltInput->dataArrays[which];
		for ( auto& line : data )
		{
			// initialize x axis for all data sets.
			UINT count = 0;
			for ( auto& keyItem : pltInput->key )
			{
				line->at ( count++ ).x = keyItem;
			}
		}
		plotInc++;
	}
}
