#include "stdafx.h"
#include "ExperimentManager.h"

void NiawgAndIntensityManager::startThread( niawgIntensityThreadInput* inputParam )
{
	eAbortNiawgFlag = false;
	eExperimentThreadHandle = (HANDLE)_beginthreadex( 0, 0, &NiawgAndIntensityManager::niawgIntensityProgrammingThread,
		(LPVOID *)inputParam, 0, NULL );
}


/*
 * This runs the niawg and intensity systems. It calls analyzeNiawgScripts and then procedurally goes through all 
 * variable values. It also communicates with the other computer throughout the process. inputParam is the list of all
 * of the relevant parameters to be used during this run of the experiment.
 */
unsigned __stdcall NiawgAndIntensityManager::niawgIntensityProgrammingThread(LPVOID inputParam)
{
	///	Initializing and Checking Options ////////////////////////////////////////////////////////////////////////////
	// must be deleted by end of thread.
	niawgIntensityThreadInput* input = (niawgIntensityThreadInput*)inputParam;
	AfxSocketInit();
	UINT repetitions = 0;
	NiawgWaiter waiter;
	std::vector<std::vector<double> > varValues;
	niawgPair<std::vector<std::fstream>> scriptFiles;
	std::vector<std::fstream> intensityScriptFiles;
	std::string userScriptNameString = "experimentScript";
	std::vector<ViChar> userScriptSubmit;
	bool intIsVaried;
	std::vector<long> variedMixedSize;
	std::vector<myMath::minMax> intensityRanges;
	
	std::vector<variable> singletons, variables;
	NiawgOutputInfo output;
	output.isDefault = false;
	try
	{ 
		ProfileSystem::getConfigInfo( scriptFiles, intensityScriptFiles, input->profile, singletons, variables, 
									  input->settings.programIntensity );
		repetitions = 0;
		input->comm->sendStatus( "Repetition # after sequence normalization: " + str( repetitions ) + "\r\n" );
		input->comm->sendStatus( "Beginning Waveform Read, Calculate, and Write Procedure...\r\n" );
		waiter.systemAbortCheck( input->comm );
		std::vector<std::string> workingUserScripts( input->profile.sequenceConfigNames.size() );
		// initialize to 2 because of default waveforms...
		output.waveCount = 2;
		output.predefinedWaveCount = 0;
		output.waves.resize( 2 );
		// analyze each script in sequence.
		for (int sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
		{
			niawgPair<ScriptStream> scripts;
			output.niawgLanguageScript = "";
			input->comm->sendStatus("Working with configuraiton # " + str(sequenceInc + 1) + " in Sequence...\r\n");
			/// Create Script and Write Waveforms ////////////////////////////////////////////////////////////////////
			scripts[Vertical] << scriptFiles[Vertical][sequenceInc].rdbuf();
			scripts[Horizontal] << scriptFiles[Horizontal][sequenceInc].rdbuf();
			std::string warnings;
			input->niawg->analyzeNiawgScripts( scripts, output, input->profile, input->debugInfo, warnings );
			if (warnings != "")
			{
				input->comm->sendError(warnings);
			}
			if (input->debugInfo.message != "")
			{
				input->comm->sendDebug(input->debugInfo.message);
			}
			workingUserScripts[sequenceInc] = output.niawgLanguageScript;
			waiter.systemAbortCheck( input->comm );
		}
		input->comm->sendStatus("Constant Waveform Preparation Completed...\r\n");
		input->niawg->finalizeScript( repetitions, userScriptNameString, workingUserScripts, userScriptSubmit);
		if (input->debugInfo.outputNiawgMachineScript)
		{
			input->comm->sendDebug( boost::replace_all_copy( "NIAWG Machine Script:\n" 
															 + std::string(userScriptSubmit.begin(), userScriptSubmit.end())
															 + "end Script\n\n", "\n", "\r\n" ) );
		}
		ExperimentLogger::generateNiawgLog( input, scriptFiles, intensityScriptFiles, repetitions );
		// If there are any varible files, enter big variable loop.
		if (variables.size() > 0)
		{
			input->comm->sendStatus("Begining Variation Loop.\r\n");
			waiter.initialize();
			/// variation loop
			for (std::size_t variation = 0; variation < varValues[0].size(); variation++)
			{
				std::string warnings;
				input->niawg->handleVariations( output, variables, varValues, variation, variedMixedSize, warnings, 
											   input->debugInfo);
				/*
				if (input->debugInfo.message != "") { input->comm->sendDebug( input->debugInfo.message ); }
				if (warnings != "") { input->comm->sendError(warnings); }

				std::string message = ("Outputting Series #" + str( variation ) + ". Waiting to Initialize Script from"
									   " series #" + str( variation + 1 ) + ".\r\n");
				for (int varInc = 0; varInc < variables.size(); varInc++)
				{
					if (variation != 0)
					{
						message += variables[varInc].name + " = " + str(varValues[varInc][variation - 1]) + "; ";
					}
				}
				input->comm->sendColorBox( Niawg, 'G');
				input->comm->sendStatus( message );
				*/

				// If running the default script, stop the default script so that a moment later, when the program 
				// checks if the output is done, the output will be done.
				if (  (input->niawg->getCurrentScript() == "Default" + AXES_NAMES[Horizontal]	+ "ConfigScript" 
					|| input->niawg->getCurrentScript() == "Default" + AXES_NAMES[Vertical]		+ "ConfigScript") 
					&& !input->dontActuallyGenerate )
				{
					input->niawg->turnOff();
				}
				// wait until the niawg has finished outputting previous variation.
				waiter.wait( input->comm );
				// Tell the master to begin the experiment
				std::string varMessage = ("Variation #" + str( variation + 1 ) + "/" + str( varValues[0].size() )
										   + ". Variable values are:\r\n");
				for (int varNumInc = 0; varNumInc < variables.size(); varNumInc++)
				{
					varMessage += "\t" + variables[varNumInc].name + " = " + str(varValues[varNumInc][variation]) 
						+ "\r\n";
				}
				input->comm->sendStatus(varMessage);
				
				if (input->dontActuallyGenerate) { continue; }
				// Restart Waveform
				input->niawg->turnOff();
				input->niawg->programVariations(variation, variedMixedSize, output);
				input->niawg->writeScript(userScriptSubmit);
				input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
				// initiate generation before telling the master. this is because scripts are supposed to be designed to sit on an 
				// initial waveform until the master sends it a trigger.
				input->niawg->turnOn();
				
				waiter.startWaitThread( input );					
				for (int waveInc = 2; waveInc < output.waveCount; waveInc++)
				{
					output.waves[waveInc].waveVals.clear();
					output.waves[waveInc].waveVals.shrink_to_fit();
				}
				variedMixedSize.clear();
			}
		}
		else
		{
			///					If no Variables					///
			input->comm->sendStatus("NO Variable looping this run.\r\n");

			std::string warnings;
			input->niawg->checkThatWaveformsAreSensible( warnings, output );
			if (warnings != "") { input->comm->sendError(warnings); }

			if (!input->dontActuallyGenerate)
			{
				input->niawg->turnOff();
				// Should be just ready to go
				input->niawg->writeScript(userScriptSubmit);
				input->niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript");
				input->niawg->turnOn();
				waiter.startWaitThread( input );
			}
		}

		if ( repetitions == 0 || (variables.size() == 0 && NIAWG_SAFEMODE))
		{
			input->comm->sendColorBox( Niawg, 'G' );
			input->comm->sendStatus( "Scripts Loaded into NIAWG. This waveform sequence will run until aborted by the user.\r\n" );
		}
		else
		{
			std::string message;
			if (varValues.size() > 0)
			{
				message = "Outputting LAST niawg configuration #" + str( varValues[0].size() ) + ".\r\n";
			}
			else
			{
				message = "Outputting the only niawg configuration.\r\n";
			}
			for (int varInc = 0; varInc < variables.size(); varInc++)
			{
				message += variables[varInc].name + " = " + str( varValues[varInc].back() ) + "; ";
			}
			input->comm->sendColorBox( Niawg, 'G' );
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
		}	

		waiter.wait( input->comm );

		// Clear waveforms off of NIAWG (not working??? memory appears to still run out...)
		
		for (int waveformInc = 2; waveformInc < output.waveCount; waveformInc++)
		{
			std::string waveformToDelete = "Waveform" + str( waveformInc );
			input->niawg->deleteWaveform( cstr(waveformToDelete) );
		}
		if (!input->dontActuallyGenerate)
		{
			input->niawg->deleteScript("experimentScript");
		}
		for (auto& wave : output.waves)
		{
			wave.waveVals.clear();
			wave.waveVals.shrink_to_fit();
		}
	}
	catch (Error& exception)
	{
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
		for (auto& wave : output.waves)
		{
			wave.waveVals.clear();
			wave.waveVals.shrink_to_fit();
		}
		if (exception.whatBare() != "Aborted!\r\n")
		{
			input->comm->sendFatalError("ERROR: " + exception.whatStr());
		}
		else
		{
			input->comm->sendStatus(exception.what());
		}
	}
	delete input;
	return 0;
};

