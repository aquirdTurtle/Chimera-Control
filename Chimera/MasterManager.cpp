#include "stdafx.h"
#include "MasterManager.h"
#include "nidaqmx2.h"
#include <fstream>
#include "DioSystem.h"
#include "DacSystem.h"
#include "constants.h"
#include "AuxiliaryWindow.h"
#include "NiawgWaiter.h"
#include "Expression.h"

MasterManager::MasterManager()
{
	functionsFolderLocation = FUNCTIONS_FOLDER_LOCATION;
}


bool MasterManager::getAbortStatus()
{
	return isAborting;
}


/*
 * The workhorse of actually running experiments. This thread procedure analyzes all of the GUI settings and current 
 * configuration settings to determine how to program and run the experiment.
 * @param voidInput: This is the only input to the procedure. It MUST be a pointer to a ExperimentThreadInput structure.
 * @return UINT: The return value is not used, i just return TRUE.
 */
UINT __cdecl MasterManager::experimentThreadProcedure( void* voidInput )
{
	/// initialize various structures
	// convert the input to the correct structure.
	MasterThreadInput* input = (MasterThreadInput*)voidInput;
	// change the status of the parent object to reflect that the thread is running.
	input->thisObj->experimentIsRunning = true;
	// warnings will be passed by reference to a series of function calls which can append warnings to the string.
	// at a certain point the string will get outputted to the error console. Remember, errors themselves are handled 
	// by thrower() calls.
	std::string warnings;
	std::string abortString = "\r\nABORTED!\r\n";
	std::chrono::time_point<chronoClock> startTime( chronoClock::now( ) );
	std::vector<long> variedMixedSize;
	niawgPair<std::vector<std::fstream>> niawgFiles;
	NiawgOutput output;
	std::vector<ViChar> userScriptSubmit;
	output.isDefault = false;
	// initialize to 2 because of default waveforms. This can probably be changed to 1, since only one default waveform
	// now, but might cause temporary breakages.
	output.waves.resize( 2 );
	std::vector<std::pair<UINT, UINT>> ttlShadeLocs;
	std::vector<UINT> dacShadeLocs;
	bool foundRearrangement = false;
	/// ////////////////////////////
	/// start analysis & experiment
	try
	{
		UINT variations = determineVariationNumber( input->variables );
		// finishing sentence from before start I think...
		expUpdate( "Done.\r\n", input->comm, input->quiet );
		/// Prep agilents
		expUpdate( "Loading Agilent Info...", input->comm, input->quiet );
		for (auto agilent : input->agilents)
		{
			RunInfo dum;
			agilent->handleInput( input->profile.categoryPath, dum );
		}
		/// prep master systems
		expUpdate( "Analyzing Master Script...", input->comm, input->quiet );
		input->dacs->resetDacEvents();
		input->ttls->resetTtlEvents();
		input->rsg->clearFrequencies();
		if (input->runMaster)
		{
			input->thisObj->analyzeMasterScript( input->ttls, input->dacs, ttlShadeLocs, dacShadeLocs, input->rsg,
												 input->variables );
		}
		/// prep NIAWG
		if (input->runNiawg)
		{
			input->niawg->prepareNiawg(  input, output, niawgFiles, warnings, userScriptSubmit, foundRearrangement, 
											input->rearrangeInfo, input->variables );
			input->niawg->writeStaticNiawg( output, input->debugOptions, input->constants );
		}
		if ( input->thisObj->isAborting )
		{
			thrower( abortString );
		}
		/// update ttl and dac looks & interaction based on which ones are used in the experiment.
		if (input->runMaster)
		{
			input->ttls->shadeTTLs( ttlShadeLocs );
			input->dacs->shadeDacs( dacShadeLocs );
		}
		// go ahead and check if abort was pressed real fast...
		if (input->thisObj->isAborting)
		{
			thrower( abortString );
		}
		/// The Key Interpretation step.
		// at this point, all scripts have been analyzed, and each system takes the key and generates all of the data
		// it needs for each variation of the experiment. All these calculations happen at this step.
		expUpdate( "Programming All Variation Data...\r\n", input->comm, input->quiet );
		std::chrono::time_point<chronoClock> varProgramStartTime( chronoClock::now( ) );
		input->thisObj->loadSkipTimes.resize( variations );
		if (input->runMaster)
		{
			input->ttls->interpretKey( input->variables );
			input->dacs->interpretKey( input->variables, warnings );
		}
		input->rsg->interpretKey( input->variables );
		input->topBottomTek->interpretKey( input->variables );
		input->eoAxialTek->interpretKey( input->variables );
		/// organize commands, prepping final forms of the data for each repetition.
		// This must be done after the "interpret key" step, as before that commands don't have hard times attached to 
		// them.
		for (UINT variationInc = 0; variationInc < variations; variationInc++)
		{
			// reading these variables should be safe.
			if (input->thisObj->isAborting)
			{
				thrower( abortString );
			}
			if (input->runMaster)
			{
				double& currLoadSkipTime = input->thisObj->loadSkipTimes[variationInc];
				currLoadSkipTime = MasterManager::convertToTime( input->thisObj->loadSkipTime, input->variables,
																 variationInc );
				// organize & format the ttl and dac commands
				input->dacs->organizeDacCommands( variationInc );
				input->dacs->setDacTriggerEvents( input->ttls, variationInc );
				input->dacs->findLoadSkipSnapshots( currLoadSkipTime, input->variables, variationInc );
				input->dacs->makeFinalDataFormat( variationInc );
				input->ttls->organizeTtlCommands( variationInc );
				input->ttls->findLoadSkipSnapshots( currLoadSkipTime, input->variables, variationInc );
				input->ttls->convertToFinalFormat( variationInc );
				// run a couple checks.
				input->ttls->checkNotTooManyTimes( variationInc );
				input->ttls->checkFinalFormatTimes( variationInc );
				if (input->ttls->countDacTriggers( variationInc ) != input->dacs->getNumberSnapshots( variationInc ))
				{
					thrower( "ERROR: number of dac triggers from the ttl system does not match the number of dac snapshots!"
							 " Number of dac triggers was " + str( input->ttls->countDacTriggers( variationInc ) ) + " while number of dac "
							 "snapshots was " + str( input->dacs->getNumberSnapshots( variationInc ) ) );
				}
				input->dacs->checkTimingsWork( variationInc );
				if ( input->runNiawg )
				{
					if ( input->ttls->countTriggers( input->niawg->getTrigLines( ).first,
													 input->niawg->getTrigLines( ).second, variationInc ) !=
						 input->niawg->getNumberTrigsInScript( ) )
					{
						warnings += "WARNING: NIAWG is not getting triggered by the ttl system the same number of times a"
							" trigger command appears in the NIAWG script.";
					}
				}
			}
			input->rsg->orderEvents( variationInc );
		}
		/// output some timing information
		std::chrono::time_point<chronoClock> varProgramEndTime( chronoClock::now( ) );
		expUpdate( "Programming took " 
				   + str( std::chrono::duration<double>( (varProgramEndTime - varProgramStartTime) ).count( ) / 1000.0 )
				   + " seconds.\r\n", input->comm, input->quiet );
		if (input->runMaster)
		{
			expUpdate( "Programmed time per repetition: " + str( input->ttls->getTotalTime( 0 ) ) + "\r\n", 
					   input->comm, input->quiet );
			ULONGLONG totalTime = 0;
			for (USHORT variationNumber = 0; variationNumber < variations; variationNumber++)
			{
				totalTime += ULONGLONG(input->ttls->getTotalTime( variationNumber ) * input->repetitionNumber);
			}
			expUpdate( "Programmed Total Experiment time: " + str( totalTime ) + "\r\n", input->comm, input->quiet );
			expUpdate( "Number of TTL Events in experiment: " + str( input->ttls->getNumberEvents( 0 ) ) + "\r\n", 
					   input->comm, input->quiet );
			expUpdate( "Number of DAC Events in experiment: " + str( input->dacs->getNumberEvents( 0 ) ) + "\r\n", 
					   input->comm, input->quiet );
		}
		/// finish up
		handleDebugPlots( input->debugOptions, input->comm, input->ttls, input->dacs, input->quiet, input->python );
		input->comm->sendError( warnings );
		// update the colors of the global variable control.
		input->globalControl->setUsages( input->variables );
		/// /////////////////////////////
		/// Begin experiment loop
		/// //////////
		// TODO: Handle randomizing repetitions. The thread will need to split into separate if/else statements here.
		if (input->runMaster)
		{
			input->comm->sendColorBox( Master, 'G' );
		}
		// loop for variations
		for (const UINT& variationInc : range( variations ))
		{
			expUpdate( "Variation #" + str( variationInc + 1 ) + "\r\n", input->comm, input->quiet );
			Sleep( input->debugOptions.sleepTime );
			for (auto tempVariable : input->variables)
			{
				// if varies...
				if (tempVariable.valuesVary)
				{
					if (tempVariable.keyValues.size() == 0)
					{
						thrower( "ERROR: Variable " + tempVariable.name + " varies, but has no values assigned to it!" );
					}
					expUpdate( tempVariable.name + ": " + str( tempVariable.keyValues[variationInc], 12) + "\r\n", 
							   input->comm, input->quiet );
				}
			}
			expUpdate( "Programming RSG, Agilents, NIAWG, & Teltronics...\r\n", input->comm, input->quiet );
			input->rsg->programRsg( variationInc );
			input->rsg->setInfoDisp( variationInc );
			// program devices
			for (auto& agilent : input->agilents)
			{
				input->comm->sendColorBox( Intensity, 'Y' );
				agilent->setAgilent( variationInc, input->variables );
				input->comm->sendColorBox( Intensity, 'G' );
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
					UINT ttlTrigs;
					if ( input->runMaster )
					{
						ttlTrigs = input->ttls->countTriggers( agilent->getTriggerLine( ).first,
																 agilent->getTriggerLine( ).second, variationInc );
					}
					else
					{
						ttlTrigs = 0;
					}					 
					UINT agilentExpectedTrigs = agilent->getOutputInfo( ).channel[chan].scriptedArb.wave.getNumTrigs( );
					if ( ttlTrigs != agilentExpectedTrigs )
					{
						warnings += "WARNING: Agilent " + agilent->getName( ) + " is not getting triggered by the "
							"ttl system the same number of times a trigger command appears in the agilent channel "
							+ str( chan + 1 ) + " script. There are " + str( agilentExpectedTrigs ) + " triggers in"
							" the agilent script, and " + str( ttlTrigs ) + " ttl triggers sent to that agilent.";
					}
				}
			}
			if (input->runNiawg)
			{
				input->niawg->programNiawg( input, output, warnings, variationInc, variations, variedMixedSize,
											userScriptSubmit );
				if ( foundRearrangement )
				{
					input->niawg->turnOffRerng( );
					input->conditionVariableForRearrangement->notify_all( );
					input->niawg->handleStartingRerng( input, output );
				}
			}
			input->comm->sendError( warnings );
			input->topBottomTek->programMachine( variationInc );
			input->eoAxialTek->programMachine( variationInc );
			//
			input->comm->sendRepProgress( 0 );
			expUpdate( "Running Experiment.\r\n", input->comm, input->quiet );
			for (UINT repInc = 0; repInc < input->repetitionNumber; repInc++)
			{
				if (input->thisObj->isAborting)
				{
					thrower( abortString );
				}
				else if (input->thisObj->isPaused)
				{
					expUpdate( "Paused\r\n!", input->comm, input->quiet );
					// wait...
					while (input->thisObj->isPaused)
					{
						// this could be changed to be a bit smarter using a std::condition_variable
						Sleep( 100 );
					}
					expUpdate( "Un-Paused!\r\n", input->comm, input->quiet );
				}
				input->comm->sendRepProgress( repInc + 1 );
				// this was re-written each time from looking at the VB6 code.
				if (input->runMaster)
				{
					input->dacs->stopDacs();
					// it's important to grab the skipoption from input->skipNext only once because in principle
					// if the cruncher thread was running behind, it could change between writing and configuring the 
					// dacs and configuring the TTLs;
					bool skipOption;
					if ( input->skipNext == NULL )
					{
						skipOption = false;
					}
					else
					{
						skipOption = input->skipNext->load( );
					}
					input->dacs->configureClocks( variationInc, skipOption );
					input->dacs->writeDacs( variationInc, skipOption );
					input->dacs->startDacs();
					input->ttls->writeTtlData( variationInc, skipOption );
					input->ttls->startBoard();
					input->ttls->waitTillFinished( variationInc, skipOption );
				}
			}
			expUpdate( "\r\n", input->comm, input->quiet );
		}
		/// conclude.
		expUpdate( "\r\nExperiment Finished Normally.\r\n", input->comm, input->quiet );
		input->comm->sendColorBox( Master, 'B' );
		if (input->runMaster)
		{
			// stop is necessary; Else the dac system will still be "running" and won't allow updates through normal 
			// means.
			input->dacs->stopDacs();
			input->dacs->unshadeDacs();
		}
		if ( input->runMaster )
		{
			try
			{
				// make sure the display accurately displays the state that the experiment finished at.
				input->dacs->setDacStatusNoForceOut( input->dacs->getFinalSnapshot( ) );
				input->ttls->unshadeTtls( );
				input->ttls->setTtlStatusNoForceOut( input->ttls->getFinalSnapshot( ) );
			}
			catch ( Error& ) { /* this gets thrown if no dac events. just continue.*/ }
		}
		if (input->runNiawg)
		{
			input->niawg->cleanupNiawg( input->profile, input->runMaster, niawgFiles, output, input->comm,
										   input->settings.dontActuallyGenerate );
		}
		input->comm->sendNormalFinish( );
	}
	catch (Error& exception)
	{
		if (input->runNiawg)
		{
			for (const auto& sequenceInc : range( input->profile.sequenceConfigNames.size() ))
			{
				for (const auto& axis : AXES)
				{
					if (niawgFiles[axis].size() != 0)
					{
						if (niawgFiles[axis][sequenceInc].is_open())
						{
							niawgFiles[axis][sequenceInc].close();
						}
					}
				}
			}
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
			input->dacs->unshadeDacs();
		}
		if ( input->thisObj->isAborting )
		{
			expUpdate( abortString, input->comm, input->quiet );
			input->comm->sendColorBox( Master, 'B' );
		}
		else
		{
			// No quiet option for a bad exit.
			input->comm->sendColorBox( Master, 'R' );
			input->comm->sendStatus( "Bad Exit!\r\n" );
			std::string exceptionTxt = exception.what( );
			input->comm->sendError( exception.what( ) );
			input->comm->sendFatalError( "Exited main experiment thread abnormally." );
		}	
	}
	std::chrono::time_point<chronoClock> endTime( chronoClock::now( ) );
	expUpdate( "Experiment took " + str( std::chrono::duration<double>( (endTime - startTime) ).count( ) ) 
			   + " seconds.\r\n", input->comm, input->quiet );
	input->thisObj->experimentIsRunning = false;
	delete input;
	return false;
}


double MasterManager::convertToTime( timeType time, std::vector<variableType> variables, UINT variation )
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


void MasterManager::handleDebugPlots( debugInfo debugOptions, Communicator* comm, DioSystem* ttls, DacSystem* dacs,
									  bool quiet, EmbeddedPythonHandler* python )
{
	if ( debugOptions.showTtls )
	{
		// output to status
		comm->sendDebug( ttls->getTtlSequenceMessage( 0 ) );
		// output to debug file
		std::ofstream debugFile( cstr( DEBUG_OUTPUT_LOCATION + str( "TTL-Sequence.txt" ) ) );
		if ( debugFile.is_open( ) )
		{
			debugFile << ttls->getTtlSequenceMessage( 0 );
			debugFile.close( );
		}
		else
		{
			expUpdate( "ERROR: Debug text file failed to open! Continuing...\r\n", comm, quiet );
		}
		python->runPlotTtls( );
	}
	if ( debugOptions.showDacs )
	{
		// output to status
		comm->sendDebug( dacs->getDacSequenceMessage( 0 ) );
		// output to debug file.
		std::ofstream  debugFile( cstr( DEBUG_OUTPUT_LOCATION + str( "DAC-Sequence.txt" ) ) );
		if ( debugFile.is_open( ) )
		{
			debugFile << dacs->getDacSequenceMessage( 0 );
			debugFile.close( );
		}
		else
		{
			comm->sendError( "ERROR: Debug text file failed to open! Continuing...\r\n" );
		}

		python->runPlotDacs( );
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
		thrower( "Experiment is Running! Please abort the current run before setting the MOT settings." );
	}

	loadMasterScript(input->masterScriptAddress);
	input->thisObj = this;
	//input->key->loadVariables(input->variables);
	VariableSystem::generateKey( input->variables, false );
	// start thread.
	runningThread = AfxBeginThread(experimentThreadProcedure, input);	
}


void MasterManager::startExperimentThread(MasterThreadInput* input)
{
	if ( !input )
	{
		thrower( "ERROR: Input to start experiment thread was null?!?!?" );
	}
	if ( experimentIsRunning )
	{
		delete input;
		thrower( "Experiment is already Running!  You can only run one experiment at a time! Please abort before "
				 "running again." );
	}
	input->thisObj = this;
	if (input->runMaster)
	{
		loadMasterScript( input->masterScriptAddress );
	}
	// start thread.
	runningThread = AfxBeginThread(experimentThreadProcedure, input, THREAD_PRIORITY_HIGHEST);
}


bool MasterManager::getIsPaused()
{
	return isPaused;
}


void MasterManager::pause()
{
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = true;
}


void MasterManager::unPause()
{
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = false;
}


void MasterManager::abort()
{
	std::lock_guard<std::mutex> locker( abortLock );
	isAborting = true;
	//experimentIsRunning = false;
}


void MasterManager::loadMasterScript(std::string scriptAddress)
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, cstr(scriptAddress), "r" );
	if ( !file )
	{
		thrower("ERROR: Master Script File " + scriptAddress + " does not exist!");
	}
	else
	{
		fclose( file );
	}
	scriptFile.open(cstr(scriptAddress));
	// check opened correctly
	if (!scriptFile.is_open())
	{
		thrower("ERROR: File passed test making sure the file exists, but it still failed to open!");
	}
	// dump the file into the stringstream.
	std::stringstream buf( std::ios_base::app | std::ios_base::out | std::ios_base::in );
	buf << scriptFile.rdbuf();
	// IMPORTANT!
	// always pulses the oscilloscope trigger at the end!
	buf << "\r\n t += 0.1 \r\n pulseon: " + str(OSCILLOSCOPE_TRIGGER) + " 0.5";
	// this is used to more easily deal some of the analysis of the script.
	buf << "\r\n\r\n__END__";
	// for whatever reason, after loading rdbuf into a stringstream, the stream seems to not 
	// want to >> into a string. tried resetting too using seekg, but whatever, this works.
	currentMasterScript.str("");
	currentMasterScript.str( buf.str());
	currentMasterScript.clear();
	currentMasterScript.seekg(0);
	//std::string str(currentMasterScript.str());
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
		thrower("ERROR: Function file in functions folder was not a function because it did not start with \"def\"! Functions must start with this. Instead it"
			" started with " + word);
	}
	std::string functionDeclaration, functionArgumentList;
	functionDeclaration = defStream.getline( ':' );
	int initNamePos = defLine.find_first_not_of(" \t");
	functionName = functionDeclaration.substr(initNamePos, functionDeclaration.find_first_of("(") - initNamePos);

	if (functionName.find_first_of(" ") != std::string::npos)
	{
		thrower("ERROR: Function name included a space!");
	}
	int initPos = functionDeclaration.find_first_of("(");
	if (initPos == std::string::npos)
	{
		thrower("ERROR: No starting parenthesis \"(\" in function definition. Use \"()\" if no arguments.");
	}
	initPos++;
	int endPos = functionDeclaration.find_last_of(")");
	if (endPos == std::string::npos)
	{
		thrower("ERROR: No ending parenthesis \")\" in function definition. Use \"()\" if no arguments.");
	}
	functionArgumentList = functionDeclaration.substr(initPos, endPos - initPos);
	endPos = functionArgumentList.find_first_of(",");
	initPos = functionArgumentList.find_first_not_of(" \t");
	bool good = true;
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
			thrower("ERROR: bad argument list in function. It looks like there might have been a space or tab inside the function argument?");
		}
		if (tempArg == "")
		{
			thrower("ERROR: bad argument list in function. It looks like there might have been a stray \",\"?");
		}
		args.push_back(tempArg);
		endPos = functionArgumentList.find_first_of(",");
		initPos = functionArgumentList.find_first_not_of(" \t");
	}
}


void MasterManager::analyzeFunction( std::string function, std::vector<std::string> args, DioSystem* ttls,
									 DacSystem* dacs, std::vector<std::pair<UINT, UINT>>& ttlShades,
									 std::vector<UINT>& dacShades, RhodeSchwarz* rsg, std::vector<variableType>& vars)
{
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, cstr(FUNCTIONS_FOLDER_LOCATION + function + "." + FUNCTION_EXTENSION), "r" );
	if ( !file )
	{
		thrower("ERROR: Function " + function + " does not exist!");
	}
	else
	{
		fclose( file );
	}
	functionFile.open(FUNCTIONS_FOLDER_LOCATION + function + "." + FUNCTION_EXTENSION, std::ios::in);
	// check opened correctly
	if (!functionFile.is_open())
	{
		thrower("ERROR: Function file " + function + "File passed test making sure the file exists, but it still failed to open!");
	}
	// append __END__ to the end of the file for analysis purposes.
	std::stringstream buf;
	ScriptStream functionStream;
	buf << functionFile.rdbuf();
	functionStream << buf.str();
	functionStream << "\r\n\r\n__END__";
	functionFile.close();
	// functionStream.loadReplacements()
	if (functionStream.str() == "")
	{
		thrower("ERROR: Function File for " + function + "was empty!");
	}
	std::string word;
	// the following are used for repeat: functionality
	std::vector<ULONG> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	/// get the function arguments.
	std::string defLine;
	defLine = functionStream.getline( ':' );
	std::string name;
	std::vector<std::string> functionArgs;
	analyzeFunctionDefinition( defLine, name, functionArgs );
	if (functionArgs.size() != args.size())
	{
		std::string functionArgsString;
		for (auto elem : args)
		{
			functionArgsString += elem + ",";
		}
		thrower("ERROR: incorrect number of arguments in the call for function " + function + ". Number in call was: "
				 + str(args.size()) + ", number expected was " + str(functionArgs.size()) + ". "
				"Function arguments were:" + functionArgsString + ".");
	}
	std::vector<std::pair<std::string, std::string>> replacements;
	for (UINT replacementInc =0; replacementInc < args.size(); replacementInc++)
	{
		replacements.push_back( { functionArgs[replacementInc], args[replacementInc] } );
	}
	functionStream.loadReplacements( replacements );
	currentFunctionText = functionStream.str();
	//
	functionStream >> word;
	while (!(functionStream.peek() == EOF) || word != "__end__")
	{
		if ( handleTimeCommands( word, functionStream, vars ))
		{
			// got handled
		}
		/// callcppcode command
		else if (word == "callcppcode")
		{
			// and that's it... 
			callCppCodeFunction();
		}
		/// deal with ttl commands
		else if (word == "on:" || word == "off:")
		{
			std::string name;
			functionStream >> name;
			ttls->handleTtlScriptCommand( word, operationTime, name, ttlShades, vars );
		}
		else if (word == "pulseon:" || word == "pulseoff:")
		{
			// this requires handling time as it is handled above.
			std::string name;
			Expression pulseLength;
			functionStream >> name;
			functionStream >> pulseLength;
			// should be good to go.
			ttls->handleTtlScriptCommand( word, operationTime, name, pulseLength, ttlShades, vars );
		}

		/// deal with dac commands
		else if (word == "dac:")
		{
			DacCommandForm command;
			std::string name;
			functionStream >> name;
			functionStream >> command.finalVal;
			command.finalVal.assertValid( vars );
			command.time = operationTime;
			command.commandName = "dac:";
			command.initVal.expressionStr = "__NONE__";
			command.numSteps.expressionStr = "__NONE__";
			command.rampInc.expressionStr = "__NONE__";
			command.rampTime.expressionStr = "__NONE__";
			try
			{
				dacs->handleDacScriptCommand(command,  name, dacShades, vars, ttls);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dac:\" command inside function " + function);
			}
		}
		else if ( word == "daclinspace:" )
		{
			DacCommandForm command;
			std::string name;
			// get dac name
			functionStream >> name;
			// get ramp initial value
			functionStream >> command.initVal;
			command.initVal.assertValid( vars );
			// get ramp final value
			functionStream >> command.finalVal;
			command.finalVal.assertValid( vars );
			// get total ramp time;
			functionStream >> command.rampTime;
			command.rampTime.assertValid( vars );
			// get ramp point increment.
			functionStream >> command.numSteps;
			command.numSteps.assertValid( vars );
			command.time = operationTime;
			command.commandName = "daclinspace:";
			// not used here.
			command.rampInc.expressionStr = "__NONE__";
			//
			try
			{
				dacs->handleDacScriptCommand( command, name, dacShades, vars, ttls );
			}
			catch ( Error& err )
			{
				thrower( err.whatStr( ) + "... in \"dacLinSpace:\" command inside function " + function );
			}
		}
		else if (word == "dacarange:")
		{
			DacCommandForm command;
			std::string name;
			// get dac name
			functionStream >> name;
			// get ramp initial value
			functionStream >> command.initVal;
			command.initVal.assertValid( vars );
			// get ramp final value
			functionStream >> command.finalVal;
			command.finalVal.assertValid( vars );
			// get total ramp time;
			functionStream >> command.rampTime;
			command.rampTime.assertValid( vars );
			// get ramp point increment.
			functionStream >> command.rampInc;
			command.rampInc.assertValid( vars );
			command.time = operationTime;
			command.commandName = "dacarange:";
			// not used here.
			command.numSteps.expressionStr = "__NONE__";
			//
			try
			{
				dacs->handleDacScriptCommand(command, name, dacShades, vars, ttls);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dacArange:\" command inside function " + function);
			}
		}
		/// Handle RSG calls.
		else if (word == "rsg:")
		{
			rsgEventForm info;
			functionStream >> info.frequency;
			info.frequency.assertValid( vars );
			functionStream >> info.power;
			info.power.assertValid( vars );
			// test frequency
			info.time = operationTime;
			rsg->addFrequency( info );
		}
		/// deal with function calls.
		else if (word == "call")
		{
			// calling a user-defined function. Get the name and the arguments to pass to the function handler.
			std::string functionCall, functionName, functionInputArgs;
			functionCall = functionStream.getline( '\r' );
			int pos = functionCall.find_first_of("(") + 1;
			int finalpos2 = functionCall.find_last_of(")");
			int finalpos = functionCall.find_last_of(")");
			functionName = functionCall.substr(0, pos - 1);
			functionInputArgs = functionCall.substr(pos, finalpos - pos);
			std::string arg;
			std::vector<std::string> newArgs;
			while (true)
			{
				pos = functionInputArgs.find_first_of(',');
				if (pos == std::string::npos)
				{
					arg = functionInputArgs.substr(0, functionInputArgs.size());
					if (arg != "")
					{
						newArgs.push_back(arg);
					}
					break;
				}
				arg = functionInputArgs.substr(0, pos);
				if (arg != "")
				{
					newArgs.push_back(arg);
				}
				// cut out that argument off the string.
				functionInputArgs = functionInputArgs.substr(pos, functionInputArgs.size());
			}
			if (functionName == function)
			{
				thrower( "ERROR: Recursive function call detected! " + function + " called itself! This is not allowed." );
			}
			try
			{
				analyzeFunction(functionName, newArgs, ttls, dacs, ttlShades, dacShades, rsg, vars);
			}
			catch (Error& err)
			{
				// the fact that each function call will re-throw with this will end up putting the whole function call
				// stack onto the error message.
				thrower(err.whatStr() + "... In function call to function " + functionName + "\r\n");
			}
		}
		else if ( word == "repeat:" )
		{
			std::string repeatStr;
			functionStream >> repeatStr;
			try
			{
				totalRepeatNum.push_back( std::stoi( repeatStr ) );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: the repeat number failed to convert to an integer! Note that the repeat number can not"
						 " currently be a variable." );
			}
			repeatPos.push_back( functionStream.tellg() );
			currentRepeatNum.push_back(1);
		}
		else if ( word == "end" )
		{
			if (currentRepeatNum.size() == 0)
			{
				thrower( "ERROR: mismatched \"end\" command for repeat loop! there were more \"end\" commands than \"repeat\" commands." );
			}
			if ( currentRepeatNum.back() < totalRepeatNum.back() )
			{
				functionStream.seekg( repeatPos.back() );
				currentRepeatNum.back()++;
			}
			else
			{
				// remove the entries corresponding to this repeat loop.
				currentRepeatNum.pop_back();
				repeatPos.pop_back();
				totalRepeatNum.pop_back();				
				// and continue (no seekg)
			}
		}
		else
		{
			thrower("ERROR: unrecognized master script command: " + word);
		}
		functionStream >> word;
	}
}

// if it handled it, returns true, else returns false.
bool MasterManager::handleTimeCommands( std::string word, ScriptStream& stream, std::vector<variableType>& vars )
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
			time.assertValid( vars );
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
		catch ( std::invalid_argument & )
		{
			time.assertValid( vars );
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


void MasterManager::analyzeMasterScript( DioSystem* ttls, DacSystem* dacs,
										 std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
										 RhodeSchwarz* rsg, std::vector<variableType>& vars)
{
	// reset some things.
	loadSkipTime.first.clear( );
	loadSkipTime.second = 0;
	loadSkipTimes.clear( );
	currentMasterScriptText = currentMasterScript.str();
	// starts at 0.1 if not initialized by the user.
	operationTime.second = 0.1;
	operationTime.first.clear();
	if (currentMasterScript.str() == "")
	{
		thrower("ERROR: Master script is empty!\r\n");
	}
	std::string word;
	currentMasterScript >> word;
	std::vector<UINT> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	// the analysis loop.
	while (!(currentMasterScript.peek() == EOF) || word != "__end__")
	{
		//std::stringstream individualCommandStream;
		if (handleTimeCommands(word, currentMasterScript, vars ) )
		{
			// got handled.
		}
		/// callcppcode function
		else if (word == "callcppcode")
		{			
			// and that's it... 
			callCppCodeFunction();
		}
		/// deal with ttl commands
		else if ( word == "loadskipentrypoint!" )
		{
			loadSkipTime = operationTime;
		}
		else if (word == "on:" || word == "off:")
		{
			std::string name;
			currentMasterScript >> name;
			ttls->handleTtlScriptCommand( word, operationTime, name, ttlShades, vars );
		}
		else if (word == "pulseon:" || word == "pulseoff:")
		{
			// this requires handling time as it is handled above.
			std::string name;
			Expression pulseLength;
			currentMasterScript >> name;
			currentMasterScript >> pulseLength;
			pulseLength.assertValid( vars );
			// should be good to go.
			ttls->handleTtlScriptCommand( word, operationTime, name, pulseLength, ttlShades, vars );
		}

		/// deal with dac commands
		else if (word == "dac:")
		{
			DacCommandForm command;
			std::string name;
			currentMasterScript >> name;
			std::string value;
			currentMasterScript >> command.finalVal;
			command.finalVal.assertValid( vars );
			command.time = operationTime;
			command.commandName = "dac:";
			command.initVal.expressionStr = "__NONE__";
			command.numSteps.expressionStr = "__NONE__";
			command.rampInc.expressionStr = "__NONE__";
			command.rampTime.expressionStr = "__NONE__";
			try
			{
				dacs->handleDacScriptCommand(command, name, dacShades, vars, ttls);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dacArange:\" command inside main script");
			}
		}
		else if ( word == "daclinspace:" )
		{
			DacCommandForm command;
			std::string name;
			currentMasterScript >> name;
			currentMasterScript >> command.initVal;
			command.initVal.assertValid( vars );
			currentMasterScript >> command.finalVal;
			command.finalVal.assertValid( vars );
			currentMasterScript >> command.rampTime;
			command.rampTime.assertValid( vars );
			currentMasterScript >> command.numSteps;
			command.numSteps.assertValid( vars );
			command.time = operationTime;
			command.commandName = "daclinspace:";
			// not used here.
			command.rampInc.expressionStr = "__NONE__";
			//
			try
			{
				dacs->handleDacScriptCommand( command, name, dacShades, vars, ttls );
			}
			catch ( Error& err )
			{
				thrower( err.whatStr( ) + "... in \"dacLinSpace:\" command inside main script.\r\n" );
			}
		}
		else if (word == "dacarange:")
		{
			DacCommandForm command;
			std::string name;
			currentMasterScript >> name;
			currentMasterScript >> command.initVal;
			command.initVal.assertValid( vars );
			currentMasterScript >> command.finalVal;
			command.finalVal.assertValid( vars );
			currentMasterScript >> command.rampTime;
			command.rampTime.assertValid( vars );
			currentMasterScript >> command.rampInc;
			command.rampInc.assertValid( vars );
			command.time = operationTime;
			command.commandName = "dacarange:";
			// not used here.
			command.numSteps.expressionStr = "__NONE__";
			try
			{
				dacs->handleDacScriptCommand( command, name, dacShades, vars, ttls );
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dacArange:\" command inside main script");
			}
		}
		/// Deal with RSG calls
		else if (word == "rsg:")
		{
			rsgEventForm info;
			currentMasterScript >> info.frequency;
			info.frequency.assertValid( vars );
			currentMasterScript >> info.power;
			info.power.assertValid( vars );
			info.time = operationTime;
			rsg->addFrequency( info );
		}
		/// deal with raman beam calls (setting raman frequency).
		/// deal with function calls.
		else if (word == "call")
		{
			// calling a user-defined function. Get the name and the arguments to pass to the function handler.
			std::string functionCall, functionName, functionArgs;
			functionCall = currentMasterScript.getline( '\r' );
			int pos = functionCall.find_first_of("(") + 1;
			int finalpos2 = functionCall.find_last_of(")");
			int finalpos = functionCall.find_last_of(")");
			
			functionName = functionCall.substr(0, pos - 1);
			functionArgs = functionCall.substr(pos, finalpos - pos);
			std::string arg;
			std::vector<std::string> args;
			while (true)
			{
				pos = functionArgs.find_first_of(',');
				if (pos == std::string::npos)
				{
					arg = functionArgs.substr(0, functionArgs.size());
					if ( arg != "" )
					{
						args.push_back( arg );
					}
					break;
				}
				arg = functionArgs.substr(0, pos);
				args.push_back(arg);
				// cut oinputut that argument off the string.
				functionArgs = functionArgs.substr(pos + 1, functionArgs.size());
			}
			try
			{
				analyzeFunction(functionName, args, ttls, dacs, ttlShades, dacShades, rsg, vars);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... In Function call to function " + functionName);
			}
		}
		else if (word == "repeat:")
		{
			Expression repeatStr;
			currentMasterScript >> repeatStr;
			try
			{
				totalRepeatNum.push_back( repeatStr.evaluate( ) );
			}
			catch (Error&)
			{
				thrower("ERROR: the repeat number failed to convert to an integer! Note that the repeat number can not"
						 " currently be a variable.");
			}
			repeatPos.push_back( currentMasterScript.tellg() );
			currentRepeatNum.push_back(1);
		}
		// (look for end of repeat)
		else if (word == "end")
		{
			if (currentRepeatNum.size() == 0)
			{
				thrower("ERROR! Tried to end repeat, but you weren't repeating!");
			}
			if (currentRepeatNum.back() < totalRepeatNum.back())
			{
				currentMasterScript.seekg(repeatPos.back());
				currentRepeatNum.back()++;
			}
			else
			{
				currentRepeatNum.pop_back();
				repeatPos.pop_back();
				totalRepeatNum.pop_back();
			}
		}
		else
		{
			thrower("ERROR: unrecognized master script command: \"" + word + "\"");
		}
		word = "";
		currentMasterScript >> word;
	}
}


std::string MasterManager::getErrorMessage(int errorCode)
{
	char* errorChars;
	long bufferSize;
	long status;
	//Find out the error message length.
	bufferSize = DAQmxGetErrorString(errorCode, 0, 0);
	//Allocate enough space in the string.
	errorChars = new char[bufferSize];
	//Get the actual error message.
	status = DAQmxGetErrorString(errorCode, errorChars, bufferSize);
	std::string errorString(errorChars);
	delete errorChars;
	return errorString;
}


/*
	this function can be called directly from scripts. Insert things inside the function to make it do something
	custom that's not possible inside the scripting language.
*/
void MasterManager::callCppCodeFunction()
{
	
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

UINT MasterManager::determineVariationNumber( std::vector<variableType> variables )
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
