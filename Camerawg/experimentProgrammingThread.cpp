#include "stdafx.h"
#include "experimentProgrammingThread.h"
#include "externals.h"
#include "constants.h"
// Some headers used for communication protocols.
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include "myAgilent.h"
#include "NiawgController.h"
#include "experimentThreadInputStructure.h"
#include <sstream>
#include "NIAWGWaitThread.h"
#include "boost/cast.hpp"
#include "systemAbortCheck.h"
#include "VariableSystem.h"
#include <boost/algorithm/string/replace.hpp>
#include <afxsock.h>
#include "SocketWrapper.h"
#include "ExperimentLogger.h"


/*
 * This runs the experiment. It calls analyzeNiawgScripts and then procedurally goes through all variable values. It also communicates with
 *  the other computer throughout the process. inputParam is the list of all of the relevant parameters to be used during this run of the experiment.
 */
unsigned __stdcall experimentProgrammingThread(LPVOID inputParam)
{
	///	Initializing and Checking Options /////////////////////////////////////////////////////////////////////////////////////////////////
	experimentThreadInputStructure* input = (experimentThreadInputStructure*)inputParam;
	AfxSocketInit();
	bool deleteWaveforms = false;
	SocketWrapper masterSocket;
	std::vector<std::size_t> variableValuesLengths;
	std::vector<std::vector<double> > variableValues;
	niawgPair<std::vector<std::fstream>> scriptFiles;
	std::vector<std::fstream> intensityScriptFiles;
	ViChar* userScriptSubmit;
	bool intIsVaried;
	std::vector<long int> variedMixedSize;
	std::vector<myMath::minMaxDoublet> intensitySequenceMinAndMaxVector;
	std::string finalUserScriptString, userScriptNameString;
	std::vector<ViReal64 *> mixedWaveforms;
	// initialize the script string. The script needs a script name at the top.
	userScriptNameString = "experimentScript";
	std::vector<variable> singletons, variables;
	try
	{ 
		ConfigurationFileSystem::getConfigInfo( scriptFiles, intensityScriptFiles, input->profile, singletons, variables );

		if (input->settings.connectToMaster == true)
		{
			masterSocket.initialize();
			input->comm->sendStatus( "Attempting to connect......" );
			masterSocket.connect();
			input->comm->sendStatus( "Established Connection!\r\nWaiting for Repetition # from master..." );
			input->repetitions = masterSocket.getAccumulations( input->profile.sequenceConfigNames.size() );
			input->comm->sendStatus( "Recieved!\r\n" );
		}
		else
		{
			input->repetitions = 0;
		}
		input->comm->sendStatus( "Repetition # after sequence normalization: " + std::to_string( (*input).repetitions ) + "\r\n" );
		input->comm->sendStatus( "Beginning Waveform Read, Calculate, and Write Procedure...\r\n" );

		bool dummy;
		systemAbortCheck( input->comm, dummy );

		std::vector<std::string> workingUserScriptString;
		workingUserScriptString.resize( (input->profile.sequenceConfigNames.size()) );	
		outputInfo output;
		library libWaveformArray;
		// initialize to 2 because of default waveforms...
		output.waveCount = 2;
		output.predefinedWaveCount = 0;
		output.chan[Vertical].waveInfo.resize( 2 );
		output.chan[Horizontal].waveInfo.resize( 2 );
		for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
		{
			output.niawgLanguageScript = "";
			input->comm->sendStatus("Working with configuraiton # " + std::to_string(sequenceInc + 1) + " in Sequence...\r\n");
			/// Create Script and Write Waveforms ////////////////////////////////////////////////////////////////////////////////////////////
			niawgPair<ScriptStream> scripts;
			scripts[Vertical] << scriptFiles[Vertical][sequenceInc].rdbuf();
			scripts[Horizontal] << scriptFiles[Horizontal][sequenceInc].rdbuf();
			
			std::string warnings, debugMessages;
			input->niawg->analyzeNiawgScripts( scripts, output, libWaveformArray, input->profile, singletons, input->debugInfo, warnings, 
											   debugMessages );
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
			bool dummy;
			systemAbortCheck( input->comm, dummy );
		}
		/// Final Cleanup before waveform initiation
		input->comm->sendStatus("Constant Waveform Preparation Completed...\r\n");
		// format the script to send to the 5451 according to the accumulation number and based on the number of sequences.
		finalUserScriptString = "script " + userScriptNameString + "\n";
		if (input->repetitions == 0)
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
			for (unsigned int accumCount = 0; accumCount < (*input).repetitions; accumCount++)
			{
				for (int sequenceInc = 0; sequenceInc < workingUserScriptString.size(); sequenceInc++)
				{
					finalUserScriptString += workingUserScriptString[sequenceInc];
				}
			}
		}
		// the NIAWG requires that the script file must end with "end script".
		finalUserScriptString += "end Script";
		if (input->debugInfo.outputNiawgMachineScript)
		{
			input->comm->sendDebug( boost::replace_all_copy( "Single Repetition NIAWG Machine Script:\n"
															 + finalUserScriptString + "end Script\n\n", "\n", "\r\n" ) );
		}
		// Convert script string to ViConstString. +1 for a null character on the end.
		unsigned int scriptLength = boost::numeric_cast<unsigned int>(finalUserScriptString.length() + 1);
		userScriptSubmit = new ViChar[scriptLength];
		sprintf_s(userScriptSubmit, scriptLength, "%s", finalUserScriptString.c_str());
		strcpy_s(userScriptSubmit, scriptLength, finalUserScriptString.c_str());

		///					Begin Handling Variables
		/// get var files from master if necessary
		if (input->settings.getVariables == true)
		{
			variableValues.resize(variables.size(), std::vector<double>(0));
			variableValuesLengths.resize(variables.size());
			std::string recievedMsg;
			for (int variableNameInc = 0; variableNameInc < variables.size(); variableNameInc++)
			{
				std::string message = "Waiting for Variable Set #" + std::to_string(variableNameInc + 1) + "... ";
				input->comm->sendStatus(message);
				if (!TWEEZER_COMPUTER_SAFEMODE)    
				{
					masterSocket.send( "next variable" );
					recievedMsg = masterSocket.recieve();
				}
				input->comm->sendStatus("Received!\r\n");
				int varNameCursor = -1;
				std::string tempVarName;
				std::stringstream variableStream;
				variableStream << recievedMsg;
				variableStream >> tempVarName;
				for (int parameterInc = 0; parameterInc < variables.size(); parameterInc++)
				{
					if (tempVarName == (variables[parameterInc]).name)
					{
						varNameCursor = parameterInc;
						break;
					}
				}
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					if (varNameCursor == -1)
					{
						thrower( "ERROR: The variable name sent by the master computer (" + tempVarName + ") doesn't match any current "
									"variables!\r\n" );
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
						if (variables[varNameCursor].timelike)
						{
							if (input->niawg->waveformSizeCalc(tempDouble) % 4 != 0)
							{
								thrower("ERROR: a timelike value sent by the master computer did not correspond to an integer number of 4 samples. The "
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
			}
		}

		ExperimentLogger::generateNiawgLog( input, scriptFiles, intensityScriptFiles );

		/// Variable Cleanup Before Execution
		// check that variable files are the same length.
		for (std::size_t varNameInc = 0; varNameInc + 1 < variableValuesLengths.size(); varNameInc++)
		{
			if (variableValuesLengths[varNameInc] != variableValuesLengths[varNameInc + 1])
			{
				thrower("Error: lengths of variable values are not all the same! They must be the same\r\n");
			}
		}

		std::vector<std::vector<POINT>> intensityPoints;

		if (input->settings.programIntensity == true)
		{
			input->comm->sendStatus("Programing Intensity Profile(s)...");
			input->comm->sendColorBox( { /*niawg*/'-', /*camera*/'-', /*intensity*/'Y' } );
			myAgilent::programIntensity( boost::numeric_cast<int>(variables.size()), variables, variableValues, intIsVaried, 
										 intensitySequenceMinAndMaxVector, intensityPoints, intensityScriptFiles, singletons, input->profile );
			input->comm->sendStatus("Complete!\r\n");
			// select the first one.
			myAgilent::selectIntensityProfile( 0, intIsVaried, intensitySequenceMinAndMaxVector );
			input->comm->sendStatus("Intensity Profile Selected.\r\n");
		}
		else
		{
			input->comm->sendStatus("Intensity Profile NOT being programed.\r\n");
		}

		///	Big Variable Handling Loop
		// If there are any varible files, enter big variable loop.
		if (variables.size() > 0)
		{
			input->comm->sendStatus("Begin Variation Loop.\r\n");
			eWaitingForNIAWGEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("eWaitingForNIAWGEvent"));
			// for every variation of variable values in the files...
			for (std::size_t variation = 0; variation < variableValuesLengths[0]; variation++)
			{
				std::string warnings, debugMsgs;
				input->niawg->handleVariations( output ,variables, variableValues, variation, variedMixedSize, mixedWaveforms, warnings, 
												debugMsgs, libWaveformArray, input->debugInfo);
				if (warnings != "")
				{
					input->comm->sendError( warnings );
				}
				if (debugMsgs != "")
				{
					input->comm->sendDebug( debugMsgs );
				}
				input->niawg->checkThatWaveformsAreSensible( input->comm, output );

				/// Wait until previous script has finished.
				// Update General Status Text & Color.
				std::string message = "Outputting Series #" + std::to_string(variation) + ". Ready and Waiting to Initialize Script "
					"from series #" + std::to_string(variation + 1) + ".\r\n";
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					// skip the first time.
					if (variation != 0)
					{
						message += variables[varInc].name + " = " + std::to_string(variableValues[varInc][variation - 1]) + "; ";
					}
				}
				input->comm->sendColorBox( { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' } );
				input->comm->sendStatus( message );
				///					wait until done
				// If running the default script, stop the default script so that a moment later, when the program checks if the output is 
				// done, the output will be done.
				if (input->currentScript == "DefaultScript" && input->dontActuallyGenerate == false)
				{
					input->niawg->configureOutputEnabled( VI_FALSE );
					input->niawg->abortGeneration();
				}
			
				///
				systemAbortCheck( input->comm, deleteWaveforms );
				// set the waiting event.
				SetEvent(eWaitingForNIAWGEvent);
				// wait untill the waiting thread completes.
				WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);

				systemAbortCheck( input->comm, deleteWaveforms );

				// check this flag set by the wait thread.
				if (eWaitError == true)
				{
					eWaitError = false;
					thrower("ERROR: Error in the wait function!\r\n");
				}

				if (input->settings.programIntensity == true && variation != 0)
				{
					input->comm->sendColorBox( { /*niawg*/'-', /*camera*/'-', /*intensity*/'Y' } );
					myAgilent::selectIntensityProfile( boost::numeric_cast<int>(variation), intIsVaried, 
													   intensitySequenceMinAndMaxVector );
					if (input->settings.programIntensity == true && intIsVaried == true)
					{
						input->comm->sendStatus("Intensity Profile Selected.\r\n");
					}
					input->comm->sendColorBox( { /*niawg*/'-', /*camera*/'-', /*intensity*/'G' } );
				}
				// Tell the master to begin the experiment
				std::string varBaseString = "Variable loop #" + std::to_string(variation + 1) + "/" 
					+ std::to_string(variableValuesLengths[0]) + ". Variable values are:\r\n";
				for (int varNumInc = 0; varNumInc < variables.size(); varNumInc++)
				{
					varBaseString += "\t" + variables[varNumInc].name + " = " + std::to_string(variableValues[varNumInc][variation]) 
						+ "\r\n";
				}
				input->comm->sendStatus(varBaseString);
				// Restart Waveform
				if (input->dontActuallyGenerate == false)
				{
					input->niawg->configureOutputEnabled(VI_FALSE);
					input->niawg->abortGeneration();
					// skip defaults so start at 2.
					int mixedWriteCount = 0;
					for (int waveInc = 2; waveInc < output.waveCount; waveInc++)
					{
						// delete old waveforms
						ViChar variedWaveformName[11];
						sprintf_s(variedWaveformName, 11, "Waveform%i", waveInc);
						if (output.chan[Vertical].waveInfo[waveInc].varies == true || output.chan[Horizontal].waveInfo[waveInc].varies == true)
						{
							if (variation != 0)
							{
								input->niawg->deleteWaveform(variedWaveformName);
							}
							// And write the new one.
							input->niawg->allocateWaveform(variedWaveformName, variedMixedSize[mixedWriteCount] / 2);
							input->niawg->writeWaveform(variedWaveformName, variedMixedSize[mixedWriteCount], mixedWaveforms[mixedWriteCount]);
							mixedWriteCount++;
						}
					}
					input->niawg->writeScript(userScriptSubmit);
					input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
					input->currentScript = "UserScript";
					input->niawg->configureOutputEnabled(VI_TRUE);
					input->niawg->initiateGeneration();
					if (input->settings.connectToMaster == true)
					{
						masterSocket.send( "go" );
					}
					eWaitError = false;
					waitThreadInput waitInput;
					waitInput.niawg = input->niawg;
					waitInput.profile = input->profile;
					// create the waiting thread.
					unsigned int NIAWGThreadID;
					eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex(0, 0, niawgWaitThread, &waitInput, 0, &NIAWGThreadID);
					// clear some memory.
					for (auto& waveform : mixedWaveforms)
					{
						delete[] waveform;
					}
					mixedWaveforms.clear();
					variedMixedSize.clear();
				}
			}
		}
		else
		{
			///					If no Variables					///
			input->comm->sendStatus("NO Variable looping this run.\r\n");
			input->niawg->checkThatWaveformsAreSensible( input->comm, output );
			if (input->dontActuallyGenerate == false)
			{
				input->niawg->configureOutputEnabled(VI_FALSE);
				input->niawg->abortGeneration();
				// Should be just ready to go
				input->niawg->writeScript(userScriptSubmit);
				input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
				input->currentScript = "UserScript";
				input->niawg->configureOutputEnabled(VI_TRUE);
				input->niawg->initiateGeneration();
				if (input->settings.connectToMaster == true)
				{
					masterSocket.send( "go" );
				}
				waitThreadInput waitInput;
				waitInput.niawg = input->niawg;
				waitInput.profile = input->profile;
				eWaitError = false;
				unsigned int NIAWGThreadID;
				eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex(0, 0, niawgWaitThread, &waitInput, 0, &NIAWGThreadID);
			}
		}
		///					Cleanup (after all generate code)
		// close things
		for (int sequenceInc = 0; sequenceInc < input->profile.sequenceConfigNames.size(); sequenceInc++)
		{
			for (auto axis : AXES)
			{
				if (scriptFiles[axis][sequenceInc].is_open())
				{
					scriptFiles[axis][sequenceInc].close();
				}
			}
			if (intensityScriptFiles[sequenceInc].is_open())
			{
				intensityScriptFiles[sequenceInc].close();
			}
		}

		if (input->repetitions == 0 || (input->variableNumber == 0 && TWEEZER_COMPUTER_SAFEMODE))
		{
			input->comm->sendColorBox( { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' } );
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
			for (int varInc = 0; varInc < variables.size(); varInc++)
			{
				message += variables[varInc].name + " = " + std::to_string(variableValues[varInc].back()) + "; ";
			}
			input->comm->sendColorBox( { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' } );
			input->comm->sendStatus("Completed Sending Scripts. Waiting for last script to finish.\r\nmessage\r\n");
		}
	
		systemAbortCheck( input->comm, deleteWaveforms );
		WaitForSingleObject(eNIAWGWaitThreadHandle, INFINITE);
		systemAbortCheck( input->comm, deleteWaveforms );

		// check if this flag set by the wait thread.
		if (eWaitError)
		{
			eWaitError = false;
			thrower("ERROR: Wait thread error: " + input->niawg->getErrorMsg());
		}
		// Clear waveforms off of NIAWG (not working??? memory appears to still run out...)
		ViChar waveformDeleteName[11];
		for (int waveformInc = 2; waveformInc < output.waveCount; waveformInc++)
		{
			sprintf_s(waveformDeleteName, 11, "Waveform%i", waveformInc);
			input->niawg->deleteWaveform(waveformDeleteName);
		}
		if (input->dontActuallyGenerate == false)
		{
			input->niawg->deleteScript("experimentScript");
		}
		if (masterSocket.isConnected())
		{
			masterSocket.close();
		}
		delete[] userScriptSubmit;
		for (auto& waveform : mixedWaveforms)
		{
			delete[] waveform;
		}
		mixedWaveforms.clear();
		PostMessage(eMainWindowHandle, eNormalFinishMessageID, 0, 0);

	}
	catch (myException& exception)
	{
		if (eDontActuallyGenerate == false)
		{
			// a check later checks the value and handles it specially in this case.
			eCurrentScript = "continue";
		}
		// close socket.
		if (masterSocket.isConnected())
		{
			masterSocket.close();
		}
		// turn the agilent to the default setting.
		myAgilent::agilentDefault();
		// close files.
		for (auto axis : AXES)
		{
			for (auto& file : scriptFiles[axis])
			{
				if (file.is_open())
				{
					file.close();
				}
			}
		}
		if (deleteWaveforms)
		{
			for (int deleteInc = 0; deleteInc < mixedWaveforms.size(); deleteInc++)
			{
				delete[] mixedWaveforms[deleteInc];
			}
		}
		input->comm->sendFatalError( "ERROR: " + exception.whatStr() );
	}
	delete input;
};

