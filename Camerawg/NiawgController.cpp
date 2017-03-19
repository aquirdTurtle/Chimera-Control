#include "stdafx.h"
#include "NiawgController.h"
#include "externals.h"
#include "constants.h"
#include "rmWhite.h"
#include <algorithm>
#include "myMath.h"
#include "myAgilent.h"
#include "postMyString.h"
#include "myErrorHandler.h"
#include "miscellaneousCommonFunctions.h"
#include "ScriptStream.h"

bool NiawgController::isRunning()
{
	return runningState;
}


void NiawgController::setRunningState( bool newRunningState )
{
	this->runningState = newRunningState;
}


void NiawgController::configureOutputMode()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker(niFgen_ConfigureOutputMode(sessionHandle, OUTPUT_MODE));
	}
}


void NiawgController::setDefaultWaveforms( MainWindow* mainWin, bool isFirstLoad )
{
	if (!isFirstLoad)
	{
		delete[] defaultScripts[Vertical];
		delete[] defaultScripts[Horizontal];
		delete[] defaultMixedWaveforms[Vertical];
		delete[] defaultMixedWaveforms[Horizontal];
	}
	// counts the number of predefined waveforms that have been handled or defined.
	int predWaveformCount = 0;
	// Socket object for communicating with the other computer.
	CSocket* socket = mainWin->getSocket();
	// A vector which stores the number of values that a given variable will take through an experiment.
	std::vector<std::size_t> length;
	// An array of vectors holding strings. Each vector within the array is for a specific type of waveform output (e.g. gen 1, const). The each string within a
	// vector contains unique descriptions of waveforms. I check this list to see if a waveform has been written already.
	library waveLibrary;
	// first level is for different configurations, second is for horizontal or vertical file within a configuration.
	niawgPair<niawgPair<std::vector<std::fstream>>> configFiles;
	//std::vector<std::fstream> hConfigHFile, hConfigVFile, vConfigHFile, vConfigVFile;
	configFiles[Horizontal][Horizontal].push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript" ) );
	configFiles[Horizontal][Vertical].push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript" ) );
	configFiles[Vertical][Horizontal].push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript" ) );
	configFiles[Vertical][Vertical].push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript" ) );
	// check errors
	for (auto configAxis : AXES)
	{
		for (auto fileAxis : AXES)
		{
			if (!configFiles[configAxis][fileAxis].back().is_open())
			{
				thrower( "FATAL ERROR: Couldn't open " + AXES_NAMES[configAxis] + " configuration " + AXES_NAMES[fileAxis] + " default file." );
			}
		}
	}
	outputInfo output;
	output.waveCount = 0;
	output.predefinedWaveCount = 0;
	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Load Default Waveforms
	///
	// Contains a bool that tells whether the user script has been written or not. This is used to tell whether I need to delete it or not.
	bool userScriptIsWritten = false;
	int defPredWaveformCount = 0;
	// analyze the input files and create the xy-script. Originally, I thought I'd write the script in two parts, the x and y parts, but it turns out not to 
	// work like I thought it did. If  I'd known this from the start, I probably wouldn't have created this subroutine, except perhaps for the fact that it get 
	// called recursively by predefined scripts in the instructions file.
	/// Create Horizontal Configuration
	debugInfo dummyOptions;
	dummyOptions.outputAgilentScript = false;
	std::vector<variable> noSingletons;
	std::string warnings, debugMessages;
	// doesn't get used for anything...
	profileSettings profile;
	try
	{
		for (auto configAxis : AXES)
		{
			output.niawgLanguageScript = "script Default" + AXES_NAMES[configAxis] + "ConfigScript\n";
			niawgPair<ScriptStream> scripts;
			scripts[Horizontal] << configFiles[configAxis][Horizontal].back().rdbuf();
			scripts[Vertical] << configFiles[configAxis][Vertical].back().rdbuf();
			profile.orientation = ORIENTATION_AXES[configAxis];
			mainWin->setOrientation( ORIENTATION_AXES[configAxis] );
			analyzeNiawgScripts( scripts, output, waveLibrary, profile, noSingletons, dummyOptions, warnings, debugMessages );
			// the script file must end with "end script".
			output.niawgLanguageScript += "end Script";
			// Convert script string to ViConstString. +1 for a null character on the end.
			defaultScripts[configAxis] = new ViChar[output.niawgLanguageScript.length() + 1];
			sprintf_s( defaultScripts[configAxis], output.niawgLanguageScript.length() + 1, "%s", output.niawgLanguageScript.c_str() );
			strcpy_s( defaultScripts[configAxis], output.niawgLanguageScript.length() + 1, output.niawgLanguageScript.c_str() );
		}
	}
	catch (myException& except)
	{
		thrower( "FATAL ERROR: Analysis of Default Waveforms and Default Script Has Failed: " + except.whatStr() );
	}
	// check for warnings.
	if (warnings != "")
	{
		errBox( "Warnings detected during initial default waveform script analysis: " + warnings );
	}
	if (debugMessages != "")
	{
		errBox( "Debug messages detected during initial default waveform script analysis: " + debugMessages );
	}
	// but the default starts in the horizontal configuration, so switch back and start in this config.
	mainWin->setOrientation( this->defaultOrientation );
}


signed short NiawgController::isDone()
{
	ViBoolean isDone = 0;
	if (TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker(niFgen_IsDone(sessionHandle, &isDone));
	}
	return isDone;
}


void NiawgController::initialize()
{
	/// Initialize the waveform generator. Currently this is set to reset the initialization parameters from the last run.
	// initializes the session handle.
	init(NI_5451_LOCATION, VI_TRUE, VI_TRUE);
	// tells the niaw where I'm outputting.
	configureChannels(outputChannels);
	// Set output mode of the device to scripting mode (defined in constants.h)
	configureOutputMode();
	// configure marker event. This is set to output on PFI1, a port on the front of the card.
	configureMarker("Marker0", "PFI1");
	// enable flatness correction. This allows there to be a bit less frequency dependence on the power outputted by the waveform generator.
	setViBooleanAttribute(NIFGEN_ATTR_FLATNESS_CORRECTION_ENABLED, VI_TRUE);
	// configure the trigger. Trigger mode doesn't need to be set because I'm using scripting mode.
	configureDigtalEdgeScriptTrigger();
	// Configure the gain of the signal amplification.
	configureGain(GAIN);
	// Configure Sample Rate. The maximum value of this is 400 mega-samples per second, but it is quite buggy, so we've been using 350 MS/s
	configureSampleRate(SAMPLE_RATE);
	// Configure the analogue filter. This is important for high frequency signals, as it smooths out the discrete steps that the waveform generator outputs.
	// This is a low-pass filter.
	enableAnalogFilter(NIAWG_FILTER_FREQENCY);
	/// Configure Clock input
	// uncomment for high resolution mode
	configureClockMode(NIFGEN_VAL_HIGH_RESOLUTION);
	// uncomment for default onboard clock
	//myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleClockSource(eSessionHandle, "OnboardClock")
	// Unccoment for using an external clock as a "sample clock"
	// myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleClockSource(eSessionHandle, "ClkIn")
	// Uncomment for using an external clock as a reference clock
	// myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureReferenceClock(eSessionHandle, "ClkIn", 10000000), HORIZONTAL_ORIENTATION, theMainApplicationWindow.getComm())
}


void NiawgController::restartDefault()
{
	try
	{
		configureOutputEnabled( VI_FALSE );
		abortGeneration();
		clearMemory();
		// do I really want this here? prob not.
		myAgilent::agilentDefault();

		for (auto axis : AXES)
		{
			if (defaultOrientation == ORIENTATION_AXES[axis])
			{
				createWaveform( defaultMixedSizes[axis], defaultMixedWaveforms[axis] );
				allocateWaveform( defaultWaveformNames[axis].c_str(), defaultMixedSizes[axis] / 2 );
				writeWaveform( defaultWaveformNames[axis].c_str(), defaultMixedSizes[axis], defaultMixedWaveforms[axis] );
				writeScript( defaultScripts[axis] );
				eCurrentScript = "Default" + AXES_NAMES[axis] + "ConfigScript";
			}
		}
		configureOutputEnabled( VI_TRUE );
		setAttributeViString( NIFGEN_ATTR_SCRIPT_TO_GENERATE, ViString(eCurrentScript.c_str()) );
		initiateGeneration();
	}
	catch (myException& except)
	{
		thrower( "WARNING! The NIAWG encountered an error and was not able to restart smoothly. It is (probably) not outputting anything. You may "
				 "consider restarting the code. Inside the restart area, NIAWG function returned " + except.whatStr() );
	}
}


void NiawgController::setDefaultOrientation( std::string orientation )
{
	this->defaultOrientation = orientation;
}


/**
  * The purpose of this function is to systematically read the input instructions files, create and read waveforms associated with
  * them, write the script as it goes, and eventually combine the x and y-waveforms into their final form for being sent to the waveform 
  * generator.
  */
void NiawgController::analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, outputInfo& output, library &waveLibrary, 
										   profileSettings profile, std::vector<variable> singletons, debugInfo options, 
										   std::string& warnings, std::string& debugMsg )
{
	/// Preparation
	std::stringstream tempStream;
	for (auto& axis : AXES)
	{
		tempStream = std::stringstream();
		tempStream << scripts[axis].rdbuf();
		currentScripts[axis] = tempStream.str();
		scripts[axis].clear();
		scripts[axis].seekg( 0, std::ios::beg );
	}
	// Some declarations.
	niawgPair<std::string> command;
	// This parameter is used to hold the waveform name (e.g. waveform1) that actually appears in the final script. I included a couple 
	// extra places for good measure for large numbers of waveforms.
	ViChar tempWaveformName[11];
	ViChar prevTempWaveName[11];
	// these hold actual waveform data.
	niawgPair<ViReal64*> waveVoltages = { NULL, NULL }, prevVoltages = { NULL, NULL };
	ViReal64* mixedWaveVoltages = NULL, *prevMixedVoltages = NULL;
	// open up the files and check what I have stored.
	openWaveformFiles( waveLibrary );
	// get the first input
	for (auto axis : AXES)
	{
		// get the instructions line.
		command[axis] = scripts[axis].getline();
		// handle trailing newline characters
		if (command[axis].length() != 0)
		{
			if (command[axis][command[axis].length() - 1] == '\r')
			{
				command[axis].erase( command[axis].length() - 1 );
			}
		}
	}

	/// Analyze!. 
	/// 
	while (!(scripts[Vertical].peek() == EOF) && !(scripts[Horizontal].peek() == EOF))
	{
		// these are the variables that hold current basic information about the waveform being generated. They are used to create the raw 
		// data points. They are temporary, and pass their information into the larger array of waveInfo structures.
		niawgPair<waveInfo> waveParams;
		sprintf_s( tempWaveformName, 11, "Waveform%i", output.waveCount );
		sprintf_s( prevTempWaveName, 11, "Waveform%i", output.waveCount - 1 );
		// Send the command to different places depending on what type of command it is.
		if (isLogicCommand( command[Horizontal] ) && isLogicCommand( command[Vertical] ))
		{
			logic( scripts, command, output.niawgLanguageScript );
		}
		else if (isSpecialCommand( command[Horizontal] ) && isSpecialCommand( command[Vertical] ))
		{
			special( scripts, output, command, waveLibrary, profile, singletons, options, warnings, debugMsg );
		}
		// the most complex case.
		else if (isGenCommand( command[Horizontal] ) && isGenCommand( command[Vertical] ))
		{
			// not sure why I have this limitation built in.
			if ((output.waveCount == 1 && output.isDefault && profile.orientation == ORIENTATION_AXES[Vertical])
				 || (output.waveCount == 2 && output.isDefault && profile.orientation == ORIENTATION_AXES[Horizontal]))
			{
				thrower( "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must be a "
						 "single waveform, not a sequence.\r\n" );
			}
			// 
			for (auto axis : AXES)
			{
				// The default value is false. If the waveform isn't varied, it gets changed to true.
				waveParams[axis].varies = false;
				// Get a number corresponding directly to the given input type.
				getInputType( command[axis], waveParams[axis] );
				// infer the number of signals from the type assigned.
				waveParams[axis].signalNum = waveParams[axis].initType % MAX_NIAWG_SIGNALS;
				// Gather the parameters the user inputted for the waveforms and sort them into the appropriate data structures.
				getWaveData( scripts[axis], waveParams[axis], singletons );

				if (waveParams[axis].delim != "#")
				{
					thrower( "ERROR: The delimeter is missing in the " + AXES_NAMES[axis] + " script file for waveform #"
							 + str( output.waveCount - 1 ) + "The value placed in the delimeter location was " + waveParams[axis].delim
							 + " while it should have been '#'. This indicates that either the code is not interpreting the user input "
							 "correctly or that the user has inputted too many parameters for this type of waveform." );
				}
				if (waveParams[axis].sampleNum % 4 != 0)
				{
					thrower( "ERROR: Invalid sample number in " + AXES_NAMES[axis] + " waveform #" + str( output.waveCount - 1 )
							 + ". The time that resulted in this was " + str( waveParams[axis].time ) + " which gave a sample number of "
							 + str( waveParams[axis].sampleNum ) + "\r\n" );
				}
			}
			
			// make sure that the waveform management options match (they must, or else the times might not match )
			if (waveParams[Horizontal].phaseOption != waveParams[Vertical].phaseOption)
			{
				thrower( "ERROR: the vertical and horizontal waveforms must have the same time management option. They appear to be "
						 "mismatched for waveform #" + str( output.waveCount - 1 ) + "!" );
			}
			// set this as a more transparent way of checking the current time management option.
			int currentPhaseOption = waveParams[Horizontal].phaseOption;
			
			// make sure the times match to nanosecond precision.
			if (!(fabs( waveParams[Horizontal].time - waveParams[Vertical].time ) < 1e-6))
			{
				thrower( "ERROR: the horizontal and vertical waveforms must have the same time value. They appear to be mismatched for waveform #"
						 + str( output.waveCount - 1 ) + "!" );
			}

			///	Handle -1 Phase (Use the phase that the previous waveform ended with) /////////////////////////////////////////////////////
			for (auto axis : AXES)
			{
				// If the user used a '-1' for the initial phase, this means the user wants to copy the ending phase of the previous waveform.
				int count = 0;
				// loop through all signals in a the current waveform for a given axis.
				for (auto signal : waveParams[axis].signals)
				{
					if (signal.initPhase == -1)
					{
						// if you are trying to copy the phase from a waveform that is being varied, this can only be accomplished if this 
						// waveform is also varied. mark this waveform for varying and break.
						if (output.chan[axis].waveInfo[output.waveCount - 1].varies == true)
						{
							waveParams[Horizontal].varies = waveParams[Vertical].varies = true;
							waveParams[axis].varNum++;
							// the ' sign is reserved. It's just a place-holder here to make sure the number of varied waveforms gets understood properly.
							waveParams[axis].varNames.push_back( "\'" );
							waveParams[axis].varTypes.push_back( -1 );
							break;
						}

						if (count + 1 > output.chan[axis].waveInfo[output.waveCount - 1].signalNum)
						{
							thrower( "ERROR: You are trying to copy the phase of signal " + str( count + 1 ) + "  of " + AXES_NAMES[axis]
									 + " waveform #" + str( output.waveCount - 1 ) + ", but the previous waveform only had "
									 + str( output.chan[axis].waveInfo[output.waveCount - 1].signalNum ) + " signals!\n" );
						}

						signal.initPhase = output.chan[axis].waveInfo[output.waveCount - 1].signals[count].finPhase;
					}
					count++;
				}
			}

			///	Handle -1 Time Management (Bring the frequencies in the previous waveforms to 0 phase. ///////////////////////////////////
			// Check if user used -1 for the time management of this waveform.
			if (currentPhaseOption == -1)
			{
				std::vector<double> initPhasesForTimeManage;
				// Make sure that both waveforms have the right number of signals.
				for (auto axis : AXES)
				{
					if (waveParams[axis].signalNum != output.chan[axis].waveInfo[output.waveCount - 1].signalNum)
					{
						thrower( "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
								 str( output.waveCount - 2 ) + " with waveform number" + str( output.waveCount - 1 ) + ", but the " +
								 AXES_NAMES[axis] + " component of the former has " + str( output.chan[axis].waveInfo[output.waveCount - 1].signalNum )
								 + " signals and the latter has " + str( waveParams[axis].signalNum ) + " signals. In order for a waveform "
								 "to correct the time of another waveform, the two must have the same number of signals." );
					}
					for (auto signal : range( output.chan[axis].waveInfo[output.waveCount - 1].signalNum ))
					{
						// Check...
						if (output.chan[axis].waveInfo[output.waveCount - 1].signals[signal].freqFin 
							 != waveParams[axis].signals[signal].freqInit)
						{
							// report error
							thrower( "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
									 + str( output.waveCount - 2 ) + " with waveform number" + str( output.waveCount - 1 ) + ", but signal "
									 + str( signal ) + " in the vertical component of the former has final frequency "
									 + str( output.chan[axis].waveInfo[output.waveCount - 1].signals[signal].freqFin ) + " and signal " 
									 + str( signal ) + " in the " + AXES_NAMES[axis] + " component of the latter has initial frequency of"
									 + str( waveParams[axis].signals[signal].freqInit ) + " signals. In order for a waveform to correct the "
									 "time of another waveform, these frequencies should match." );
						}
						initPhasesForTimeManage.push_back( waveParams[axis].signals[signal].initPhase );
					}
				}			
				if (output.chan[Vertical].waveInfo[output.waveCount - 1].varies == true 
					 || output.chan[Horizontal].waveInfo[output.waveCount - 1].varies == true)
				{
					for (auto axis : AXES)
					{
						waveParams[axis].varies = true;
						waveParams[axis].varNum++;
						// the ' single quotation mark is reserved. Don't use it in a script file. It's just a place-holder here to make sure 
						// the number of varied waveforms gets understood properly.
						waveParams[axis].varNames.push_back( "\'" );
						waveParams[axis].varTypes.push_back( -2 );
					}
				}
				else
				{
					double errVal = myMath::calculateCorrectionTime( waveParams[Vertical], waveParams[Horizontal], initPhasesForTimeManage, 
																	 "after" );
					if (errVal == -1)
					{
						thrower( "ERROR: Correction waveform was not able to match phases." );
					}
					else if (errVal != 0)
					{
						warnings += "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
							+ str( errVal ) + "radians.\r\n";
					}
				}
			}

			/// Waveform Creation /////////////////////////////////////////////////////////////////////////////////////////////////////////
			// only create waveform data if neither waveform is being varried and if the time management option is either 0 or -1. The time
			//  management option has already been checked to be the same for Vertical and Horizontal waveforms.
			for (auto axis : AXES)
			{
				if (waveParams[Vertical].varNum == 0 && waveParams[Horizontal].varNum == 0 && currentPhaseOption < 1)
				{
					// Initialize the giant waveform arrays.
					waveVoltages[axis] = new ViReal64[waveParams[axis].sampleNum];
					// either calculate or read waveform data into the above arrays. 
					generateWaveform( waveVoltages[axis], waveParams[axis], waveLibrary, options, debugMsg );
				}
				else if (waveParams[axis].varNum > 0)
				{
					// Mark this waveform as being varied. 
					waveParams[Horizontal].varies = waveParams[Vertical].varies = true;
				}
			}

			/// Handle +1 Time Management ////////////////////////////////////////////////////////////////////////////////////////////////
			// if not the first waveform
			if (output.waveCount != 0)
			{
				// check if user used +1 for the time of the previous waveform.
				for (auto axis : AXES)
				{
					if (output.chan[axis].waveInfo[output.waveCount - 1].phaseOption == 1)
					{
						// Make sure the waveforms are compatible.
						std::vector<double> initialTimeManagementPhases;
						if (waveParams[axis].signalNum != output.chan[axis].waveInfo[output.waveCount - 1].signalNum)
						{
							thrower( "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
									 str( output.waveCount - 1 ) + " with waveform number" + str( output.waveCount - 2 ) + ", but the vertical "
									 "component of the former has " + str( waveParams[axis].signalNum ) + " signals and the latter has " +
									 str( output.chan[axis].waveInfo[output.waveCount - 1].signalNum ) + " signals. In order for a waveform to "
									 "correct the time of another waveform, the two must have the same number of signals." );
						}
						// check to make sure that the X frequencies match
						for (int signal = 0; signal < output.chan[axis].waveInfo[output.waveCount - 1].signalNum; signal++)
						{
							if (output.chan[axis].waveInfo[output.waveCount - 1].signals[signal].freqInit 
								 != waveParams[axis].signals[signal].freqInit)
							{
								thrower( "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
										 + str( output.waveCount - 1 ) + " with waveform number" + str( output.waveCount - 2 ) +
										 ", but signal " + str( signal ) + " in the vertical component of the former has final frequency "
										 + str( waveParams[axis].signals[signal].freqInit ) + " and signal " + str( signal ) + " in the "
										 "vertical component of the latter has initial frequency of "
										 + str( output.chan[axis].waveInfo[output.waveCount - 1].signals[signal].freqFin ) + " signals. In order "
										 "for a waveform to correct the time of another waveform, these frequencies should match." );
							}
							initialTimeManagementPhases.push_back( waveParams[axis].signals[signal].finPhase 
															   + output.chan[axis].waveInfo[output.waveCount - 1].signals[signal].initPhase );
						}
						// check to make sure that the frequencies match
						for (int signal = 0; signal < output.chan[ALT_AXES[axis]].waveInfo[output.waveCount - 1].signalNum; signal++)
						{
							// check to make sure that the frequencies match
							if (output.chan[ALT_AXES[axis]].waveInfo[output.waveCount - 1].signals[signal].freqInit 
								 != waveParams[ALT_AXES[axis]].signals[signal].freqInit)
							{
								thrower( "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
										 + str( output.waveCount - 1 ) + " with waveform number" + str( output.waveCount - 2 ) + ", but signal "
										 + str( signal ) + " in the " + AXES_NAMES[ALT_AXES[axis]] + " component of the former has final frequency "
										 + str( waveParams[ALT_AXES[axis]].signals[signal].freqInit ) + " and signal " + str( signal ) + " in the "
										 + AXES_NAMES[ALT_AXES[axis]] + " component of the latter has initial frequency of "
										 + str( output.chan[ALT_AXES[axis]].waveInfo[output.waveCount - 1].signals[signal].freqFin ) +
										 " signals. In order for a waveform to correct the time of another waveform, these frequencies should match." );
							}
							initialTimeManagementPhases.push_back( waveParams[ALT_AXES[axis]].signals[signal].finPhase
															   + output.chan[ALT_AXES[axis]].waveInfo[output.waveCount - 1].signals[signal].initPhase );
						}

						if (waveParams[axis].varies == true)
						{
							output.chan[Vertical].waveInfo[output.waveCount - 1].varies 
								= output.chan[Horizontal].waveInfo[output.waveCount - 1].varies = true;
							output.chan[axis].waveInfo[output.waveCount - 1].varNum++;
							// the ' sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms gets
							// understood properly.
							output.chan[axis].waveInfo[output.waveCount - 1].varNames.push_back( "\'" );
							output.chan[axis].waveInfo[output.waveCount - 1].varTypes.push_back( -3 );
						}
						else
						{
							// calculate a time for the previous waveform.
							double errVal = myMath::calculateCorrectionTime( output.chan[axis].waveInfo[output.waveCount - 1],
																			 output.chan[ALT_AXES[axis]].waveInfo[output.waveCount - 1],
																			 initialTimeManagementPhases, "before" );
							if (errVal == -1)
							{
								thrower( "ERROR: Correction waveform was not able to match phases." );
							}
							else if (errVal != 0)
							{
								warnings += "WARNING: Correction waveform was not able to match phases very well. The total phase"
											" mismatch was " + str( errVal ) + "radians.\r\n";
							}

							// calculate the previous waveform.
							// Initialize the giant waveform arrays.
							
							prevVoltages[axis] = new ViReal64[output.chan[axis].waveInfo[output.waveCount - 1].sampleNum];
							// either calculate or read waveform data into the above arrays. 
							generateWaveform( prevVoltages[axis], output.chan[axis].waveInfo[output.waveCount - 1], waveLibrary, options, 
										 debugMsg );
							// modify the phases of the current waveform such that they reach the 0 phase after the waveform.
							for (int signal = 0; signal < waveParams[axis].signalNum; signal++)
							{
								waveParams[axis].signals[signal].initPhase = 2 * PI - waveParams[axis].signals[signal].finPhase;
							}
						}
					}
				}


				// Now That I've calculated the waveforms, I need to actually write them. I should always enter all three of these if 
				// statements if any.
				if (output.chan[Horizontal].waveInfo[output.waveCount - 1].phaseOption == 1 
					 && output.chan[Vertical].waveInfo[output.waveCount - 1].phaseOption == 1
					 && waveParams[Horizontal].varies == false && waveParams[Vertical].varies == false)
				{
					// Check for bad input
					if (output.chan[Vertical].waveInfo[output.waveCount - 1].sampleNum 
						 != output.chan[Horizontal].waveInfo[output.waveCount - 1].sampleNum)
					{
						thrower( "ERROR: the x and y waveforms must have the same time values option. They appear to be mismatched for waveform #"
								 + str( output.waveCount - 1 ) + "!" );
					}
					// Create the mixed waveform holder
					long int mixedSize = 2 * output.chan[Vertical].waveInfo[output.waveCount - 1].sampleNum;
					prevMixedVoltages = new ViReal64[mixedSize];
					// Mix the waveforms
					mixWaveforms( prevVoltages[Vertical], prevVoltages[Horizontal], prevMixedVoltages, 
								  output.chan[Vertical].waveInfo[output.waveCount - 1].sampleNum );
					// create waveform (necessary?)
					// these three functions are capable of throwing myException. analyzeNiawgScripts should always be in a try/catch.
					createWaveform( mixedSize, prevMixedVoltages );
					// ...
					allocateWaveform( prevTempWaveName, mixedSize / 2 );
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					writeWaveform( prevTempWaveName, mixedSize, prevMixedVoltages );
					// avoid memory leaks.
					delete[] prevVoltages[Vertical];
					delete[] prevVoltages[Horizontal];
					delete[] prevMixedVoltages;
				}

				// only create waveform data if neither waveform is being varried, and for this second time, if the previous wave was 
				// correcting this waveform such that the waveform needs to be recalculated.
				for (auto axis : AXES)
				{
					if (waveParams[Vertical].varNum == 0 && waveParams[Horizontal].varNum == 0
						 && waveParams[axis].phaseOption < 1
						 && output.chan[axis].waveInfo[output.waveCount - 1].phaseOption == 1)
					{
						// Initialize the giant waveform arrays.
						waveVoltages[axis] = new ViReal64[waveParams[axis].sampleNum];
						// either calculate or read waveform data into the above arrays. 
						generateWaveform( waveVoltages[axis], waveParams[axis], waveLibrary, options, debugMsg );
					}
					else if (waveParams[axis].varNum > 0)
					{
						// Mark this waveform as being varied.
						waveParams[axis].varies = true;
						waveParams[ALT_AXES[axis]].varies = true;
					}
				}
			}

			for (auto axis : AXES)
			{
				if (waveParams[axis].varNum >= 0)
				{
					output.chan[axis].waveInfo.push_back( waveParams[axis] );
				}
			}

			if (waveParams[Horizontal].varNum == 0 && waveParams[Vertical].varNum == 0 && waveParams[Vertical].phaseOption < 1)
			{
				// Check for bad input
				if (waveParams[Vertical].sampleNum != waveParams[Horizontal].sampleNum)
				{
					thrower( "ERROR: the vertical and horizontal waveforms must have the same time values option. They appear to be "
							 "mismatched for waveform #" + str( output.waveCount - 1 ) + "!" );
				}
				// Create the mixed waveform holder
				long int mixedSize = 2 * waveParams[Vertical].sampleNum;
				mixedWaveVoltages = new ViReal64[mixedSize];
				// Mix the waveforms
				mixWaveforms( waveVoltages[Vertical], waveVoltages[Horizontal], mixedWaveVoltages, waveParams[Vertical].sampleNum );
				// these three functions are capable of throwing myException. analyzeNiawgScripts should always be in a try/catch.
				// create waveform (necessary?)
				createWaveform( mixedSize, mixedWaveVoltages );
				// allocate waveform into the device memory
				allocateWaveform( tempWaveformName, mixedSize / 2 );
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
				writeWaveform( tempWaveformName, mixedSize, mixedWaveVoltages );
				// avoid memory leaks, but only if not default...
				if (output.isDefault)
				{
					for (auto axis : AXES)
					{
						if (profile.orientation == ORIENTATION_AXES[axis])
						{
							defaultMixedWaveforms[axis] = mixedWaveVoltages;
							defaultMixedSizes[axis] = mixedSize;
							defaultWaveformNames[axis] = tempWaveformName;
						}
					}
				}
				else
				{
					delete[] mixedWaveVoltages;
				}
				delete[] waveVoltages[Vertical];
				delete[] waveVoltages[Horizontal];
			}
			// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
			//  have to rewrite the script to insert the new waveform name into it.
			std::string tempWfmNameString( tempWaveformName );
			output.niawgLanguageScript += "generate " + tempWfmNameString + "\n";
			// increment waveform count.
			output.waveCount++;
		}
		else
		{
			// Catch bad input.
			thrower( "ERROR: Input types from the two files do not match or are unrecofgnized!\nBoth must be logic commands, both must be "
					 "generate commands, or both must be special commands. See documentation on the correct format for these commands.\n\n"
					 "The two inputted types are: " + command[Vertical] + " and " + command[Horizontal]+ " for waveform #" 
					 + str( output.waveCount - 1) + "!" );
		}
		// get next input.
		for (auto axis : AXES)
		{
			// get the instructions line.
			command[axis] = scripts[axis].getline();
			// handle trailing newline characters
			if (command[axis].length() != 0)
			{
				if (command[axis][command[axis].length() - 1] == '\r')
				{
					command[axis].erase( command[axis].length() - 1 );
				}
			}
		}
		// end while !eof() loop
	}
}

/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NiawgController::handleVariations( outputInfo& output, std::vector<variable>& variables,  
										std::vector<std::vector<double>> variableValues, size_t& variation, 
										std::vector<long> mixedWaveSizes, std::vector<ViReal64 *> mixedWaveforms, std::string& warnings, 
										std::string& debugMsgs, library& waveLibrary, debugInfo debugOptions )
{				
	niawgPair<std::vector<ViReal64 *>> variedWaves;
	niawgPair<int> writeCounts = { 0, 0 };
	int mixedCount = 0;
	// reset some variables which might have odd values if this is reached from a break from within the variable waveform calculations.
	bool correctionFlag = false;
	bool repeatFlag = false;
	bool rewriteFlag = false;
	// I think waveInc = 0 & 1 are the default.. should I be handling that at all??? shouldn't make a difference I don't think. 
	for (int waveInc = 0; waveInc < output.waveCount; waveInc++)
	{
	BeginningOfWaveformLoop:
		// currently both axes always vary together and get rewritten together. This could be modified to be smarter for a slightly more 
		// efficient programming sequence between variations. //
		if (output.chan[Vertical].waveInfo[waveInc].varies && output.chan[Horizontal].waveInfo[waveInc].varies)
		{
			// For all Variables...
			for (std::size_t variableInc = 0; variableInc < variables.size() + 1; variableInc++)
			{
				std::string currentVar;
				if (variableInc < variables.size())
				{
					currentVar = variables[variableInc].name;
				}
				else
				{
					// This is the reserved character for a variable time resulting from waveform correction.
					// This value is always set once to check for this because variableInc loops to and including variableNames.size().
					currentVar = '\'';
				}
				std::string warnings;
				for (auto axis : AXES)
				{
					/// Loop for varibles in given Waveform
					for (int variableNumber = 0; variableNumber < output.chan[axis].waveInfo[waveInc].varNum; variableNumber++)
					{
						// if parameter's variable matches current variable...
						if (output.chan[axis].waveInfo[waveInc].varNames[variableNumber] == currentVar)
						{
							double variableValue;
							if (variableInc < variables.size())
							{
								variableValue = variableValues[variableInc][variation];
							}
							else
							{
								// dummy value. Not used in this case.
								variableValue = 0;
							}
							/// For Correction Waveforms...
							// Structure is weird for waveforms correcting the next waveform.
							if (output.chan[axis].waveInfo[waveInc].varTypes[variableNumber] == -3)
							{
								// set it to -4 so that it is correctly recognized the next time through.
								output.chan[axis].waveInfo[waveInc].varTypes[variableNumber] = -4;
								// The corresponding x variable might be in a different location; I need to find that location
								for (auto& varType : output.chan[ALT_AXES[axis]].waveInfo[waveInc].varTypes)
								{
									if (varType == -3)
									{
										varType = -4;
									}
								}
								/// Set all the parameters as if this had exited normally. There's got to be a smarter way of doing this.
								// increment the waveform being corrected.
								waveInc++;
								// increment the number of vertical, horizontal, and mixed waveforms that have been "written"
								writeCounts[Vertical]++;
								writeCounts[Horizontal]++;
								mixedCount++;
								// add extra spots for waveforms here.
								variedWaves[Horizontal].resize( variedWaves[Horizontal].size() + 1 );
								variedWaves[Vertical].resize( variedWaves[Vertical].size() + 1 );
								mixedWaveSizes.resize( mixedWaveSizes.size() + 1 );
								mixedWaveforms.resize( mixedWaveforms.size() + 1 );
								// this flag lets the code know that the next waveform is being corrected.
								correctionFlag = true;
								// Return to the beginning of the for loop without actually writing.
								goto BeginningOfWaveformLoop;
							}
							/// Not correction waveform...
							// change parameters, depending on the case. The varTypes and variable Value sets were set previously.
							varyParam( output.chan[axis].waveInfo, output.chan[ALT_AXES[axis]].waveInfo, waveInc, 
									   output.chan[axis].waveInfo[waveInc].varTypes[variableNumber], variableValue, warnings );
						} // end "if variables match"
					} // end "parameters loop"
				} // end axis loop
			} // end variables loop
			// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform". 
			// You only get here if flagged for it during read.
			for (auto& axis : output.chan)
			{
				for (int signal = 0; signal < axis.waveInfo[waveInc].signalNum; signal++)
				{
					if (axis.waveInfo[waveInc].signals[signal].initPhase == -1)
					{
						if (signal + 1 > axis.waveInfo[waveInc - 1].signalNum)
						{
							thrower( "ERROR: You are trying to copy the phase of the " + std::to_string( signal + 1 )
									 + " signal of the previous waveform, but the previous waveform only had "
									 + std::to_string( axis.waveInfo[waveInc - 1].signalNum ) + " signals!\r\n" );
						}
						axis.waveInfo[waveInc].signals[signal].initPhase = axis.waveInfo[waveInc - 1].signals[signal].finPhase;
					}
				}
			}
			/// write the new waveform
			// If the correction flag is set, that means that the previous waveform was already passed by, and that I need to address a particular
			// entry of the data, not the next entry.
			for (auto axis : AXES)
			{
				if (repeatFlag == true || rewriteFlag == true)
				{
					if (rewriteFlag == true)
					{
						// In this case, these entries in the waveform vectors have already been written. 
						delete[] variedWaves[axis][writeCounts[axis]];
					}
					variedWaves[axis][writeCounts[axis]] = new ViReal64[output.chan[axis].waveInfo[waveInc].sampleNum];
				}
				else if (repeatFlag == false && rewriteFlag == false)
				{
					// this is equivalent to an else.
					variedWaves[axis].push_back( new ViReal64[output.chan[axis].waveInfo[waveInc].sampleNum] );
				}
				getVariedWaveform( output.chan[axis].waveInfo[waveInc], output.chan[axis].waveInfo, waveInc, waveLibrary, 
								   variedWaves[axis][writeCounts[axis]], debugOptions, debugMsgs );
			}
			// keep in mind that the vertical and horizontal sample numbers should be the same (and this should have been verified)
			// at this point
			if (repeatFlag == true || rewriteFlag == true)
			{
				if (rewriteFlag == true)
				{
					// see above comment about deleting in this case.
					delete[] mixedWaveforms[mixedCount];
				}
				mixedWaveSizes[mixedCount] = 2 * output.chan[Vertical].waveInfo[waveInc].sampleNum;
				mixedWaveforms[mixedCount] = new ViReal64[mixedWaveSizes[mixedCount]];
			}
			else if (repeatFlag == false && rewriteFlag == false)
			{
				mixedWaveSizes.push_back( 2 * output.chan[Vertical].waveInfo[waveInc].sampleNum );
				mixedWaveforms.push_back( new ViReal64[mixedWaveSizes[mixedCount]] );
			}

			mixWaveforms( variedWaves[Vertical][writeCounts[Vertical]],variedWaves[Horizontal][writeCounts[Horizontal]], 
						  mixedWaveforms[mixedCount], output.chan[Vertical].waveInfo[waveInc].sampleNum );

			// reset initial phase if needed...?
			if (waveInc != 0)
			{
				for (auto axis : AXES)
				{
					if (output.chan[axis].waveInfo[waveInc - 1].phaseOption == 1)
					{
						for (int signal = 0; signal < output.chan[axis].waveInfo[waveInc].signalNum; signal++)
						{
							output.chan[axis].waveInfo[waveInc].signals[signal].initPhase = 0;
						}
					}
				}
			}
			if (correctionFlag == true)
			{
				// then I need to go back.
				// Set the correctionFlag back to false so that I don't go back again unless it is called for.
				correctionFlag = false;
				waveInc -= 2;
				writeCounts[Vertical] -= 2;
				writeCounts[Horizontal] -= 2;
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
				rewriteFlag = false;
			}
			// normal exit.
			writeCounts[Vertical]++;
			writeCounts[Horizontal]++;
			mixedCount++;
		}
		// End WaveformLoop
	}
	// clear some memory
	for (auto axis : AXES)
	{
		for (auto& waveform : variedWaves[axis])
		{
			delete[] waveform;
		}
		variedWaves[axis].clear();
	}
}
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void NiawgController::getVariedWaveform(waveInfo &varWaveInfo, std::vector<waveInfo> waveParams, int waveOrderNum, library &waveLibrary, 
										 ViReal64 * waveformRawData, debugInfo options, std::string& debugMsg)
{
	// check phase correction.
	for ( auto signal : range( varWaveInfo.signalNum ) )
	{
		// check if this is supposed to be overwritten during variable runs.
		for ( auto vartype : varWaveInfo.varTypes )
		{
			if ( vartype == -1 )
			{
				if ( signal + 1 > waveParams[waveOrderNum - 1].signalNum )
				{
					thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 ) + "th signal of the previous waveform, "
							 "but the previous waveform only had " + str( waveParams[waveOrderNum - 1].signalNum ) + " signals!\n" );
				}
				varWaveInfo.signals[signal].initPhase = waveParams[waveOrderNum - 1].signals[signal].finPhase;
			}
		}
	}
	// either calculate or read waveform data into the above arrays.
	generateWaveform( waveformRawData, varWaveInfo, waveLibrary, options, debugMsg );
}

/*
	* This function is called when it is time to vary a parameter of a waveform description file before getting the data corresponding to the new waveform.
	* It is mostly a large case structure and has different cases for all of the different parameters you could be changing, as well as several cases for 
	* dealing with correction waveforms and all of their glory.
	*/
void NiawgController::varyParam(std::vector<waveInfo>& allWvInfo1, std::vector<waveInfo>& allWvInfo2, int wfCount, int& paramNum,
								 double paramVal, std::string& warnings)
{
	// change parameters, depending on the case. This was set during input reading.
	std::vector<double> initPhases;
	switch (paramNum)
	{
		case -4: 
		{
			// this is the 1 option for time management control when the next waveform is being varied. 
			// calculate a time for the previous waveform.
			for (auto signal : range( allWvInfo1[wfCount].signalNum ) )
			{
				// check to make sure that the frequencies match
				initPhases.push_back(allWvInfo1[wfCount + 1].signals[signal].finPhase + allWvInfo1[wfCount].signals[signal].initPhase);
			}
			for ( auto signal : range( allWvInfo2[wfCount].signalNum ) )
			{
				// check to make sure that the frequencies match
				initPhases.push_back(allWvInfo2[wfCount + 1].signals[signal].finPhase + allWvInfo2[wfCount].signals[signal].initPhase);
			}
				
			double errVal = myMath::calculateCorrectionTime(allWvInfo1[wfCount], allWvInfo2[wfCount], initPhases, "before");
			if (errVal == -1)
			{
				thrower("ERROR: Correction waveform was not able to match phases.");
			}
			else if (errVal != 0)
			{
				// throw warning
				warnings += "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
					+ str(errVal) + "radians.\r\n";
			}

			// modify the phases of the current waveform such that they reach the 0 phase after the waveform.
			for (int v = 0; v < allWvInfo1[wfCount + 1].signalNum; v++) 
			{
				allWvInfo1[wfCount + 1].signals[v].initPhase = 2 * PI - allWvInfo1[wfCount + 1].signals[v].finPhase;
			}
			// need to reset this so that on future variables things still get calculated in the right order. This doesn't actually lead to case -3 for this variable.
			paramNum = -3;
			break;
		}
		case -3: 
		{
			// Case -3 is the case initially set by a correction wavefunction set to correct the /next/ (as opposed to the previous) 
			// waveform. However, at the time that this gets called, the wavefunction can't get the correct time yet because the next 
			// waveform hasn't been written. This location should never be reached.
			thrower("ERRROR: Bad location reached: -3 case of varyParam(). This is a bug.......");
		}

		case -2:
		{
			// this is the -1 option for time management control when the previous waveform is being varied. 
			for (auto signal : range(allWvInfo1[wfCount - 1].signalNum) )
			{
				initPhases.push_back(allWvInfo1[wfCount].signals[signal].initPhase);
			}
			for ( auto signal : range( allWvInfo2[wfCount - 1].signalNum ) )
			{
				initPhases.push_back(allWvInfo2[wfCount].signals[signal].initPhase);
			}
			double errVal = myMath::calculateCorrectionTime(allWvInfo1[wfCount], allWvInfo2[wfCount], initPhases, 
															 "after");
			if (errVal == -1)
			{
				thrower("ERROR: Correction waveform was not able to match phases.");
			}
			else if (errVal != 0)
			{
				warnings += "WARNING: Correction waveform was not able to match phases very well. The total phase mismatch was "
					+ str(errVal) + "radians.\r\n";
				// throw warning
			}
			break;
		}
		case -1:
		{
			// This is the -1 option for phase control when the previous waveform is being varied.
			for (auto signal : range(allWvInfo1[wfCount].signalNum) )
			{
				// check if this is supposed to be overwritten during variable runs.
				for (auto varType : allWvInfo1[wfCount].varTypes )
				{
					if (varType == -1) 
					{
						if (signal + 1 > allWvInfo1[wfCount - 1].signalNum) 
						{
							thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 )
									 + "th signal of the previous waveform, but the previous "
									 "waveform only had " + str( allWvInfo1[wfCount - 1].signalNum ) + " signals!\n)" );
						}
						allWvInfo1[wfCount].signals[signal].initPhase = allWvInfo1[wfCount - 1].signals[signal].finPhase;
					}
				}
			}
			for (auto signal : range( allWvInfo2[wfCount].signalNum ) )
			{
				// need to check both waveforms at once on this specifically since this information gets used by time-correction waveforms, and the time-correction
				// procedure for x will get called before y changes its initial phase. This is not very efficient currently since this will cause this phase
				// information to get written twice.
				for (auto varType : allWvInfo2[wfCount].varTypes )
				{
					if (varType == -1) 
					{
						if (signal + 1 > allWvInfo2[wfCount - 1].signalNum) 
						{
							thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 )
									 + "th signal of the previous waveform, but the previous "
									 "waveform only had " + str( allWvInfo2[wfCount - 1].signalNum ) + " signals!\n)" );
						}
						allWvInfo2[wfCount].signals[signal].initPhase = allWvInfo2[wfCount - 1].signals[signal].finPhase;
					}
				}
			}

			break;
		}
		case 5 * MAX_NIAWG_SIGNALS + 1:
		{
			if ( paramVal < 0 )
			{
				thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was"
						 + str( paramVal ) );
			}
			// CONVERT from milliseconds to seconds.
			allWvInfo1[wfCount].time = paramVal * 0.001;
			// Changing the time changes the sample number, so recalculate that.
			allWvInfo1[wfCount].sampleNum = this->waveformSizeCalc( allWvInfo1[wfCount] );
			break;
		}
		// all the cases in-between 0 and 5 * MAX_NIAWG_SIGNALS + 1
		default:
		{
			int signalNum = paramNum / 5;
			if (paramNum % 5 + 1 != 5 && (paramVal < 0))
			{
				thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was"
						 + str( paramVal ) );
			}
			switch ( paramNum % 5 + 1)
			{
				case 1:
					allWvInfo1[wfCount].signals[signalNum].freqInit = paramVal;
					break;
				case 2:
					allWvInfo1[wfCount].signals[signalNum].freqFin = paramVal;
					break;
				case 3:
					allWvInfo1[wfCount].signals[signalNum].initPower = paramVal;
					break;
				case 4:
					allWvInfo1[wfCount].signals[signalNum].finPower = paramVal;
					break;
				case 5:
					allWvInfo1[wfCount].signals[signalNum].initPhase = paramVal;
					break;
			}
		}
	}
}


void NiawgController::errChecker(int err)
{
	if (err < 0)
	{		
		thrower( "NIAWG Error: " + this->getErrorMsg() );
	}
	return;
}

/**
* getInputType takes in a string, the string representation of the input type, and returns a number directly corresponding to that input type.
*/
void NiawgController::getInputType(std::string inputType, waveInfo &wvInfo)
{
	// Check against every possible generate input type. 
	wvInfo.initType = -1;
	for ( auto number : range( MAX_NIAWG_SIGNALS ) )
	{
		number += 1;
		if ( inputType == "gen " + str( number ) + ", const" )
		{
			wvInfo.initType = number;
		}
		else if ( inputType == "gen " + str( number ) + ", amp ramp " )
		{
			wvInfo.initType = number + MAX_NIAWG_SIGNALS;
		}
		else if ( inputType == "gen " + str( number ) + ", freq ramp " )
		{
			wvInfo.initType = number + 2 * MAX_NIAWG_SIGNALS;
		}
		else if ( inputType == "gen " + str( number ) + ", freq & amp ramp " )
		{
			wvInfo.initType = number + 3 * MAX_NIAWG_SIGNALS;
		}
	}
	if (wvInfo.initType == -1)
	{
		thrower( "ERROR: waveform input type not found for this input.\n\n" );
	}
}


/**
* WaveformData1 gathers input from the file, and based on the input type, sorts that input into the appropriate structure. It also calculates the number of
* samples that the waveform will have to contain. This function deals with single waveforms.
*/
void NiawgController::getWaveData( ScriptStream &script, waveInfo &waveInfo, std::vector<variable> singletons )
{
	// Initialize the variable counter inside the waveInfo struct to zero:
	waveInfo.varNum = 0;
	waveInfo.signals.resize( waveInfo.signalNum );

	for (int signal = 0; signal < waveInfo.signalNum; signal++)
	{
		switch (waveInfo.initType / MAX_NIAWG_SIGNALS)
		{
			/// the case for "gen ?, const"
			case 0:
			{
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				loadParam( waveInfo.signals[signal].freqInit, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes, 
						   { 5 * signal + 1, 5 * signal + 2 }, singletons );
				// Scale the frequencies to be in hertz. (input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin = waveInfo.signals[signal].freqInit;
				// Can't be varied for this case type
				waveInfo.signals[signal].freqRampType = "nr";
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				loadParam( waveInfo.signals[signal].initPower, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 3, 5 * signal + 4 }, singletons );
				waveInfo.signals[signal].finPower = waveInfo.signals[signal].initPower;
				// Can't be varied
				waveInfo.signals[signal].powerRampType = "nr";
				// Get phase, unless varied.
				loadParam( waveInfo.signals[signal].initPhase, script, waveInfo.varNum, waveInfo.varNames,
						   waveInfo.varTypes, { 5 * signal + 5 }, singletons );
				break;
			}
			/// The case for "gen ?, amp ramp"
			case 1:
			{
				// set the initial and final values to be equal, and to not use a ramp.
				loadParam( waveInfo.signals[signal].freqInit, script, waveInfo.varNum, waveInfo.varNames,
						   waveInfo.varTypes, { 5 * signal + 1, 5 * signal + 2 }, singletons );
			    // Scale the frequencies to be in hertz.(input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin = waveInfo.signals[signal].freqInit;
				// can't be varried.
				waveInfo.signals[signal].freqRampType = "nr";
				std::string tempStr;
				script >> tempStr;
				waveInfo.signals[signal].powerRampType = tempStr;
				loadParam( waveInfo.signals[signal].initPower, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 3 }, singletons );
				loadParam( waveInfo.signals[signal].finPower, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 4 }, singletons );
				loadParam( waveInfo.signals[signal].initPhase, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 5 }, singletons );
				break;
			}
			/// The case for "gen ?, freq ramp"
			case 2:
			{
				// get all parameters from the file
				std::string tempStr;
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), tolower );
				waveInfo.signals[signal].freqRampType = tempStr;
				loadParam( waveInfo.signals[signal].freqInit, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 1 }, singletons );
				loadParam( waveInfo.signals[signal].freqFin, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 2 }, singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin *= 1000000.;
				// set the initial and final values to be equal, and to not use a ramp.
				loadParam( waveInfo.signals[signal].initPower, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 3, 5 * signal + 4 }, singletons );
				waveInfo.signals[signal].finPower = waveInfo.signals[signal].initPower;
				waveInfo.signals[signal].powerRampType = "nr";
				loadParam( waveInfo.signals[signal].initPhase, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 5 }, singletons );
				break;
			}
				/// The case for "gen ?, freq & amp ramp"
			case 3:
			{
				// get all parameters from the file
				std::string tempStr;
				script >> tempStr;
				waveInfo.signals[signal].freqRampType = tempStr;
				loadParam( waveInfo.signals[signal].freqInit, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 1 }, singletons );
				loadParam( waveInfo.signals[signal].freqFin, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 2 }, singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin *= 1000000.;

				// get all parameters from the file
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower );
				waveInfo.signals[signal].powerRampType = tempStr;
				loadParam( waveInfo.signals[signal].initPower, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 3 }, singletons );
				loadParam( waveInfo.signals[signal].finPower, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 4 }, singletons );
				loadParam( waveInfo.signals[signal].initPhase, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
						   { 5 * signal + 5 }, singletons );
			}
			break;
		}
	}
	// get the common things.
	loadParam( waveInfo.time, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
			   { MAX_NIAWG_SIGNALS * 5 + 1 }, singletons );
	// Scale the time to be in seconds. (input is ms)
	waveInfo.time *= 0.001;
	loadParam( waveInfo.phaseOption, script, waveInfo.varNum, waveInfo.varNames,
			   waveInfo.varTypes, { MAX_NIAWG_SIGNALS * 5 + 2 }, singletons );
	script >> waveInfo.delim;
	// Also figure out the size
	waveInfo.sampleNum = waveformSizeCalc( waveInfo );
}


// load all values for waveLibrary.
void NiawgController::openWaveformFiles( library &waveLibrary )
{
	std::string tempStr;
	std::fstream libNameFile;
	for (int folderInc = 0; folderInc < WAVEFORM_TYPE_FOLDERS->size(); folderInc++)
	{
		std::string folderPath = LIB_PATH + WAVEFORM_TYPE_FOLDERS[folderInc];
		folderPath.resize( folderPath.size() - 1 );
		DWORD ftyp = GetFileAttributesA( folderPath.c_str() );
		if (ftyp == INVALID_FILE_ATTRIBUTES || !(ftyp & FILE_ATTRIBUTE_DIRECTORY))
		{
			// create directory
			if (!CreateDirectory( (folderPath).c_str(), NULL ))
			{
				thrower( "ERROR: Error Creating directory for waveform library system. Error was windows error " + str( GetLastError() ) );
			}
		}
		// open the file. It's written in binary.
		std::string libNameFilePath = LIB_PATH + WAVEFORM_TYPE_FOLDERS[folderInc] + WAVEFORM_NAME_FILES[folderInc];
		libNameFile.open( libNameFilePath.c_str(), std::ios::binary | std::ios::in );
		// Make sure that the file opened correctly.
		if (!libNameFile.is_open())
		{
			// try creating a new file.
			libNameFile.open( libNameFilePath.c_str(), std::ios::binary | std::ios::out );
			if (!libNameFile.is_open())
			{
				thrower( "ERROR: waveform library file did not open correctly. Name was " + libNameFilePath );
			}
			libNameFile.close();
		}
		else
		{
			// read all of the waveforms into waveLibrary
			int waveInfoInc = 0;
			// if not empty, the first line will just have a newline on it, so there is no harm in getting rid of it.
			std::getline( libNameFile, tempStr, '\n' );
			while (!libNameFile.eof())
			{
				// read waveform names...
				std::getline( libNameFile, tempStr, '\n' );
				// put them into the array...
				waveLibrary[folderInc].push_back( tempStr );
				waveInfoInc++;
			}
			// save the fileOpened value reflecting it's new status.
			libNameFile.close();
		}
	}
}

/**
* This function works with input data including only one signal.
* This important function takes in the input parameters and from those creates the waveform data array. It is inside this function that the waveform
* "library" functionality is realized. The function checks a list of strings to see if the waveform has previously been generated, and if it has, it reads
* the waveform data from a binary file. If it hasn't been created before, it writes a file containing all of the waveform data.
* Appended to the end of the waveform data files is the final phase of each of the signals involved in the file. This must be stripped off of the voltage
* data that populates the rest of the file as it's being read, and must be appended to the voltage data before it is written to a new file.
*
*/
void NiawgController::generateWaveform( ViReal64 * & tempWaveform, waveInfo & waveInfo, library &waveLibrary, debugInfo options, 
										std::string& debugMsg )
{
	
	// structures that hold a time stamp
	unsigned long long time1, time2;
	// the number of seconds
	std::string waveformFileSpecs, tempStr, waveformFileName;
	std::ifstream waveformFileRead, libNameFileRead;
	std::ofstream waveformFileWrite, libNameFileWrite;
	std::fstream libNameFile;

	// Construct the name of the raw data file from the parameters for the waveform. This can be a pretty long name, but that's okay because it's just text
	// in a file at the end.
	// this might become a problem if the name gets toooo long...
	for ( auto signal : range( waveInfo.signalNum ) )
	{
		waveformFileSpecs += (str( waveInfo.signals[signal].freqInit ) + " " + str( waveInfo.signals[signal].freqFin ) + " "
							   + waveInfo.signals[signal].freqRampType + " " + str( waveInfo.signals[signal].initPower ) + " "
							   + str( waveInfo.signals[signal].finPower ) + " " + waveInfo.signals[signal].powerRampType + " "
							   + str( waveInfo.signals[signal].initPhase ) + ", ");
	}
	waveformFileSpecs += str( waveInfo.time * 1000.0 ) + "; ";
	// Start timer
	time1 = GetTickCount64();
	/// Loop over all previously recorded files (these should have been filled by a previous call to openWaveformFiles()).
	for ( unsigned int fileInc = 0; fileInc < waveLibrary[waveInfo.initType].size(); fileInc++ )
	{
		// if you find this waveform to have already been written...
		if ( waveLibrary[waveInfo.initType][fileInc] == waveformFileSpecs )
		{
			// Construct the file address
			std::string waveFileReadName = LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + str( waveInfo.initType ) + "_"
				+ str( fileInc ) + ".txt";
			waveformFileRead.open( waveFileReadName, std::ios::binary | std::ios::in );
			ViReal64* readData = new ViReal64[waveInfo.sampleNum + waveInfo.signalNum];
			waveformFileRead.read( (char *) readData, (waveInfo.sampleNum + waveInfo.signalNum) * sizeof( ViReal64 ) );
			// grab the phase data off of the end.
			for ( auto signal : range( waveInfo.signalNum ) )
			{
				waveInfo.signals[signal].finPhase = readData[waveInfo.sampleNum + signal];
			}
			// put the relevant voltage data into a the new array.
			std::copy_n( readData, waveInfo.sampleNum, tempWaveform );
			delete[] readData;
			waveformFileRead.close();
			if ( options.showReadProgress == true )
			{
				time2 = GetTickCount64();
				double ellapsedTime = (time2 - time1) / 1000.0;
				debugMsg += "Finished Reading Waveform. Ellapsed Time: " + str(ellapsedTime) + " seconds.\r\n";
			}
			// if the file got read, I don't need to do any writing, so go ahead and return.
			return;
		}
	}

	/// if the code reaches this point, it could not find a file to read, and so will now create the data from scratch and write it. 
	waveformFileName = (LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + str( waveInfo.initType ) + "_"
						 + str( waveLibrary[waveInfo.initType].size() ) + ".txt");
	// open file for writing.
	waveformFileWrite.open( waveformFileName, std::ios::binary | std::ios::out );
	// make sure it opened.
	if ( !waveformFileWrite.is_open() )
	{
		// shouldn't ever happen.
		thrower( "ERROR: Data Storage File could not open. Shouldn't happen. File name is probably too long? File name is " 
				 + waveformFileName + ", which is " + str(waveformFileName.size()) + " characters long.");
	}
	else
	{
		// start timer.
		unsigned long long time1 = GetTickCount64();
		// calculate all voltage values and final phases and store them in the readData variable.
		ViReal64* readData = new ViReal64[waveInfo.sampleNum + waveInfo.signalNum];
		calcWaveData( waveInfo, readData );
		// Write the data, with phases, to the write file.
		waveformFileWrite.write( (const char *) readData, (waveInfo.sampleNum + waveInfo.signalNum) * sizeof( ViReal64 ) );
		waveformFileWrite.close();
		// put the relevant data into another string.
		std::copy_n( readData, waveInfo.sampleNum, tempWaveform );
		delete[] readData;
		// write the newly written waveform's name to the library file.
		libNameFile.open( LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + WAVEFORM_NAME_FILES[waveInfo.initType],
						  std::ios::binary | std::ios::out | std::ios::app );
		if ( !libNameFile.is_open() )
		{
			thrower("ERROR! waveform name file not opening correctly.\n");
		}
		// add the waveform name to the current list of strings. do it BEFORE adding the newline T.T
		waveLibrary[waveInfo.initType].push_back( waveformFileSpecs.c_str() );
		// put a newline in front of the name so that all of the names don't get put on the same line.
		waveformFileSpecs = "\n" + waveformFileSpecs;
		libNameFile.write( (waveformFileSpecs).c_str(), waveformFileSpecs.size() );
		libNameFile.close();
		if ( options.showWriteProgress == true )
		{
			char processTimeMsg[200];
			time2 = GetTickCount64();
			double ellapsedTime = (time2 - time1) / 1000.0;
			sprintf_s( processTimeMsg, "Finished Writing Waveform. Ellapsed Time: %.3f seconds.\r\n", ellapsedTime );
			debugMsg += "Finished Reading Waveform. Ellapsed Time: %.3f seconds.\r\n";
		}
	}
	return;
}


/**
* This function handles logic input. Most of the function of this is to simply figure out which command the user was going for, and append that to the
* actual script file.
*
*/
void NiawgController::logic( niawgPair<ScriptStream>& scripts, niawgPair<std::string> inputs, std::string& scriptString )
{
	// Declare some parameters.
	niawgPair<int> sampleNums;
	niawgPair<int> repeatNums;
	for (auto axis : AXES)
	{
		sampleNums[axis] = 0;
		repeatNums[axis] = 0;
	}

	// In each of the following cases, the x input string is used first identified, and the the y-input command is checked to make sure that it matches
	// the x input command.

	if (inputs[Horizontal] != inputs[Vertical])
	{
		thrower( "ERROR: logical input commands must match, and they don't!\n\n" );
	}
	// Wait commands //
	if (inputs[Horizontal] == "wait until trigger")
	{
		// Append command to script holder
		scriptString += "wait until " + std::string( this->EXTERNAL_TRIGGER_NAME ) + "\n";
	}
	else if (inputs[Horizontal] == "wait set #")
	{
		// grab the # of samples the user wants to wait.
		std::string temp;
		try
		{
			for (auto axis : AXES)
			{
				scripts[axis] >> temp;
				sampleNums[axis] = std::stoi( temp );
			}
		}
		catch (std::invalid_argument&)
		{
			thrower( "ERROR: Sample number inside wait command wasn't an integer! Value was " + temp );
		}
		// make sure they are the same.
		if (sampleNums[Horizontal] != sampleNums[Vertical])
		{
			thrower( "ERROR: sample numbers to wait for must match, and they don't!\n\n" );
		}
		// append command to script holder
		scriptString += "wait " + str( (long long)sampleNums[Horizontal] ) + "\n";
	}
	// Repeat commands // 
	else if (inputs[Horizontal] == "repeat set #")
	{
		// grab the number of times to repeat the user is going for.
		std::string temp;
		try
		{
			for (auto axis : AXES)
			{
				scripts[axis] >> temp;
				repeatNums[axis] = std::stoi( temp );
			}
		}
		catch (std::invalid_argument&)
		{
			thrower( "ERROR: repeat number was not an integer! value was " + temp );
		}
		// verify that the numbers match
		if (repeatNums[Horizontal] != repeatNums[Vertical])
		{
			thrower( "ERROR: number of times to repeat must match, and they don't!\n\n" );
		}
		scriptString += "repeat " + str( (long long)repeatNums[Horizontal] ) + "\n";

	}
	else if (inputs[Horizontal] == "repeat until trigger")
	{
		scriptString += "repeat until " + std::string( EXTERNAL_TRIGGER_NAME ) + "\n";
	}
	else if (inputs[Horizontal] == "repeat forever")
	{
		scriptString += "repeat forever\n";
	}
	else if (inputs[Horizontal] == "end repeat")
	{
		scriptString += "end repeat\n";
	}
	// if-else Commands //
	else if (inputs[Horizontal] == "if trigger")
	{
		scriptString += "if " + std::string( EXTERNAL_TRIGGER_NAME ) + "\n";
	}
	else if (inputs[Horizontal] == "else")
	{
		scriptString += "else\n";
	}
	else if (inputs[Horizontal] == "end if")
	{
		scriptString += "end if\n";
	}
	return;
}


/**
* This function handles the weird case commands, like ones that reference other waveforms or scripts. They are the special cases.
*/
void NiawgController::special( niawgPair<ScriptStream>& scripts, outputInfo& output, niawgPair<std::string> inputTypes, 
							   library &waveLibrary, profileSettings profile, std::vector<variable> singletons, debugInfo options, 
							   std::string& warnings, std::string& debugMessages )
{
	// declare some variables
	// first level is file the name was found in, second level is the name inside that file.
	niawgPair<niawgPair<std::string>> externalScriptNames, externalWaveformNames;
	niawgPair<std::string> waitSamples, importTypes;
	niawgPair<std::fstream> externalWaveformFiles;

	// Interpreting a predefined scripting file.
	if (inputTypes[Vertical] == "predefined script")
	{
		// make sure that both scripts refer to the same predefined scripts
		if (inputTypes[Horizontal]!= "predefined script")
		{
			thrower("ERROR: logical input commands must match, and they don't!\n\n");
		}
		// grab the script names from each file
		for (auto axis : AXES)
		{
			// get both the names from a given script.
			externalScriptNames[axis][Vertical] = scripts[axis].getline( '\r' );
			externalScriptNames[axis][Horizontal] = scripts[axis].getline( '\r' );
		}
		// make sure the names match.
		if (externalScriptNames[Vertical][Vertical] != externalScriptNames[Horizontal][Vertical]
			 || externalScriptNames[Vertical][Horizontal] != externalScriptNames[Horizontal][Horizontal])
		{
			thrower("ERROR: external script names must match for a predefined script command!\n\n");
		}
		// open the scripts
		niawgPair<std::fstream> externalScriptFiles;
		for (auto axis : AXES)
		{
			externalScriptFiles[axis].open( profile.categoryPath + "\\" + externalScriptNames[Vertical][axis] );
			if (!externalScriptFiles[axis].is_open())
			{
				thrower( "ERROR: external " + AXES_NAMES[axis] + " script file: " + externalScriptNames[Vertical][axis]
						 + " could not be opened! make sure it exists in the current category folder: " + profile.categoryPath );
			}
		}
		// recursively call the analyzeNiawgScripts function. This will go through the new script files, write all of their commands to the same script, write
		// waveforms to the AWG, and eventually make it's way back here.
		// this doesn't need a try/except because it's always inside another analyze call.
		niawgPair<ScriptStream> externalScripts;
		for (auto axis : AXES)
		{
			externalScripts[axis] << externalScriptFiles[axis].rdbuf();
		}

		analyzeNiawgScripts( externalScripts, output, waveLibrary, profile, singletons, options, warnings, debugMessages );
	}
	// work with marker events
	else if (inputTypes[Vertical] == "create marker event")
	{
		// Make sure the commands match
		if (inputTypes[Horizontal] != "create marker event")
		{
			thrower("ERROR: logical input commands must match, and they don't!\n\n");
		}
		// get the timing information from the file.
		for (auto axis : AXES)
		{
			scripts[axis] >> waitSamples[axis];
		}
		if (waitSamples[Vertical] != waitSamples[Horizontal])
		{
			thrower( "ERROR: Numer of samples to wait must be the same in the vertical and horizontal files, and they aren't!" );
		}
		// ! remove previous newline to put this command on the same line as a generate command, as it needs to be for the final script. !
		output.niawgLanguageScript.pop_back();
		// append script
		output.niawgLanguageScript += " marker0 (" + waitSamples[Vertical] + ")\n";
	}
	// work with predefined waveforms (mini-scripts)
	else if (inputTypes[Vertical] == "predefined waveform" )
	{
		// make sure the commands match
		if (inputTypes[Horizontal] != "predefined waveform")
		{
			thrower("ERROR: logical input commands must match, and they don't!\n\n");
		}
		// grab the waveform names and command types
		// grab the script names from each file
		for (auto axis : AXES)
		{
			// get both the names from a given script.
			externalWaveformNames[axis][Vertical] = scripts[axis].getline( '\r' );
			externalWaveformNames[axis][Horizontal] = scripts[axis].getline( '\r' );
		}
		// make sure names match
		if (externalWaveformNames[Vertical][Vertical] != externalWaveformNames[Horizontal][Vertical]
			 || externalWaveformNames[Vertical][Horizontal] != externalWaveformNames[Horizontal][Horizontal])
		{
			thrower("ERROR: external waveform names must match!");
		}
		// check if this waveform has already been used. If it has, then just refer to that waveform by name in the script.
		for (int predefinedWaveformInc = 0; predefinedWaveformInc < output.predefinedWaveCount; predefinedWaveformInc++) 
		{
			if (externalWaveformNames[Vertical][Vertical] == output.chan[Vertical].predefinedWaveNames[predefinedWaveformInc]) 
			{
				if (externalWaveformNames[Vertical][Horizontal] == output.chan[Horizontal].predefinedWaveNames[predefinedWaveformInc])
				{
					output.niawgLanguageScript += "generate Waveform" + str(output.predefinedWaveLocs[predefinedWaveformInc]) + "\n";
				}
			}
		}
		// add the name of the waveform to the list.
		output.chan[Vertical].predefinedWaveNames.push_back( externalWaveformNames[Vertical][Vertical] );
		output.chan[Horizontal].predefinedWaveNames.push_back( externalWaveformNames[Vertical][Horizontal] );
		// record the waveform number that corresponds to this new waveform.
		output.predefinedWaveLocs.push_back(output.predefinedWaveCount);
		// incremend the number of predefined waves that have been used.
		output.predefinedWaveCount++;

		// open the waveform files
		externalWaveformFiles[Vertical].open( externalWaveformNames[Vertical][Vertical] );
		externalWaveformFiles[Horizontal].open( externalWaveformNames[Vertical][Horizontal] );
		niawgPair<ScriptStream> externalWaveformStreams;
		for (auto axis : AXES)
		{
			externalWaveformStreams[axis] << externalWaveformFiles[axis].rdbuf();
		}
		// analyzeNiawgScripts here works in the same way as it does for longer scripts. The effect here is simply to read the one waveform into the same script 
		// file.
		analyzeNiawgScripts( externalWaveformStreams, output, waveLibrary, profile, singletons, options, warnings, debugMessages );
	}
	else
	{
		thrower("ERROR: special command not found!");
	}
}


/**
* This namespace includes all of the relevant functions for generating waveforms.
*/
long NiawgController::waveformSizeCalc(double time)
{
	double waveSize = time * SAMPLE_RATE;
	return (long int)(waveSize + 0.5);
}


/**
* This function takes in the data for a single waveform and calculates all if the waveform's data points, and returns a pointer to an array containing
* these data points.
*/
ViReal64* NiawgController::calcWaveData(waveInfo& inputData, ViReal64*& readData)
{
	// Declarations
	std::vector<double> powerPos, freqRampPos, phasePos(inputData.signalNum);
	std::vector<double*> powerRampFileData;
	std::vector<double*> freqRampFileData;
	std::fstream powerRampFile, freqRampFile;
	std::string tempStr;

	/// deal with ramp calibration files. check all signals for files and read if yes.
	for (auto signal : range(inputData.signalNum))
	{
		// create spots for the ramp positions.
		powerPos.push_back(0);
		freqRampPos.push_back(0);
		// If the ramp type isn't a standard command...
		if (inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr" 
			 && inputData.signals[signal].powerRampType != "tanh")
		{
			powerRampFileData.push_back(new double[inputData.sampleNum]);
			// try to open it as a file
			powerRampFile.open(inputData.signals[signal].powerRampType);
			// if successful....
			if (powerRampFile.is_open())
			{
				int powerValNumber = 0;
				// load the data in
				while (!powerRampFile.eof())
				{
					powerRampFile >> powerRampFileData[signal][powerValNumber];
					powerValNumber++;
				}
				// error message for bad size (powerRampFile.eof() reached too early or too late).
				if (powerValNumber != inputData.sampleNum + 1)
				{
					
					thrower( "ERROR: file not the correct size?\nSize of upload is " + str( powerValNumber ) 
							 + "; size of file is " + str( inputData.sampleNum ));
				}
				// close the file.
				powerRampFile.close();
			}
			else
			{
				thrower("ERROR: ramp type " + std::string(inputData.signals[signal].powerRampType)
						+ " is unrecognized. If this is a file name, make sure the file exists and is in the project folder. ");
			}
		}
		// If the ramp type isn't a standard command...
		if (inputData.signals[signal].freqRampType != "lin" && inputData.signals[signal].freqRampType != "nr"
			 && inputData.signals[signal].freqRampType != "tanh")
		{
			// try to open it
			freqRampFileData.push_back(new double[inputData.sampleNum]);
			freqRampFile.open(inputData.signals[signal].freqRampType, std::ios::in);
			// if open
			if (freqRampFile.is_open())
			{
				int freqRampValNum = 0;
				while (!freqRampFile.eof())
				{
					freqRampFile >> freqRampFileData[signal][freqRampValNum];
					freqRampValNum++;
				}
				// error message for bad size (powerRampFile.eof() reached too early or too late).
				if (freqRampValNum != inputData.sampleNum + 1)
				{
					thrower("ERROR: file not the correct size?\nSize of upload is " + str(freqRampValNum) 
							 + "; size of file is " + str( inputData.sampleNum ));
				}
				// close file
				freqRampFile.close();
			}
			else
			{
				thrower( "ERROR: ramp type " + inputData.signals[signal].freqRampType + " is unrecognized. If this is a file name, make sure the"
						 " file exists and is in the project folder." );
			}
		}
	}

	/// calculate frequency differences for every signal. This is used for frequency ramps.
	std::vector<double> deltaOmega;
	double deltaTanh = std::tanh(4) - std::tanh(-4);
	for (int signal = 0; signal < inputData.signalNum; signal++)
	{
		deltaOmega.push_back(2 * PI * (inputData.signals[signal].freqFin - inputData.signals[signal].freqInit));
	}
	///		Get Data Points.		///
	// initialize signalInc before the loop so that I have access to it afterwards.
	int sample = 0;
	/// increment through all samples
	for (; sample < inputData.sampleNum; sample++)
	{
		// calculate the time that this sample number refers to
		double curTime = (double)sample / SAMPLE_RATE;
		/// Calculate Phase and Power Positions. For Every signal...
		for (auto signal : range(inputData.signalNum) )
		{
			/// Handle Frequency Ramps
			// Frequency ramps are actually a little complex. we have dPhi/dt = omega(t) and we need phi to calculate data points. So in order to get 
			// the phase you need to integrate the omega(t) you want and modify the integration constant to get your initial phase.
			if (inputData.signals[signal].freqRampType == "lin")
			{
				// W{t} = Wi + (DeltaW * t) / (Tfin)
				// Phi{t}   = Wi * t + (DeltaW * t ^ 2) / 2 + phi_i
				phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + deltaOmega[signal] * pow(curTime, 2) / (2 * inputData.time) 
							+ inputData.signals[signal].initPhase;
			}
			else if (inputData.signals[signal].freqRampType == "tanh")
			{
				// We want
				// W{t} = Wi + (DeltaW * (Tanh{-4 + 8 * t' / Tf} - Tanh{-4}) / (Tanh{4} - Tanh{-4})
				// This gives the correct initial value, final value, and symmetry of the ramp for the frequency. -4 was picked somewhat arbitrarily.
				// The integral of this is
				// dw/(2*tanh{4}) * T_f/8 * ln{cosh{-4+8t/T_f}} + (w_i - (dw * tanh{-4})/2*tanh{4}) t + C
				// Evaluating C to give the correct phase gives
				// phi{t} = (w_i+dw/2)t+(dw)/dtanh * T_f/8 * (ln{cosh{-4+8t/T_f}}-ln{cosh{-4}}) + phi_0
				// See onenote for more math.
				phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
								+ (deltaOmega[signal] / deltaTanh ) * (inputData.time / 8.0) * ( std::log(std::cosh(4 - (8 / inputData.time) * curTime))
								- std::log(std::cosh(4))) + inputData.signals[signal].initPhase;
			}
			else if (inputData.signals[signal].freqRampType == "nr")
			{
				// omega{t} = omega
				// phi = omega*t
				phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + inputData.signals[signal].initPhase;
			}
			else
			{
				// special ramp case. I'm not sure if this is actually useful. The frequency file would have to be designed very carefully.
				freqRampPos[signal] = freqRampFileData[signal][sample] * (inputData.signals[signal].freqFin - inputData.signals[signal].freqInit);
				phasePos[signal] = (ViReal64)sample * 2 * PI * (inputData.signals[signal].freqInit + freqRampPos[signal]) / SAMPLE_RATE
								+ inputData.signals[signal].initPhase;
			}
			/// amplitude ramps are much simpler.
			if (inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr" && inputData.signals[signal].powerRampType
				 != "tanh")
			{
				// use data from file
				powerPos[signal] = powerRampFileData[signal][sample] * (inputData.signals[signal].finPower - inputData.signals[signal].initPower);
			}
			else
			{
				// use the ramp calc function to find the current power.
				powerPos[signal] = myMath::rampCalc( inputData.sampleNum, sample, inputData.signals[signal].initPower, inputData.signals[signal].finPower,
													 inputData.signals[signal].powerRampType );
			}
		}

		/// If option is marked, then normalize the power.
		if (CONST_POWER_OUTPUT == true)
		{
			double currentPower = 0;
			// calculate the total current amplitude.
			for (auto signal : range(inputData.signalNum) )
			{
				currentPower += fabs(inputData.signals[signal].initPower + powerPos[signal]);
				/// modify here for frequency calibrations!
				/// need current frequency and calibration file.
			}

			// normalize each signal.
			for (auto signal : range(inputData.signalNum) )
			{
				powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal]) 
					* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
			}
		}
		///  Calculate data point.
		readData[sample] = 0;
		for (auto signal : range(inputData.signalNum) )
		{
			// get data point. V = Sqrt(Power) * Sin(Phase)
			readData[sample] += sqrt(inputData.signals[signal].initPower + powerPos[signal]) * sin(phasePos[signal]);
		}
	}

	/// Calculate one last time for the final phases. I want the final phase to be the phase of the NEXT data point. Then, following waveforms can
	/// START at this phase.
	double curTime = (double)sample / SAMPLE_RATE;
	for (auto signal : range( inputData.signalNum ) )
	{
		// Calculate Phase Position. See above for description.
		if (inputData.signals[signal].freqRampType == "lin")
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + deltaOmega[signal] * pow(curTime, 2) * 1 / (2 * inputData.time) 
							+ inputData.signals[signal].initPhase;
		}
		else if (inputData.signals[signal].freqRampType == "tanh")
		{
			phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
				+ (deltaOmega[signal] / deltaTanh) * (inputData.time / 8.0) * std::log(std::cosh(4 - (8 / inputData.time) * curTime))
				- (deltaOmega[signal] / deltaTanh) * (inputData.time / 8.0) * std::log(std::cosh(4))
				+ inputData.signals[signal].initPhase;
		}
		else if (inputData.signals[signal].freqRampType == "nr")
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + inputData.signals[signal].initPhase;
		}
		else
		{
			freqRampPos[signal] = freqRampFileData[signal][sample] * (inputData.signals[signal].freqFin - inputData.signals[signal].freqInit);
			phasePos[signal] = (ViReal64)sample * 2 * PI * (inputData.signals[signal].freqInit + freqRampPos[signal]) / (SAMPLE_RATE)
							+inputData.signals[signal].initPhase;
		}
		// Don't need amplitude info.
	}
	
	for (auto signal : range(inputData.signalNum) )
	{
		// get the final phase of this waveform. Note that this is the phase of the /next/ data point (the last time signalInc gets incremented, the for loop 
		// doesn't run) so that if the next waveform starts at this data point, it will avoid repeating the same data point. This is used for the 
		// option where the user uses this phase as the starting phase of the next waveform.
		inputData.signals[signal].finPhase = fmod(phasePos[signal], 2 * PI);
		// catch the case in which the final phase is virtually identical to 2*PI, which isn't caught in the above line because of bad floating point 
		// arithmetic.
		if (fabs(inputData.signals[signal].finPhase - 2 * PI) < 0.00000005)
		{
			inputData.signals[signal].finPhase = 0;
		}
		// put the final phase in the last data point.
		readData[sample + signal] = inputData.signals[signal].finPhase;
	}
	return readData;
}


/**
* This function takes two filled waveform arrays, and interweaves them into a new data array. this is required (strangely) by the NI card for outputting to
* both outputs separately.
*/
ViReal64* NiawgController::mixWaveforms( ViReal64* verticalWaveform, ViReal64* HorizontalWaveform, ViReal64* finalWaveform, 
										 long waveformSize )
{
	for (auto sample : range(waveformSize))
	{
		// the order (Vertical -> Horizontal) here is important. Vertical is first because it's port zero on the Niawg. I believe that
		// switching the order here and changing nothing else would flip the output of the niawg..
		finalWaveform[2 * sample] = verticalWaveform[sample];
		finalWaveform[2 * sample + 1] = HorizontalWaveform[sample];
	}
	return finalWaveform;
}


/**
* this function takes in a command and checks it against all "logic commands", returing true if the inputted command matches a logic 
* command and false otherwise
*/
bool NiawgController::isLogicCommand(std::string command)
{
	if (command == "wait until trigger" || command == "wait set #" || command == "repeat set #" || command == "repeat until trigger" 
		 || command == "repeat forever" || command == "end repeat" || command == "if trigger" || command == "else" || command == "end if")
	{
		return true;
	}
	else
	{
		return false;
	}
}


/**
* this function takes in a command and checks it against all "generate commands", returing true if the inputted command matches a generate 
* command and false otherwise
*/
bool NiawgController::isGenCommand(std::string inputType)
{
	for ( auto number : range( MAX_NIAWG_SIGNALS ) )
	{
		if ( inputType == "gen " + str( number ) + ", const" || inputType == "gen " + str( number ) + ", amp ramp " 
			 || inputType == "gen " + str( number ) + ", freq ramp " || inputType == "gen " + str( number ) + ", freq & amp ramp ")
		{
			return true;
		}
	}
	return false;
}


/**
	* this function takes in a command and checks it against all "special commands", returing true if the inputted command matches a special command and false
	* otherwise
	*/
bool NiawgController::isSpecialCommand(std::string command)
{
	if (command == "predefined script" || command == "create marker event" || command == "predefined waveform")
	{
		return true;
	}
	else 
	{
		return false;
	}
}


//
//		A series of sanity checks on the waveform parameters. This is ment to catch user error. The following checks for...
//		- Phase Continuity between waveforms (not checking repeating waveforms (yet))
//		- Amplitude Continuity between waveforms
//		- Frequency Continuity between waveforms
//		- Sensible Ramping Options (initial and final freq/amp values reflect choice of ramp or no ramp).
//		- Sensible Phase Correction Options
// 			
void NiawgController::checkThatWaveformsAreSensible(Communicator* comm, outputInfo& output)
{
	for (auto axis : AXES)
	{
		for (int contInc = 2; contInc < output.chan[axis].waveInfo.size(); contInc++)
		{
			// if two waveforms have the same number of parameters...
			if (output.chan[axis].waveInfo[contInc].signals.size() == output.chan[axis].waveInfo[contInc - 1].signals.size())
			{
				for (int waveInc = 0; waveInc < output.chan[axis].waveInfo[contInc].signals.size(); waveInc++)
				{
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].initPower
						 != output.chan[axis].waveInfo[contInc - 1].signals[waveInc].finPower)
					{
						comm->sendError( "Warning: Amplitude jump at waveform #" + std::to_string( contInc )
												+ " in " + AXES_NAMES[axis] + " component detected!\r\n" );
					}
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].freqInit
						 != output.chan[axis].waveInfo[contInc - 1].signals[waveInc].freqFin)
					{
						comm->sendError( "Warning: Frequency jump at waveform #" + std::to_string( contInc ) + " in "
												+ AXES_NAMES[axis] + " component detected!\r\n" );
					}
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].initPhase
						 - output.chan[axis].waveInfo[contInc - 1].signals[waveInc].finPhase
				> CORRECTION_WAVEFORM_ERROR_THRESHOLD)
					{
						comm->sendError( "Warning: Phase jump (greater than what's wanted for correction waveforms) at "
												"waveform #" + std::to_string( contInc ) + " in " + AXES_NAMES[axis] + " component "
												"detected!\r\n" );
					}
					// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].powerRampType != "nr"
						 && (output.chan[axis].waveInfo[contInc].signals[waveInc].initPower
							  == output.chan[axis].waveInfo[contInc].signals[waveInc].finPower))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to amplitude ramp, but the initial and final "
												"amplitudes are the same. This is not a ramp.\r\n" );
					}
					// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].freqRampType != "nr"
						 && (output.chan[axis].waveInfo[contInc].signals[waveInc].freqInit
							  == output.chan[axis].waveInfo[contInc].signals[waveInc].freqFin))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to frequency ramp, but the initial and final "
												"frequencies are the same. This is not a ramp.\r\n" );
					}

					// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually ramping.
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].powerRampType == "nr"
						 && (output.chan[axis].waveInfo[contInc].signals[waveInc].initPower
							  != output.chan[axis].waveInfo[contInc].signals[waveInc].finPower))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to no amplitude ramp, but the initial and final amplitudes are the "
												"different. This is not a ramp, the initial value will be used.\r\n" );
					}
					// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
					if (output.chan[axis].waveInfo[contInc].signals[waveInc].freqRampType == "nr"
						 && (output.chan[axis].waveInfo[contInc].signals[waveInc].freqInit
							  != output.chan[axis].waveInfo[contInc].signals[waveInc].freqInit))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to no frequency ramp, but the initial and final frequencies are "
												"different. This is not a ramp, the initial value will be used throughout.\r\n" );
					}
					if (output.chan[axis].waveInfo[contInc].phaseOption == -1
						 && output.chan[axis].waveInfo[contInc].signals[waveInc].phaseOption != -1)
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set correct the phase of the previous waveform, but is not using the final "
												"phase of the previous waveform. If you want phase continuity, set the initial "
												"phase of this waveform to be -1, which is code for grabbing the final phase of "
												"the previous waveform.\r\n" );
					}
					if (contInc != 0)
					{
						// check if user is trying to copy the final phase of the correction waveform. You aren't supposed to do that.
						if (output.chan[axis].waveInfo[contInc - 1].phaseOption == 1
							 && output.chan[axis].waveInfo[contInc].signals[waveInc].phaseOption != 0)
						{
							comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
													+ "is set correct the phase of the following waveform, but the following "
													"waveform is grabbing the phase of this correction waveform. It's not supposed"
													" to do that, it needs to start at zero phase in order for this to work "
													"correctly.\r\n" );
						}
					}
				}
			}
		}
	}
}


std::string NiawgController::getErrorMsg()
{
	ViChar* errMsg;
	int errMsgSize = 0;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errMsgSize = niFgen_GetError( sessionHandle, VI_NULL, 0, VI_NULL );
	}
	errMsg = (ViChar *)malloc( sizeof( ViChar ) * errMsgSize );
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		niFgen_GetError( sessionHandle, &error, errMsgSize, errMsg );
	}
	std::string errStr( errMsg );
	free( errMsg );
	return errStr;
}


void NiawgController::configureGain( ViReal64 gain )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureGain( sessionHandle, outputChannels, gain ) );
	}
}


void NiawgController::configureSampleRate( ViReal64 sampleRate )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureSampleRate( sessionHandle, sampleRate ) );
	}
}


void NiawgController::configureChannels( ViConstString channelName )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureChannels( sessionHandle, channelName ) );
	}
}


void NiawgController::configureMarker( ViConstString markerName, ViConstString outputLocation )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ExportSignal( sessionHandle, NIFGEN_VAL_MARKER_EVENT, markerName, outputLocation ) );
	}
}


// initialize the session handle, which is a member of this class.
void NiawgController::init( ViRsrc location, ViBoolean idQuery, ViBoolean resetDevice )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_init( location, idQuery, resetDevice, &sessionHandle ) );
	}
}


void NiawgController::createWaveform( long size, ViReal64* mixedWaveform )
{
	ViInt32 waveID;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_CreateWaveformF64( sessionHandle, outputChannels, size, mixedWaveform, &waveID ) );
	}
}


// put waveform into the device memory
void NiawgController::writeWaveform( ViConstString waveformName, ViInt32 mixedSampleNumber, ViReal64* mixedWaveform )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_WriteNamedWaveformF64( sessionHandle, outputChannels, waveformName, mixedSampleNumber, mixedWaveform ) );
	}
}


void NiawgController::writeScript( ViConstString script )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_WriteScript( sessionHandle, outputChannels, script ) );
	}
}


void NiawgController::deleteWaveform( ViConstString waveformName )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_DeleteNamedWaveform( sessionHandle, outputChannels, waveformName ) );
	}
}


void NiawgController::deleteScript( ViConstString scriptName )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_DeleteScript( sessionHandle, outputChannels, scriptName ) );
	}
}


void NiawgController::allocateWaveform( ViConstString waveformName, ViInt32 unmixedSampleNumber )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_AllocateNamedWaveform( sessionHandle, outputChannels, waveformName, unmixedSampleNumber ) );
	}
}


void NiawgController::configureOutputEnabled( int state )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureOutputEnabled( sessionHandle, outputChannels, VI_FALSE ) );
	}
}


void NiawgController::clearMemory()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ClearArbMemory( sessionHandle ) );
	}
}


void NiawgController::setViStringAttribute( ViAttr atributeID, ViConstString attributeValue )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViString( sessionHandle, outputChannels, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript" ) );
	}
}


void NiawgController::setViBooleanAttribute( ViAttr attribute, bool state )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViBoolean( sessionHandle, outputChannels, NIFGEN_ATTR_FLATNESS_CORRECTION_ENABLED, VI_TRUE ) );
	}
}


void NiawgController::enableAnalogFilter( ViReal64 filterFrequency )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_EnableAnalogFilter( sessionHandle, outputChannels, filterFrequency ) );
	}
}


void NiawgController::configureSoftwareTrigger()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureSoftwareEdgeScriptTrigger( sessionHandle, SOFTWARE_TRIGGER_NAME ) );
	}
}


void NiawgController::configureDigtalEdgeScriptTrigger()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureDigitalEdgeScriptTrigger( sessionHandle, EXTERNAL_TRIGGER_NAME, TRIGGER_SOURCE, TRIGGER_EDGE_TYPE ) );
	}
}


void NiawgController::configureClockMode( ViInt32 clockMode )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureClockMode( sessionHandle, clockMode ) );
	}
}


void NiawgController::initiateGeneration()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_InitiateGeneration( sessionHandle ) );
	}
}


void NiawgController::abortGeneration()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_AbortGeneration( sessionHandle ) );
	}
}


void NiawgController::setAttributeViString( ViAttr attribute, ViString string )
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViString( sessionHandle, outputChannels, attribute, string ) );
	}
}

ViInt32 NiawgController::getInt32Attribute( ViAttr attribute )
{
	ViInt32 value = 0;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViInt32( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


ViInt64 NiawgController::getInt64Attribute( ViAttr attribute )
{
	ViInt64 value = 0;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViInt64(sessionHandle, outputChannels, attribute, &value ));
	}
	return value;
}


ViReal64 NiawgController::getReal64Attribute( ViAttr attribute )
{
	ViReal64 value = 0;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViReal64( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


std::string NiawgController::getViStringAttribute( ViAttr attribute )
{
	char value[256];
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViString( sessionHandle, outputChannels, attribute, 256, value ) );
	}
	return std::string(value);
}


ViBoolean NiawgController::getViBoolAttribute( ViAttr attribute )
{
	ViBoolean value = false;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViBoolean( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


ViSession NiawgController::getViSessionAttribute( ViAttr attribute )
{
	ViSession value = 0;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViSession(sessionHandle, outputChannels, attribute, &value) );
	}
	return value;
}

