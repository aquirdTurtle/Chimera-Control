#include "stdafx.h"
#include "ExperimentManager.h"
#include "nidaqmx2.h"
#include <fstream>
#include "TTL_System.h"
#include "DAC_System.h"
#include "constants.h"
#include "MasterWindow.h"

ExperimentManager::ExperimentManager()
{
	functionsFolderLocation = FUNCTIONS_FOLDER_LOCATION;
}


UINT __cdecl ExperimentManager::experimentThreadProcedure(LPVOID rawInput)
{
	
	// convert it to the correct structure.
	ExperimentThreadInput* input = (ExperimentThreadInput*)rawInput;
	input->thisObj->experimentIsRunning = true;
	SocketWrapper socket;
	try
	{
		input->status->addStatusText( "Loading Key...", 0 );
		// load the variables.
		input->key->loadVariables( input->vars );
		// create the key
		input->key->generateKey();
		input->status->addStatusText( "Done.\r\n", 0 );
		input->status->addStatusText( "Exporting Key...", 0 );
		input->key->exportKey();
		input->status->addStatusText( "Done.\r\n", 0 );
		// connect to the niawg program.
		if ( input->connectToNIAWG )
		{
			// must initialize socket inside the thread.
			input->status->addStatusText( "Initializing Socket...", 0 );
			socket.initialize();
			input->status->addStatusText( "Done.!\r\n", 0 );

			input->status->addStatusText( "Connecting to NIAWG...", 0 );
			socket.connect();
			input->status->addStatusText( "Done.!\r\n", 0 );
		}
		else
		{
			input->status->addStatusText( "NOT Connecting to NIAWG.\r\n", 0 );
		}

		input->status->addStatusText( "Loading Agilent Info...", 0 );
		for (auto agilent : input->agilents)
		{
			agilent->handleInput();
		}
		input->status->addStatusText( "Analyzing Master Script...", 0 );
		// analyze the master script.
		std::vector<std::pair<unsigned int, unsigned int>> ttlShadeLocations;
		std::vector<unsigned int> dacShadeLocations;
		std::array<std::string, 3> ramanFrequencies;
		input->dacs->resetDACEvents();
		input->ttls->resetTTLEvents();
		input->rsg->clearFrequencies();
		input->thisObj->loadVariables( input->vars );
		input->thisObj->analyzeCurrentMasterScript( input->ttls, input->dacs, ttlShadeLocations, dacShadeLocations, input->rsg, ramanFrequencies );
		input->status->addStatusText( "Done.\r\n", 0 );
		// update ttl and dac looks & interaction based on which ones are used in the experiment.
		input->ttls->shadeTTLs( ttlShadeLocations );
		input->dacs->shadeDacs( dacShadeLocations );
		//
		key workingKey;
		workingKey = input->key->getKey();
		int variations;
		if ( input->thisObj->variables.size() == 0 )
		{
			variations = 1;
		}
		else
		{
			variations = workingKey[input->thisObj->variables[0].name].first.size();
			if ( variations == 0 )
			{
				variations = 1;
			}
		}
		// go ahead and check if abort was pressed real fast...
		if ( input->thisObj->isAborting )
		{
			input->status->addStatusText( "\r\nABORTED!\r\n", 0 );
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

		/// /////////////////////////////
		/// Begin experiment loop
		/// //////////
		// loop for variations
		for (int varInc = 0; varInc < variations; varInc++)
		{
			input->status->addStatusText( "Variation #" + std::to_string( varInc + 1 ) + "\r\n", 0 );
			for (auto var : input->key->getKey())
			{
				// if varies...
				if (var.second.second)
				{
					if (var.second.first.size() == 0)
					{
						thrower("ERROR: Variable " + var.first + " varies, but has no values assigned to it!");
					}
					input->status->addStatusText( var.first + ": " + std::to_string( var.second.first[varInc] ) + "\r\n", 0 );
				}
			}
			// update things based on the new key value.
			input->status->addStatusText( "Loading Key & Analyzing Script Data...\r\n", 0 );
			input->dacs->interpretKey( input->key->getKey(), varInc, input->vars );
			input->dacs->analyzeDAC_Commands();

			// must interpret key before setting the trigger events.
			input->ttls->interpretKey( input->key->getKey(), varInc );
			input->dacs->setDacTtlTriggerEvents(input->ttls);
			input->rsg->interpretKey( input->key->getKey(), varInc );
			std::array<double, 3> freqs = input->gpibHandler->interpretKeyForRaman( ramanFrequencies, input->key->getKey(), varInc );
			// prepare dac and ttls. data to final forms.
			input->dacs->makeFinalDataFormat();
			input->ttls->analyzeCommandList();
			input->ttls->convertToFinalFormat();

			// program devices
			input->status->addStatusText( "Programming Hardware...\r\n", 0 );
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
							thrower( "ERROR: unrecognized channel 1 setting: " + std::to_string( info.channel[chan].option ) );
					}
				}
			}

			input->gpibHandler->programRamanFGs( freqs[0], freqs[1], freqs[2] );
			input->rsg->orderEvents();
			input->rsg->programRSG( input->gpibHandler );
			input->rsg->setInfoDisp();

			if ( input->debugOptions.showTtls )
			{
				input->status->addStatusText(input->ttls->getTtlSequenceMessage(), 0);
			}
			if ( input->debugOptions.showDacs )
			{
				input->status->addStatusText( input->dacs->getDacSequenceMessage(), 0);
			}

			input->status->addStatusText( "Total Repetition time: " + std::to_string( input->ttls->getTotalTime() ) + "\r\n", 0 );
			input->status->addStatusText( "Total Experiment time: " + std::to_string( input->ttls->getTotalTime() * input->repetitions * variations ) + "\r\n", 0 );
			// loop for repetitions
			input->status->addStatusText( "Repetition...", 0 );
			for ( int repInc = 0; repInc < input->repetitions; repInc++ )
			{
				// reading these variables should be safe.
				if ( input->thisObj->isAborting )
				{
					thrower( "\r\nABORTED!\r\n", 0 );
				}
				else if ( input->thisObj->isPaused )
				{
					input->status->addStatusText( "\r\nPaused!\r\n...", 0 );
					// wait...
					while ( input->thisObj->isPaused )
					{
						// this could be changed to be a bit smarter I think.
						Sleep( 100 );
					}
					input->status->addStatusText( "\r\nUn-Paused!\r\nRepetition.....", 0 );
				}
				if ( repInc != 0 )
				{
					input->status->deleteChars( std::to_string( repInc ).size() );
				}
				input->status->addStatusText( std::to_string(repInc + 1), 0 );
				// this apparently needs to be re-written each time from looking at the VB6 code.
				input->dacs->stopDacs();
				input->dacs->configureClocks();
				input->dacs->writeDacs();
				input->dacs->startDacs();
				input->ttls->writeData();
				input->ttls->startBoard();
				// wait until finished.
				input->ttls->waitTillFinished();
			}
			input->status->addStatusText( "\r\n", 0 );
		}
		// do experiment stuff...
		input->status->addStatusText( "\r\nExperiment Finished Normally.\r\n", 0 );
		// this is necessary. If not, the dac system will still be "running" and won't allow updates through normal 
		// means.
		input->dacs->stopDacs();
		input->dacs->unshadeDacs();
		// make sure the display accurately displays the state that the experiment finished at.
		input->dacs->setDacStatusNoForceOut(input->dacs->getFinalSnapshot());

		input->ttls->unshadeTTLs();
		input->ttls->setTtlStatusNoForceOut(input->ttls->getFinalSnapshot());
	}
	catch ( Error& exception)
	{
		if ( input->thisObj->isAborting )
		{
			input->status->addStatusText( "\r\nABORTED!\r\n", 0 );
		}
		else
		{
			input->status->addStatusText( "Bad Exit!\r\n", 0 );
			input->error->addStatusText( exception.what(), 0 );
		}
		input->thisObj->experimentIsRunning = false;
		std::lock_guard<std::mutex> locker( input->thisObj->abortLock );
		input->thisObj->isAborting = false;
		delete input;
		return false;
	}
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
	input->ttls = &master->ttlBoard;
	input->dacs = &master->dacBoards;
	// don't get configuration variables. The MOT shouldn't depend on config variables.
	input->vars = master->globalVariables.getEverything();
	// Only set it once, clearly.
	input->repetitions = 1;
	input->thisObj = this;
	input->status = &master->generalStatus;
	input->error = &master->errorStatus;
	input->key = &master->masterKey;
	input->masterScriptAddress = MOT_ROUTINE_ADDRESS;
	input->rsg = &master->RhodeSchwarzGenerator;
	input->gpibHandler = &master->gpibHandler;
	input->debugOptions = master->debugControl.getOptions();
	input->agilents.push_back( &master->topBottomAgilent );
	input->agilents.push_back( &master->uWaveAxialAgilent );

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
		thrower( "Experiment is already Running!  You can only run one experiment at a time!" );
	}
	ExperimentThreadInput* input = new ExperimentThreadInput;
	input->ttls = &master->ttlBoard;
	input->dacs = &master->dacBoards;
	input->vars = master->configVariables.getEverything();
	std::vector<variable> globals = master->globalVariables.getEverything();
	// it's only necessary to add the two sets of variables together at this 
	// point. They are not treated differently by the analysis.
	input->vars.insert( input->vars.end(), globals.begin(), globals.end());
	input->repetitions = master->repetitionControl.getRepetitionNumber();
	input->thisObj = this;
	input->status = &master->generalStatus;
	input->error = &master->errorStatus;
	input->key = &master->masterKey;
	input->masterScriptAddress = master->profile.getMasterAddressFromConfig();
	input->rsg = &master->RhodeSchwarzGenerator;
	input->gpibHandler = &master->gpibHandler;
	input->debugOptions = master->debugControl.getOptions();
	input->agilents.push_back( &master->topBottomAgilent );
	input->agilents.push_back( &master->uWaveAxialAgilent );

	loadMasterScript(master->profile.getMasterAddressFromConfig());
	master->logger.generateLog(master);
	master->logger.exportLog();
	// TODO: Make a control for this.
	input->connectToNIAWG = false;
	input->niawgSocket = &master->niawgSocket;
	// gather parameters.
	// start thread.
	runningThread = AfxBeginThread(experimentThreadProcedure, input);
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
	// this is used to more easily deal some of the analysis of the script.
	buf << "\r\n\r\n__END__";
	// for whatever reason, after loading rdbuf into a stringstream, the stream seems to not 
	// want to >> into a string. tried resetting too using seekg, but whatever, this works.
	currentMasterScript.str("");
	//this->currentMasterScript = std::stringstream(buf.str());
	currentMasterScript.str( buf.str());
	currentMasterScript.clear();
	currentMasterScript.seekg(0);
	//std::string str(this->currentMasterScript.str());
	scriptFile.close();
}

void ExperimentManager::loadVariables(std::vector<variable> newVariables)
{
	this->variables = newVariables;
	return;
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
		return;
	}
	int initPos = functionDeclaration.find_first_of("(");
	if (initPos == std::string::npos)
	{
		thrower("ERROR: No starting parenthesis \"(\" in function definition. Use \"()\" if no arguments.");
		return;
	}
	initPos++;
	int endPos = functionDeclaration.find_last_of(")");
	if (endPos == std::string::npos)
	{
		thrower("ERROR: No ending parenthesis \")\" in function definition. Use \"()\" if no arguments.");
		return;
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
			return;
		}
		if (tempArg == "")
		{
			thrower("ERROR: bad argument list in function. It looks like there might have been a stray \",\"?");
			return;
		}
		args.push_back(tempArg);
		endPos = functionArgumentList.find_first_of(",");
		initPos = functionArgumentList.find_first_not_of(" \t");
	}
	return;
}


void ExperimentManager::analyzeFunction(std::string function, std::vector<std::string> args, TtlSystem* ttls, DacSystem* dacs, 
	std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, 
	std::array<std::string, 3>& ramanFreqs)
{
	/// load the file
	std::fstream functionFile;
	// check if file address is good.

	FILE *file;
	fopen_s( &file, (FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION).c_str(), "r" );
	if ( !file )
	{
		thrower("ERROR: Function " + function + " does not exist!");
		return;
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
		return;
	}
	std::stringstream buf;
	ScriptStream functionStream;
	buf << functionFile.rdbuf();
	functionStream << buf.str();
	functionStream << "\r\n\r\n__END__";
	functionFile.close();
//	functionStream.loadReplacements()
	if (functionStream.str() == "")
	{
		thrower("ERROR: Function File for " + function + "was empty!");
		return;
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
		thrower("ERROR: incorrect number of arguments in the function call. Number in call was: " 
				 + std::to_string(args.size()) + ", number expected was " + std::to_string(functionArgs.size()));
		return;
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
				this->operationTime.second++;
			}
			if (command == "+=")
			{
				functionStream >> time;
				try
				{
					operationTime.second += std::stoi(time);
				}
				catch (std::invalid_argument &exception)
				{
					// check if it's a variable.
					bool isVar = false;
					for (int varInc = 0; varInc < variables.size(); varInc++)
					{
						if (variables[varInc].name == time)
						{
							isVar = true;
							operationTime.first.push_back( time );
							break;
						}
					}
					if (!isVar)
					{
						thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.");
						return;
					}
				}
			}
			else if (command == "=")
			{
				functionStream >> time;
				try
				{
					operationTime.second = std::stoi(time);
				}
				catch (std::invalid_argument &exception)
				{
					// check if it's a variable.
					bool isVar = false;
					for (int varInc = 0; varInc < variables.size(); varInc++)
					{
						if (variables[varInc].name == time)
						{
							isVar = true;
							operationTime.first.push_back(time);
							// because it's an equals. There shouldn't be anything added on to this value. 
							// This should reset all times after this one to be relative to the = variable.
							operationTime.second = 0;
							break;
						}
					}
					if (!isVar)
					{
						thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.");
						return;
					}
				}
			}
			else
			{
				thrower("ERROR: unrecognized time operator: " + command + ". Expected operators are \"++\", \"+=\", and \"=\"");
				return;
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
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == time)
					{
						isVar = true;
						operationTime.first.push_back(time);
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.");
					return;
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
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == time)
					{
						isVar = true;
						operationTime.first.push_back(time);
						// because it's equals. There shouldn't be any extra terms added to this now.
						operationTime.second = 0;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: tried and failed to convert " + time + " to an integer for a time += command.");
					return;
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
			ttls->handleTTL_ScriptCommand( word, operationTime, name, ttlShades );
		}
		else if (word == "pulseon:" || word == "pulseoff:")
		{
			// this requires handling time as it is handled above.
			std::string name;
			std::string pulseLength;
			functionStream >> name;
			functionStream >> pulseLength;
			bool isVar = false;
			try
			{
				double test = std::stod( pulseLength );
			}
			catch(std::invalid_argument&)
			{
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == pulseLength)
					{
						isVar = true;
						// then it will parse okay.
						break;
					}
				}
				if (!isVar)
				{
					thrower( "ERROR: tried and failed to convert " + pulseLength + " to an integer for a pulse command." );
					return;
				}
			}
			// should be good to go.
			ttls->handleTTL_ScriptCommand( word, operationTime, name, pulseLength, ttlShades );
		}

		/// deal with dac commands
		else if (word == "dac:")
		{
			std::string name;
			functionStream >> name;
			std::string value;
			functionStream >> value;
			dacs->handleDAC_ScriptCommand( operationTime, name, "__NONE__", value, "0", "__NONE__", dacShades, variables, ttls );
		}
		else if (word == "dacramp:")
		{
			std::string name, initVal, finalVal, rampTime, rampInc;
			// get dac name
			this->currentMasterScript >> name;
			// get ramp initial value
			currentMasterScript >> initVal;
			// get ramp final value
			currentMasterScript >> finalVal;
			// get total ramp time;
			currentMasterScript >> rampTime;
			// get ramp point increment.
			currentMasterScript >> rampInc;
			//
			dacs->handleDAC_ScriptCommand( operationTime, name, initVal, finalVal, rampTime, rampInc, dacShades, variables, ttls );
		}
		/// Handle RSG calls.
		else if (word == "RSG:")
		{
			rsgEventStructuralInfo info;
			currentMasterScript >> info.frequency;
			currentMasterScript >> info.power;
			// test frequency
			try
			{
				double test = std::stod( info.frequency );
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == info.frequency)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: Argument " + info.frequency + " is not a double or a variable name. Can't program the RSG "
							 "using this.");
					return;
				}
			}
			// test power
			try
			{
				double test = std::stod( info.power );
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == info.power)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower( "ERROR: Argument " + info.power + " is not a double or a variable name. Can't program the RSG "
							 "using this." );
					return;
				}
			}
			info.time = operationTime;
			rsg->addFrequency( info );
			// set up a trigger for this event.
			ttls->handleTTL_ScriptCommand( "pulseon:", operationTime, rsg->getRsgTtl(), std::to_string(rsg->getTriggerTime()), ttlShades );
		}
		/// deal with raman calls (setting cooling frequencies)
		else if (word == "raman:")
		{
			if (ramanFreqs[0] != "")
			{
				thrower("ERROR: you tried to set raman frequencies twice!");
				return;
			}
			std::string top, bottom, axial;
			currentMasterScript >> top;
			currentMasterScript >> bottom;
			currentMasterScript >> axial;
			// deal with top frequency.
			try
			{
				double topFreq = std::stod(top);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == top)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the string " + top + " is neither a double nor a variable!");
					return;
				}
			}
			ramanFreqs[0] = top;

			// deal with bottom frequency.
			try
			{
				double bottomFreq = std::stod(bottom);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == bottom)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the string " + bottom + " is neither a double nor a variable!");
					return;
				}
			}
			ramanFreqs[1] = bottom;
			// deal with axial frequency.
			try
			{
				double axialFreq = std::stod(axial);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == axial)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the string " + axial + " is neither a double nor a variable!");
					return;
				}
			}
			ramanFreqs[2] = axial;
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
					newArgs.push_back(arg);
					break;
				}
				arg = functionInputArgs.substr(0, pos);
				newArgs.push_back(arg);
				// cut out that argument off the string.
				functionInputArgs = functionInputArgs.substr(pos, functionInputArgs.size());
			}
			if (functionName == function)
			{
				thrower( "ERROR: Recursive function call detected! " + function + " called itself! This is not allowed." );
				return;
			}
			this->analyzeFunction(functionName, newArgs, ttls, dacs, ttlShades, dacShades, rsg, ramanFreqs);
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
				return;
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


void ExperimentManager::analyzeCurrentMasterScript(TtlSystem* ttls, DacSystem* dacs, std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, 
												   std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, std::array<std::string, 3>& ramanFreqs)
{
	// reset this.
	currentMasterScriptText = currentMasterScript.str();

	operationTime.second = 1;
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
				operationTime.second += std::stoi(time);
			}
			catch (std::invalid_argument &exception)
			{
				// check if it's a variable.
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == time)
					{
						isVar = true;
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
			currentMasterScript >> time;
			try
			{
				operationTime.second = std::stoi(time);
				if (operationTime.second < 1)
				{
					thrower("ERROR: you attempted to set commands before the first 1 ms of the experiment. This time is"
							"reserved by the code for initializing the dac state. Please start at 1ms.");
				}
			}
			catch (std::invalid_argument &exception)
			{
				// check if it's a variable.
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == time)
					{
						isVar = true;
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
			ttls->handleTTL_ScriptCommand( word, operationTime, name, ttlShades );
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
				double test = std::stod( pulseLength );
			}
			catch (std::invalid_argument&)
			{
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == pulseLength)
					{
						isVar = true;
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
			ttls->handleTTL_ScriptCommand( word, operationTime, name, pulseLength, ttlShades );
		}

		/// deal with dac commands
		else if (word == "dac:")
		{
			std::string dacName, dacVoltageValue;
			currentMasterScript >> dacName;
			currentMasterScript >> dacVoltageValue;
			dacs->handleDAC_ScriptCommand( operationTime, dacName, "__NONE__", dacVoltageValue, "0", "__NONE__", dacShades, variables, ttls );
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

			dacs->handleDAC_ScriptCommand( operationTime, name, initVal, finalVal, rampTime, rampInc, dacShades, variables, ttls );
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
				double test = std::stod( info.frequency );
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == info.frequency )
					{
						isVar = true;
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
				double test = std::stod( info.power );
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == info.power )
					{
						isVar = true;
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
			ttls->handleTTL_ScriptCommand( "pulseon:", operationTime, rsg->getRsgTtl(), std::to_string( rsg->getTriggerTime() ), ttlShades );
		}
		/// deal with raman beam calls (setting raman frequency).
		else if (word == "raman:")
		{
			if (ramanFreqs[0] != "")
			{
				thrower("ERROR: you tried to set raman frequencies twice!");
			}
			std::string top, bottom, axial;
			this->currentMasterScript >> top;
			this->currentMasterScript >> bottom;
			this->currentMasterScript >> axial;
			// deal with top frequency.
			try
			{
				double topFreq = std::stod(top);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == top)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the string " + top + " is neither a double nor a variable!");
				}
			}
			ramanFreqs[0] = top;

			// deal with bottom frequency.
			try
			{
				double bottomFreq = std::stod(bottom);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == bottom)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the string " + bottom + " is neither a double nor a variable!");
				}
			}
			ramanFreqs[1] = bottom;
			// deal with axial frequency.
			try
			{
				double axialFreq = std::stod(axial);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == axial)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the string " + axial + " is neither a double nor a variable!");
				}
			}
			ramanFreqs[2] = axial;
		}
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
			analyzeFunction(functionName, args, ttls, dacs, ttlShades, dacShades, rsg, ramanFreqs);
		}
		else if (word == "repeat:")
		{
			std::string repeatStr;
			currentMasterScript >> repeatStr;
			try
			{
				totalRepeatNum.push_back( std::stoi( repeatStr ) );
			}
			catch (std::invalid_argument&)
			{
				thrower("ERROR: the repeat number failed to convert to an integer! Note that the repeat number can not"
						 " currently be a variable.");
			}
			repeatPos.push_back( this->currentMasterScript.tellg() );

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

	//'Find out the error message length.
	bufferSize = DAQmxGetErrorString(errorCode, 0, 0);
	//'Allocate enough space in the string.
	errorChars = new char[bufferSize];
	//'Get the actual error message.
	status = DAQmxGetErrorString(errorCode, errorChars, bufferSize);
	std::string errorString(errorChars);
	return errorString;
}





/*
this function can be called directly from scripts. Insert things inside the function to make it do something
custom that's not possible inside the scripting language.
*/
void ExperimentManager::callCppCodeFunction()
{
	operationTime.second += 5000;
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