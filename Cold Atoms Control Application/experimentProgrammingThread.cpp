#include "stdafx.h"
#include "experimentProgrammingThread.h"
#include "externals.h"
#include "constants.h"
// Some headers used for communication protocols.
#include <winsock2.h>
#include <ws2tcpip.h>
#include "myErrorHandler.h"
#include "myAgilent.h"
#include "myNIAWG.h"
#include "experimentThreadInputStructure.h"
#include <sstream>
#include "NIAWGWaitThread.h"
#include "boost/cast.hpp"
#include "systemAbortCheck.h"
#include "postMyString.h"

/*
 * This runs the experiment. It calls createXYScript and then procedurally goes through all variable values. It also communicates with the other computer
 * throughout the process.
 * inputParam is the list of all of the relevant parameters to be used during this run of the experiment.
 */
unsigned __stdcall experimentProgrammingThread(LPVOID inputParam)
{
	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Initializing and Checking Options
	///

	experimentThreadInputStructure* inputStruct = (experimentThreadInputStructure*)inputParam;
	ViBoolean isDoneTest = VI_FALSE;
	char userScriptName[FILENAME_MAX];
	SOCKET ConnectSocket = INVALID_SOCKET;
	bool userScriptIsWritten = false;
	// initialize to 2 because of default waveforms...
	int waveformCount = 2;
	std::vector<std::string> libWaveformArray[NUMBER_OF_LIBRARY_FILES];
	bool fileOpenedStatus[NUMBER_OF_LIBRARY_FILES] = { 0 };
	std::vector<std::size_t> variableValuesLengths;
	int xVarWriteCount, yVarWriteCount, mixedCount, mixedWriteCount;
	std::vector<std::fstream> verticalScriptFiles, horizontalScriptFiles, intensityScriptFiles;
	// reset some variables which might have odd values if this is reached from a break from within the variable waveform calculations.
	bool correctionFlag = false;
	bool repeatFlag = false;
	bool rewriteFlag = false;
	ViChar* userScriptSubmit;
	int predWaveformCount = 0;
	bool intIsVaried;
	// clear old vectors...
	std::vector<long int> variedMixedSize;
	std::vector<waveData> allXWaveformParameters, allYWaveformParameters;
	allXWaveformParameters.resize(2);
	allYWaveformParameters.resize(2);
	std::vector<bool> xWaveformIsVaried, yWaveformIsVaried, intensityIsVaried;
	xWaveformIsVaried.resize(2);
	yWaveformIsVaried.resize(2);
	intensityIsVaried.resize(2);
	std::vector<myMath::minMaxDoublet> intensitySequenceMinAndMaxVector;
	
	std::string	xWaveformNumString, yWaveformNumString;
	std::string inputType, xVarType, yVarType, finalUserScriptString;
	std::vector<std::string> workingUserScriptString;
	std::vector<ViReal64 *> xVariedWaveformPairs, yVariedWaveformPairs, xVariedWaveforms, yVariedWaveforms, xVariedWaveformReads, yVariedWaveformReads,
							mixedWaveforms;
	std::vector<ViReal64 *> currentWaveformPointers;
	// Create arrays for storing these names.
	std::vector<std::string> xPredWaveformNames;
	std::vector<std::string> yPredWaveformNames;
	std::string userScriptNameString;
	// This array stores the waveform #s of the predefined waveforms.
	std::vector<int> predWaveLocs;

	// initialize some socket stuff
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	xWaveformIsVaried.resize(2);
	yWaveformIsVaried.resize(2);
	/// Initialization
	// Communication object used to open up the windows socket applications (WSA) DLL. 
	WSADATA wsaData;
	// object that contains error information.
	int iResult;
	// the socket object used to connect to the other computer. Starts invalid because it isn't active yet.
	std::vector<std::vector<double> > variableValues;
	// Initialize Winsock
	// I have no idea what this MAKEWORD function is doing.
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// check for errors initializing winsock
	if (iResult != 0) 
	{
		std::string errMsg = "WSAStartup failed: " + std::to_string(iResult) + "\r\n";
		MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
		delete inputStruct;
		return 1;
	}
	// register messages for main window.
	eVariableStatusMessageID = RegisterWindowMessage("ID_THREAD_VARIABLE_STATUS");
	eGreenMessageID = RegisterWindowMessage("ID_THREAD_GUI_GREEN");
	eStatusTextMessageID = RegisterWindowMessage("ID_THREAD_STATUS_MESSAGE");
	eErrorTextMessageID = RegisterWindowMessage("ID_THREAD_ERROR_MESSAGE");
	eFatalErrorMessageID = RegisterWindowMessage("ID_THREAD_FATAL_ERROR_MESSAGE");
	eNormalFinishMessageID = RegisterWindowMessage("ID_THREAD_NORMAL_FINISH_MESSAGE");
	eColoredEditMessageID = RegisterWindowMessage("ID_VARIABLE_VALES_MESSAGE");
	
	// initialize the script string. The script needs a script name at the top.
	userScriptNameString = "experimentScript";
	workingUserScriptString.resize(eSequenceFileNames.size());
	for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
	{
		workingUserScriptString[sequenceInc] = "";
	}

	// Open files
	verticalScriptFiles.resize(inputStruct->threadSequenceFileNames.size());
	horizontalScriptFiles.resize(inputStruct->threadSequenceFileNames.size());
	intensityScriptFiles.resize(inputStruct->threadSequenceFileNames.size());
	(*inputStruct).threadVariableNames.resize(0);
	for (int sequenceInc = 0; sequenceInc < inputStruct->threadSequenceFileNames.size(); sequenceInc++)
	{
		// open configuration file
		std::fstream configFile(inputStruct->currentFolderLocation + "\\" + inputStruct->threadSequenceFileNames[sequenceInc]);
		std::string verticalScriptAddress, horizontalScriptAddress, intensityScriptAddress;
		// in order...
		
		/// load vertical file
		getline(configFile, verticalScriptAddress); 
		verticalScriptFiles[sequenceInc].open(verticalScriptAddress);
		if (!verticalScriptFiles[sequenceInc].is_open())
		{
			// send error
			if (myErrorHandler(1, "ERROR: Failed to open vertical script file named: " + verticalScriptAddress + " found in configuration: " 
								  + inputStruct->threadSequenceFileNames[sequenceInc] + "\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false,
				eError, eSessionHandle, userScriptIsWritten, userScriptName, false, false, true) == true)
			{
				postMyString(eErrorTextMessageID, "ERROR: Failed to open vertical script file named: " + verticalScriptAddress + " found in configuration: "
					+ inputStruct->threadSequenceFileNames[sequenceInc] + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
		}
		/// load horizontal file
		getline(configFile, horizontalScriptAddress);
		horizontalScriptFiles[sequenceInc].open(horizontalScriptAddress);
		if (!horizontalScriptFiles[sequenceInc].is_open())
		{
			// send error
			if (myErrorHandler(1, "ERROR: Failed to open horizontal script file named: " + horizontalScriptAddress + " found in configuration: "
				+ inputStruct->threadSequenceFileNames[sequenceInc] + "\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false,
				eError, eSessionHandle, userScriptIsWritten, userScriptName, false, false, true) == true)
			{
				postMyString(eErrorTextMessageID, "ERROR: Failed to open horizontal script file named: " + horizontalScriptAddress + " found in configuration: "
					+ inputStruct->threadSequenceFileNames[sequenceInc] + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
		}
		/// load intensity file
		getline(configFile, intensityScriptAddress);
		intensityScriptFiles[sequenceInc].open(intensityScriptAddress);
		if (!intensityScriptFiles[sequenceInc].is_open())
		{
			// send error
			if (myErrorHandler(1, "ERROR: Failed to open intensity script file named: " + intensityScriptAddress + " found in configuration: "
				+ inputStruct->threadSequenceFileNames[sequenceInc] + "\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false,
				eError, eSessionHandle, userScriptIsWritten, userScriptName, false, false, true) == true)
			{
				postMyString(eErrorTextMessageID, "ERROR: Failed to open intensity script file named: " + intensityScriptAddress + " found in configuration: "
					+ inputStruct->threadSequenceFileNames[sequenceInc] + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
		}
		// load variables
		int varNum;
		configFile >> varNum;
		for (int varInc = 0; varInc < varNum; varInc++)
		{
			// get next variable
			std::string tempVar;
			configFile >> tempVar;
			bool alreadyExists = false;
			for (int varInc2 = 0; varInc2 < (*inputStruct).threadVariableNames.size(); varInc2++)
			{
				if (tempVar == (*inputStruct).threadVariableNames[varInc2])
				{
					alreadyExists = true;
				}
			}
			if (!alreadyExists)
			{
				(*inputStruct).threadVariableNames.push_back(tempVar);
			}
		}
	}
	
	// initialize some socket stuffs
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (!SAFEMODE)
	{
		// Resolve the server address and port
		iResult = getaddrinfo(SERVER_ADDRESS, DEFAULT_PORT, &hints, &result);
		if (myErrorHandler(iResult, "ERROR: getaddrinfo failed: " + std::to_string(iResult) + "\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false,
			eError, eSessionHandle, userScriptIsWritten, userScriptName, false, false, true) == true)
		{
			postMyString(eErrorTextMessageID, "ERROR: getaddrinfo failed: " + std::to_string(iResult) + "\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			delete inputStruct;

			return -1;
		}
	}
	ptr = result;

	// check acuumulations. if 0, this is code for setting the code to just repeat forever, 
	if ((*inputStruct).threadConnectToMaster == false)
	{
		(*inputStruct).threadAccumulations = 0;
	}

	if ((*inputStruct).threadConnectToMaster == true)
	{
		if (!SAFEMODE)
		{
			// make socket object
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			// Handle Errors
			if (myErrorHandler((ConnectSocket == -1), "ERROR: at socket() function: " + std::to_string(WSAGetLastError()) + "\r\n", ConnectSocket, verticalScriptFiles,
				horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, false, false, true))
			{
				postMyString(eErrorTextMessageID, "ERROR: at socket() function: " + std::to_string(WSAGetLastError()) + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;

				return -1;
			}
		}
		postMyString(eStatusTextMessageID, "Attempting to connect......");
		if (!SAFEMODE)
		{
			// Handle Errors
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (myErrorHandler(iResult, "Unable to connect to server!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles,	/*aborting = */false,
				eError, eSessionHandle, userScriptIsWritten, userScriptName, /*Socket Active = */false, false, true))
			{
				postMyString(eErrorTextMessageID, "Unable to connect to server!\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
			freeaddrinfo(result);
		}
		postMyString(eStatusTextMessageID, "Established Connection!\r\n");
	}

	/// Get accumulations from master. Procedure is
	// this computer >> Other Computer ("Acumulations?")
	// Other Computer >> this computer ("Accumulations: (#)")
	// this computer >> other computer ("Received Accumulations.")
	if ((*inputStruct).threadConnectToMaster == true)
	{
		char recvbuf[256];
		int recvbufn = 256;
		postMyString(eStatusTextMessageID, "Waiting for Accumulations # from master...");
		if (!SAFEMODE)
		{

			iResult = send(ConnectSocket, "Accumulations?", 14, 0);
			if (myErrorHandler(iResult == -1, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n", ConnectSocket,
				verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
			{
				postMyString(eErrorTextMessageID, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
			iResult = recv(ConnectSocket, recvbuf, recvbufn, 0);
		}
		else
		{
			iResult = 1;
		}
		postMyString(eStatusTextMessageID, "Received!\r\n");
		if (!SAFEMODE)
		{
			std::string tempAccumulations;
			std::stringstream accumulationsStream;
			accumulationsStream << recvbuf;
			accumulationsStream >> tempAccumulations;
			if (myErrorHandler(tempAccumulations != "Accumulations:", "ERROR: master's message did not start with \"Accumulations:\". It started with " + tempAccumulations + " . Assuming fatal error.", ConnectSocket,
				verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
			{
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
			accumulationsStream >> tempAccumulations;
			try
			{
				(*inputStruct).threadAccumulations = std::stoi(tempAccumulations);
			}
			catch (std::invalid_argument&)
			{
				if (myErrorHandler(true, "ERROR: master's message's number did not convert correctly to an integer. String trying to convert is "
					+ tempAccumulations + ". Assuming fatal error.", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten,
					userScriptName, true, false, true))
				{
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					delete inputStruct;
					return -1;
				}
			}
			if (myErrorHandler((*inputStruct).threadAccumulations < 0, "ERROR: master's message's number was negative! String trying to convert is " + tempAccumulations
				+ ". Assuming fatal error.", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName,
				true, false, true))
			{
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
			iResult = send(ConnectSocket, "Received Accumulations.", 23, 0);
			if (myErrorHandler(iResult == -1, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n", ConnectSocket,
				verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
			{
				postMyString(eErrorTextMessageID, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
			if (myErrorHandler((*inputStruct).threadAccumulations % ((*inputStruct).threadSequenceFileNames.size() + 1) != 0, 
							   "ERROR: Number of accumulations received from master: " + std::to_string((*inputStruct).threadAccumulations) 
							   + ", is not an integer multiple of the number of configurations in the sequence: " 
							   + std::to_string((*inputStruct).threadSequenceFileNames.size()) + ". It must be.\r\n", ConnectSocket, verticalScriptFiles, 
							   horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
			{
				postMyString(eErrorTextMessageID, "ERROR: Number of accumulations received from master: " + std::to_string((*inputStruct).threadAccumulations)
													+ ", is not an integer multiple of the number of configurations in the sequence: "
													+ std::to_string((*inputStruct).threadSequenceFileNames.size()) + ". It must be.\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
		}
		else
		{
			(*inputStruct).threadAccumulations = 100;
		}
	}
	(*inputStruct).threadAccumulations /= (*inputStruct).threadSequenceFileNames.size();

	std::string message = "Accumulations # after sequence normalization: " + std::to_string((*inputStruct).threadAccumulations) + "\r\n";
	postMyString(eStatusTextMessageID, message);

	// analyze the input files and create the xy-script. Originally, I thought I'd write the script in two parts, the x and y parts, but it turns out 
	// not to work like I thought it did. If I'd known this from the start, I probably wouldn't have created this subroutine, except perhaps for the 
	// fact that it get called recursively by predefined scripts in the instructions file.
	postMyString(eStatusTextMessageID, "Beginning Waveform Read, Calculate, and Write\r\nProcedure...\r\n");
	if (systemAbortCheck())
	{
		for (int deleteInc = 0; deleteInc < yVariedWaveforms.size(); deleteInc++)
		{
			delete[] yVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < yVariedWaveformReads.size(); deleteInc++)
		{
			delete[] yVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < xVariedWaveforms.size(); deleteInc++)
		{
			delete[] xVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < xVariedWaveformReads.size(); deleteInc++)
		{
			delete[] xVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
		{
			delete[] mixedWaveforms[deleteInc];
		}
		delete inputStruct;
		return 0;
	}
	for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
	{
		postMyString(eStatusTextMessageID, "Working with configuraiton # " + std::to_string(sequenceInc) + " in Sequence...\r\n");
		/// Create Script and Write Waveforms //////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (myErrorHandler(myNIAWG::createXYScript(verticalScriptFiles[sequenceInc], horizontalScriptFiles[sequenceInc], workingUserScriptString[sequenceInc], TRIGGER_NAME, waveformCount, eSessionHandle,
			SESSION_CHANNELS, eError, xPredWaveformNames, yPredWaveformNames, predWaveformCount, predWaveLocs, libWaveformArray, fileOpenedStatus,
			allXWaveformParameters, xWaveformIsVaried, allYWaveformParameters, yWaveformIsVaried, false, true, (*inputStruct).currentFolderLocation), "createXYScript() threw an error!\r\n",
			ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
		{
			postMyString(eErrorTextMessageID, "createXYScript() threw an error!\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			delete inputStruct;
			return -1;
		}
		if (systemAbortCheck())
		{
			for (int deleteInc = 0; deleteInc < yVariedWaveforms.size(); deleteInc++)
			{
				delete[] yVariedWaveforms[deleteInc];
			}
			for (int deleteInc = 0; deleteInc < yVariedWaveformReads.size(); deleteInc++)
			{
				delete[] yVariedWaveformReads[deleteInc];
			}
			for (int deleteInc = 0; deleteInc < xVariedWaveforms.size(); deleteInc++)
			{
				delete[] xVariedWaveforms[deleteInc];
			}
			for (int deleteInc = 0; deleteInc < xVariedWaveformReads.size(); deleteInc++)
			{
				delete[] xVariedWaveformReads[deleteInc];
			}
			for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
			{
				delete[] mixedWaveforms[deleteInc];
			}
			delete inputStruct;
			return 0;
		}
	}
	/// Final Cleanup before waveform initiation
	postMyString(eStatusTextMessageID, "Constant Waveform Preparation Completed...\r\n");

	// format the script to send to the 5451 according to the accumulation number and based on the number of sequences.
	finalUserScriptString = "script " + userScriptNameString + "\n";
	if ((*inputStruct).threadAccumulations == 0)
	{
		finalUserScriptString += "repeat forever\n";
		for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
		{
			finalUserScriptString += workingUserScriptString[sequenceInc];
		}
		finalUserScriptString += "end repeat\n";
	}
	else
	{
		// repeat the script once for every accumulation.
		for (unsigned int accumCount = 0; accumCount < (*inputStruct).threadAccumulations; accumCount++)
		{
			for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
			{
				finalUserScriptString += workingUserScriptString[sequenceInc];
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
	if ((*inputStruct).threadGetVarFilesFromMaster == true)
	{
		variableValues.resize((*inputStruct).threadVariableNames.size(), std::vector<double>(0));
		variableValuesLengths.resize((*inputStruct).threadVariableNames.size());
		char recvbuf[1000];
		int recvbufn = 1000;
		for (int variableNameInc = 0; variableNameInc < (*inputStruct).threadVariableNames.size(); variableNameInc++)
		{
			std::string message = "Waiting for Variable Set #" + std::to_string(variableNameInc) + "... ";
			postMyString(eStatusTextMessageID, message);
			if (!SAFEMODE)
			{
				iResult = send(ConnectSocket, "next variable", 13, 0);
				if (myErrorHandler(iResult == -1, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n", ConnectSocket,
					verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
				{
					postMyString(eErrorTextMessageID, "ERROR: Socket send failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					delete inputStruct;
					return -1;
				}
			}
			if (!SAFEMODE)
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
				postMyString(eStatusTextMessageID, "Received!\r\n");
				int varNameCursor = -1;
				std::string tempVarName;
				std::stringstream variableStream;
				variableStream << recvbuf;
				variableStream >> tempVarName;
				for (int q1 = 0; q1 < (*inputStruct).threadVariableNames.size(); q1++)
				{
					if (tempVarName == (*inputStruct).threadVariableNames[q1])
					{
						varNameCursor = q1;
						break;
					}
				}
				if (!SAFEMODE)
				{
					if (myErrorHandler(varNameCursor == -1, "ERROR: The variable name sent by the master computer doesn't match any current variables!\r\n",
						ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false,
						true))
					{
						postMyString(eErrorTextMessageID, "ERROR: The variable name sent by the master computer doesn't match any current variables!\r\n");
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
						delete inputStruct;
						return -1;
					}
				}
				// WRITE VAR VALUES
				int j = 0;
				std::string tempString;
				double tempDouble;
				if (!SAFEMODE)
				{
					while (true)
					{
						variableStream >> tempString;
						if (tempString == "done!")
						{
							break;
						}
						tempDouble = stod(tempString);
						variableValues[varNameCursor].resize(j + 1);
						variableValues[varNameCursor][j] = tempDouble;
						j++;
					}
					variableValuesLengths[varNameCursor] = j;
				}
				else if (SAFEMODE)
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
				if (myErrorHandler(-1, "ERROR: The connection with the master computer was closed!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles,
					false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
				{
					postMyString(eErrorTextMessageID, "ERROR: The connection with the master computer was closed!\r\n");
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					delete inputStruct;
					return -1;
				}
			}
			else
			{
				if (myErrorHandler(-1, "ERROR: Socket Recieve failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n", ConnectSocket,
					verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
				{
					postMyString(eErrorTextMessageID, "ERROR: Socket Recieve failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					delete inputStruct;
					return -1;
				}
				postMyString(eErrorTextMessageID, "ERROR: Socket Recieve failed with error code: " + std::to_string(WSAGetLastError()) + "\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
		}
	}
	/// Create Dummy Variable List if Necessary
	else if ((*inputStruct).threadUseDummyVariables == true)
	{
		// There shouldn't be any variables loaded. Check this.
		if (myErrorHandler(boost::numeric_cast<int>((*inputStruct).threadVariableNames.size()), "ERROR: There are variables loaded when a dummy variable is being used.\r\n", ConnectSocket,
			verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
		{
			postMyString(eErrorTextMessageID, "ERROR: There are variables loaded when a dummy variable is being used.\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			delete inputStruct;
			return -1;
		}
		variableValues.resize(1);
		variableValues[0].resize((*inputStruct).threadDummyNum);
		variableValuesLengths.resize(1);
		variableValuesLengths[0] = (*inputStruct).threadDummyNum;
		// Reserved Character!
		(*inputStruct).threadVariableNames.push_back("%");
		// Load 0s in. Could load anything in, these values don't get used because the user can't enter a % as a variable.
		for (int dummyInc = 0; dummyInc < (*inputStruct).threadDummyNum; dummyInc++)
		{
			variableValues[0][dummyInc] = 0;
		}
	}
	/// Read Variables from files (Deprecated)
	else if ((*inputStruct).threadVariableNames.size() != 0)
	{
		// ERROR
		if (myErrorHandler(-1, "ERROR: Code entered variable file reading section. This shouldn't happen. Check for Logic Errors in the code.\r\n", ConnectSocket,
						   verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
		{
			postMyString(eErrorTextMessageID, "ERROR: Code entered variable file reading section. This shouldn't happen. Check for Logic Errors in the code.\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			delete inputStruct;
			return -1;
		}
	}
	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Logging
	///
	if (!SAFEMODE)
	{
		// This report goes to a folder I create on the Andor.
		if ((*inputStruct).threadLogScriptAndParams == true)
		{
			postMyString(eStatusTextMessageID, "Logging Script and Experiment Parameters...\r\n");

			std::string xScriptLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Vertical Script.txt";
			std::string yScriptLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Horizontal Script.txt";
			std::string intensityLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Intensity Script.txt";
			std::string parametersFileLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Parameters.txt";

			bool andorConnected = false;
			do {
				std::ofstream verticalScriptLog(xScriptLogPath);
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
						if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n", ConnectSocket,
							verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
						{
							postMyString(eErrorTextMessageID, "ERROR: Andor Disconected. User Aborted.\r\n");
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							delete inputStruct;
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
					for (int sequenceInc = 0; sequenceInc < (*inputStruct).threadSequenceFileNames.size(); sequenceInc++)
					{
						verticalScriptLog << verticalScriptFiles[sequenceInc].rdbuf();
						verticalScriptFiles[sequenceInc].clear();
						verticalScriptFiles[sequenceInc].seekg(0, std::ios::beg);
					}
				}
			} while (andorConnected == false);

			andorConnected = false;
			do {
				std::ofstream horizontalScriptLog(yScriptLogPath);
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
						if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError,
							eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
						{
							postMyString(eErrorTextMessageID, "ERROR: Andor Disconected. User Aborted.\r\n");
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							delete inputStruct;
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
					
					for (int sequenceInc = 0; sequenceInc < (*inputStruct).threadSequenceFileNames.size(); sequenceInc++)
					{
						horizontalScriptLog << horizontalScriptFiles[sequenceInc].rdbuf();
						horizontalScriptFiles[sequenceInc].clear();
						horizontalScriptFiles[sequenceInc].seekg(0, std::ios::beg);
					}
				}
			} while (andorConnected == false);

			andorConnected = false;

			do
			{
				std::ofstream intensityScriptLog(intensityLogPath);
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
							if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError,
								eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
							{
								postMyString(eErrorTextMessageID, "ERROR: Andor Disconected. User Aborted.\r\n");
								PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
								delete inputStruct;
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
					for (int sequenceInc = 0; sequenceInc < (*inputStruct).threadSequenceFileNames.size(); sequenceInc++)
					{
						intensityScriptLog << intensityScriptFiles[sequenceInc].rdbuf();
						intensityScriptFiles[sequenceInc].clear();
						intensityScriptFiles[sequenceInc].seekg(0, std::ios::beg);
					}
				}
			} while (andorConnected == false);

			andorConnected = false;
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
						if (myErrorHandler(-1, "ERROR: Andor Disconected. User Aborted.\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError,
							eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
						{
							postMyString(eErrorTextMessageID, "ERROR: Andor Disconected. User Aborted.\r\n");
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							delete inputStruct;
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
					std::string message = "Script Saved: " + std::to_string((*inputStruct).threadXScriptSaved) + "\nDont Actually Generate: " + std::to_string((*inputStruct).threadDontActuallyGenerate)
						+ "\nConnect To Master: " + std::to_string((*inputStruct).threadConnectToMaster) + "\nGet Variable Files From Master: "
						+ std::to_string((*inputStruct).threadGetVarFilesFromMaster) + "\nAccumulations: " + std::to_string((*inputStruct).threadAccumulations);
					parametersFileLog << message;
				}
			} while (andorConnected == false);
		}
	}
	/// Variable Cleanup Before Execution
	// check that variable files are the same length.
	for (std::size_t varNameInc = 0; varNameInc + 1 < (*inputStruct).threadVariableNames.size(); varNameInc++)
	{
		if (variableValuesLengths[varNameInc] != variableValuesLengths[varNameInc + 1])
		{
			if (myErrorHandler(-1, "Error: lengths of variable values are not all the same! They must be the same\r\n", ConnectSocket, verticalScriptFiles,
				horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
			{
				postMyString(eErrorTextMessageID, "Error: lengths of variable values are not all the same! They must be the same\r\n");
				PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
				delete inputStruct;
				return -1;
			}
		}
	}

	std::vector<std::vector<POINT>> intensityPoints;
	if ((*inputStruct).threadProgramIntensityOption == true)
	{
		postMyString(eStatusTextMessageID, "Programing Intensity Profile(s)...");

		if (myErrorHandler(myAgilent::programIntensity(boost::numeric_cast<int>((*inputStruct).threadVariableNames.size()), (*inputStruct).threadVariableNames, variableValues, intIsVaried,
			intensitySequenceMinAndMaxVector, intensityPoints, intensityScriptFiles),
			"ERROR: Intensity Programming Failed!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle,
			userScriptIsWritten, userScriptName, /*Socket Active = */true, false, true))
		{
			postMyString(eErrorTextMessageID, "ERROR: Intensity Programming Failed!\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			delete inputStruct;
			return -1;
		}
		postMyString(eStatusTextMessageID, "Complete!\r\n");

		// select the first one.
		if (myErrorHandler(myAgilent::selectIntensityProfile(0, intIsVaried, intensitySequenceMinAndMaxVector),
			"ERROR: intensity profile selection failed!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError,
			eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
		{
			postMyString(eErrorTextMessageID, "ERROR: intensity profile selection failed!\r\n");
			PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
			delete inputStruct;
			return -1;
		}
		postMyString(eStatusTextMessageID, "Intensity Profile Selected.\r\n");
	}
	else
	{
		postMyString(eStatusTextMessageID, "Intensity Profile NOT being programed.\r\n");
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Big Variable Handling Loop
	///
	// If there are any varible files, enter big variable loop.
	if ((*inputStruct).threadVariableNames.size() > 0)
	{
		postMyString(eStatusTextMessageID, "Begin Variable & Execution Loop.\r\n");
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
				postMyString(eStatusTextMessageID, "Approximate Time To Complete: " + std::to_string(timeToCompleteM) + ":"
					+ std::to_string(timeToCompleteS) + " (min:sec)\r\n");
			}
			// Tell the main thread that another variable is being written.
			//PostMessage(eMainWindowHandle, eVariableStatusMessageID, 0, varNameInc);

			xVarWriteCount = 0;
			yVarWriteCount = 0;
			mixedCount = 0;
			mixedWriteCount = 0;
			// for every waveform...
			/// I think j = 0 is the default... should I be handling that at all??? shouldn't make a difference I don't think. ????????????????????????
			for (int j = 0; j < waveformCount; j++)
			{
			BeginningOfWaveformLoop:
				// change the waveform, if needed.
				/// if need to change both... /////////////////////////////////////////////////////////////////////////////////////////////////////////
				if (xWaveformIsVaried[j] == true && yWaveformIsVaried[j] == true)
				{
					// For all Variables...
					for (std::size_t k = 0; k < (*inputStruct).threadVariableNames.size() + 1; k++)
					{
						std::string currentVar;
						// Set variable name. Should be the same between variableNames and yVarNames.
						if (k < (*inputStruct).threadVariableNames.size())
						{
							currentVar = (*inputStruct).threadVariableNames[k];
						}
						else
						{
							// This is the reserved character for a variable time resulting from waveform correction.
							// This value is always set once to check for this because k loops to and including variableNames.size().
							currentVar = '\'';
						}
						/// Loop for jth Y Waveform
						for (int m = 0; m < allYWaveformParameters[j].varNum; m++)
						{
							// if parameter's variable matches current variable...
							if (allYWaveformParameters[j].varNames[m] == currentVar)
							{
								double variableValue;
								if (k < (*inputStruct).threadVariableNames.size())
								{
									variableValue = variableValues[k][varValueLengthInc];
								}
								else
								{
									// dummy value. Not used in this case.
									variableValue = 0;
								}
								/// For Correction Waveforms...
								// The vartypes value for a correction waveform is -3.
								// Structure is weird for waveforms correcting the next waveform.
								if (allYWaveformParameters[j].varTypes[m] == -3)
								{
									// set it to -4 so that it is correctly recognized the next time through.
									allYWaveformParameters[j].varTypes[m] = -4;
									// The corresponding x variable might be in a different location; I need to find that location
									for (int x_min_3_loc = 0; x_min_3_loc < allXWaveformParameters[j].varNum; x_min_3_loc++)
									{
										if (allXWaveformParameters[j].varTypes[x_min_3_loc] == -3)
										{
											allXWaveformParameters[j].varTypes[x_min_3_loc] = -4;
										}

									}
									/// Set all the parameters as if this had exited normally.
									// increment the waveform being corrected.
									j++;
									// increment the number of x, y, and mixed waveforms that have been "written"
									xVarWriteCount++;
									yVarWriteCount++;
									mixedCount++;
									// add extra spots for waveforms here.
									yVariedWaveforms.resize(yVariedWaveforms.size() + 1);
									yVariedWaveformReads.resize(yVariedWaveformReads.size() + 1);
									xVariedWaveforms.resize(xVariedWaveforms.size() + 1);
									xVariedWaveformReads.resize(xVariedWaveformReads.size() + 1);
									variedMixedSize.resize(variedMixedSize.size() + 1);
									mixedWaveforms.resize(mixedWaveforms.size() + 1);
									// this flag lets the code know that the next waveform is being corrected.
									correctionFlag = true;

									// Return to the beginning of the for loop without
									goto BeginningOfWaveformLoop;
								}
								/// Not correction waveform...
								// change parameters, depending on the case. The varTypes and variable Value sets were set previously.
								if (myErrorHandler(myNIAWG::varyParam(allYWaveformParameters, allXWaveformParameters, j, allYWaveformParameters[j].varTypes[m],
									variableValue), "ERROR: varyParam() returned an error.\r\n", ConnectSocket, verticalScriptFiles,
									horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
								{
									postMyString(eErrorTextMessageID, "ERROR: varyParam() returned an error.\r\n");
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
									delete inputStruct;
									return -1;
								}
							} // end "if variables match"

						} // end "parameters loop"
						/// Loop for jth X Waveform
						for (int m = 0; m < allXWaveformParameters[j].varNum; m++)
						{
							// if parameter's variable matches variable...
							if (allXWaveformParameters[j].varNames[m] == currentVar)
							{
								double variableValue;
								// change parameters, depending on the case. This was set during input reading.
								if (k < (*inputStruct).threadVariableNames.size())
								{
									variableValue = variableValues[k][varValueLengthInc];
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
								if (allXWaveformParameters[j].varTypes[m] == -3) {
									if (myErrorHandler(-1, "ERROR: The code has entered a part of the code which it should never enter, indicating a logic"
										"error somewhere. Search \"Error location #1\" in the code to find this location. The code will "
										"continue to run, but is likely about to crash.", ConnectSocket, verticalScriptFiles, horizontalScriptFiles,
										false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
									{
										postMyString(eErrorTextMessageID, "ERROR: The code has entered a part of "
											"the code which it should never enter, indicating a logic error somewhere. Search \"Error location #1\" in "
											"the code to find this location. The code will continue to run, but is likely about to crash.");
										PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
										delete inputStruct;
										return -1;
									}
									// set it to -4 so that it is correctly recognized the next time through.
									allXWaveformParameters[j].varTypes[m] = -4;
									allYWaveformParameters[j].varTypes[m] = -4;
									// increment the waveform being corrected.
									j++;
									// increment the number of x, y, and mixed waveforms that have been "written"
									xVarWriteCount++;
									yVarWriteCount++;
									mixedCount++;
									// add extra spots for waveforms here.
									yVariedWaveforms.resize(yVariedWaveforms.size() + 1);
									yVariedWaveformReads.resize(yVariedWaveformReads.size() + 1);
									xVariedWaveforms.resize(xVariedWaveforms.size() + 1);
									xVariedWaveformReads.resize(xVariedWaveformReads.size() + 1);
									variedMixedSize.resize(variedMixedSize.size() + 1);
									mixedWaveforms.resize(mixedWaveforms.size() + 1);
									// this flag lets the code know that the next waveform is being corrected.
									correctionFlag = true;
									// Return to the beginning of the for loop without
									goto BeginningOfWaveformLoop;
								}
								// change parameters, depending on the case. The varTypes and variable Value sets were set previously.
								if (myErrorHandler(myNIAWG::varyParam(allXWaveformParameters, allYWaveformParameters, j, allXWaveformParameters[j].varTypes[m],
									variableValue),
									"ERROR: varyParam() returned an error.\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError,
									eSessionHandle, userScriptIsWritten, userScriptName, /*Socket Active = */true, false, true))
								{
									postMyString(eErrorTextMessageID, "ERROR: varyParam() returned an error.\r\n");
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
									delete inputStruct;
									return -1;
								}
							} // end if variables match
						} // end parameters loop
					} // end variables loop

					// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform". You only get here if 
					// flagged for it during read.
					for (int n = 0; n < allXWaveformParameters[j].signalNum; n++)
					{
						if (allXWaveformParameters[j].signals[n].initPhase == -1)
						{
							allXWaveformParameters[j].signals[n].phaseOption = -1;
							if (n + 1 > allXWaveformParameters[j - 1].signalNum) {
								if (myErrorHandler(-1, "ERROR: You are trying to copy the phase of the " + std::to_string(n + 1) + " signal of the "
									"previous waveform, but the previous waveform only had "
									+ std::to_string(allXWaveformParameters[j - 1].signalNum) + " signals!\r\n", ConnectSocket,
									verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten, userScriptName,
									true, false))
								{
									postMyString(eErrorTextMessageID, "ERROR: You are trying to copy the phase of "
										"the " + std::to_string(n + 1) + " signal of the previous waveform, but the previous waveform only had "
										+ std::to_string(allXWaveformParameters[j - 1].signalNum) + " signals!\r\n");
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
									delete inputStruct;
									return -1;
								}
							}
							allXWaveformParameters[j].signals[n].initPhase = allXWaveformParameters[j - 1].signals[n].finPhase;
						}
					}

					// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform". You only get here if 
					// flagged for it during read.
					for (int n = 0; n < allYWaveformParameters[j].signalNum; n++)
					{
						if (allYWaveformParameters[j].signals[n].initPhase == -1)
						{
							allYWaveformParameters[j].signals[n].phaseOption = -1;
							if (n + 1 > allYWaveformParameters[j - 1].signalNum)
							{
								if (myErrorHandler(-1, "ERROR: You are trying to copy the phase of the " + std::to_string(n + 1) + " signal of the "
									"previous waveform, but the previous waveform only had " + std::to_string(allYWaveformParameters[j - 1].signalNum)
									+ " signals!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError, eSessionHandle, userScriptIsWritten,
									userScriptName, true, false, true))
								{
									postMyString(eErrorTextMessageID, "ERROR: You are trying to copy the phase of "
										"the " + std::to_string(n + 1) + " signal of the previous waveform, but the previous waveform only had "
										+ std::to_string(allYWaveformParameters[j - 1].signalNum) + " signals!\r\n");
									PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
									delete inputStruct;
									return -1;
								}
							}
							allYWaveformParameters[j].signals[n].initPhase = allYWaveformParameters[j - 1].signals[n].finPhase;
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
							delete[] yVariedWaveforms[yVarWriteCount];
							delete[] yVariedWaveformReads[yVarWriteCount];
							delete[] xVariedWaveforms[xVarWriteCount];
							delete[] xVariedWaveformReads[xVarWriteCount];
						}
						yVariedWaveforms[yVarWriteCount] = new ViReal64[allYWaveformParameters[j].sampleNum];
						yVariedWaveformReads[yVarWriteCount] = new ViReal64[allYWaveformParameters[j].sampleNum + allYWaveformParameters[j].signalNum];
						xVariedWaveforms[xVarWriteCount] = new ViReal64[allXWaveformParameters[j].sampleNum];
						xVariedWaveformReads[xVarWriteCount] = new ViReal64[allXWaveformParameters[j].sampleNum + allXWaveformParameters[j].signalNum];


					}
					else if (repeatFlag == false && rewriteFlag == false)
					{
						// this is equivalent to an else.
						yVariedWaveforms.push_back(new ViReal64[allYWaveformParameters[j].sampleNum]);
						yVariedWaveformReads.push_back(new ViReal64[allYWaveformParameters[j].sampleNum + allYWaveformParameters[j].signalNum]);
						xVariedWaveforms.push_back(new ViReal64[allXWaveformParameters[j].sampleNum]);
						xVariedWaveformReads.push_back(new ViReal64[allXWaveformParameters[j].sampleNum + allXWaveformParameters[j].signalNum]);
					}

					myNIAWG::getVariedWaveform(allXWaveformParameters[j], allXWaveformParameters, j, libWaveformArray, fileOpenedStatus,
						xVariedWaveforms[xVarWriteCount]);
					myNIAWG::getVariedWaveform(allYWaveformParameters[j], allYWaveformParameters, j, libWaveformArray, fileOpenedStatus,
						yVariedWaveforms[yVarWriteCount]);
					if (repeatFlag == true || rewriteFlag == true)
					{
						if (rewriteFlag == true)
						{
							// see above comment about deleting in this case.
							delete[] mixedWaveforms[mixedCount];
						}
						variedMixedSize[mixedCount] = 2 * allXWaveformParameters[j].sampleNum;
						mixedWaveforms[mixedCount] = new ViReal64[variedMixedSize[mixedCount]];
					}
					else if (repeatFlag == false && rewriteFlag == false)
					{
						variedMixedSize.push_back(2 * allXWaveformParameters[j].sampleNum);
						mixedWaveforms.push_back(new ViReal64[variedMixedSize[mixedCount]]);
					}
					myNIAWG::script::mixWaveforms(xVariedWaveforms[xVarWriteCount], yVariedWaveforms[yVarWriteCount], mixedWaveforms[mixedCount],
						allXWaveformParameters[j].sampleNum);

					// reset initial phase if needed.
					if (j != 0)
					{
						if (allXWaveformParameters[j - 1].phaseManagementOption == 1)
						{
							for (int z = 0; z < allXWaveformParameters[j].signalNum; z++)
							{
								allXWaveformParameters[j].signals[z].initPhase = 0;
							}
						}
						if (allYWaveformParameters[j - 1].phaseManagementOption == 1)
						{
							for (int z = 0; z < allYWaveformParameters[j].signalNum; z++)
							{
								allYWaveformParameters[j].signals[z].initPhase = 0;
							}
						}
					}

					if (correctionFlag == true)
					{
						// then I need to go back.
						// Set the correctionFlag back to false so that I don't go back again unless it is called for.
						correctionFlag = false;
						j -= 2;
						xVarWriteCount -= 2;
						yVarWriteCount -= 2;
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
					xVarWriteCount++;
					yVarWriteCount++;
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
			for (int contInc = 2; contInc < allXWaveformParameters.size(); contInc++)
			{
				// if two waveforms have the same number of parameters...
				if (allXWaveformParameters[contInc].signals.size() == allXWaveformParameters[contInc - 1].signals.size())
				{
					for (int waveInc = 0; waveInc < allXWaveformParameters[contInc].signals.size(); waveInc++)
					{
						if (allXWaveformParameters[contInc].signals[waveInc].initPower != allXWaveformParameters[contInc - 1].signals[waveInc].finPower)
						{
							std::string message = "Warning: Amplitude jump at waveform #" + std::to_string(contInc) + " in X component detected!\r\n";
							postMyString(eErrorTextMessageID, message);
						}
						if (allXWaveformParameters[contInc].signals[waveInc].freqInit != allXWaveformParameters[contInc - 1].signals[waveInc].freqFin)
						{
							std::string message = "Warning: Frequency jump at waveform #" + std::to_string(contInc) + " in X component detected!\r\n";
							postMyString(eErrorTextMessageID, message);
						}
						if (allXWaveformParameters[contInc].signals[waveInc].initPhase - allXWaveformParameters[contInc - 1].signals[waveInc].finPhase
					> CORRECTION_WAVEFORM_ERROR_THRESHOLD)
						{
							std::string message = "Warning: Phase jump (greater than what's wanted for correction waveforms) at waveform #" + std::to_string(contInc) + " in X component "
								"detected!\r\n";
							postMyString(eErrorTextMessageID, message);
						}
						// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
						if (allXWaveformParameters[contInc].signals[waveInc].powerRampType != "nr"
							&& (allXWaveformParameters[contInc].signals[waveInc].initPower == allXWaveformParameters[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to amplitude ramp, but the initial and final "
								"amplitudes are the same. This is not a ramp.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
						if (allXWaveformParameters[contInc].signals[waveInc].freqRampType != "nr"
							&& (allXWaveformParameters[contInc].signals[waveInc].freqInit == allXWaveformParameters[contInc].signals[waveInc].freqFin))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to frequency ramp, but the initial and final "
								"frequencies are the same. This is not a ramp.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}

						// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually ramping.
						if (allXWaveformParameters[contInc].signals[waveInc].powerRampType == "nr"
							&& (allXWaveformParameters[contInc].signals[waveInc].initPower != allXWaveformParameters[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to no amplitude ramp, but the initial and final "
								"amplitudes are the different. This is not a ramp, the initial value will be used.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
						if (allXWaveformParameters[contInc].signals[waveInc].freqRampType == "nr"
							&& (allXWaveformParameters[contInc].signals[waveInc].freqInit != allXWaveformParameters[contInc].signals[waveInc].freqInit))
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set to no frequency ramp, but the initial and final "
								"frequencies are different. This is not a ramp, the initial value will be used throughout.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						if (allXWaveformParameters[contInc].phaseManagementOption == -1 && allXWaveformParameters[contInc].signals[waveInc].phaseOption != -1)
						{
							std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set correct the phase of the previous waveform, "
								"but is not using the final phase of the previous waveform. If you want phase continuity, set the initial phase of this waveform"
								" to be -1, which is code for grabbing the final phase of the previous waveform.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						if (contInc != 0)
						{
							// check if user is trying to copy the final phase of the correction waveform. You aren't supposed to do that.
							if (allXWaveformParameters[contInc - 1].phaseManagementOption == 1 && allXWaveformParameters[contInc].signals[waveInc].phaseOption != 0)
							{
								std::string warningText = "Warning: X waveform #" + std::to_string(contInc) + "is set correct the phase of the following waveform, "
									"but the following waveform is grabbing the phase of this correction waveform. It's not supposed to do that, it needs to"
									" start at zero phase in order for this to work correctly.\r\n";
								postMyString(eErrorTextMessageID, warningText);
							}
						}
					}
				}
				if (allYWaveformParameters[contInc].signals.size() == allYWaveformParameters[contInc - 1].signals.size())
				{
					for (int waveInc = 0; waveInc < allYWaveformParameters[contInc].signals.size(); waveInc++) {
						if (allYWaveformParameters[contInc].signals[waveInc].initPower != allYWaveformParameters[contInc - 1].signals[waveInc].finPower)
						{
							std::string message = "Warning: Amplitude jump at waveform #" + std::to_string(contInc) + " in Y component detected!\r\n";
							postMyString(eErrorTextMessageID, message);
						}
						if (allYWaveformParameters[contInc].signals[waveInc].freqInit != allYWaveformParameters[contInc - 1].signals[waveInc].freqFin)
						{
							std::string message = "Warning: Frequency jump at waveform #" + std::to_string(contInc) + " in Y component detected!\r\n";
							postMyString(eErrorTextMessageID, message);

						}
						if (allYWaveformParameters[contInc].signals[waveInc].initPhase - allYWaveformParameters[contInc - 1].signals[waveInc].finPhase
					> CORRECTION_WAVEFORM_ERROR_THRESHOLD)
						{
							std::string message = "Warning: Phase jump (greater than what's wanted for correction waveforms) at waveform #" + std::to_string(contInc) + " in Y component "
								"detected!\r\n";
							postMyString(eErrorTextMessageID, message);
						}
						// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
						if (allYWaveformParameters[contInc].signals[waveInc].powerRampType != "nr"
							&& (allYWaveformParameters[contInc].signals[waveInc].initPower == allYWaveformParameters[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to amplitude ramp, but the initial and final "
								"amplitudes are the same. This is not a ramp.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
						if (allYWaveformParameters[contInc].signals[waveInc].freqRampType != "nr"
							&& (allYWaveformParameters[contInc].signals[waveInc].freqInit == allYWaveformParameters[contInc].signals[waveInc].freqFin))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to frequency ramp, but the initial and final "
								"frequencies are the same. This is not a ramp.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually ramping.
						if (allYWaveformParameters[contInc].signals[waveInc].powerRampType == "nr"
							&& (allYWaveformParameters[contInc].signals[waveInc].initPower != allYWaveformParameters[contInc].signals[waveInc].finPower))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to no amplitude ramp, but the initial and final "
								"amplitudes are the different. This is not a ramp, the initial value will be used.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
						if (allYWaveformParameters[contInc].signals[waveInc].freqRampType == "nr"
							&& (allYWaveformParameters[contInc].signals[waveInc].freqInit != allYWaveformParameters[contInc].signals[waveInc].freqInit))
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set to no frequency ramp, but the initial and final "
								"frequencies are different. This is not a ramp, the initial value will be used throughout.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						if (allYWaveformParameters[contInc].phaseManagementOption == -1 && allYWaveformParameters[contInc].signals[waveInc].phaseOption != -1)
						{
							std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set correct the phase of the previous waveform, "
								"but is not using the final phase of the previous waveform. If you want phase continuity, set the initial phase of this waveform"
								" to be -1, which is code for grabbing the final phase of the previous waveform.\r\n";
							postMyString(eErrorTextMessageID, warningText);
						}
						if (contInc != 0)
						{
							// check if user is trying to copy the final phase of the correction waveform. You aren't supposed to do that.
							if (allYWaveformParameters[contInc - 1].phaseManagementOption == 1 && allYWaveformParameters[contInc].signals[waveInc].phaseOption != 0)
							{
								std::string warningText = "Warning: Y waveform #" + std::to_string(contInc) + "is set correct the phase of the following waveform, "
									"but the following waveform is grabbing the phase of this correction waveform. It's not supposed to do that, it needs to"
									" start at zero phase in order for this to work correctly.\r\n";
								postMyString(eErrorTextMessageID, warningText);
							}
						}
					}
				}
			}
			/// Wait until previous script has finished.
			// Update General Status Text & Color.
			PostMessage(eMainWindowHandle, eGreenMessageID, 0, 0);
			std::string message = "Outputting Series #" + std::to_string(varValueLengthInc) + ". Ready and Waiting to Initialize Script from series #"
				+ std::to_string(varValueLengthInc + 1) + ".\r\n";
			for (int varInc = 0; varInc < (*inputStruct).threadVariableNames.size(); varInc++)
			{
				// skip the first time.
				if (varValueLengthInc != 0)
				{
					message += (*inputStruct).threadVariableNames[varInc] + " = " + std::to_string(variableValues[varInc][varValueLengthInc - 1]) + "; ";
				}
			}
			postMyString(eColoredEditMessageID, message);
			/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///
			///					wait until done
			///
			isDoneTest = VI_FALSE;
			// If running the default script, stop the default script so that a moment later, when the program checks if the output is done, the output
			// will be done.
			if ((*inputStruct).threadCurrentScript == "DefaultScript" && (*inputStruct).threadDontActuallyGenerate == false)
			{
				if (!SAFEMODE)
				{
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
					{
						delete inputStruct;
						return -1;
					}
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_AbortGeneration(eSessionHandle)))
					{
						delete inputStruct;
						return -1;
					}
				}
			}
			if (systemAbortCheck())
			{
				for (int deleteInc = 0; deleteInc < yVariedWaveforms.size(); deleteInc++)
				{
					delete[] yVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < yVariedWaveformReads.size(); deleteInc++)
				{
					delete[] yVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < xVariedWaveforms.size(); deleteInc++)
				{
					delete[] xVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < xVariedWaveformReads.size(); deleteInc++)
				{
					delete[] xVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
				{
					delete[] mixedWaveforms[deleteInc];
				}
				delete inputStruct;
				return 0;
			}
			// set the waiting event.
			SetEvent(eWaitingForNIAWGEvent);
			// wait untill the waiting thread completes.
			int result = WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);
			if (systemAbortCheck())
			{
				for (int deleteInc = 0; deleteInc < yVariedWaveforms.size(); deleteInc++)
				{
					delete[] yVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < yVariedWaveformReads.size(); deleteInc++)
				{
					delete[] yVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < xVariedWaveforms.size(); deleteInc++)
				{
					delete[] xVariedWaveforms[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < xVariedWaveformReads.size(); deleteInc++)
				{
					delete[] xVariedWaveformReads[deleteInc];
				}
				for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
				{
					delete[] mixedWaveforms[deleteInc];
				}
				delete inputStruct;
				return 0;
			}
			// check this flag set by the wait thread.
			if (eWaitError == true)
			{
				eWaitError = false;

				myNIAWG::NIAWG_CheckProgrammingError(-1);
				delete inputStruct;
				return -1;
			}

			if ((*inputStruct).threadProgramIntensityOption == true && varValueLengthInc != 0)
			{
				if (myErrorHandler(myAgilent::selectIntensityProfile(boost::numeric_cast<int>(varValueLengthInc), intIsVaried, intensitySequenceMinAndMaxVector),
					"ERROR: intensity profile selection failed!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles, false, eError,
					eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
				{
					postMyString(eErrorTextMessageID, "ERROR: intensity profile selection failed!\r\n");
					PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
					delete inputStruct;
					return -1;
				}
				if ((*inputStruct).threadProgramIntensityOption == true && intIsVaried == true)
				{
					postMyString(eStatusTextMessageID, "Intensity Profile Selected.\r\n");
				}
			}
			/// Reinitialize Waveform Generation
			// Tell the master to begin the experiment
			isDoneTest = VI_FALSE;

			std::string varBaseString = "Variable loop #" + std::to_string(varValueLengthInc + 1) + "/" + std::to_string(variableValuesLengths[0]) + ". Variable values are:\r\n";
			for (int varNumInc = 0; varNumInc < (*inputStruct).threadVariableNames.size(); varNumInc++)
			{
				varBaseString += "\t" + (*inputStruct).threadVariableNames[varNumInc] + " = " + std::to_string(variableValues[varNumInc][varValueLengthInc]) + "\r\n";
			}
			postMyString(eStatusTextMessageID, varBaseString);
			// Restart Waveform
			if ((*inputStruct).threadDontActuallyGenerate == false)
			{
				if (!SAFEMODE)
				{
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
					{
						delete inputStruct;
						return -1;
					}
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_AbortGeneration(eSessionHandle)))
					{
						delete inputStruct;
						return -1;
					}
				}
				// skip defaults so start at 2.
				for (int o = 2; o < waveformCount; o++)
				{
					// delete old waveforms
					ViChar variedWaveformName[11];
					sprintf_s(variedWaveformName, 11, "Waveform%i", o);
					if (xWaveformIsVaried[o] == true || yWaveformIsVaried[o] == true)
					{
						if (varValueLengthInc != 0)
						{
							if (!SAFEMODE)
							{
								if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_DeleteNamedWaveform(eSessionHandle, SESSION_CHANNELS, variedWaveformName)))
								{
									delete inputStruct;
									return -1;
								}
							}
						}
						if (!SAFEMODE)
						{
							// And write the new one.
							if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, variedWaveformName,
								variedMixedSize[mixedWriteCount] / 2)))
							{
								delete inputStruct;
								return -1;
							}
							if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, variedWaveformName, variedMixedSize[mixedWriteCount],
								mixedWaveforms[mixedWriteCount])))
							{
								delete inputStruct;
								return -1;
							}
						}
						mixedWriteCount++;
					}
				}

				ViBoolean individualAccumulationIsDone = false;
				if (!SAFEMODE)
				{
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, userScriptSubmit)))
					{
						delete inputStruct;
						return -1;
					}
				}
				userScriptIsWritten = true;
				if (!SAFEMODE)
				{
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript")))
					{
						delete inputStruct;
						return -1;
					}
				}

				(*inputStruct).threadCurrentScript = "UserScript";
				if (!SAFEMODE)
				{
					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
					{
						delete inputStruct;
						return -1;
					}

					if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_InitiateGeneration(eSessionHandle)))
					{
						delete inputStruct;
						return -1;
					}
				}
				if ((*inputStruct).threadConnectToMaster == true)
				{
					if (!SAFEMODE)
					{
						// Send returns -1 if failed, 0 otherwise.
						iResult = send(ConnectSocket, "go", 2, 0);
						if (myErrorHandler(iResult == -1, "ERROR: send failed!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles,
							false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, false, true))
						{
							postMyString(eErrorTextMessageID, "ERROR: send failed!\r\n");
							PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
							delete inputStruct;
							return -1;
						}
					}
					iResult = 0;
				}
				eWaitError = false;
				// create the waiting thread.
				unsigned int NIAWGThreadID;
				eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex(0, 0, NIAWGWaitThread, &eSessionHandle, 0, &NIAWGThreadID);
				// clear some memory.
				for (int cl = 0; cl < xVariedWaveforms.size(); cl++)
				{
					delete[] xVariedWaveforms[cl];
				}
				xVariedWaveforms.clear();
				for (int cl = 0; cl < yVariedWaveforms.size(); cl++)
				{
					delete[] yVariedWaveforms[cl];
				}
				yVariedWaveforms.clear();
				for (int cl = 0; cl < xVariedWaveformReads.size(); cl++)
				{
					delete[] xVariedWaveformReads[cl];
				}
				xVariedWaveformReads.clear();
				for (int cl = 0; cl < yVariedWaveformReads.size(); cl++)
				{
					delete[] yVariedWaveformReads[cl];
				}
				yVariedWaveformReads.clear();
				for (int cl = 0; cl < mixedWaveforms.size(); cl++)
				{
					delete[] mixedWaveforms[cl];
				}
				mixedWaveforms.clear();
				variedMixedSize.clear();
			}
		}
		// close things
		for (int sequenceInc = 0; sequenceInc < (*inputStruct).threadSequenceFileNames.size(); sequenceInc++)
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
	///					If no Variables
	///
	else
	{
		postMyString(eStatusTextMessageID, "NO Variable looping this run.\r\n");

		if ((*inputStruct).threadDontActuallyGenerate == false)
		{
			ViBoolean individualAccumulationIsDone = false;
			if (!SAFEMODE)
			{
				if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
				{
					delete inputStruct;
					return -1;
				}
				if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_AbortGeneration(eSessionHandle)))
				{
					delete inputStruct;
					return -1;
				}
			}
			// Should be just ready to go
			if (!SAFEMODE)
			{
				if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, userScriptSubmit)))
				{
					delete inputStruct;
					return -1;
				}
			}
			userScriptIsWritten = true;
			if (!SAFEMODE)
			{
				if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript")))
				{
					delete inputStruct;
					return -1;
				}
			}
			(*inputStruct).threadCurrentScript = "UserScript";
			if (!SAFEMODE)
			{
				if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
				{
					delete inputStruct;
					return -1;
				}
				if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_InitiateGeneration(eSessionHandle)))
				{
					delete inputStruct;
					return -1;
				}
				if ((*inputStruct).threadConnectToMaster == true)
				{
					iResult = send(ConnectSocket, "go", 2, 0);
					if (myErrorHandler(iResult == -1, "ERROR: intensity profile selection failed!\r\n", ConnectSocket, verticalScriptFiles, horizontalScriptFiles,
						false, eError, eSessionHandle, userScriptIsWritten, userScriptName, true, true, true))
					{
						postMyString(eErrorTextMessageID, "ERROR: intensity profile selection failed!\r\n");
						PostMessage(eMainWindowHandle, eFatalErrorMessageID, 0, 0);
						delete inputStruct;
						return -1;
					}
				}
			}

			eWaitError = false;
			unsigned int NIAWGThreadID;
			eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex(0, 0, NIAWGWaitThread, &eSessionHandle, 0, &NIAWGThreadID);
		}
		// close some things
		for (int sequenceInc = 0; sequenceInc < (*inputStruct).threadSequenceFileNames.size(); sequenceInc++)
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
	if ((*inputStruct).threadAccumulations == 0 || (eVariableNames.size() == 0 && SAFEMODE))
	{
		postMyString(eStatusTextMessageID, "Scripts Loaded into NIAWG. This waveform sequence will run until aborted by the user.\r\n\r\n");
		postMyString(eColoredEditMessageID, "Scripts Loaded into NIAWG. This waveform sequence will run until aborted by the user.");
		PostMessage(eMainWindowHandle, eGreenMessageID, 0, 0);
	}
	else 
	{
		std::string message = "Outputting LAST Series #" + std::to_string((*inputStruct).threadVariableNames.size()) + ".\r\n";
		for (int varInc = 0; varInc < (*inputStruct).threadVariableNames.size(); varInc++)
		{
			message += (*inputStruct).threadVariableNames[varInc] + " = " + std::to_string(variableValues[varInc].back()) + "; ";
		}
		PostMessage(eMainWindowHandle, eGreenMessageID, 0, 0);
		postMyString(eColoredEditMessageID, message);
		postMyString(eStatusTextMessageID, "Completed Sending Scripts. Waiting for last script to finish.\r\n\r\n");		
	}
	
	if (systemAbortCheck())
	{
		for (int deleteInc = 0; deleteInc < yVariedWaveforms.size(); deleteInc++)
		{
			delete[] yVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < yVariedWaveformReads.size(); deleteInc++)
		{
			delete[] yVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < xVariedWaveforms.size(); deleteInc++)
		{
			delete[] xVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < xVariedWaveformReads.size(); deleteInc++)
		{
			delete[] xVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
		{
			delete[] mixedWaveforms[deleteInc];
		}
		delete inputStruct;
		return 0;
	}
	// wait...
	int result2 = WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);
	if (systemAbortCheck())
	{
		for (int deleteInc = 0; deleteInc < yVariedWaveforms.size(); deleteInc++)
		{
			delete[] yVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < yVariedWaveformReads.size(); deleteInc++)
		{
			delete[] yVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < xVariedWaveforms.size(); deleteInc++)
		{
			delete[] xVariedWaveforms[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < xVariedWaveformReads.size(); deleteInc++)
		{
			delete[] xVariedWaveformReads[deleteInc];
		}
		for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
		{
			delete[] mixedWaveforms[deleteInc];
		}
		delete inputStruct;
		return 0;
	}
	// check this flag set by the wait thread.
	if (eWaitError)
	{
		eWaitError = false;
		myNIAWG::NIAWG_CheckProgrammingError(-1);
		delete inputStruct;
		return -1;
	}
	// Clear waveforms off of NIAWG (not working??? memory appears to still run out...)
	ViChar waveformDeleteName[11];
	for (int v = 2; v < waveformCount; v++)
	{
		sprintf_s(waveformDeleteName, 11, "Waveform%i", v);
		if (!SAFEMODE)
		{
			if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_DeleteNamedWaveform(eSessionHandle, SESSION_CHANNELS, waveformDeleteName)))
			{
				delete inputStruct;
				return -1;
			}
		}
	}
	if ((*inputStruct).threadDontActuallyGenerate == false)
	{
		if (!SAFEMODE)
		{
			// Delete relevant onboard memory.
			if (myNIAWG::NIAWG_CheckProgrammingError(niFgen_DeleteScript(eSessionHandle, SESSION_CHANNELS, userScriptName)))
			{
				delete inputStruct;
				return -1;
			}
		}
		userScriptIsWritten = false;
	}
	// Delete the user script
	delete[] userScriptSubmit;
	// Delete waveforms in memory

	for (int cl = 0; cl < xVariedWaveforms.size(); cl++)
	{
		delete[] xVariedWaveforms[cl];

	}
	xVariedWaveforms.clear();
	for (int cl = 0; cl < yVariedWaveforms.size(); cl++)
	{
		delete[] yVariedWaveforms[cl];
	}
	yVariedWaveforms.clear();
	for (int cl = 0; cl < xVariedWaveformReads.size(); cl++)
	{
		delete[] xVariedWaveformReads[cl];
	}
	xVariedWaveformReads.clear();
	for (int cl = 0; cl < yVariedWaveformReads.size(); cl++)
	{
		delete[] yVariedWaveformReads[cl];
	}
	yVariedWaveformReads.clear();
	for (int cl = 0; cl < mixedWaveforms.size(); cl++)
	{
		delete[] mixedWaveforms[cl];
	}
	mixedWaveforms.clear();

	PostMessage(eMainWindowHandle, eNormalFinishMessageID, 0, 0);
	delete inputStruct;
	return 0;
}
