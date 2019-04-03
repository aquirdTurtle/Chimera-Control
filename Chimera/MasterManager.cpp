#include "stdafx.h"
#include "MasterManager.h"
#include "DioSystem.h"
#include "AoSystem.h"
#include "CodeTimer.h"
#include "constants.h"
#include "AuxiliaryWindow.h"
#include "NiawgWaiter.h"
#include "Expression.h"
#include "Thrower.h"
#include "range.h"
#include "MainWindow.h"
#include "nidaqmx2.h"
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <regex>

MasterManager::MasterManager() {}

/*
 * The workhorse of actually running experiments. This thread procedure analyzes all of the GUI settings and current 
 * configuration settings to determine how to program and run the experiment.
 * @param voidInput: This is the only input to the proxcedure. It MUST be a pointer to a ExperimentThreadInput structure.
 * @return UINT: The return value is not used, i just return TRUE.
 */
unsigned int __stdcall MasterManager::experimentThreadProcedure( void* voidInput )
{
	CodeTimer timer;
	timer.tick("Procedure-Start");
	/// initialize various structures
	// convert the input to the correct structure.
	MasterThreadInput* input = (MasterThreadInput*)voidInput;
	// change the status of the parent object to reflect that the thread is running.
	input->thisObj->experimentIsRunning = true;
	seqInfo expSeq( input->seq.sequence.size( ) );
	UINT seqNum = 0;
	try
	{
		for ( auto& config : input->seq.sequence )
		{
			auto& seq = expSeq.sequence[seqNum]; 
			if ( input->runMaster )
			{
				seq.masterScript = ProfileSystem::getMasterAddressFromConfig( config );
				input->thisObj->loadMasterScript( seq.masterScript, seq.masterStream );
			}
			if ( input->runNiawg )
			{
				seq.niawgScript = ProfileSystem::getNiawgScriptAddrFromConfig( config );
				input->thisObj->loadNiawgScript ( seq.niawgScript, seq.niawgStream );
				if ( input->debugOptions.outputNiawgHumanScript )
				{
					input->comm->sendDebug ( std::regex_replace ( 
						"Human Script: " + seq.niawgStream.str ( ) + "\n\n", std::regex ( "[^\r]\n" ), "\r\n" ) );
				}
			}
			seqNum++;
		}
	}
	catch ( Error& err)
	{
		errBox( "Failed to load experiment sequence files!  (A low level bug, this shouldn't happen). \n" 
				+ err.trace() );
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
	auto quiet = input->quiet;
	// a couple shortcuts.
	auto& ttls = input->ttls;
	auto& aoSys = input->aoSys;
	auto& comm = input->comm;
	timer.tick("After-File-Init");

	/// ////////////////////////////
	/// start analysis & experiment
	try
	{
		bool useAuxDevices = ( input->expType == ExperimentType::MachineOptimization || input->expType == ExperimentType::Normal );
		if ( !useAuxDevices )
		{
			 expUpdate ( "Non-standard experiment type, so Tektronics & RSG will not be run.", comm, quiet );
		}
		aoSys->resetDacEvents( );
		ttls->resetTtlEvents( );
		aoSys->initializeDataObjects( input->seq.sequence.size( ), 0 );
		ttls->initializeDataObjects( input->seq.sequence.size( ), 0 );
		input->thisObj->loadSkipTimes.clear( );
		input->thisObj->loadSkipTimes.resize( input->seq.sequence.size( ) );
		input->thisObj->loadSkipTime.resize( input->seq.sequence.size( ) );
		input->rsg->clearFrequencies( );
		if ( input->runNiawg )
		{
			input->niawg->initForExperiment ( );
		}
		UINT variations;
		timer.tick("After-Init");
		for ( auto seqNum : range( input->seq.sequence.size() ) )
		{
			auto& seqVariables = input->variables[seqNum];
			//auto& seqConstants = input->constants[seqNum];
			auto& seq = expSeq.sequence[ seqNum ];
			if ( seqNum == 0 )
			{
				variations = determineVariationNumber( seqVariables );
			}
			else
			{
				if ( variations != determineVariationNumber( seqVariables ) )
				{
					thrower ( "Variation number changes between sequences! the number of variations must match"
							 " between sequences.  (A low level bug, this shouldn't happen)" );
				}
			}
			/// Prep agilents
			expUpdate( "Loading Agilent Info...", comm, quiet );
			timer.tick(str(seqNum) + "-Variation-Number-Handling");
			for ( auto agilent : input->agilents )
			{
				RunInfo dum;
				agilent->handleInput( input->profile.categoryPath, dum );
				for ( auto channelInc : range ( 2 ) )
				{
					agilent->analyzeAgilentScript ( channelInc, seqVariables );
				}
			}
			timer.tick(str(seqNum) + "-All-Agilent-Handle-Input");
			/// prep master systems
			expUpdate( "Analyzing Master Script...", comm, quiet );
			if ( input->runMaster )
			{
				comm->sendColorBox ( System::Master, 'Y' );
				input->thisObj->analyzeMasterScript( ttls, aoSys, ttlShadeLocs, dacShadeLocs, input->rsg,
													 seqVariables, seq.masterStream, seqNum,
													 input->settings.atomThresholdForSkip != UINT_MAX, warnings );
				timer.tick(str(seqNum) + "-Analyzing-Master-Script");
			}
			if ( input->thisObj->isAborting ) { thrower ( abortString ); }
			/// prep NIAWG
			if ( input->runNiawg )
			{
				comm->sendColorBox ( System::Niawg, 'Y' );
				input->niawg->analyzeNiawgScript ( seq.niawgStream, output, input->profile, input->debugOptions, 
												   warnings, input->rerngGuiForm, seqVariables );
				workingNiawgScripts[ seqNum ] = output.niawgLanguageScript;
				timer.tick(str(seqNum) + "-Preparing-Niawg");
			}
			if ( input->thisObj->isAborting ) { thrower ( abortString ); }
			input->thisObj->loadSkipTimes[seqNum].resize( variations );
		}
		if ( input->runNiawg )
		{
			input->niawg->finalizeScript ( input->repetitionNumber, "experimentScript", workingNiawgScripts, niawgMachineScript,
										   !input->niawg->outputVaries ( output ) );
			if ( input->debugOptions.outputNiawgMachineScript )
			{
				comm->sendDebug ( std::regex_replace ( 
					"NIAWG Machine Script:\n" + std::string ( niawgMachineScript.begin ( ), niawgMachineScript.end ( ) ) 
					+ "\n\n", std::regex ( "[^\r]\n" ), "\r\n" ) );
			}
			for ( auto& seqVariables : input->variables )
			{
				input->niawg->writeStaticNiawg ( output, input->debugOptions, seqVariables );
			}
			expUpdate("Constant NIAWG Waveform Preparation Completed...\r\n", comm, input->quiet );
		}
		timer.tick("After-Shading-Ttls-And-Dacs");
		if ( input->thisObj->isAborting ) { thrower ( abortString ); }
		/// The Key Interpretation step.
		// at this point, all scripts have been analyzed, and each system takes the key and generates all of the data
		// it needs for each variation of the experiment. All these calculations happen at this step.
		expUpdate( "Programming All Variation Data...\r\n", comm, quiet );
		if ( input->runMaster )
		{
			ttls->shadeTTLs ( ttlShadeLocs );
			aoSys->shadeDacs ( dacShadeLocs );
			ttls->interpretKey( input->variables );
			aoSys->interpretKey( input->variables, warnings );
		}
		if ( useAuxDevices )
		{
			input->rsg->interpretKey ( input->variables );
			input->topBottomTek->interpretKey ( input->variables );
			input->eoAxialTek->interpretKey ( input->variables );
		}
		timer.tick("Key-Interpretation");
		/// organize commands, prepping final forms of the data for each repetition.
		// This must be done after the "interpret key" step; before that commands don't have times attached to them.
		for ( auto seqInc : range( input->seq.sequence.size( ) ) )
		{
			auto& seqVariables = input->variables[seqInc];
			for ( UINT variationInc = 0; variationInc < variations; variationInc++ )
			{
				timer.tick("Variation-"+str(variationInc)+"-start");
				if ( input->thisObj->isAborting ) { thrower ( abortString ); }
				if ( input->runMaster )
				{
					double& currLoadSkipTime = input->thisObj->loadSkipTimes[seqInc][variationInc];
					currLoadSkipTime = MasterManager::convertToTime( input->thisObj->loadSkipTime[seqInc], 
																	 seqVariables, variationInc );
				    // organize & format the ttl and dac commands
					aoSys->organizeDacCommands( variationInc, seqInc );
					aoSys->setDacTriggerEvents( ttls, variationInc, seqInc );
					aoSys->findLoadSkipSnapshots( currLoadSkipTime, seqVariables, variationInc, seqInc );
					aoSys->makeFinalDataFormat( variationInc, seqInc );
					ttls->organizeTtlCommands( variationInc, seqInc );
					ttls->findLoadSkipSnapshots( currLoadSkipTime, seqVariables, variationInc, seqInc );
					ttls->convertToFinalViewpointFormat( variationInc, seqInc );
					timer.tick(str(variationInc) + "-After-Ao-And-Dio-Main");
					// run a couple checks.
					ttls->checkNotTooManyTimes( variationInc, seqInc );
					ttls->checkFinalFormatTimes( variationInc, seqInc );
					if ( ttls->countDacTriggers( variationInc, seqInc ) != aoSys->getNumberSnapshots( variationInc, seqInc ) )
					{
						thrower ( "the number of dac triggers that the ttl system sends to the dac line does not "
								 "match the number of dac snapshots! Number of dac triggers was " 
								 + str( ttls->countDacTriggers( variationInc, seqInc ) ) + " while number of dac "
								 "snapshots was " + str( aoSys->getNumberSnapshots( variationInc, seqInc ) ) );
					}
					aoSys->checkTimingsWork( variationInc, seqInc );
					if ( input->runNiawg )
					{
						if ( ttls->countTriggers ( input->niawg->getTrigLines ( ), variationInc, seqInc )
							 != input->niawg->getNumberTrigsInScript( ) )
						{
							warnings += "WARNING: the NIAWG is not getting triggered by the ttl system the same number"
								" of times a trigger command appears in the NIAWG script.\r\n";
						}
					}
					timer.tick(str(variationInc) + "-After-Ao-And-Dio-Checks");
				}
				if ( useAuxDevices )
				{
					input->rsg->orderEvents ( variationInc );
				}
			}
		}
		/// output some timing information
		timer.tick("After-All-Variation-Calculations");
		//expUpdate(timer.getTimingMessage(), comm, input->quiet);
		if (input->runMaster)
		{
			expUpdate( "Programmed time per repetition: " + str( ttls->getTotalTime( 0, 0 ) ) + "\r\n", 
					   comm, quiet );
			ULONGLONG totalTime = 0;
			for ( auto seqInc : range( input->seq.sequence.size()) )
			{
				for ( auto variationNumber : range(variations) )
				{
					totalTime += ULONGLONG( ttls->getTotalTime( variationNumber, seqInc ) 
											* input->repetitionNumber );
				}
			}
			expUpdate( "Programmed Total Experiment time: " + str( totalTime ) + "\r\n", comm, quiet );
			expUpdate( "Number of TTL Events in experiment: " + str( ttls->getNumberEvents( 0, 0 ) ) + "\r\n", comm, quiet );
			expUpdate( "Number of DAC Events in experiment: " + str( aoSys->getNumberEvents( 0, 0 ) ) + "\r\n", comm, quiet );
		}
		/// finish up
		if ( input->runMaster )
		{
			handleDebugPlots( input->debugOptions, comm, ttls, aoSys, input->ttlData, input->dacData );
		}
		// update the colors of the global variable control.
		input->globalControl->setUsages( input->variables );
		for ( auto& seqvars : input->variables )
		{
			for ( auto& var : seqvars )
			{
				if ( !var.constant && !var.active )
				{
					warnings += "WARNING: Variable " + var.name + " is varied, but not being used?!?";                 
				}
			}
		}
		comm->sendError( warnings );
		/// /////////////////////////////
		/// Begin experiment loop
		// TODO: Handle randomizing repetitions. The thread will need to split into separate if/else statements here.
		if (input->runMaster)
		{
			comm->sendColorBox( System::Master, 'G' );
		}
		// loop for variations
		for (const UINT& variationInc : range( variations ))
		{
			timer.tick("Variation-"+str(variationInc+1)+"-Start");
			expUpdate( "Variation #" + str( variationInc + 1 ) + "\r\n", comm, quiet );
			if ( input->aiSys->wantsQueryBetweenVariations( ) )
			{
				expUpdate( "Querying Voltages...\r\n", comm, quiet );
				input->auxWin->PostMessage( MainWindow::LogVoltsMessageID, variationInc );
			}
			if ( input->debugOptions.sleepTime != 0 )
			{
				expUpdate ( "PAUSED!\r\n", comm, quiet );
				Sleep ( input->debugOptions.sleepTime );
				expUpdate ( "UNPAUSED!\r\n", comm, quiet );
			}
			for ( auto seqInc : range(input->seq.sequence.size( ) ) )
			{
				for (auto tempVariable : input->variables[seqInc])
				{
					// if varies...
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
			expUpdate( "Programming RSG, Agilents, NIAWG, & Teltronics...\r\n", comm, quiet );
			if ( useAuxDevices )
			{
				input->rsg->programRsg ( variationInc );
				input->rsg->setInfoDisp ( variationInc );
			}
			timer.tick(str(variationInc + 1)+"-After-Programming-Rsg");
			// program devices
			for (auto& agilent : input->agilents)
			{
				agilent->setAgilent( variationInc, input->variables[0] );
			}
			// check right number of triggers (currently must be done after agilent is set.
			for ( auto& agilent : input->agilents )
			{
				for ( auto chan : range( 2 ) )
				{
					if ( agilent->getOutputInfo( ).channel[chan].option != 4 )
					{
						continue;
					}
					UINT ttlTrigs = input->runMaster? ttls->countTriggers ( agilent->getTriggerLine ( ), variationInc, 0 ) : 0;
					UINT agilentExpectedTrigs = agilent->getOutputInfo( ).channel[chan].scriptedArb.wave.getNumTrigs( );
					if ( ttlTrigs != agilentExpectedTrigs )
					{
						warnings += "WARNING: Agilent " + agilent->configDelim + " is not getting triggered by the "
							"ttl system the same number of times a trigger command appears in the agilent channel "
							+ str( chan + 1 ) + " script. There are " + str( agilentExpectedTrigs ) + " triggers in"
							" the agilent script, and " + str( ttlTrigs ) + " ttl triggers sent to that agilent.\r\n";
					}
				}
			}
			timer.tick(str(variationInc + 1) + "-After-Programming-Agilents");
			if (input->runNiawg)
			{
				input->niawg->programNiawg( input, output, warnings, variationInc, variations, variedMixedSize,
											niawgMachineScript, input->rerngGuiForm, input->rerngGui );
				input->niawg->turnOffRerng( );
				input->conditionVariableForRerng->notify_all( );
				input->niawg->waitForRerng( false );
				input->niawg->handleStartingRerng( input, output );
				timer.tick(str(variationInc + 1) + "-After-Programming-NIAWG");
			}
			comm->sendError( warnings );
			if ( useAuxDevices )
			{
				input->topBottomTek->programMachine ( variationInc );
				input->eoAxialTek->programMachine ( variationInc );
			}
			timer.tick(str(variationInc + 1) + "-After-Programming-Tektronix");
			timer.tick(str(variationInc + 1) + "-After-All-Programming");
			//
			comm->sendRepProgress( 0 );
			expUpdate( "Running Experiment.\r\n", comm, quiet );
			for (UINT repInc = 0; repInc < input->repetitionNumber; repInc++)
			{
				for (auto seqInc : range(input->seq.sequence.size()))
				{
					if (input->thisObj->isAborting) { thrower ( abortString ); }
					else if (input->thisObj->isPaused)
					{
						expUpdate( "Paused\r\n!", comm, quiet );
						while (input->thisObj->isPaused)
						{
							// this could be changed to be a bit smarter using a std::condition_variable
							Sleep( 100 );
							if ( input->thisObj->isAborting ) { thrower ( abortString ); }
						}
						expUpdate( "Un-Paused!\r\n", comm, quiet );
					}
					comm->sendRepProgress( repInc + 1 );
					if (input->runMaster)
					{
						aoSys->stopDacs();
						// it's important to grab the skipoption from input->skipNext only once because in principle
						// if the cruncher thread was running behind, it could change between writing and configuring the 
						// aoSys and configuring the TTLs;
						bool skipOption = input->skipNext == NULL ? false : input->skipNext->load ( );
						aoSys->configureClocks( variationInc, seqInc, skipOption);
						aoSys->writeDacs( variationInc, seqInc, skipOption);
						aoSys->startDacs();
						ttls->writeTtlData( variationInc, seqInc, skipOption);
						ttls->startBoard();
						ttls->waitTillFinished( variationInc, seqInc, skipOption);
					}
				}
			}
			expUpdate( "\r\n", comm, quiet );
		}
		/// conclude.
		expUpdate( "\r\nExperiment Finished Normally.\r\n", comm, quiet );
		comm->sendColorBox( System::Master, 'B' );
		if (input->runMaster)
		{
			// stop is necessary else the dac system will still be running and won't allow updates through normal means.
			aoSys->stopDacs();
			aoSys->unshadeDacs();
			try
			{
				// make sure the display accurately displays the state that the experiment finished at.
				aoSys->setDacStatusNoForceOut( aoSys->getFinalSnapshot( ) );
				ttls->unshadeTtls( );
				ttls->setTtlStatusNoForceOut( ttls->getFinalSnapshot( ) );
			}
			catch ( Error& ) { /* this gets thrown if no dac events. just continue.*/ }
		}
		if (input->runNiawg)
		{
			input->niawg->cleanupNiawg( input->profile, input->runMaster, output, comm, 
										input->settings.dontActuallyGenerate );
		}
		input->thisObj->experimentIsRunning = false;
		switch ( input->expType )
		{
			case ExperimentType::CameraCal:
				comm->sendCameraCalFin ( );
				break;
			case ExperimentType::LoadMot:
			case ExperimentType::MachineOptimization:
			case ExperimentType::MotSize:
			case ExperimentType::MotTemperature:
			case ExperimentType::PgcTemperature:
			case ExperimentType::GreyTemperature:
				comm->sendFinish ( input->expType );
				break;
			default:
				comm->sendFinish ( ExperimentType::Normal );
		}
	}
	catch (Error& exception)
	{
		if (input->runNiawg)
		{
			// clear out some niawg stuff
			for (auto& wave : output.waves)
			{
				wave.core.waveVals.clear();
				wave.core.waveVals.shrink_to_fit();
			}
		}
		input->thisObj->experimentIsRunning = false;
		{
			std::lock_guard<std::mutex> locker( input->thisObj->abortLock );
			input->thisObj->isAborting = false;
		}
		if (input->runMaster)
		{
			input->ttls->unshadeTtls();
			input->aoSys->unshadeDacs();
		}
		if ( input->thisObj->isAborting )
		{
			expUpdate( abortString, comm, quiet );
			comm->sendColorBox( System::Master, 'B' );
		}
		else
		{
			// No quiet option for a bad exit.
			comm->sendColorBox( System::Master, 'R' );
			comm->sendStatus( "Bad Exit!\r\n" );
			auto txt = "Exited main experiment thread abnormally." + exception.trace ( );
			//comm->sendError( txt );
			comm->sendFatalError( txt );
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


void MasterManager::analyzeMasterScript ( DioSystem* ttls, AoSystem* aoSys,
										  std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades,
										  RhodeSchwarz* rsg, std::vector<parameterType>& vars,
										  ScriptStream& currentMasterScript, UINT seqNum, bool expectsLoadSkip,
										  std::string& warnings )
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
 		if ( handleTimeCommands ( word, currentMasterScript, vars, scope ) )
		{
			// got handled, so break out of the if-else by entering this scope.
		}
		else if ( handleVariableDeclaration ( word, currentMasterScript, vars, scope, warnings ) )
		{
		}
		else if ( handleDioCommands ( word, currentMasterScript, vars, ttls, ttlShades, seqNum, scope ) )
		{
		}
		else if ( handleAoCommands ( word, currentMasterScript, vars, aoSys, dacShades, ttls, seqNum, scope ) )
		{
		}
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
			rsgEventForm info;
			currentMasterScript >> info.frequency;
			info.frequency.assertValid ( vars, scope );
			currentMasterScript >> info.power;
			info.power.assertValid ( vars, scope );
			info.time = operationTime;
			rsg->addFrequency ( info );
		}
		/// deal with function calls.
		else if ( handleFunctionCall ( word, currentMasterScript, vars, ttls, aoSys, ttlShades, dacShades, rsg, seqNum,
									   warnings, PARENT_PARAMETER_SCOPE ) )
		{
		}
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


void MasterManager::analyzeFunction ( std::string function, std::vector<std::string> args, DioSystem* ttls,
									  AoSystem* aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades,
									  std::vector<UINT>& dacShades, RhodeSchwarz* rsg, std::vector<parameterType>& vars,
									  UINT seqNum, std::string& warnings )
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
				  + str ( args.size ( ) ) + ", number expected was " + str ( functionArgs.size ( ) ) + ". Function arguments were:"
				  + functionArgsString + "." );
	}
	std::vector<std::pair<std::string, std::string>> replacements;
	for ( UINT replacementInc = 0; replacementInc < args.size ( ); replacementInc++ )
	{
		replacements.push_back ( { functionArgs[ replacementInc ], args[ replacementInc ] } );
	}
	functionStream.loadReplacements ( replacements );
	std::string currentFunctionText = functionStream.str ( );
	///
	functionStream >> word;
	while ( !( functionStream.peek ( ) == EOF ) || word != "__end__" )
	{
		if ( handleTimeCommands ( word, functionStream, vars, scope ) )
		{
			// got handled
		}
		else if ( handleVariableDeclaration ( word, functionStream, vars, scope, warnings ) )
		{
		}
		else if ( handleDioCommands ( word, functionStream, vars, ttls, ttlShades, seqNum, scope ) )
		{
		}
		else if ( handleAoCommands ( word, functionStream, vars, aoSys, dacShades, ttls, seqNum, scope ) )
		{
		}
		/// callcppcode command
		else if ( word == "callcppcode" )
		{
			// and that's it... 
			callCppCodeFunction ( );
		}
		/// Handle RSG calls.
		else if ( word == "rsg:" )
		{
			rsgEventForm info;
			functionStream >> info.frequency >> info.power;
			info.frequency.assertValid ( vars, scope );
			info.power.assertValid ( vars, scope );
			// test frequency
			info.time = operationTime;
			rsg->addFrequency ( info );
		}
		/// deal with function calls.
		else if ( handleFunctionCall ( word, functionStream, vars, ttls, aoSys, ttlShades, dacShades, rsg, seqNum,
									   warnings, function ) )
		{
		}
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


bool MasterManager::getAbortStatus ( )
{
	return isAborting;
}

double MasterManager::convertToTime( timeType time, std::vector<parameterType> variables, UINT variation )
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

/*
I think I can get rid of this??? or maybe just simplify a lot...
*/
void MasterManager::handleDebugPlots( debugInfo debugOptions, Communicator* comm, DioSystem* ttls, AoSystem* aoSys,
									  std::vector<std::vector<pPlotDataVec>> ttlData, 
									  std::vector<std::vector<pPlotDataVec>> dacData )
{
	// handle on-screen plots.
	ttls->fillPlotData( 0, ttlData );
	aoSys->fillPlotData( 0, dacData, ttls->getFinalTimes() );
	if ( debugOptions.showTtls )
	{
		comm->sendDebug( ttls->getTtlSequenceMessage( 0, 0 ) );
	}
	if ( debugOptions.showDacs )
	{
		comm->sendDebug( aoSys->getDacSequenceMessage( 0, 0 ) );
	}
	// no quiet on warnings or debug messages.
	comm->sendDebug( debugOptions.message );
}


bool MasterManager::runningStatus()
{
	return experimentIsRunning;
}


/***
 * this function is very similar to startExperimentThread but instead of getting anything from the current profile, it
 * knows exactly where to look for the MOT profile. This is currently hard-coded.
 */
void MasterManager::loadMotSettings(MasterThreadInput* input)
{	
	if ( experimentIsRunning )
	{
		delete input;
		thrower ( "Experiment is Running! Please abort the current run before setting the MOT settings." );
	}
	input->thisObj = this;
	ParameterSystem::generateKey( input->variables, false, input->variableRangeInfo );
	runningThread = (HANDLE)_beginthreadex( NULL, NULL, &MasterManager::experimentThreadProcedure, input, NULL, NULL );
}


HANDLE MasterManager::startExperimentThread(MasterThreadInput* input)
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
	runningThread = (HANDLE)_beginthreadex( NULL, NULL, &MasterManager::experimentThreadProcedure, input, NULL, NULL );
	SetThreadPriority( runningThread, THREAD_PRIORITY_HIGHEST );
	return runningThread;
}


bool MasterManager::getIsPaused()
{
	return isPaused;
}


void MasterManager::pause()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't pause the experiment if the experiment isn't running!" );
	}
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = true;
}


void MasterManager::unPause()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't unpause the experiment if the experiment isn't running!" );
	}
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = false;
}


void MasterManager::abort()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't abort the experiment if the experiment isn't running!" );
	}
	std::lock_guard<std::mutex> locker( abortLock );
	isAborting = true;
}


void MasterManager::loadNiawgScript ( std::string scriptAddress, ScriptStream& niawgScript )
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s ( &file, cstr ( scriptAddress ), "r" );
	if ( !file )
	{
		thrower ( "The Master Script File " + scriptAddress + " does not exist! The Master-Manager tried to "
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


void MasterManager::loadMasterScript(std::string scriptAddress, ScriptStream& currentMasterScript )
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
void MasterManager::analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args)
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


bool MasterManager::handleVariableDeclaration( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
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
				warnings += "Warning: local variable \"" + var.name + "\" is being overwritten by a global or configuration"
					" variable with the same name.\r\n";
				// this variable is being overwritten, so don't add this variable vector
				return true;
			}
			else if ( str( var.parameterScope, 13, false, true ) == str( scope, 13, false, true ) )
			{
				// being overwritten, but the variable was specific, so this must be fine.
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
bool MasterManager::handleTimeCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars, 
										std::string scope )
{
	if ( word == "t" )
	{
		std::string command;
		stream >> command;
		word += command;
	}
	//
	if ( word == "t++" )
	{
		operationTime.second++;
	}
	else if ( word == "t+=" )
	{
		Expression time;
		stream >> time;
		try
		{
			operationTime.second += time.evaluate( );
		}
		catch ( Error& )
		{
			time.assertValid( vars, scope );
			operationTime.first.push_back( time );
		}
	}
	else if ( word == "t=" )
	{
		Expression time;
		stream >> time;
		try
		{
			operationTime.second = time.evaluate( );
		}
		catch ( Error & )
		{
			time.assertValid( vars, scope );
			operationTime.first.push_back( time );
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

/* returns true if handles word, false otherwise. */
bool MasterManager::handleDioCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
									   DioSystem* ttls, std::vector<std::pair<UINT, UINT>>& ttlShades, UINT seqNum, 
									   std::string scope )
{
	if ( word == "on:" || word == "off:" )
	{
		std::string name;
		stream >> name;
		ttls->handleTtlScriptCommand( word, operationTime, name, ttlShades, vars, seqNum, scope );
	}
	else if ( word == "pulseon:" || word == "pulseoff:" )
	{
		// this requires handling time as it is handled above.
		std::string name;
		Expression pulseLength;
		stream >> name >> pulseLength;
		ttls->handleTtlScriptCommand( word, operationTime, name, pulseLength, ttlShades, vars, seqNum, scope );
	}
	else
	{
		return false;
	}
	return true;
}

/* returns true if handles word, false otherwise. */
bool MasterManager::handleAoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
									  AoSystem* aoSys, std::vector<UINT>& dacShades, DioSystem* ttls, UINT seqNum, 
									  std::string scope )
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
			aoSys->handleDacScriptCommand( command, name, dacShades, vars, ttls, seqNum );
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
			aoSys->handleDacScriptCommand( command, name, dacShades, vars, ttls, seqNum );
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
			aoSys->handleDacScriptCommand( command, name, dacShades, vars, ttls, seqNum );
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
void MasterManager::callCppCodeFunction()
{
	// not used at the moment
}


bool MasterManager::isValidWord( std::string word )
{
	if (word == "t" || word == "t++" || word == "t+=" || word == "t=" || word == "on:" || word == "off:"
		 || word == "dac:" || word == "dacarange:" || word == "daclinspace:" || word == "rsg:" || word == "call" 
		 || word == "repeat:" || word == "end" || word == "pulseon:" || word == "pulseoff:" || word == "callcppcode")
	{
		return true;
	}
	return false;
}

// just a simple wrapper so that I don't have if (!quiet){ everywhere in the main thread.
void MasterManager::expUpdate(std::string text, Communicator* comm, bool quiet)
{
	if (!quiet)
	{
		comm->sendStatus(text);
	}
}

UINT MasterManager::determineVariationNumber( std::vector<parameterType> variables )
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


bool MasterManager::handleFunctionCall( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
										DioSystem* ttls, AoSystem* aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades, 
										std::vector<UINT>& dacShades, RhodeSchwarz* rsg, UINT seqNum, std::string& warnings,
										std::string callingFunction )
{
	if ( word != "call" )
	{
		return false;
	}
	// calling a user-defined function. Get the name and the arguments to pass to the function handler.
	std::string functionCall, functionName, functionArgs;
	functionCall = stream.getline( '\r' );
	int pos = functionCall.find_first_of( "(" ) + 1;
	int finalpos2 = functionCall.find_last_of( ")" );
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
		analyzeFunction( functionName, args, ttls, aoSys, ttlShades, dacShades, rsg, vars, seqNum, warnings );
	}
	catch ( Error& )
	{
		throwNested( "Error handling Function call to function " + functionName + "." );
	}
	return true;
}