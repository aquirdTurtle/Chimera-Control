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
	SocketWrapper socket;
	input->status->appendText("............................\r\n", 1);
	// load the variables.
	input->status->appendText("Loading Variables...", 0);
	if (input->key->loadVariables(input->vars))
	{
		input->status->appendText("Done.\r\n", 0);
	}
	else
	{
		input->status->appendText("FAILED!\r\n", 0);
		return false;
	}
	// create the key
	input->status->appendText("Creating Key...", 0);
	if (input->key->generateKey())
	{
		input->status->appendText("Done.\r\n", 0);
	}
	else
	{
		input->status->appendText("FAILED!\r\n", 0);
		return false;
	}
	input->status->appendText("Exporting Key...", 0);
	if (input->key->exportKey())
	{
		input->status->appendText("Done.\r\n", 0);
	}
	else
	{
		input->status->appendText("FAILED!\r\n", 0);
		return false;
	}
	// connect to the niawg program.
	if (input->connectToNIAWG)
	{

		// must initialize socket inside the thread.
		input->status->appendText("Initializing Socket...", 0);
		if (socket.initialize())
		{
			input->status->appendText("Done.!\r\n", 0);
		}
		else
		{
			input->status->appendText("FAILED!\r\n", 0);
			return false;
		}

		input->status->appendText("Connecting to NIAWG...", 0);
		if (socket.connect())
		{
			input->status->appendText("Done.!\r\n", 0);
		}
		else
		{
			input->status->appendText("FAILED!\r\n", 0);
			return false;
		}
	}
	else
	{
		input->status->appendText("Not Connecting to NIAWG.\r\n", 0);
	}
	input->status->appendText("Analyzing Master Script...", 0);
	// analyze the master script.
	std::vector<std::pair<unsigned int, unsigned int>> ttlShadeLocations;
	std::vector<unsigned int> dacShadeLocations;
	std::array<std::string, 3> ramanFrequencies;
	input->dacs->resetDACEvents();
	input->ttls->resetTTLEvents();
	input->thisObj->loadVariables(input->vars);
	if (input->thisObj->analyzeCurrentMasterScript(input->ttls, input->dacs, ttlShadeLocations, dacShadeLocations, input->rsg, ramanFrequencies))
	{
		input->status->appendText("Done.\r\n", 0);
	}
	else
	{
		input->status->appendText("FAILED!\r\n", 0);
		return false;
	}
	// update ttl and dac looks based on which ones are used in the experiment.
	input->ttls->shadeTTLs(ttlShadeLocations);
	input->dacs->shadeDacs(dacShadeLocations);

	std::unordered_map<std::string, std::vector<double>> workingKey;
	workingKey = input->key->getKey();
	int variations;
	if (input->thisObj->variables.size() == 0)
	{
		variations = 1;
	}
	else
	{
		variations = workingKey[input->thisObj->variables[0].name].size();
		if (variations == 0)
		{
			variations = 1;
		}
	}
	/// /////////////////////////////
	/// Begin experiment loop
	/// //////////
	// loop for variations
	for (int varInc = 0; varInc < variations; varInc++)
	{
		// update things based on the new key value.
		input->status->appendText("Loading Key...", 0);
		input->dacs->interpretKey( input->key->getKey(), varInc, input->vars );
		input->ttls->interpretKey( input->key->getKey(), varInc );
		input->rsg->interpretKey( input->key->getKey(), varInc );
		std::array<double, 3> freqs = input->gpibHandler->interpretKeyForRaman(ramanFrequencies, input->key->getKey(), varInc);
		// prepare dac and ttls. data to final forms.
		input->status->appendText("Analyzing Script Data...", 0);
		input->dacs->analyzeDAC_Commands();
		input->dacs->makeFinalDataFormat();
		input->ttls->analyzeCommandList();
		input->ttls->convertToFinalFormat();
		// program devices
		input->status->appendText("Programming Hardware...", 0);
		input->gpibHandler->programRamanFGs( freqs[0], freqs[1], freqs[2] );
		input->rsg->programRSG( input->gpibHandler );
		// loop for repetitions
		for (int repInc = 0; repInc < input->repetitions; repInc++)
		{
			// this apparently needs to be re-written each time from looking at the VB6 code.
			input->dacs->writeDacs();

			input->ttls->writeData();
			input->ttls->startBoard();
			// wait until finished.
			input->ttls->waitTillFinished();
		}
	}
	// do experiment stuff...
	input->status->appendText("Experiment Finished.", 0);
	input->dacs->unshadeDacs();
	input->ttls->unshadeTTLs();
	delete input;
	return true;
}

bool ExperimentManager::startExperimentThread(MasterWindow* master)
{
	ExperimentThreadInput* input = new ExperimentThreadInput;
	input->ttls = &master->ttlBoard;
	input->dacs = &master->dacBoards;
	input->vars = master->variables.getEverything();
	input->repetitions = master->repetitionControl.getRepetitionNumber();
	input->thisObj = this;
	input->status = &master->generalStatus;
	input->error = &master->errorStatus;
	input->key = &master->masterKey;
	input->masterScriptAddress = master->profile.getMasterAddressFromConfig();
	input->rsg = &master->RhodeSchwarzGenerator;
	input->gpibHandler = &master->gpibHandler;
	this->loadMasterScript(master->profile.getMasterAddressFromConfig());
	master->logger.generateLog(master);
	master->logger.exportLog();
	// TODO: Make a control for this.
	input->connectToNIAWG = false;
	input->niawgSocket = &master->niawgSocket;
	// gather parameters.
	runningThread = AfxBeginThread(experimentThreadProcedure, input);
	// start thread.

	return false;
}

bool ExperimentManager::pause()
{
	return true;
}

bool ExperimentManager::stop()
{
	return true;
}

bool ExperimentManager::clear()
{
	return true;
}

bool ExperimentManager::loadMasterScript(std::string scriptAddress)
{
	std::ifstream scriptFile;
	// check if file address is good.
	struct stat buffer;
	if (stat(scriptAddress.c_str(), &buffer) != 0)
	{
		MessageBox(0, ("ERROR: File " + scriptAddress + " does not exist!").c_str(), 0, 0);
		return false;
	}
	scriptFile.open(scriptAddress.c_str());
	// check opened correctly
	if (!scriptFile.is_open())
	{
		MessageBox(0, "ERROR: File passed test making sure the file exists, but it still failed to open!", 0, 0);
		return false;
	}
	// dump the file into the stringstream.
	std::stringstream buf;
	buf << scriptFile.rdbuf();
	// for whatever reason, after loading rdbuf into a stringstream, the stream seems to not want to >> into a string. tried resetting too using seekg, but whatever, this works.
	this->currentMasterScript.clear();
	this->currentMasterScript = std::stringstream(buf.str());
	//std::string str(this->currentMasterScript.str());
	scriptFile.close();
	return true;
}

bool ExperimentManager::loadVariables(std::vector<variable> newVariables)
{
	this->variables = newVariables;
	return true;
}

// makes sure formatting is correct, returns the arguments and the function name from reading the firs real line of a function file.
bool ExperimentManager::analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args)
{
	args.clear();
	std::stringstream defStream(defLine);
	std::string word;
	defStream >> word;
	if (word == "")
	{
		defStream >> word;
	}
	if (word != "def")
	{
		errBox("ERROR: Function file in functions folder was not a function because it did not start with \"def\"! Functions must start with this. Instead it"
			" started with " + word);
		return false;
	}
	std::string functionDeclaration, functionArgumentList;
	std::getline(defStream, functionDeclaration, ':');
	int initNamePos = defLine.find_first_not_of(" \t")+1;
	functionName = functionDeclaration.substr(initNamePos, functionDeclaration.find_first_of("(") - initNamePos);

	if (functionName.find_first_of(" ") != std::string::npos)
	{
		errBox("ERROR: Function name included a space!");
		return false;
	}
	int initPos = functionDeclaration.find_first_of("(");
	if (initPos == std::string::npos)
	{
		errBox("ERROR: No starting parenthesis \"(\" in function definition. Use \"()\" if no arguments.");
		return false;
	}
	initPos++;
	int endPos = functionDeclaration.find_last_of(")");
	if (endPos == std::string::npos)
	{
		errBox("ERROR: No ending parenthesis \")\" in function definition. Use \"()\" if no arguments.");
		return false;
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
			errBox("ERROR: bad argument list in function. It looks like there might have been a space or tab inside the function argument?");
			good = false;
			break;
		}

		if (tempArg == "")
		{
			errBox("ERROR: bad argument list in function. It looks like there might have been a stray \",\"?");
			good = false;
			break;
		}
		args.push_back(tempArg);
		endPos = functionArgumentList.find_first_of(",");
		initPos = functionArgumentList.find_first_not_of(" \t");
	}
	if (!good)
	{
		return false;
	}
	return true;
}

bool ExperimentManager::analyzeFunction(std::string function, std::vector<std::string> args, TTL_System* ttls, DAC_System* dacs, 
	std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, 
	std::array<std::string, 3>& ramanFreqs)
{
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	struct stat buffer;
	if (stat((FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION).c_str(), &buffer) != 0)
	{
		MessageBox(0, ("ERROR: Function " + function + " does not exist!").c_str(), 0, 0);
		return false;
	}
	functionFile.open((FUNCTIONS_FOLDER_LOCATION + function + FUNCTION_EXTENSION).c_str(), std::ios::in);
	// check opened correctly
	if (!functionFile.is_open())
	{
		MessageBox(0, ("ERROR: Function file " + function + "File passed test making sure the file exists, but it still failed to open!").c_str(), 0, 0);
		return false;
	}
	std::stringstream buf, functionStream;
	buf << functionFile.rdbuf();
	functionStream = std::stringstream(buf.str());
	functionFile.close();
	if (functionStream.str() == "")
	{
		MessageBox(0, ("ERROR: Function File for " + function + "was empty!").c_str(), 0, 0);
		return true;
	}

	std::string word;
	/// get the function arguments.
	this->eatComments(&functionStream);
	std::string defLine;
	std::getline(functionStream, defLine, ':');
	std::string name;
	std::vector<std::string> functionArgs;
	if (!this->analyzeFunctionDefinition(defLine, name, functionArgs))
	{
		return false;
	}

	if (functionArgs.size() != args.size())
	{
		errBox("ERROR: incorrect number of arguments in the function call. Number in call was: " + std::to_string(args.size()) + ", number expected was " + std::to_string(functionArgs.size()));
		return false;
	}
	functionStream >> word;
	while (!functionStream.eof())
	{
		this->eatComments(&functionStream);

		this->eatComments(&functionStream);
		if (word == "t")
		{
			std::string command, time;
			functionStream >> command;
			this->eatComments(&functionStream);
			if (command == "++")
			{
				this->operationTime.second++;
			}
			if (command == "+=")
			{
				functionStream >> time;
				for (int argInc = 0; argInc < functionArgs.size(); argInc++)
				{
					if (time == functionArgs[argInc])
					{
						time = args[argInc];
					}
				}
				this->eatComments(&functionStream);
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
							if (operationTime.first != "")
							{
								errBox("ERROR: the time control only currently supports a single variable in it's incrementation!");
								return false;
							}
							operationTime.first = time;
							break;
						}
					}
					if (!isVar)
					{
						MessageBox(0, ("ERROR: tried and failed to convert " + time + " to an integer for a time += command.").c_str(), 0, 0);
						return false;
					}
				}
			}
			else if (command == "=")
			{
				functionStream >> time;
				for (int argInc = 0; argInc < functionArgs.size(); argInc++)
				{
					if (time == functionArgs[argInc])
					{
						time = args[argInc];
					}
				}
				this->eatComments(&functionStream);
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
							if (operationTime.first != "")
							{
								errBox("ERROR: the time control only currently supports a single variable in it's incrementation!");
								return false;
							}
							operationTime.first = time;
							// because it's an equals. There shouldn't be anything added on to this value.
							operationTime.second = 0;
							break;
						}
					}
					if (!isVar)
					{
						MessageBox(0, ("ERROR: tried and failed to convert " + time + " to an integer for a time += command.").c_str(), 0, 0);
						return false;
					}
				}
			}
			else
			{
				MessageBox(0, ("ERROR: unrecognized time operator: " + command + ". Expected operators are \"++\", \"+=\", and \"=\"").c_str(), 0, 0);
				return false;
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

			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (time == functionArgs[argInc])
				{
					time = args[argInc];
				}
			}
			this->eatComments(&functionStream);
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
						if (operationTime.first != "")
						{
							errBox("ERROR: the time control only currently supports a single variable in it's incrementation!");
							return false;
						}
						operationTime.first = time;
						break;
					}
				}
				if (!isVar)
				{
					MessageBox(0, ("ERROR: tried and failed to convert " + time + " to an integer for a time += command.").c_str(), 0, 0);
					return false;
				}
			}
		}
		else if (word == "t=")
		{
			std::string time;
			functionStream >> time;
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (time == functionArgs[argInc])
				{
					time = args[argInc];
				}
			}
			this->eatComments(&functionStream);
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
						if (operationTime.first != "")
						{
							errBox("ERROR: the time control only currently supports a single variable in it's incrementation!");
							return false;
						}
						operationTime.first = time;
						// because it's equals. There shouldn't be any extra terms added to this now.
						operationTime.second = 0;
						break;
					}
				}
				if (!isVar)
				{
					MessageBox(0, ("ERROR: tried and failed to convert " + time + " to an integer for a time += command.").c_str(), 0, 0);
					return false;
				}
			}
		}
		/// deal with ttl commands
		else if (word == "on:" || word == "off:")
		{
			std::string name;
			this->eatComments(&functionStream);
			functionStream >> name;
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (name == functionArgs[argInc])
				{
					name = args[argInc];
				}
			}
			this->eatComments(&functionStream);
			ttls->handleTTL_ScriptCommand( word, operationTime, name, ttlShades );
			this->eatComments(&functionStream);
		}
		/// deal with dac commands
		else if (word == "dac:")
		{
			std::string name;
			this->eatComments(&functionStream);
			functionStream >> name;
			this->eatComments(&functionStream);
			std::string value;
			functionStream >> value;
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (name == functionArgs[argInc])
				{
					name = args[argInc];
				}
				if (value == functionArgs[argInc])
				{
					value = args[argInc];
				}
			}
			this->eatComments(&functionStream);
			dacs->handleDAC_ScriptCommand( operationTime, name, value, 0, 0, 0, dacShades, variables, ttls );
			this->eatComments(&functionStream);
		}
		else if (word == "dacramp:")
		{
			std::string name, initVal, finalVal, rampTime, rampInc;
			// get dac name
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> name;
			// get ramp initial value
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> initVal;
			// get ramp final value
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> finalVal;
			// get total ramp time;
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> rampTime;
			// ge ramp point increment.
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> rampInc;
			this->eatComments(&this->currentMasterScript);
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (name == functionArgs[argInc])
				{
					name = args[argInc];
				}
				if (initVal == functionArgs[argInc])
				{
					initVal = args[argInc];
				}
				if (finalVal == functionArgs[argInc])
				{
					finalVal = args[argInc];
				}
				if (rampTime == functionArgs[argInc])
				{
					rampTime = args[argInc];
				}
				if (rampInc == functionArgs[argInc])
				{
					rampInc = args[argInc];
				}
			}
			dacs->handleDAC_ScriptCommand( operationTime, name, initVal, finalVal, rampTime, rampInc, dacShades, variables, ttls );
		}
		/// Handle RSG calls.
		else if (word == "RSG:")
		{
			this->eatComments(&this->currentMasterScript);
			std::string word;
			this->currentMasterScript >> word;
			try
			{
				double test = std::stod(word);

			}
			catch (std::invalid_argument& exception)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == word)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: Argument " + word + " is not a double or a variable name. Can't program the RSG using this.");
					return false;
				}
			}
			rsg->addFrequency(word);
		}
		/// deal with raman calls (setting cooling frequencies)
		else if (word == "raman:")
		{
			if (ramanFreqs[0] != "")
			{
				errBox("ERROR: you tried to set raman frequencies twice!");
				return false;
			}
			std::string top, bottom, axial;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> top;
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (top == functionArgs[argInc])
				{
					top = args[argInc];
				}
			}
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> bottom;
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (bottom == functionArgs[argInc])
				{
					bottom = args[argInc];
				}
			}
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> axial;
			for (int argInc = 0; argInc < functionArgs.size(); argInc++)
			{
				if (axial == functionArgs[argInc])
				{
					axial = args[argInc];
				}
			}
			this->eatComments(&this->currentMasterScript);
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
					errBox("ERROR: the string " + top + " is neither a double nor a variable!");
					return false;
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
					errBox("ERROR: the string " + bottom + " is neither a double nor a variable!");
					return false;
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
					errBox("ERROR: the string " + axial + " is neither a double nor a variable!");
					return false;
				}
			}
			ramanFreqs[2] = axial;
		}
		/// deal with function calls.
		else if (word == "call")
		{
			// calling a user-defined function. Get the name and the arguments to pass to the function handler.
			std::string functionCall, functionName, functionInputArgs;
			std::getline(functionStream, functionCall, '\r');
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
					for (int argInc = 0; argInc < functionArgs.size(); argInc++)
					{
						if (arg == functionArgs[argInc])
						{
							arg = args[argInc];
						}
					}
					newArgs.push_back(arg);
					break;
				}
				arg = functionInputArgs.substr(0, pos);
				for (int argInc = 0; argInc < functionArgs.size(); argInc++)
				{
					if (arg == functionArgs[argInc])
					{
						arg = args[argInc];
					}
				}
				newArgs.push_back(arg);
				// cut out that argument off the string.
				functionInputArgs = functionInputArgs.substr(pos, functionInputArgs.size());
			}
			this->analyzeFunction(functionName, newArgs, ttls, dacs, ttlShades, dacShades, rsg, ramanFreqs);
		}
		else
		{
			MessageBox(0, ("ERROR: unrecognized master script command: " + word).c_str(), 0, 0);
			return false;
		}
		functionStream >> word;
	}
	return true;
}

bool ExperimentManager::analyzeCurrentMasterScript(TTL_System* ttls, DAC_System* dacs, std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, 
	std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, std::array<std::string, 3>& ramanFreqs)
{
	// reset this.
	this->operationTime.second = 0;
	this->operationTime.first = "";
	if (this->currentMasterScript.str() == "")
	{
		MessageBox(0, "ERROR: Master script is empty!", 0, 0);
		return false;
	}
	std::string word;
	this->eatComments(&this->currentMasterScript);
	this->currentMasterScript >> word;
	unsigned int totalRepeatNum = 0;
	unsigned int currentRepeatNum = 0;
	bool repeating = false;
	unsigned int repeatPos;
	std::stringstream repeatStream;
	while (!this->currentMasterScript.eof())
	{

		this->eatComments(&this->currentMasterScript);
		// catch if the user uses a space between t and it's operator.
		if (word == "t")
		{
			std::string command, time;
			this->currentMasterScript >> command;
			word += command;
			this->eatComments(&this->currentMasterScript);
		}
		/// Handle Time Commands.		
		if (word == "t++")
		{
			operationTime.second++;
		}
		else if (word == "t+=")
		{
			std::string time;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> time;
			this->eatComments(&this->currentMasterScript);
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
						if (operationTime.first != "")
						{
							errBox("ERROR: the time control only currently supports a single variable in it's incrementation!");
							return false;
						}
						operationTime.first = time;
						break;
					}
				}
				if (!isVar)
				{
					MessageBox(0, ("ERROR: tried and failed to convert " + time + " to an integer for a time += command.").c_str(), 0, 0);
					return false;
				}
			}
		}
		else if (word == "t=")
		{
			std::string time;
			this->currentMasterScript >> time;
			this->eatComments(&this->currentMasterScript);
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
						if (operationTime.first != "")
						{
							errBox("ERROR: the time control only currently supports a single variable in it's incrementation!");
							return false;
						}
						operationTime.first = time;
						// because it's equals. There shouldn't be any extra terms added to this now.
						operationTime.second = 0;
						break;
					}
				}
				if (!isVar)
				{
					MessageBox(0, ("ERROR: tried and failed to convert " + time + " to an integer for a time += command.").c_str(), 0, 0);
					return false;
				}
			}
		}
		/// deal with ttl commands
		else if (word == "on:" || word == "off:")
		{
			std::string name;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> name;
			this->eatComments(&this->currentMasterScript);
			ttls->handleTTL_ScriptCommand( word, operationTime, name, ttlShades );
			this->eatComments(&this->currentMasterScript);
		}
		/// deal with dac commands
		else if (word == "dac:")
		{
			std::string dacName, dacVoltageValue;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> dacName;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> dacVoltageValue;
			this->eatComments(&this->currentMasterScript);
			dacs->handleDAC_ScriptCommand( operationTime, dacName, dacVoltageValue, "", "", "", dacShades, variables, ttls );
			this->eatComments(&this->currentMasterScript);
		}
		else if (word == "dacramp:")
		{
			std::string name, initVal, finalVal, rampTime, rampInc;
			// get dac name
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> name;
			// get ramp initial value
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> initVal;
			// get ramp final value
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> finalVal;
			// get total ramp time;
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> rampTime;
			// ge ramp point increment.
			this->eatComments(&this->currentMasterScript);
			currentMasterScript >> rampInc;
			this->eatComments(&this->currentMasterScript);
			dacs->handleDAC_ScriptCommand( operationTime, name, initVal, finalVal, rampTime, rampInc, dacShades, variables, ttls );
		}
		/// Deal with RSG calls
		else if (word == "rsg:")
		{
			this->eatComments(&this->currentMasterScript);
			std::string word;
			this->currentMasterScript >> word;
			try
			{
				double test = std::stod(word);

			}
			catch (std::invalid_argument& exception)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variables[varInc].name == word)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: Argument " + word + " is not a double or a variable name. Can't program the RSG using this.");
					return false;
				}
			}
			rsg->addFrequency(word);
		}
		/// deal with raman beam calls (setting raman frequency).
		else if (word == "raman:")
		{
			if (ramanFreqs[0] != "")
			{
				errBox("ERROR: you tried to set raman frequencies twice!");
				return false;
			}
			std::string top, bottom, axial;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> top;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> bottom;
			this->eatComments(&this->currentMasterScript);
			this->currentMasterScript >> axial;
			this->eatComments(&this->currentMasterScript);
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
					errBox("ERROR: the string " + top + " is neither a double nor a variable!");
					return false;
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
					errBox("ERROR: the string " + bottom + " is neither a double nor a variable!");
					return false;
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
					errBox("ERROR: the string " + axial + " is neither a double nor a variable!");
					return false;
				}
			}
			ramanFreqs[2] = axial;
		}
		/// deal with function calls.
		else if (word == "call")
		{
			// calling a user-defined function. Get the name and the arguments to pass to the function handler.
			std::string functionCall, functionName, functionArgs;
			this->eatComments(&this->currentMasterScript);
			std::getline(this->currentMasterScript, functionCall, '\r');
			this->eatComments(&this->currentMasterScript);
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
					args.push_back(arg);
					break;
				}
				arg = functionArgs.substr(0, pos);
				args.push_back(arg);
				// cut oinputut that argument off the string.
				functionArgs = functionArgs.substr(pos + 1, functionArgs.size());
			}
			this->analyzeFunction(functionName, args, ttls, dacs, ttlShades, dacShades, rsg, ramanFreqs);
		}
		else if (word == "repeat:")
		{
			if (repeating)
			{
				errBox("ERROR! Nested repeat loops are not allowed!");
				return false;
			}
			this->eatComments(&this->currentMasterScript);
			std::string repeatStr;
			currentMasterScript >> repeatStr;
			this->eatComments(&this->currentMasterScript);
			try
			{
				totalRepeatNum = std::stoi(repeatStr);
			}
			catch (std::invalid_argument&)
			{
				errBox("ERROR: the repeat number failed to convert to an integer! Note that the repeat number can not currently be a variable.");
				return false;
			}
			repeating = true;
			repeatPos = this->currentMasterScript.tellp();
			currentRepeatNum = 0;
		}
		else if (word == "end")
		{
			if (!repeating)
			{
				errBox("ERROR! Tried to end repeat, but you weren't repeating!");
				return false;
			}
			if (currentRepeatNum < totalRepeatNum)
			{
				this->currentMasterScript.seekp(repeatPos);
				currentRepeatNum++;
			}
			else
			{
				repeating = false;
			}
		}
		else
		{
			MessageBox(0, ("ERROR: unrecognized master script command: " + word).c_str(), 0, 0);
			return false;
		}
		this->eatComments(&this->currentMasterScript);
		this->currentMasterScript >> word;
		this->eatComments(&this->currentMasterScript);
	}
	return true;
}

bool ExperimentManager::eatComments(std::stringstream* stream)
{
	// Grab the first character
	std::string comment;
	char currentChar = stream->get();
	// including the !file.eof() to avoid grabbing the null character at the end. 
	while ((currentChar == ' ' && !stream->eof()) || (currentChar == '\n' && !stream->eof()) || (currentChar == '\r' && !stream->eof())
		|| (currentChar == '\t' && !stream->eof()) || currentChar == '%' || (currentChar == ';' && !stream->eof()))
	{
		// remove entire comments from the input
		if (currentChar == '%')
		{
			std::getline(*stream, comment, '\n');
		}
		// get the next char
		currentChar = stream->get();
	}
	char next = stream->peek();
	if (next == EOF)
	{
		return true;
	}
	std::streamoff position = stream->tellg();
	// when it exits the loop, it will just have moved passed the first non-whitespace character. I want that character. Go back.
	if (position == 0)
	{
		stream->seekg(0, std::ios::beg);
	}
	else
	{
		stream->seekg(-1, SEEK_CUR);
	}
	return true;
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


