#include "stdafx.h"
#include "experimentProgrammingThread.h"
#include "externals.h"
#include "constants.h"
// Some headers used for communication protocols.
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include "myErrorHandler.h"
#include "myAgilent.h"
#include "NiawgController.h"
#include "experimentThreadInputStructure.h"
#include <sstream>
#include "NIAWGWaitThread.h"
#include "boost/cast.hpp"
#include "systemAbortCheck.h"
#include "postMyString.h"
#include "VariableSystem.h"
#include <boost/algorithm/string/replace.hpp>
#include <afxsock.h>


/*
 * This runs the experiment. It calls analyzeNiawgScripts and then procedurally goes through all variable values. It also communicates with the other computer
 * throughout the process.
 * inputParam is the list of all of the relevant parameters to be used during this run of the experiment.
 */
unsigned __stdcall experimentProgrammingThread(LPVOID inputParam)
{

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Initializing and Checking Options
	///

	experimentThreadInputStructure* input = (experimentThreadInputStructure*)inputParam;
	AfxSocketInit();
	ViBoolean isDoneTest = VI_FALSE;
	char userScriptName[FILENAME_MAX];
	CSocket ConnectSocket;
	bool userScriptIsWritten = false;

	std::vector<std::string> libWaveformArray[MAX_NIAWG_SIGNALS * 4];
	//bool fileOpenedStatus[MAX_NIAWG_SIGNALS * 4] = { 0 };
	std::vector<std::size_t> variableValuesLengths;
	niawgPair<int> writeCounts;
	int mixedCount, mixedWriteCount;
	niawgPair<std::vector<std::fstream>> scriptFiles;
	std::vector<std::fstream> intensityScriptFiles;
	// reset some variables which might have odd values if this is reached from a break from within the variable waveform calculations.
	bool correctionFlag = false;
	bool repeatFlag = false;
	bool rewriteFlag = false;
	ViChar* userScriptSubmit;
	bool intIsVaried;
	// clear old vectors...
	std::vector<long int> variedMixedSize;
	std::vector<myMath::minMaxDoublet> intensitySequenceMinAndMaxVector;
	std::string finalUserScriptString;
	niawgPair<std::vector<ViReal64 *>> variedWaveforms, variedWaveformFileData;
	std::vector<ViReal64 *> mixedWaveforms, currentWaveformPointers;
	std::string userScriptNameString;
	// initialize some socket stuff
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	/// Initialization
	// Communication object used to open up the windows socket applications (WSA) DLL. 
	WSADATA wsaData;
	// object that contains error information.
	int iResult;
	// the socket object used to connect to the other computer. Starts invalid because it isn't active yet.
	std::vector<std::vector<double> > variableValues;
	// Initialize Winsock
	// I have no idea what this MAKEWORD function is doing.f
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// check for errors initializing winsock
	if (iResult != 0) 
	{
		errBox("WSAStartup failed: " + std::to_string(iResult));
		delete input;
		return 1;
	}

	// initialize the script string. The script needs a script name at the top.
	userScriptNameString = "experimentScript";
	
	scriptFiles[Vertical].resize(input->sequenceFileNames.size());
	scriptFiles[Horizontal].resize(input->sequenceFileNames.size());
	intensityScriptFiles.resize(input->sequenceFileNames.size());
	std::vector<variable> singletons;
	std::vector<variable> varyingParameters;
	/// gather information from every configuration in the sequence. //////////////////////////////////////////////////////////////////////////////////////////
	for (int sequenceInc = 0; sequenceInc < input->sequenceFileNames.size(); sequenceInc++)
	{
		// open configuration file
		std::fstream configFile(input->currentFolderLocation + "\\" + input->sequenceFileNames[sequenceInc]);
		std::string intensityScriptAddress, version;
		niawgPair<std::string> niawgScriptAddresses;
		// first get version info:
		std::getline(configFile, version);
		// in order...
		/// load files
		for (auto axis : AXES)
		{
			getline( configFile, niawgScriptAddresses[axis] );
			scriptFiles[axis][sequenceInc].open( niawgScriptAddresses[axis] );
			if (!scriptFiles[axis][sequenceInc].is_open())
			{
				// send error
				if (myErrorHandler( 1, "ERROR: Failed to open vertical script file named: " + niawgScriptAddresses[axis] 
									+ " found in configuration: " + input->sequenceFileNames[sequenceInc] + "\r\n", &ConnectSocket, 
									scriptFiles, false, userScriptIsWritten, userScriptName, false, false, true, input->comm ) == true)
				{
					input->comm->sendFatalError( "ERROR: Failed to open vertical script file named: " + niawgScriptAddresses[axis] 
												 + " found in configuration: " + input->sequenceFileNames[sequenceInc] + "\r\n" );
					delete input;
					return -1;
				}
			}
		}
		/// load intensity file
		getline(configFile, intensityScriptAddress);
		intensityScriptFiles[sequenceInc].open(intensityScriptAddress);
		if (!intensityScriptFiles[sequenceInc].is_open())
		{
			// send error
			if (myErrorHandler(1, "ERROR: Failed to open intensity script file named: " + intensityScriptAddress + " found in configuration: "
				+ input->sequenceFileNames[sequenceInc] + "\r\n", &ConnectSocket, scriptFiles, false,
				  userScriptIsWritten, userScriptName, false, false, true, input->comm) == true)
			{
				input->comm->sendFatalError("ERROR: Failed to open intensity script file named: " + intensityScriptAddress + " found in configuration: "
					+ input->sequenceFileNames[sequenceInc] + "\r\n");
				delete input;
				return -1;
			}
		}
		/// load variables
		int varNum;
		configFile >> varNum;
		// early version didn't have variable type indicators.
		if (version == "Version: 1.0")
		{
			for (int varInc = 0; varInc < varNum; varInc++)
			{
				std::string varName;
				configFile >> varName;
				variable tempVariable;
				tempVariable.name = varName;
				// assume certain things for old files. E.g. singletons didn't exist. 
				tempVariable.singleton = false;
				tempVariable.timelike = false;
				tempVariable.value = 0;
				bool alreadyExists = false;
				for (int varInc = 0; varInc < varyingParameters.size(); varInc++)
				{
					if (tempVariable.name == varyingParameters[varInc].name)
					{
						alreadyExists = true;
						break;
					}
				}
				if (!alreadyExists)
				{
					// add new varying parameters.
					varyingParameters.push_back(tempVariable);
				}
			}
		}
		else if (version == "Version: 1.1")
		{
			for (int varInc = 0; varInc < varNum; varInc++)
			{
				variable tempVar;
				std::string varName, timelikeText, typeText, valueString;
				bool timelike;
				bool singleton;
				double value;
				configFile >> varName;
				configFile >> timelikeText;
				configFile >> typeText;
				configFile >> valueString;
				if (timelikeText == "Timelike")
				{
					timelike = true;
				}
				else if (timelikeText == "Not_Timelike")
				{
					timelike = false;
				}
				else
				{
					MessageBox(0, "ERROR: unknown timelike option. Check the formatting of the configuration file.", 0, 0);
					return true;
				}
				if (typeText == "Singleton")
				{
					singleton = true;
				}
				else if (typeText == "From_Master")
				{
					singleton = false;
				}
				else
				{
					MessageBox(0, "ERROR: unknown variable type option. Check the formatting of the configuration file.", 0, 0);
					return true;
				}
				try
				{
					value = std::stod(valueString);
				}
				catch (std::invalid_argument&)
				{
					MessageBox(0, ("ERROR: Failed to convert value in configuration file for variable's double value. Value was: " + valueString).c_str(), 0, 0);
					break;
				}
				tempVar.name = varName;
				tempVar.timelike = timelike;
				tempVar.singleton = singleton;
				tempVar.value = value;
				
				if (tempVar.singleton)
				{
					// handle singletons
					// check if it already has been loaded
					bool alreadyExists = false;
					for (int varInc = 0; varInc < singletons.size(); varInc++)
					{
						if (tempVar.name == singletons[varInc].name)
						{
							alreadyExists = true;
							break;
						}
					}
					if (!alreadyExists)
					{
						// load new singleton
						singletons.push_back(tempVar);
					}
				}
				else
				{
					// handle varying parameters
					bool alreadyExists = false;
					for (int varInc = 0; varInc < varyingParameters.size(); varInc++)
					{
						if (tempVar.name == varyingParameters[varInc].name)
						{
							alreadyExists = true;
							break;
						}
					}
					if (!alreadyExists)
					{
						// add new varying parameters.
						varyingParameters.push_back(tempVar);
					}
				}
			}
		}
		else
		{
			MessageBox(0, "ERROR: Unrecognized configuration version! Ask Mark about bugs.", 0, 0);
			return true;
		}
	}
	// initialize some socket stuffs
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		// Resolve the server address and port
		iResult = getaddrinfo(SERVER_ADDRESS, DEFAULT_PORT, &hints, &result);
		if (myErrorHandler(iResult, "ERROR: getaddrinfo failed: " + std::to_string(iResult) + "\r\n", &ConnectSocket, scriptFiles, false,
			  userScriptIsWritten, userScriptName, false, false, true, input->comm) == true)
		{
			input->comm->sendFatalError("ERROR: getaddrinfo failed: " + std::to_string(iResult) + "\r\n");
			delete input;
			return -1;
		}
	}
	ptr = result;

	// check acuumulations. if 0, this is code for setting the code to just repeat forever, 
	if (input->settings.connectToMaster == false)
	{
		input->repetitions = 0;
	}
	else
	{
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			// make socket object
			ConnectSocket.Socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			//ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			// Handle Errors
			if (myErrorHandler((ConnectSocket == -1), "ERROR: at socket() function: " + std::to_string(WSAGetLastError()) + "\r\n",
								&ConnectSocket, scriptFiles, false, userScriptIsWritten, userScriptName, false, false, true, input->comm))
			{
				input->comm->sendFatalError("ERROR: at socket() function: " + std::to_string(WSAGetLastError()) + "\r\n");
				delete input;
				return -1;
			}
		}
		input->comm->sendStatus("Attempting to connect......");
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			// Handle Errors
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (myErrorHandler(iResult, "Unable to connect to server!\r\n", &ConnectSocket, scriptFiles, /*aborting = */false,
				  userScriptIsWritten, userScriptName, /*Socket Active = */false, false, true, input->comm))
			{
				input->comm->sendFatalError("Unable to connect to server!\r\n");
				delete input;
				return -1;
			}
			freeaddrinfo(result);
		}
		input->comm->sendStatus("Established Connection!\r\n");
	}

	/// Get accumulations from master. Procedure is
	// this computer >> Other Computer ("Acumulations?")
	// Other Computer >> this computer ("Accumulations: (#)")
	// this computer >> other computer ("Received Accumulations.")
	if (input->settings.connectToMaster == true)
	{
		char recvbuf[256];
		int recvbufn = 256;
		input->comm->sendStatus("Waiting for Accumulations # from master...");
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{

			iResult = send(ConnectSocket, "Accumulations?", 14, 0);
			if (myErrorHandler(iResult == -1, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n",&ConnectSocket,
				scriptFiles, false, userScriptIsWritten, userScriptName, true, false, true, input->comm))
			{
				input->comm->sendFatalError("ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
				delete input;
				return -1;
			}
			iResult = recv(ConnectSocket, recvbuf, recvbufn, 0);
		}
		else
		{
			iResult = 1;
		}
		input->comm->sendStatus("Received!\r\n");
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			std::string tempAccumulations;
			std::stringstream accumulationsStream;
			accumulationsStream << recvbuf;
			accumulationsStream >> tempAccumulations;
			if (myErrorHandler(tempAccumulations != "Accumulations:", "ERROR: master's message did not start with \"Accumulations:\". It "
								"started with " + tempAccumulations + " . Assuming fatal error.",&ConnectSocket, scriptFiles, false, 
								userScriptIsWritten, userScriptName, true, false, true, input->comm))
			{
				input->comm->sendFatalError("!");
				delete input;
				return -1;
			}
			accumulationsStream >> tempAccumulations;
			try
			{
				(*input).repetitions = std::stoi(tempAccumulations);
			}
			catch (std::invalid_argument&)
			{
				if (myErrorHandler(true, "ERROR: master's message's number did not convert correctly to an integer. String trying to convert is "
									+ tempAccumulations + ". Assuming fatal error.",&ConnectSocket, scriptFiles, false, userScriptIsWritten, 
									userScriptName, true, false, true, input->comm))
				{
					input->comm->sendFatalError("!");
					delete input;
					return -1;
				}
			}
			if (myErrorHandler((*input).repetitions < 0, "ERROR: master's message's number was negative! String trying to convert is " + tempAccumulations
								+ ". Assuming fatal error.",&ConnectSocket, scriptFiles, false, userScriptIsWritten, userScriptName,
								true, false, true, input->comm))
			{
				input->comm->sendFatalError("!");
				delete input;
				return -1;
			}
			iResult = send(ConnectSocket, "Received Accumulations.", 23, 0);
			if (myErrorHandler(iResult == -1, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n",
								&ConnectSocket, scriptFiles, false, userScriptIsWritten, userScriptName, true, false, true, input->comm))
			{
				input->comm->sendFatalError("ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
				delete input;
				return -1;
			}
			if (myErrorHandler((*input).repetitions % ((*input).sequenceFileNames.size()) != 0, 
							   "ERROR: Number of accumulations received from master: " + std::to_string((*input).repetitions) 
							   + ", is not an integer multiple of the number of configurations in the sequence: " 
							   + std::to_string((*input).sequenceFileNames.size()) + ". It must be.\r\n",&ConnectSocket, scriptFiles, false,
								userScriptIsWritten, userScriptName, true, false, true, input->comm))
			{
				input->comm->sendFatalError("ERROR: Number of accumulations received from master: " + std::to_string((*input).repetitions)
													+ ", is not an integer multiple of the number of configurations in the sequence: "
													+ std::to_string((*input).sequenceFileNames.size()) + ". It must be.\r\n");
				delete input;
				return -1;
			}
		}
		else
		{
			(*input).repetitions = 100;
		}
	}
	(*input).repetitions /= (*input).sequenceFileNames.size();

	std::string message = "Accumulations # after sequence normalization: " + std::to_string((*input).repetitions) + "\r\n";
	input->comm->sendStatus( message );
	// analyze the input files and create the xy-script. Originally, I thought I'd write the script in two parts, the x and y parts, but it turns out 
	// not to work like I thought it did. If I'd known this from the start, I probably wouldn't have created this subroutine, except perhaps for the 
	// fact that it get called recursively by predefined scripts in the instructions file.
	input->comm->sendStatus("Beginning Waveform Read, Calculate, and Write\r\nProcedure...\r\n");
	if (systemAbortCheck(input->comm))
	{
		for (auto axis : AXES)
		{
			for (auto& waveform : variedWaveforms[axis])
			{
				delete[] waveform;
			}
			for (auto& waveform : variedWaveformFileData[axis])
			{
				delete[] waveform;
			}
		}
		for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
		{
			delete[] mixedWaveforms[deleteInc];
		}
		delete input;
		return 0;
	}
	std::vector<std::string> workingUserScriptString;
	workingUserScriptString.resize( (input->profile.sequenceConfigNames.size()) );	
	outputInfo output;
	// initialize to 2 because of default waveforms...
	output.waveCount = 2;
	output.predefinedWaveCount = 0;
	output.channel[Vertical].waveData.resize( 2 );
	output.channel[Horizontal].waveData.resize( 2 );
	for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
	{
		output.niawgLanguageScript = "";
		input->comm->sendStatus("Working with configuraiton # " + std::to_string(sequenceInc + 1) + " in Sequence...\r\n");
		/// Create Script and Write Waveforms ////////////////////////////////////////////////////////////////////////////////////////////
		niawgPair<ScriptStream> scripts;
		scripts[Vertical] << scriptFiles[Vertical][sequenceInc].rdbuf();
		scripts[Horizontal] << scriptFiles[Horizontal][sequenceInc].rdbuf();
		std::string warnings, debugMessages;
		try
		{
			input->niawg->analyzeNiawgScripts( scripts, output, libWaveformArray, input->profile, singletons, input->debugInfo, warnings, 
											   debugMessages );
		}
		catch (myException& except)
		{
			input->comm->sendFatalError("analyzeNIAWGScripts() threw an error!\r\n" + except.whatStr());
			delete input;
			return -1;
		}
		// check if there are any warnings or debug messages from the script analysis.
		if (warnings != "")
		{
			input->comm->sendError(warnings);
		}
		if (debugMessages != "")
		{
			input->comm->sendDebug(debugMessages);
		}
		workingUserScriptString[sequenceInc] = output.niawgLanguageScript;
		// 
		if (systemAbortCheck(input->comm))
		{
			for (auto axis : AXES)
			{
				for (auto& waveform : variedWaveforms[axis])
				{
					delete[] waveform;
				}
				for (auto& waveform : variedWaveformFileData[axis])
				{
					delete[] waveform;
				}
			}
			for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
			{
				delete[] mixedWaveforms[deleteInc];
			}
			delete input;
			return 0;
		}
	}
	/// Final Cleanup before waveform initiation
	input->comm->sendStatus("Constant Waveform Preparation Completed...\r\n");
	// format the script to send to the 5451 according to the accumulation number and based on the number of sequences.
	finalUserScriptString = "script " + userScriptNameString + "\n";
	if ((*input).repetitions == 0)
	{
		finalUserScriptString += "repeat forever\n";
		for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
		{
			finalUserScriptString += workingUserScriptString[sequenceInc];
		}
		finalUserScriptString += "end repeat\n";
		if ((*input).debugInfo.outputNiawgMachineScript)
		{
			input->comm->sendDebug(boost::replace_all_copy("Entire NIAWG Machine Script:\n" + finalUserScriptString + "end Script\n\n", "\n", "\r\n"));
		}
	}
	else
	{
		// repeat the script once for every accumulation.
		for (unsigned int accumCount = 0; accumCount < (*input).repetitions; accumCount++)
		{
			for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
			{
				finalUserScriptString += workingUserScriptString[sequenceInc];
			}
			if (accumCount == 0)
			{
				if ((*input).debugInfo.outputNiawgMachineScript)
				{

					input->comm->sendDebug( boost::replace_all_copy( "Single Repetition NIAWG Machine Script:\n"
																		   + finalUserScriptString + "end Script\n\n", "\n", "\r\n" ) );
				}
			}
		}
	}
	// the NIAWG requires that the script file must end with "end script".
	finalUserScriptString += "end Script";
	// Convert script string to ViConstString. +1 for a null character on the end.
	unsigned int scriptLength = boost::numeric_cast<unsigned int>(finalUserScriptString.length() + 1);
	userScriptSubmit = new ViChar[scriptLength];
	sprintf_s(userScriptSubmit, scriptLength, "%s", finalUserScriptString.c_str());
	strcpy_s(userScriptSubmit, scriptLength, finalUserScriptString.c_str());

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Begin Handling Variables
	///

	/// get var files from master if necessary
	if (input->settings.getVariables == true)
	{
		variableValues.resize(varyingParameters.size(), std::vector<double>(0));
		variableValuesLengths.resize(varyingParameters.size());
		char recvbuf[1000];
		int recvbufn = 1000;
		for (int variableNameInc = 0; variableNameInc < varyingParameters.size(); variableNameInc++)
		{
			std::string message = "Waiting for Variable Set #" + std::to_string(variableNameInc + 1) + "... ";

			input->comm->sendStatus(message);
			if (!TWEEZER_COMPUTER_SAFEMODE)    
			{
				iResult = send(ConnectSocket, "next variable", 13, 0);
				if (myErrorHandler(iResult == -1, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) 
									+ "\r\n", &ConnectSocket, scriptFiles, false, userScriptIsWritten, userScriptName, true, false, true, 
									input->comm))
				{
					input->comm->sendFatalError("ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
					delete input;
					return -1;
				}
			}
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				iResult = recv(ConnectSocket, recvbuf, recvbufn, 0);
			}
			else
			{
				// set to 1 to enter the next loop.
				iResult = 1;
			}
			if (iResult > 0)
			{
				input->comm->sendStatus("Received!\r\n");
				int varNameCursor = -1;
				std::string tempVarName;
				std::stringstream variableStream;
				variableStream << recvbuf;
				variableStream >> tempVarName;
				for (int parameterInc = 0; parameterInc < varyingParameters.size(); parameterInc++)
				{
					if (tempVarName == (varyingParameters[parameterInc]).name)
					{
						varNameCursor = parameterInc;
						break;
					}
				}
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					if (myErrorHandler(varNameCursor == -1, "ERROR: The variable name sent by the master computer (" + tempVarName 
										+ ") doesn't match any current variables!\r\n", &ConnectSocket, scriptFiles, false, 
										userScriptIsWritten, userScriptName, true, false, true, input->comm))
					{
						input->comm->sendFatalError("ERROR: The variable name sent by the master computer (" 
							+ tempVarName + ") doesn't match any current variables!\r\n");
						delete input;
						return -1;
					}
				}
				// WRITE VAR VALUES
				int j = 0;
				std::string tempString;
				double tempDouble;
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					// only exit while loop after recieving "done!" message.
					while (true)
					{
						variableStream >> tempString;
						if (tempString == "done!")
						{
							break;
						}
						tempDouble = stod(tempString);
						// if it's a time value, check to make sure it's valid.
						if (varyingParameters[varNameCursor].timelike)
						{
							if (input->niawg->waveformSizeCalc(tempDouble) % 4 != 0)
							{
								input->comm->sendFatalError("ERROR: a timelike value sent by the master computer did not correspond to an integer number of 4 samples. The "
									"value was " + std::to_string(tempDouble) + "\r\n");
								delete input;
								return -1;
							}
						}
						variableValues[varNameCursor].resize(j + 1);
						variableValues[varNameCursor][j] = tempDouble;
						j++;
					}
					variableValuesLengths[varNameCursor] = j;
				}
				else if (TWEEZER_COMPUTER_SAFEMODE)
				{
					// set all values to zero. This should work for the majority of variables possible.
					variableValues[variableNameInc].resize(3);
					variableValues[variableNameInc][0] = 0;
					variableValues[variableNameInc][1] = 0;
					variableValues[variableNameInc][2] = 0;
					variableValuesLengths[variableNameInc] = 3;
				}
				// tell the master to continue
			}
			else if (iResult == 0)
			{
				if (myErrorHandler(-1, "ERROR: The connection with the master computer was closed!\r\n",&ConnectSocket, scriptFiles,
									false, userScriptIsWritten, userScriptName, true, false, true, input->comm))
				{
					input->comm->sendFatalError("ERROR: The connection with the master computer was closed!\r\n");
					delete input;
					return -1;
				}
			}
			else
			{
				if (myErrorHandler(-1, "ERROR: Socket Recieve failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n",&ConnectSocket,
									scriptFiles, false, userScriptIsWritten, userScriptName, true, false, true, input->comm))
				{
					input->comm->sendFatalError("ERROR: Socket Recieve failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
					delete input;
					return -1;
				}
				input->comm->sendFatalError("ERROR: Socket Recieve failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
				delete input;
				return -1;
			}
		}
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Logging
	///
	if (true)
	//if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		// This report goes to a folder I create on the Andor. NEW: Always log.
		input->comm->sendStatus("Logging Script and Experiment Parameters...\r\n");
		std::string verticalScriptLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Vertical Script.txt";
		std::string horizontalScriptLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Horizontal Script.txt";
		std::string intensityLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Intensity Script.txt";
		std::string parametersFileLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Parameters.txt";

		bool andorConnected = false;
		/// Log the vertical script
		do 
		{
			std::ofstream verticalScriptLog(verticalScriptLogPath);
			std::string verticalScriptText = "\n\n====================\nVertical Script Being Used:\n====================\n";
			if (verticalScriptLog.is_open() == false)
			{
				int andorDisconnectedOption = MessageBox(NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
					"current script output sequence and will keep the default waveform running."
					" (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
					"without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE);
				switch (andorDisconnectedOption)
				{
					case IDABORT:
					{
						if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n",&ConnectSocket,
							scriptFiles, false, userScriptIsWritten, userScriptName, true, false, true, input->comm))
						{
							input->comm->sendFatalError("ERROR: Andor Disconected. User Aborted.\r\n");
							delete input;
							return -1;
						}
						// doesn't get reached.
						break;

					}
					case IDRETRY:
					{
						break;
					}
					case IDIGNORE:
					{
						// break out without writing file.
						andorConnected = true;
						break;
					}
				}
			}
			for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
			{
				verticalScriptFiles[sequenceInc].clear();
				verticalScriptFiles[sequenceInc].seekg(0, std::ios::beg);
				verticalScriptText += "***Configuration [" + (*input).sequenceFileNames[sequenceInc] 
										+ "] Vertical NIAWG Script***\n";
				std::stringstream tempStream;
				tempStream << verticalScriptFiles[sequenceInc].rdbuf();
				verticalScriptText += tempStream.str();
				verticalScriptFiles[sequenceInc].clear();
				verticalScriptFiles[sequenceInc].seekg(0, std::ios::beg);
			}
			verticalScriptText += "\n";
			// make sure all line endings are \r\signal.
			boost::replace_all(verticalScriptText, "\r", "");
			boost::replace_all(verticalScriptText, "\n", "\r\n");
			if (andorConnected && !TWEEZER_COMPUTER_SAFEMODE)
			{
				verticalScriptLog << verticalScriptText;
			}
			if ((*input).debugInfo.outputNiawgHumanScript)
			{
				input->comm->sendDebug(verticalScriptText);
			}

			andorConnected = true;

		} while (andorConnected == false);

		andorConnected = false;
		/// Log the horizontal script
		do {
			std::ofstream horizontalScriptLog(horizontalScriptLogPath);
			std::string horizontalScriptText = "\n\n====================\nHorizontal Script Being Used:\n====================\n";

			if (horizontalScriptLog.is_open() == false)
			{
				int andorDisconnectedOption = MessageBox(NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
					"current script output sequence and will keep the default waveform running."
					" (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
					"without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE);
				switch (andorDisconnectedOption)
				{
				case IDABORT:
				{
					if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, 
						userScriptIsWritten, userScriptName, true, false, true, input->comm))
					{
						input->comm->sendFatalError("ERROR: Andor Disconected. User Aborted.\r\n");
						delete input;
						return -1;
					}
					// doesn't get reached.
					break;
				}
				case IDRETRY:
				{
					break;
				}
				case IDIGNORE:
				{
					// break out without writing file.
					andorConnected = true;
					break;
				}
				}
			}
			
				
			for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
			{
				horizontalScriptFiles[sequenceInc].clear();
				horizontalScriptFiles[sequenceInc].seekg(0, std::ios::beg);
				horizontalScriptText += "***Configuration [" + (*input).sequenceFileNames[sequenceInc]
					+ "] Horizontal NIAWG Script***\n";
				std::stringstream tempStream;
				tempStream << horizontalScriptFiles[sequenceInc].rdbuf();
				horizontalScriptText += tempStream.str();

				//horizontalScriptLog << "***Configuration [" + (*input).sequenceFileNames[sequenceInc] + "] Horizontal NIAWG Script***\signal";
				//horizontalScriptLog << horizontalScriptFiles[sequenceInc].rdbuf();
				horizontalScriptFiles[sequenceInc].clear();
				horizontalScriptFiles[sequenceInc].seekg(0, std::ios::beg);
			}
			horizontalScriptText += "\n";
			// make sure all line endings are \r\signal.
			boost::replace_all(horizontalScriptText, "\r", "");
			boost::replace_all(horizontalScriptText, "\n", "\r\n");
			if (andorConnected && !TWEEZER_COMPUTER_SAFEMODE)
			{
				horizontalScriptLog << horizontalScriptText;
			}
			if ((*input).debugInfo.outputNiawgHumanScript)
			{
				input->comm->sendDebug(horizontalScriptText);
			}
			andorConnected = true;
		} while (andorConnected == false);

		andorConnected = false;
		/// Log the intensity script
		do
		{
			std::ofstream intensityScriptLog(intensityLogPath);
			std::string intensityScriptText = "\n\n====================\nIntensity Script Being Used:\n====================\n";
			if (intensityScriptLog.is_open() == false)
			{
				int andorDisconnectedOption = MessageBox(NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
					"current script output sequence and will keep the default waveform running."
					" (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
					"without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE);
				switch (andorDisconnectedOption)
				{
					case IDABORT:
					{
						if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, 
							userScriptIsWritten, userScriptName, true, false, true, input->comm))
						{
							input->comm->sendFatalError("ERROR: Andor Disconected. User Aborted.\r\n");
							delete input;
							return -1;
						}
						// doesn't get reached.
						break;
					}
					case IDRETRY:
					{
						continue;
					}
					case IDIGNORE:
					{
						// break out without writing file.
						andorConnected = true;
						break;
					}
				}
			}

				
			for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
			{
				intensityScriptFiles[sequenceInc].clear();
				intensityScriptFiles[sequenceInc].seekg(0, std::ios::beg);

				intensityScriptText += "***Configuration [" + (*input).sequenceFileNames[sequenceInc]
					+ "] Intensity NIAWG Script***\n";
				std::stringstream tempStream;
				tempStream << intensityScriptFiles[sequenceInc].rdbuf();
				intensityScriptText += tempStream.str();
				//intensityScriptLog << "***Configuration [" + (*input).sequenceFileNames[sequenceInc] + "] intensity Script***\signal";
				//intensityScriptLog << intensityScriptFiles[sequenceInc].rdbuf();
				intensityScriptFiles[sequenceInc].clear();
				intensityScriptFiles[sequenceInc].seekg(0, std::ios::beg);
			}
			intensityScriptText += "\n";
			// make sure all line endings are \r\signal.
			boost::replace_all(intensityScriptText, "\r", "");
			boost::replace_all(intensityScriptText, "\n", "\r\n");
			if (andorConnected && !TWEEZER_COMPUTER_SAFEMODE)
			{
				intensityScriptLog << intensityScriptText;
			}
			if ((*input).debugInfo.outputAgilentScript)
			{
				input->comm->sendDebug(intensityScriptText);
			}
			andorConnected = true;
		} while (andorConnected == false);

		andorConnected = false;
		/// Log other parameters
		do
		{
			std::ofstream parametersFileLog(parametersFileLogPath);
			if (parametersFileLog.is_open() == false)
			{
				int andorDisconnectedOption = MessageBox(NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
					"current script output sequence and will keep the default waveform running."
					" (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
					"without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE);
				switch (andorDisconnectedOption)
				{
					case IDABORT:
					{
						if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, 
							userScriptIsWritten, userScriptName, true, false, true, input->comm))
						{
							input->comm->sendFatalError("ERROR: Andor Disconected. User Aborted.\r\n");
							delete input;
							return -1;
						}
						// doesn't get reached.
						break;
					}
					case IDRETRY:
					{
						break;
					}
					case IDIGNORE:
					{
						// break out without writing file.
						andorConnected = true;
						break;
					}
				}
			}
			else
			{
				andorConnected = true;
				// prepare the parameters list for the log
				std::string paramtersString = "\nDont Actually Generate = " + std::to_string((*input).dontActuallyGenerate)
					+ "\nConnect To Master = " + std::to_string(input->settings.connectToMaster)
					+ "\nGet Variable Files From Master = " + std::to_string(input->settings.getVariables)
					+ "\nRepetitions = " + std::to_string((*input).repetitions)
					+ "\nDon't Actually Generate = " + std::to_string((*input).dontActuallyGenerate)
					+ "\nProgramming Intensity = " + std::to_string(input->settings.programIntensity)
					+ "\nSequence File Names = \n";

				for (unsigned int seqInc = 0; seqInc < (*input).sequenceFileNames.size(); seqInc++)
				{
					paramtersString += "\t" + (*input).sequenceFileNames[seqInc] + "\n";
				}
				parametersFileLog << paramtersString;
			}
		} while (andorConnected == false);
	}
	/// Variable Cleanup Before Execution
	// check that variable files are the same length.
	for (std::size_t varNameInc = 0; varNameInc + 1 < variableValuesLengths.size(); varNameInc++)
	{
		if (variableValuesLengths[varNameInc] != variableValuesLengths[varNameInc + 1])
		{
			if (myErrorHandler(-1, "Error: lengths of variable values are not all the same! They must be the same\r\n",&ConnectSocket, verticalScriptFiles,
				horizontalScriptFiles, false,   userScriptIsWritten, userScriptName, true, false, true, input->comm))
			{
				input->comm->sendFatalError("Error: lengths of variable values are not all the same! They must be the same\r\n");
				delete input;
				return -1;
			}
		}
	}

	std::vector<std::vector<POINT>> intensityPoints;
	if (input->settings.programIntensity == true)
	{
		input->comm->sendStatus("Programing Intensity Profile(s)...");
		colorBoxes<char> colors = { /*niawg*/'-', /*camera*/'-', /*intensity*/'Y' };
		input->comm->sendColorBox( colors );
		if (myErrorHandler(myAgilent::programIntensity(boost::numeric_cast<int>(varyingParameters.size()), varyingParameters, variableValues, intIsVaried,
			intensitySequenceMinAndMaxVector, intensityPoints, intensityScriptFiles, singletons, input->profile),
			"ERROR: Intensity Programming Failed!\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false,  
			userScriptIsWritten, userScriptName, /*Socket Active = */true, false, true, input->comm))
		{
			input->comm->sendFatalError("ERROR: Intensity Programming Failed!\r\n");
			delete input;
			return -1;
		}

		input->comm->sendStatus("Complete!\r\n");
		// select the first one.
		if (myErrorHandler(myAgilent::selectIntensityProfile(0, intIsVaried, intensitySequenceMinAndMaxVector),
			"ERROR: intensity profile selection failed!\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, 
			userScriptIsWritten, userScriptName, true, false, true, input->comm))
		{
			input->comm->sendFatalError("ERROR: intensity profile selection failed!\r\n");
			delete input;
			return -1;
		}
		input->comm->sendStatus("Intensity Profile Selected.\r\n");
	}
	else
	{
		input->comm->sendStatus("Intensity Profile NOT being programed.\r\n");
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Big Variable Handling Loop
	///
	// If there are any varible files, enter big variable loop.
	if (varyingParameters.size() > 0)
	{
		input->comm->sendStatus("Begin Variable & Execution Loop.\r\n");
		// create event thread
		eWaitingForNIAWGEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("eWaitingForNIAWGEvent"));
		time_t timeObjCurr;
		struct tm currentTimeCurr;
		long timePrev, timeCurr = 0;
		// for every variable value in the files...
		for (std::size_t varValueLengthInc = 0; varValueLengthInc < variableValuesLengths[0]; varValueLengthInc++)
		{
			timePrev = timeCurr;
			// get time now
			timeObjCurr = time(0);
			localtime_s(&currentTimeCurr, &timeObjCurr);
			timeCurr = currentTimeCurr.tm_sec + currentTimeCurr.tm_min * 60 + currentTimeCurr.tm_hour * 3600;
			if (varValueLengthInc > 0)
			{
				long timeToCompleteS = (timeCurr - timePrev) * (variableValuesLengths[0] - varValueLengthInc);
				long timeToCompleteM = timeToCompleteS / 60;
				timeToCompleteS = timeToCompleteS % 60;
				input->comm->sendStatus("Approximate Time To Complete: " + std::to_string(timeToCompleteM) + ":"
											  + std::to_string(timeToCompleteS) + " (min:sec)\r\n");
			}
			// Tell the main thread that another variable is being written.
			//PostMessage(eMainWindowHandle, eVariableStatusMessageID, 0, varNameInc);

			verticalVarWriteCount = 0;
			horizontalVarWriteCount = 0;
			mixedCount = 0;
			mixedWriteCount = 0;
			// for every waveform...
			/// I think waveformInc = 0 is the default... should I be handling that at all??? shouldn't make a difference I don't think. ????????????????????????
			for (int waveformInc = 0; waveformInc < output.waveCount; waveformInc++)
			{
			// oh god... why am I using a goto???
			BeginningOfWaveformLoop:
				// change the waveform, if needed.
				/// if need to change both... /////////////////////////////////////////////////////////////////////////////////////////////////////////
				if (output.channel[Vertical].waveData[waveformInc].isVaried == true 
					 && output.channel[Horizontal].waveData[waveformInc].isVaried == true)
				{
					// For all Variables...
					for (std::size_t variableInc = 0; variableInc < varyingParameters.size() + 1; variableInc++)
					{
						std::string currentVar;
						// Set variable name. Should be the same between variableNames and yVarNames.
						if (variableInc < varyingParameters.size())
						{
							currentVar = varyingParameters[variableInc].name;
						}
						else
						{
							// This is the reserved character for a variable time resulting from waveform correction.
							// This value is always set once to check for this because variableInc loops to and including variableNames.size().
							currentVar = '\'';
						}
						// will check this later.
						std::string warnings;
						for (auto axis : AXES)
						{
							/// Loop for varibles in given Waveform
							for (int variableNumber = 0; variableNumber < output.channel[axis].waveData[waveformInc].varNum; variableNumber++)
							{
								// if parameter's variable matches current variable...
								if (output.channel[axis].waveData[waveformInc].varNames[variableNumber] == currentVar)
								{
									double variableValue;
									if (variableInc < varyingParameters.size())
									{
										variableValue = variableValues[variableInc][varValueLengthInc];
									}
									else
									{
										// dummy value. Not used in this case.
										variableValue = 0;
									}
									/// For Correction Waveforms...
									// The vartypes value for a correction waveform is -3.
									// Structure is weird for waveforms correcting the next waveform.
									if (output.channel[axis].waveData[waveformInc].varTypes[variableNumber] == -3)
									{
										// set it to -4 so that it is correctly recognized the next time through.
										output.channel[axis].waveData[waveformInc].varTypes[variableNumber] = -4;
										// The corresponding x variable might be in a different location; I need to find that location
										for (int x_min_3_loc = 0; x_min_3_loc < output.channel[ALT_AXES[axis]].waveData[waveformInc].varNum; x_min_3_loc++)
										{
											if (output.channel[ALT_AXES[axis]].waveData[waveformInc].varTypes[x_min_3_loc] == -3)
											{
												output.channel[ALT_AXES[axis]].waveData[waveformInc].varTypes[x_min_3_loc] = -4;
											}

										}
										/// Set all the parameters as if this had exited normally.
										// increment the waveform being corrected.
										waveformInc++;
										// increment the number of vertical, horizontal, and mixed waveforms that have been "written"
										verticalVarWriteCount++;
										horizontalVarWriteCount++;
										mixedCount++;
										// add extra spots for waveforms here.
										horizontalVariedWaveforms.resize( horizontalVariedWaveforms.size() + 1 );
										horizontalVariedWaveformReads.resize( horizontalVariedWaveformReads.size() + 1 );
										verticalVariedWaveforms.resize( verticalVariedWaveforms.size() + 1 );
										verticalVariedWaveformReads.resize( verticalVariedWaveformReads.size() + 1 );
										variedMixedSize.resize( variedMixedSize.size() + 1 );
										mixedWaveforms.resize( mixedWaveforms.size() + 1 );
										// this flag lets the code know that the next waveform is being corrected.
										correctionFlag = true;
										// Return to the beginning of the for loop without actually writing.
										goto BeginningOfWaveformLoop;
									}
									/// Not correction waveform...
									// change parameters, depending on the case. The varTypes and variable Value sets were set previously.
									try
									{
										input->niawg->varyParam( output.hor.waveData, output.vert.waveData, waveformInc,
																 output.hor.waveData[waveformInc].varTypes[variableNumber],
																 variableValue, warnings );
									}
									catch (myException& exception)
									{
										input->comm->sendFatalError( "ERROR: varyParam() returned an error: " + exception.whatStr()
																	 + "\r\n" );
										delete input;
										return -1;
									}
								} // end "if variables match"
							} // end "parameters loop"
						}
						/// Loop for jth X Waveform
						for (int variableNumber = 0; variableNumber < output.vert.waveData[waveformInc].varNum; variableNumber++)
						{
							// if parameter's variable matches variable...
							if (output.vert.waveData[waveformInc].varNames[variableNumber] == currentVar)
							{
								double variableValue;
								// change parameters, depending on the case. This was set during input reading.
								if (variableInc < varyingParameters.size())
								{
									variableValue = variableValues[variableInc][varValueLengthInc];
								}
								else
								{
									// dummy value. Not used in this case.
									variableValue = 0;
								}
								/// For Correction Waveform...
								// The vartypes value for a correction waveform is -3.
								// Structure is weird for waveforms correcting the next waveform.
								/// !!! This one shouldn't actually happen, since it should get caught in the test in the Y waveform, but for symmetry...
								if (output.vert.waveData[waveformInc].varTypes[variableNumber] == -3)
								{
									input->comm->sendFatalError( "ERROR: The code has entered a part of "
																	   "the code which it should never enter, indicating a logic error somewhere. Search \"Error location #1\" in "
																	   "the code to find this location. The code will continue to run, but is likely about to crash." );
									delete input;
									return -1;
									// set it to -4 so that it is correctly recognized the next time through.
									output.vert.waveData[waveformInc].varTypes[variableNumber] = -4;
									output.hor.waveData[waveformInc].varTypes[variableNumber] = -4;
									// increment the waveform being corrected.
									waveformInc++;
									// increment the number of x, y, and mixed waveforms that have been "written"
									verticalVarWriteCount++;
									horizontalVarWriteCount++;
									mixedCount++;
									// add extra spots for waveforms here.
									horizontalVariedWaveforms.resize( horizontalVariedWaveforms.size() + 1 );
									horizontalVariedWaveformReads.resize( horizontalVariedWaveformReads.size() + 1 );
									verticalVariedWaveforms.resize( verticalVariedWaveforms.size() + 1 );
									verticalVariedWaveformReads.resize( verticalVariedWaveformReads.size() + 1 );
									variedMixedSize.resize( variedMixedSize.size() + 1 );
									mixedWaveforms.resize( mixedWaveforms.size() + 1 );
									// this flag lets the code know that the next waveform is being corrected.
									correctionFlag = true;
									// Return to the beginning of the for loop without
									goto BeginningOfWaveformLoop;
								}
								// change parameters, depending on the case. The varTypes and variable Value sets were set previously.
								try
								{
									input->niawg->varyParam( output.vert.waveData, output.hor.waveData, waveformInc, 
																   output.vert.waveData[waveformInc].varTypes[variableNumber],
																   variableValue, warnings );
								}
								catch (myException& exception)
								{
									input->comm->sendFatalError( "ERROR: varyParam() returned an error: " + exception.whatStr() 
																	   + ".\r\n" );
									delete input;
									return -1;
								}
							} // end if variables match
						} // end parameters loop
						if (warnings != "")
						{
							input->comm->sendError(warnings);
						}
					} // end variables loop

					// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform". You only get here if 
					// flagged for it during read.
					for (int signal = 0; signal < output.vert.waveData[waveformInc].signalNum; signal++)
					{
						if (output.vert.waveData[waveformInc].signals[signal].initPhase == -1)
						{
							output.vert.waveData[waveformInc].signals[signal].phaseOption = -1;
							if (signal + 1 > output.vert.waveData[waveformInc - 1].signalNum) {
								if (myErrorHandler(-1, "ERROR: You are trying to copy the phase of the " + std::to_string(signal + 1) + " signal of the "
									"previous waveform, but the previous waveform only had "
									+ std::to_string( output.vert.waveData[waveformInc - 1].signalNum) + " signals!\r\n",&ConnectSocket,
									verticalScriptFiles, horizontalScriptFiles, false,   userScriptIsWritten, userScriptName,
									true, false, input->settings.connectToMaster, input->comm))
								{
									input->comm->sendFatalError("ERROR: You are trying to copy the phase of "
										"the " + std::to_string(signal + 1) + " signal of the previous waveform, but the previous waveform only had "
										+ std::to_string( output.vert.waveData[waveformInc - 1].signalNum) + " signals!\r\n");
									delete input;
									return -1;
								}
							}
							output.vert.waveData[waveformInc].signals[signal].initPhase = output.vert.waveData[waveformInc - 1].signals[signal].finPhase;
						}
					}

					// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform". You only get here if 
					// flagged for it during read.
					for (int signal = 0; signal < output.hor.waveData[waveformInc].signalNum; signal++)
					{
						if (output.hor.waveData[waveformInc].signals[signal].initPhase == -1)
						{
							output.hor.waveData[waveformInc].signals[signal].phaseOption = -1;
							if (signal + 1 > output.hor.waveData[waveformInc - 1].signalNum)
							{
								if (myErrorHandler(-1, "ERROR: You are trying to copy the phase of the " + std::to_string(signal + 1) + " signal of the "
									"previous waveform, but the previous waveform only had " + std::to_string( output.hor.waveData[waveformInc - 1].signalNum)
									+ " signals!\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false,   userScriptIsWritten,
									userScriptName, true, false, true, input->comm))
								{
									input->comm->sendFatalError("ERROR: You are trying to copy the phase of "
										"the " + std::to_string(signal + 1) + " signal of the previous waveform, but the previous waveform only had "
										+ std::to_string( output.hor.waveData[waveformInc - 1].signalNum) + " signals!\r\n");
									delete input;
									return -1;
								}
							}
							output.hor.waveData[waveformInc].signals[signal].initPhase = output.hor.waveData[waveformInc - 1].signals[signal].finPhase;
						}
					}
					// write the new waveform.//////////
					// If the correction flag is set, that means that the previous waveform was already passed by, and that I need to address a particular
					// entry of the data, not the next entry.
					if (repeatFlag == true || rewriteFlag == true)
					{
						if (rewriteFlag == true)
						{
							// In this case, these entries in the waveform vectors have already been written. Using new again won't overwrite the 
							// previous data, it will assign the pointer to a new set of data, and the pointer to the old set is lost: memory leak. 
							// Therefore I must delete the old waveforms here.
							delete[] horizontalVariedWaveforms[horizontalVarWriteCount];
							delete[] horizontalVariedWaveformReads[horizontalVarWriteCount];
							delete[] verticalVariedWaveforms[verticalVarWriteCount];
							delete[] verticalVariedWaveformReads[verticalVarWriteCount];
						}
						horizontalVariedWaveforms[horizontalVarWriteCount] = new ViReal64[output.hor.waveData[waveformInc].sampleNum];
						horizontalVariedWaveformReads[horizontalVarWriteCount] = new ViReal64[output.hor.waveData[waveformInc].sampleNum 
							+ output.hor.waveData[waveformInc].signalNum];
						verticalVariedWaveforms[verticalVarWriteCount] = new ViReal64[output.vert.waveData[waveformInc].sampleNum];
						verticalVariedWaveformReads[verticalVarWriteCount] = new ViReal64[output.vert.waveData[waveformInc].sampleNum 
							+ output.vert.waveData[waveformInc].signalNum];
					}
					else if (repeatFlag == false && rewriteFlag == false)
					{
						// this is equivalent to an else.
						horizontalVariedWaveforms.push_back(new ViReal64[output.hor.waveData[waveformInc].sampleNum]);
						horizontalVariedWaveformReads.push_back(new ViReal64[output.hor.waveData[waveformInc].sampleNum + output.hor.waveData[waveformInc].signalNum]);
						verticalVariedWaveforms.push_back(new ViReal64[output.vert.waveData[waveformInc].sampleNum]);
						verticalVariedWaveformReads.push_back(new ViReal64[output.vert.waveData[waveformInc].sampleNum + output.vert.waveData[waveformInc].signalNum]);
					}
					std::string debugMsg;
					input->niawg->getVariedWaveform( output.vert.waveData[waveformInc], output.vert.waveData, 
														   waveformInc, libWaveformArray, verticalVariedWaveforms[verticalVarWriteCount], 
														   input->debugInfo, debugMsg);
					input->niawg->getVariedWaveform( output.hor.waveData[waveformInc], output.hor.waveData, 
														   waveformInc, libWaveformArray, horizontalVariedWaveforms[horizontalVarWriteCount], 
														   input->debugInfo, debugMsg);
					if (debugMsg != "")
					{
						input->comm->sendDebug(debugMsg);
					}
					if (repeatFlag == true || rewriteFlag == true)
					{
						if (rewriteFlag == true)
						{
							// see above comment about deleting in this case.
							delete[] mixedWaveforms[mixedCount];
						}
						variedMixedSize[mixedCount] = 2 * output.vert.waveData[waveformInc].sampleNum;
						mixedWaveforms[mixedCount] = new ViReal64[variedMixedSize[mixedCount]];
					}
					else if (repeatFlag == false && rewriteFlag == false)
					{
						variedMixedSize.push_back(2 * output.vert.waveData[waveformInc].sampleNum);
						mixedWaveforms.push_back(new ViReal64[variedMixedSize[mixedCount]]);
					}
					input->niawg->mixWaveforms(verticalVariedWaveforms[verticalVarWriteCount], 
													  horizontalVariedWaveforms[horizontalVarWriteCount], 
													  mixedWaveforms[mixedCount],
													  output.vert.waveData[waveformInc].sampleNum);

					// reset initial phase if needed.
					if (waveformInc != 0)
					{
						if (output.vert.waveData[waveformInc - 1].phaseManagementOption == 1)
						{
							for (int signal = 0; signal < output.vert.waveData[waveformInc].signalNum; signal++)
							{
								output.vert.waveData[waveformInc].signals[signal].initPhase = 0;
							}
						}
						if (output.hor.waveData[waveformInc - 1].phaseManagementOption == 1)
						{
							for (int signal = 0; signal < output.hor.waveData[waveformInc].signalNum; signal++)
							{
								output.vert.waveData[waveformInc].signals[signal].initPhase = 0;
							}
						}
					}

					if (correctionFlag == true)
					{
						// then I need to go back.
						// Set the correctionFlag back to false so that I don't go back again unless it is called for.
						correctionFlag = false;
						waveformInc -= 2;
						verticalVarWriteCount -= 2;
						horizontalVarWriteCount -= 2;
						mixedCount -= 2;
						repeatFlag = true;
					}
					else if (repeatFlag == true)
					{
						repeatFlag = false;
						rewriteFlag = true;
					}
					else if (rewriteFlag == true)
					{
						// only safe because in else if. If this was another if, it would always get immediately set to false if true.
						rewriteFlag = false;
					}
					// normal exit.
					verticalVarWriteCount++;
					horizontalVarWriteCount++;
					mixedCount++;
				}
				// End WaveformLoop
			}
			
			/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///
			///		A series of sanity checks on the waveform parameters. This is ment to catch user error. The following checks for...
			///		- Phase Continuity between waveforms (not checking repeating waveforms (yet))
			///		- Amplitude Continuity between waveforms
			///		- Frequency Continuity between waveforms
			///		- Sensible Ramping Options (initial and final freq/amp values reflect choice of ramp or no ramp).
			///		- Sensible Phase Correction Options
			/// 			
			for (int contInc = 2; contInc < output.vert.waveData.size(); contInc++)
			{
				// if two waveforms have the same number of parameters...
				if (output.vert.waveData[contInc].signals.size() == output.vert.waveData[contInc - 1].signals.size())
				{
					for (int waveInc = 0; waveInc < output.vert.waveData[contInc].signals.size(); waveInc++)
					{
						if (output.vert.waveData[contInc].signals[waveInc].initPower != output.vert.waveData[contInc - 1].signals[waveInc].finPower)
						{
							std::string message = "Warning: Amplitude jump at waveform #" + std::to_string(contInc) + " in X component detected!\r\n";
							input->comm->sendError(message);
						}
						if (output.vert.waveData[contInc].signals[waveInc].freqInit != output.vert.waveData[contInc - 1].signals[waveInc].freqFin)
						{
							std::string message = "Warning: Frequency jump at waveform #" + std::to_string(contInc) + " in X component detected!\r\n";
							input->comm->sendError(message);
						}
						if (output.vert.waveData[contInc].signals[waveInc].initPhase - output.vert.waveData[contInc - 1].signals[waveInc].finPhase
							> CORRECTION_WAVEFORM_ERROR_THRESHOLD)
						{
							std::string message = "Warning: Phase jump (greater than what's wanted for correction waveforms) at waveform #" + std::to_string(contInc) + " in X component "
								"detected!\r\n";
							input->comm->sendError(message);
						}
						// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
						if (output.vert.waveData[contInc].signals[waveInc].powerRampType != "nr"
							&& (output.vert.waveData[contInc].signals[waveInc].initPower == output.vert.waveData[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to amplitude ramp, but the initial and final "
								"amplitudes are the same. This is not a ramp.\r\n";
							input->comm->sendError(message);
						}
						// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
						if (output.vert.waveData[contInc].signals[waveInc].freqRampType != "nr"
							&& (output.vert.waveData[contInc].signals[waveInc].freqInit == output.vert.waveData[contInc].signals[waveInc].freqFin))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to frequency ramp, but the initial and final "
								"frequencies are the same. This is not a ramp.\r\n";
							input->comm->sendError(message);
						}

						// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually ramping.
						if (output.vert.waveData[contInc].signals[waveInc].powerRampType == "nr"
							&& (output.vert.waveData[contInc].signals[waveInc].initPower != output.vert.waveData[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to no amplitude ramp, but the initial and final "
								"amplitudes are the different. This is not a ramp, the initial value will be used.\r\n";
							input->comm->sendError(message);
						}
						// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
						if (output.vert.waveData[contInc].signals[waveInc].freqRampType == "nr"
							&& (output.vert.waveData[contInc].signals[waveInc].freqInit != output.vert.waveData[contInc].signals[waveInc].freqInit))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to no frequency ramp, but the initial and final "
								"frequencies are different. This is not a ramp, the initial value will be used throughout.\r\n";
							input->comm->sendError(message);
						}
						if (output.vert.waveData[contInc].phaseManagementOption == -1 && output.vert.waveData[contInc].signals[waveInc].phaseOption != -1)
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set correct the phase of the previous waveform, "
								"but is not using the final phase of the previous waveform. If you want phase continuity, set the initial phase of this waveform"
								" to be -1, which is code for grabbing the final phase of the previous waveform.\r\n";
							input->comm->sendError(message);
						}
						if (contInc != 0)
						{
							// check if user is trying to copy the final phase of the correction waveform. You aren't supposed to do that.
							if (output.vert.waveData[contInc - 1].phaseManagementOption == 1 && output.vert.waveData[contInc].signals[waveInc].phaseOption != 0)
							{
								std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set correct the phase of the following waveform, "
									"but the following waveform is grabbing the phase of this correction waveform. It's not supposed to do that, it needs to"
									" start at zero phase in order for this to work correctly.\r\n";
								input->comm->sendError(message);
							}
						}
					}
				}
				if (output.hor.waveData[contInc].signals.size() == output.hor.waveData[contInc - 1].signals.size())
				{
					for (int waveInc = 0; waveInc < output.hor.waveData[contInc].signals.size(); waveInc++) {
						if (output.hor.waveData[contInc].signals[waveInc].initPower != output.hor.waveData[contInc - 1].signals[waveInc].finPower)
						{
							std::string message = "Warning: Amplitude jump at waveform #" + std::to_string(contInc) + " in Y component detected!\r\n";
							input->comm->sendError(message);
						}
						if (output.hor.waveData[contInc].signals[waveInc].freqInit != output.hor.waveData[contInc - 1].signals[waveInc].freqFin)
						{
							std::string message = "Warning: Frequency jump at waveform #" + std::to_string(contInc) + " in Y component detected!\r\n";
							input->comm->sendError(message);
						}
						if (output.hor.waveData[contInc].signals[waveInc].initPhase - output.hor.waveData[contInc - 1].signals[waveInc].finPhase
							> CORRECTION_WAVEFORM_ERROR_THRESHOLD)
						{
							std::string message = "Warning: Phase jump (greater than what's wanted for correction waveforms) at waveform #" + std::to_string(contInc) + " in Y component "
								"detected!\r\n";
							input->comm->sendError(message);
						}
						// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
						if (output.hor.waveData[contInc].signals[waveInc].powerRampType != "nr"
							&& (output.hor.waveData[contInc].signals[waveInc].initPower == output.hor.waveData[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to amplitude ramp, but the initial and final "
								"amplitudes are the same. This is not a ramp.\r\n";
							input->comm->sendError(message);
						}
						// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
						if (output.hor.waveData[contInc].signals[waveInc].freqRampType != "nr"
							&& (output.hor.waveData[contInc].signals[waveInc].freqInit == output.hor.waveData[contInc].signals[waveInc].freqFin))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to frequency ramp, but the initial and final "
								"frequencies are the same. This is not a ramp.\r\n";
							input->comm->sendError(message);
						}
						// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually ramping.
						if (output.hor.waveData[contInc].signals[waveInc].powerRampType == "nr"
							&& (output.hor.waveData[contInc].signals[waveInc].initPower != output.hor.waveData[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to no amplitude ramp, but the initial and final "
								"amplitudes are the different. This is not a ramp, the initial value will be used.\r\n";
							input->comm->sendError(message);
						}
						// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
						if (output.hor.waveData[contInc].signals[waveInc].freqRampType == "nr"
							&& (output.hor.waveData[contInc].signals[waveInc].freqInit != output.hor.waveData[contInc].signals[waveInc].freqInit))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to no frequency ramp, but the initial and final "
								"frequencies are different. This is not a ramp, the initial value will be used throughout.\r\n";
							input->comm->sendError(message);
						}
						if (output.hor.waveData[contInc].phaseManagementOption == -1 && output.hor.waveData[contInc].signals[waveInc].phaseOption != -1)
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set correct the phase of the previous waveform, "
								"but is not using the final phase of the previous waveform. If you want phase continuity, set the initial phase of this waveform"
								" to be -1, which is code for grabbing the final phase of the previous waveform.\r\n";
							input->comm->sendError(message);
						}
						if (contInc != 0)
						{
							// check if user is trying to copy the final phase of the correction waveform. You aren't supposed to do that.
							if (output.hor.waveData[contInc - 1].phaseManagementOption == 1 && output.hor.waveData[contInc].signals[waveInc].phaseOption != 0)
							{
								std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set correct the phase of the following waveform, "
									"but the following waveform is grabbing the phase of this correction waveform. It's not supposed to do that, it needs to"
									" start at zero phase in order for this to work correctly.\r\n";
								input->comm->sendError(message);
							}
						}
					}
				}
			}
			/// Wait until previous script has finished.
			// Update General Status Text & Color.
			std::string message = "Outputting Series #" + std::to_string(varValueLengthInc) + ". Ready and Waiting to Initialize Script from series #"
				+ std::to_string(varValueLengthInc + 1) + ".\r\n";
			for (int varInc = 0; varInc < varyingParameters.size(); varInc++)
			{
				// skip the first time.
				if (varValueLengthInc != 0)
				{
					message += varyingParameters[varInc].name + " = " + std::to_string(variableValues[varInc][varValueLengthInc - 1]) + "; ";
				}
			}
			colorBoxes<char> colors = { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' };
			input->comm->sendColorBox( colors );
			input->comm->sendStatus( message );
			/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///
			///					wait until done
			///
			isDoneTest = VI_FALSE;
			// If running the default script, stop the default script so that a moment later, when the program checks if the output is done, the output
			// will be done.
			if ((*input).currentScript == "DefaultScript" && (*input).dontActuallyGenerate == false)
			{

				try
				{
					input->niawg->configureOutputEnabled(VI_FALSE);
					input->niawg->abortGeneration();
				}
				catch (myException& excep)
				{
					input->comm->sendStatus("EXITED WITH ERROR!\r\n");
					input->comm->sendError("EXITED WITH ERROR!\r\n");
					input->comm->sendStatus("Initialized Default Waveform");
					input->comm->sendFatalError("NIAWG ERROR : " + excep.whatStr() + "\r\n");
					delete input;
					return -1;
				}
			}
			
			///
			if (systemAbortCheck(input->comm))
			{
				for (int deleteInc = 0; deleteInc < horizontalVariedWaveforms.size(); deleteInc++)
				{
					delete[] horizontalVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < horizontalVariedWaveformReads.size(); deleteInc++)
				{
					delete[] horizontalVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < verticalVariedWaveforms.size(); deleteInc++)
				{
					delete[] verticalVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < verticalVariedWaveformReads.size(); deleteInc++)
				{
					delete[] verticalVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
				{
					delete[] mixedWaveforms[deleteInc];
				}
				delete input;
				return 0;
			}
			// set the waiting event.
			SetEvent(eWaitingForNIAWGEvent);
			// wait untill the waiting thread completes.
			int result = WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);
			if (systemAbortCheck(input->comm))
			{
				for (int deleteInc = 0; deleteInc < horizontalVariedWaveforms.size(); deleteInc++)
				{
					delete[] horizontalVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < horizontalVariedWaveformReads.size(); deleteInc++)
				{
					delete[] horizontalVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < verticalVariedWaveforms.size(); deleteInc++)
				{
					delete[] verticalVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < verticalVariedWaveformReads.size(); deleteInc++)
				{
					delete[] verticalVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
				{
					delete[] mixedWaveforms[deleteInc];
				}
				delete input;
				return 0;
			}
			// check this flag set by the wait thread.
			if (eWaitError == true)
			{
				eWaitError = false;
				input->comm->sendFatalError("ERROR: Error in the wait function!\r\n");
				delete input;
				return -1;
			}

			if (input->settings.programIntensity == true && varValueLengthInc != 0)
			{
				colorBoxes<char> colors = { /*niawg*/'-', /*camera*/'-', /*intensity*/'Y' };
				input->comm->sendColorBox( colors );
				if (myErrorHandler(myAgilent::selectIntensityProfile(boost::numeric_cast<int>(varValueLengthInc), 
																	 intIsVaried, intensitySequenceMinAndMaxVector),
					"ERROR: intensity profile selection failed!\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, 
					userScriptIsWritten, userScriptName, true, false, true, input->comm))
				{
					input->comm->sendFatalError("ERROR: intensity profile selection failed!\r\n");
					delete input;
					return -1;
				}
				if (input->settings.programIntensity == true && intIsVaried == true)
				{
					input->comm->sendStatus("Intensity Profile Selected.\r\n");
				}
				colors = { /*niawg*/'-', /*camera*/'-', /*intensity*/'G' };
				input->comm->sendColorBox( colors );
			}
			/// Reinitialize Waveform Generation
			// Tell the master to begin the experiment
			isDoneTest = VI_FALSE;

			std::string varBaseString = "Variable loop #" + std::to_string(varValueLengthInc + 1) + "/" + std::to_string(variableValuesLengths[0]) + ". Variable values are:\r\n";
			for (int varNumInc = 0; varNumInc < varyingParameters.size(); varNumInc++)
			{
				varBaseString += "\t" + varyingParameters[varNumInc].name + " = " + std::to_string(variableValues[varNumInc][varValueLengthInc]) + "\r\n";
			}
			input->comm->sendStatus(varBaseString);
			// Restart Waveform
			if ((*input).dontActuallyGenerate == false)
			{
				try
				{
					input->niawg->configureOutputEnabled(VI_FALSE);
					input->niawg->abortGeneration();
					// skip defaults so start at 2.
					for (int waveformInc = 2; waveformInc < output.waveCount; waveformInc++)
					{
						// delete old waveforms
						ViChar variedWaveformName[11];
						sprintf_s(variedWaveformName, 11, "Waveform%i", waveformInc);
						if (output.vert.waveData[waveformInc].isVaried == true 
							 || output.hor.waveData[waveformInc].isVaried == true)
						{

							if (varValueLengthInc != 0)
							{
								input->niawg->deleteWaveform(variedWaveformName);
							}
							// And write the new one.
							input->niawg->allocateWaveform(variedWaveformName, variedMixedSize[mixedWriteCount] / 2);
							input->niawg->writeWaveform(variedWaveformName, variedMixedSize[mixedWriteCount], mixedWaveforms[mixedWriteCount]);
							mixedWriteCount++;
						}
					}

					ViBoolean individualAccumulationIsDone = false;
					input->niawg->writeScript(userScriptSubmit);
					userScriptIsWritten = true;

					input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");

					(*input).currentScript = "UserScript";

					input->niawg->configureOutputEnabled(VI_TRUE);
					input->niawg->initiateGeneration();

					if (input->settings.connectToMaster == true)
					{
						// Send returns -1 if failed, 0 otherwise.
						iResult = send(ConnectSocket, "go", 2, 0);
						myErrorHandler(iResult == -1, "ERROR: send failed!\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles,
							false,   userScriptIsWritten, userScriptName, true, false, true, input->comm);
						iResult = 0;
					}
				}
				catch (myException& excep)
				{
					input->comm->sendStatus("EXITED WITH ERROR!\r\n");
					input->comm->sendError("EXITED WITH ERROR!\r\n");
					input->comm->sendStatus("Initialized Default Waveform");
					input->comm->sendFatalError("NIAWG ERROR : " + excep.whatStr() + "\r\n");
					delete input;
					return -1;
				}
				eWaitError = false;
				waitThreadInput waitInput;
				waitInput.niawg = input->niawg;
				waitInput.profile = input->profile;
				// create the waiting thread.
				unsigned int NIAWGThreadID;
				eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex(0, 0, niawgWaitThread, &waitInput, 0, &NIAWGThreadID);
				// clear some memory.
				for (int cl = 0; cl < verticalVariedWaveforms.size(); cl++)
				{
					delete[] verticalVariedWaveforms[cl];
				}
				verticalVariedWaveforms.clear();
				for (int cl = 0; cl < horizontalVariedWaveforms.size(); cl++)
				{
					delete[] horizontalVariedWaveforms[cl];
				}
				horizontalVariedWaveforms.clear();
				for (int cl = 0; cl < verticalVariedWaveformReads.size(); cl++)
				{
					delete[] verticalVariedWaveformReads[cl];
				}
				verticalVariedWaveformReads.clear();
				for (int cl = 0; cl < horizontalVariedWaveformReads.size(); cl++)
				{
					delete[] horizontalVariedWaveformReads[cl];
				}
				horizontalVariedWaveformReads.clear();
				for (int cl = 0; cl < mixedWaveforms.size(); cl++)
				{
					delete[] mixedWaveforms[cl];
				}
				mixedWaveforms.clear();
				variedMixedSize.clear();
				
			}
		}
		// close things
		for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
		{
			if (verticalScriptFiles[sequenceInc].is_open())
			{
				verticalScriptFiles[sequenceInc].close();
			}
			if (horizontalScriptFiles[sequenceInc].is_open())
			{
				horizontalScriptFiles[sequenceInc].close();
			}
			if (intensityScriptFiles[sequenceInc].is_open())
			{
				intensityScriptFiles[sequenceInc].close();
			}
		}
	}
	else
	{
		/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///
		///					If no Variables
		///

		input->comm->sendStatus("NO Variable looping this run.\r\n");
		if ((*input).dontActuallyGenerate == false)
		{
			try
			{
				ViBoolean individualAccumulationIsDone = false;
				input->niawg->configureOutputEnabled(VI_FALSE);
				input->niawg->abortGeneration();
				// Should be just ready to go
				input->niawg->writeScript(userScriptSubmit);
				userScriptIsWritten = true;
				input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
				(*input).currentScript = "UserScript";
				input->niawg->configureOutputEnabled(VI_TRUE);
				input->niawg->initiateGeneration();
				if (input->settings.connectToMaster == true)
				{
					iResult = send(ConnectSocket, "go", 2, 0);
					if (myErrorHandler(iResult == -1, "ERROR: intensity profile selection failed!\r\n",&ConnectSocket, verticalScriptFiles, horizontalScriptFiles,
						false,   userScriptIsWritten, userScriptName, true, true, true, input->comm))
					{
						input->comm->sendFatalError("ERROR: intensity profile selection failed!\r\n");
						delete input;
						return -1;
					}
				}
			}
			catch (myException& excep)
			{
				input->comm->sendStatus("EXITED WITH ERROR!\r\n");
				input->comm->sendError("EXITED WITH ERROR!\r\n");
				input->comm->sendStatus("Initialized Default Waveform");
				input->comm->sendFatalError("NIAWG ERROR : " + excep.whatStr() + "\r\n");
				delete input;
				return -1;
			}
			waitThreadInput waitInput;
			waitInput.niawg = input->niawg;
			waitInput.profile = input->profile;
			eWaitError = false;
			unsigned int NIAWGThreadID;
			eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex(0, 0, niawgWaitThread, &waitInput, 0, &NIAWGThreadID);
		}
		// close some things
		for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
		{
			if (verticalScriptFiles[sequenceInc].is_open())
			{
				verticalScriptFiles[sequenceInc].close();
			}
			if (horizontalScriptFiles[sequenceInc].is_open())
			{
				horizontalScriptFiles[sequenceInc].close();
			}
			if (intensityScriptFiles[sequenceInc].is_open())
			{
				intensityScriptFiles[sequenceInc].close();
			}
		}
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Cleanup
	///
	if ((*input).repetitions == 0 || (input->variableNumber == 0 && TWEEZER_COMPUTER_SAFEMODE))
	{
		colorBoxes<char> colors = { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' };
		input->comm->sendColorBox( colors );
		input->comm->sendStatus("Scripts Loaded into NIAWG. This waveform sequence will run until aborted by the user.\r\n");
	}
	else 
	{
		std::string message;
		if (variableValues.size() > 0)
		{
			message = "Outputting LAST Series #" + std::to_string(variableValues[0].size()) + ".\r\n";
		}
		else
		{
			message = "Outputting ONLY Series.\r\n";
		}
		for (int varInc = 0; varInc < varyingParameters.size(); varInc++)
		{
			message += varyingParameters[varInc].name + " = " + std::to_string(variableValues[varInc].back()) + "; ";
		}
		colorBoxes<char> colors = { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' };
		input->comm->sendColorBox( colors );
		input->comm->sendStatus("Completed Sending Scripts. Waiting for last script to finish.\r\nmessage\r\n");
	}
	
	if (systemAbortCheck(input->comm))
	{
		for (int deleteInc = 0; deleteInc < horizontalVariedWaveforms.size(); deleteInc++)
		{
			delete[] horizontalVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < horizontalVariedWaveformReads.size(); deleteInc++)
		{
			delete[] horizontalVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < verticalVariedWaveforms.size(); deleteInc++)
		{
			delete[] verticalVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < verticalVariedWaveformReads.size(); deleteInc++)
		{
			delete[] verticalVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
		{
			delete[] mixedWaveforms[deleteInc];
		}
		delete input;
		return 0;
	}
	// wait...
	int result2 = WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);
	if (systemAbortCheck(input->comm))
	{
		for (int deleteInc = 0; deleteInc < horizontalVariedWaveforms.size(); deleteInc++)
		{
			delete[] horizontalVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < horizontalVariedWaveformReads.size(); deleteInc++)
		{
			delete[] horizontalVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < verticalVariedWaveforms.size(); deleteInc++)
		{
			delete[] verticalVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < verticalVariedWaveformReads.size(); deleteInc++)
		{
			delete[] verticalVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
		{
			delete[] mixedWaveforms[deleteInc];
		}
		delete input;
		return 0;
	}
	// check this flag set by the wait thread.
	if (eWaitError)
	{
		eWaitError = false;
		// does not throw.
		std::string msg = input->niawg->getErrorMsg();
		input->comm->sendFatalError("ERROR: Wait thread error: " + msg);
		delete input;
		return -1;
	}
	// Clear waveforms off of NIAWG (not working??? memory appears to still run out...)
	ViChar waveformDeleteName[11];
	try
	{
		for (int waveformInc = 2; waveformInc < output.waveCount; waveformInc++)
		{
			sprintf_s(waveformDeleteName, 11, "Waveform%i", waveformInc);
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				input->niawg->deleteWaveform(waveformDeleteName);
			}
		}
		if ((*input).dontActuallyGenerate == false)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// Delete relevant onboard memory.
				input->niawg->deleteScript("experimentScript");
			}
			userScriptIsWritten = false;
		}
	}
	catch (myException& excep)
	{
		input->comm->sendStatus("EXITED WITH ERROR!\r\n");
		input->comm->sendError("EXITED WITH ERROR!\r\n");
		input->comm->sendStatus("Initialized Default Waveform");
		input->comm->sendFatalError("NIAWG ERROR : " + excep.whatStr() + "\r\n");
		delete input;
		return -1;
	}
	// Delete the user script
	delete[] userScriptSubmit;
	// Delete waveforms in memory
	for (auto& waveform : verticalVariedWaveforms)
	{
		delete[] waveform;
	}
	verticalVariedWaveforms.clear();
	for (auto& waveform : horizontalVariedWaveforms)
	{
		delete[] waveform;
	}
	horizontalVariedWaveforms.clear();
	for (auto& read : verticalVariedWaveformReads)
	{
		delete[] read;
	}
	verticalVariedWaveformReads.clear();
	for (auto& read : horizontalVariedWaveformReads)
	{
		delete[] read;
	}
	horizontalVariedWaveformReads.clear();
	for (auto& waveform : mixedWaveforms)
	{
		delete[] waveform;
	}
	mixedWaveforms.clear();

	PostMessage(eMainWindowHandle, eNormalFinishMessageID, 0, 0);
	delete input;
	return 0;
};









