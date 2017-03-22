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
#include "NiawgWaiter.h"
#include "boost/cast.hpp"
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
	// must be deleted by end of thread.
	experimentThreadInput* input = (experimentThreadInput*)inputParam;
	AfxSocketInit();
	bool deleteWaveforms = false;
	SocketWrapper masterSocket;
	NiawgWaiter waiter;
	std::vector<std::vector<double> > varValues;
	niawgPair<std::vector<std::fstream>> scriptFiles;
	std::vector<std::fstream> intensityScriptFiles;
	// this can be leaked by this code. I should fix that.
	std::vector<ViChar> userScriptSubmit;
	bool intIsVaried;
	std::vector<long int> variedMixedSize;
	std::vector<myMath::minMaxDoublet> intensityRanges;
	std::string userScriptNameString;
	//std::vector<ViReal64 *> mixedWaveforms;
	// initialize the script string. The script needs a script name at the top.
	userScriptNameString = "experimentScript";
	std::vector<variable> singletons, variables;		
	outputInfo output;
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
		input->comm->sendStatus( "Repetition # after sequence normalization: " + std::to_string( input->repetitions ) + "\r\n" );
		input->comm->sendStatus( "Beginning Waveform Read, Calculate, and Write Procedure...\r\n" );
		waiter.systemAbortCheck( input->comm );
		std::vector<std::string> workingUserScripts( input->profile.sequenceConfigNames.size() );

		library libWaveformArray;
		// initialize to 2 because of default waveforms...
		output.waveCount = 2;
		output.predefinedWaveCount = 0;
		output.waves.resize( 2 );
		for (int sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
		{
			niawgPair<ScriptStream> scripts; 
			output.niawgLanguageScript = "";
			input->comm->sendStatus("Working with configuraiton # " + std::to_string(sequenceInc + 1) + " in Sequence...\r\n");
			/// Create Script and Write Waveforms ////////////////////////////////////////////////////////////////////////////////////////
			scripts[Vertical] << scriptFiles[Vertical][sequenceInc].rdbuf();
			scripts[Horizontal] << scriptFiles[Horizontal][sequenceInc].rdbuf();
			std::string warnings;
			input->niawg->analyzeNiawgScripts( scripts, output, input->profile, singletons, input->debugInfo, warnings );
			if (warnings != "")
			{
				input->comm->sendError(warnings);
			}
			if (input->debugInfo.message != "")
			{
				input->comm->sendDebug(input->debugInfo.message);
			}
			workingUserScripts[sequenceInc] = output.niawgLanguageScript;
			// 
			waiter.systemAbortCheck( input->comm );
		}
		input->comm->sendStatus("Constant Waveform Preparation Completed...\r\n");
		input->niawg->finalizeScript(input->repetitions, userScriptNameString, workingUserScripts, userScriptSubmit);
		if (input->debugInfo.outputNiawgMachineScript)
		{
			input->comm->sendDebug( boost::replace_all_copy( "Single Repetition NIAWG Machine Script:\n" 
															 + std::string(userScriptSubmit.begin(), userScriptSubmit.end())
															 + "end Script\n\n", "\n", "\r\n" ) );
		}
		if (input->settings.getVariables == true)
		{
			input->comm->sendStatus( "Getting Variables from Master..." );
			input->niawg->getVariables(masterSocket, varValues, variables);
			input->comm->sendStatus( "Complete!\r\n" );
		}
		ExperimentLogger::generateNiawgLog( input, scriptFiles, intensityScriptFiles );
		if (input->settings.programIntensity == true)
		{
			input->comm->sendStatus("Programing Intensity Profile(s)...");
			input->comm->sendColorBox( { /*niawg*/'-', /*camera*/'-', /*intensity*/'Y' } );
			myAgilent::programIntensity( boost::numeric_cast<int>(variables.size()), variables, varValues, intIsVaried, intensityRanges, 
										 intensityScriptFiles, singletons, input->profile );
			input->comm->sendStatus("Complete!\r\n");
			myAgilent::selectIntensityProfile( 0, intIsVaried, intensityRanges );
			input->comm->sendStatus("Intensity Profile Selected.\r\n");
		}
		else
		{
			input->comm->sendStatus("Intensity Profile NOT being programed.\r\n");
		}

		// If there are any varible files, enter big variable loop.
		if (variables.size() > 0)
		{
			input->comm->sendStatus("Begining Variation Loop.\r\n");
			waiter.initialize();
			/// variation loop
			for (std::size_t variation = 0; variation < varValues[0].size(); variation++)
			{
				std::string warnings;
				input->niawg->handleVariations( output ,variables, varValues, variation, variedMixedSize, warnings, input->debugInfo);
				if (warnings != "")
				{
					input->comm->sendError( warnings );
				}
				if (input->debugInfo.message != "")
				{
					input->comm->sendDebug( input->debugInfo.message );
				}
				input->niawg->checkThatWaveformsAreSensible( input->comm, output );
				std::string message = ("Outputting Series #" + std::to_string( variation ) + ". Ready and Waiting to Initialize Script "
										"from series #" + std::to_string( variation + 1 ) + ".\r\n");
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					// skip the first time.
					if (variation != 0)
					{
						message += variables[varInc].name + " = " + std::to_string(varValues[varInc][variation - 1]) + "; ";
					}
				}
				input->comm->sendColorBox( { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' } );
				input->comm->sendStatus( message );
				// If running the default script, stop the default script so that a moment later, when the program checks if the output is 
				// done, the output will be done.
				if (input->currentScript == "DefaultScript" && input->dontActuallyGenerate == false)
				{
					input->niawg->configureOutputEnabled( VI_FALSE );
					input->niawg->abortGeneration();
				}
				waiter.wait( input->comm, deleteWaveforms );
				if (input->settings.programIntensity == true && variation != 0)
				{
					input->comm->sendColorBox( { /*niawg*/'-', /*camera*/'-', /*intensity*/'Y' } );
					myAgilent::selectIntensityProfile( boost::numeric_cast<int>(variation), intIsVaried, intensityRanges );
					if (input->settings.programIntensity == true && intIsVaried == true)
					{
						input->comm->sendStatus("Intensity Profile Selected.\r\n");
					}
					input->comm->sendColorBox( { /*niawg*/'-', /*camera*/'-', /*intensity*/'G' } );
				}
				// Tell the master to begin the experiment
				std::string varBaseString = ("Variable loop #" + std::to_string( variation + 1 ) + "/"
											  + std::to_string( varValues[0].size() ) + ". Variable values are:\r\n");
				for (int varNumInc = 0; varNumInc < variables.size(); varNumInc++)
				{
					varBaseString += "\t" + variables[varNumInc].name + " = " + std::to_string(varValues[varNumInc][variation]) + "\r\n";
				}
				input->comm->sendStatus(varBaseString);
				// Restart Waveform
				if (input->dontActuallyGenerate == false)
				{
					input->niawg->configureOutputEnabled(VI_FALSE);
					input->niawg->abortGeneration();
					int mixedWriteCount = 0;
					// skip defaults so start at 2.
					for (int waveInc = 2; waveInc < output.waveCount; waveInc++)
					{
						// delete old waveforms
						ViChar variedWaveformName[11];
						sprintf_s(variedWaveformName, 11, "Waveform%i", waveInc);
						if (output.waves[waveInc].varies == true )
						{
							if (variation != 0)
							{
								input->niawg->deleteWaveform(variedWaveformName);
							}
							// And write the new one.
							input->niawg->allocateNamedWaveform(variedWaveformName, variedMixedSize[mixedWriteCount] / 2);
							input->niawg->writeNamedWaveform(variedWaveformName, variedMixedSize[mixedWriteCount], output.waves[waveInc].mixedWaveform.data());
							mixedWriteCount++;
						}
					}
					input->niawg->writeScript(userScriptSubmit.data());
					input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
					input->currentScript = "UserScript";
					input->niawg->configureOutputEnabled(VI_TRUE);
					// initiate generation before telling the master. this is because scripts are supposed to be designed to sit on an 
					// initial waveform until the master sends it a trigger.
					input->niawg->initiateGeneration();
					if (input->settings.connectToMaster == true)
					{
						masterSocket.send( "go" );
					}
					waiter.startWait( input );

					for (int waveInc = 2; waveInc < output.waveCount; waveInc++)
					{
						output.waves[waveInc].mixedWaveform.clear();
						output.waves[waveInc].mixedWaveform.shrink_to_fit();
					}
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
				input->niawg->writeScript(userScriptSubmit.data());
				input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
				input->currentScript = "UserScript";
				input->niawg->configureOutputEnabled(VI_TRUE);
				input->niawg->initiateGeneration();
				if (input->settings.connectToMaster)
				{
					masterSocket.send( "go" );
				}
				waiter.startWait( input );
			}
		}

		if (input->repetitions == 0 || (input->variableNumber == 0 && TWEEZER_COMPUTER_SAFEMODE))
		{
			input->comm->sendColorBox( { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' } );
			input->comm->sendStatus( "Scripts Loaded into NIAWG. This waveform sequence will run until aborted by the user.\r\n" );
		}
		else
		{
			std::string message;
			if (varValues.size() > 0)
			{
				message = "Outputting LAST niawg configuration #" + std::to_string( varValues[0].size() ) + ".\r\n";
			}
			else
			{
				message = "Outputting the only niawg configuration.\r\n";
			}
			for (int varInc = 0; varInc < variables.size(); varInc++)
			{
				message += variables[varInc].name + " = " + std::to_string( varValues[varInc].back() ) + "; ";
			}
			input->comm->sendColorBox( { /*niawg*/'G', /*camera*/'-', /*intensity*/'-' } );
			input->comm->sendStatus( "Completed Sending Scripts. Waiting for last script to finish.\r\nmessage\r\n" );
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

		waiter.wait( input->comm, deleteWaveforms );

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
		for (auto& waveform : output.waves)
		{
			waveform.mixedWaveform.clear();
			waveform.mixedWaveform.shrink_to_fit();
		}
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
		for (auto& waveform : output.waves)
		{
			waveform.mixedWaveform.clear();
			waveform.mixedWaveform.shrink_to_fit();
		}
		input->comm->sendFatalError( "ERROR: " + exception.whatStr() );
	}
	delete input;
	return 0;
};

