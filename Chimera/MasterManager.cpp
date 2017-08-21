#include "stdafx.h"
#include "MasterManager.h"
#include "nidaqmx2.h"
#include <fstream>
#include "TtlSystem.h"
#include "DacSystem.h"
#include "constants.h"
#include "AuxiliaryWindow.h"
#include "NiawgWaiter.h"

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
	// convert the input to the correct structure.
	MasterThreadInput* input = (MasterThreadInput*)voidInput;
	// change the status of the parent object to reflect that the thread is running.
	input->thisObj->experimentIsRunning = true;
	// warnings will be passed by reference to a series of function calls which can append warnings to the string.
	// at a certain point the string will get outputted to the error console. Remember, errors themselves are handled 
	// by thrower() calls.
	std::string warnings;
	ULONGLONG startTime = GetTickCount();
	std::vector<long> variedMixedSize;
	bool intIsVaried = false;
	std::vector<std::pair<double, double>> intensityRanges;
	std::vector<std::fstream> intensityScriptFiles;
	niawgPair<std::vector<std::fstream>> niawgFiles;
	NiawgWaiter waiter;
	NiawgOutputInfo output;
	std::vector<ViChar> userScriptSubmit;
	output.isDefault = false;
	// initialize to 2 because of default waveforms...
	output.waveCount = 2;
	output.predefinedWaveCount = 0;
	output.waves.resize( 2 );

	std::vector<std::pair<UINT, UINT>> ttlShadeLocs;
	std::vector<UINT> dacShadeLocs;
	
	//
	try
	{
		UINT variations = determineVariationNumber( input->variables, input->key->getKey() );
		expUpdate( "Done.\r\n", input->comm, input->quiet );
		// Prep agilents
		expUpdate( "Loading Agilent Info...", input->comm, input->quiet );
		for (auto agilent : input->agilents)
		{
			std::vector<std::ofstream> dummyFiles;
			agilent->handleInput( 0 );
			agilent->handleInput( 1 );
		}
		//
		expUpdate( "Analyzing Master Script...", input->comm, input->quiet );
		input->dacs->resetDacEvents();
		input->ttls->resetTtlEvents();
		input->rsg->clearFrequencies();
		input->thisObj->loadVariables( input->variables );
		if (input->runMaster)
		{
			input->thisObj->analyzeMasterScript( input->ttls, input->dacs, ttlShadeLocs, dacShadeLocs, input->rsg,
												 input->variables );
		}
		/// 
		// a relic from the niawg thread. I Should re-organize how I get the niawg files and intensity script files to
		// be consistent with the master script file.
		if (input->programIntensity || input->runNiawg)
		{
			ProfileSystem::getConfigInfo( niawgFiles, intensityScriptFiles, input->profile, input->programIntensity,
										  input->runNiawg );
		}
		if (input->programIntensity)
		{
			input->comm->sendStatus( "Programing Intensity Profile(s)..." );
			input->comm->sendColorBox( Intensity, 'Y' );
			/*
			input->intensityAgilent->programIntensity( input->key->getKey(), intIsVaried, intensityRanges, 
													   intensityScriptFiles, variations );
			*/
			expUpdate( "Done.\r\n", input->comm, input->quiet );
			input->comm->sendColorBox( Intensity, 'G' );
			input->comm->sendStatus( "Intensity Profile Selected.\r\n" );
		}
		if (input->runNiawg)
		{
			input->comm->sendColorBox( Niawg, 'Y' );
			input->niawg->prepareNiawg( input, output, niawgFiles, warnings, userScriptSubmit );
			// check if any waveforms are rearrangement instructions.
			bool foundRearrangement = false;
			for (auto& wave : output.waves)
			{
				if (wave.isRearrangement)
				{
					// if already found one...
					if (foundRearrangement)
					{
						thrower( "ERROR: Multiple rearrangement waveforms not allowed!" );
					}
					foundRearrangement = true;
					// start rearrangement thread. Give the thread the queue.
					input->niawg->startRearrangementThread( input->atomQueueForRearrangement, wave );
				}
			}
			if (input->rearrangingAtoms && !foundRearrangement )
			{
				thrower( "ERROR: system is primed for rearranging atoms, but no rearrangement waveform was found!" );
			}
			else if (!input->rearrangingAtoms && foundRearrangement)
			{
				thrower( "ERROR: System was not primed for rearrangign atoms, but a rearrangement waveform was found!" );
			}
		}
		// update ttl and dac looks & interaction based on which ones are used in the experiment.
		if (input->runMaster)
		{
			input->ttls->shadeTTLs( ttlShadeLocs );
			input->dacs->shadeDacs( dacShadeLocs );
		}
		// go ahead and check if abort was pressed real fast...
		if (input->thisObj->isAborting)
		{
			thrower( "\r\nABORTED!\r\n" );
		}
		// must interpret key before setting the trigger events.
		if (input->runMaster)
		{
			input->ttls->interpretKey( input->key->getKey(), input->variables );
			input->dacs->interpretKey( input->key->getKey(), input->variables, warnings );
		}
		input->rsg->interpretKey( input->key->getKey(), input->variables );
		input->topBottomTek->interpretKey( input->key->getKey(), input->variables );
		input->eoAxialTek->interpretKey( input->key->getKey(), input->variables );
		ULONGLONG varProgramStartTime = GetTickCount();
		expUpdate( "Programming All Variation Data...\r\n", input->comm, input->quiet );
		for (UINT varInc = 0; varInc < variations; varInc++)
		{
			// reading these variables should be safe.
			if (input->thisObj->isAborting)
			{
				thrower( "\r\nABORTED!\r\n" );
			}
			expUpdate( "Programming Variation #" + str( varInc + 1 ) + " Data...\r\n", input->comm, input->quiet );
			if (input->runMaster)
			{
				input->dacs->analyzeDacCommands( varInc );
				input->dacs->setDacTriggerEvents( input->ttls, varInc );
				// prepare dac and ttls. data to final forms.
				input->dacs->makeFinalDataFormat( varInc );
				input->ttls->analyzeCommandList( varInc );
				input->ttls->convertToFinalFormat( varInc );
				if (input->ttls->countDacTriggers( varInc ) != input->dacs->getNumberSnapshots( varInc ))
				{
					thrower( "ERROR: number of dac triggers from the ttl system does not match the number of dac snapshots!"
							 " Number of dac triggers was " + str( input->ttls->countDacTriggers( varInc ) ) + " while number of dac "
							 "snapshots was " + str( input->dacs->getNumberSnapshots( varInc ) ) );
				}
				input->dacs->checkTimingsWork( varInc );
			}
			input->rsg->orderEvents( varInc );
		}

		ULONGLONG varProgramEndTime = GetTickCount();
		expUpdate( "Programming took " + str( (varProgramEndTime - varProgramStartTime) / 1000.0 ) + " seconds.\r\n",
				   input->comm, input->quiet );
		if (input->runMaster)
		{
			expUpdate( "Programmed time per repetition: " + str( input->ttls->getTotalTime( 0 ) ) + "\r\n",
					   input->comm, input->quiet );
		ULONGLONG totalTime = 0;
		for (USHORT var = 0; var < variations; var++)
		{
			totalTime += input->ttls->getTotalTime( var ) * input->repetitionNumber;
		}
		expUpdate( "Programmed Total Experiment time: " + str( totalTime ) + "\r\n", input->comm, input->quiet );
		expUpdate( "Number of TTL Events in experiment: " + str( input->ttls->getNumberEvents( 0 ) ) + "\r\n", input->comm,
				   input->quiet );
		expUpdate( "Number of DAC Events in experiment: " + str( input->dacs->getNumberEvents( 0 ) ) + "\r\n", input->comm,
				   input->quiet );
		}

		if (input->debugOptions.showTtls)
		{
			// output to status
			input->comm->sendStatus( input->ttls->getTtlSequenceMessage( 0 ) );
			// output to debug file
			std::ofstream debugFile( cstr( DEBUG_OUTPUT_LOCATION + str( "TTL-Sequence.txt" ) ), std::ios_base::app );
			if (debugFile.is_open())
			{
				debugFile << input->ttls->getTtlSequenceMessage( 0 );
				debugFile.close();
			}
			else
			{
				expUpdate( "ERROR: Debug text file failed to open! Continuing...\r\n", input->comm, input->quiet );
			}
		}
		if (input->debugOptions.showDacs)
		{
			// output to status
			input->comm->sendStatus( input->dacs->getDacSequenceMessage( 0 ) );
			// output to debug file.
			std::ofstream  debugFile( cstr( DEBUG_OUTPUT_LOCATION + str( "DAC-Sequence.txt" ) ),
									  std::ios_base::app );
			if (debugFile.is_open())
			{
				debugFile << input->dacs->getDacSequenceMessage( 0 );
				debugFile.close();
			}
			else
			{
				input->comm->sendError( "ERROR: Debug text file failed to open! Continuing...\r\n" );
			}
		}

		input->globalControl->setUsages( input->variables );
		// no quiet on warnings.
		expUpdate( warnings, input->comm );
		input->comm->sendDebug( input->debugOptions.message );

		/// /////////////////////////////
		/// Begin experiment loop
		/// //////////
		// loop for variations
		// TODO: Handle randomizing repetitions. This will need to split off here.
		if (input->runMaster)
		{
			input->comm->sendColorBox( Master, 'G' );
		}
		for (const UINT& varInc : range( variations ))
		{
			expUpdate( "Variation #" + str( varInc + 1 ) + "\r\n", input->comm, input->quiet );
			Sleep( input->debugOptions.sleepTime );
			for (auto var : input->key->getKey())
			{
				// if varies...
				if (var.second.second)
				{
					if (var.second.first.size() == 0)
					{
						thrower( "ERROR: Variable " + var.first + " varies, but has no values assigned to it!" );
					}
					expUpdate( var.first + ": " + str( var.second.first[varInc] ) + "\r\n", input->comm,
							   input->quiet );
				}
			}
			expUpdate( "Programming Hardware...\r\n", input->comm, input->quiet );
			input->rsg->programRSG( input->gpib, varInc );
			input->rsg->setInfoDisp( varInc );
			// program devices
			for (auto& agilent : input->agilents)
			{
				agilent->setAgilent( input->key->getKey(), varInc );
			}

			// program the intensity agilent. Right now this is a little different than the above because I haven't 
			// implemented scripting in the generic agilent control, and this is left-over from the intensity control
			// in the Niawg-Only program.
			if (input->programIntensity)
			{
				input->comm->sendColorBox( Intensity, 'Y' );
				//input->intensityAgilent->setScriptOutput( varInc, intensityRanges );
				if (intIsVaried)
				{
					input->comm->sendStatus( "Intensity Profile Selected.\r\n" );
				}
				input->comm->sendColorBox( Intensity, 'G' );
			}
			if (input->runNiawg)
			{
				input->comm->sendColorBox( Niawg, 'Y' );
				input->niawg->programNiawg( input, output, waiter, warnings, varInc, variations, variedMixedSize,
											userScriptSubmit );
				input->comm->sendColorBox( Niawg, 'G' );
			}

			input->topBottomTek->programMachine( varInc );
			input->eoAxialTek->programMachine( varInc );
			//
			input->comm->sendRepProgress( 0 );
			expUpdate( "Running Experiment.\r\n", input->comm, input->quiet );
			for (UINT repInc = 0; repInc < input->repetitionNumber; repInc++)
			{
				// reading these variables should be safe.
				if (input->thisObj->isAborting)
				{
					thrower( "\r\nABORTED!\r\n" );
				}
				else if (input->thisObj->isPaused)
				{
					expUpdate( "\r\nPaused!\r\n...", input->comm, input->quiet );
					// wait...
					while (input->thisObj->isPaused)
					{
						// this could be changed to be a bit smarter I think.
						Sleep( 100 );
					}
					expUpdate( "\r\nUn-Paused!\r\n", input->comm, input->quiet );
				}
				input->comm->sendRepProgress( repInc + 1 );
				// this apparently needs to be re-written each time from looking at the VB6 code.
				if (input->runMaster)
				{
					input->dacs->stopDacs();
					input->dacs->configureClocks( varInc );
					input->dacs->writeDacs( varInc );
					input->dacs->startDacs();
					input->ttls->writeData( varInc );
					input->ttls->startBoard();
					// wait until finished.
					input->ttls->waitTillFinished( varInc );
				}
			}
			expUpdate( "\r\n", input->comm, input->quiet );
		}
		// do experiment stuff...
		expUpdate( "\r\nExperiment Finished Normally.\r\n", input->comm, input->quiet );
		input->comm->sendColorBox( Master, 'B' );
		// this is necessary. If not, the dac system will still be "running" and won't allow updates through normal 
		// means.
		if (input->runMaster)
		{
			input->dacs->stopDacs();
			input->dacs->unshadeDacs();
		}
		// make sure the display accurately displays the state that the experiment finished at.
		try
		{
			input->dacs->setDacStatusNoForceOut( input->dacs->getFinalSnapshot() );
		}
		catch (Error&) { /* this gets thrown if no dac events. just continue.*/ }
		if (input->runMaster)
		{
			input->ttls->unshadeTtls();
			input->ttls->setTtlStatusNoForceOut( input->ttls->getFinalSnapshot() );
		}
		///
		///
		///

		///	Cleanup NIAWG stuff (after all generate code)
		// close things
		if (input->runNiawg)
		{
			for (const auto& sequenceInc : range( input->profile.sequenceConfigNames.size() ))
			{
				for (const auto& axis : AXES)
				{
					if (niawgFiles[axis][sequenceInc].is_open())
					{
						niawgFiles[axis][sequenceInc].close();
					}
				}
			}

			waiter.wait( input->comm );
			// Clear waveforms off of NIAWG (not working??? memory appears to still run out...)
			for (int waveformInc = 2; waveformInc < output.waveCount; waveformInc++)
			{
				std::string waveformToDelete = "Waveform" + str( waveformInc );
				input->niawg->fgenConduit.deleteWaveform( cstr( waveformToDelete ) );
			}
			if (!input->dontActuallyGenerate)
			{
				input->niawg->fgenConduit.deleteScript( "experimentScript" );
			}
			for (auto& wave : output.waves)
			{
				wave.core.waveVals.clear();
				wave.core.waveVals.shrink_to_fit();
			}
		}
	}
	catch (Error& exception)
	{
		input->intensityAgilent->setDefualt(0);

		if (input->runNiawg)
		{
			for (const auto& sequenceInc : range( input->profile.sequenceConfigNames.size() ))
			{
				for (const auto& axis : AXES)
				{
					if (niawgFiles[axis][sequenceInc].is_open())
					{
						niawgFiles[axis][sequenceInc].close();
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

		if (input->thisObj->isAborting)
		{
			expUpdate( "\r\nABORTED!\r\n", input->comm, input->quiet );
			input->comm->sendColorBox( Master, 'B' );
		}
		else
		{
			// No quiet option for a bad exit.
			input->comm->sendColorBox( Master, 'R' );
			input->comm->sendStatus( "Bad Exit!\r\n" );
			std::string exceptionTxt = exception.what();
			input->comm->sendError( exception.what() );
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
		delete input;
		return false;
	}
	ULONGLONG endTime = GetTickCount();
	expUpdate( "Experiment took " + str( (endTime - startTime) / 1000.0 ) + " seconds.\r\n", input->comm, input->quiet );
	input->thisObj->experimentIsRunning = false;
	return true;
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
	// start thread.
	runningThread = AfxBeginThread(experimentThreadProcedure, input);	
}


void MasterManager::startExperimentThread(MasterThreadInput* input)
{
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


void MasterManager::loadVariables(std::vector<variable> newVariables)
{
	//variables = newVariables;
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


void MasterManager::analyzeFunction( std::string function, std::vector<std::string> args, TtlSystem* ttls,
										 DacSystem* dacs, std::vector<std::pair<UINT, UINT>>& ttlShades,
										 std::vector<UINT>& dacShades, RhodeSchwarz* rsg, std::vector<variable>& vars)
{
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, cstr(FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION), "r" );
	if ( !file )
	{
		thrower("ERROR: Function " + function + " does not exist!");
	}
	else
	{
		fclose( file );
	}
	functionFile.open(FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION, std::ios::in);
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
	std::vector<UINT> totalRepeatNum, currentRepeatNum, repeatPos;
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
		if (word == "t")
		{
			std::string command, time;
			functionStream >> command;
			if (command == "++")
			{
				operationTime.second++;
			}
			if (command == "+=")
			{
				functionStream >> time;
				try
				{
					operationTime.second += reduce(time);
				}
				catch (Error&)
				{
					// Assume it's an expression with variables, to be evaluated later.
					operationTime.first.push_back(time);
				}
			}
			else if (command == "=")
			{
				functionStream >> time;
				try
				{
					operationTime.second = reduce(time);
				}
				catch (Error&)
				{
					operationTime.first.push_back(time);
					// check if it's a variable.
					bool isVar = false;
					for (UINT varInc = 0; varInc < vars.size(); varInc++)
					{
						// assume it's an expression with a variable to be evaluated later
						
					}
					if (!isVar)
					{
						thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.\r\n");
					}
				}
			}
			else
			{
				thrower("ERROR: unrecognized time operator: " + command + ". Expected operators are \"++\", \"+=\", "
						"and \"=\"\r\n");
			}
		}
		else if (word == "t++")
		{
			operationTime.second++;
		}
		else if (word == "t+=")
		{
			std::string time;
			functionStream >> time;
			try
			{
				operationTime.second += std::stoi(time);
			}
			catch (std::invalid_argument&)
			{
				// check if it's a variable.
				bool isVar = false;
				for (UINT varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == time)
					{
						isVar = true;
						vars[varInc].active = true;
						operationTime.first.push_back(time);
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.");
				}
			}
		}
		else if (word == "t=")
		{
			std::string time;
			functionStream >> time;
			try
			{
				operationTime.second = std::stoi(time);
			}
			catch (std::invalid_argument &)
			{
				// check if it's a variable.
				bool isVar = false;
				for (UINT varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == time)
					{
						isVar = true;
						vars[varInc].active = true;
						operationTime.first.push_back(time);
						// because it's equals. There shouldn't be any extra terms added to this now.
						operationTime.second = 0;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.");
				}
			}
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
			ttls->handleTtlScriptCommand( word, operationTime, name, ttlShades );
		}
		else if (word == "pulseon:" || word == "pulseoff:")
		{
			// this requires handling time as it is handled above.
			std::string name;
			std::string pulseLength;
			functionStream >> name;
			functionStream >> pulseLength;
			// should be good to go.
			ttls->handleTtlScriptCommand( word, operationTime, name, pulseLength, ttlShades );
		}

		/// deal with dac commands
		else if (word == "dac:")
		{
			std::string name;
			functionStream >> name;
			std::string value;
			functionStream >> value;
			try
			{
				dacs->handleDacScriptCommand(operationTime, name, "__NONE__", value, "0", "__NONE__", dacShades, vars, ttls);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dac:\" command inside function " + function);
			}
		}
		else if (word == "dacramp:")
		{
			std::string name, initVal, finalVal, rampTime, rampInc;
			// get dac name
			functionStream >> name;
			// get ramp initial value
			functionStream >> initVal;
			// get ramp final value
			functionStream >> finalVal;
			// get total ramp time;
			functionStream >> rampTime;
			// get ramp point increment.
			functionStream >> rampInc;
			//
			try
			{
				dacs->handleDacScriptCommand(operationTime, name, initVal, finalVal, rampTime, rampInc, dacShades, vars, ttls);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dacramp:\" command inside function " + function);
			}
		}
		/// Handle RSG calls.
		else if (word == "RSG:")
		{
			rsgEventStructuralInfo info;
			functionStream >> info.frequency;
			functionStream >> info.power;
			// test frequency
			info.time = operationTime;
			rsg->addFrequency( info );
			// set up a trigger for this event.
			ttls->handleTtlScriptCommand( "pulseon:", operationTime, rsg->getRsgTtl(), str(rsg->getTriggerTime()), ttlShades );
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
				thrower(err.whatStr() + "... In function call to function " + functionName);
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


void MasterManager::analyzeMasterScript( TtlSystem* ttls, DacSystem* dacs,
										 std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
										 RhodeSchwarz* rsg, std::vector<variable>& vars)
{
	// reset this.
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
	std::vector<UINT> totalRepeatNum, currentRepeatNum, repeatPos;
	// the analysis loop.
	while (!(currentMasterScript.peek() == EOF) || word != "__end__")
	{
		std::stringstream individualCommandStream;
		// catch if the user uses a space between t and it's operator.
		if (word == "t")
		{
			std::string command;
			// assume that there's an '=' or '+=' etc. to still get...
			currentMasterScript >> command;
			word += command;
		}
		/// Handle Time Commands.		
		if (word == "t++")
		{
			operationTime.second++;
		}
		else if (word == "t+=")
		{
			std::string time;
			currentMasterScript >> time;
			try
			{
				operationTime.second += reduce(time);
			}
			catch (Error&)
			{
				operationTime.first.push_back(time);
			}
		}
		else if (word == "t=")
		{
			std::string time;
			currentMasterScript >> time;
			try
			{
				operationTime.second = reduce(time);
				if (operationTime.second < 0.1)
				{
					thrower("ERROR: you attempted to set commands before the first 1 ms of the experiment. This time is"
							"reserved by the code for initializing the dac state. Please start at 1ms.");
				}
			}
			catch (Error&)
			{
				// should I clear this?
				operationTime.first.push_back(time);
				// because it's equals. There shouldn't be any extra terms added to this now.
				operationTime.second = 0;
			}
		}
		/// callcppcode function
		else if (word == "callcppcode")
		{
			// and that's it... 
			callCppCodeFunction();
		}
		/// deal with ttl commands
		else if (word == "on:" || word == "off:")
		{
			std::string name;
			currentMasterScript >> name;
			ttls->handleTtlScriptCommand( word, operationTime, name, ttlShades );
		}
		else if (word == "pulseon:" || word == "pulseoff:")
		{
			// this requires handling time as it is handled above.
			std::string name;
			std::string pulseLength;
			currentMasterScript >> name;
			currentMasterScript >> pulseLength;
			bool isVar = false;
			try
			{
				double test = reduce( pulseLength );
			}
			catch (Error&)
			{
				for (UINT varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == pulseLength)
					{
						isVar = true;
						vars[varInc].active = true;
						// then it will parse okay.
						break;
					}
				}
				if (!isVar)
				{
					thrower( "ERROR: tried and failed to convert " + pulseLength + " to an integer for a pulse command." );
				}
			}
			// should be good to go.
			ttls->handleTtlScriptCommand( word, operationTime, name, pulseLength, ttlShades );
		}

		/// deal with dac commands
		else if (word == "dac:")
		{
			std::string dacName, dacVoltageValue;
			currentMasterScript >> dacName;
			currentMasterScript >> dacVoltageValue;
			try
			{
				dacs->handleDacScriptCommand(operationTime, dacName, "__NONE__", dacVoltageValue, "0", "__NONE__", 
											  dacShades, vars, ttls);
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dac:\" command inside main script");
			}
		}
		else if (word == "dacramp:")
		{
			std::string name, initVal, finalVal, rampTime, rampInc;
			// get dac name
			currentMasterScript >> name;
			// get ramp initial value
			currentMasterScript >> initVal;
			// get ramp final value
			currentMasterScript >> finalVal;
			// get total ramp time;
			currentMasterScript >> rampTime;
			// ge ramp point increment.
			currentMasterScript >> rampInc;
			try
			{
				dacs->handleDacScriptCommand( operationTime, name, initVal, finalVal, rampTime, rampInc, dacShades, 
											 vars, ttls );
			}
			catch (Error& err)
			{
				thrower(err.whatStr() + "... in \"dacramp:\" command inside main script");
			}
		}
		/// Deal with RSG calls
		else if (word == "rsg:")
		{
			rsgEventStructuralInfo info;
			currentMasterScript >> info.frequency;
			currentMasterScript >> info.power;
			// test frequency
			try
			{
				double test = reduce( info.frequency );

			}
			catch (Error&)
			{
				bool isVar = false;
				for (UINT varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == info.frequency )
					{
						isVar = true;
						vars[varInc].active = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower( "ERROR: Argument " + info.frequency + " is not a double or a variable name. Can't program the RSG "
							 "using this." );
				}
			}
			// test power
			try
			{
				double test = reduce( info.power );
			}
			catch (Error&)
			{
				bool isVar = false;
				for (UINT varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == info.power )
					{
						isVar = true;
						vars[varInc].active = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower( "ERROR: Argument " + info.power + " is not a double or a variable name. Can't program the RSG "
							 "using this." );
				}
			}
			info.time = operationTime;

			rsg->addFrequency( info );
			ttls->handleTtlScriptCommand( "pulseon:", operationTime, rsg->getRsgTtl(), str( rsg->getTriggerTime() ), ttlShades );
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
			std::string repeatStr;
			currentMasterScript >> repeatStr;
			try
			{
				totalRepeatNum.push_back( reduce( repeatStr ) );
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
		 || word == "dac:" || word == "dacramp:" || word == "RSG:" || word == "raman:" || word == "call"
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

UINT MasterManager::determineVariationNumber( std::vector<variable> vars, key tempKey )
{
	int variationNumber;
	if (vars.size() == 0)
	{
		variationNumber = 1;
	}
	else
	{
		variationNumber = tempKey[vars[0].name].first.size();
		if (variationNumber == 0)
		{
			variationNumber = 1;
		}
	}
	return variationNumber;
}