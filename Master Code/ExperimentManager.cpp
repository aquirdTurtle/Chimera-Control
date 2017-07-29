#include "stdafx.h"
#include "ExperimentManager.h"
#include "nidaqmx2.h"
#include <fstream>
#include "TtlSystem.h"
#include "DacSystem.h"
#include "constants.h"
#include "MasterWindow.h"

ExperimentManager::ExperimentManager()
{
	functionsFolderLocation = FUNCTIONS_FOLDER_LOCATION;
}


/*
 * The workhorse of actually running experiments. This thread procedure analyzes all of the GUI settings and current 
 * configuration settings to determine how to program and run the experiment.
 * @param rawInput: This is the only input to the procedure. It MUST be a pointer to a ExperimentThreadInput structure.
 * @return UINT: The return value is not used, i just return TRUE.
 */
UINT __cdecl ExperimentManager::experimentThreadProcedure(LPVOID rawInput)
{
	// convert the input to the correct structure.
	ExperimentThreadInput* input = (ExperimentThreadInput*)rawInput;
	// change the status of the parent object to reflect that the thread is running.
	input->thisObj->experimentIsRunning = true;
	// warnings will be passed by reference to a series of function calls which can append warnings to the string.
	// at a certain point the string will get outputted to the error console. Remember, errors themselves are handled 
	// by thrower() calls.
	std::string warnings;
	ULONGLONG startTime = GetTickCount();
	try
	{
		expUpdate("Handling Key... ", input->status, input->quiet);
		// load the variables.
		input->key->loadVariables( input->vars );
		// create the key. Happens only once.
		input->key->generateKey();
		input->key->exportKey();
		expUpdate("Done.\r\n", input->status, input->quiet);
		expUpdate("Loading Agilent Info...", input->status, input->quiet);
		for (auto agilent : input->agilents)
		{
			agilent->handleInput();
		}
		expUpdate("Analyzing Master Script...", input->status, input->quiet);
		// analyze the master script.
		std::vector<std::pair<unsigned int, unsigned int>> ttlShadeLocations;
		std::vector<unsigned int> dacShadeLocations;
		//tektronicsInfo tektronicsSettings;
		input->dacs->resetDacEvents();
		input->ttls->resetTtlEvents();
		input->rsg->clearFrequencies();
		input->thisObj->loadVariables( input->vars );
		input->thisObj->analyzeMasterScript( input->ttls, input->dacs, ttlShadeLocations, dacShadeLocations, input->rsg,
											input->vars);
		expUpdate("Done.\r\n", input->status, input->quiet);
		// update ttl and dac looks & interaction based on which ones are used in the experiment.
		input->ttls->shadeTTLs( ttlShadeLocations );
		input->dacs->shadeDacs( dacShadeLocations );
		//
		key workingKey;
		workingKey = input->key->getKey();
		int variations;
		if (input->vars.size() == 0 )
		{
			variations = 1;
		}
		else
		{
			variations = workingKey[input->vars[0].name].first.size();
			if ( variations == 0 )
			{
				variations = 1;
			}
		}
		// go ahead and check if abort was pressed real fast...
		if ( input->thisObj->isAborting )
		{
			expUpdate("\r\nABORTED!\r\n", input->status, input->quiet);
			return 0;
		}

		for (auto& agilent : input->agilents)
		{
			for (int varInc = 0; varInc < variations; varInc++)
			{
				// not needed yet because no agilent scripts in this program.
				//agilent->programScript( varInc, input->key->getKey(), ? ? ? , input-> ? ? ? );
			}
		}

		// must interpret key before setting the trigger events.
		input->ttls->interpretKey(input->key->getKey(), input->vars);
		input->dacs->interpretKey(input->key->getKey(), input->vars, warnings);
		input->rsg->interpretKey(input->key->getKey(), input->vars);
		input->tektronics1->interpretKey(input->key->getKey(), input->vars);
		input->tektronics2->interpretKey(input->key->getKey(), input->vars);
		ULONGLONG varProgramStartTime = GetTickCount();
		expUpdate("Programming All Variation Data...\r\n", input->status, input->quiet);
		for (int varInc = 0; varInc < variations; varInc++)
		{
			// reading these variables should be safe.
			if (input->thisObj->isAborting)
			{
				thrower("\r\nABORTED!\r\n", 0);
			}
			expUpdate("Programming Variation #" + str(varInc+1) + " Data...\r\n", input->status, input->quiet);
			input->dacs->analyzeDacCommands(varInc);
			input->dacs->setDacTriggerEvents(input->ttls, varInc);
			// prepare dac and ttls. data to final forms.
			input->dacs->makeFinalDataFormat(varInc);
			input->ttls->analyzeCommandList(varInc);
			input->ttls->convertToFinalFormat(varInc);
			if (input->ttls->countDacTriggers(varInc) != input->dacs->getNumberSnapshots(varInc))
			{
				thrower("ERROR: number of dac triggers from the ttl system does not match the number of dac snapshots!"
						" Number of dac triggers was " + str(input->ttls->countDacTriggers(varInc)) + " while number of dac "
						"snapshots was " + str(input->dacs->getNumberSnapshots(varInc)));
			}
			input->rsg->orderEvents(varInc);
			input->dacs->checkTimingsWork(varInc);
		}

		if (input->niawgSocket->connectSelected())
		{
			expUpdate("Connecting to new master...", input->status, input->quiet);
			input->niawgSocket->initializeWinsock();
			input->niawgSocket->connect();
			input->niawgSocket->send("Repetitions: " + str(input->repetitionNumber));
			input->niawgSocket->sendVars(input->vars, input->key->getKey());
			expUpdate("success.\r\n", input->status, input->quiet);
		}
		ULONGLONG varProgramEndTime = GetTickCount();
		expUpdate("Programming took " + str((varProgramEndTime - varProgramStartTime) / 1000.0) + " seconds.\r\n", 
				  input->status, input->quiet);
		expUpdate("Programmed time per repetition: " + str(input->ttls->getTotalTime(0)) + "\r\n",
				  input->status, input->quiet);
		ULONGLONG totalTime = 0;
		for (USHORT var = 0; var < variations; var++)
		{
			totalTime += input->ttls->getTotalTime(var) * input->repetitionNumber;
		}
		expUpdate("Programmed Total Experiment time: " + str(totalTime) + "\r\n", input->status, input->quiet);
		expUpdate("Number of TTL Events in experiment: " + str(input->ttls->getNumberEvents(0)) + "\r\n", input->status, 
				  input->quiet);
		expUpdate("Number of DAC Events in experiment: " + str(input->dacs->getNumberEvents(0)) + "\r\n", input->status, 
				  input->quiet);

		if (input->debugOptions.showTtls)
		{
			// output to status
			input->status->addStatusText(input->ttls->getTtlSequenceMessage(0));
			// output to debug file
			std::ofstream debugFile((DEBUG_OUTPUT_LOCATION + str("TTL-Sequence.txt")).c_str(), std::ios_base::app);
			if (debugFile.is_open())
			{
				debugFile << input->ttls->getTtlSequenceMessage(0);
				debugFile.close();
			}
			else
			{
				expUpdate("ERROR: Debug text file failed to open! Continuing...\r\n", input->status,
						  input->quiet);
			}
		}
		if (input->debugOptions.showDacs)
		{
			// output to status
			input->status->addStatusText(input->dacs->getDacSequenceMessage(0));
			// output to debug file.
			std::ofstream  debugFile((DEBUG_OUTPUT_LOCATION + str("DAC-Sequence.txt")).c_str(),
									 std::ios_base::app);
			if (debugFile.is_open())
			{
				debugFile << input->dacs->getDacSequenceMessage(0);
				debugFile.close();
			}
			else
			{
				input->error->addStatusText("ERROR: Debug text file failed to open! Continuing...\r\n");
			}
		}

		input->globalControl->setUsages(input->vars);
		// no quiet on warnings.
		expUpdate(warnings, input->status);
		/// /////////////////////////////
		/// Begin experiment loop
		/// //////////
		// loop for variations
		// TODO: If ! randomizing repetitions
		for (int varInc = 0; varInc < variations; varInc++)
		{
			expUpdate("Variation #" + str(varInc + 1) + "\r\n", input->status, input->quiet);
			Sleep(input->debugOptions.sleepTime);
			for (auto var : input->key->getKey())
			{
				// if varies...
				if (var.second.second)
				{
					if (var.second.first.size() == 0)
					{
						thrower("ERROR: Variable " + var.first + " varies, but has no values assigned to it!");
					}
					expUpdate(var.first + ": " + str(var.second.first[varInc]) + "\r\n", input->status, 
							  input->quiet);
				}
			}
			expUpdate("Programming Hardware...\r\n", input->status, input->quiet);
			input->rsg->programRSG(input->gpib, varInc);
			input->rsg->setInfoDisp(varInc);
			// program devices
			for (auto& agilent : input->agilents)
			{
				if (!agilent->connected())
				{
					continue;
				}
				agilent->convertInputToFinalSettings( input->key->getKey(), varInc );
				deviceOutputInfo info = agilent->getOutputInfo();
				for (auto chan : range( 2 ))
				{
					switch (info.channel[chan].option)
					{
						case -2:
							// don't do anything.
							break;
						case -1:
							agilent->outputOff( chan );
							break;
						case 0:
							agilent->setDC( chan, info.channel[chan].dc );
							break;
						case 1:
							agilent->setSingleFreq( chan, info.channel[chan].sine );
							break;
						case 2:
							agilent->setSquare( chan, info.channel[chan].square );
							break;
						case 3:
							agilent->setExistingWaveform( chan, info.channel[chan].preloadedArb );
							break;
						case 4:
							// TODO
						default:
							thrower( "ERROR: unrecognized channel 1 setting: " + str( info.channel[chan].option ) );
					}
				}
			}
			input->tektronics1->programMachine(input->gpib, varInc);
			input->tektronics2->programMachine(input->gpib, varInc);			
			// loop for repetitionNumber
			input->repControl->updateNumber(0);
			if (input->niawgSocket->connectSelected())
			{
				std::string msg = input->niawgSocket->recieve();
				if (msg != "go")
				{
					thrower("ERROR: Recieved message from new master that wasn't go! Message was: \"" + msg + "\"");
				}
				expUpdate("Recieved \"go\" flag from new master.\r\n", input->status, input->quiet);
			}
			expUpdate("Running Experiment.\r\n", input->status, input->quiet);
			for ( int repInc = 0; repInc < input->repetitionNumber; repInc++ )
			{
				// reading these variables should be safe.
				if ( input->thisObj->isAborting )
				{
					thrower( "\r\nABORTED!\r\n", 0 );
				}
				else if ( input->thisObj->isPaused )
				{
					expUpdate("\r\nPaused!\r\n...", input->status, input->quiet);
					// wait...
					while ( input->thisObj->isPaused )
					{
						// this could be changed to be a bit smarter I think.
						Sleep( 100 );
					}
					expUpdate("\r\nUn-Paused!\r\n", input->status, input->quiet);
				}
				input->repControl->updateNumber(repInc + 1);
				// this apparently needs to be re-written each time from looking at the VB6 code.
				input->dacs->stopDacs();
				input->dacs->configureClocks(varInc);
				input->dacs->writeDacs(varInc);
				input->dacs->startDacs();
				input->ttls->writeData(varInc);
				input->ttls->startBoard();
				// wait until finished.
				input->ttls->waitTillFinished(varInc);
			}
			expUpdate("\r\n", input->status, input->quiet);
		}
		// do experiment stuff...
		expUpdate("\r\nExperiment Finished Normally.\r\n", input->status, input->quiet);
		// this is necessary. If not, the dac system will still be "running" and won't allow updates through normal 
		// means.
		input->dacs->stopDacs();
		input->dacs->unshadeDacs();
		// make sure the display accurately displays the state that the experiment finished at.
		try
		{
			// it's possible to run experiments without dacs.
			input->dacs->setDacStatusNoForceOut(input->dacs->getFinalSnapshot());
		}
		catch (Error& err)
		{
			// this gets thrown if no dac events. just continue.
		}

		input->ttls->unshadeTtls();
		input->ttls->setTtlStatusNoForceOut(input->ttls->getFinalSnapshot());
	}
	catch ( Error& exception)
	{
		if ( input->thisObj->isAborting )
		{
			expUpdate("\r\nABORTED!\r\n", input->status, input->quiet);
		}
		else
		{
			// No quiet option for a bad exit.
			input->status->addStatusText( "Bad Exit!\r\n", 0 );
			std::string exceptionTxt = exception.what();
			input->error->addStatusText( exception.what(), 0 );
		}
		input->thisObj->experimentIsRunning = false;
		std::lock_guard<std::mutex> locker( input->thisObj->abortLock );
		input->thisObj->isAborting = false;
		input->ttls->unshadeTtls();
		input->dacs->unshadeDacs();
		delete input;
		return false;
	}
	ULONGLONG endTime = GetTickCount();
	expUpdate("Experiment took " + str((endTime - startTime) / 1000.0) + " seconds.\r\n", input->status, input->quiet);
	input->thisObj->experimentIsRunning = false;
	return true;
}


bool ExperimentManager::runningStatus()
{
	return experimentIsRunning;
}

/*** 
 * this function is very similar to startExperimentThread but instead of getting anything from the current profile, it
 * knows exactly where to look for the MOT profile. This is currently hard-coded.
 */
void ExperimentManager::loadMotSettings(MasterWindow* master)
{	
	if ( experimentIsRunning )
	{
		thrower( "Experiment is Running! Please abort the current run before setting the MOT settings." );
	}
	ExperimentThreadInput* input = new ExperimentThreadInput;
	input->quiet = true;
	input->ttls = &master->ttlBoard;
	input->dacs = &master->dacBoards;
	input->globalControl = &master->globalVariables;
	// don't get configuration variables. The MOT shouldn't depend on config variables.
	input->vars = master->globalVariables.getEverything();
	// Only set it once, clearly.
	input->repetitionNumber = 1;
	input->repControl = &master->repetitionControl;
	input->thisObj = this;
	input->status = &master->generalStatus;
	input->error = &master->errorStatus;
	input->key = &master->masterKey;
	input->masterScriptAddress = MOT_ROUTINE_ADDRESS;
	input->rsg = &master->RhodeSchwarzGenerator;
	input->gpib = &master->gpib;
	input->debugOptions = master->debugControl.getOptions();
	input->agilents.push_back( &master->topBottomAgilent );
	input->agilents.push_back( &master->uWaveAxialAgilent );
	input->tektronics1 = &master->tektronics1;
	input->tektronics2 = &master->tektronics2;

	// don't load, leave whatever the user is working with on the screen.
	// no logging needed for simple mot load.
	
	// TODO: Make a control for this.
	input->connectToNIAWG = false;
	input->niawgSocket = &master->niawgSocket;
	// gather parameters.
	loadMasterScript(input->masterScriptAddress);
	// start thread.
	runningThread = AfxBeginThread(experimentThreadProcedure, input);	
}


void ExperimentManager::startExperimentThread(MasterWindow* master)
{
	if ( experimentIsRunning )
	{
		thrower( "Experiment is already Running!  You can only run one experiment at a time! Please abort before "
				 "running again." );
	}
	ExperimentThreadInput* input = new ExperimentThreadInput;
	input->quiet = false;
	input->ttls = &master->ttlBoard;
	input->dacs = &master->dacBoards;
	input->globalControl = &master->globalVariables;
	// load the variables. This little loop is for letting configuration variables overwrite the globals.
	std::vector<variable> configVars = master->configVariables.getEverything();
	std::vector<variable> globals = master->globalVariables.getEverything();
	std::vector<variable> experimentVars = configVars;
	for (auto& globalVar : globals)
	{
		globalVar.overwritten = false;
		bool nameExists = false;
		for (auto& configVar : experimentVars)
		{
			if (configVar.name == globalVar.name)
			{
				globalVar.overwritten = true;
				configVar.overwritten = true;
			}
		}
		if (!globalVar.overwritten)
		{
			experimentVars.push_back(globalVar);			
		}
	}
	input->vars = experimentVars;
	master->globalVariables.setUsages(globals);
	// it's only necessary to add the two sets of variables together at this 
	// point. They are not treated differently by the analysis.
	//input->vars.insert( input->vars.end(), globals.begin(), globals.end());
	input->repetitionNumber = master->repetitionControl.getRepetitionNumber();
	input->repControl = &master->repetitionControl;
	input->thisObj = this;
	input->status = &master->generalStatus;
	input->error = &master->errorStatus;
	input->key = &master->masterKey;
	input->masterScriptAddress = master->profile.getMasterAddressFromConfig();
	input->rsg = &master->RhodeSchwarzGenerator;
	input->gpib = &master->gpib;
	input->debugOptions = master->debugControl.getOptions();
	input->agilents.push_back( &master->topBottomAgilent );
	input->agilents.push_back( &master->uWaveAxialAgilent );
	master->tektronics1.getSettings();
	master->tektronics2.getSettings();
	input->tektronics1 = &master->tektronics1;
	input->tektronics2 = &master->tektronics2;

	loadMasterScript(master->profile.getMasterAddressFromConfig());
	master->logger.generateLog(master);
	master->logger.exportLog();
	// TODO: Make a control for this?
	input->connectToNIAWG = false;
	input->niawgSocket = &master->niawgSocket;
	// start thread.
	runningThread = AfxBeginThread(experimentThreadProcedure, input, THREAD_PRIORITY_HIGHEST);
}


bool ExperimentManager::getIsPaused()
{
	return isPaused;
}

void ExperimentManager::pause()
{
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = true;
}


void ExperimentManager::unPause()
{
	// the locker object locks the lock (the pauseLock obj), and unlocks it when it is destroyed at the end of this function.
	std::lock_guard<std::mutex> locker( pauseLock );
	isPaused = false;
}

void ExperimentManager::abort()
{
	std::lock_guard<std::mutex> locker( abortLock );
	isAborting = true;
}

void ExperimentManager::loadMasterScript(std::string scriptAddress)
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, scriptAddress.c_str(), "r" );
	if ( !file )
	{
		thrower("ERROR: Master Script File " + scriptAddress + " does not exist!");
	}
	else
	{
		fclose( file );
	}
	scriptFile.open(scriptAddress.c_str());
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
	//currentMasterScript = std::stringstream(buf.str());
	currentMasterScript.str( buf.str());
	currentMasterScript.clear();
	currentMasterScript.seekg(0);
	//std::string str(currentMasterScript.str());
	scriptFile.close();
}


void ExperimentManager::loadVariables(std::vector<variable> newVariables)
{
	//variables = newVariables;
}


// makes sure formatting is correct, returns the arguments and the function name from reading the firs real line of a function file.
void ExperimentManager::analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args)
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


void ExperimentManager::analyzeFunction( std::string function, std::vector<std::string> args, TtlSystem* ttls, 
										 DacSystem* dacs, std::vector<std::pair<unsigned int, unsigned int>>& ttlShades,
										 std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, std::vector<variable>& vars)
{
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, (FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION).c_str(), "r" );
	if ( !file )
	{
		thrower("ERROR: Function " + function + " does not exist!");
	}
	else
	{
		fclose( file );
	}
	functionFile.open((FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION).c_str(), std::ios::in);
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
	std::vector<unsigned int> totalRepeatNum;
	std::vector<unsigned int> currentRepeatNum;
	std::vector<unsigned int> repeatPos;
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
	for (int replacementInc =0; replacementInc < args.size(); replacementInc++)
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
				catch (Error& exception)
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
				catch (Error& exception)
				{
					operationTime.first.push_back(time);
					// check if it's a variable.
					bool isVar = false;
					for (int varInc = 0; varInc < vars.size(); varInc++)
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
			catch (std::invalid_argument &exception)
			{
				// check if it's a variable.
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
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
			catch (std::invalid_argument &exception)
			{
				// check if it's a variable.
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
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


void ExperimentManager::analyzeMasterScript(TtlSystem* ttls, DacSystem* dacs, 
											std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, 
											std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, std::vector<variable>& vars)
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
	std::vector<unsigned int > totalRepeatNum;
	std::vector<unsigned int > currentRepeatNum;
	std::vector<unsigned int > repeatPos;
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
			catch (Error& exception)
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
			catch (Error& exception)
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
				for (int varInc = 0; varInc < vars.size(); varInc++)
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
				for (int varInc = 0; varInc < vars.size(); varInc++)
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
				for (int varInc = 0; varInc < vars.size(); varInc++)
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


std::string ExperimentManager::getErrorMessage(int errorCode)
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
void ExperimentManager::callCppCodeFunction()
{
		
}


bool ExperimentManager::isValidWord( std::string word )
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
void ExperimentManager::expUpdate(std::string text, StatusControl* status, bool quiet)
{
	if (!quiet)
	{
		status->addStatusText(text, 0);
	}
}
