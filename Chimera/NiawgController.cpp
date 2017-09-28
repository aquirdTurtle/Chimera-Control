#include "stdafx.h"
#include <boost/algorithm/string/replace.hpp>
#include "NiawgController.h"
#include "MasterManager.h"
#include "NiawgWaiter.h"
#include "Rearranger.h"
#include <chrono>

void NiawgController::initialize()
{
	// open up the files and check what I have stored.
	openWaveformFiles();
	/// Initialize the waveform generator via FGEN. Currently this is set to reset the initialization parameters from 
	/// the last run.
	// initializes the session handle.
	fgenConduit.init( NI_5451_LOCATION, VI_TRUE, VI_TRUE );
	// tells the niaw where I'm outputting.
	fgenConduit.configureChannels();
	// Set output mode of the device to scripting mode (defined in constants.h)
	fgenConduit.configureOutputMode();
	// configure marker event. This is set to output on PFI1, a port on the front of the card.
	fgenConduit.configureMarker( "Marker0", "PFI1" );
	// enable flatness correction. This allows there to be a bit less frequency dependence on the power outputted by 
	// the waveform generator.
	fgenConduit.setViBooleanAttribute( NIFGEN_ATTR_FLATNESS_CORRECTION_ENABLED, VI_TRUE );
	// configure the trigger. Trigger mode doesn't need to be set because I'm using scripting mode.
	fgenConduit.configureDigtalEdgeScriptTrigger();
	// Configure the gain of the signal amplification.
	fgenConduit.configureGain( NIAWG_GAIN );
	// Configure Sample Rate. The maximum value of this is 400 mega-samples per second, but it is quite buggy, so we've
	// been using 320 MS/s
	fgenConduit.configureSampleRate( NIAWG_SAMPLE_RATE );
	// Configure the analogue filter. This is important for high frequency signals, as it smooths out the discrete 
	// steps that the waveform generator outputs. This is a low-pass filter.
	fgenConduit.enableAnalogFilter( NIAWG_FILTER_FREQENCY );
	/// Configure Clock input
	// uncomment for high resolution mode
	fgenConduit.configureClockMode( NIFGEN_VAL_HIGH_RESOLUTION );
	// uncomment for default onboard clock
	// myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleClockSource(eSessionHandle, "OnboardClock")
	// Unccoment for using an external clock as a "sample clock"
	// myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureSampleClockSource(eSessionHandle, "ClkIn")
	// Uncomment for using an external clock as a reference clock
	// myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureReferenceClock(eSessionHandle, "ClkIn", 10000000), HORIZONTAL_ORIENTATION, 
	//									theMainApplicationWindow.getComm())
	//fgenConduit.setViReal64Attribute( NIFGEN_ATTR_CHANNEL_DELAY, 0.000000250, "0" );
}


void NiawgController::programNiawg( MasterThreadInput* input, NiawgOutputInfo& output, std::string& warnings, 
									UINT variation, UINT totalVariations, std::vector<long>& variedMixedSize, 
									std::vector<ViChar>& userScriptSubmit )
{
	input->comm->sendColorBox( Niawg, 'Y' );
	input->niawg->handleVariations( output, input->key->getKey(), variation, variedMixedSize, warnings,
									input->debugOptions, totalVariations );
	if (input->settings.dontActuallyGenerate) { return; }

	// Restart Waveform
	input->niawg->turnOff();
	input->niawg->programVariations( variation, variedMixedSize, output );
	input->niawg->fgenConduit.writeScript( userScriptSubmit );
	input->niawg->fgenConduit.setViStringAttribute( NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript" );
	// initiate generation before telling the master. this is because scripts are supposed to be designed to sit on an 
	// initial waveform until the master sends it a trigger.
	input->niawg->turnOn();
	for (UINT waveInc = 2; waveInc < output.waves.size(); waveInc++)
	{
		output.waves[waveInc].core.waveVals.clear();
		output.waves[waveInc].core.waveVals.shrink_to_fit();
	}
	variedMixedSize.clear();
	input->comm->sendColorBox( Niawg, 'G' );
}


bool NiawgController::outputVaries(NiawgOutputInfo output)
{
	for (auto wave : output.waves)
	{
		if (wave.core.varies)
		{
			// if any wave varies...
			return true;
		}
	}
	return false;
}

// analyzes all niawg scripts and prepares the output structure.
// this handling is a relic from the old individual niawg thread. 
// I Should re-organize how I get the niawg files and intensity script files to be consistent with the 
// master script file.
void NiawgController::prepareNiawg(MasterThreadInput* input, NiawgOutputInfo& output, 
									niawgPair<std::vector<std::fstream>>& niawgFiles, std::string& warnings, 
									std::vector<ViChar>& userScriptSubmit, bool& foundRearrangement, 
									rearrangeParams rInfo )
{
	input->comm->sendColorBox( Niawg, 'Y' );
	ProfileSystem::openNiawgFiles( niawgFiles, input->profile, input->runNiawg );
	std::vector<std::string> workingUserScripts( input->profile.sequenceConfigNames.size() );
	// analyze each script in sequence.
	for (UINT sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
	{
		niawgPair<ScriptStream> scripts;
		output.niawgLanguageScript = "";
		input->comm->sendStatus( "Working with configuraiton # " + str( sequenceInc + 1 ) + " in Sequence...\r\n" );
		/// Create Script and Write Waveforms ////////////////////////////////////////////////////////////////////
		scripts[Vertical] << niawgFiles[Vertical][sequenceInc].rdbuf();
		scripts[Horizontal] << niawgFiles[Horizontal][sequenceInc].rdbuf();
		if ( input->debugOptions.outputNiawgHumanScript )
		{
			input->comm->sendDebug( boost::replace_all_copy( "NIAWG Human Script:\n"
															 + scripts[Vertical].str( )
															 + "\n\n", "\n", "\r\n" ) );
			input->comm->sendDebug( boost::replace_all_copy( "NIAWG Human Script:\n"
															 + scripts[Horizontal].str( )
															 + "\n\n", "\n", "\r\n" ) );
		}
		input->niawg->analyzeNiawgScripts( scripts, output, input->profile, input->debugOptions, warnings,
										   rInfo );
		workingUserScripts[sequenceInc] = output.niawgLanguageScript;

		if (input->thisObj->getAbortStatus()) { thrower( "\r\nABORTED!\r\n" ); }
	}
	input->comm->sendStatus( "Constant Waveform Preparation Completed...\r\n" );
	input->niawg->finalizeScript( input->repetitionNumber, "experimentScript", workingUserScripts, userScriptSubmit, 
								  !input->niawg->outputVaries(output) );
	if (input->debugOptions.outputNiawgMachineScript)
	{
		input->comm->sendDebug( boost::replace_all_copy( "NIAWG Machine Script:\n"
														 + std::string( userScriptSubmit.begin(), userScriptSubmit.end() )
														 + "\n\n", "\n", "\r\n" ) );
	}
	// check if any waveforms are rearrangement instructions.
	for ( auto& wave : output.waves )
	{
		if ( wave.isRearrangement )
		{
			// if already found one...
			if ( foundRearrangement )
			{
				thrower( "ERROR: Multiple rearrangement waveforms found, but not allowed!" );
			}
			foundRearrangement = true;
			// start rearrangement thread. Give the thread the queue.
			input->niawg->startRearrangementThread( input->atomQueueForRearrangement, wave, input->comm,
													input->rearrangerLock, input->andorsImageTimes,
													input->grabTimes, input->conditionVariableForRearrangement,
													input->rearrangeInfo );
		}
	}
	if ( input->rearrangeInfo.active && !foundRearrangement )
	{
		thrower( "ERROR: system is primed for rearranging atoms, but no rearrangement waveform was found!" );
	}
	else if ( !input->rearrangeInfo.active && foundRearrangement )
	{
		thrower( "ERROR: System was not primed for rearrangign atoms, but a rearrangement waveform was found!" );
	}
}


bool NiawgController::niawgIsRunning()
{
	return runningState;
}


void NiawgController::setRunningState( bool newRunningState )
{
	runningState = newRunningState;
}


std::string NiawgController::getCurrentScript()
{
	return fgenConduit.getCurrentScript();
}


void NiawgController::setDefaultWaveforms( MainWindow* mainWin )
{
	defaultScript.clear();
	defaultScript.shrink_to_fit();
	defaultMixedWaveform.clear();
	defaultMixedWaveform.shrink_to_fit();
	// counts the number of predefined waveforms that have been handled or defined.
	int predWaveformCount = 0;
	// A vector which stores the number of values that a given variable will take through an experiment.
	std::vector<std::size_t> length;
	// first level is for different configurations, second is for horizontal or vertical file within a configuration.
	niawgPair<std::vector<std::fstream>> configFiles;
	configFiles[Horizontal].push_back( std::fstream( str(DEFAULT_SCRIPT_FOLDER_PATH) + "DEFAULT_HORIZONTAL_SCRIPT.nScript" ) );
	configFiles[Vertical].push_back( std::fstream( str(DEFAULT_SCRIPT_FOLDER_PATH) + "DEFAULT_VERTICAL_SCRIPT.nScript" ) );
	// check errors
	for ( auto fileAxis : AXES )
	{
		if ( !configFiles[fileAxis].back( ).is_open( ) )
		{
			thrower( "FATAL ERROR: Couldn't open default configuration " + AXES_NAMES[fileAxis] + " default file." );
		}
	}
	NiawgOutputInfo output;
	output.isDefault = true;
	///	Load Default Waveforms
	debugInfo debug;
	std::string warnings;
	profileSettings profile;
	try
	{
		output.niawgLanguageScript = "script DefaultConfigScript\n";
		niawgPair<ScriptStream> scripts;
		scripts[Horizontal] << configFiles[Horizontal].back( ).rdbuf( );
		scripts[Vertical] << configFiles[Vertical].back( ).rdbuf( );
		rearrangeParams rInfoDummy;
		rInfoDummy.moveSpeed = 0.00006;
		analyzeNiawgScripts( scripts, output, profile, debug, warnings, rInfoDummy );
		// the script file must end with "end script".
		output.niawgLanguageScript += "end Script";
		// Convert script string to ViConstString. +1 for a null character on the end.
		defaultScript = std::vector<ViChar>( output.niawgLanguageScript.begin( ), output.niawgLanguageScript.end( ) );
	}
	catch (Error& except)
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
}

// this is to be run at the end of the experiment procedure.
void NiawgController::cleanupNiawg( profileSettings profile, bool masterWasRunning, 
									niawgPair<std::vector<std::fstream>>& niawgFiles, NiawgOutputInfo& output,
									Communicator* comm, bool dontGenerate)
{
	// close things
	for ( const auto& sequenceInc : range( profile.sequenceConfigNames.size() ) )
	{
		for ( const auto& axis : AXES )
		{
			if ( niawgFiles[axis][sequenceInc].is_open( ) )
			{
				niawgFiles[axis][sequenceInc].close( );
			}
		}
	}
	if ( !masterWasRunning )
	{
		// this has got to be overkill...
		NiawgWaiter waiter;
		waiter.startWaitThread( this, profile );
		waiter.wait( comm );
	}
	else
	{
		try
		{
			turnOff( );
		}
		catch ( Error& ) {}
	}
	// Clear waveforms off of NIAWG (not working??? memory appears to still run out... (that's a very old note, 
	// haven't tested in a long time but has never been an issue.))
	for ( UINT waveformInc = 2; waveformInc < output.waves.size( ); waveformInc++ )
	{
		// wave name is set by size of waves vector, size is not zero-indexed.
		// name can be empty for some special cases like re-arrangement waves.
		if ( output.waves[waveformInc].core.name != "" )
		{
			fgenConduit.deleteWaveform( cstr( output.waves[waveformInc].core.name ) );
		}
	}
	if ( !dontGenerate )
	{
		fgenConduit.deleteScript( "experimentScript" );
	}
	for ( auto& wave : output.waves )
	{
		wave.core.waveVals.clear( );
		wave.core.waveVals.shrink_to_fit( );
	}
}


void NiawgController::waitForRearranger( )
{
	int result = WaitForSingleObject( rearrangerThreadHandle, 500 );
	if ( result == WAIT_TIMEOUT )
	{
		thrower( "ERROR: waiting for Rearranger thread to finish timed out!?!?!?" );
	}
	try
	{
		deleteRearrangementWave( );
	}
	catch ( Error& )
	{

	}
}


void NiawgController::turnOffRearranger( )
{
	// make sure the rearranger thread is off.
	threadStateSignal = false;
}


void NiawgController::restartDefault()
{
	try
	{
		// to be sure.
		turnOffRearranger( );		
		turnOff();
		fgenConduit.clearMemory();
		fgenConduit.allocateNamedWaveform( cstr( defaultWaveName ), defaultMixedWaveform.size( ) / 2 );
		fgenConduit.writeNamedWaveform( cstr( defaultWaveName ), defaultMixedWaveform.size( ),
										defaultMixedWaveform.data( ) );
		fgenConduit.writeScript( defaultScript );
		setDefaultWaveformScript( );
		turnOn();
	}
	catch (Error& except)
	{
		thrower( "WARNING! The NIAWG encountered an error and was not able to restart smoothly. It is (probably) not outputting anything. You may "
				 "consider restarting the code. Inside the restart area, NIAWG function returned " + except.whatStr() );
	}
}


void NiawgController::programVariations( UINT variation, std::vector<long>& variedMixedSize, NiawgOutputInfo& output )
{
	int mixedWriteCount = 0;
	// skip defaults so start at 2.
	for (UINT waveInc = 2; waveInc < output.waves.size(); waveInc++)
	{
		std::string variedWaveformName = "Waveform" + str( waveInc );
		if (output.waves[waveInc].core.varies)
		{
			if (variation != 0)
			{
				fgenConduit.deleteWaveform( cstr( variedWaveformName ) );
			}
			// And write the new one.
			fgenConduit.allocateNamedWaveform( cstr( variedWaveformName ), variedMixedSize[mixedWriteCount] / 2 );
			fgenConduit.writeNamedWaveform( cstr( variedWaveformName ), variedMixedSize[mixedWriteCount],
											output.waves[waveInc].core.waveVals.data() );
			mixedWriteCount++;
		}
	}
}


/**
  * The purpose of this function is to systematically read the input instructions files, create and read waveforms associated with
  * them, write the script as it goes, and eventually combine the x and y-waveforms into their final form for being sent to the waveform 
  * generator.
  */
void NiawgController::analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, NiawgOutputInfo& output, 
										   profileSettings profile, debugInfo& options, std::string& warnings, 
										   rearrangeParams rInfo )
{
	/// Preparation
	for (auto& axis : AXES)
	{
		currentScripts[axis] = scripts[axis].str();
		scripts[axis].clear();
		scripts[axis].seekg( 0, std::ios::beg );
	}
	// Some declarations.
	niawgPair<std::string> command;
	// get the first input
	for (auto axis : AXES)
	{
		scripts[axis] >> command[axis];
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
			handleSpecial( scripts, output, command, profile, options, warnings );
		}
		else if (isStandardWaveform( command[Horizontal] ) && isStandardWaveform( command[Vertical] ))
		{
			handleStandardWaveform( output, profile, command, scripts, options );
		}
		else if (isSpecialWaveform( command[Horizontal] ) && isSpecialWaveform( command[Vertical] ))
		{
			handleSpecialWaveform( output, profile, command, scripts, options, rInfo );
		}
		else
		{
			thrower( "ERROR: Input types from the two files do not match or are unrecofgnized!\nBoth must be logic commands, both must be "
					 "generate commands, or both must be special commands. See documentation on the correct format for these commands.\n\n"
					 "The two inputted types are: " + command[Vertical] + " and " + command[Horizontal] + " for waveform #"
					 + str( output.waves.size() - 1 ) + "!" );
		}
		// get next input.
		for (auto axis : AXES)
		{
			scripts[axis] >> command[axis];
		}
	}
}


/**/
void NiawgController::handleVariations( NiawgOutputInfo& output, key varKey, const UINT variation, 
										std::vector<long>& mixedWaveSizes, std::string& warnings, 
										debugInfo& debugOptions, UINT totalVariations )
{
	int mixedCount = 0;

	// I think waveInc = 0 & 1 are always the default.. should I be handling that at all? shouldn't make a difference 
	// I don't think.
	for (UINT waveInc = 0; waveInc < output.waves.size(); waveInc++)
	{
		if (output.waves[waveInc].core.varies)
		{
			// loop through variables
			for (auto const& variable : varKey)
			{
				if (output.waves[waveInc].isFlashing)
				{
					std::string currentVar = variable.first, warnings;
					for ( UINT flashInc = 0; flashInc < output.waves[waveInc].flash.flashNumber; flashInc++)
					{
						for (auto axis : AXES)
						{
							/// Loop for varibles in given Waveform
							for (UINT varNumber = 0; varNumber < output.waves[waveInc].flash.flashWaves[flashInc].chan[axis].varNum; varNumber++)
							{
								if (output.waves[waveInc].core.chan[axis].varNames[varNumber] == currentVar)
								{
									simpleWave& currentWave = output.waves[waveInc].flash.flashWaves[flashInc];
									// always.The flashing waves shouldn't really "know" anything about each other. 
									// I think that this is the most symmetric way to do this.
									waveInfo& previousWave = output.waves[waveInc - 1];
									varyParam( currentWave, previousWave, axis, currentWave.chan[axis].varTypes[varNumber],
											   variable.second.first[variation], warnings );
								}
							}
						}
					}
				}
				else
				{
					std::string currentVar = variable.first, warnings;
					for (auto axis : AXES)
					{
						/// Loop for varibles in given Waveform
						for (UINT varNumber = 0; varNumber < output.waves[waveInc].core.chan[axis].varNum; varNumber++)
						{
							if (output.waves[waveInc].core.chan[axis].varNames[varNumber] == currentVar)
							{
								varyParam( output.waves[waveInc].core, output.waves[waveInc - 1], axis, 
										   output.waves[waveInc].core.chan[axis].varTypes[varNumber],
										   variable.second.first[variation], warnings );
							}
						}
					}
				}
			}
			finalizeStandardWave( output.waves[waveInc].core, debugOptions );
			mixedWaveSizes.push_back( 2 * output.waves[waveInc].core.sampleNum );
			mixedCount++;
		} // end if varies
	}
	checkThatWaveformsAreSensible( warnings, output );
}

/* * * * *
* This function is called when it is time to vary a parameter of a waveform description file before getting the data corresponding to the new waveform.
* It is mostly a large case structure and has different cases for all of the different parameters you could be changing, as well as several cases for
* dealing with correction waveforms and all of their glory.
* waveType can be waveInfo or simpleWave.
* */
void NiawgController::varyParam( simpleWave& wave, waveInfo previousWave, int axis, int paramNum, double paramVal, 
								 std::string& warnings )
{
	// change parameters, depending on the case. This was set during input reading.
	std::vector<double> initPhases;
	switch (paramNum)
	{
		case -1:
		{
			// This is the -1 option for phase control when the previous waveform is being varied.
			for (auto axis : AXES)
			{
				for (auto signal : range( wave.chan[axis].signals.size() ))
				{
					// check if this is supposed to be overwritten during variable runs.
					for (auto varType : wave.chan[axis].varTypes)
					{
						if (varType == -1)
						{
							if (signal + 1 > previousWave.core.chan[axis].signals.size())
							{
								thrower( "ERROR: You are trying to copy the phase of the " + str( signal + 1 ) + "the signal of the previous "
										 "waveform, but the previous waveform only had "
										 + str( previousWave.core.chan[axis].signals.size() ) + " signals!\n)" );
							}
							if (previousWave.isFlashing)
							{
								// note: I force it to take the final phase of the first flashing waveform. I could add functionality to change
								// this and have it as an extra option, but this I think is the most "symmetric" option.
								wave.chan[axis].signals[signal].initPhase = previousWave.flash.flashWaves.front().chan[axis].signals[signal].finPhase;
							}
							else
							{
								wave.chan[axis].signals[signal].initPhase = previousWave.core.chan[axis].signals[signal].finPhase;
							}
						}
					}
				}
			}
			break;
		}
		case 5 * MAX_NIAWG_SIGNALS + 1:
		{
			if (paramVal < 0)
			{
				thrower( "ERROR: Attempted to set negative waveform parameter. Don't do that. Value was"
						 + str( paramVal ) );
			}
			// CONVERT from milliseconds to seconds.
			wave.time = paramVal * 0.001;
			// Changing the time changes the sample number, so recalculate that.
			wave.sampleNum = waveformSizeCalc( wave.time );
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
			switch (paramNum % 5)
			{
				case 1:
					wave.chan[axis].signals[signalNum].freqInit = paramVal;
					break;
				case 2:
					wave.chan[axis].signals[signalNum].freqFin = paramVal;
					break;
				case 3:
					wave.chan[axis].signals[signalNum].initPower = paramVal;
					break;
				case 4:
					wave.chan[axis].signals[signalNum].finPower = paramVal;
					break;
				case 5:
					wave.chan[axis].signals[signalNum].initPhase = paramVal;
					break;
			}
		}
	}
}






/**
 * getInputType takes in a string, the string representation of the input type, and returns a number directly corresponding to that input type.
 */
void NiawgController::loadStandardInputType( std::string inputType, channelWave &wvInfo )
{
	// Check against every possible generate input type. 
	wvInfo.initType = -1;
	for (auto number : range( MAX_NIAWG_SIGNALS ))
	{
		number += 1;
		if (inputType == "gen" + str( number ) + "const")
		{
			wvInfo.initType = number;
		}
		else if (inputType == "gen" + str( number ) + "ampramp")
		{
			wvInfo.initType = number + MAX_NIAWG_SIGNALS;
		}
		else if (inputType == "gen" + str( number ) + "freqramp")
		{
			wvInfo.initType = number + 2 * MAX_NIAWG_SIGNALS;
		}
		else if (inputType == "gen" + str( number ) + "freq&ampramp")
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
 * loads the relevant parameters from the file into the wave information structure for a single channel. just loads them, more
 * analysis & checks done later.
 */
void NiawgController::openWaveformFiles()
{
	std::string tempStr;
	std::fstream libNameFile;
	for (UINT folderInc = 0; folderInc < WAVEFORM_TYPE_FOLDERS.size(); folderInc++)
	{
		std::string folderPath = LIB_PATH + WAVEFORM_TYPE_FOLDERS[folderInc];
		folderPath.resize( folderPath.size() - 1 );
		DWORD ftyp = GetFileAttributesA( cstr(folderPath) );
		if (ftyp == INVALID_FILE_ATTRIBUTES || !(ftyp & FILE_ATTRIBUTE_DIRECTORY))
		{
			// create directory
			if (!CreateDirectory( cstr(folderPath), NULL ))
			{
				thrower( "ERROR: Error Creating directory for waveform library system. Error was windows error " + str( GetLastError()) + ", Path was " + folderPath);
			}
		}
		// open the file. It's written in binary.
		std::string libNameFilePath = LIB_PATH + WAVEFORM_TYPE_FOLDERS[folderInc] + WAVEFORM_NAME_FILES[folderInc];
		libNameFile.open( libNameFilePath, std::ios::binary | std::ios::in );
		// Make sure that the file opened correctly.
		if (!libNameFile.is_open())
		{
			// try creating a new file.
			libNameFile.open( libNameFilePath, std::ios::binary | std::ios::out );
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
* This important function takes in the input parameters and from those creates the waveform data array. It is inside 
* this function that the waveform "library" functionality is realized. The function checks a list of strings to see if
* the waveform has previously been generated, and if it has, it reads the waveform data from a binary file. If it 
* hasn't been created before, it writes a file containing all of the waveform data. Appended to the end of the 
* waveform data files is the final phase of each of the signals involved in the file. This must be stripped off of the 
* voltage data that populates the rest of the file as it's being read, and must be appended to the voltage data before
* it is written to a new file.
*/
void NiawgController::generateWaveform( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time)
{
	waveInfo.wave.resize( sampleNum );
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
	waveformFileSpecs += str( time * 1000.0 ) + "; ";
	// Start timer
	std::chrono::time_point<chronoClock> time1( chronoClock::now( ));
	/// Loop over all previously recorded files (these should have been filled by a previous call to openWaveformFiles()).
	for ( UINT fileInc = 0; fileInc < waveLibrary[waveInfo.initType].size(); fileInc++ )
	{
		// if you find this waveform to have already been written...
		if ( waveLibrary[waveInfo.initType][fileInc] == waveformFileSpecs )
		{
			// Construct the file address
			std::string waveFileReadName = LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + str( waveInfo.initType ) + "_"
				+ str( fileInc ) + ".txt";
			waveformFileRead.open( waveFileReadName, std::ios::binary | std::ios::in );
			std::vector<ViReal64> readData(sampleNum + waveInfo.signals.size());
			waveformFileRead.read( (char *) readData.data(), (sampleNum + waveInfo.signals.size()) * sizeof( ViReal64 ) );
			// grab the phase data off of the end.
			for ( auto signal : range( waveInfo.signals.size() ) )
			{
				waveInfo.signals[signal].finPhase = readData[sampleNum + signal];
			}
			// put the relevant voltage data into a the new array.
			waveInfo.wave = std::vector<ViReal64>( readData.begin(), readData.begin() + sampleNum );
			readData.clear();
			// make sure the large amount of memory is deallocated.
			readData.shrink_to_fit();
			waveformFileRead.close();
			if ( options.showReadProgress)
			{
				std::chrono::time_point<chronoClock> time2( chronoClock::now( ) );
				double ellapsedTime(std::chrono::duration<double>( (time2-time1) ).count());
				options.message += "Finished Reading Waveform. Ellapsed Time: " + str(ellapsedTime) + " seconds.\r\n";
			}
			// if the file got read, I don't need to do any writing, so go ahead and return.
			return;
		}
	}

	// if the code reaches this point, it could not find a file to read, and so will now create the data from scratch 
	// and write it. 
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
		std::chrono::time_point<chronoClock> time1( chronoClock::now( ) );
		//ULONGLONG time1 = GetTickCount64();
		// calculate all voltage values and final phases and store them in the readData variable.
		std::vector<ViReal64> readData( sampleNum + waveInfo.signals.size() );
		calcWaveData( waveInfo, readData, sampleNum, time );
		// Write the data, with phases, to the write file.
		waveformFileWrite.write( (const char *) readData.data(), (sampleNum + waveInfo.signals.size()) * sizeof( ViReal64 ) );
		waveformFileWrite.close();
		// put the relevant data into another string.
		waveInfo.wave = std::vector<ViReal64>( readData.begin(), readData.begin() + sampleNum );
		readData.clear();
		// make sure the large amount of memory is deallocated.
		readData.shrink_to_fit();
		// write the newly written waveform's name to the library file.
		std::fstream libNameFile;
		libNameFile.open( LIB_PATH + WAVEFORM_TYPE_FOLDERS[waveInfo.initType] + WAVEFORM_NAME_FILES[waveInfo.initType],
						  std::ios::binary | std::ios::out | std::ios::app );
		if ( !libNameFile.is_open() )
		{
			thrower("ERROR! waveform name file not opening correctly.\n");
		}
		// add the waveform name to the current list of strings. do it BEFORE adding the newline T.T
		waveLibrary[waveInfo.initType].push_back( cstr(waveformFileSpecs));
		// put a newline in front of the name so that all of the names don't get put on the same line.
		waveformFileSpecs = "\n" + waveformFileSpecs;
		libNameFile.write( cstr(waveformFileSpecs), waveformFileSpecs.size() );
		libNameFile.close();
		if ( options.showWriteProgress )
		{
			//char processTimeMsg[200];
			std::chrono::time_point<chronoClock> time2( chronoClock::now( ) );
			double ellapsedTime = std::chrono::duration<double>( time2 - time1 ).count( );
			//sprintf_s( processTimeMsg, "Finished Writing Waveform. Ellapsed Time: %.3f seconds.\r\n", ellapsedTime );
			options.message += "Finished writing waveform. Ellapsed Time: " + str( ellapsedTime ) + " seconds.\r\n";
		}
	}
}


/**
 * This function handles logic input. Most of the function of this is to simply figure out which command the user was 
 * going for, and append that to the actual script file.
 */
void NiawgController::handleLogic( niawgPair<ScriptStream>& scripts, niawgPair<std::string> inputs,
								   std::string& rawNiawgScriptString )
{
	// Declare some parameters.
	niawgPair<int> sampleNums = { 0,0 }, repeatNums = { 0, 0 };
	if (inputs[Horizontal] != inputs[Vertical])
	{
		thrower( "ERROR: logical input commands must match, and they don't!\n\n" );
	}
	std::string input = inputs[Horizontal];
	if (input == "waittiltrig")
	{
		// Append command to script holder
		rawNiawgScriptString += "wait until " + fgenConduit.getExternalTriggerName() + "\n";
	}
	else if (input == "waitTilsoftwaretrig")
	{
		// Append command to script holder
		rawNiawgScriptString += "wait until " + fgenConduit.getSoftwareTriggerName() + "\n";
	}
	else if (input == "waitset#")
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
		rawNiawgScriptString += "wait " + str( (long long)sampleNums[Horizontal] ) + "\n";
	}
	// Repeat commands // 
	else if (input == "repeatset#")
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
		rawNiawgScriptString += "repeat " + str( (long long)repeatNums[Horizontal] ) + "\n";
	}
	else if (input == "repeattiltrig")
	{
		rawNiawgScriptString += "repeat until " + fgenConduit.getExternalTriggerName() + "\n";
	}
	else if (input == "repeattilsoftwaretrig")
	{
		rawNiawgScriptString += "repeat until " + fgenConduit.getSoftwareTriggerName() + "\n";
	}
	else if (input == "repeatforever")
	{
		rawNiawgScriptString += "repeat forever\n";
	}
	else if (input == "endrepeat")
	{
		rawNiawgScriptString += "end repeat\n";
	}
	// if-else Commands //
	else if (input == "iftrig")
	{
		rawNiawgScriptString += "if " + fgenConduit.getExternalTriggerName() + "\n";
	}
	else if (input == "ifsoftwaretrig")
	{
		rawNiawgScriptString += "if " + fgenConduit.getSoftwareTriggerName() + "\n";
	}
	else if (input == "else")
	{
		rawNiawgScriptString += "else\n";
	}
	else if (input == "end if")
	{
		rawNiawgScriptString += "end if\n";
	}
}


/**
* This function handles the weird case commands, like ones that reference other waveforms or scripts. They are the special cases.
*/
void NiawgController::handleSpecial( niawgPair<ScriptStream>& scripts, NiawgOutputInfo& output, niawgPair<std::string> inputTypes, 
									 profileSettings profile, debugInfo& options, std::string& warnings )
{
	// declare some variables
	// first level is file the name was found in, second level is the name inside that file.
	niawgPair<niawgPair<std::string>> externalScriptNames, externalWaveformNames;
	niawgPair<std::string> waitSamples, importTypes;
	niawgPair<std::fstream> externalWaveformFiles;

	// work with marker events
	if (inputTypes[Vertical] == "create marker event")
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
	double waveSize = time * NIAWG_SAMPLE_RATE;
	// round to an integer.
	return (long)(waveSize + 0.5);
}


/**
* This function takes in the data for a single waveform and calculates all if the waveform's data points, and returns a pointer to an array containing
* these data points.
*/
void NiawgController::calcWaveData(channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, double time)
{
	// Declarations
	std::vector<double> powerPos, freqRampPos, phasePos(inputData.signals.size() );
	std::vector<double*> powerRampFileData, freqRampFileData;
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
			powerRampFileData.push_back(new double[sampleNum]);
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
				if (powerValNumber != sampleNum + 1)
				{
					thrower( "ERROR: file not the correct size?\nSize of upload is " + str( powerValNumber ) 
							 + "; size of file is " + str( sampleNum ));
				}
				// close the file.
				powerRampFile.close();
			}
			else
			{
				thrower("ERROR: ramp type " + str(inputData.signals[signal].powerRampType) + " is unrecognized. If "
						"this is a file name, make sure the file exists and is in the project folder. ");
			}
		}
		// If the ramp type isn't a standard command...
		if (inputData.signals[signal].freqRampType != "lin" && inputData.signals[signal].freqRampType != "nr"
			 && inputData.signals[signal].freqRampType != "tanh")
		{
			// try to open it
			freqRampFileData.push_back(new double[sampleNum]);
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
				if (freqRampValNum != sampleNum + 1)
				{
					thrower("ERROR: file not the correct size?\nSize of upload is " + str(freqRampValNum) 
							 + "; size of file is " + str( sampleNum ));
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
	for (UINT signal = 0; signal < inputData.signals.size(); signal++)
	{
		deltaOmega.push_back(2 * PI * (inputData.signals[signal].freqFin - inputData.signals[signal].freqInit));
	}
	///		Get Data Points.		///
	// initialize signalInc before the loop so that I have access to it afterwards.
	int sample = 0;
	/// increment through all samples
	for (; sample < sampleNum; sample++)
	{
		// calculate the time that this sample number refers to
		double curTime = (double)sample / NIAWG_SAMPLE_RATE;
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
				phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit * curTime + deltaOmega[signal] * pow( curTime, 2 ) / (2 * time)
									 + inputData.signals[signal].initPhase);
			}
			else if (inputData.signals[signal].freqRampType == "tanh")
			{
				// We want
				// W{t} = Wi + (DeltaW * (Tanh{-4 + 8 * t' / Tf} - Tanh{-4}) / (Tanh{4} - Tanh{-4})
				// This gives the correct initial value, final value, and symmetry of the ramp for the frequency.
				// -4 was picked somewhat arbitrarily. The integral of this is
				// dw/(2*tanh{4}) * T_f/8 * ln{cosh{-4+8t/T_f}} + (w_i - (dw * tanh{-4})/2*tanh{4}) t + C
				// Evaluating C to give the correct phase gives
				// phi{t} = (w_i+dw/2)t+(dw)/dtanh * T_f/8 * (ln{cosh{-4+8t/T_f}}-ln{cosh{-4}}) + phi_0
				// See onenote for more math.
				phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
								+ (deltaOmega[signal] / deltaTanh ) * (time / 8.0) * ( std::log(std::cosh(4 - (8 / time) * curTime))
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
				freqRampPos[signal] = freqRampFileData[signal][sample] * (inputData.signals[signal].freqFin 
																		   - inputData.signals[signal].freqInit);
				phasePos[signal] = (ViReal64)sample * 2 * PI * (inputData.signals[signal].freqInit + freqRampPos[signal]) / NIAWG_SAMPLE_RATE
								+ inputData.signals[signal].initPhase;
			}
			/// amplitude ramps are much simpler.
			if (inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr" 
				 && inputData.signals[signal].powerRampType
				 != "tanh")
			{
				// use data from file
				powerPos[signal] = powerRampFileData[signal][sample] * (inputData.signals[signal].finPower 
																		 - inputData.signals[signal].initPower);
			}
			else
			{
				// use the ramp calc function to find the current power.
				powerPos[signal] = NiawgController::rampCalc( sampleNum, sample, inputData.signals[signal].initPower,
															  inputData.signals[signal].finPower, 
															  inputData.signals[signal].powerRampType );
			}
		}

		/// If option is marked, then normalize the power.
		if ( CONST_POWER_OUTPUT )
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

	/// Calculate one last time for the final phases. I want the final phase to be the phase of the NEXT data point. 
	/// Then, following waveforms can START at this phase.
	double curTime = (double)sample / NIAWG_SAMPLE_RATE;
	for (auto signal : range( inputData.signals.size() ) )
	{
		// Calculate Phase Position. See above for description.
		if (inputData.signals[signal].freqRampType == "lin")
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + deltaOmega[signal] * pow(curTime, 2) * 1 / (2 * time) 
							+ inputData.signals[signal].initPhase;
		}
		else if (inputData.signals[signal].freqRampType == "tanh")
		{
			phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
				+ (deltaOmega[signal] / deltaTanh) * (time / 8.0) * std::log(std::cosh(4 - (8 / time) * curTime))
				- (deltaOmega[signal] / deltaTanh) * (time / 8.0) * std::log(std::cosh(4))
				+ inputData.signals[signal].initPhase;
		}
		else if (inputData.signals[signal].freqRampType == "nr")
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + inputData.signals[signal].initPhase;
		}
		else
		{
			freqRampPos[signal] = freqRampFileData[signal][sample] * (inputData.signals[signal].freqFin - inputData.signals[signal].freqInit);
			phasePos[signal] = (ViReal64)sample * 2 * PI * (inputData.signals[signal].freqInit + freqRampPos[signal]) / (NIAWG_SAMPLE_RATE)
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
}


/**
* This function takes two filled waveform arrays, and interweaves them into a new data array. this is required (strangely) by the NI card for outputting to
* both outputs separately.
*/
void NiawgController::mixWaveforms( simpleWave& waveCore )
{
	waveCore.waveVals.resize( 2 * waveCore.sampleNum );
	for (auto sample : range( waveCore.sampleNum))
	{
		// the order (Vertical -> Horizontal) here is important. Vertical is first because it's port zero on the Niawg. I believe that
		// switching the order here and changing nothing else would flip the output of the niawg..			
		waveCore.waveVals[2 * sample] = waveCore.chan[Vertical].wave[sample];
		waveCore.waveVals[2 * sample + 1] = waveCore.chan[Horizontal].wave[sample];
	}
}


/**
* this function takes in a command and checks it against all "logic commands", returing true if the inputted command matches a logic 
* command and false otherwise
*/
bool NiawgController::isLogic(std::string command)
{
	if (command == "waittiltrig" || command == "iftrig" || command == "repeattiltrig" 
		 || command == "waittilsoftwaretrig" || command == "ifsoftwaretrig" || command == "repeattilsoftwaretrig"
		 || command == "waitset#" || command == "repeatset#" || command == "repeatforever" || command == "endrepeat" 
		 || command == "else" || command == "endif")
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
 * loads data from file, does various checks on it, interprets input for variations.
 */
void NiawgController::loadWaveformParameters( NiawgOutputInfo& output, profileSettings profile, 
											  niawgPair<std::string> command, debugInfo& debug, 
											  niawgPair<ScriptStream>& scripts )
{
	simpleWave wave;
	// not sure why I have this limitation built in.
	if (output.isDefault && output.waves.size() == 1)
	{
		thrower( "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must "
				 "be a single waveform, not a sequence.\r\n" );
	}
	niawgPair<double> time;
	for (auto axis : AXES)
	{
		// Get a number corresponding directly to the given input type.
		loadStandardInputType( command[axis], wave.chan[axis] );
		// Initialize the variable counter inside the wave struct to zero:
		wave.chan[axis].varNum = 0;
		// infer the number of signals from the type assigned.
		if (wave.chan[axis].initType % MAX_NIAWG_SIGNALS == 0)
		{
			wave.chan[axis].signals.resize( MAX_NIAWG_SIGNALS );
		}
		else
		{
			wave.chan[axis].signals.resize( wave.chan[axis].initType % MAX_NIAWG_SIGNALS );
		}

		for (int signal = 0; signal < int(wave.chan[axis].signals.size()); signal++)
		{
			switch ((wave.chan[axis].initType - 1) / MAX_NIAWG_SIGNALS)
			{
				/// the case for "gen ?, const"
				case 0:
				{
					// set the initial and final values to be equal, and to not use a ramp, unless variable present.
					loadParam<double>( wave.chan[axis].signals[signal].freqInit, scripts[axis], 
									   wave.chan[axis].varNum, wave.chan[axis].varNames, 
									   wave.chan[axis].varTypes, { 5 * signal + 1, 5 * signal + 2 } );
					// Scale the frequencies to be in hertz. (input is MHz)
					wave.chan[axis].signals[signal].freqInit *= 1000000.;
					wave.chan[axis].signals[signal].freqFin = wave.chan[axis].signals[signal].freqInit;
					// Can't be varied for this case type
					wave.chan[axis].signals[signal].freqRampType = "nr";
					// set the initial and final values to be equal, and to not use a ramp, unless variable present.
					loadParam<double>( wave.chan[axis].signals[signal].initPower, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 3, 5 * signal + 4 } );
					wave.chan[axis].signals[signal].finPower = wave.chan[axis].signals[signal].initPower;
					// Can't be varied
					wave.chan[axis].signals[signal].powerRampType = "nr";
					// Get phase, unless varied.
					loadParam<double>( wave.chan[axis].signals[signal].initPhase, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 5 } );
					break;
				}
				/// The case for "gen ?, amp ramp"
				case 1:
				{
					// set the initial and final values to be equal, and to not use a ramp.
					loadParam<double>( wave.chan[axis].signals[signal].freqInit, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 1, 5 * signal + 2 } );
					// Scale the frequencies to be in hertz.(input is MHz)
					wave.chan[axis].signals[signal].freqInit *= 1000000.;
					wave.chan[axis].signals[signal].freqFin = wave.chan[axis].signals[signal].freqInit;
					// can't be varried.
					wave.chan[axis].signals[signal].freqRampType = "nr";
					std::string tempStr;
					scripts[axis] >> tempStr;
					wave.chan[axis].signals[signal].powerRampType = tempStr;
					loadParam<double>( wave.chan[axis].signals[signal].initPower, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 3 } );
					loadParam<double>( wave.chan[axis].signals[signal].finPower, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 4 } );
					loadParam<double>( wave.chan[axis].signals[signal].initPhase, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 5 } );
					break;
				}
				/// The case for "gen ?, freq ramp"
				case 2:
				{
					// get all parameters from the file
					std::string tempStr;
					scripts[axis] >> tempStr;
					std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), tolower );
					wave.chan[axis].signals[signal].freqRampType = tempStr;
					loadParam<double>( wave.chan[axis].signals[signal].freqInit, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 1 } );
					loadParam<double>( wave.chan[axis].signals[signal].freqFin, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 2 } );
							// Scale the frequencies to be in hertz.(input is MHz)
					wave.chan[axis].signals[signal].freqInit *= 1000000.;
					wave.chan[axis].signals[signal].freqFin *= 1000000.;
					// set the initial and final values to be equal, and to not use a ramp.
					loadParam<double>( wave.chan[axis].signals[signal].initPower, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 3, 5 * signal + 4 });
					wave.chan[axis].signals[signal].finPower = wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].powerRampType = "nr";
					loadParam<double>( wave.chan[axis].signals[signal].initPhase, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 5 } );
					break;
				}
				/// The case for "gen ?, freq & amp ramp"
				case 3:
				{
					// get all parameters from the file
					std::string tempStr;
					scripts[axis] >> tempStr;
					wave.chan[axis].signals[signal].freqRampType = tempStr;
					loadParam<double>( wave.chan[axis].signals[signal].freqInit, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 1 } );
					loadParam<double>( wave.chan[axis].signals[signal].freqFin, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 2 } );
					// Scale the frequencies to be in hertz.(input is MHz)
					wave.chan[axis].signals[signal].freqInit *= 1000000.;
					wave.chan[axis].signals[signal].freqFin *= 1000000.;

					// get all parameters from the file
					scripts[axis] >> tempStr;
					std::transform( tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower );
					wave.chan[axis].signals[signal].powerRampType = tempStr;
					loadParam<double>( wave.chan[axis].signals[signal].initPower, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 3 } );
					loadParam<double>( wave.chan[axis].signals[signal].finPower, scripts[axis], wave.chan[axis].varNum, 
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 4 } );
					loadParam<double>( wave.chan[axis].signals[signal].initPhase, scripts[axis], wave.chan[axis].varNum,
									   wave.chan[axis].varNames, wave.chan[axis].varTypes, { 5 * signal + 5 } );
				}
				break;
			}
		}
		// get the common things.
		loadParam<double>( time[axis], scripts[axis], wave.chan[axis].varNum, wave.chan[axis].varNames, 
						   wave.chan[axis].varTypes, { MAX_NIAWG_SIGNALS * 5 + 1 } );
		// Scale the time to be in seconds. (input is ms)
		time[axis] *= 0.001;
		loadParam<int>( wave.chan[axis].phaseOption, scripts[axis], wave.chan[axis].varNum, wave.chan[axis].varNames, 
						wave.chan[axis].varTypes, { MAX_NIAWG_SIGNALS * 5 + 2 } );
		scripts[axis] >> wave.chan[axis].delim;
		// check delimiter
		if (wave.chan[axis].delim != "#")
		{
			thrower( "ERROR: The delimeter is missing in the " + AXES_NAMES[axis] + " script file for waveform #"
					 + str( output.waves.size() - 1 ) + "The value placed in the delimeter location was " + wave.chan[axis].delim
					 + " while it should have been '#'. This indicates that either the code is not interpreting the user input "
					 "correctly or that the user has inputted too many parameters for this type of waveform." );
		}
	}
	// make sure the times match to nanosecond precision.
	if (!(fabs( time[Horizontal] - time[Vertical] ) < 1e-6))
	{
		thrower( "ERROR: the horizontal and vertical waveforms must have the same time value. They appear to be mismatched for waveform #"
				 + str( output.waves.size() - 1 ) + "!" );
	}
	wave.time = time[Horizontal];
	wave.sampleNum = waveformSizeCalc( wave.time );
	// figure out if the waveform varies
	// only create waveform data if the waveform is not being varried.
	wave.varies = false;
	for (auto axis : AXES)
	{
		if (wave.chan[axis].varNum > 0)
		{
			wave.varies = true;
		}
	}
	//	Handle -1 Phase (start with the phase that the previous waveform ended with)
	for (auto axis : AXES)
	{
		UINT count = 0;
		// loop through all signals in a the current waveform for a given axis.
		for (auto signal : wave.chan[axis].signals)
		{
			// If the user used a '-1' for the initial phase, this means the user wants to copy the ending phase of the previous waveform.
			if (signal.initPhase == -1)
			{
				if (count + 1 > output.waves[output.waves.size() - 1].core.chan[axis].signals.size())
				{
					thrower( "ERROR: You are trying to copy the phase of signal " + str( count + 1 ) + "  of " 
							 + AXES_NAMES[axis] + " waveform #" + str( output.waves.size() - 1 )
							 + ", but the previous waveform only had " 
							 + str( output.waves[output.waves.size() - 1].core.chan[axis].signals.size() ) + " signals!\n" );
				}
				// if you are trying to copy the phase from a waveform that is being varied, this can only be 
				// accomplished if this waveform is also varied. mark this waveform for varying and break.
				if (output.waves[output.waves.size() - 1].core.varies || wave.varies)
				{
					wave.varies = true;
					wave.chan[axis].varNum++;
					// the ' sign is reserved. It's just a place-holder here to make sure the number of varied 
					// waveforms gets understood properly.
					wave.chan[axis].varNames.push_back( "\'" );
					wave.chan[axis].varTypes.push_back( -1 );
					break;
				}

				if (output.waves[output.waves.size() - 1].isFlashing)
				{
					signal.initPhase = output.waves[output.waves.size() - 1].flash.flashWaves.front().chan[axis].signals[count].finPhase;
				}
				else
				{
					signal.initPhase = output.waves[output.waves.size() - 1].core.chan[axis].signals[count].finPhase;
				}
			}
			count++;
		}
	}
	output.waves.push_back( toWaveInfo( wave ));
}


// handles constant & ramping waveforms.
void NiawgController::handleStandardWaveform( NiawgOutputInfo& output, profileSettings profile, niawgPair<std::string> command,
											  niawgPair<ScriptStream>& scripts, debugInfo& options )
{
	loadWaveformParameters( output, profile, command, options, scripts );
	output.waves.back().core.name = "Waveform" + str( output.waves.size());
	if (!output.waves.back().core.varies)
	{
		// prepare the waveforms/
		finalizeStandardWave( output.waves.back().core, options );
		// allocate waveform into the device memory
		fgenConduit.allocateNamedWaveform( cstr(output.waves.back().core.name), output.waves.back().core.waveVals.size() / 2 );
		// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
		fgenConduit.writeNamedWaveform( cstr(output.waves.back().core.name), output.waves.back().core.waveVals.size(),
						   output.waves.back().core.waveVals.data() );
		// avoid memory leaks, but only if not default...
		if (output.isDefault)
		{
			defaultMixedWaveform = output.waves.back( ).core.waveVals;
			defaultWaveName = output.waves.back( ).core.name;
		}
		else
		{
			output.waves.back().core.waveVals.clear();
			output.waves.back().core.waveVals.shrink_to_fit();
		}

	}
	// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
	// have to rewrite the script to insert the new waveform name into it.
	output.niawgLanguageScript += "generate " + output.waves.back().core.name + "\n";
	// increment waveform count.
	//output.waveCount++;
}


void NiawgController::finalizeScript( ULONGLONG repetitions, std::string name, std::vector<std::string> workingUserScripts, 
									  std::vector<ViChar>& userScriptSubmit, bool repeatForever )
{
	// format the script to send to the 5451 according to the accumulation number and based on the number of sequences.
	std::string finalUserScriptString = "script " + name + "\n";
	if (repeatForever)
	{
		finalUserScriptString += "repeat forever\n";
		for (UINT sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
		{
			finalUserScriptString += workingUserScripts[sequenceInc];
		}
		finalUserScriptString += "end repeat\n";
	}
	else
	{
		// repeat the script once for every accumulation.
		for (UINT accumCount = 0; accumCount < repetitions; accumCount++)
		{
			for (UINT sequenceInc = 0; sequenceInc < workingUserScripts.size(); sequenceInc++)
			{
				finalUserScriptString += workingUserScripts[sequenceInc];
			}
		}
	}
	// the NIAWG requires that the script file must end with "end script".
	finalUserScriptString += "end Script";

	// Convert script string to ViConstString. +1 for a null character on the end.
	userScriptSubmit = std::vector<ViChar>( finalUserScriptString.begin(), finalUserScriptString.end() );
}


// handle flashing and streaming. Future: handle rearranging. Perhaps in the future, handle phase-managed waveforms 
// here too, but those might go better in standard waveform handling, not sure.
void NiawgController::handleSpecialWaveform( NiawgOutputInfo& output, profileSettings profile, 
											 niawgPair<std::string> commands, niawgPair<ScriptStream>& scripts, 
											 debugInfo& options, rearrangeParams rInfo )
{
	if (commands[Horizontal] != commands[Vertical])
	{
		thrower( "ERROR: Special waveform commands must match!" );
	}
	std::string command = commands[Horizontal];
	if (command == "flash")
	{
		waveInfo flashingWave;
		flashingWave.isFlashing = true;
		/// Load general flashing info from file
		niawgPair<std::string> waveformsToFlashInput;
		try
		{
			
			niawgPair<int> flashNum;
			for (auto axis : AXES)
			{
				scripts[axis] >> waveformsToFlashInput[axis];
				scripts[axis] >> flashingWave.flash.flashCycleFreqInput[axis];
				flashNum[axis] = std::stoi( waveformsToFlashInput[axis] );
			}
			if (flashNum[Horizontal] != flashNum[Vertical])
			{
				thrower( "ERROR: Flashing number didn't match between the horizontal and vertical files!" );
			}
			flashingWave.flash.flashNumber = flashNum[Horizontal];
		}
		catch (std::invalid_argument&)
		{
			thrower( "ERROR: flashing number failed to convert to an integer! This parameter cannot be varied." );
		}
		flashingWave.core.varies = true;
		try
		{
			niawgPair<double> flashCycleFreq, totalTime;
			for (auto axis : AXES)
			{
				scripts[axis] >> flashingWave.flash.totalTimeInput[axis];
				flashCycleFreq[axis] = std::stod( flashingWave.flash.flashCycleFreqInput[axis] );
				// convert to Hertz from Megahertz
				flashCycleFreq[axis] *= 1e6;
				
				totalTime[axis] = std::stod( flashingWave.flash.totalTimeInput[axis] );
				// convert to s from ms
				totalTime[axis] *= 1e-3;
			}
			
			flashingWave.core.varies = false;
			if (flashCycleFreq[Horizontal] != flashCycleFreq[Vertical])
			{
				thrower( "ERROR: Flashing cycle frequency didn't match between the horizontal and vertical files!" );
			}
			flashingWave.flash.flashCycleFreq = flashCycleFreq[Horizontal];

			if (totalTime[Horizontal] != totalTime[Vertical])
			{
				thrower( "ERROR: Flashing cycle frequency didn't match between the horizontal and vertical files!" );
			}
			flashingWave.core.time = totalTime[Horizontal];
		}
		catch (std::invalid_argument&) 
		{
			// that's fine, prob just means it varies. Will get caught later if it's not a variable. Check to make 
			// sure variables match.
			if (flashingWave.flash.flashCycleFreqInput[Horizontal] != flashingWave.flash.flashCycleFreqInput[Vertical])
			{
				thrower( "ERROR: Flashing cycle frequency didn't match between the horizontal and vertical files!" );
			}
			if (flashingWave.flash.totalTimeInput[Horizontal] != flashingWave.flash.totalTimeInput[Vertical])
			{
				thrower( "ERROR: Flashing cycle frequency didn't match between the horizontal and vertical files!" );
			}
		}
		// bracket
		for (auto axis : AXES)
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if (bracket != "{")
			{
				thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] 
						 + " File during flashing waveform read" );
			}
		}

		/// get waveforms to flash.
		NiawgOutputInfo flashingOutputInfo = output;
		for (auto waveCount : range( flashingWave.flash.flashNumber ))
		{
			niawgPair<std::string> flashingWaveCommands;
			// get the first input
			for (auto axis : AXES)
			{
				scripts[axis] >> flashingWaveCommands[axis];
				// handle trailing newline characters
				if (flashingWaveCommands[axis].length() != 0)
				{
					if (flashingWaveCommands[axis][flashingWaveCommands[axis].length() - 1] == '\r')
					{
						flashingWaveCommands[axis].erase( flashingWaveCommands[axis].length() - 1 );
					}
				}
			}

			if (flashingWaveCommands[Horizontal] == "}" || flashingWaveCommands[Vertical] == "}")
			{
				thrower( "ERROR: Expected " + str( flashingWave.flash.flashNumber ) + " waveforms for flashing but only found" 
						 + str(waveCount) );
			}
			if (!isStandardWaveform( flashingWaveCommands[Horizontal] ) || !isStandardWaveform( flashingWaveCommands[Vertical] ))
			{
				thrower( "ERROR: detected command in flashing section that does not denote a standard waveform (e.g. a"
					" logic command). This is not allowed!" );
			}
			loadWaveformParameters( flashingOutputInfo, profile, flashingWaveCommands, options, scripts );
			// add the new wave in flashingOutputInfo to flashingInfo structure
			flashingWave.flash.flashWaves.push_back( flashingOutputInfo.waves.back().core );
			// immediately kill the original waveforms here so as to reduce memory usage.
			flashingOutputInfo.waves.back().core.waveVals.clear();
			flashingOutputInfo.waves.back().core.waveVals.shrink_to_fit();
		}
		
		// make sure ends with }
		for (auto axis : AXES)
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if (bracket != "}")
			{
				thrower( "ERROR: Expected \"}\" but found " + bracket + " in" + AXES_NAMES[axis] + "File during flashing waveform read" );
			}
		}

		/// load these waveforms into the flashing info
		flashingWave.core.time = 0;
		double singleWaveTime = flashingWave.flash.flashWaves.front().time;

		for (UINT waveCount = 0; waveCount < flashingWave.flash.flashWaves.size(); waveCount++)
		{
			if (flashingWave.flash.flashWaves[waveCount].varies)
			{
				flashingWave.core.varies = true;
			}
			flashingWave.core.time += flashingWave.flash.flashWaves[waveCount].time;
		}
		
		//if (flashingWave.core.time != flashingWave.flash.flashNumber * singleWaveTime)
		//{
		//	thrower( "ERROR: somehow, the total time doesn't appear to be the number of flashing waveforms times the time of each waveform???" );
		//}
		
		if (!flashingWave.core.varies)
		{
			createFlashingWave( flashingWave, options );
			flashingWave.core.name = "Waveform" + str(output.waves.size()+1);			
			output.waves.push_back(flashingWave);
			fgenConduit.allocateNamedWaveform( cstr(output.waves.back().core.name), 
											   long(output.waves.back().core.waveVals.size() / 2) );
			// write named waveform on the device. Now the device knows what "waveform0" refers to when it sees it in 
			// the script. 
			fgenConduit.writeNamedWaveform(cstr(output.waves.back().core.name),
											long(output.waves.back().core.waveVals.size()),
											output.waves.back().core.waveVals.data() );
			writeToFile( 0, output.waves.back( ).core.waveVals );
			// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
			// have to rewrite the script to insert the new waveform name into it.
			output.niawgLanguageScript += "generate " + output.waves.back().core.name + "\n";
		}
 	}
	else if (command == "stream")
	{
		/// bracket
		for (auto axis : AXES)
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if (bracket != "{")
			{
				thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] + " File during flashing waveform read" );
			}
		}
		/// get waveforms to flash.
		NiawgOutputInfo streamInfo = output;
		waveInfo info;
		info.isStreamed = true;
		niawgPair<std::string> streamCommand;
		// get the first input
		for (auto axis : AXES)
		{
			scripts[axis] >> streamCommand[axis];
			// handle trailing newline characters
			if (streamCommand[axis].length() != 0)
			{
				if (streamCommand[axis][streamCommand[axis].length() - 1] == '\r')
				{
					streamCommand[axis].erase( streamCommand[axis].length() - 1 );
				}
			}
		}

		if (streamCommand[Horizontal] == "}" || streamCommand[Vertical] == "}")
		{
			thrower( "ERROR: Expected waveform for flashing didn't find it! " );
		}
		if (!isStandardWaveform( streamCommand[Horizontal] ) || !isStandardWaveform( streamCommand[Vertical] ))
		{
			thrower( "ERROR: detected command in flashing section that does not denote a standard waveform (e.g. a logic command or a "
					 "pre-written system). This is not allowed!" );
		}

		loadWaveformParameters( streamInfo, profile, streamCommand, options, scripts );

		/// bracket
		for (auto axis : AXES)
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if (bracket != "}")
			{
				thrower( "ERROR: Expected \"}\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] + " File during flashing waveform read" );
			}
		}

		finalizeStandardWave( streamInfo.waves.back().core, options );

		streamWaveformVals = streamInfo.waves.back().core.waveVals;
		// immediately kill the original waveforms here so as to reduce memory usage.
		streamInfo.waves.back().core.waveVals.clear();
		streamInfo.waves.back().core.waveVals.shrink_to_fit();

		//configureOutputEnabled( VI_FALSE );
		//abortGeneration();

		fgenConduit.allocateNamedWaveform( cstr( streamWaveName ), streamWaveformVals.size() );
		//streamWaveHandle = allocateUnNamedWaveform( streamWaveformVals.size() );

		// output must be off in order to set the streaming waveform handle. I will need a better way of handling this.

		// tell the niawg which waveform is streamed.
		//setViInt32Attribute( NIFGEN_ATTR_STREAMING_WAVEFORM_HANDLE, streamWaveHandle );
		// get the name of the waveform. Now this can be used in the script sent to the niawg.
		//streamWaveName = getViStringAttribute(NIFGEN_ATTR_STREAMING_WAVEFORM_NAME);

		//restartDefault();

		// 3) Fill the Streaming Memory Buffer with data using niFgen_WriteBinary16Waveform and Streaming Waveform Handle
		output.niawgLanguageScript += "generate " + streamWaveName + "\n";
		// the niawg will expect to have waveform in its stream buffer when this runs.
		waveInfo tempInfo;
		tempInfo.isStreamed = true;
		tempInfo.core.name = streamWaveName;
		output.waves.push_back( tempInfo );
	}
	else if (command == "rearrange")
	{
		waveInfo rearrangeWave;
		rearrangeWave.rearrange.timePerMove = rInfo.moveSpeed;
		rearrangeWave.isRearrangement = true;
		// the following two options are for simple flashing and simple streaming, not rearrangement, although 
		// rearrangment technically involves both
		rearrangeWave.isFlashing = false;
		rearrangeWave.isStreamed = false;
		/// bracket
		for (auto axis : AXES)
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if (bracket != "{")
			{
				thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] + " File during flashing waveform read" );
			}
		}
		/// get pic dims
		// get the dimensions of the target picture.
		niawgPair<UINT> rows, cols;
		for (auto axis : AXES)
		{
			std::string temp;
			scripts[axis] >> temp;
			rows[axis] = std::stoi( temp );
			scripts[axis] >> temp;
			cols[axis] = std::stoi( temp );
		}
		// check consistency. Should probably verify that this is consistent with actual picture?
		if (rows[Horizontal] != rows[Vertical] || cols[Horizontal] != cols[Vertical])
		{
			thrower( "ERROR: Target Picture size doesn't match between horizontal and vertical scripts!" );
		}
		rearrangeWave.rearrange.targetRows = rows[Horizontal];
		rearrangeWave.rearrange.targetCols = cols[Horizontal];
		/// get calibration parameters.
		// these are the frequencies that the niawg would need to output to reach the lower left corner (I think?) of 
		// the picture.
		niawgPair<double> lowestHorFreq, lowestVertFreq, freqPerPixel;
		for (auto axis : AXES)
		{
			std::string tempStr;
			scripts[axis] >> tempStr;
			lowestHorFreq[axis] = std::stod(tempStr);
			scripts[axis] >> tempStr;
			lowestVertFreq[axis] = std::stod(tempStr);
			scripts[axis] >> tempStr;
			freqPerPixel[axis] = std::stod( tempStr );
		}
		if (lowestHorFreq[Horizontal] != lowestHorFreq[Vertical] ||
			 lowestVertFreq[Horizontal] != lowestVertFreq[Vertical] ||
			 freqPerPixel[Horizontal] != freqPerPixel[Vertical])
		{
			thrower( "ERROR: Rearrangement parameters don't match between horizontal and vertical files!" );
		}
		rearrangeWave.rearrange.lowestFreq[Horizontal] = lowestHorFreq[Horizontal];
		rearrangeWave.rearrange.lowestFreq[Vertical] = lowestVertFreq[Horizontal];
		rearrangeWave.rearrange.freqPerPixel = freqPerPixel[Horizontal];
		/// get static pattern
		// this is the pattern that holds non-moving atoms in place. The algorithm calculates the moves, and then mixes
		// those moves with this waveform, but this waveform is always static.
		niawgPair<std::string> holdingCommands;
		// get the first input
		for (auto axis : AXES)
		{
			scripts[axis] >> holdingCommands[axis];
			// handle trailing newline characters
			if (holdingCommands[axis].length() != 0)
			{
				if (holdingCommands[axis][holdingCommands[axis].length() - 1] == '\r')
				{
					holdingCommands[axis].erase( holdingCommands[axis].length() - 1 );
				}
			}
		}
		if (!isStandardWaveform( holdingCommands[Horizontal] ) || !isStandardWaveform( holdingCommands[Vertical] ))
		{
			thrower( "ERROR: detected command in flashing section that does not denote a standard waveform (e.g. a "
					 "logic command or something special). This is not allowed!" );
		}
		// don't want to add directly 
		NiawgOutputInfo rearrangementOutputInfo = output;
		loadWaveformParameters( rearrangementOutputInfo, profile, holdingCommands, options, scripts );
		// this wave only needs to be written once.
		finalizeStandardWave( rearrangementOutputInfo.waves.back( ).core, options );
		//finalizeStandardWave( wave.flash.flashWaves[waveInc], options );
		// add the new wave in flashingOutputInfo to flashingInfo structure
		rearrangeWave.rearrange.staticWave = rearrangementOutputInfo.waves.back().core;

		/// get the target picture
		niawgPair<std::vector<std::vector<bool>>> targetTemp;
		// get the target picture. The picture must be replicated in each file.
		for (auto axis : AXES)
		{
			targetTemp[axis].resize( rows[axis] );
			for (auto rowInc : range( rows[axis] ))
			{
				targetTemp[axis][rowInc].resize( cols[axis] );
				std::string line = scripts[axis].getline( '\r' );
				ScriptStream lineScript( line );
				std::string singlePixelStatus;
				for (auto colInc : range( cols[axis] ))
				{
					lineScript >> singlePixelStatus;
					try
					{
						targetTemp[axis][rowInc][colInc] = bool( std::stoi( singlePixelStatus ) );
					}
					catch (std::invalid_argument&)
					{
						thrower( "ERROR: Failed to load the user's input for a rearrangement target picture! Loading failed"
								 " on this line: " + line + "\r\n");
					}
				}
			}
		}

		for (auto rowInc : range( rearrangeWave.rearrange.targetRows ))
		{
			for (auto colInc : range( rearrangeWave.rearrange.targetCols ))
			{
				if (targetTemp[Horizontal][rowInc][colInc] != targetTemp[Vertical][rowInc][colInc])
				{
					thrower( "ERROR: Target pictures didn't match between the Horizontal and Vertical Files!" );
				}
			}
		}
		rearrangeWave.rearrange.target = targetTemp[Horizontal];

		for (auto axis : AXES)
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if (bracket != "}")
			{
				thrower( "ERROR: Expected \"}\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] + " File during flashing waveform read." );
			}
		}
		// get the upper limit of the nuumber of moves that this could involve.
		rearrangeWave.rearrange.moveLimit = getMaxMoves( rearrangeWave.rearrange.target);
		output.waves.push_back( rearrangeWave );
		long samples = long( output.waves.back( ).rearrange.moveLimit
							 * output.waves.back( ).rearrange.timePerMove * NIAWG_SAMPLE_RATE );
		fgenConduit.allocateNamedWaveform( cstr( rearrangeWaveName ), samples );
		output.niawgLanguageScript += "generate " + rearrangeWaveName + "\n";
	}
	else
	{
		thrower( "ERROR: Bad waveform command!" );
	}
	//output.waveCount++;
}


void NiawgController::deleteRearrangementWave( )
{
	fgenConduit.deleteWaveform( cstr(rearrangeWaveName) );
}


// generic stream.
void NiawgController::streamWaveform()
{
	fgenConduit.writeNamedWaveform( cstr(streamWaveName), streamWaveformVals.size(), streamWaveformVals.data());
}

// expects the rearrangmenet waveform to have already been filled into rearrangeWaveVals.
void NiawgController::streamRearrangement()
{
	fgenConduit.writeNamedWaveform( cstr( rearrangeWaveName ), rearrangeWaveVals.size(), rearrangeWaveVals.data() );
}


// calculates the data, mixes it, and cleans up the calculated data.
void NiawgController::finalizeStandardWave( simpleWave& wave, debugInfo& options )
{
	// prepare each channel
	generateWaveform( wave.chan[Horizontal], options, wave.sampleNum, wave.time );
	generateWaveform( wave.chan[Vertical], options, wave.sampleNum, wave.time );
	// mix
	mixWaveforms( wave );
	// clear channel data, no longer needed.
	wave.chan[Vertical].wave.clear();
	// not sure if shrink_to_fit is necessary, but might help with data management
	wave.chan[Vertical].wave.shrink_to_fit();
	wave.chan[Horizontal].wave.clear();
	wave.chan[Horizontal].wave.shrink_to_fit();
}

// which should be Horizontal or Vertical.
void NiawgController::setDefaultWaveformScript( )
{
	fgenConduit.setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, cstr("DefaultConfigScript"));
}


/**
 * creates the mixed flashing wave from the wave dta in the flash part of the waveform. This is called when the user
 * deterministically uses the "flash" command in the script, but can also be used by the rearranging procedure, you
 * just need to fill in the waveInfo before-hand
 */
void NiawgController::createFlashingWave( waveInfo& wave, debugInfo options )
{
	/// quick check
	if (!wave.isFlashing)
	{
		thrower( "ERROR: tried to create flashing wave data for a waveform that wasn't flashing!" );
	}
	/// create the data for each wave that's gonna be flashed between.
	for (UINT waveInc = 0; waveInc < wave.flash.flashNumber; waveInc++)
	{
		finalizeStandardWave( wave.flash.flashWaves[waveInc], options );
	}
	mixFlashingWaves( wave, 0, 1 );
}


/** 
 * this is separated from the above function so that I can call it with pre-written waves
 */
void NiawgController::mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio )
{
	// firstDutyCycle is set to -1 if doing a non-rearranging waveform.
	if ( wave.flash.flashNumber > 2)
	{ 
		thrower( "ERROR: firstDutyCycle is set to a non-negative value (negative value is the dummy value for this "
				 "input case), but more than 2 flashing waveforms! This is considered undefined and an error." );
	}
	/// then mix them to create the flashing version.
	// total period time in seconds...
	double period = 1.0 / wave.flash.flashCycleFreq;
	// total period in samples...
	long totalPeriodInSamples = long( period * NIAWG_SAMPLE_RATE + 0.5 );
	long samplesPerWavePerPeriod = totalPeriodInSamples / wave.flash.flashNumber;
	// *2 because of mixing
	long deadSamples = deadTime * NIAWG_SAMPLE_RATE * 2;

	std::vector<long> samplesInWave( wave.flash.flashNumber );
	if ( wave.flash.flashNumber > 2 )
	{
		for ( auto& wave : samplesInWave )
		{
			wave = samplesPerWavePerPeriod;
		}
	}
	else
	{
		// static wave
		samplesInWave[1] = long( staticMovingRatio * totalPeriodInSamples / (1 + staticMovingRatio) );
		// moving wave
		samplesInWave[0] = totalPeriodInSamples - samplesInWave[1];
	}
	if (!(fabs( std::floor( wave.core.time / period ) - wave.core.time / period ) < 1e-9))
	{
		thrower( "ERROR: flashing cycle time doesn't result in an integer number of flashing cycles during the given waveform time!"
				 " This is not allowed currently." );
	}
	long cycles = long( std::floor( wave.core.time / period ) );
	/// mix the waves together
	wave.core.waveVals.resize( 2 * waveformSizeCalc( wave.core.time ) );
	//wave.core.waveVals.resize( wave.flash.flashWaves.front().waveVals.size() * wave.flash.flashNumber );
	for (auto cycleInc : range( cycles ))
	{
		UINT cycleSamples = cycleInc * wave.flash.flashNumber * 2 * samplesPerWavePerPeriod;
		UINT waveSamples = 0;
		for (auto waveInc : range( wave.flash.flashNumber ))
		{
			// samplesPerWavePerPeriod * 2 because need to account for the mixed nature of the waveform I'm adding.
			for (auto sampleInc : range( 2 * samplesInWave[waveInc] ))
			{
				int newSampleNum = sampleInc + waveSamples + cycleSamples;
				if ( sampleInc > 2 * samplesInWave[waveInc] - deadSamples )
				{
					// not in duty cycle, NIAWG is to output nothing.
					wave.core.waveVals[newSampleNum] = 0;
				}
				else
				{
					int sampleFromMixed = sampleInc + cycleInc * 2 * samplesInWave[waveInc];
					wave.core.waveVals[newSampleNum] = wave.flash.flashWaves[waveInc].waveVals[sampleFromMixed];
				}
			}
			waveSamples += 2 * samplesInWave[waveInc];
		}
	}

	/// cleanup
	// should be good now. Immediately delete the old waveforms. Wait until after all cycles done.
	for (auto waveInc : range( wave.flash.flashNumber ))
	{
		wave.flash.flashWaves[waveInc].waveVals.clear();
		wave.flash.flashWaves[waveInc].waveVals.shrink_to_fit();
	}
}


/**
 * this function takes in a command and checks it against all "generate commands", returing true if the inputted command matches a generate 
 * command and false otherwise
 * */
bool NiawgController::isStandardWaveform(std::string inputType)
{
	for ( auto number : range( MAX_NIAWG_SIGNALS ) )
	{
		if ( inputType == "gen" + str( number+1 ) + "const" || inputType == "gen" + str(number + 1) + "ampramp"
			 || inputType == "gen" + str(number + 1) + "freqramp" || inputType == "gen" + str(number + 1) + "freq&ampramp")
		{
			return true;
		}
	}
	return false;
}


bool NiawgController::isSpecialWaveform( std::string command )
{
	if (command == "flash" || command == "stream" || command == "rearrange")
	{
		return true;
	}
	return false;
}


/* * * * * *
 * this function takes in a command and checks it against all "special commands", returing true if the inputted command 
 * matches a special command and false otherwise
 * */
bool NiawgController::isSpecialCommand(std::string command)
{
	if (command == "predefined script" || command == "create marker event" || command == "predefined waveform")
	{
		return true;
	}
	return false;
}

// simple wrapper
void NiawgController::turnOff()
{
	fgenConduit.configureOutputEnabled(VI_FALSE);
	fgenConduit.abortGeneration();
}

// simple wrapper
void NiawgController::turnOn()
{
	fgenConduit.configureOutputEnabled(VI_TRUE);
	fgenConduit.initiateGeneration();
}


/* * * * * *
 * A series of sanity checks on the waveform parameters. This is ment to catch user error. The following checks for...
 * - Phase Continuity between waveforms (not checking repeating waveforms (yet))
 * - Amplitude Continuity between waveforms
 * - Frequency Continuity between waveforms
 * - Sensible Ramping Options (initial and final freq/amp values reflect choice of ramp or no ramp).
 * - Sensible Phase Correction Options
 * */
void NiawgController::checkThatWaveformsAreSensible(std::string& warnings, NiawgOutputInfo& output)
{
	for (auto axis : AXES)
	{
		for (UINT waveInc = 2; waveInc < output.waves.size(); waveInc++)
		{
			// if two waveforms have the same number of parameters... (elsewise its non-trivial to assume anything about what the user 
			// is doing)
			if (output.waves[waveInc].core.chan[axis].signals.size() == output.waves[waveInc-1].core.chan[axis].signals.size())
			{
				for (UINT signalNum = 0; signalNum < output.waves[waveInc].core.chan[axis].signals.size(); signalNum++)
				{
					waveSignal& currentSignal = output.waves[waveInc].core.chan[axis].signals[signalNum];
					waveSignal& previousSignal = output.waves[waveInc - 1].core.chan[axis].signals[signalNum];
					if (currentSignal.initPower != previousSignal.finPower)
					{
						warnings += "Warning: Amplitude jump at waveform #" + str( waveInc ) + " in " + AXES_NAMES[axis]
										 + " component detected!\r\n";
					}
					if (currentSignal.freqInit != previousSignal.freqFin)
					{
						warnings += "Warning: Frequency jump at waveform #" + str( waveInc ) + " in " + AXES_NAMES[axis]
										 + " component detected!\r\n";
					}
					if (currentSignal.initPhase - previousSignal.finPhase > CORRECTION_WAVEFORM_ERROR_THRESHOLD)
					{
						warnings += "Warning: Phase jump (greater than what's wanted for correction waveforms) at waveform #"
										 + str( waveInc ) + " in " + AXES_NAMES[axis] + " component detected!\r\n";
					}
					// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. It's not actually ramping.
					if (currentSignal.powerRampType != "nr" && (currentSignal.initPower == currentSignal.finPower))
					{
						warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to amplitude ramp, but "
										 "the initial and final amplitudes are the same. This is not a ramp.\r\n";
					}
					// if there signal is ramping but the beginning and end frequencies are the same, that's weird. It's not actually ramping.
					if (currentSignal.freqRampType != "nr" && (currentSignal.freqInit == currentSignal.freqFin))
					{
						warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to frequency ramp, but "
										 "the initial and final frequencies are the same. This is not a ramp.\r\n";
					}
					// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. It's not actually 
					// ramping.
					if (currentSignal.powerRampType == "nr" && (currentSignal.initPower != currentSignal.finPower))
					{
						warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to no amplitude ramp, "
										 "but the initial and final amplitudes are the different. This is not a ramp, the initial value "
										 "will be used.\r\n";
					}
					// if there signal is not ramping but the beginning and end frequencies are different, that's weird. It's not actually ramping.
					if (currentSignal.freqRampType == "nr" && (currentSignal.freqInit != currentSignal.freqInit))
					{
						warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to no frequency ramp, "
										 "but the initial and final frequencies are different. This is not a ramp, the initial value will "
										 "be used throughout.\r\n";
					}
					if (output.waves[waveInc].core.chan[axis].phaseOption != 0)
					{
						warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + " has a non-zero phase correction"
										 " option, but phase correction has been discontinued! This option being set won't do anything...";
					}
				}
			}
		}
	}
}


waveInfo NiawgController::toWaveInfo( simpleWave wave )
{
	waveInfo info;
	info.core.chan = wave.chan;
	info.core.name = wave.name;
	info.core.sampleNum = wave.sampleNum;
	info.core.time = wave.time;
	info.core.varies = wave.varies;
	info.core.waveVals = wave.waveVals;
	info.isFlashing = false;
	info.isRearrangement = false;
	info.isStreamed = false;
	return info;
}



/**
* This function is designed to replace getVartime in function. It uses a numerical, rather than mostly analytical, algorithm for finding a good time for a
* correction waveform. The algorithm increments through all possible sample numbers for a waveform (multiples of 4 and between the min and max values) and
* checks whether the phases of the waveforms are near zero to some tolerance.
* The algorithm:
* Take input data, create array of frequencies and array of phases.
* For all possible data points...
*	For all signals...
*	  Calculate phases at this point
*	For all signals...
*	  Check if phases are all near zero
*	  Throw flags if bad or terrible matching
*	If bad flag not thrown, break, a good time has been found.
*	Else, check if terrible flag thrown
*	If not, check if this is the best time so far, and if so, set best.
* If bad flag not thrown, continue happily. Exited well.
* Else if terrible flag not thrown, report best time and throw warning.
* Else throw error.
* Return phase mismatch.
*/
double NiawgController::calculateCorrectionTime( channelWave& wvData1, channelWave& wvData2,
												 std::vector<double> startPhases, std::string order, double time,
												 long sampleNum )
{
	std::vector<double> freqList;
	for (UINT signalInc = 0; signalInc < wvData1.signals.size(); signalInc++)
	{
		freqList.push_back( wvData1.signals[signalInc].freqInit );
	}
	for (UINT signalInc = 0; signalInc < wvData2.signals.size(); signalInc++)
	{
		freqList.push_back( wvData2.signals[signalInc].freqInit );
	}
	int testSampleNum;
	if (order == "before")
	{
		testSampleNum = 2;
	}
	else
	{
		testSampleNum = 0;
	}
	bool matchIsGood = true;
	bool matchIsOkay = true;
	bool foundOkayMatch = false;
	double bestTime = DBL_MAX;
	int bestSampleNum;
	double totalPhaseMismatch = PI;
	for (; (double)testSampleNum / NIAWG_SAMPLE_RATE < MAX_CORRECTION_WAVEFORM_TIME; testSampleNum += 4)
	{
		if ((double)testSampleNum / NIAWG_SAMPLE_RATE < MIN_CORRECTION_WAVEFORM_TIME)
		{
			continue;
		}
		std::vector<double> currentPhases;
		// calculate phases...
		for (UINT signalInc = 0; signalInc < freqList.size(); signalInc++)
		{
			// sin{omega*t+phi} = sin{2*PI*frequency*t+phi} = sin{2*PI*frequency*(currentSample / SampleRate) + phi}
			// need to modulate for 2*PI.
			double phase, phaseDif;
			phase = fmod( 2 * PI * freqList[signalInc] * ((double)testSampleNum / NIAWG_SAMPLE_RATE) + startPhases[signalInc], 2 * PI );
			if (phase > PI)
			{
				if (phase > 2 * PI)
				{
					// ERROR
					thrower( "ERROR! Bad location in calculateCorrectionTime." );
				}
				phaseDif = 2 * PI - phase;
			}
			else
			{
				phaseDif = phase;
			}
			currentPhases.push_back( phaseDif );
		}
		matchIsGood = true;
		matchIsOkay = true;
		for (UINT signalInc = 0; signalInc < freqList.size(); signalInc++)
		{
			if (currentPhases[signalInc] > CORRECTION_WAVEFORM_GOAL)
			{
				matchIsGood = false;
				if (currentPhases[signalInc] > CORRECTION_WAVEFORM_ERROR_THRESHOLD)
				{
					matchIsOkay = false;
				}
			}
		}
		if (matchIsGood)
		{
			bestTime = (double)testSampleNum / NIAWG_SAMPLE_RATE;
			bestSampleNum = testSampleNum;
			break;
		}
		else if (matchIsOkay)
		{
			double testTotalPhaseMismatch = 0;
			for (UINT signalInc = 0; signalInc < currentPhases.size(); signalInc++)
			{
				testTotalPhaseMismatch += currentPhases[signalInc];
			}
			if ((double)testTotalPhaseMismatch < totalPhaseMismatch)
			{
				foundOkayMatch = true;
				bestTime = testSampleNum / NIAWG_SAMPLE_RATE;
				bestSampleNum = testSampleNum;
				totalPhaseMismatch = testTotalPhaseMismatch;
			}
		}
	}
	if (!matchIsGood)
	{
		if (!foundOkayMatch)
		{
			// throw error
			return -1;
		}
		else
		{
			// set values
			if (order == "before")
			{
				time = (bestSampleNum - 2) / NIAWG_SAMPLE_RATE;
				sampleNum = bestSampleNum;
			}
			else
			{
				time = bestSampleNum / NIAWG_SAMPLE_RATE;
				sampleNum = bestSampleNum;
			}
			// throw warning!
			return totalPhaseMismatch;
		}
	}
	// set values
	if (order == "before")
	{
		time = (bestSampleNum - 2) / NIAWG_SAMPLE_RATE;
		sampleNum = bestSampleNum;
	}
	else
	{
		time = bestSampleNum / NIAWG_SAMPLE_RATE;
		sampleNum = bestSampleNum;
	}
	// if here, a good time must have been found.
return 0;
}

/**
* This function takes ramp-related information as an input and returns the "position" in the ramp (i.e. the amount to add to the initial value due to ramping)
* that the waveform should be at.
*
* @return double is the ramp position.
*
* @param size is the total size of the waveform, in numbers of samples
* @param iteration is the sample number that the waveform is currently at.
* @param initPos is the initial frequency or amplitude of the waveform.
* @param finPos is the final frequency or amplitude of the waveform.
* @param rampType is the type of ramp being executed, as specified by the reader.
*/
double NiawgController::rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType )
{
	// for linear ramps
	if ( rampType == "lin" )
	{
		return iteration * (finPos - initPos) / size;
	}
	// for no ramp
	else if ( rampType == "nr" )
	{
		return 0;
	}
	// for hyperbolic tangent ramps
	else if ( rampType == "tanh" )
	{
		return (finPos - initPos) * (tanh( -4 + 8 * (double)iteration / size ) + 1) / 2;
	}
	// error message. I've already checked (outside this function) whether the ramp-type is a filename.
	else
	{
		std::string errMsg = "ERROR: ramp type " + rampType + " is unrecognized. If this is a file name, make sure the file exists and is in the project folder.\r\n";
		errBox( errMsg );
		return 0;
	}
}
 

/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///								Rearrangement stuffs
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void NiawgController::startRearrangementThread( std::vector<std::vector<bool>>* atomQueue, waveInfo wave,
												Communicator* comm, std::mutex* rearrangerLock,
												chronoTimes* andorImageTimes, chronoTimes* grabTimes,
												std::condition_variable* rearrangerConditionWatcher,
												rearrangeParams rearrangeInfo)
{
	threadStateSignal = true;
	rearrangementThreadInput* input = new rearrangementThreadInput;
	input->info = rearrangeInfo;
	input->pictureTimes = andorImageTimes;
	input->grabTimes = grabTimes;
	input->rearrangerLock = rearrangerLock;
	input->threadActive = &threadStateSignal;
	input->comm = comm;
	input->niawg = this;
	input->atomsQueue = atomQueue;
	input->rearrangementWave = wave;
	input->rearrangerConditionWatcher = rearrangerConditionWatcher;

	UINT rearrangerId;
	rearrangerThreadHandle = (HANDLE)_beginthreadex( 0, 0, NiawgController::rearrangerThreadProcedure, (void*)input,
													 STACK_SIZE_PARAM_IS_A_RESERVATION, &rearrangerId );
	if ( !rearrangerThreadHandle )
	{
		errBox( "beginThreadEx error: " + str( GetLastError( ) ) );
	}
	if ( !SetThreadPriority( rearrangerThreadHandle, THREAD_PRIORITY_TIME_CRITICAL ) )
	{
		errBox( "Set Thread priority error: " + str( GetLastError( ) ) );
	}
}


bool NiawgController::rearrangementThreadIsActive( )
{
	return threadStateSignal;
}


void NiawgController::rearrange( )
{
	// calc pattern.
	// convert pattern to waveform data. 
	// stream data.
	// send software trigger.
}


void NiawgController::calculateRearrangingMoves( std::vector<std::vector<bool>> initArrangement )
{
	//... Kai's work
}


/// things that might make faster:
// preprogram all possible fillers
// 1D rearranging instead of 2D
// only look at select pixels
// faster moves
// preprogram all individual moves
// don't use vector
// make rearrangement algorithm work with flattened matrix
UINT __stdcall NiawgController::rearrangerThreadProcedure( void* voidInput )
{
	rearrangementThreadInput* input = (rearrangementThreadInput*)voidInput;
	std::vector<bool> triedRearranging;
	std::vector<double> calcTime, streamTime, triggerTime, resetPositionTime, picHandlingTime, picGrabTime;
	chronoTimes startCalc, stopCalc, stopReset, stopStream, stopTrigger;
	try
	{
		// wait for data
		while ( *input->threadActive )
		{
			std::vector<bool> tempAtoms;
			if ( input->atomsQueue->size( ) == 0 )
			{
				// wait for the next image using a condition_variable.
				std::unique_lock<std::mutex> locker( *input->rearrangerLock );
				input->rearrangerConditionWatcher->wait( locker );
				if ( !*input->threadActive )
				{
					break;
				}
				if ( input->atomsQueue->size( ) == 0)
				{
					input->comm->sendStatus( "Woke up?" );
					continue;
				}
			}
			{
				std::unique_lock<std::mutex> locker( *input->rearrangerLock );
				if ( input->atomsQueue->size( ) == 0 )
				{
					// spurious wake-up?
					input->comm->sendStatus( "Rearrangement Thread woke up???" );
					continue;
				}
				tempAtoms = (*input->atomsQueue)[0];
			}
			if ( tempAtoms.size( ) == 0 )
			{
				// spurious wake-up? This one probably never happens now that I've implemented the 
				// condition_variable.
				continue;
			}
			input->atomsQueue->erase( input->atomsQueue->begin( ) );
			startCalc.push_back(std::chrono::high_resolution_clock::now( ));			
			rearrangeInfo& info = input->rearrangementWave.rearrange;
			info.timePerMove = input->info.moveSpeed;
			info.flashingFreq = input->info.flashingRate;
			
			// right now I need to re-shape the atomqueue matrix. I should probably modify Kai's code to work with a 
			// flattened source matrix for speed.
			std::vector<std::vector<bool>> source;
			source.resize( info.targetRows );
			for ( auto& row : source )
			{
				row.resize(info.targetCols);
			}
			UINT count = 0;
			for ( auto colCount : range( info.targetCols ) )
			{
				for ( auto rowCount : range( info.targetRows ) )
				{
					source[source.size( ) - 1 - rowCount][colCount] = tempAtoms[count++];
				}
			}
			std::vector<simpleMove> operationsMatrix;
			try
			{
				rearrangement( source, info.target, operationsMatrix );
			}
			catch ( Error& )
			{
				// as of now, just ignore.
			}
			input->niawg->rearrangeWaveVals.clear( );
			/// program niawg
			debugInfo options;
			for ( auto move : operationsMatrix )
			{
				// program this move.
				double freqPerPixel = info.freqPerPixel;
				niawgPair<int> initPos = { move.initRow, move.initCol };
				niawgPair<int> finPos = { move.finRow, move.finCol };
				simpleWave moveWave;
				moveWave.varies = false;
				// not used bc not programmed directly.
				moveWave.name = "NA";
				// moveWave.time = input->rearrangementWave.rearrange.timePerMove / 2.0;
				// static wave
				//samplesInWave[1] = long( staticMovingRatio * totalPeriodInSamples / (1 + staticMovingRatio) );
				// moving wave
				//samplesInWave[0] = totalPeriodInSamples - samplesInWave[1];
				moveWave.time = input->rearrangementWave.rearrange.timePerMove / (input->info.staticMovingRatio + 1);
				moveWave.sampleNum = input->niawg->waveformSizeCalc( moveWave.time );
				UINT movingAxis, movingSize, staticAxis;
				if ( move.finCol != move.initCol )
				{
					movingAxis = Horizontal;
					staticAxis = Vertical;
					movingSize = info.targetCols;
				}
				else
				{
					movingAxis = Vertical;
					movingSize = info.targetRows;
					staticAxis = Horizontal;
				}
				double movingFrac = input->info.moveBias;
				double nonMovingFrac = (1 - movingFrac) / (movingSize - 2);
				
				/// handle moving axis
				// 1 less signal because of the two locations that the moving tweezer spans
				moveWave.chan[movingAxis].signals.resize( movingSize-1 );
				bool foundMoving = false;
				UINT gridLocation = 0;
				for ( auto signalNum : range(movingSize-1) )
				{
					waveSignal& sig = moveWave.chan[movingAxis].signals[signalNum];
					sig.powerRampType = "nr";
					sig.initPhase = 0;
					//
					if ( (signalNum == initPos[movingAxis] || signalNum == finPos[movingAxis]) && !foundMoving )
					{
						// SKIP the next one, which should be the next of the pair of locations that is moving.
						gridLocation++;
						// this is the moving signal. set foundmoving to true so that you only make one moving signal.
						foundMoving = true;
						sig.initPower = movingFrac;
						sig.finPower = movingFrac;
						sig.freqRampType = "lin";

						if ( movingAxis == Horizontal )
						{
							sig.freqInit = ((info.targetCols - initPos[movingAxis] - 1)
											 * freqPerPixel + info.lowestFreq[movingAxis]) * 1e6;
							sig.freqFin = ((info.targetCols - finPos[movingAxis] - 1)
											* freqPerPixel + info.lowestFreq[movingAxis]) * 1e6;
						}
						else
						{
							sig.freqInit = (initPos[movingAxis] * freqPerPixel + info.lowestFreq[movingAxis]) * 1e6;
							sig.freqFin = (finPos[movingAxis] * freqPerPixel + info.lowestFreq[movingAxis]) * 1e6;
						}
					}
					else
					{
						sig.initPower = nonMovingFrac;
						sig.finPower = nonMovingFrac;
						sig.freqRampType = "nr";
						if ( movingAxis == Horizontal )
						{
							sig.freqInit = ((info.targetCols - gridLocation - 1) * freqPerPixel 
											 + info.lowestFreq[movingAxis]) * 1e6;
							sig.freqFin = sig.freqInit;
						}
						else
						{
							sig.freqInit = (gridLocation * freqPerPixel + info.lowestFreq[movingAxis]) * 1e6;
							sig.freqFin = sig.freqInit;
						}
					}
					gridLocation++;
				}

				/// handle other axis
				moveWave.chan[staticAxis].signals.resize( 1 );
				waveSignal& sig = moveWave.chan[staticAxis].signals[0];
				// only matters for the horizontal AOM which gets the extra tone at the moment.
				sig.initPower = 1;
				sig.finPower = 1;
				sig.powerRampType = "nr";
				sig.initPhase = 0;
				sig.freqRampType = "nr";
				if ( staticAxis == Horizontal )
				{
					// convert to Hz
					sig.freqInit = ((info.targetCols - initPos[staticAxis]-1) * freqPerPixel 
									 + info.lowestFreq[staticAxis]) * 1e6;
					sig.freqFin = sig.freqInit;
				}
				else
				{
					// convert to Hz
					sig.freqInit = (initPos[staticAxis] * freqPerPixel + info.lowestFreq[staticAxis])*1e6;
					sig.freqFin = sig.freqInit;
				}
				/// finalize info & calc stuffs
				input->niawg->finalizeStandardWave( moveWave, options );
				// now put together into small temporary flashing wave
				waveInfo flashMove;
				flashMove.core.time = info.timePerMove;
				flashMove.isFlashing = true;
				flashMove.flash.flashNumber = 2;
				if ( fabs(info.staticWave.time + moveWave.time - info.timePerMove) > 1e-9  )
				{
					thrower( "ERROR: static wave and moving wave don't add up to the total time of the flashing wave! "
							 "Sizes were "+ str( info.staticWave.waveVals.size( ) ) + " and " 
							 + str( moveWave.waveVals.size( ) ) + " respectively.\r\n" );
				}
				flashMove.flash.flashWaves.push_back( moveWave );
				flashMove.flash.flashWaves.push_back( info.staticWave );
				flashMove.flash.flashCycleFreqInput = { str( info.flashingFreq ), str( info.flashingFreq ) };
				flashMove.flash.flashCycleFreq = info.flashingFreq;
				input->niawg->mixFlashingWaves( flashMove, input->info.deadTime, input->info.staticMovingRatio );
				// now add to main wave.
				input->niawg->rearrangeWaveVals.insert( input->niawg->rearrangeWaveVals.end( ), 
														flashMove.core.waveVals.begin( ),
														flashMove.core.waveVals.end( ) );
			}
			// fill out the rest of the waveform.
			simpleWave fillerWave = info.staticWave;
			fillerWave.time = (info.moveLimit - operationsMatrix.size( )) * info.timePerMove;
			fillerWave.sampleNum = input->niawg->waveformSizeCalc( fillerWave.time );
			input->niawg->finalizeStandardWave( fillerWave, options );
			input->niawg->rearrangeWaveVals.insert( input->niawg->rearrangeWaveVals.end( ), 
													fillerWave.waveVals.begin( ), fillerWave.waveVals.end( ) );
			stopCalc.push_back(chronoClock::now( ));
			input->niawg->streamRearrangement( );
			stopStream.push_back( chronoClock::now( ) );
			input->niawg->fgenConduit.sendSoftwareTrigger( );
			stopTrigger.push_back( chronoClock::now( ));
			input->niawg->fgenConduit.resetWritePosition( );
			stopReset.push_back( chronoClock::now( ));
			if ( operationsMatrix.size( ) )
			{
				triedRearranging.push_back( true );
			}
			else
			{
				triedRearranging.push_back( false );
			}
			input->niawg->rearrangeWaveVals.clear( );
			if ( operationsMatrix.size( ) != 0 )
			{
				input->comm->sendStatus( "Tried Moving. Calc Time = " 
										 + str( std::chrono::duration<double>( stopCalc.back() - startCalc.back( ) ).count( ))
										 + "\r\n");
			}
		}
		for ( auto inc : range( startCalc.size( ) ) )
		{
			/*
			streamTime.push_back( std::chrono::duration<double>( stopStream[inc] - stopCalc[inc] ).count( ) );
			triggerTime.push_back( std::chrono::duration<double>( stopTrigger[inc] - stopStream[inc] ).count( ) );
			calcTime.push_back( std::chrono::duration<double>( stopCalc[inc] - startCalc[inc] ).count( ) );
			resetPositionTime.push_back( std::chrono::duration<double>( stopReset[inc] - stopTrigger[inc] ).count( ) );
			picHandlingTime.push_back( std::chrono::duration<double>( startCalc[inc] - (*input->grabTimes)[inc] ).count() );
			picGrabTime.push_back( std::chrono::duration<double>( (*input->grabTimes)[inc] - (*input->pictureTimes)[inc]).count( ) );
			*/
		}
		/*
		(*input->pictureTimes).clear( );
		(*input->grabTimes).clear( );

		std::ofstream dataFile( TIMING_OUTPUT_LOCATION  + "rearrangementLog.txt" );
		dataFile 
				<< "PicHandlingTime "
				<< "PicGrabTime "
				<< "CalcTime "
				<< "ResetPositionTime "
				<< "StreamTime "
				<< "TriggerTime\n";
		if ( !dataFile.is_open( ) )
		{
			errBox( "ERROR: data file failed to open for rearrangement log!" );
		}
		for ( auto count : range( triedRearranging.size( ) ) )
		{
			dataFile 
					<< picHandlingTime [count] << " " 
					<< picGrabTime[count] << " " 
					<< calcTime[count] << " "
					<< resetPositionTime[count] << " " 
					<< streamTime[count] << " " 
					<< triggerTime[count] << "\n";
 		}
		dataFile.close( );
		*/
	}
	catch ( Error& err )
	{
		errBox( "ERROR in rearrangement thread! " + err.whatStr( ) );
	}
	try
	{
	}
	catch ( Error& err)
	{
		input->comm->sendError( "Failed to delete rearrangement waveform at end of rearranging!: " + err.whatStr( ) );
	}
	input->comm->sendStatus( "Exiting rearranging thread.\r\n" );
	delete input;
	return 0;
}


/// everything below here is primarily Kai-Niklas Schymik's work, with minor modifications. Some modifications are
/// minor to improve style consistency with my code, some are renaming variables so that I can make sense of what's 
/// going on. I also had to change it to make it compatible with non-square input.

int NiawgController::sign( int x )
{
	if (x > 0)
	{
		return 1;
	}
	else if (x < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


double NiawgController::minCostMatching( const std::vector<std::vector<double>> &cost, std::vector<int> &Lmate,
										 std::vector<int> &Rmate )
{
	int n = int( cost.size() );
	// construct dual feasible solution
	std::vector<double> u( n );
	std::vector<double> v( n );

	for (int i = 0; i < n; i++)
	{
		u[i] = cost[i][0];
		for (int j = 1; j < n; j++)
		{
			u[i] = min( u[i], cost[i][j] );
		}
	}

	for (int j = 0; j < n; j++)
	{
		v[j] = cost[0][j] - u[0];
		for (int i = 1; i < n; i++)
		{
			v[j] = min( v[j], cost[i][j] - u[i] );
		}
	}

	// construct primal solution satisfying complementary slackness
	Lmate = std::vector<int>( n, -1 );
	Rmate = std::vector<int>( n, -1 );
	int mated = 0;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (Rmate[j] != -1)
			{
				continue;
			}
			if (fabs( cost[i][j] - u[i] - v[j] ) < 1e-10)
			{
				Lmate[i] = j;
				Rmate[j] = i;
				mated++;
				break;
			}
		}
	}
	
	std::vector<double> dist( n );
	std::vector<int> dad( n ), seen( n );

	// repeat until primal solution is feasible
	while (mated < n)
	{
		// find an unmatched left node
		int s = 0;
		while (Lmate[s] != -1)
		{
			s++;
		}

		// initialize Dijkstra
		fill( dad.begin(), dad.end(), -1 );
		fill( seen.begin(), seen.end(), 0 );
		for (int k = 0; k < n; k++)
		{
			dist[k] = cost[s][k] - u[s] - v[k];
		}

		int j = 0;
		while (true)
		{
			// find closest
			j = -1;
			for (int k = 0; k < n; k++)
			{
				if (seen[k])
				{
					continue;
				}
				if (j == -1 || dist[k] < dist[j])
				{
					j = k;
				}
			}
			seen[j] = 1;

			// termination condition
			if (Rmate[j] == -1)
			{
				break;
			}

			// relax neighbors
			const int i = Rmate[j];

			for (int k = 0; k < n; k++)
			{
				if (seen[k])
				{
					continue;
				}

				const double new_dist = dist[j] + cost[i][k] - u[i] - v[k];

				if (dist[k] > new_dist)
				{
					dist[k] = new_dist;
					dad[k] = j;
				}
			}
		}

		// update dual variables
		for (int k = 0; k < n; k++)
		{
			if (k == j || !seen[k])
			{
				continue;
			}

			const int i = Rmate[k];
			v[k] += dist[k] - dist[j];
			u[i] -= dist[k] - dist[j];
		}

		u[s] += dist[j];
		// augment along path
		while (dad[j] >= 0)
		{
			const int d = dad[j];
			Rmate[j] = Rmate[d];
			Lmate[Rmate[j]] = j;
			j = d;
		}
		Rmate[j] = s;
		Lmate[s] = j;
		mated++;
	}
	double value = 0;

	for (int i = 0; i < n; i++)
	{
		value += cost[i][Lmate[i]];
	}
	return value;
}


double NiawgController::rearrangement( const std::vector<std::vector<bool>> & sourceMatrix,
									   const std::vector<std::vector<bool>> & targetMatrix,
									   std::vector<simpleMove> & operationsMatrix )
{
	// I am sure this might be also included directly after evaluating the image, but for safety
	// I also included it here
	int numberTargets = 0;
	int numberSources = 0;
	for (UINT rowInc = 0; rowInc < sourceMatrix.size(); rowInc++)
	{
		for (UINT colInc = 0; colInc < sourceMatrix[rowInc].size(); colInc++)
		{
			if (targetMatrix[rowInc][colInc] == 1)
			{
				numberTargets++;
			}
			if (sourceMatrix[rowInc][colInc] == 1)
			{
				numberSources++;
			}
		}
	}
	// Throw, if  less atoms than targets! myexception class defined above
	if (numberSources < numberTargets)
	{
		thrower( "Less atoms than targets!\nN source: " + str( numberSources ) + ", matrixSize target: " + str( numberTargets ) );
	}

	//------------------------------------------------------------------------------------------
	//calculate cost matrix from Source and Targetmatrix
	//------------------------------------------------------------------------------------------
	// Cost matrix. Stores path length for each source atom to each target position
	std::vector<std::vector<double> > costMatrix( numberSources, std::vector<double>( numberSources, 0 ) );
	// Indices of atoms in initial config
	std::vector<std::vector<int> > SourceIndice( numberSources, std::vector<int>( 2, 0 ) );
	// Indices of atoms in final config
	std::vector<std::vector<int> > TargetIndice( numberTargets, std::vector<int>( 2, 0 ) );

	//Find out the indice
	int sourcecounter = 0;
	int targetcounter = 0;
	for (UINT rowInc = 0; rowInc < sourceMatrix.size(); rowInc++)
	{
		for (UINT columnInc = 0; columnInc < sourceMatrix[rowInc].size(); columnInc++)
		{
			if (sourceMatrix[rowInc][columnInc] == 1)
			{
				SourceIndice[sourcecounter][0] = rowInc;
				SourceIndice[sourcecounter][1] = columnInc;
				sourcecounter++;
			}
			if (targetMatrix[rowInc][columnInc] == 1)
			{
				TargetIndice[targetcounter][0] = rowInc;
				TargetIndice[targetcounter][1] = columnInc;
				targetcounter++;
			}
		}
	}
	double pathlength = 0;
	// Now compute the pathlengths
	for (int sourceInc = 0; sourceInc < numberSources; sourceInc++)
	{
		for (int targetInc = 0; targetInc < numberTargets; targetInc++)
		{
			costMatrix[sourceInc][targetInc] = abs( SourceIndice[sourceInc][0] - TargetIndice[targetInc][0] )
				+ abs( SourceIndice[sourceInc][1] - TargetIndice[targetInc][1] );

			pathlength += costMatrix[sourceInc][targetInc];
		}
	}

	//------------------------------------------------------------------------------
	//Use MinCostMatching algorithm
	//------------------------------------------------------------------------------
	//input for bipartite matching algorithm, Algorithm writes into these vectors
	std::vector<int> left;
	std::vector<int> right;

	//The returned cost is the travelled distance
	double cost = minCostMatching( costMatrix, left, right );

	//------------------------------------------------------------------------------
	//calculate the operationsMatrix
	//------------------------------------------------------------------------------

	//First resize operationsMatrix, empty in code, but now we now how many entrys: cost!
	operationsMatrix.resize( cost, { 0,0,0,0 } );

	std::vector<std::vector<int> > matching( numberTargets, std::vector<int>( 4, 0 ) );
	//matching matrix, numberTargets x 4, Source and Target indice in each row
	for (int i = 0; i < numberTargets; i++)
	{
		matching[i][0] = SourceIndice[right[i]][0];
		matching[i][1] = SourceIndice[right[i]][1];
		matching[i][2] = TargetIndice[i][0];
		matching[i][3] = TargetIndice[i][1];
	}

	int step_x, step_y, init_x, init_y;
	int counter = 0;

	// Setting up the operationsMatrix (only elementary steps) from the matching matrix (source - target)
	for (int targetInc = 0; targetInc < numberTargets; targetInc++)
	{
		step_x = matching[targetInc][2] - matching[targetInc][0];
		step_y = matching[targetInc][3] - matching[targetInc][1];
		init_x = matching[targetInc][0];
		init_y = matching[targetInc][1];
		for (int xStepInc = 0; xStepInc < abs( step_x ); xStepInc++)
		{
			operationsMatrix[counter].initRow = init_x;
			operationsMatrix[counter].initCol = init_y;
			operationsMatrix[counter].finRow = init_x + sign( step_x );
			operationsMatrix[counter].finCol = init_y;
			init_x = init_x + sign( step_x );
			counter++;
		}
		for (int yStepInc = 0; yStepInc < abs( step_y ); yStepInc++)
		{
			operationsMatrix[counter].initRow = init_x;
			operationsMatrix[counter].initCol = init_y;
			operationsMatrix[counter].finRow = init_x;
			operationsMatrix[counter].finCol = init_y + sign( step_y );
			init_y = init_y + sign( step_y );
			counter++;
		}
	}
	// travelled distance
	return cost; 
}


void NiawgController::writeToFile( UINT fileNum, std::vector<double> waveVals )
{
	std::ofstream file( NIAWG_WAVEFORM_OUTPUT_LOCATION + "Wave_" + str( fileNum ) + ".txt" );
	if ( !file.is_open( ) )
	{
		thrower( "ERROR: Niawg wave file failed to open!" );
	}
	for ( auto val : waveVals )
	{
		file << val << " ";
	}
	file.close( );
}


//How does this algorithm work? Simple Flow algorithm that makes sure that all moves will be done regardless of the order!
//while loop until the operationsMatrix (list of moves) is empty
//delete moves that will be done
//Look at all moves that go from row to row+1 (after that row->row-1,col->col+1,col->col-1)
//to find more than one atom in each row/column to move at the same time
//Be vary of duplicates, the move (init_row,init_col)->(fin_row,fin_col) might be done more than once during the whole 
//algorithm, but only move and delete it once at the same time
double NiawgController::parallelMoves( std::vector<std::vector<int>> operationsMatrix,
								  std::vector<std::vector<int>> source, double matrixSize,
								  std::vector<parallelMovesContainer> &moves )
{
	// Vector that should save indice of all the moves (operationmatrix) that are in a certain column/row
	std::vector<int> opM_ix;
	// vector that should save the moves, important to check for duplicates
	std::vector<std::vector<int> > selecteditems;
	// bool for avoiding duplicates
	bool check = true;
	// true if there is sth to move
	bool move;

	//Saves the number of moves that the parallization takes. One count if several atoms move from row to row+1
	int parallelMoveNumber = 0;
	while (operationsMatrix.size() != 0)
	{
		//First moves from row to row+1. Fill opM_ix with indice of the moves you want to make
		//Because the other for loops are similarily structured, I included the comments only here
		for (int row = 0; row < matrixSize; row++)
		{
			move = false;
			for (UINT i = 0; i < operationsMatrix.size(); i++)
			{
				if (operationsMatrix[i][0] == row && operationsMatrix[i][2] == row + 1)
				{
					check = true;
					//erase duplicates
					for (UINT k = 0; k != selecteditems.size(); k++)
					{
						if (selecteditems[k] == operationsMatrix[i])
						{
							check = false;
							break;
						}
					}
					if (check)
					{
						opM_ix.push_back( i );
						selecteditems.push_back( operationsMatrix[i] );
					}
				}
			}

			//From all the moves in operationsMatrix that go from row to row+1, select those that have a atom at the initial position
			//and have no atom at the final position!
			if (opM_ix.size() != 0)
			{
				for (unsigned k = opM_ix.size(); k-- > 0; )
				{
					//only move if there is an atom to move and if target is free!
					if (source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0
						 && source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0)
					{
						move = true;
					}
					else
					{
						opM_ix.erase( opM_ix.begin() + k );
						selecteditems.erase( selecteditems.begin() + k );
					}

				}
				if (move)
				{
					parallelMoveNumber++;
					// Save the moves
					moves.push_back( parallelMovesContainer() );
					moves.back().rowOrColumn = "row";
					moves.back().upOrDown = 1;
					moves.back().which_rowOrColumn = row;

					for (unsigned k = opM_ix.size(); k-- > 0;)
					{
						source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						moves.back().whichAtoms.push_back( operationsMatrix[opM_ix[k]][1] );

						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
			opM_ix.clear();
			selecteditems.clear();
		}

		//reset some values
		check = true;
		opM_ix.clear();
		selecteditems.clear();
		//Second moves from row to row-1
		for (unsigned row = matrixSize; row-- > 0;)
		{
			move = false;
			for (UINT i = 0; i < operationsMatrix.size(); i++)
			{
				if (operationsMatrix[i][0] == row && operationsMatrix[i][2] == row - 1)
				{
					check = true;
					//erase duplicates
					for (UINT k = 0; k != selecteditems.size(); k++)
					{
						if (selecteditems[k] == operationsMatrix[i])
						{
							check = false;
							break;
						}
					}
					if (check)
					{
						opM_ix.push_back( i );
						selecteditems.push_back( operationsMatrix[i] );
					}
				}
			}

			if (opM_ix.size() != 0)
			{
				for (UINT k = opM_ix.size(); k-- > 0; )
				{
					if (source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0
						 && source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0)
					{
						move = true;
					}
					else
					{
						opM_ix.erase( opM_ix.begin() + k );
						selecteditems.erase( selecteditems.begin() + k );
					}
				}
				if (move)
				{
					parallelMoveNumber++;
					moves.push_back( parallelMovesContainer() );
					moves.back().rowOrColumn = "row";
					moves.back().upOrDown = -1;
					moves.back().which_rowOrColumn = row;

					for (unsigned k = opM_ix.size(); k-- > 0;)
					{
						source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						moves.back().whichAtoms.push_back( operationsMatrix[opM_ix[k]][1] );

						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
			opM_ix.clear();
			selecteditems.clear();
		}

		//reset some values
		check = true;
		opM_ix.clear();
		selecteditems.clear();
		//Third moves from col to col+1
		for (int col = 0; col < matrixSize; col++)
		{
			move = false;
			for (UINT i = 0; i < operationsMatrix.size(); i++)
			{
				if (operationsMatrix[i][1] == col && operationsMatrix[i][3] == col + 1)
				{
					check = true;
					//erase duplicates
					for (int k = 0; k != selecteditems.size(); k++)
					{
						if (selecteditems[k] == operationsMatrix[i])
						{
							check = false;
							break;
						}
					}
					if (check)
					{
						opM_ix.push_back( i );
						selecteditems.push_back( operationsMatrix[i] );
					}
				}
			}

			if (opM_ix.size() != 0)
			{
				for (unsigned k = opM_ix.size(); k-- > 0; )
				{

					if (source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0
						 && source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0)
					{
						move = true;
					}
					else
					{
						opM_ix.erase( opM_ix.begin() + k );
						selecteditems.erase( selecteditems.begin() + k );
					}

				}
				if (move)
				{
					parallelMoveNumber++;
					moves.push_back( parallelMovesContainer() );
					moves.back().rowOrColumn = "column";
					moves.back().upOrDown = 1;
					moves.back().which_rowOrColumn = col;

					for (unsigned k = opM_ix.size(); k-- > 0;)
					{
						source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						moves.back().whichAtoms.push_back( operationsMatrix[opM_ix[k]][0] );
						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
			opM_ix.clear();
			selecteditems.clear();
		}

		// reset some values
		check = true;
		opM_ix.clear();
		selecteditems.clear();
		//Fourth moves from col to col-1
		for (unsigned col = matrixSize; col-- > 0;)
		{
			move = false;
			//get all elements in this row that move to row-1
			opM_ix.clear();
			selecteditems.clear();
			for (UINT i = 0; i < operationsMatrix.size(); i++)
			{
				if (operationsMatrix[i][1] == col && operationsMatrix[i][3] == col - 1) {
					check = true;
					//erase duplicates
					for (UINT k = 0; k != selecteditems.size(); k++)
					{
						if (selecteditems[k] == operationsMatrix[i])
						{
							check = false;
							break;
						}
					}
					if (check)
					{
						opM_ix.push_back( i );
						selecteditems.push_back( operationsMatrix[i] );
					}
				}
			}

			if (opM_ix.size() != 0)
			{
				for (unsigned k = opM_ix.size(); k-- > 0;)
				{
					if (source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0
						 && source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0)
					{
						move = true;
					}
					else
					{
						opM_ix.erase( opM_ix.begin() + k );
						selecteditems.erase( selecteditems.begin() + k );
					}
				}
				if (move)
				{
					parallelMoveNumber++;
					moves.push_back( parallelMovesContainer() );
					moves.back().rowOrColumn = "column";
					moves.back().upOrDown = -1;
					moves.back().which_rowOrColumn = col;

					for (unsigned k = opM_ix.size(); k-- > 0;) {
						//cout << "----------------------" << endl;
						source[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						source[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						moves.back().whichAtoms.push_back( operationsMatrix[opM_ix[k]][0] );

						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
		}
	}
	return parallelMoveNumber;
}


//Task was: Find out the maximum number of moves, by only knowing the Target Matrix configuration
//I added together the furthest distances from each target.
//Therefore it assumes there is no atom on each target
//This is really overestimating the number of moves, but it is a maximum
//Is overestimating the most if you have a very small target in a big lattice.
//If you wanted to scale it down, one idea might be to scale getMaxMoves with the filling fraction!
//Also: Not super fast because of nested for loops
UINT NiawgController::getMaxMoves( const std::vector<std::vector<bool>> targetmatrix )
{
	int targetNumber = 0;
	for (auto row : targetmatrix)
	{
		for (auto elem : row)
		{
			if (elem == 1)
			{
				targetNumber++;
			}
		}
	}

	std::vector<std::vector<int> >targetIndice( targetNumber, std::vector<int>( 2, 0 ) );
	UINT targetcounter = 0;
	for (UINT i = 0; i < targetmatrix.size(); i++)
	{
		for (UINT j = 0; j < targetmatrix[0].size(); j++)
		{
			if (targetmatrix[i][j] == 1)
			{
				targetIndice[targetcounter][0] = i;
				targetIndice[targetcounter][1] = j;
				targetcounter++;
			}
		}
	}
	UINT maxlength = 0, sumlength = 0, length = 0;
	for (UINT k = 0; k < targetcounter; k++)
	{
		for (UINT i = 0; i < targetmatrix.size(); i++)
		{
			for (UINT j = 0; j < targetmatrix[0].size(); j++)
			{
				length = abs( int(i) - targetIndice[k][0] ) + abs( int(j) - targetIndice[k][1] );
				if (length > maxlength)
				{
					maxlength = length;
				}
			}
		}
		sumlength += maxlength;
	}
	return sumlength;
}

