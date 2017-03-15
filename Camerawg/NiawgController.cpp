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
		delete default_vConfigScript;
		delete default_hConfigScript;
		delete default_hConfigMixedWaveform;
		delete default_vConfigMixedWaveform;
	}
	// counts the number of predefined waveforms that have been handled or defined.
	int predWaveformCount = 0;
	// counts the number of varied waveform pairs for x or y waveforms.
	int horizontalVariedWaveformPairsCount = 0, verticalVariedWaveformPairsCount = 0;
	// Socket object for communicating with the other computer.
	CSocket* socket = mainWin->getSocket();
	//SOCKET ConnectSocket = INVALID_SOCKET;
	// An array of variable files. Only used if not receiving variable information from the master computer.
	std::vector<std::fstream> xVarFiles;
	// Vectors of flags that signify whether a given waveform is being varied or not.
	std::vector<bool> verticalWaveformIsVaried, horizontalWaveformIsVaried, intensityIsVaried;
	// A vector of vectors that hold variables. Each sub-vector contains all of the values that an individual variable will take on. The main vector then 
	// contains all of the variable value data.
	std::vector<std::vector<double> > variableValues;
	// A vector which stores the number of values that a given variable will take through an experiment.
	std::vector<std::size_t> length;
	// An array of vectors holding strings. Each vector within the array is for a specific type of waveform output (e.g. gen 1, const). The each string within a
	// vector contains unique descriptions of waveforms. I check this list to see if a waveform has been written already.
	std::vector<std::string> libWaveformArray[MAX_NIAWG_SIGNALS * 4];
	std::vector<std::fstream> hConfigHFile, hConfigVFile, vConfigHFile, vConfigVFile;

	hConfigHFile.push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript" ) );
	hConfigVFile.push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript" ) );
	vConfigHFile.push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript" ) );
	vConfigVFile.push_back( std::fstream( DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript" ) );
	// check errors
	if (!hConfigHFile[0].is_open())
	{
		thrower( "FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH
				 + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript" );
	}
	if (!hConfigHFile[0].is_open())
	{
		thrower( "FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH
				 + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript" );
	}
	if (!hConfigHFile[0].is_open())
	{
		thrower( "FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH
				 + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript" );
	}
	if (!hConfigHFile[0].is_open())
	{
		thrower( "FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH
				 + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript" );
	}

	outputInfo output;
	// first line of every script is "script 'name'".
	output.niawgLanguageScript = "script DefaultHConfigScript\n";
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
	profile.orientation = HORIZONTAL_ORIENTATION;
	try
	{
		niawgPair<ScriptStream> scripts;
		scripts[Horizontal] << hConfigHFile[0].rdbuf();
		scripts[Vertical] << hConfigVFile[0].rdbuf();
		mainWin->setOrientation( HORIZONTAL_ORIENTATION );
		analyzeNiawgScripts( scripts, output, libWaveformArray, profile, noSingletons, dummyOptions, warnings, debugMessages );
		// the script file must end with "end script".
		output.niawgLanguageScript += "end Script";
		// Convert script string to ViConstString. +1 for a null character on the end.
		default_hConfigScript = new ViChar[output.niawgLanguageScript.length() + 1];
		sprintf_s( default_hConfigScript, output.niawgLanguageScript.length() + 1, "%s", output.niawgLanguageScript.c_str() );
		strcpy_s( default_hConfigScript, output.niawgLanguageScript.length() + 1, output.niawgLanguageScript.c_str() );
		output.niawgLanguageScript = "script DefaultVConfigScript\n";
		profile.orientation = VERTICAL_ORIENTATION;
		mainWin->setOrientation( VERTICAL_ORIENTATION );
		scripts[Horizontal].clear();
		scripts[Horizontal] << vConfigHFile[0].rdbuf();
		scripts[Vertical].clear();
		scripts[Vertical] << vConfigVFile[0].rdbuf();
		analyzeNiawgScripts( scripts, output, libWaveformArray, profile, noSingletons, dummyOptions, warnings, debugMessages );
		// Convert script string to ViConstString. +1 for a null character on the end.
		default_vConfigScript = new ViChar[output.niawgLanguageScript.length() + 1];
		sprintf_s( default_vConfigScript, output.niawgLanguageScript.length() + 1, "%s", output.niawgLanguageScript.c_str() );
		strcpy_s( default_vConfigScript, output.niawgLanguageScript.length() + 1, output.niawgLanguageScript.c_str() );
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
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			errChecker(niFgen_ConfigureOutputEnabled(sessionHandle, outputChannels, VI_FALSE));
			// Officially stop trying to generate anything.
			errChecker(niFgen_AbortGeneration(sessionHandle));
		}
		myAgilent::agilentDefault();
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			// clear the memory
			errChecker(niFgen_ClearArbMemory(sessionHandle));
		}
		ViInt32 waveID;
		if (defaultOrientation == HORIZONTAL_ORIENTATION)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// create waveform (necessary?)
				errChecker(niFgen_CreateWaveformF64(sessionHandle, outputChannels, default_hConfigMixedSize, default_hConfigMixedWaveform, &waveID));
				// allocate waveform into the device memory
				errChecker(niFgen_AllocateNamedWaveform(sessionHandle, outputChannels, default_hConfigWaveformName.c_str(), default_hConfigMixedSize / 2));
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
errChecker( niFgen_WriteNamedWaveformF64( sessionHandle, outputChannels, default_hConfigWaveformName.c_str(), default_hConfigMixedSize, default_hConfigMixedWaveform ) );
// rewrite the script. default_hConfigScript should still be valid.
errChecker( niFgen_WriteScript( sessionHandle, outputChannels, default_hConfigScript ) );
			}
			eCurrentScript = "DefaultHConfigScript";
		}
		else if (defaultOrientation == VERTICAL_ORIENTATION)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// create waveform (necessary?)
				errChecker( niFgen_CreateWaveformF64( sessionHandle, outputChannels, default_vConfigMixedSize, default_vConfigMixedWaveform, &waveID ) );
				// allocate waveform into the device memory
				errChecker( niFgen_AllocateNamedWaveform( sessionHandle, outputChannels, default_vConfigWaveformName.c_str(), default_vConfigMixedSize / 2 ) );
				// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
				errChecker( niFgen_WriteNamedWaveformF64( sessionHandle, outputChannels, default_vConfigWaveformName.c_str(), default_vConfigMixedSize, default_vConfigMixedWaveform ) );
				// rewrite the script. default_hConfigScript should still be valid.
				errChecker( niFgen_WriteScript( sessionHandle, outputChannels, default_vConfigScript ) );
			}
			eCurrentScript = "DefaultVConfigScript";
		}
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{

			errChecker( niFgen_ConfigureOutputEnabled( sessionHandle, outputChannels, VI_TRUE ) );
			errChecker( niFgen_SetAttributeViString( sessionHandle, outputChannels, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript" ) );
			// Initiate Generation.
			errChecker( niFgen_InitiateGeneration( sessionHandle ) );
		}
	}
	catch (myException& except)
	{
		std::string errMsg = "WARNING! The NIAWG encountered an error and was not able to restart smoothly. It is (probably) not outputting anything. You may "
			"consider restarting the code. Inside the restart area, NIAWG function returned " + except.whatStr();
		errBox( errMsg.c_str() );
	}
}


void NiawgController::setDefaultOrientation( std::string orientation )
{
	this->defaultOrientation = orientation;
}


/**
	* analyzeNiawgScripts() is essentially a massive subroutine. In retrospect, making this a subroutine was probably mostly unnecessary, and makes the code heierarchy
	* needlessly more complicated. The purpose of this function is to systematically read the input instructions files, create and read waveforms associated with
	* them, write the script as it goes, and eventually combine the x and y-waveforms into their final form for being sent to the waveform generator.
	*/
void NiawgController::analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, outputInfo& output,
										   std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4],
										   profileSettings profile, std::vector<variable> singletons, debugInfo options,
										   std::string& warnings, std::string& debugMsg )
{
	std::stringstream temp;
	for (auto& axis : AXES)
	{
		temp = std::stringstream();
		temp << scripts[axis].rdbuf();
		currentScripts[axis] = temp.str();
		scripts[axis].clear();
		scripts[axis].seekg( 0, std::ios::beg );
	}
	// Some declarations.
	niawgPair<std::string> inputTypes;
	// This parameter is used to hold the waveform name (e.g. waveform1) that actually appears in the final script. I included a couple 
	// extra places for good measure.
	ViChar tempWaveformName[11];
	ViChar prevTempWaveName[11];
	// these hold actual waveform data.
	niawgPair<ViReal64*> waveVoltages = { NULL, NULL }, waveFileData = { NULL, NULL }, prevVoltages = { NULL, NULL },
		prevFileData = { NULL, NULL };
	ViReal64* mixedWaveVoltages = NULL, *prevMixedVoltages = NULL;
	openWaveformFiles( libWaveformArray );
	/// analyze complete vertical file.
	while (!scripts[Vertical].eof())
	{
		if (scripts[Horizontal].eof())
		{
			thrower( "User's horizontal script file has ended before the vertical script file. Please make sure that the "
					 "number of commands in each file matches.\r\n" );
		}
		// these are the variables that hold current basic information about the waveform being generated. They are used to create the raw 
		// data points. They are temporary, and pass their information into the larger array of waveData structures.
		niawgPair<waveData> waveParams;

		sprintf_s( tempWaveformName, 11, "Waveform%i", output.waveCount );
		sprintf_s( prevTempWaveName, 11, "Waveform%i", output.waveCount - 1 );
		for (auto axis : AXES)
		//for (auto& type : inputTypes.AXES)
		{
			// get the instructions line.
			inputTypes[axis] = scripts[axis].getline();
			// handle trailing newline characters
			if (inputTypes[axis].length() != 0)
			{
				if (inputTypes[axis][inputTypes[axis].length() - 1] == '\r')
				{
					inputTypes[axis].erase( inputTypes[axis].length() - 1 );
				}
			}
		}

		// Send the command to different places depending on what type of command it is.
		if (inputTypes[Horizontal] != inputTypes[Vertical])
		{
			thrower( "ERROR: Input types must match, and they don't!" );
		}
		else if (isLogicCommand( inputTypes[Horizontal] ))
		{
			logic( scripts, inputTypes, output.niawgLanguageScript );
		}
		else if (isSpecialCommand( inputTypes[Horizontal] ) )
		{
			special( scripts, output, inputTypes, libWaveformArray, profile, singletons, options, warnings, debugMsg );
		}
		// the most complex case.
		else if (isGenCommand( inputTypes[Horizontal] ) )
		{
			//
			if ((output.waveCount == 1 && output.isDefault && profile.orientation == HORIZONTAL_ORIENTATION)
				 || (output.waveCount == 2 && output.isDefault && profile.orientation == VERTICAL_ORIENTATION))
			{
				thrower( "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must be a "
						 "single waveform, not a sequence.\r\n" );
			}
			// 
			for (auto axis : AXES)
			{
				// The default value is false. If the waveform isn't varied, it gets changed to true.
				waveParams[axis].isVaried = false;
				// Get a number corresponding directly to the given input type.
				getInputType(inputTypes[axis], waveParams[axis]);
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

			int currentPhaseManageOption;
			// make sure that the waveform management options match (they must, or else the times might not match.
			if (waveParams[Horizontal].phaseManagementOption != waveParams[Vertical].phaseManagementOption)
			{
				thrower( "ERROR: the vertical and horizontal waveforms must have the same time management option. They appear to be "
						 "mismatched for waveform #" + str( output.waveCount - 1 ) + "!" );
			}
			// set this as a more transparent way of checking the current time management option. X or y should be the same.
			currentPhaseManageOption = waveParams[Horizontal].phaseManagementOption;
			// make sure the times match.
			if (!(fabs( waveParams[Horizontal].time - waveParams[Vertical].time ) < 1e-6))
			{
				thrower( "ERROR: the horizontal and vertical waveforms must have the same time value. They appear to be mismatched for waveform #"
						 + str( output.waveCount - 1 ) + "!" );
			}
			/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///
			///					Handle -1 Phase (Use the phase that the previous waveform ended with)
			///
			// loop through both AXES.
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
						if (output.channel[axis].waveData[output.waveCount - 1].isVaried == true)
						{
							waveParams[Horizontal].isVaried = true;
							waveParams[Vertical].isVaried = true;
							waveParams[axis].varNum++;
							// the ' sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of 
							// varied waveforms gets understood properly.
							waveParams[axis].varNames.push_back( "\'" );
							waveParams[axis].varTypes.push_back( -1 );
							break;
						}
						if (count + 1 > output.channel[axis].waveData[output.waveCount - 1].signalNum)
						{
							thrower( "ERROR: You are trying to copy the phase of signal " + str( count + 1 ) + "  of " + AXES_NAMES[axis] 
									 + " waveform #" + str( output.waveCount - 1 ) + ", but the previous waveform only had "
									 + str( output.channel[axis].waveData[output.waveCount - 1].signalNum ) + " signals!\n" );
						}
						signal.initPhase = output.channel[axis].waveData[output.waveCount - 1].signals[count].finPhase;
					}
					count++;
				}
			}

			/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///
			///					Handle -1 Time Management (Bring the frequencies in the previous waveforms to 0 phase.
			///
			///

			/// Handle -1 Time Management for Both
			// Check if user used -1 for the time management of this waveform.
			if (currentPhaseManageOption == -1)
			{
				std::vector<double> initPhasesForTimeManage;
				// Make sure that both waveforms have the right number of signals.
				for (auto axis : AXES)
				{
					if (waveParams[axis].signalNum != output.channel[axis].waveData[output.waveCount - 1].signalNum)
					{
						thrower( "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
								 str( output.waveCount - 2 ) + " with waveform number" + str( output.waveCount - 1 ) + ", but the " +
								 AXES_NAMES[axis] + " component of the former has " + str( output.channel[axis].waveData[output.waveCount - 1].signalNum )
								 + " signals and the latter has " + str( waveParams[axis].signalNum ) + " signals. In order for a waveform "
								 "to correct the time of another waveform, the two must have the same number of signals." );
					}
					for (auto signal : range( output.channel[axis].waveData[output.waveCount - 1].signalNum ))
					{
						// Check...
						if (output.channel[axis].waveData[output.waveCount - 1].signals[signal].freqFin 
							 != waveParams[axis].signals[signal].freqInit)
						{
							// report error
							thrower( "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
									 + str( output.waveCount - 2 ) + " with waveform number" + str( output.waveCount - 1 ) + ", but signal "
									 + str( signal ) + " in the vertical component of the former has final frequency "
									 + str( output.channel[axis].waveData[output.waveCount - 1].signals[signal].freqFin ) + " and signal " 
									 + str( signal ) + " in the " + AXES_NAMES[axis] + " component of the latter has initial frequency of"
									 + str( waveParams[axis].signals[signal].freqInit ) + " signals. In order for a waveform to correct the "
									 "time of another waveform, these frequencies should match." );
						}
						initPhasesForTimeManage.push_back( waveParams[axis].signals[signal].initPhase );
					}
				}			
				if (output.channel[Vertical].waveData[output.waveCount - 1].isVaried == true 
					 || output.channel[Horizontal].waveData[output.waveCount - 1].isVaried == true)
				{
					for (auto axis : AXES)
					{
						waveParams[axis].isVaried = true;
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
			// only create waveform data if neither waveform is being varried and if the time management option is either 0 or -1. The time management  
			// option has already been checked to be the same for x and Y waveforms.
			for (auto axis : AXES)
			{
				if (waveParams[Vertical].varNum == 0 && waveParams[Horizontal].varNum == 0 && waveParams[axis].phaseManagementOption < 1)
				{
					// Initialize the giant waveform arrays.
					waveVoltages[axis] = new ViReal64[waveParams[axis].sampleNum];
					waveFileData[axis] = new ViReal64[waveParams[axis].sampleNum + waveParams[axis].signalNum];
					// either calculate or read waveform data into the above arrays. 
					waveformGen( waveVoltages[axis], waveFileData[axis], waveParams[axis], libWaveformArray, options, debugMsg );
					delete[] waveFileData[axis];
				}
				else if (waveParams[axis].varNum > 0)
				{
					// Mark this waveform as being varied. 
					waveParams[Horizontal].isVaried = true;
					waveParams[Vertical].isVaried = true;
				}
			}

			/// Handle +1 Time Management ////////////////////////////////////////////////////////////////////////////////////////////////
			// if not the first waveform
			if (output.waveCount != 0)
			{
				/// Handle +1 for vertical
				// check if user used +1 for the time of the previous waveform.
				for (auto axis : AXES)
				{
					if (output.channel[axis].waveData[output.waveCount - 1].phaseManagementOption == 1)
					{
						// Make sure the waveforms are compatible.
						std::vector<double> initPhasesForTimeManage;
						if (waveParams[axis].signalNum != output.channel[axis].waveData[output.waveCount - 1].signalNum)
						{
							thrower( "ERROR: Signal Number Mismatch! You appear to be attempting to correcting the phase of waveform number " +
									 str( output.waveCount - 1 ) + " with waveform number" + str( output.waveCount - 2 ) + ", but the vertical "
									 "component of the former has " + str( waveParams[axis].signalNum ) + " signals and the latter has " +
									 str( output.channel[axis].waveData[output.waveCount - 1].signalNum ) + " signals. In order for a waveform to "
									 "correct the time of another waveform, the two must have the same number of signals." );
						}
						// check to make sure that the X frequencies match
						for (int signal = 0; signal < output.channel[axis].waveData[output.waveCount - 1].signalNum; signal++)
						{
							if (output.channel[axis].waveData[output.waveCount - 1].signals[signal].freqInit 
								 != waveParams[axis].signals[signal].freqInit)
							{
								thrower( "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
										 + str( output.waveCount - 1 ) + " with waveform number" + str( output.waveCount - 2 ) +
										 ", but signal " + str( signal ) + " in the vertical component of the former has final frequency "
										 + str( waveParams[axis].signals[signal].freqInit ) + " and signal " + str( signal ) + " in the "
										 "vertical component of the latter has initial frequency of "
										 + str( output.channel[axis].waveData[output.waveCount - 1].signals[signal].freqFin ) + " signals. In order "
										 "for a waveform to correct the time of another waveform, these frequencies should match." );
							}
							initPhasesForTimeManage.push_back( waveParams[axis].signals[signal].finPhase 
															   + output.channel[axis].waveData[output.waveCount - 1].signals[signal].initPhase );
						}
						// check to make sure that the frequencies match
						for (int signal = 0; signal < output.channel[ALT_AXES[axis]].waveData[output.waveCount - 1].signalNum; signal++)
						{
							// check to make sure that the frequencies match
							if (output.channel[ALT_AXES[axis]].waveData[output.waveCount - 1].signals[signal].freqInit 
								 != waveParams[ALT_AXES[axis]].signals[signal].freqInit)
							{
								thrower( "ERROR: Frequency Mismatch! You appear to be attempting to correcting the phase of waveform number "
										 + str( output.waveCount - 1 ) + " with waveform number" + str( output.waveCount - 2 ) + ", but signal "
										 + str( signal ) + " in the " + AXES_NAMES[ALT_AXES[axis]] + " component of the former has final frequency "
										 + str( waveParams[ALT_AXES[axis]].signals[signal].freqInit ) + " and signal " + str( signal ) + " in the "
										 + AXES_NAMES[ALT_AXES[axis]] + " component of the latter has initial frequency of "
										 + str( output.channel[ALT_AXES[axis]].waveData[output.waveCount - 1].signals[signal].freqFin ) +
										 " signals. In order for a waveform to correct the time of another waveform, these frequencies should match." );
							}
							initPhasesForTimeManage.push_back( waveParams[ALT_AXES[axis]].signals[signal].finPhase
															   + output.channel[ALT_AXES[axis]].waveData[output.waveCount - 1].signals[signal].initPhase );
						}

						if (waveParams[axis].isVaried == true)
						{
							output.channel[Vertical].waveData[output.waveCount - 1].isVaried = true;
							output.channel[Horizontal].waveData[output.waveCount - 1].isVaried = true;
							output.channel[axis].waveData[output.waveCount - 1].varNum++;
							// the ' sign is reserved. Don't use it in a script file. It's just a place-holder here to make sure the number of varied waveforms gets
							// understood properly.
							output.channel[axis].waveData[output.waveCount - 1].varNames.push_back( "\'" );
							output.channel[axis].waveData[output.waveCount - 1].varTypes.push_back( -3 );
						}
						else
						{
							// calculate a time for the previous waveform.
							double errVal = myMath::calculateCorrectionTime( output.channel[axis].waveData[output.waveCount - 1],
																			 output.channel[ALT_AXES[axis]].waveData[output.waveCount - 1],
																			 initPhasesForTimeManage, "before" );
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
							
							prevVoltages[axis] = new ViReal64[output.channel[axis].waveData[output.waveCount - 1].sampleNum];
							prevFileData[axis] = new ViReal64[output.channel[axis].waveData[output.waveCount - 1].sampleNum 
															  + output.channel[axis].waveData[output.waveCount - 1].signalNum];
							// either calculate or read waveform data into the above arrays. 
							waveformGen( prevVoltages[axis], prevFileData[axis], output.channel[axis].waveData[output.waveCount - 1],
										 libWaveformArray, options, debugMsg );
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
				if (output.channel[Horizontal].waveData[output.waveCount - 1].phaseManagementOption == 1 
					 && output.channel[Vertical].waveData[output.waveCount - 1].phaseManagementOption == 1
					 && waveParams[Horizontal].isVaried == false && waveParams[Vertical].isVaried == false)
				{
					// Check for bad input
					if (output.channel[Vertical].waveData[output.waveCount - 1].sampleNum 
						 != output.channel[Horizontal].waveData[output.waveCount - 1].sampleNum)
					{
						thrower( "ERROR: the x and y waveforms must have the same time values option. They appear to be mismatched for waveform #"
								 + str( output.waveCount - 1 ) + "!" );
					}
					// Create the mixed waveform holder
					long int mixedSize = 2 * output.channel[Vertical].waveData[output.waveCount - 1].sampleNum;
					prevMixedVoltages = new ViReal64[mixedSize];
					// Mix the waveforms
					mixWaveforms( prevVoltages, prevMixedVoltages, output.channel[Vertical].waveData[output.waveCount - 1].sampleNum );
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
					delete[] prevFileData[Vertical];
					delete[] prevFileData[Horizontal];
					delete[] prevMixedVoltages;
				}

				// only create waveform data if neither waveform is being varried, and for this second time, if the previous wave was 
				// correcting this waveform such that the waveform needs to be recalculated.
				for (auto axis : AXES)
				{
					if (waveParams[Vertical].varNum == 0 && waveParams[Horizontal].varNum == 0
						 && waveParams[axis].phaseManagementOption < 1
						 && output.channel[axis].waveData[output.waveCount - 1].phaseManagementOption == 1)
					{
						// Initialize the giant waveform arrays.
						waveVoltages[axis] = new ViReal64[waveParams[axis].sampleNum];
						waveFileData[axis] = new ViReal64[waveParams[axis].sampleNum + waveParams[axis].signalNum];
						// either calculate or read waveform data into the above arrays. 
						waveformGen( waveVoltages[axis], waveFileData[axis], waveParams[axis], libWaveformArray, options, debugMsg );
						delete[] waveFileData[axis];
					}
					else if (waveParams[axis].varNum > 0)
					{
						// Mark this waveform as being varied.
						waveParams[axis].isVaried = true;
						waveParams[ALT_AXES[axis]].isVaried = true;
					}
				}
			}

			for (auto axis : AXES)
			{
				if (waveParams[axis].varNum >= 0)
				{
					output.channel[axis].waveData.push_back( waveParams[axis] );
				}
			}

			if (waveParams[Horizontal].varNum == 0 && waveParams[Vertical].varNum == 0 && waveParams[Vertical].phaseManagementOption < 1)
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
				mixWaveforms( waveVoltages, mixedWaveVoltages, waveParams[Vertical].sampleNum );
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					// these three functions are capable of throwing myException. analyzeNiawgScripts should always be in a try/catch.
					// create waveform (necessary?)
					createWaveform( mixedSize, mixedWaveVoltages );
					// allocate waveform into the device memory
					allocateWaveform( tempWaveformName, mixedSize / 2 );
					// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
					writeWaveform( tempWaveformName, mixedSize, mixedWaveVoltages );
				}
				// avoid memory leaks, but only if not default...
				if (output.isDefault)
				{
					if (profile.orientation == HORIZONTAL_ORIENTATION)
					{
						default_hConfigMixedWaveform = mixedWaveVoltages;
						default_hConfigMixedSize = mixedSize;
						default_hConfigWaveformName = tempWaveformName;
					}
					if (profile.orientation == VERTICAL_ORIENTATION)
					{
						default_vConfigMixedWaveform = mixedWaveVoltages;
						default_vConfigMixedSize = mixedSize;
						default_vConfigWaveformName = tempWaveformName;
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
					 "The two inputed types are: " + inputTypes[Vertical] + " and " + inputTypes[Horizontal]+ " for waveform #" 
					 + str( output.waveCount - 1) + "!" );
		}
		// end while !eof() loop
	}
}


void NiawgController::getVariedWaveform(waveData &varWvFmInfo, std::vector<waveData> all_X_Or_Y_WvFmParam, int waveOrderNum, 
										 std::vector<std::string>(&libWvFmArray)[MAX_NIAWG_SIGNALS * 4], ViReal64 * waveformRawData, 
										 debugInfo options, std::string& debugMsg)
{
	for ( auto signal : range( varWvFmInfo.signalNum ) )
	{
		// check if this is supposed to be overwritten during variable runs.
		for ( auto vartype : varWvFmInfo.varTypes )
		{
			if ( vartype == -1 )
			{
				if ( signal + 1 > all_X_Or_Y_WvFmParam[waveOrderNum - 1].signalNum )
				{
					thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 )
							 + "th signal of the previous waveform, but the previous waveform only had "
							 + str( all_X_Or_Y_WvFmParam[waveOrderNum - 1].signalNum ) + " signals!\n" );
				}
				varWvFmInfo.signals[signal].initPhase = all_X_Or_Y_WvFmParam[waveOrderNum - 1].signals[signal].finPhase;
			}
		}
	}
	ViReal64* waveReadData;
	waveReadData = new ViReal64[varWvFmInfo.sampleNum + varWvFmInfo.signalNum];
	// either calculate or read waveform data into the above arrays.
	waveformGen( waveformRawData, waveReadData, varWvFmInfo, libWvFmArray, options, debugMsg );
	delete[] waveReadData;
}

/*
	* This function is called when it is time to vary a parameter of a waveform description file before getting the data corresponding to the new waveform.
	* It is mostly a large case structure and has different cases for all of the different parameters you could be changing, as well as several cases for 
	* dealing with correction waveforms and all of their glory.
	* 
	* std::vector<waveData>& allWvInfo1: This is the primary vector of data to be changed. For most cases, an entry inside of this vector is changed and 
										allWvInfo2 is not used.
	* std::vector<waveData>& allWvInfo2: This is used in the case of correction waveforms as you need both waveform's information in order to calculate the 
										correction time.
	* int wfCount: The current waveform being handled. Used to get information from and assign information to the correct waveform in the variation sequence.
	* int& paramNum: A number which tells the function what to do with the data, signalInc.e. which value where to assign it to. It get's altered during one of the 
					correction waveform handling places.
	* double paramVal: the value of the variable which is getting loaded into waveforms.
	*/
void NiawgController::varyParam(std::vector<waveData>& allWvInfo1, std::vector<waveData>& allWvInfo2, int wfCount, int& paramNum,
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
			// Case -3 is the case initially set by a correction wavefunction set to correct the /next/ (as opposed to the previous) waveform. However, at the
			// time that this gets called, the wavefunction can't get the correct time yet because the next waveform hasn't been written. This location should
			// never be reached.
			thrower("BUG! Bad location reached: -3 case of varyParam().");
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
			// CONVERT from milliseconds...
			allWvInfo1[wfCount].time = paramVal * 0.001;
			// Changing the time changes the sample number, so recalculate that.
			allWvInfo1[wfCount].sampleNum = this->waveformSizeCalc( allWvInfo1[wfCount] );
			break;
		}
		// all the cases in-between 0 and 5 * MAX_NIAWG_SIGNALS + 1
		default:
		{
			int signalNum = paramNum / 5;
			switch ( paramNum % 5 + 1)
			{
				case 1:
				{
					if ( paramVal < 0 )
					{
						thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was" 
								 + str( paramVal ) );
					}
					allWvInfo1[wfCount].signals[signalNum].freqInit = paramVal;
					break;
				}
				case 2:
				{
					if ( paramVal < 0 )
					{
						thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was" + str( paramVal ) );
					}
					allWvInfo1[wfCount].signals[signalNum].freqFin = paramVal;
					break;
				}
				case 3:
				{
					if ( paramVal < 0 )
					{
						thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was"
								+ str( paramVal ) );
					}
					allWvInfo1[wfCount].signals[signalNum].initPower = paramVal;
					break;
				}
				case 4:
				{
					if ( paramVal < 0 )
					{
						thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was"
								+ str( paramVal ) );
					}
					allWvInfo1[wfCount].signals[signalNum].finPower = paramVal;
					break;
				}
				case 5:
				{
					allWvInfo1[wfCount].signals[signalNum].initPhase = paramVal;
					break;
				}
			}
		}
	}
}


void NiawgController::errChecker(int err)
{
	if (err < 0)
	{		
		thrower( "NIAWG Errorr: " + this->getErrorMsg() );
	}
	return;
}

/*
* various functions for interpreting the inputted script.
*/

/**
* getInputType takes in a string, the string representation of the input type, and returns a number directly corresponding to that input type.
* @param inputType is the string to be analyzed
* @return int is the number corresponding to the input type.
*/
void NiawgController::getInputType(std::string inputType, waveData &wvInfo)
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
* @param inputCase is the number associated with the input type, obtained from getInputType() above.
* @param script is the name of the file being read, which contains all of the waveform data on the next line.
* @param size is the size of the waveform in question, to be determined by this function.
* @param waveInfo is the structure which stores the info being read in this function.
*/
void NiawgController::getWaveData( ScriptStream &script, waveData &waveInfo, std::vector<variable> singletons )
{
	// Initialize the variable counter inside the waveData struct to zero:
	waveInfo.varNum = 0;
	waveInfo.signals.resize( waveInfo.signalNum );

	switch ( waveInfo.initType / MAX_NIAWG_SIGNALS )
	{
		/// the case for "gen ?, const"
		case 0:
		{
			for ( auto signal : range( waveInfo.signalNum ) )
			{
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				getParamCheckVarConst( waveInfo.signals[signal].freqInit, waveInfo.signals[signal].freqFin, script, waveInfo.varNum,   
									   waveInfo.varNames, waveInfo.varTypes, 5 * signal + 1, 5 * signal + 2, singletons );
				// Scale the frequencies to be in hertz. (input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin *= 1000000.;
				// Can't be varied for this case type
				waveInfo.signals[signal].freqRampType = "nr";
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				getParamCheckVarConst( waveInfo.signals[signal].initPower, waveInfo.signals[signal].finPower,
									   script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
									   5 * signal + 3, 5 * signal + 4, singletons );
				// Can't be varied
				waveInfo.signals[signal].powerRampType = "nr";
				// Get phase, unless varied.
				getParamCheckVar( waveInfo.signals[signal].initPhase, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 5, singletons );
			}
			// Get time, unless varied.
			getParamCheckVar( waveInfo.time, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
							  MAX_NIAWG_SIGNALS * 5 + 1, singletons );
			// Scale the time to be in seconds. (input is ms)
			waveInfo.time *= 0.001;
			// get the time management option.
			getParamCheckVar( waveInfo.phaseManagementOption, script, waveInfo.varNum, waveInfo.varNames,
							  waveInfo.varTypes, MAX_NIAWG_SIGNALS * 5 + 2, singletons );
			// pick up the delimeter.
			script >> waveInfo.delim;
			break;
		}
		/// The case for "gen ?, amp ramp"
		case 1:
		{
			for ( auto signal : range( waveInfo.signalNum ) )
			{
				// set the initial and final values to be equal, and to not use a ramp.
				getParamCheckVarConst( waveInfo.signals[signal].freqInit, waveInfo.signals[signal].freqFin,
									   script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
									   5 * signal + 1, 5 * signal + 2, singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin *= 1000000.;
				// can't be varried.
				waveInfo.signals[signal].freqRampType = "nr";
				std::string tempStr;
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower );
				waveInfo.signals[signal].powerRampType = tempStr;
				getParamCheckVar( waveInfo.signals[signal].initPower, script, waveInfo.varNum,
								  waveInfo.varNames, waveInfo.varTypes, 5 * signal + 3, singletons );
				getParamCheckVar( waveInfo.signals[signal].finPower, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 4, singletons );
				getParamCheckVar( waveInfo.signals[signal].initPhase, script, waveInfo.varNum,
								  waveInfo.varNames, waveInfo.varTypes, 5 * signal + 5,
								  singletons );
			}
			getParamCheckVar( waveInfo.time, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
							  MAX_NIAWG_SIGNALS * 5 + 1, singletons );
			// Scale the time to be in seconds. (input is ms)
			waveInfo.time *= 0.001;
			// get the time management option.
			getParamCheckVar( waveInfo.phaseManagementOption, script, waveInfo.varNum, waveInfo.varNames,
							  waveInfo.varTypes, MAX_NIAWG_SIGNALS * 5 + 2, singletons );
			// pick up the delimeter.
			script >> waveInfo.delim;
			break;
		}
		/// The case for "gen ?, freq ramp"
		case 2:
		{
			for ( auto signal : range( waveInfo.signalNum ) )
			{
				// get all parameters from the file
				std::string tempStr;
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), tolower );
				waveInfo.signals[signal].freqRampType = tempStr;
				getParamCheckVar( waveInfo.signals[signal].freqInit, script, waveInfo.varNum, waveInfo.varNames, 
								  waveInfo.varTypes, 5 * signal + 1, singletons );
				getParamCheckVar( waveInfo.signals[signal].freqFin, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 2, singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin *= 1000000.;
				// set the initial and final values to be equal, and to not use a ramp.
				getParamCheckVarConst( waveInfo.signals[signal].initPower, waveInfo.signals[signal].finPower,
									   script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
									   5 * signal + 3, 5 * signal + 4, singletons );
				waveInfo.signals[signal].powerRampType = "nr";
				getParamCheckVar( waveInfo.signals[signal].initPhase, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 5, singletons );
			}
			getParamCheckVar( waveInfo.time, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
							  MAX_NIAWG_SIGNALS * 5 + 1, singletons );
			// Scale the time to be in seconds. (input is ms)
			waveInfo.time *= 0.001;
			// get the time management option.
			getParamCheckVar( waveInfo.phaseManagementOption, script, waveInfo.varNum, waveInfo.varNames,
							  waveInfo.varTypes, MAX_NIAWG_SIGNALS * 5 + 2, singletons );
			// pick up the delimeter.
			script >> waveInfo.delim;
			break;
		}
			/// The case for "gen ?, freq & amp ramp"
		case 3:
		{
			for ( int signal = 0; signal < waveInfo.signalNum; signal++ )
			{
				// get all parameters from the file
				std::string tempStr;
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower );
				waveInfo.signals[signal].freqRampType = tempStr;
				getParamCheckVar( waveInfo.signals[signal].freqInit, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 1, singletons );
				getParamCheckVar( waveInfo.signals[signal].freqFin, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 2, singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				waveInfo.signals[signal].freqInit *= 1000000.;
				waveInfo.signals[signal].freqFin *= 1000000.;

				// get all parameters from the file
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower );
				waveInfo.signals[signal].powerRampType = tempStr;
				getParamCheckVar( waveInfo.signals[signal].initPower, script, waveInfo.varNum,
								  waveInfo.varNames, waveInfo.varTypes, 5 * signal + 3, singletons );
				getParamCheckVar( waveInfo.signals[signal].finPower, script, waveInfo.varNum, waveInfo.varNames,
								  waveInfo.varTypes, 5 * signal + 4, singletons );
				getParamCheckVar( waveInfo.signals[signal].initPhase, script, waveInfo.varNum,
								  waveInfo.varNames, waveInfo.varTypes, 5 * signal + 5, singletons );

				//?????????????????????????????????????????????????????????????????????????????
				//waveInfo.signals.push_back(waveInfo.signals[signalInc]); // Why is this here???????????
				//?????????????????????????????????????????????????????????????????????????????
			}
			getParamCheckVar( waveInfo.time, script, waveInfo.varNum, waveInfo.varNames, waveInfo.varTypes,
							  MAX_NIAWG_SIGNALS * 5 + 2, singletons );
			// Scale the time to be in seconds. (input is ms)
			waveInfo.time *= 0.001;
			// get the time management option.
			getParamCheckVar( waveInfo.phaseManagementOption, script, waveInfo.varNum, waveInfo.varNames,
							  waveInfo.varTypes, MAX_NIAWG_SIGNALS * 5 + 2, singletons );
			// pick up the delimeter.
			script >> waveInfo.delim;
			break;
		}
	}
	// Also get the size (all cases)
	waveInfo.sampleNum = waveformSizeCalc( waveInfo );
	return;
}


// load all values for libWaveformArray.
void NiawgController::openWaveformFiles( std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4] )
{
	std::string tempStr;
	std::fstream libNameFile;
	for (int folderInc = 0; folderInc < WAVEFORM_TYPE_FOLDERS->size(); folderInc++)
	{
		// open the file. It's written in binary.
		std::string libNameFilePath = LIB_PATH + WAVEFORM_TYPE_FOLDERS[folderInc] + WAVEFORM_NAME_FILES[folderInc];
		libNameFile.open( libNameFilePath.c_str(), std::ios::binary | std::ios::in );
		// Make sure that the file opened correctly.
		if (!libNameFile.is_open())
		{
			thrower( "ERROR: waveform library file did not open correctly. Name was " + libNameFilePath );
		}
		// read all of the waveforms into libWaveformArray
		int waveInfoInc = 0;
		// if not empty, the first line will just have a newline on it, so there is no harm in getting rid of it.
		std::getline( libNameFile, tempStr, '\n' );
		while (!libNameFile.eof())
		{
			// read waveform names...
			std::getline( libNameFile, tempStr, '\n' );
			// put them into the array...
			libWaveformArray[folderInc].push_back( tempStr );
			waveInfoInc++;
		}
		// save the fileOpened value reflecting it's new status.
		libNameFile.close();
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
* @return void This function does not return anything currently. This may be changed at some point to include error information in the return value.
*
* @param tempWaveform this is the array that will carry the waveform information out of the function into the rest of the program to be written to the AWG.
*
* @param readData this is the array that carries the information as it is first written / read from the file, and is slightly longer than waveformData to
* include the phase data.
*
* @param waveInfo this is the parameter that carries the basic information about the waveform, and is used in the writing of the waveforms name (for the
* library) and for writing the raw data if needed.
*
* @param size this is the number of samples for the waveform, calculated previously. This is needed to know how many data points to read into the data
* arrays.
*
* @param libWaveformArray this is an array of vectors which contain strings. Each vector corresponds to each of the input cases (hence size 20), and each
* string in the vector corresponds to the name of some waveform which has previously been written. This list of previously written waveforms is used to
* tell if a waveform has been written before.
*
* @param fileOpened this carries information that tells if the file correspoinding to the inputCase in question has been opened, and the libWaveformArray
* strings read, before. If it hasn't the code knows to open the file and read in the strings for the first time.
*/
void NiawgController::waveformGen( ViReal64 * & tempWaveform, ViReal64 * & readData, waveData & waveInfo, 
								   std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4], debugInfo options, 
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
	for ( unsigned int fileInc = 0; fileInc < libWaveformArray[waveInfo.initType].size(); fileInc++ )
	{
		// if you find this waveform to have already been written...
		if ( libWaveformArray[waveInfo.initType][fileInc] == waveformFileSpecs )
		{
			// Construct the file address
			std::string waveFileReadName = LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + str( waveInfo.initType ) + "_"
				+ str( fileInc ) + ".txt";
			waveformFileRead.open( waveFileReadName, std::ios::binary | std::ios::in );
			// read the file. size + signalnum because of the phase data on the end.
			waveformFileRead.read( (char *) readData, (waveInfo.sampleNum + waveInfo.signalNum) * sizeof( ViReal64 ) );
			// grab the phase data off of the end.
			for ( auto signal : range( waveInfo.signalNum ) )
			{
				waveInfo.signals[signal].finPhase = readData[waveInfo.sampleNum + signal];
			}
			// put the relevant voltage data into a the new array.
			std::copy_n( readData, waveInfo.sampleNum, tempWaveform );
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
						 + str( libWaveformArray[waveInfo.initType].size() ) + ".txt");
	// open file for writing.
	waveformFileWrite.open( waveformFileName, std::ios::binary | std::ios::out );
	// make sure it opened.
	if ( !waveformFileWrite.is_open() )
	{
		// shouldn't ever happen.
		thrower( "ERROR: Data Storage File could not open. File name is probably too long? File name is " + waveformFileName );
	}
	else
	{
		// start timer.
		unsigned long long time1 = GetTickCount64();
		// calculate all voltage values and final phases and store them in the readData variable.
		calcWaveData( waveInfo, readData );
		// Write the data, with phases, to the write file.
		waveformFileWrite.write( (const char *) readData, (waveInfo.sampleNum + waveInfo.signalNum) * sizeof( ViReal64 ) );
		waveformFileWrite.close();
		// put the relevant data into another string.
		std::copy_n( readData, waveInfo.sampleNum, tempWaveform );
		// write the newly written waveform's name to the library file.
		libNameFile.open( LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + WAVEFORM_NAME_FILES[waveInfo.initType],
						  std::ios::binary | std::ios::out | std::ios::app );
		if ( !libNameFile.is_open() )
		{
			thrower("ERROR! waveform name file not opening correctly.\n");
		}
		// add the waveform name to the current list of strings. do it BEFORE adding the newline T.T
		libWaveformArray[waveInfo.initType].push_back( waveformFileSpecs.c_str() );
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
* @return int this carries error information.
*
* @param verticalFile this is the name of the instructions file for the x-waveforms.
* @param horizontalFile this is the name of the instructions file for the y-waveforms.
* @param verticalInput this is the actual command name recently grabbed from the verticalFile.
* @param horizontalInput this is the actual command name recently grabbed from the horizontalFile.
* @param scriptString this is the string that contains all of the final script. It gets written to.
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
		scriptString += "wait until " + std::string( this->TRIGGER_NAME ) + "\n";
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
		scriptString += "repeat until " + std::string( TRIGGER_NAME ) + "\n";
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
		scriptString += "if " + std::string( TRIGGER_NAME ) + "\n";
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
*
* @return int this is for error information.
*
* @param verticalFile this is the instructiosn file which carries commands for the x waveforms.
* @param horizontalFile this is the instructions file which carries commands for the y waveforms.
* @param verticalInputType this contains the command the user inputted for the x waveforms, which was just read from the verticalFile recently.
* @param horizontalInputType this contains the command the user inputted for the y waveforms, which was just read from the horizontalFile recently.
* @param scriptString this carries the final script to be outputted to the AWG.
* @param triggerName this carries the string version of the trigger name.
* @param waveCount this integer is the number of waveforms that have been written so far.
* @param vi is the session handle currently in use for communication with the AWG.
* @param channelName is the channel name ("0,1" normally) that the signal is currently being put through.
* @param error is the NI fgen error variable which grabs error messages.
* @param verticalWaveformList is the list of all of the previously written x-waveforms.
* @param horizontalWaveformList is the list of all of the previously written y-waveforms.
* @param predWaveCount is the number of predefined waves (signalInc.e. referring to a file that contains the waveform parameters) that have been used so far.
* @param waveListWaveCounts is the array that tells which waveforms (signalInc.e. waveform0, waveform1, the names used for the final script) the predefined
* waveforms refer to.
* @param libWaveformArray is the array of vectors to strings that contain all of the names of the waveforms currently in the library.
* @param fileStatus is the array that contains the information as to whether each library file has been opened yet this run or not.
*/
void NiawgController::special( niawgPair<ScriptStream>& scripts, outputInfo& output, niawgPair<std::string> inputTypes, 
							   std::vector<std::string>( &libWaveformArray )[MAX_NIAWG_SIGNALS * 4], profileSettings profile, 
							   std::vector<variable> singletons, debugInfo options, std::string& warnings, std::string& debugMessages )
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

		analyzeNiawgScripts( externalScripts, output, libWaveformArray, profile, singletons, options, warnings, debugMessages );
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
			if (externalWaveformNames[Vertical][Vertical] == output.channel[Vertical].predefinedWaveNames[predefinedWaveformInc]) 
			{
				if (externalWaveformNames[Vertical][Horizontal] == output.channel[Horizontal].predefinedWaveNames[predefinedWaveformInc])
				{
					output.niawgLanguageScript += "generate Waveform" + str(output.predefinedWaveLocs[predefinedWaveformInc]) + "\n";
				}
			}
		}
		// add the name of the waveform to the list.
		output.channel[Vertical].predefinedWaveNames.push_back( externalWaveformNames[Vertical][Vertical] );
		output.channel[Horizontal].predefinedWaveNames.push_back( externalWaveformNames[Vertical][Horizontal] );
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
		analyzeNiawgScripts( externalWaveformStreams, output, libWaveformArray, profile, singletons, options, warnings, debugMessages );
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
*
* @return ViReal64* this is the pointer to the array that contains all of the now calculated data points.
*
* @param inputData is the data structure that contains the general parameters of the wavefunction.
* @param readData is the array which will hold all of the waveform and phase data.
* @param waveformSize is the size of the waveform to be written in number of samples.
*/
ViReal64* NiawgController::calcWaveData(waveData& inputData, ViReal64*& readData)
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
	/// ////////////////////////////////////////
	///
	///		Get Data Points.
	///
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
				/// modify here for calibrations!
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
		/// Calculate Phase Position. See above for description.
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
* @param waveform1 the first waveform (usually x)
* @param waveform2 the second waveform (usually y)
* @param finalWaveform the larger (2X) array which enters unfilled and exits with the interweaved data
* @param waveformSize the size in samples of one of the original arrays (they should be the same size).
*/
ViReal64* NiawgController::mixWaveforms( niawgPair<ViReal64*> waveforms, ViReal64* finalWaveform, long int waveformSize )
{
	for (auto sample : range(waveformSize))
	{
		// the order (Vertical -> Horizontal) here is important. Vertical is first because it's port zero on the Niawg. I believe that
		// switching the order here and changing nothing else would flip the output of the niawg..
		finalWaveform[2 * sample] = waveforms[Vertical][sample];
		finalWaveform[2 * sample + 1] = waveforms[Horizontal][sample];
	}
	return finalWaveform;
}

/**
* this function takes in a command and checks it against all "logic commands", returing true if the inputted command matches a logic command and false
* otherwise
* @param command the command being checked for being logic or not.
* @return bool the truth value to the statement "command is a logic command"
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
* this function takes in a command and checks it against all "generate commands", returing true if the inputted command matches a generate command and false
* otherwise
* @param command the command being checked for being a generate command or not.
* @return bool the truth value to the statement "command is a generate command"
*/
bool NiawgController::isGenCommand(std::string inputType)
{
	for ( auto number : range( MAX_NIAWG_SIGNALS ) )
	{
		if ( inputType == "gen " + str( number ) + ", const" )
		{
			return true;
		}
		else if ( inputType == "gen " + str( number ) + ", amp ramp " )
		{
			return true;
		}
		else if ( inputType == "gen " + str( number ) + ", freq ramp " )
		{
			return true;
		}
		else if ( inputType == "gen " + str( number ) + ", freq & amp ramp " )
		{
			return true;
		}
	}
	return false;
}

/**
	* this function takes in a command and checks it against all "special commands", returing true if the inputted command matches a special command and false
	* otherwise
	* @param command the command being checked for being special or not.
	* @return bool the truth value to the statement "command is a special command"
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


/**
	* Overload for double input.
	* Test if the input is a variable. If it was, store what parameter is getting varied (varParamTypes), store which variable name is associated
	* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
	* function using this function to check the arrays that get returned for the specified number of variables.
	*
	* @param dataToAssign is the value of a waveData variable that is being read in. If a variable is found it isn't actually assigned.
	* @param file is the instructions file being read for this input.
	* @param vCount is the number of variables that have been assigned so far.
	* @param vNames is a vector that holds the names of the variables that are found in the script.
	* @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
	* @param dataType is the number to assign to vParamTypes if a variable is being used.
	*/
void NiawgController::getParamCheckVar(double& dataToAssign, ScriptStream& fName, int& vCount, std::vector<std::string>& vNames, std::vector<int>& vParamTypes,
						int dataType, std::vector<variable> singletons)
{
	std::string tempInput;
	int stringPos;
	fName >> tempInput;
	if (tempInput[0] == '\'')
	{
		thrower("ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n");
	}
	if (tempInput[0] == '#')
	{
		thrower("ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
				"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
	}
	if (tempInput[0] == '%')
	{
		thrower("ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
	}
	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '-' || tempInput[0] == '.')
	{
		stringPos = 1;
		if (tempInput[1] == '-' || tempInput[1] == '.')
		{
			thrower("ERROR: The first two characters of some input are both either '-' or '.'. This might be because you tried to input a negative"
								"decimal, which you aren't allowed to do.");
			return;
		}
	}
	else 
	{
		stringPos = 0;
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			dataToAssign = singletons[singletonInc].value;
			return;
		}
	}
	if (!isdigit(tempInput[stringPos]))
	{
		// load variable name into structure.
		vNames.push_back(tempInput);
		vParamTypes.push_back(dataType);
		vCount++;
	}
	else
	{
		// this should happen most of the time.
		dataToAssign = (std::stod(tempInput));
	}
	return;
}


/**
	* overload for integer input.
	* Test if the input is a variable. If it was, store what parameter is getting varied (varParamTypes), store which variable name is associated
	* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
	* function using this function to check the arrays that get returned for the specified number of variables.
	*
	* @param dataToAssign is the value of a waveData variable that is being read in. If a variable is found it isn't actually assigned.
	* @param file is the instructions file being read for this input.
	* @param vCount is the number of variables that have been assigned so far.
	* @param vNames is a vector that holds the names of the variables that are found in the script.
	* @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
	* @param dataType is the number to assign to vParamTypes if a variable is being used.
	*/
void NiawgController::getParamCheckVar(int& dataToAssign, ScriptStream& scriptName, int& vCount, std::vector<std::string>& vNames, std::vector<int>& vParamTypes,
										int dataType, std::vector<variable> singletons)
{
	std::string tempInput;
	int stringPos;
	scriptName >> tempInput;
	if (tempInput[0] == '\'') 
	{
		thrower("ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n");
	}
	if (tempInput[0] == '#') 
	{
		thrower("ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
							"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
	}
	if (tempInput[0] == '%') 
	{
		thrower("ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
	}
	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '-' || tempInput[0] == '.') 
	{
		stringPos = 1;
		if (tempInput[1] == '-' || tempInput[1] == '.') 
		{
			thrower("ERROR: The first two characters of some input are both either '-' or '.'. This might be because you tried to input a negative"
								"decimal, which you aren't allowed to do.");
		}
	}
	else 
	{
		stringPos = 0;
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			dataToAssign = singletons[singletonInc].value;
		}
	}

	if (!isdigit(tempInput[stringPos])) 
	{
		// load variable name into structure.
		vNames.push_back(tempInput);
		vParamTypes.push_back(dataType);
		vCount++;
	}
	else 
	{
		// this should happen most of the time.
		dataToAssign = (std::stoi(tempInput));
	}
	return;
}


/**
	* Overload for double input.
	* Test if the input is a variable. If it was, store what parameter is getting varied (vParamTypes), store which variable name is associated
	* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
	* function using this function to check the arrays that get returned for the specified number of variables. This function deals with constant waveforms,
	* where either the frequency or the ramp is not being ramped, and so the inputted data needs to be assigned to both the initial and final values of the
	* parameter type.
	*
	* data1ToAssign is the value of the first waveData variable that is being read in. If a variable is found it isn't actually assigned.
	* data2ToAssign is the value of the second waveData variable that is being read in. If a variable is found it isn't actually assigned.
	* file is the instructions file being read for this input.
	* vCount is the number of variables that have been assigned so far.
	* vNames is a vector that holds the names of the variables that are found in the script.
	* vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
	* dataType1 is the number to assign to vParamTypes for the first parameter if a variable is being used.
	* dataType2 is the number to assign to vParamTypes for the second parameter if a variable is being used.
	* singletons is the list of singletons that the user set for this configuration. If a variable name is found to match a singleton name, the value is 
	*		immediately set to the singleton's value.
	*/
void NiawgController::getParamCheckVarConst(double& data1ToAssign, double& data2ToAssign, ScriptStream& file, int& vCount, 
											 std::vector<std::string>& vNames, std::vector<int>& vParamTypes, int dataType1, int dataType2,
											 std::vector<variable> singletons)
{
	std::string tempInput;
	file >> tempInput;
	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '\'')
	{
		thrower("ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.");
	}
	if (tempInput[0] == '#')
	{
		thrower("ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
			"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
	}
	if (tempInput[0] == '%')
	{
		thrower("ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
	}
	if (tempInput[0] == '-')
	{
		thrower("ERROR: it appears that you entered a negative frequency or amplitude. You can't do that.");
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			data1ToAssign = singletons[singletonInc].value;
			data2ToAssign = data1ToAssign;
			return;
		}
	}
	// I don't need to check for -1 input because this function should never be used on the phase or the time, only frequency or amplitude for non-ramping waveforms. 
	if (!isdigit(tempInput[0]))
	{
		// add variable name
		vNames.push_back(tempInput);
		vParamTypes.push_back(dataType1);
		vCount++;
		// Do the same for the second data that needs to be assigned.
		vNames.push_back(tempInput);
		vParamTypes.push_back(dataType2);
		vCount++;
	}
	else
	{
		// this should happen most of the time.
		data1ToAssign = std::stod(tempInput);
		data2ToAssign = data1ToAssign;
	}
	return;
}


/**
	* Overload for integer input.
	* Test if the input is a variable. If it was, store what parameter is getting varied (vParamTypes), store which variable name is associated
	* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
	* function using this function to check the arrays that get returned for the specified number of variables. This function deals with constant waveforms,
	* where either the frequency or the ramp is not being ramped, and so the inputted data needs to be assigned to both the initial and final values of the
	* parameter type.
	*
	* @param data1ToAssign is the value of the first waveData variable that is being read in. If a variable is found it isn't actually assigned.
	* @param data2ToAssign is the value of the second waveData variable that is being read in. If a variable is found it isn't actually assigned.
	* @param file is the instructions file being read for this input.
	* @param vCount is the number of variables that have been assigned so far.
	* @param vNames is a vector that holds the names of the variables that are found in the script.
	* @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
	* @param dataType1 is the number to assign to vParamTypes for the first parameter if a variable is being used.
	* @param dataType2 is the number to assign to vParamTypes for the second parameter if a variable is being used.
	*/
void NiawgController::getParamCheckVarConst(int& data1ToAssign, double& data2ToAssign, ScriptStream& scriptName, int& vCount, 
											 std::vector<std::string>& vNames, std::vector<int>& vParamTypes, int dataType1, int dataType2,
											 std::vector<variable> singletons)
{
	std::string tempInput;
	scriptName >> tempInput;
	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '\'')
	{
		thrower("ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n");
	}
	if (tempInput[0] == '#')
	{
		thrower("ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
			"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
	}
	if (tempInput[0] == '%')
	{
		thrower("ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
	}
	if (tempInput[0] == '-')
	{
		thrower("ERROR: it appears that you entered a negative frequency or amplitude. You can't do that.");
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			data1ToAssign = singletons[singletonInc].value;
			data2ToAssign = data1ToAssign;
			return;
		}
	}
	// I don't need to check for -1 input because this function should never be used on the phase or the time, only frequency or amplitude for non-ramping waveforms. 
	if (!isdigit(tempInput[0]))
	{
		// add variable name
		vNames.push_back(tempInput);
		vParamTypes.push_back(dataType1);
		vCount++;
		// Do the same for the second data that needs to be assigned.
		vNames.push_back(tempInput);
		vParamTypes.push_back(dataType2);
		vCount++;
	}
	else
	{
		// this should happen most of the time.
		data1ToAssign = std::stoi(tempInput);
		data2ToAssign = data1ToAssign;
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


// create waveform (necessary?)
// allocate waveform into the device memory
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


void NiawgController::configureDigtalEdgeScriptTrigger()
{
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		errChecker( niFgen_ConfigureDigitalEdgeScriptTrigger( sessionHandle, TRIGGER_NAME, TRIGGER_SOURCE, TRIGGER_EDGE_TYPE ) );
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

