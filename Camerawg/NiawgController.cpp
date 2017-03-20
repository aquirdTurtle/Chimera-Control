#include "stdafx.h"

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
	debugInfo debug;
	debug.outputAgilentScript = false;
	std::vector<variable> noSingletons;
	std::string warnings;
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
			profile.orientation = ORIENTATION[configAxis];
			mainWin->setOrientation( ORIENTATION[configAxis] );
			analyzeNiawgScripts( scripts, output, waveLibrary, profile, noSingletons, debug, warnings );
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
	if (debug.message != "")
	{
		errBox( "Debug messages detected during initial default waveform script analysis: " + debug.message );
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
			if (defaultOrientation == ORIENTATION[axis])
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
	defaultOrientation = orientation;
}


/**
  * The purpose of this function is to systematically read the input instructions files, create and read waveforms associated with
  * them, write the script as it goes, and eventually combine the x and y-waveforms into their final form for being sent to the waveform 
  * generator.
  */
void NiawgController::analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, outputInfo& output, library &waveLibrary, 
										   profileSettings profile, std::vector<variable> singletons, debugInfo& options, 
										   std::string& warnings )
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
	// open up the files and check what I have stored.
	openWaveformFiles( waveLibrary );
	// get the first input
	for (auto axis : AXES)
	{
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

	/// Analyze!
	while (!(scripts[Vertical].peek() == EOF) && !(scripts[Horizontal].peek() == EOF))
	{
		if (isLogic( command[Horizontal] ) && isLogic( command[Vertical] ))
		{
			handleLogic( scripts, command, output.niawgLanguageScript );
		}
		else if (isSpecialCommand( command[Horizontal] ) && isSpecialCommand( command[Vertical] ))
		{
			handleSpecial( scripts, output, command, waveLibrary, profile, singletons, options, warnings );
		}
		else if (isStandardWaveform( command[Horizontal] ) && isStandardWaveform( command[Vertical] ))
		{
			handleStandardWaveform( output, profile, command, scripts, singletons, waveLibrary, options);
		}
		else if (isSpecialWaveform( command[Horizontal] ) && isSpecialWaveform( command[Vertical] ))
		{
			handleSpecialWaveform( output, profile, command, scripts, singletons, waveLibrary, options );
		}
		else
		{
			thrower( "ERROR: Input types from the two files do not match or are unrecofgnized!\nBoth must be logic commands, both must be "
					 "generate commands, or both must be special commands. See documentation on the correct format for these commands.\n\n"
					 "The two inputted types are: " + command[Vertical] + " and " + command[Horizontal]+ " for waveform #" 
					 + str( output.waveCount - 1) + "!" );
		}
		// get next input.
		for (auto axis : AXES)
		{
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
	}
}

/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NiawgController::handleVariations( outputInfo& output, std::vector<variable>& variables, std::vector<std::vector<double>> varValues, 
										size_t& variation, std::vector<long> mixedWaveSizes, std::vector<ViReal64 *> mixedWaves,
										std::string& warnings, library& waveLibrary, debugInfo& debugOptions )
{				
	niawgPair<std::vector<ViReal64 *>> variedWaves;
	int mixedCount = 0;
	// I think waveInc = 0 & 1 are always the default.. should I be handling that at all??? shouldn't make a difference I don't think. 
	for (int waveInc = 0; waveInc < output.waveCount; waveInc++)
	{
		// currently both axes always vary together and get rewritten together. This could be modified to be smarter for a slightly more 
		// efficient programming sequence between variations. //
		if (output.chan[Vertical].waves[waveInc].varies && output.chan[Horizontal].waves[waveInc].varies)
		{
			// For all Variables...
			for (std::size_t variableInc = 0; variableInc < variables.size(); variableInc++)
			{
				std::string currentVar = variables[variableInc].name;
				std::string warnings;		
				for (auto axis : AXES)
				{
					/// Loop for varibles in given Waveform
					for (int varNumber = 0; varNumber < output.chan[axis].waves[waveInc].varNum; varNumber++)
					{
						// if parameter's variable matches current variable...
						if (output.chan[axis].waves[waveInc].varNames[varNumber] == currentVar)
						{
							// change parameters, depending on the case.
							varyParam( output.chan[axis].waves, output.chan[ALT_AXES[axis]].waves, waveInc, 
									   output.chan[axis].waves[waveInc].varTypes[varNumber], varValues[variableInc][variation],
									   warnings );
						} 
					} 
				} 
			} 
			// If the user used a '-1' for the initial phase, this is code for "copy the ending phase of the previous waveform". 
			for (auto& axis : output.chan)
			{
				for (int signal = 0; signal < axis.waves[waveInc].signals.size(); signal++)
				{
					if (axis.waves[waveInc].signals[signal].initPhase == -1)
					{
						if (signal + 1 > axis.waves[waveInc - 1].signals.size())
						{
							thrower( "ERROR: You are trying to copy the phase of the " + std::to_string( signal + 1 )
									 + " signal of the previous waveform, but the previous waveform only had "
									 + std::to_string( axis.waves[waveInc - 1].signals.size() ) + " signals!\r\n" );
						}
						axis.waves[waveInc].signals[signal].initPhase = axis.waves[waveInc - 1].signals[signal].finPhase;
					}
				}
			}
			/// write the new waveform
			// If the correction flag is set, that means that the previous waveform was already passed by, and that I need to address a particular
			// entry of the data, not the next entry.
			for (auto axis : AXES)
			{
				variedWaves[axis].push_back( new ViReal64[output.chan[axis].waves[waveInc].sampleNum] );
				getVariedWaveform( output.chan[axis].waves[waveInc], output.chan[axis].waves, waveInc, waveLibrary, 
								   variedWaves[axis][mixedCount], debugOptions );
			}
			// keep in mind that the vertical and horizontal sample numbers should be the same (and this should have been verified)
			// at this point
			mixedWaveSizes.push_back( 2 * output.chan[Vertical].waves[waveInc].sampleNum );
			mixedWaves.push_back( new ViReal64[mixedWaveSizes[mixedCount]] );
			
			mixWaveforms( variedWaves[Vertical][mixedCount],variedWaves[Horizontal][mixedCount], mixedWaves[mixedCount], 
						  output.chan[Vertical].waves[waveInc].sampleNum );
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


void NiawgController::getVariables( SocketWrapper& socket, std::vector<std::vector<double>>& varValues, std::vector<variable> variables )
{
	std::vector<long> varLengths;
	/// get var files from master if necessary
	{
		varValues.resize( variables.size(), std::vector<double>( 0 ) );
		varLengths.resize( variables.size() );
		std::string recievedMsg;
		for (int variableNameInc = 0; variableNameInc < variables.size(); variableNameInc++)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				socket.send( "next variable" );
				recievedMsg = socket.recieve();
			}
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
					tempDouble = stod( tempString );
					// if it's a time value, check to make sure it's valid.
					if (variables[varNameCursor].timelike)
					{
						if (waveformSizeCalc( tempDouble ) % 4 != 0)
						{
							thrower( "ERROR: a timelike value sent by the master computer did not correspond to an integer number of 4 samples. The "
									 "value was " + std::to_string( tempDouble ) + "\r\n" );
						}
					}
					varValues[varNameCursor].resize( j + 1 );
					varValues[varNameCursor][j] = tempDouble;
					j++;
				}
				varLengths[varNameCursor] = j;
			}
			else if (TWEEZER_COMPUTER_SAFEMODE)
			{
				// set all values to zero. This should work for the majority of variables possible.
				varValues[variableNameInc].resize( 3 );
				varValues[variableNameInc][0] = 0;
				varValues[variableNameInc][1] = 0;
				varValues[variableNameInc][2] = 0;
				varLengths[variableNameInc] = 3;
			}
		}
	}
	for (int varCount = 0; varCount < varValues.size(); varCount++)
	{
		if (varValues[varCount].size() != varLengths[varCount])
		{
			thrower( "ERROR: VarLengths doesn't match size of varValues! This is a lower level logic error, talk to Mark." );
		}
	}

	// check that variable files are the same length.
	for (std::size_t varNameInc = 0; varNameInc + 1 < varValues.size(); varNameInc++)
	{
		if (varValues[varNameInc].size() != varValues[varNameInc + 1].size())
		{
			thrower( "Error: lengths of variable values are not all the same! They must be the same\r\n" );
		}
	}
}


void NiawgController::getVariedWaveform(waveInfo &varWaveInfo, std::vector<waveInfo> waveParams, int waveOrderNum, library &waveLibrary, 
										 ViReal64 * waveformRawData, debugInfo& options)
{
	// check phase correction.
	for ( auto signal : range( varWaveInfo.signals.size() ) )
	{
		// check if this is supposed to be overwritten during variable runs.
		for ( auto vartype : varWaveInfo.varTypes )
		{
			if ( vartype == -1 )
			{
				if ( signal + 1 > waveParams[waveOrderNum - 1].signals.size())
				{
					thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 ) + "th signal of the previous waveform, "
							 "but the previous waveform only had " + str( waveParams[waveOrderNum - 1].signals.size() ) + " signals!\n" );
				}
				varWaveInfo.signals[signal].initPhase = waveParams[waveOrderNum - 1].signals[signal].finPhase;
			}
		}
	}
	// either calculate or read waveform data into the above arrays.
	generateWaveform( waveformRawData, varWaveInfo, waveLibrary, options );
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
		case -1:
		{
			// This is the -1 option for phase control when the previous waveform is being varied.
			for (auto signal : range(allWvInfo1[wfCount].signals.size() ) )
			{
				// check if this is supposed to be overwritten during variable runs.
				for (auto varType : allWvInfo1[wfCount].varTypes )
				{
					if (varType == -1) 
					{
						if (signal + 1 > allWvInfo1[wfCount - 1].signals.size())
						{
							thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 ) + "the signal of the previous "
									 "waveform, but the previous waveform only had " + str( allWvInfo1[wfCount - 1].signals.size() )
									 + " signals!\n)" );
						}
						allWvInfo1[wfCount].signals[signal].initPhase = allWvInfo1[wfCount - 1].signals[signal].finPhase;
					}
				}
			}
			for (auto signal : range( allWvInfo2[wfCount].signals.size() ) )
			{
				// need to check both waveforms at once on this specifically since this information gets used by time-correction waveforms, and the time-correction
				// procedure for x will get called before y changes its initial phase. This is not very efficient currently since this will cause this phase
				// information to get written twice.
				for (auto varType : allWvInfo2[wfCount].varTypes )
				{
					if (varType == -1) 
					{
						if (signal + 1 > allWvInfo2[wfCount - 1].signals.size())
						{
							thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 ) + "the signal of the previous "
									 "waveform, but the previous waveform only had " + str( allWvInfo2[wfCount - 1].signals.size() )
									 + " signals!\n)" );
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


void NiawgController::errChecker( int err )
{
	if (err < 0)
	{
		thrower( "NIAWG Error: " + this->getErrorMsg() );
	}
}

/**
* getInputType takes in a string, the string representation of the input type, and returns a number directly corresponding to that input type.
*/
void NiawgController::getStandardInputType( std::string inputType, waveInfo &wvInfo )
{
	// Check against every possible generate input type. 
	wvInfo.initType = -1;
	for (auto number : range( MAX_NIAWG_SIGNALS ))
	{
		number += 1;
		if (inputType == "gen " + str( number ) + ", const")
		{
			wvInfo.initType = number;
		}
		else if (inputType == "gen " + str( number ) + ", amp ramp ")
		{
			wvInfo.initType = number + MAX_NIAWG_SIGNALS;
		}
		else if (inputType == "gen " + str( number ) + ", freq ramp ")
		{
			wvInfo.initType = number + 2 * MAX_NIAWG_SIGNALS;
		}
		else if (inputType == "gen " + str( number ) + ", freq & amp ramp ")
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
void NiawgController::getWaveData( ScriptStream &script, waveInfo &wave, std::vector<variable> singletons )
{
	// Initialize the variable counter inside the wave struct to zero:
	wave.varNum = 0;
	// infer the number of signals from the type assigned.
	wave.signals.resize( wave.initType % MAX_NIAWG_SIGNALS );

	for (int signal = 0; signal < wave.signals.size(); signal++)
	{
		switch (wave.initType / MAX_NIAWG_SIGNALS)
		{
			/// the case for "gen ?, const"
			case 0:
			{
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				loadParam( wave.signals[signal].freqInit, script, wave.varNum, wave.varNames, wave.varTypes, 
						   { 5 * signal + 1, 5 * signal + 2 }, singletons );
				// Scale the frequencies to be in hertz. (input is MHz)
				wave.signals[signal].freqInit *= 1000000.;
				wave.signals[signal].freqFin = wave.signals[signal].freqInit;
				// Can't be varied for this case type
				wave.signals[signal].freqRampType = "nr";
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				loadParam( wave.signals[signal].initPower, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 3, 5 * signal + 4 }, singletons );
				wave.signals[signal].finPower = wave.signals[signal].initPower;
				// Can't be varied
				wave.signals[signal].powerRampType = "nr";
				// Get phase, unless varied.
				loadParam( wave.signals[signal].initPhase, script, wave.varNum, wave.varNames,
						   wave.varTypes, { 5 * signal + 5 }, singletons );
				break;
			}
			/// The case for "gen ?, amp ramp"
			case 1:
			{
				// set the initial and final values to be equal, and to not use a ramp.
				loadParam( wave.signals[signal].freqInit, script, wave.varNum, wave.varNames,
						   wave.varTypes, { 5 * signal + 1, 5 * signal + 2 }, singletons );
			    // Scale the frequencies to be in hertz.(input is MHz)
				wave.signals[signal].freqInit *= 1000000.;
				wave.signals[signal].freqFin = wave.signals[signal].freqInit;
				// can't be varried.
				wave.signals[signal].freqRampType = "nr";
				std::string tempStr;
				script >> tempStr;
				wave.signals[signal].powerRampType = tempStr;
				loadParam( wave.signals[signal].initPower, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 3 }, singletons );
				loadParam( wave.signals[signal].finPower, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 4 }, singletons );
				loadParam( wave.signals[signal].initPhase, script, wave.varNum, wave.varNames, wave.varTypes,
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
				wave.signals[signal].freqRampType = tempStr;
				loadParam( wave.signals[signal].freqInit, script, wave.varNum, wave.varNames, wave.varTypes, { 5 * signal + 1 }, 
						   singletons );
				loadParam( wave.signals[signal].freqFin, script, wave.varNum, wave.varNames, wave.varTypes, { 5 * signal + 2 }, 
						   singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				wave.signals[signal].freqInit *= 1000000.;
				wave.signals[signal].freqFin *= 1000000.;
				// set the initial and final values to be equal, and to not use a ramp.
				loadParam( wave.signals[signal].initPower, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 3, 5 * signal + 4 }, singletons );
				wave.signals[signal].finPower = wave.signals[signal].initPower;
				wave.signals[signal].powerRampType = "nr";
				loadParam( wave.signals[signal].initPhase, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 5 }, singletons );
				break;
			}
				/// The case for "gen ?, freq & amp ramp"
			case 3:
			{
				// get all parameters from the file
				std::string tempStr;
				script >> tempStr;
				wave.signals[signal].freqRampType = tempStr;
				loadParam( wave.signals[signal].freqInit, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 1 }, singletons );
				loadParam( wave.signals[signal].freqFin, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 2 }, singletons );
				// Scale the frequencies to be in hertz.(input is MHz)
				wave.signals[signal].freqInit *= 1000000.;
				wave.signals[signal].freqFin *= 1000000.;

				// get all parameters from the file
				script >> tempStr;
				std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower );
				wave.signals[signal].powerRampType = tempStr;
				loadParam( wave.signals[signal].initPower, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 3 }, singletons );
				loadParam( wave.signals[signal].finPower, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 4 }, singletons );
				loadParam( wave.signals[signal].initPhase, script, wave.varNum, wave.varNames, wave.varTypes,
						   { 5 * signal + 5 }, singletons );
			}
			break;
		}
	}
	// get the common things.
	loadParam( wave.time, script, wave.varNum, wave.varNames, wave.varTypes,
			   { MAX_NIAWG_SIGNALS * 5 + 1 }, singletons );
	// Scale the time to be in seconds. (input is ms)
	wave.time *= 0.001;
	loadParam( wave.phaseOption, script, wave.varNum, wave.varNames, wave.varTypes, { MAX_NIAWG_SIGNALS * 5 + 2 }, 
			   singletons );
	script >> wave.delim;

	wave.sampleNum = waveformSizeCalc( wave );
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
* This important function takes in the input parameters and from those creates the waveform data array. It is inside this function that the waveform
* "library" functionality is realized. The function checks a list of strings to see if the waveform has previously been generated, and if it has, it reads
* the waveform data from a binary file. If it hasn't been created before, it writes a file containing all of the waveform data.
* Appended to the end of the waveform data files is the final phase of each of the signals involved in the file. This must be stripped off of the voltage
* data that populates the rest of the file as it's being read, and must be appended to the voltage data before it is written to a new file.
*/
void NiawgController::generateWaveform( ViReal64 * & tempWaveform, waveInfo & waveInfo, library &waveLibrary, debugInfo& options )
{
	unsigned long long time1, time2;
	// the number of seconds
	std::string waveformFileSpecs, waveformFileName;
	std::ifstream waveformFileRead;
	std::ofstream waveformFileWrite;

	// Construct the name of the raw data file from the parameters for the waveform. This can be a pretty long name, but that's okay 
	// because it's just text in a file at the end. This might become a problem if the name gets toooo long...
	for ( auto signal : range( waveInfo.signals.size() ) )
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
			ViReal64* readData = new ViReal64[waveInfo.sampleNum + waveInfo.signals.size()];
			waveformFileRead.read( (char *) readData, (waveInfo.sampleNum + waveInfo.signals.size()) * sizeof( ViReal64 ) );
			// grab the phase data off of the end.
			for ( auto signal : range( waveInfo.signals.size() ) )
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
				options.message += "Finished Reading Waveform. Ellapsed Time: " + str(ellapsedTime) + " seconds.\r\n";
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
		ViReal64* readData = new ViReal64[waveInfo.sampleNum + waveInfo.signals.size()];
		calcWaveData( waveInfo, readData );
		// Write the data, with phases, to the write file.
		waveformFileWrite.write( (const char *) readData, (waveInfo.sampleNum + waveInfo.signals.size()) * sizeof( ViReal64 ) );
		waveformFileWrite.close();
		// put the relevant data into another string.
		std::copy_n( readData, waveInfo.sampleNum, tempWaveform );
		delete[] readData;
		// write the newly written waveform's name to the library file.
		std::fstream libNameFile;
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
			options.message += "Finished Reading Waveform. Ellapsed Time: %.3f seconds.\r\n";
		}
	}
}


/**
 * This function handles logic input. Most of the function of this is to simply figure out which command the user was going for, and append that to the
 * actual script file.
 */
void NiawgController::handleLogic( niawgPair<ScriptStream>& scripts, niawgPair<std::string> inputs, std::string& scriptString )
{
	// Declare some parameters.
	niawgPair<int> sampleNums = { 0,0 }, repeatNums = { 0, 0 };
	if (inputs[Horizontal] != inputs[Vertical])
	{
		thrower( "ERROR: logical input commands must match, and they don't!\n\n" );
	}
	std::string input = inputs[Horizontal];
	if (input == "wait until trigger")
	{
		// Append command to script holder
		scriptString += "wait until " + std::string( EXTERNAL_TRIGGER_NAME ) + "\n";
	}
	else if (input == "wait until software trigger")
	{
		// Append command to script holder
		scriptString += "wait until " + std::string( SOFTWARE_TRIGGER_NAME ) + "\n";
	}
	else if (input == "wait set #")
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
	else if (input == "repeat set #")
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
	else if (input == "repeat until trigger")
	{
		scriptString += "repeat until " + std::string( EXTERNAL_TRIGGER_NAME ) + "\n";
	}
	else if (input == "repeat until software trigger")
	{
		scriptString += "repeat until " + std::string( SOFTWARE_TRIGGER_NAME ) + "\n";
	}
	else if (input == "repeat forever")
	{
		scriptString += "repeat forever\n";
	}
	else if (input == "end repeat")
	{
		scriptString += "end repeat\n";
	}
	// if-else Commands //
	else if (input == "if trigger")
	{
		scriptString += "if " + std::string( EXTERNAL_TRIGGER_NAME ) + "\n";
	}
	else if (input == "if software trigger")
	{
		scriptString += "if " + std::string( SOFTWARE_TRIGGER_NAME ) + "\n";
	}
	else if (input == "else")
	{
		scriptString += "else\n";
	}
	else if (input == "end if")
	{
		scriptString += "end if\n";
	}
}


/**
* This function handles the weird case commands, like ones that reference other waveforms or scripts. They are the special cases.
*/
void NiawgController::handleSpecial( niawgPair<ScriptStream>& scripts, outputInfo& output, niawgPair<std::string> inputTypes, 
									 library &waveLibrary, profileSettings profile, std::vector<variable> singletons, debugInfo& options, 
									 std::string& warnings )
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

		analyzeNiawgScripts( externalScripts, output, waveLibrary, profile, singletons, options, warnings );
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
		analyzeNiawgScripts( externalWaveformStreams, output, waveLibrary, profile, singletons, options, warnings );
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
	std::vector<double> powerPos, freqRampPos, phasePos(inputData.signals.size() );
	std::vector<double*> powerRampFileData;
	std::vector<double*> freqRampFileData;
	std::fstream powerRampFile, freqRampFile;
	std::string tempStr;

	/// deal with ramp calibration files. check all signals for files and read if yes.
	for (auto signal : range(inputData.signals.size() ))
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
	for (int signal = 0; signal < inputData.signals.size(); signal++)
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
		for (auto signal : range(inputData.signals.size() ) )
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
			for (auto signal : range(inputData.signals.size() ) )
			{
				currentPower += fabs(inputData.signals[signal].initPower + powerPos[signal]);
				/// modify here for frequency calibrations!
				/// need current frequency and calibration file.
			}

			// normalize each signal.
			for (auto signal : range(inputData.signals.size() ) )
			{
				powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal]) 
					* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
			}
		}
		///  Calculate data point.
		readData[sample] = 0;
		for (auto signal : range(inputData.signals.size() ) )
		{
			// get data point. V = Sqrt(Power) * Sin(Phase)
			readData[sample] += sqrt(inputData.signals[signal].initPower + powerPos[signal]) * sin(phasePos[signal]);
		}
	}

	/// Calculate one last time for the final phases. I want the final phase to be the phase of the NEXT data point. Then, following waveforms can
	/// START at this phase.
	double curTime = (double)sample / SAMPLE_RATE;
	for (auto signal : range( inputData.signals.size() ) )
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
	
	for (auto signal : range(inputData.signals.size() ) )
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
bool NiawgController::isLogic(std::string command)
{
	if (command == "wait until trigger" || command == "if trigger" || command == "repeat until trigger" 
		 || command == "wait until software trigger" || command == "if software trigger" || command == "repeat until software trigger"
		 || command == "wait set #" || command == "repeat set #" || command == "repeat forever" || command == "end repeat" 
		 || command == "else" || command == "end if")
	{
		return true;
	}
	else
	{
		return false;
	}
}


// handle constant & ramping waveforms.
void NiawgController::handleStandardWaveform( outputInfo& output, profileSettings profile, niawgPair<std::string> command, 
											  niawgPair<ScriptStream>& scripts, std::vector<variable> singletons, library waveLibrary, 
											  debugInfo& options )
{
	niawgPair<waveInfo> waveParams;
	// these hold literal waveform data (i.e. the voltage values that the waveforms take).
	niawgPair<ViReal64*> waveVoltages = { NULL, NULL }, prevVoltages = { NULL, NULL };
	ViReal64* mixedWaveVoltages = NULL, *prevMixedVoltages = NULL;
	// holds the waveform name (e.g. waveform1) that appears in the final script. Included extra places for large #s of waveforms.
	ViChar tempWaveformName[11];
	sprintf_s( tempWaveformName, 11, "Waveform%i", output.waveCount );
	// not sure why I have this limitation built in.
	if (output.isDefault && ((output.waveCount == 1 && profile.orientation == ORIENTATION[Vertical]) 
							  || (output.waveCount == 2 && profile.orientation == ORIENTATION[Horizontal])))
	{
		thrower( "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must be a "
				 "single waveform, not a sequence.\r\n" );
	}

	for (auto axis : AXES)
	{
		// The default value is false. If the waveform is varied, it gets changed to true.
		waveParams[axis].varies = false;
		// Get a number corresponding directly to the given input type.
		getStandardInputType( command[axis], waveParams[axis] );
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
				if (output.chan[axis].waves[output.waveCount - 1].varies == true)
				{
					waveParams[Horizontal].varies = waveParams[Vertical].varies = true;
					waveParams[axis].varNum++;
					// the ' sign is reserved. It's just a place-holder here to make sure the number of varied waveforms gets understood properly.
					waveParams[axis].varNames.push_back( "\'" );
					waveParams[axis].varTypes.push_back( -1 );
					break;
				}

				if (count + 1 > output.chan[axis].waves[output.waveCount - 1].signals.size())
				{
					thrower( "ERROR: You are trying to copy the phase of signal " + str( count + 1 ) + "  of " + AXES_NAMES[axis]
							 + " waveform #" + str( output.waveCount - 1 ) + ", but the previous waveform only had "
							 + str( output.chan[axis].waves[output.waveCount - 1].signals.size() ) + " signals!\n" );
				}

				signal.initPhase = output.chan[axis].waves[output.waveCount - 1].signals[count].finPhase;
			}
			count++;
		}
	}

	/// Waveform Creation /////////////////////////////////////////////////////////////////////////////////////////////////////////
	// only create waveform data if neither waveform is being varried and if the time management option is either 0 or -1. The time
	//  management option has already been checked to be the same for Vertical and Horizontal waveforms.
	for (auto axis : AXES)
	{
		if (waveParams[Vertical].varNum == 0 && waveParams[Horizontal].varNum == 0)
		{
			// Initialize the giant waveform arrays.
			waveVoltages[axis] = new ViReal64[waveParams[axis].sampleNum];
			// either calculate or read waveform data into the above arrays. 
			generateWaveform( waveVoltages[axis], waveParams[axis], waveLibrary, options );
		}
		else if (waveParams[axis].varNum > 0)
		{
			// Mark this waveform as being varied. 
			waveParams[Horizontal].varies = waveParams[Vertical].varies = true;
		}
	}

	for (auto axis : AXES)
	{
		if (waveParams[axis].varNum >= 0)
		{
			output.chan[axis].waves.push_back( waveParams[axis] );
		}
	}

	if (waveParams[Horizontal].varNum == 0 && waveParams[Vertical].varNum == 0)
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
				if (profile.orientation == ORIENTATION[axis])
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


void NiawgController::finalizeScript( unsigned long long repetitions, std::string name, std::vector<std::string> workingUserScripts, ViChar* userScriptSubmit )
{
	// format the script to send to the 5451 according to the accumulation number and based on the number of sequences.
	std::string finalUserScriptString = "script " + name + "\n";
	if (repetitions == 0)
	{
		finalUserScriptString += "repeat forever\n";
		for (int sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
		{
			finalUserScriptString += workingUserScripts[sequenceInc];
		}
		finalUserScriptString += "end repeat\n";
	}
	else
	{
		// repeat the script once for every accumulation.
		for (unsigned int accumCount = 0; accumCount < repetitions; accumCount++)
		{
			for (int sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
			{
				finalUserScriptString += workingUserScripts[sequenceInc];
			}
		}
	}
	// the NIAWG requires that the script file must end with "end script".
	finalUserScriptString += "end Script";

	// Convert script string to ViConstString. +1 for a null character on the end.
	userScriptSubmit = new ViChar[finalUserScriptString.length() + 1];
	sprintf_s( userScriptSubmit, finalUserScriptString.length() + 1, "%s", finalUserScriptString.c_str() );
	strcpy_s( userScriptSubmit, finalUserScriptString.length() + 1, finalUserScriptString.c_str() );
}


// handle flashing and streaming. Future: handle rearranging. Perhaps in the future, handle phase-managed waveforms here too, but those
// might go better in standard waveform handling, not sure.
void NiawgController::handleSpecialWaveform( outputInfo& output, profileSettings profile, niawgPair<std::string> commands,
											 niawgPair<ScriptStream>& scripts, std::vector<variable> singletons, library waveLibrary,
											 debugInfo& options )
{
	if (commands[Horizontal] != commands[Vertical])
	{
		thrower( "ERROR: Special waveform commands must match!" );
	}

	std::string command = commands[Horizontal];
	if (command == "flash")
	{
		flashInfo info;
		try
		{
			niawgPair<std::string> waveformsToFlashInput;
			for (auto axis : AXES)
			{
				scripts[axis] >> waveformsToFlashInput[axis];
				scripts[axis] >> info.flashCycleFreqInput[axis];
				info.flashNumber[axis] = std::stoi( waveformsToFlashInput[axis] );
			}
		}
		catch (std::invalid_argument&)
		{
			thrower( "ERROR: flashing number failed to convert to an integer! This parameter cannot be varied." );
		}
		info.varies = true;
		try
		{
			for (auto axis : AXES)
			{
				info.flashCycleFreq[axis] = std::stod( info.flashCycleFreqInput[axis] );
			}
			info.varies = false;
		}
		catch (std::invalid_argument&) {/* that's fine, prob just means it varies*/
		}

		for (auto axis : AXES)
		{
			info.waves[axis].resize( info.flashNumber[axis] );
			for (size_t waveCount = 0; waveCount < info.waves[axis].size(); waveCount++)
			{

			}
		}
		// get flashing inputs
		//		- Number of waveforms to flash
		//		- flashing period

		// for each waveform to be flashed...
		//		get that waveforms complete data.

		// mix data of different waveforms according to the flashing period, creating the mixed, flashing waveform.

		// write waveform to niawg.
	}
	else if (command == "stream")
	{
		// note: special handling must be added to do specific streams outside normal capabilities.

		// write waveform to niawg
		// tell niawg about streamed waveform
		// get & store handle to streamed waveform.
		// not sure if anything else...
	}
	else
	{
		thrower( "" );
	}
}

// should take as input info for flashing waveforms, which will include the data in each waveform and the flashing interval. Will
// probably return ViReal64*.
void NiawgController::calculateFlashingWaveform()
{

}

// gets called during execution in order to stream the data.
void NiawgController::streamWaveformData()
{

}


/**
* this function takes in a command and checks it against all "generate commands", returing true if the inputted command matches a generate 
* command and false otherwise
*/
bool NiawgController::isStandardWaveform(std::string inputType)
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

bool NiawgController::isSpecialWaveform( std::string command )
{
	if (command == "flash" || command == "stream")
	{
		return true;
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
		for (int contInc = 2; contInc < output.chan[axis].waves.size(); contInc++)
		{
			// if two waveforms have the same number of parameters...
			if (output.chan[axis].waves[contInc].signals.size() == output.chan[axis].waves[contInc - 1].signals.size())
			{
				for (int waveInc = 0; waveInc < output.chan[axis].waves[contInc].signals.size(); waveInc++)
				{
					if (output.chan[axis].waves[contInc].signals[waveInc].initPower
						 != output.chan[axis].waves[contInc - 1].signals[waveInc].finPower)
					{
						comm->sendError( "Warning: Amplitude jump at waveform #" + std::to_string( contInc )
												+ " in " + AXES_NAMES[axis] + " component detected!\r\n" );
					}
					if (output.chan[axis].waves[contInc].signals[waveInc].freqInit
						 != output.chan[axis].waves[contInc - 1].signals[waveInc].freqFin)
					{
						comm->sendError( "Warning: Frequency jump at waveform #" + std::to_string( contInc ) + " in "
												+ AXES_NAMES[axis] + " component detected!\r\n" );
					}
					if (output.chan[axis].waves[contInc].signals[waveInc].initPhase
						 - output.chan[axis].waves[contInc - 1].signals[waveInc].finPhase
				> CORRECTION_WAVEFORM_ERROR_THRESHOLD)
					{
						comm->sendError( "Warning: Phase jump (greater than what's wanted for correction waveforms) at "
												"waveform #" + std::to_string( contInc ) + " in " + AXES_NAMES[axis] + " component "
												"detected!\r\n" );
					}
					// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
					if (output.chan[axis].waves[contInc].signals[waveInc].powerRampType != "nr"
						 && (output.chan[axis].waves[contInc].signals[waveInc].initPower
							  == output.chan[axis].waves[contInc].signals[waveInc].finPower))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to amplitude ramp, but the initial and final "
												"amplitudes are the same. This is not a ramp.\r\n" );
					}
					// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
					if (output.chan[axis].waves[contInc].signals[waveInc].freqRampType != "nr"
						 && (output.chan[axis].waves[contInc].signals[waveInc].freqInit
							  == output.chan[axis].waves[contInc].signals[waveInc].freqFin))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to frequency ramp, but the initial and final "
												"frequencies are the same. This is not a ramp.\r\n" );
					}

					// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually ramping.
					if (output.chan[axis].waves[contInc].signals[waveInc].powerRampType == "nr"
						 && (output.chan[axis].waves[contInc].signals[waveInc].initPower
							  != output.chan[axis].waves[contInc].signals[waveInc].finPower))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to no amplitude ramp, but the initial and final amplitudes are the "
												"different. This is not a ramp, the initial value will be used.\r\n" );
					}
					// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
					if (output.chan[axis].waves[contInc].signals[waveInc].freqRampType == "nr"
						 && (output.chan[axis].waves[contInc].signals[waveInc].freqInit
							  != output.chan[axis].waves[contInc].signals[waveInc].freqInit))
					{
						comm->sendError( "Warning: " + AXES_NAMES[axis] + " waveform #" + std::to_string( contInc )
												+ "is set to no frequency ramp, but the initial and final frequencies are "
												"different. This is not a ramp, the initial value will be used throughout.\r\n" );
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

