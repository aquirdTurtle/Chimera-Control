#include "stdafx.h"

#include "NiawgController.h"
#include "NiawgStructures.h"
#include "MasterManager.h"
#include "NiawgWaiter.h"

#include "MasterThreadInput.h"
#include "Matrix.h"

#include <boost/algorithm/string/replace.hpp>
#include <chrono>
#include <numeric>


NiawgController::NiawgController( UINT trigRow, UINT trigNumber ) : triggerRow( trigRow ), triggerNumber( trigNumber )
{
	// initialize rearrangement calibrations.
	// default value for bias calibrations is currently 0.5.
	// 3x6 calibration
	rerngContainer<double> moveBias3x6Cal( 3, 6, 0.45);
	
	moveBias3x6Cal( 1, 4, right ) = 0.38;

	moveBias3x6Cal( 0, 0, up ) = moveBias3x6Cal( 1, 0, down ) = 0.7;
	moveBias3x6Cal( 0, 1, up ) = moveBias3x6Cal( 1, 1, down ) = 0.52;
	moveBias3x6Cal( 0, 2, up ) = moveBias3x6Cal( 1, 2, down ) = 0.48;
	moveBias3x6Cal( 0, 3, up ) = moveBias3x6Cal( 1, 3, down ) = 0.45;
	moveBias3x6Cal( 0, 4, up ) = moveBias3x6Cal( 1, 4, down ) = 0.48;
	moveBias3x6Cal( 0, 5, up ) = moveBias3x6Cal( 1, 5, down ) = 0.75;
	// 
	moveBias3x6Cal( 2, 0, down ) = moveBias3x6Cal( 1, 0, up ) = 0.8;
	moveBias3x6Cal( 2, 1, down ) = moveBias3x6Cal( 1, 1, up ) = 0.5;
	moveBias3x6Cal( 2, 2, down ) = moveBias3x6Cal( 1, 2, up ) = 0.45;
	moveBias3x6Cal( 2, 3, down ) = moveBias3x6Cal( 1, 3, up ) = 0.48;
	moveBias3x6Cal( 2, 4, down ) = moveBias3x6Cal( 1, 4, up ) = 0.35;
	moveBias3x6Cal( 2, 5, down ) = moveBias3x6Cal( 1, 5, up ) = 0.7;
	
	moveBiasCalibrations.push_back( moveBias3x6Cal );
}


void NiawgController::initialize()
{
	// open up the files and check what I have stored.
	openWaveformFiles();
	/// Initialize the waveform generator via FGEN.
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
}


bool NiawgController::isOn( )
{
	return on;
}


UINT NiawgController::getNumberTrigsInScript( )
{
	return triggersInScript;
}


std::pair<UINT, UINT> NiawgController::getTrigLines( )
{
	return { triggerRow, triggerNumber };
}


// this function returns the coordinates of the maximum convolution.
niawgPair<ULONG> NiawgController::convolve( Matrix<bool> atoms, Matrix<bool> target )
{
	Matrix<ULONG> result( atoms.getRows() - target.getRows() + 1, atoms.getCols() - target.getCols() + 1, 0);
	niawgPair<ULONG> targetCoords;
	UINT bestMatch = 0;
	for ( auto startRowInc : range( result.getRows() ) )
	{
		for ( auto startColInc : range( result.getCols() ) )
		{
			// calcualte product
			Matrix<bool> subAtoms = atoms.submatrix(startRowInc, target.getRows(), startColInc, target.getCols());
			std::vector<ULONG> tmp( target.getCols( ) );
			std::transform( target.begin(), target.end(), subAtoms.begin(), tmp.begin(), 
							[]( auto& i, auto& j ) {return long( i*j ); } );
			UINT rating = std::accumulate( tmp.begin( ), tmp.end( ), 0 );
			if ( rating > bestMatch )
			{
				targetCoords[Horizontal] = startColInc;
				targetCoords[Vertical] = startRowInc;
			}
			if ( rating == target.getRows( ) * target.getCols( ) )
			{
				// perfect match was found. 
				return targetCoords;
			}
		}
	}
	return targetCoords;
}


void NiawgController::programNiawg( MasterThreadInput* input, NiawgOutput& output, std::string& warnings,
									UINT variation, UINT totalVariations,  std::vector<long>& variedMixedSize, 
									std::vector<ViChar>& userScriptSubmit )
{
	input->comm->sendColorBox( Niawg, 'Y' );
	input->niawg->handleVariations( output, input->variables, variation, variedMixedSize, warnings, input->debugOptions,
									totalVariations );
	if ( input->settings.dontActuallyGenerate ) { return; }

	// Restart Waveform
	input->niawg->turnOff( );
	input->niawg->programVariations( variation, variedMixedSize, output );
	input->niawg->fgenConduit.writeScript( userScriptSubmit );
	input->niawg->fgenConduit.setViStringAttribute( NIFGEN_ATTR_SCRIPT_TO_GENERATE, "experimentScript" );
	// initiate generation before telling the master. this is because scripts are supposed to be designed to sit on an 
	// initial waveform until the master sends it a trigger.
	input->niawg->turnOn( );
	for ( UINT waveInc = 2; waveInc < output.waves.size( ); waveInc++ )
	{
		output.waves[waveInc].core.waveVals.clear( );
		output.waves[waveInc].core.waveVals.shrink_to_fit( );
	}
	variedMixedSize.clear( );
	input->comm->sendColorBox( Niawg, 'G' );
}


bool NiawgController::outputVaries( NiawgOutput output )
{
	for ( auto wave : output.waves )
	{
		if ( wave.core.varies )
		{
			// if any wave varies...
			return true;
		}
	}
	return false;
}


void NiawgController::prepareNiawg( MasterThreadInput* input, NiawgOutput& output, seqInfo& expSeq, 
									std::string& warnings, std::vector<ViChar>& userScriptSubmit, 
									bool& foundRearrangement, rerngOptions rInfo, std::vector<variableType>& variables )
{
	input->comm->sendColorBox( Niawg, 'Y' );
	triggersInScript = 0;
	std::vector<std::string> workingUserScripts( input->seq.sequence.size( ) );
	// analyze each script in sequence.
	UINT count = 0;
	for (auto& seq : expSeq.sequence )
	{
		ScriptStream script;
		output.niawgLanguageScript = "";
		input->comm->sendStatus( "Working with configuraiton # " + str( count + 1 ) + " in Sequence...\r\n" );
		/// Create Script and Write Waveforms ////////////////////////////////////////////////////////////////////
		script << seq.niawgScript.rdbuf( );
		if ( input->debugOptions.outputNiawgHumanScript )
		{
			input->comm->sendDebug( boost::replace_all_copy( "NIAWG Human Script:\n" + script.str( )
															 + "\n\n", "\n", "\r\n" ) );
		}
		input->niawg->analyzeNiawgScript( script, output, input->profile, input->debugOptions, warnings, rInfo,
										  variables );
		workingUserScripts[count] = output.niawgLanguageScript;
		if ( input->thisObj->getAbortStatus( ) ) { thrower( "\r\nABORTED!\r\n" ); }
		count++;
	}
	input->comm->sendStatus( "Constant Waveform Preparation Completed...\r\n" );
	input->niawg->finalizeScript( input->repetitionNumber, "experimentScript", workingUserScripts, userScriptSubmit,
								  !input->niawg->outputVaries( output ) );
	if ( input->debugOptions.outputNiawgMachineScript )
	{
		input->comm->sendDebug( boost::replace_all_copy( "NIAWG Machine Script:\n"
														 + std::string( userScriptSubmit.begin( ), userScriptSubmit.end( ) )
														 + "\n\n", "\n", "\r\n" ) );
	}
}


// this function checks if should be rearranging and if so starts the thread.
void NiawgController::handleStartingRerng( MasterThreadInput* input, NiawgOutput& output )
{
	bool foundRearrangement = false;
	// check if any waveforms are rearrangement instructions.
	for ( auto& wave : output.waves )
	{
		if ( wave.rearrange.isRearrangement )
		{
			// if already found one...
			if ( foundRearrangement )
			{
				thrower( "ERROR: Multiple rearrangement waveforms found, but not allowed!" );
			}
			foundRearrangement = true;
			// start rearrangement thread. Give the thread the queue.
			input->niawg->startRerngThread( input->atomQueueForRearrangement, wave, input->comm, input->rearrangerLock,
											input->andorsImageTimes, input->grabTimes, 
											input->conditionVariableForRearrangement, input->rearrangeInfo, 
											input->analysisGrid );
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
	NiawgOutput output;
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
		rerngOptions rInfoDummy;
		rInfoDummy.moveSpeed = 0.00006;
		analyzeNiawgScripts( scripts, output, profile, debug, warnings, rInfoDummy, std::vector<variableType>() );
		writeStaticNiawg( output, debug, std::vector<variableType>( ) );
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
void NiawgController::cleanupNiawg( profileSettings profile, bool masterWasRunning, seqInfo& expSeq, 
									NiawgOutput& output, Communicator* comm, bool dontGenerate )
{
	// close things
	for ( auto& seqIndv : expSeq.sequence )
	{
		seqIndv.niawgScript.close( );
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


void NiawgController::waitForRerng( )
{
	int result = WaitForSingleObject( rerngThreadHandle, 500 );
	if ( result == WAIT_TIMEOUT )
	{
		thrower( "ERROR: waiting for Rearranger thread to finish timed out!?!?!?" );
	}
	try
	{
		deleteRerngWave( );
	}
	catch ( Error& )
	{

	}
}


void NiawgController::turnOffRerng( )
{
	// make sure the rearranger thread is off.
	threadStateSignal = false;
}


void NiawgController::restartDefault()
{
	try
	{
		// to be sure.
		turnOffRerng( );		
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


void NiawgController::programVariations( UINT variation, std::vector<long>& variedMixedSize, NiawgOutput& output )
{
	int mixedWriteCount = 0;
	// skip defaults so start at 2.
	for ( UINT waveInc = 2; waveInc < output.waves.size( ); waveInc++ )
	{
		std::string variedWaveformName = "Waveform" + str( waveInc );
		if ( output.waves[waveInc].core.varies )
		{
			if ( variation != 0 )
			{
				fgenConduit.deleteWaveform( cstr( variedWaveformName ) );
			}
			// And write the new one.
			fgenConduit.allocateNamedWaveform( cstr( variedWaveformName ), variedMixedSize[mixedWriteCount] / 2 );
			fgenConduit.writeNamedWaveform( cstr( variedWaveformName ), variedMixedSize[mixedWriteCount],
											output.waves[waveInc].core.waveVals.data( ) );
			mixedWriteCount++;
		}
	}
}



void NiawgController::analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, NiawgOutput& output,
										   profileSettings profile, debugInfo& options, std::string& warnings,
										   rerngOptions rInfo, std::vector<variableType>& variables )
{
	writeToFileNumber = 0;
	/// Preparation
	for ( auto& axis : AXES )
	{
		currentScripts[axis] = scripts[axis].str( );
		scripts[axis].clear( );
		scripts[axis].seekg( 0, std::ios::beg );
	}
	// Some declarations.
	niawgPair<std::string> command;
	// get the first input
	for ( auto axis : AXES )
	{
		scripts[axis] >> command[axis];
	}
	/// Analyze!
	while ( !(scripts[Vertical].peek( ) == EOF) && !(scripts[Horizontal].peek( ) == EOF) )
	{
		if ( isLogic( command[Horizontal] ) && isLogic( command[Vertical] ) )
		{
			handleLogic( scripts, command, output.niawgLanguageScript );
		}
		else if ( isSpecialCommand( command[Horizontal] ) && isSpecialCommand( command[Vertical] ) )
		{
			handleSpecialForm( scripts, output, command, profile, options, warnings );
		}
		else if ( isStandardWaveform( command[Horizontal] ) && isStandardWaveform( command[Vertical] ) )
		{
			handleStandardWaveformForm( output, profile, command, scripts, options, variables );
		}
		else if ( isSpecialWaveform( command[Horizontal] ) && isSpecialWaveform( command[Vertical] ) )
		{
			handleSpecialWaveformForm( output, profile, command, scripts, options, rInfo, variables );
		}
		else
		{
			thrower( "ERROR: Input types from the two files do not match or are unrecofgnized!\nBoth must be logic "
					 "commands, both must be generate commands, or both must be special commands. See documentation "
					 "on the correct format for these commands.\n\nThe two inputted types are: " + command[Vertical] 
					 + " and " + command[Horizontal] + " for waveform #" + str( output.waveFormInfo.size( ) - 1 ) + "!" );
		}
		// get next input.
		for ( auto axis : AXES )
		{
			scripts[axis] >> command[axis];
		}
	}
	output.waves.resize( output.waveFormInfo.size( ) );
}


void NiawgController::analyzeNiawgScript( ScriptStream& script, NiawgOutput& output, profileSettings profile, 
										  debugInfo& options, std::string& warnings, rerngOptions rInfo, 
										  std::vector<variableType>& variables )
{
	writeToFileNumber = 0;
	/// Preparation
	currentScript = script.str( );
	script.clear();
	script.seekg( 0, std::ios::beg );
	std::string command;
	// get the first input
	script >> command;

	/// Analyze!
	while ( script.peek( ) != EOF )
	{
		if ( isLogic( command ) )
		{
			handleLogicSingle( script, command, output.niawgLanguageScript );
		}
		else if ( isSpecialCommand( command ) )
		{
			handleSpecialFormSingle( script, output, command, profile, options, warnings );
		}
		else if ( isStandardWaveform( command ) )
		{
			handleStandardWaveformFormSingle( output, command, script, variables );
		}
		else if ( isSpecialWaveform( command ) )
		{
			handleSpecialWaveformFormSingle( output, profile, command, script, options, rInfo, variables );
		}
		else
		{
			thrower( "ERROR: Input niawg command is unrecognized!\nMust be logic commands, generate commands, or "
					 "special commands. See documentation on the correct format for these commands.\n\n"
					 "The inputted command is: " + command + " for waveform #" 
					 + str( output.waveFormInfo.size( ) - 1 ) + "!" );
		}
		script >> command;
	}
	output.waves.resize( output.waveFormInfo.size( ) );
}



void NiawgController::writeStaticNiawg( NiawgOutput& output, debugInfo& options, 
										std::vector<variableType>& constants )
{
	for ( auto& waveInc : range(output.waveFormInfo.size()) )
	{
		waveInfoForm& waveForm( output.waveFormInfo[ waveInc ] );
		waveInfo& wave( output.waves[waveInc] );
		if ( waveForm.flash.isFlashing )
		{
			// write static flashing
			flashVaries( waveForm );
			if ( !waveForm.core.varies )
			{
				flashFormToOutput( waveForm, wave );
				writeFlashing( wave, options, 0 );
			}
		}
		else if ( waveForm.rearrange.isRearrangement )
		{
			simpleFormVaries( waveForm.rearrange.staticWave );
			simpleFormVaries( waveForm.rearrange.fillerWave );
			// write static rearrangement
			if ( !wave.rearrange.staticWave.varies && !waveForm.rearrange.fillerWave.varies )
			{
				rerngFormToOutput( waveForm, wave, constants, 0 );
				// prepare the waveforms
				finalizeStandardWave( wave.rearrange.staticWave, options );
				finalizeStandardWave( wave.rearrange.fillerWave, options );
			}
			else
			{
				thrower( "ERROR: varying rearrangement static waves not supported!" );
			}
		}
		else
		{
			// this can handle streamed waveforms too.
			simpleFormVaries( waveForm.core );
			if ( !waveForm.core.varies )
			{
				simpleFormToOutput( waveForm.core, wave.core, constants, 0 );
				writeStandardWave( wave.core, options, output.isDefault );
			}
		}
	}
}

// this function stores whether the wave varies in the wave structure.
void NiawgController::simpleFormVaries(simpleWaveForm& wave )
{
	if ( wave.time.varies() )
	{
		wave.varies = true;
		return;
	}
	for ( auto& chan : wave.chan )
	{
		for ( auto& signal : chan.signals )
		{
			if ( signal.freqInit.varies( ) || signal.freqFin.varies( ) || signal.initPower.varies( ) 
				 || signal.finPower.varies( ) || signal.initPhase.varies( ) )
			{
				wave.varies = true;
				return;
			}
		}
	}
	wave.varies = false;
}


void NiawgController::simpleFormToOutput( simpleWaveForm& formWave, simpleWave& wave,
										  std::vector<variableType>& varibles, UINT variation )
{
	try
	{
		wave.varies = formWave.varies;
		wave.time = formWave.time.evaluate( varibles, variation ) * 1e-3;
		wave.sampleNum = waveformSizeCalc( wave.time );
		wave.name = formWave.name;
		for ( auto& chanInc : range(wave.chan.size()) )
		{
			wave.chan[chanInc].delim = formWave.chan[chanInc].delim;
			wave.chan[chanInc].initType = formWave.chan[chanInc].initType;
			wave.chan[chanInc].phaseOption = formWave.chan[chanInc].phaseOption;
			wave.chan[chanInc].signals.resize( formWave.chan[chanInc].signals.size( ) );
			for ( auto& signalInc : range(wave.chan[chanInc].signals.size()))
			{
				waveSignal& signal( wave.chan[chanInc].signals[signalInc] );
				waveSignalForm& signalForm( formWave.chan[chanInc].signals[signalInc] );
				signal.initPhase = signalForm.initPhase.evaluate( varibles, variation );
				signal.freqInit = signalForm.freqInit.evaluate( varibles, variation ) * 1e6;
				signal.freqFin = signalForm.freqFin.evaluate( varibles, variation ) * 1e6;
				signal.initPower = signalForm.initPower.evaluate( varibles, variation );
				signal.finPower = signalForm.finPower.evaluate( varibles, variation );
				signal.freqRampType = signalForm.freqRampType;
				signal.powerRampType = signalForm.powerRampType;
			}
		}
		wave.varies = false;
	}
	catch ( Error& err )
	{
		thrower( "Failed to convert simple niawg wave form to simple wave data! This might mean a low-level bug where"
				 " the code thought that a wave didn't vary, but it did.\r\nError was: " + err.whatStr( ) );
	}
}


void NiawgController::writeStandardWave(simpleWave& wave, debugInfo options, bool isDefault )
{
	// prepare the waveforms
	finalizeStandardWave( wave, options );
	// allocate waveform into the device memory
	fgenConduit.allocateNamedWaveform( cstr( wave.name ), wave.waveVals.size( ) / 2 );
	// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
	fgenConduit.writeNamedWaveform( cstr( wave.name ), wave.waveVals.size( ), wave.waveVals.data( ) );
	// avoid memory leaks, but only if not default...
	if ( isDefault )
	{
		defaultMixedWaveform = wave.waveVals;
		defaultWaveName = wave.name;
	}
	else
	{
		wave.waveVals.clear( );
		wave.waveVals.shrink_to_fit( );
	}
}


void NiawgController::handleSpecialWaveformFormSingle( NiawgOutput& output, profileSettings profile, std::string cmd,
													   ScriptStream& script, debugInfo& options, rerngOptions rInfo,
													   std::vector<variableType>& variables )
{
	if ( cmd == "flash" )
	{
		/*
		Format is:
		% Command
		flash
		% bracket
		{
		% number of waveforms to flash (e.g. 3 means flashing between wvfm1, 2, 3, 1, 2, 3, ...
		2
		% flashing frequency (MHz)
		1
		% move time (ms)
		0.1
		% dead time (ns)
		0
		% all flashing waveforms, written with normal syntax. Watch out, the times must be chosen to match the overall
		% move time.
		gen6Const
		%%%%%	freq		amp		phase
		49		2.5	 	5.13919
		58		1	 	0.384594
		67		1.05	 	0.308571
		76		1.1	 	0.050804
		85		1.4	 	6.154510
		94		2.25	 	3.858584
		%%%%%	time	p.m.	delim
		0.05	0	#

		% second flash waveform
		gen2FreqRamp
		%%%%%	f.RTKW freq_i freq_f amp	phase
		nr   58	58	0.92		0
		lin  76 	67  	0.08		0
		%%%%%	time	p.m.	delim
		0.05	0	#
		% closing bracket
		}
		*/

		// bracket
		std::string bracket;
		script >> bracket;
		if ( bracket != "{" )
		{
			thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in Niawg File during flashing waveform read" );
		}
		waveInfoForm flashingWave;
		flashingWave.flash.isFlashing = true;
		/// Load general flashing info from file
		try
		{
			std::string waveformsToFlashInput;
			script >> waveformsToFlashInput;
			flashingWave.flash.flashNumber = std::stoi( waveformsToFlashInput );
			script >> flashingWave.flash.flashCycleFreq;
			flashingWave.flash.flashCycleFreq.assertValid( variables );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: flashing number failed to convert to an integer! This parameter cannot be varied." );
		}
		script >> flashingWave.core.time;
		script >> flashingWave.flash.deadTime;
		flashingWave.flash.deadTime.assertValid( variables );
		/// get waveforms to flash.
		NiawgOutput flashOutInfo = output;
		for ( auto waveCount : range( flashingWave.flash.flashNumber ) )
		{
			simpleWaveForm wave;
			std::string flashWaveCmd;
			// get the first input
			script >> flashWaveCmd;
			if ( flashWaveCmd == "}" )
			{
				thrower( "ERROR: Expected " + str( flashingWave.flash.flashNumber ) + " waveforms for flashing but "
							"only found" + str( waveCount ) );
			}
			loadFullWave( flashOutInfo, flashWaveCmd, script, variables, wave );
			flashOutInfo.waveFormInfo.push_back( toWaveInfoForm( wave ) );
			// add the new wave in flashOutInfo to flashingInfo structure
			flashingWave.flash.flashWaves.push_back( flashOutInfo.waveFormInfo.back( ).core );
		}
		// make sure ends with }
		script >> bracket;
		if ( bracket != "}" )
		{
			thrower( "ERROR: Expected \"}\" but found " + bracket + " in niawg File during flashing waveform read" );
		}
		flashingWave.core.name = "Waveform" + str( output.waveFormInfo.size( ) + 1 );
		output.waveFormInfo.push_back( flashingWave );
		// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
		// have to rewrite the script to insert the new waveform name into it.
		output.niawgLanguageScript += "generate " + output.waveFormInfo.back( ).core.name + "\n";
	}
	else if ( cmd == "stream" )
	{
		// TODO... maybe. don't think I really need to.
	}
	else if ( cmd == "rearrange" )
	{
		/*				Format:
		rearrange
		{
		rows in target
		cols in target
		lowest hor freq
		lowest vert freq
		freq spacing (usually 9MHz)
		hold waveform (e.g. gen 6 const) horizontal
		hold waveform (e.g. gen 6 const) vertical
		target pattern
		target coordinates
		}
		*/
		waveInfoForm rearrangeWave;
		rearrangeWave.rearrange.timePerMove = str( rInfo.moveSpeed );
		rearrangeWave.rearrange.isRearrangement = true;
		// the following two options are for simple flashing and simple streaming, not rearrangement, even though
		// rearrangment technically involves both
		rearrangeWave.flash.isFlashing = false;
		rearrangeWave.isStreamed = false;
		/// bracket
		std::string bracket;
		script >> bracket;
		if ( bracket != "{" )
		{
			thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in niawg File during "
					 " flashing waveform read" );
		}
		/// get pic dims
		// get the dimensions of the target picture.
		UINT rows, cols;
		std::string temp;
		try
		{
			script >> temp;
			rows = std::stoi( temp );
			script >> temp;
			cols = std::stoi( temp );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: failed to convert target row and collumn numbers to integers during niawg script "
					 "analysis for rearrange command!" );
		}
		/// get calibration parameters.
		// these are the frequencies that the niawg would need to output to reach the lower left corner (I think?) of 
		// the picture.
		std::string tempStr;
		script >> tempStr;
		rearrangeWave.rearrange.lowestFreqs[Horizontal] = std::stod( tempStr );
		script >> tempStr;
		rearrangeWave.rearrange.lowestFreqs[Vertical] = std::stod( tempStr );
		script >> tempStr;
		rearrangeWave.rearrange.freqPerPixel = std::stod( tempStr );
		/// get static pattern
		// this is the pattern that holds non-moving atoms in place. The algorithm calculates the moves, and then mixes
		// those moves with this waveform, but this waveform is always static.
		std::string holdingCommands;
		// get the first input
		script >> holdingCommands;
		// handle trailing newline characters
		if ( holdingCommands.length( ) != 0 )
		{
			if ( holdingCommands[holdingCommands.length( ) - 1] == '\r' )
			{
				holdingCommands.erase( holdingCommands.length( ) - 1 );
			}
		}
		if ( !isStandardWaveform( holdingCommands ) )
		{
			thrower( "ERROR: detected command in flashing section that does not denote a standard waveform (e.g. a "
					 "logic command or something special). This is not allowed!" );
		}
		// don't want to add to the real output variable directly, this is a little hacky.
		NiawgOutput tempInfo = output;
		loadFullWave( tempInfo, holdingCommands, script, variables, rearrangeWave.rearrange.staticWave );
		/// get the target picture
		Matrix<bool> targetTemp = Matrix<bool>( rows, cols );
		// get the target picture. The picture must be replicated in each file.
		for ( auto rowInc : range( rows ) )
		{
			std::string line = script.getline( '\r' );
			ScriptStream lineScript( line );
			std::string singlePixelStatus;
			for ( auto colInc : range( cols ) )
			{
				lineScript >> singlePixelStatus;
				try
				{
					targetTemp( rowInc, colInc ) = bool( std::stoi( singlePixelStatus ) );
				}
				catch ( std::invalid_argument& )
				{
					thrower( "ERROR: Failed to load the user's input for a rearrangement target picture! Loading failed"
								" on this line: " + line + "\r\n" );
				}
			}
		}
		rearrangeWave.rearrange.target = targetTemp;
		ULONG finLocRow, finLocCol;
		std::string tempStrRow, tempStrCol;
		script >> tempStrRow;
		script >> tempStrCol;
		try
		{
			finLocRow = std::stoul( tempStrRow );
			finLocCol = std::stoul( tempStrCol );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: final rearranging location row or column failed to convert to unsigned long in niawg script!" );
		}
		rearrangeWave.rearrange.finalPosition = { finLocRow, finLocCol };
		script >> bracket;
		if ( bracket != "}" )
		{
			thrower( "ERROR: Expected \"}\" but found \"" + bracket + "\" in niawg File during flashing waveform read." );
		}
		// get the upper limit of the nuumber of moves that this could involve.
		rearrangeWave.rearrange.moveLimit = getMaxMoves( rearrangeWave.rearrange.target );
		rearrangeWave.rearrange.fillerWave = rearrangeWave.rearrange.staticWave;
		// filler move gets the full time of the move. Need to convert the time per move to ms instead of us.
		rearrangeWave.rearrange.fillerWave.time = str( (rearrangeWave.rearrange.moveLimit
														 * rearrangeWave.rearrange.timePerMove.evaluate( )
														 + 2 * rInfo.finalMoveTime) * 1e3 );
		output.waveFormInfo.push_back( rearrangeWave );
		long samples = long( (output.waveFormInfo.back( ).rearrange.moveLimit
							   * output.waveFormInfo.back( ).rearrange.timePerMove.evaluate( ) + 2 * rInfo.finalMoveTime)* NIAWG_SAMPLE_RATE );
		fgenConduit.allocateNamedWaveform( cstr( rerngWaveName ), samples );
		output.niawgLanguageScript += "generate " + rerngWaveName + "\n";
	}
	else
	{
		thrower( "ERROR: Bad waveform command!" );
	}
}


void NiawgController::handleSpecialWaveformForm( NiawgOutput& output, profileSettings profile, 
												 niawgPair<std::string> commands, niawgPair<ScriptStream>& scripts, 
												 debugInfo& options, rerngOptions rInfo, 
												 std::vector<variableType>& variables )
{
	if ( commands[Horizontal] != commands[Vertical] )
	{
		thrower( "ERROR: Special waveform commands must match!" );
	}
	std::string command = commands[Horizontal];
	if ( command == "flash" )
	{
		/*
			Format is:
			% Command
			flash 
			% bracket
			{
			% number of waveforms to flash (e.g. 3 means flashing between wvfm1, 2, 3, 1, 2, 3, ...
			2
			% flashing frequency (MHz)
			1
			% move time (ms)
			0.1
			% dead time (ns)
			0
			% all flashing waveforms, written with normal syntax. Watch out, the times must be chosen to match the overall
			% move time.
				gen6Const
				%%%%%	freq		amp		phase
				49		2.5	 	5.13919
				58		1	 	0.384594
				67		1.05	 	0.308571
				76		1.1	 	0.050804
				85		1.4	 	6.154510
				94		2.25	 	3.858584
				%%%%%	time	p.m.	delim
				0.05	0	#

			% second flash waveform
				gen2FreqRamp
				%%%%%	f.RTKW freq_i freq_f amp	phase
				nr   58	58	0.92		0
				lin  76 	67  	0.08		0
				%%%%%	time	p.m.	delim
				0.05	0	#
			% closing bracket
			}
		*/

		// bracket
		for ( auto axis : AXES )
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if ( bracket != "{" )
			{
				thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in " + AXES_NAMES[axis]
						 + " Niawg File during flashing waveform read" );
			}
		}

		waveInfoForm flashingWave;
		flashingWave.flash.isFlashing = true;
		/// Load general flashing info from file
		niawgPair<std::string> waveformsToFlashInput;
		try
		{
			niawgPair<int> flashNum;
			for ( auto axis : AXES )
			{
				scripts[axis] >> waveformsToFlashInput[axis];
				flashNum[axis] = std::stoi( waveformsToFlashInput[axis] );
				scripts[axis] >> flashingWave.flash.flashCycleFreqInput[axis];
				flashingWave.flash.flashCycleFreqInput[axis].assertValid( variables );
			}
			if ( flashingWave.flash.flashCycleFreqInput[Horizontal].expressionStr 
				 != flashingWave.flash.flashCycleFreqInput[Vertical].expressionStr )
			{
				thrower( "ERROR: flash cycle frequency must match between the two scripts! Expressions found were " +
						 flashingWave.flash.flashCycleFreqInput[Horizontal].expressionStr + " and "
						 + flashingWave.flash.flashCycleFreqInput[Vertical].expressionStr + "for horizontal and "
						 "vertical scripts respectively." );
			}
			flashingWave.flash.flashCycleFreq = flashingWave.flash.flashCycleFreqInput[Horizontal];
			if ( flashNum[Horizontal] != flashNum[Vertical] )
			{
				thrower( "ERROR: Flashing number didn't match between the horizontal and vertical files!" );
			}
			flashingWave.flash.flashNumber = flashNum[Horizontal];
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: flashing number failed to convert to an integer! This parameter cannot be varied." );
		}
		for ( auto axis : AXES )
		{
			scripts[axis] >> flashingWave.flash.totalTimeInput[axis];
		}

		if ( flashingWave.flash.totalTimeInput[Horizontal].expressionStr != 
				flashingWave.flash.totalTimeInput[Vertical].expressionStr )
		{
			thrower( "ERROR: Flashing cycle frequency didn't match between the horizontal and vertical files!" );
		}
		flashingWave.core.time = flashingWave.flash.totalTimeInput[Vertical];

		for ( auto axis : AXES )
		{
			scripts[axis] >> flashingWave.flash.deadTimeInput[axis];
		}

		if ( flashingWave.flash.deadTimeInput[Horizontal].expressionStr !=
			 flashingWave.flash.deadTimeInput[Vertical].expressionStr )
		{
			thrower( "ERROR: Flashing cycle frequency didn't match between the horizontal and vertical files!" );
		}
		flashingWave.flash.deadTime = flashingWave.flash.deadTimeInput[Horizontal];
		flashingWave.flash.deadTime.assertValid( variables );

		/// get waveforms to flash.
		NiawgOutput flashingOutputInfo = output;
		for ( auto waveCount : range( flashingWave.flash.flashNumber ) )
		{
			niawgPair<std::string> flashingWaveCommands;
			// get the first input
			for ( auto axis : AXES )
			{
				scripts[axis] >> flashingWaveCommands[axis];
				// handle trailing newline characters
				if ( flashingWaveCommands[axis].length( ) != 0 )
				{
					if ( flashingWaveCommands[axis][flashingWaveCommands[axis].length( ) - 1] == '\r' )
					{
						flashingWaveCommands[axis].erase( flashingWaveCommands[axis].length( ) - 1 );
					}
				}
			}

			if ( flashingWaveCommands[Horizontal] == "}" || flashingWaveCommands[Vertical] == "}" )
			{
				thrower( "ERROR: Expected " + str( flashingWave.flash.flashNumber ) + " waveforms for flashing but only found"
						 + str( waveCount ) );
			}
			if ( !isStandardWaveform( flashingWaveCommands[Horizontal] ) || !isStandardWaveform( flashingWaveCommands[Vertical] ) )
			{
				thrower( "ERROR: detected command in flashing section that does not denote a standard waveform (e.g. a"
						 " logic command). This is not allowed!" );
			}
			loadWaveformParametersForm( flashingOutputInfo, profile, flashingWaveCommands, options, scripts, variables );
			// add the new wave in flashOutInfo to flashingInfo structure
			flashingWave.flash.flashWaves.push_back( flashingOutputInfo.waveFormInfo.back( ).core );
		}

		// make sure ends with }
		for ( auto axis : AXES )
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if ( bracket != "}" )
			{
				thrower( "ERROR: Expected \"}\" but found " + bracket + " in" + AXES_NAMES[axis] + "File during flashing waveform read" );
			}
		}

		//createFlashingWave( flashingWave, options );
		flashingWave.core.name = "Waveform" + str( output.waveFormInfo.size( ) + 1 );
		output.waveFormInfo.push_back( flashingWave );
		// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
		// have to rewrite the script to insert the new waveform name into it.
		output.niawgLanguageScript += "generate " + output.waveFormInfo.back( ).core.name + "\n";

	}
	else if ( command == "stream" )
	{
		// TODO... maybe. don't think I really need to.
	}
	else if ( command == "rearrange" )
	{
		/*				Format:
		rearrange
		{
			rows in target
			cols in target
			lowest hor freq
			lowest vert freq
			freq spacing (usually 9MHz)
			hold waveform (e.g. gen 6 const)
			target pattern
			target coordinates
		}
		*/
		waveInfoForm rearrangeWave;
		rearrangeWave.rearrange.timePerMove = str(rInfo.moveSpeed);
		rearrangeWave.rearrange.isRearrangement = true;
		// the following two options are for simple flashing and simple streaming, not rearrangement, even though
		// rearrangment technically involves both
		rearrangeWave.flash.isFlashing = false;
		rearrangeWave.isStreamed = false;
		/// bracket
		for ( auto axis : AXES )
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if ( bracket != "{" )
			{
				thrower( "ERROR: Expected \"{\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] + " File during flashing waveform read" );
			}
		}
		/// get pic dims
		// get the dimensions of the target picture.
		niawgPair<UINT> rows, cols;
		for ( auto axis : AXES )
		{
			std::string temp;
			try
			{
				scripts[axis] >> temp;
				rows[axis] = std::stoi( temp );
				scripts[axis] >> temp;
				cols[axis] = std::stoi( temp );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: failed to convert target row and collumn numbers to integers during niawg script "
						 "analysis for rearrange command!" );
			}
		}
		// check consistency. Should probably verify that this is consistent with actual picture?
		if ( rows[Horizontal] != rows[Vertical] || cols[Horizontal] != cols[Vertical] )
		{
			thrower( "ERROR: Target Picture size doesn't match between horizontal and vertical scripts!" );
		}
		/// get calibration parameters.
		// these are the frequencies that the niawg would need to output to reach the lower left corner (I think?) of 
		// the picture.
		niawgPair<double> lowestHorFreq, lowestVertFreq, freqPerPixel;
		for ( auto axis : AXES )
		{
			std::string tempStr;
			scripts[axis] >> tempStr;
			lowestHorFreq[axis] = std::stod( tempStr );
			scripts[axis] >> tempStr;
			lowestVertFreq[axis] = std::stod( tempStr );
			scripts[axis] >> tempStr;
			freqPerPixel[axis] = std::stod( tempStr );
		}
		if ( lowestHorFreq[Horizontal] != lowestHorFreq[Vertical] ||
			 lowestVertFreq[Horizontal] != lowestVertFreq[Vertical] ||
			 freqPerPixel[Horizontal] != freqPerPixel[Vertical] )
		{
			thrower( "ERROR: Rearrangement parameters (lowest freqs or freq per pixel) don't match between "
					 "horizontal and vertical files!" );
		}
		rearrangeWave.rearrange.lowestFreqs[Horizontal] = lowestHorFreq[Horizontal];
		rearrangeWave.rearrange.lowestFreqs[Vertical] = lowestVertFreq[Horizontal];
		rearrangeWave.rearrange.freqPerPixel = freqPerPixel[Horizontal];
		/// get static pattern
		// this is the pattern that holds non-moving atoms in place. The algorithm calculates the moves, and then mixes
		// those moves with this waveform, but this waveform is always static.
		niawgPair<std::string> holdingCommands;
		// get the first input
		for ( auto axis : AXES )
		{
			scripts[axis] >> holdingCommands[axis];
			// handle trailing newline characters
			if ( holdingCommands[axis].length( ) != 0 )
			{
				if ( holdingCommands[axis][holdingCommands[axis].length( ) - 1] == '\r' )
				{
					holdingCommands[axis].erase( holdingCommands[axis].length( ) - 1 );
				}
			}
		}
		if ( !isStandardWaveform( holdingCommands[Horizontal] ) || !isStandardWaveform( holdingCommands[Vertical] ) )
		{
			thrower( "ERROR: detected command in flashing section that does not denote a standard waveform (e.g. a "
					 "logic command or something special). This is not allowed!" );
		}

		// don't want to add directly 
		NiawgOutput tempInfo = output;
		loadWaveformParametersForm( tempInfo, profile, holdingCommands, options, scripts, variables );
		// add the new wave in flashOutInfo to flashingInfo structure
		rearrangeWave.rearrange.staticWave = tempInfo.waveFormInfo.back( ).core;
		/// get the target picture
		niawgPair<Matrix<bool>> targetTemp = { Matrix<bool>( rows[Vertical], cols[Vertical] ), 
											   Matrix<bool>( rows[Horizontal], cols[Horizontal] ) };
		// get the target picture. The picture must be replicated in each file.
		for ( auto axis : AXES )
		{
			for ( auto rowInc : range( rows[axis] ) )
			{
				std::string line = scripts[axis].getline( '\r' );
				ScriptStream lineScript( line );
				std::string singlePixelStatus;
				for ( auto colInc : range( cols[axis] ) )
				{
					lineScript >> singlePixelStatus;
					try
					{
						targetTemp[axis](rowInc, colInc) = bool( std::stoi( singlePixelStatus ) );
					}
					catch ( std::invalid_argument& )
					{
						thrower( "ERROR: Failed to load the user's input for a rearrangement target picture! Loading failed"
								 " on this line: " + line + "\r\n" );
					}
				}
			}
		}

		for ( auto rowInc : range( rearrangeWave.rearrange.target.getRows()) )
		{
			for ( auto colInc : range( rearrangeWave.rearrange.target.getCols() ) )
			{
				if ( targetTemp[Horizontal](rowInc,colInc) != targetTemp[Vertical](rowInc, colInc) )
				{
					thrower( "ERROR: Target pictures didn't match between the Horizontal and Vertical Files!" );
				}
			}
		}
		rearrangeWave.rearrange.target = targetTemp[Horizontal];

		niawgPair<ULONG> finLocRow, finLocCol;
		for ( auto axis : AXES )
		{
			std::string tempStrRow, tempStrCol;
			scripts[axis] >> tempStrRow;
			scripts[axis] >> tempStrCol;
			try
			{
				finLocRow[axis] = std::stoul( tempStrRow );
				finLocCol[axis] = std::stoul( tempStrCol );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: final rearranging location row or column failed to convert to unsigned long in "
						 + AXES_NAMES[axis] + " script!" );
			}
		}

		if ( finLocRow[Horizontal] != finLocRow[Vertical] || finLocCol[Horizontal] != finLocCol[Vertical] )
		{
			thrower( "ERROR: final rearranging location listed in horizontal script didn't match vertical script!" );
		}
		
		rearrangeWave.rearrange.finalPosition = { finLocRow[Horizontal], finLocCol[Vertical] };
		for ( auto axis : AXES )
		{
			std::string bracket;
			scripts[axis] >> bracket;
			if ( bracket != "}" )
			{
				thrower( "ERROR: Expected \"}\" but found \"" + bracket + "\" in " + AXES_NAMES[axis] + " File during flashing waveform read." );
			}
		}
		// get the upper limit of the nuumber of moves that this could involve.
		rearrangeWave.rearrange.moveLimit = getMaxMoves( rearrangeWave.rearrange.target );
		rearrangeWave.rearrange.fillerWave = rearrangeWave.rearrange.staticWave;
		// filler move gets the full time of the move. Need to convert the time per move to ms instead of us.
		rearrangeWave.rearrange.fillerWave.time = str( (rearrangeWave.rearrange.moveLimit
													   * rearrangeWave.rearrange.timePerMove.evaluate( ) 
													   + 2 * rInfo.finalMoveTime) * 1e3 );
		output.waveFormInfo.push_back( rearrangeWave );
		long samples = long( ( output.waveFormInfo.back( ).rearrange.moveLimit
							   * output.waveFormInfo.back( ).rearrange.timePerMove.evaluate( ) + 2 * rInfo.finalMoveTime )* NIAWG_SAMPLE_RATE );
		fgenConduit.allocateNamedWaveform( cstr( rerngWaveName ), samples );
		output.niawgLanguageScript += "generate " + rerngWaveName + "\n";
	}
	else
	{
		thrower( "ERROR: Bad waveform command!" );
	}
}


void NiawgController::handleVariations( NiawgOutput& output, std::vector<std::vector<variableType>>& variables, 
										UINT variation, std::vector<long>& mixedWaveSizes, std::string& warnings, 
										debugInfo& debugOptions, UINT totalVariations)
{
	int mixedCount = 0;
	// I think waveInc = 0 & 1 are always the default.. should I be handling that at all? shouldn't make a difference 
	// I don't think.
	for ( auto seqInc : range( variables.size( ) ) )
	{
		for ( auto waveInc : range( output.waveFormInfo.size( ) ) )
		{
			waveInfo& wave = output.waves[waveInc];
			waveInfoForm& waveForm = output.waveFormInfo[waveInc];
			if ( waveForm.core.varies )
			{
				if ( waveForm.flash.isFlashing )
				{
					flashFormToOutput( waveForm, wave, variables[seqInc], variation );
					writeFlashing( wave, debugOptions, variation );
				}
				else
				{
					simpleFormToOutput( waveForm.core, wave.core, variables[seqInc], variation );
					if ( variation != 0 )
					{
						fgenConduit.deleteWaveform( cstr( wave.core.name ) );
					}
					writeStandardWave( wave.core, debugOptions, output.isDefault );
				}
				mixedWaveSizes.push_back( 2 * wave.core.sampleNum );
				mixedCount++;
			}
			waveInc++;
		}
	}
	checkThatWaveformsAreSensible( warnings, output );
}


void NiawgController::loadStandardInputFormType( std::string inputType, channelWaveForm &wvInfo )
{
	// Check against every possible generate input type. 
	wvInfo.initType = -1;
	for ( auto number : range( MAX_NIAWG_SIGNALS ) )
	{
		number += 1;
		if ( inputType == "gen" + str( number ) + "const" )
		{
			wvInfo.initType = number;
		}
		else if ( inputType == "gen" + str( number ) + "ampramp" )
		{
			wvInfo.initType = number + MAX_NIAWG_SIGNALS;
		}
		else if ( inputType == "gen" + str( number ) + "freqramp" )
		{
			wvInfo.initType = number + 2 * MAX_NIAWG_SIGNALS;
		}
		else if ( inputType == "gen" + str( number ) + "freq&ampramp" )
		{
			wvInfo.initType = number + 3 * MAX_NIAWG_SIGNALS;
		}
	}
	if ( wvInfo.initType == -1 )
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
void NiawgController::generateWaveform( channelWave & chanWave, debugInfo& options, long int sampleNum, double time )
{
	chanWave.wave.resize( sampleNum );
	// the number of seconds
	std::string waveformFileSpecs, waveformFileName;
	std::ifstream waveformFileRead;
	std::ofstream waveformFileWrite;
	// Construct the name of the raw data file from the parameters for the waveform. This can be a pretty long name, but that's okay 
	// because it's just text in a file at the end. This might become a problem if the name gets toooo long...
	for ( auto signal : range( chanWave.signals.size( ) ) )
	{
		waveformFileSpecs += (str( chanWave.signals[signal].freqInit ) + " " + str( chanWave.signals[signal].freqFin ) + " "
							   + chanWave.signals[signal].freqRampType + " " + str( chanWave.signals[signal].initPower ) + " "
							   + str( chanWave.signals[signal].finPower ) + " " + chanWave.signals[signal].powerRampType + " "
							   + str( chanWave.signals[signal].initPhase ) + ", ");
	}
	waveformFileSpecs += str( time * 1000.0 ) + "; ";
	// Start timer
	std::chrono::time_point<chronoClock> time1( chronoClock::now( ) );
	/// Loop over all previously recorded files (these should have been filled by a previous call to openWaveformFiles()).
	for ( UINT fileInc = 0; fileInc < waveLibrary[chanWave.initType].size( ); fileInc++ )
	{
		// if you find this waveform to have already been written...
		if ( waveLibrary[chanWave.initType][fileInc] == waveformFileSpecs )
		{
			// Construct the file address
			std::string waveFileReadName = LIB_PATH + WAVEFORM_TYPE_FOLDERS[chanWave.initType]
										   + str( chanWave.initType ) + "_" + str( fileInc ) + ".txt";
			waveformFileRead.open( waveFileReadName, std::ios::binary | std::ios::in );
			std::vector<ViReal64> readData( sampleNum + chanWave.signals.size( ) );
			waveformFileRead.read( (char *)readData.data( ),
								   (sampleNum + chanWave.signals.size( )) * sizeof( ViReal64 ) );
			// grab the phase data off of the end.
			for ( auto signal : range( chanWave.signals.size( ) ) )
			{
				chanWave.signals[signal].finPhase = readData[sampleNum + signal];
			}
			// put the relevant voltage data into a the new array.
			chanWave.wave = std::vector<ViReal64>( readData.begin( ), readData.begin( ) + sampleNum );
			readData.clear( );
			// make sure the large amount of memory is deallocated.
			readData.shrink_to_fit( );
			waveformFileRead.close( );
			if ( options.showReadProgress )
			{
				std::chrono::time_point<chronoClock> time2( chronoClock::now( ) );
				double ellapsedTime( std::chrono::duration<double>( (time2 - time1) ).count( ) );
				options.message += "Finished Reading Waveform. Ellapsed Time: " + str( ellapsedTime ) + " seconds.\r\n";
			}
			// if the file got read, I don't need to do any writing, so go ahead and return.
			return;
		}
	}

	// if the code reaches this point, it could not find a file to read, and so will now create the data from scratch 
	// and write it. 
	waveformFileName = (LIB_PATH + WAVEFORM_TYPE_FOLDERS[chanWave.initType] + str( chanWave.initType ) + "_"
						 + str( waveLibrary[chanWave.initType].size( ) ) + ".txt");
	// open file for writing.
	waveformFileWrite.open( waveformFileName, std::ios::binary | std::ios::out );
	// make sure it opened.
	if ( !waveformFileWrite.is_open( ) )
	{
		// shouldn't ever happen.
		thrower( "ERROR: Data Storage File could not open. Shouldn't happen. File name is probably too long? File name is "
				 + waveformFileName + ", which is " + str( waveformFileName.size( ) ) + " characters long." );
	}
	else
	{
		// start timer.
		std::chrono::time_point<chronoClock> time1( chronoClock::now( ) );
		// calculate all voltage values and final phases and store them in the readData variable.
		std::vector<ViReal64> readData( sampleNum + chanWave.signals.size( ) );
		calcWaveData( chanWave, readData, sampleNum, time );
		// Write the data, with phases, to the write file.
		waveformFileWrite.write( (const char *)readData.data( ), (sampleNum + chanWave.signals.size( )) * sizeof( ViReal64 ) );
		waveformFileWrite.close( );
		// put the relevant data into another string.
		chanWave.wave = std::vector<ViReal64>( readData.begin( ), readData.begin( ) + sampleNum );
		readData.clear( );
		// make sure the large amount of memory is deallocated.
		readData.shrink_to_fit( );
		// write the newly written waveform's name to the library file.
		std::fstream libNameFile;
		libNameFile.open( LIB_PATH + WAVEFORM_TYPE_FOLDERS[chanWave.initType] + WAVEFORM_NAME_FILES[chanWave.initType],
						  std::ios::binary | std::ios::out | std::ios::app );
		if ( !libNameFile.is_open( ) )
		{
			thrower( "ERROR! waveform name file not opening correctly.\n" );
		}
		// add the waveform name to the current list of strings. do it BEFORE adding the newline T.T
		waveLibrary[chanWave.initType].push_back( cstr( waveformFileSpecs ) );
		// put a newline in front of the name so that all of the names don't get put on the same line.
		waveformFileSpecs = "\n" + waveformFileSpecs;
		libNameFile.write( cstr( waveformFileSpecs ), waveformFileSpecs.size( ) );
		libNameFile.close( );
		if ( options.showWriteProgress )
		{
			std::chrono::time_point<chronoClock> time2( chronoClock::now( ) );
			double ellapsedTime = std::chrono::duration<double>( time2 - time1 ).count( );
			options.message += "Finished writing waveform. Ellapsed Time: " + str( ellapsedTime ) + " seconds.\r\n";
		}
	}
}

void NiawgController::handleLogicSingle( ScriptStream& script, std::string cmd, std::string &scriptString )
{
	if ( cmd == "waittiltrig" )
	{
		triggersInScript++;
		scriptString += "wait until " + fgenConduit.getExternalTriggerName( ) + "\n";
	}
	else if ( cmd == "waitTilsoftwaretrig" )
	{
		// trigger count only counts hardware triggers.
		scriptString += "wait until " + fgenConduit.getSoftwareTriggerName( ) + "\n";
	}
	else if ( cmd == "waitset#" )
	{
		// grab the # of samples the user wants to wait.
		std::string temp;
		int sampleNum;
		try
		{
			script >> temp;
			sampleNum = std::stoi( temp );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: Sample number inside wait command wasn't an integer! Value was " + temp );
		}
		scriptString += "wait " + str( (long long)sampleNum ) + "\n";
	}
	// Repeat commands // 
	else if ( cmd == "repeatset#" )
	{
		// grab the number of times to repeat the user is going for.
		std::string temp;
		int repeatNum;
		try
		{
			script >> temp;
			repeatNum = std::stoi( temp );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: repeat number was not an integer! value was " + temp );
		}
		scriptString += "repeat " + str( (long long)repeatNum ) + "\n";
	}
	else if ( cmd == "repeattiltrig" )
	{
		triggersInScript++;
		scriptString += "repeat until " + fgenConduit.getExternalTriggerName( ) + "\n";
	}
	else if ( cmd == "repeattilsoftwaretrig" )
	{
		scriptString += "repeat until " + fgenConduit.getSoftwareTriggerName( ) + "\n";
	}
	else if ( cmd == "repeatforever" )
	{
		scriptString += "repeat forever\n";
	}
	else if ( cmd == "endrepeat" )
	{
		scriptString += "end repeat\n";
	}
	// if-else Commands //
	else if ( cmd == "iftrig" )
	{
		// trigger can happen or not happen here so don't count it.
		scriptString += "if " + fgenConduit.getExternalTriggerName( ) + "\n";
	}
	else if ( cmd == "ifsoftwaretrig" )
	{
		scriptString += "if " + fgenConduit.getSoftwareTriggerName( ) + "\n";
	}
	else if ( cmd == "else" )
	{
		scriptString += "else\n";
	}
	else if ( cmd == "end if" )
	{
		scriptString += "end if\n";
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
		triggersInScript++;
		// Append command to script holder
		rawNiawgScriptString += "wait until " + fgenConduit.getExternalTriggerName() + "\n";
	}
	else if (input == "waitTilsoftwaretrig")
	{
		// trigger count only counts hardware triggers.
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
		triggersInScript++;
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
		// trigger can happen or not happen here so don't count it.
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


void NiawgController::handleSpecialFormSingle( ScriptStream& script, NiawgOutput& output, std::string cmd, 
											   profileSettings profile, debugInfo& options, std::string& warnings )
{
	// work with marker events
	if ( cmd == "create marker event" )
	{
		// get the timing information from the file.
		std::string waitSamples;
		script >> waitSamples;
		// ! remove previous newline to put this command on the same line as a generate command, as it needs to be for 
		// the final script. !
		output.niawgLanguageScript.pop_back( );
		output.niawgLanguageScript += " marker0 (" + waitSamples + ")\n";
	}
	else
	{
		thrower( "ERROR: special command not found!" );
	}
}


void NiawgController::handleSpecialForm( niawgPair<ScriptStream>& scripts, NiawgOutput& output, 
										 niawgPair<std::string> inputTypes, profileSettings profile, debugInfo& options, 
										 std::string& warnings )
{
	// declare some variables
	// first level is file the name was found in, second level is the name inside that file.
	niawgPair<niawgPair<std::string>> externalScriptNames, externalWaveformNames;
	niawgPair<std::string> waitSamples, importTypes;
	niawgPair<std::fstream> externalWaveformFiles;

	// work with marker events
	if ( inputTypes[Vertical] == "create marker event" )
	{
		// Make sure the commands match
		if ( inputTypes[Horizontal] != "create marker event" )
		{
			thrower( "ERROR: logical input commands must match, and they don't!\n\n" );
		}
		// get the timing information from the file.
		for ( auto axis : AXES )
		{
			scripts[axis] >> waitSamples[axis];
		}
		if ( waitSamples[Vertical] != waitSamples[Horizontal] )
		{
			thrower( "ERROR: Numer of samples to wait must be the same in the vertical and horizontal files, and they aren't!" );
		}
		// ! remove previous newline to put this command on the same line as a generate command, as it needs to be for the final script. !
		output.niawgLanguageScript.pop_back( );
		// append script
		output.niawgLanguageScript += " marker0 (" + waitSamples[Vertical] + ")\n";
	}
	else
	{
		thrower( "ERROR: special command not found!" );
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
void NiawgController::calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, double time )
{
	// Declarations
	std::vector<double> powerPos, freqRampPos, phasePos( inputData.signals.size( ) );
	std::vector<double*> powerRampFileData, freqRampFileData;
	std::fstream powerRampFile, freqRampFile;
	std::string tempStr;

	/// deal with ramp calibration files. check all signals for files and read if yes.
	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		// create spots for the ramp positions.
		powerPos.push_back( 0 );
		freqRampPos.push_back( 0 );
		// If the ramp type isn't a standard command...
		if ( inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr"
			 && inputData.signals[signal].powerRampType != "tanh" )
		{
			powerRampFileData.push_back( new double[sampleNum] );
			// try to open it as a file
			powerRampFile.open( inputData.signals[signal].powerRampType );
			// if successful....
			if ( powerRampFile.is_open( ) )
			{
				int powerValNumber = 0;
				// load the data in
				while ( !powerRampFile.eof( ) )
				{
					powerRampFile >> powerRampFileData[signal][powerValNumber];
					powerValNumber++;
				}
				// error message for bad size (powerRampFile.eof() reached too early or too late).
				if ( powerValNumber != sampleNum + 1 )
				{
					thrower( "ERROR: file not the correct size?\nSize of upload is " + str( powerValNumber )
							 + "; size of file is " + str( sampleNum ) );
				}
				// close the file.
				powerRampFile.close( );
			}
			else
			{
				thrower( "ERROR: ramp type " + str( inputData.signals[signal].powerRampType ) + " is unrecognized. If "
						 "this is a file name, make sure the file exists and is in the project folder. " );
			}
		}
		// If the ramp type isn't a standard command...
		if ( inputData.signals[signal].freqRampType != "lin" && inputData.signals[signal].freqRampType != "nr"
			 && inputData.signals[signal].freqRampType != "tanh" )
		{
			// try to open it
			freqRampFileData.push_back( new double[sampleNum] );
			freqRampFile.open( inputData.signals[signal].freqRampType, std::ios::in );
			// if open
			if ( freqRampFile.is_open( ) )
			{
				int freqRampValNum = 0;
				while ( !freqRampFile.eof( ) )
				{
					freqRampFile >> freqRampFileData[signal][freqRampValNum];
					freqRampValNum++;
				}
				// error message for bad size (powerRampFile.eof() reached too early or too late).
				if ( freqRampValNum != sampleNum + 1 )
				{
					thrower( "ERROR: file not the correct size?\nSize of upload is " + str( freqRampValNum )
							 + "; size of file is " + str( sampleNum ) );
				}
				// close file
				freqRampFile.close( );
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
	double deltaTanh = std::tanh( 4 ) - std::tanh( -4 );
	for ( UINT signal = 0; signal < inputData.signals.size( ); signal++ )
	{
		deltaOmega.push_back( 2 * PI * (inputData.signals[signal].freqFin - inputData.signals[signal].freqInit) );
	}
	///		Get Data Points.		///
	// initialize signalInc before the loop so that I have access to it afterwards.
	int sample = 0;
	/// increment through all samples
	for ( ; sample < sampleNum; sample++ )
	{
		// calculate the time that this sample number refers to
		double curTime = (double)sample / NIAWG_SAMPLE_RATE;
		/// Calculate Phase and Power Positions. For Every signal...
		for ( auto signal : range( inputData.signals.size( ) ) )
		{
			/// Handle Frequency Ramps
			// Frequency ramps are actually a little complex. we have dPhi/dt = omega(t) and we need phi to calculate data points. So in order to get 
			// the phase you need to integrate the omega(t) you want and modify the integration constant to get your initial phase.
			if ( inputData.signals[signal].freqRampType == "lin" )
			{
				// W{t} = Wi + (DeltaW * t) / (Tfin)
				// Phi{t}   = Wi * t + (DeltaW * t ^ 2) / 2 + phi_i
				phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit * curTime + deltaOmega[signal] * pow( curTime, 2 ) / (2 * time)
									 + inputData.signals[signal].initPhase);
			}
			else if ( inputData.signals[signal].freqRampType == "tanh" )
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
					+ (deltaOmega[signal] / deltaTanh) * (time / 8.0) * (std::log( std::cosh( 4 - (8 / time) * curTime ) )
																		  - std::log( std::cosh( 4 ) )) + inputData.signals[signal].initPhase;
			}
			else if ( inputData.signals[signal].freqRampType == "nr" )
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
			if ( inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr"
				 && inputData.signals[signal].powerRampType
				 != "tanh" )
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
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				currentPower += fabs( inputData.signals[signal].initPower + powerPos[signal] );
				/// modify here for frequency calibrations!
				/// need current frequency and calibration file.
			}

			// normalize each signal.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal])
					* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
			}
		}
		///  Calculate data point.
		readData[sample] = 0;
		for ( auto signal : range( inputData.signals.size( ) ) )
		{
			// get data point. V = Sqrt(Power) * Sin(Phase)
			readData[sample] += sqrt( inputData.signals[signal].initPower + powerPos[signal] ) * sin( phasePos[signal] );
		}
	}

	/// Calculate one last time for the final phases. I want the final phase to be the phase of the NEXT data point. 
	/// Then, following waveforms can START at this phase.
	double curTime = (double)sample / NIAWG_SAMPLE_RATE;
	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		// Calculate Phase Position. See above for description.
		if ( inputData.signals[signal].freqRampType == "lin" )
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + deltaOmega[signal] * pow( curTime, 2 ) * 1 / (2 * time)
				+ inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "tanh" )
		{
			phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
				+ (deltaOmega[signal] / deltaTanh) * (time / 8.0) * std::log( std::cosh( 4 - (8 / time) * curTime ) )
				- (deltaOmega[signal] / deltaTanh) * (time / 8.0) * std::log( std::cosh( 4 ) )
				+ inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "nr" )
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

	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		// get the final phase of this waveform. Note that this is the phase of the /next/ data point (the last time signalInc gets incremented, the for loop 
		// doesn't run) so that if the next waveform starts at this data point, it will avoid repeating the same data point. This is used for the 
		// option where the user uses this phase as the starting phase of the next waveform.
		inputData.signals[signal].finPhase = fmod( phasePos[signal], 2 * PI );
		// catch the case in which the final phase is virtually identical to 2*PI, which isn't caught in the above line because of bad floating point 
		// arithmetic.
		if ( fabs( inputData.signals[signal].finPhase - 2 * PI ) < 0.00000005 )
		{
			inputData.signals[signal].finPhase = 0;
		}
		// put the final phase in the last data point.
		readData[sample + signal] = inputData.signals[signal].finPhase;
	}
}

/**
  * This function takes two filled waveform arrays, and interweaves them into a new data array. this is required 
  * by the NI card for outputting to both outputs separately.
  */
void NiawgController::mixWaveforms( simpleWave& waveCore, bool writeThisToFile )
{
	waveCore.waveVals.resize( 2 * waveCore.sampleNum );
	for ( auto sample : range( waveCore.sampleNum ) )
	{
		// the order (Vertical -> Horizontal) here is important. Vertical is first because it's port zero on the Niawg. I believe that
		// switching the order here and changing nothing else would flip the output of the niawg..			
		waveCore.waveVals[2 * sample] = waveCore.chan[Vertical].wave[sample];
		waveCore.waveVals[2 * sample + 1] = waveCore.chan[Horizontal].wave[sample];
	}
	if ( writeThisToFile )
	{
		writeToFile( waveCore.waveVals );
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


void NiawgController::loadCommonWaveParams( ScriptStream& script, simpleWaveForm& wave )
{
	Expression time;
	script >> time;
	try
	{
		time.evaluate( );
	}
	catch ( Error& err )
	{
		thrower( "ERROR: waveform time cannot be varied! Evaluation of time expression failed with error:\n"
				 + err.whatStr( ) );
	}
	std::string option;
	script >> option;
	int phaseOption;
	try
	{
		phaseOption = std::stoi( option );
		wave.chan[Horizontal].phaseOption = wave.chan[Vertical].phaseOption = phaseOption;
	}
	catch ( std::invalid_argument& )
	{
		thrower( "Error: phase option failed to convert to an integer." );
	}
}


void NiawgController::loadWaveformParametersFormSingle( NiawgOutput& output, std::string cmd, ScriptStream& script,
														std::vector<variableType> variables, int axis, 
														simpleWaveForm& wave )
{
	// Don't remember why I have this limitation built in.
	if ( output.isDefault && output.waveFormInfo.size( ) == 1 )
	{
		thrower( "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must "
				 "be a single waveform, not a sequence.\r\n" );
	}
	// Get a number corresponding directly to the given input type.
	loadStandardInputFormType( cmd, wave.chan[axis] ); 
	// infer the number of signals from the type assigned.
	if ( wave.chan[axis].initType % MAX_NIAWG_SIGNALS == 0 )
	{
		wave.chan[axis].signals.resize( MAX_NIAWG_SIGNALS );
	}
	else
	{
		wave.chan[axis].signals.resize( wave.chan[axis].initType % MAX_NIAWG_SIGNALS );
	}

	for ( int signal = 0; signal < int( wave.chan[axis].signals.size( ) ); signal++ )
	{
		auto& sig = wave.chan[axis].signals[signal];
		switch ( (wave.chan[axis].initType - 1) / MAX_NIAWG_SIGNALS )
		{
			/// the case for "gen ?, const"
			case 0:
			{
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				script >> sig.freqInit;
				sig.freqInit.assertValid( variables );
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				script >> sig.initPower;
				sig.initPower.assertValid( variables );
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables );
				sig.freqFin = sig.freqInit;
				sig.finPower = sig.initPower;
				sig.powerRampType = "nr";
				sig.freqRampType = "nr";
				break;
			}
			/// The case for "gen ?, amp ramp"
			case 1:
			{
				script >> sig.freqInit;
				sig.freqInit.assertValid( variables );
				script >> sig.powerRampType;
				script >> sig.initPower;
				sig.initPower.assertValid( variables );
				script >> sig.finPower;
				sig.finPower.assertValid( variables );
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables );
				sig.freqFin = sig.freqInit;
				sig.freqRampType = "nr";
				break;
			}
			/// The case for "gen ?, freq ramp"
			case 2:
			{
				script >> sig.freqRampType;
				script >> sig.freqInit;
				sig.freqInit.assertValid( variables );
				script >> sig.freqFin;
				sig.freqFin.assertValid( variables );
				script >> sig.initPower;
				sig.initPower.assertValid( variables );
				sig.finPower = sig.initPower;
				sig.powerRampType = "nr";
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables );
				break;
			}
			/// The case for "gen ?, freq & amp ramp"
			case 3:
			{
				script >> sig.freqRampType;
				script >> sig.freqInit;
				sig.freqInit.assertValid( variables );
				script >> sig.freqFin;
				sig.freqFin.assertValid( variables );
				script >> sig.powerRampType;
				script >> sig.initPower;
				sig.initPower.assertValid( variables );
				script >> sig.finPower;
				sig.finPower.assertValid( variables );
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables );
				break;
			}
		}
	}

	script >> wave.chan[axis].delim;
	// check delimiter
	if ( wave.chan[axis].delim != "#" )
	{
		thrower( "ERROR: The delimeter is missing in the " + AXES_NAMES[axis] + " script file for waveform #"
					+ str( output.waveFormInfo.size( ) - 1 ) + "The value placed in the delimeter location was " + wave.chan[axis].delim
					+ " while it should have been '#'. This indicates that either the code is not interpreting the user input "
					"correctly or that the user has inputted too many parameters for this type of waveform." );
	}
	//	Handle -1 Phase (start with the phase that the previous waveform ended with)
	UINT count = 0;
	// loop through all signals in a the current waveform for a given axis.
	for ( auto signal : wave.chan[axis].signals )
	{
		// If the user used a '-1' for the initial phase, this means the user wants to copy the ending phase of the 
		// previous waveform.
		if ( signal.initPhase.evaluate( ) == -1 )
		{
			UINT prevNum = output.waveFormInfo.size( ) - 1;
			UINT signalNum = output.waveFormInfo[prevNum].core.chan[axis].signals.size( );
			if ( count + 1 > signalNum )
			{
				thrower( "ERROR: You are trying to copy the phase of signal " + str( count + 1 ) + "  of " 
						 + AXES_NAMES[axis] + " waveform #" + str( prevNum ) + ", but the previous waveform only had " 
						 + str( signalNum ) + " signals!\n" );
			}
		}
		count++;
	}
}


void NiawgController::loadWaveformParametersForm( NiawgOutput& output, profileSettings profile,
												  niawgPair<std::string> command, debugInfo& debug,
												  niawgPair<ScriptStream>& scripts, std::vector<variableType> variables )
{
	simpleWaveForm wave;
	// not sure why I have this limitation built in.
	if ( output.isDefault && output.waveFormInfo.size( ) == 1 )
	{
		thrower( "ERROR: The default waveform files contain sequences of waveforms. Right now, the default waveforms must "
				 "be a single waveform, not a sequence.\r\n" );
	}
	niawgPair<Expression> time;
	for ( auto axis : AXES )
	{
		// Get a number corresponding directly to the given input type.
		loadStandardInputFormType( command[axis], wave.chan[axis] );
		// infer the number of signals from the type assigned.
		if ( wave.chan[axis].initType % MAX_NIAWG_SIGNALS == 0 )
		{
			wave.chan[axis].signals.resize( MAX_NIAWG_SIGNALS );
		}
		else
		{
			wave.chan[axis].signals.resize( wave.chan[axis].initType % MAX_NIAWG_SIGNALS );
		}
		for ( int signal = 0; signal < int( wave.chan[axis].signals.size( ) ); signal++ )
		{
			switch ( (wave.chan[axis].initType - 1) / MAX_NIAWG_SIGNALS )
			{
				/// the case for "gen ?, const"
				case 0:
				{
					// set the initial and final values to be equal, and to not use a ramp, unless variable present.
					scripts[axis] >> wave.chan[axis].signals[signal].freqInit;
					wave.chan[axis].signals[signal].freqInit.assertValid( variables );
					// set the initial and final values to be equal, and to not use a ramp, unless variable present.
					scripts[axis] >> wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].initPower.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].initPhase;
					wave.chan[axis].signals[signal].initPhase.assertValid( variables );
					//
					wave.chan[axis].signals[signal].freqFin = wave.chan[axis].signals[signal].freqInit;
					wave.chan[axis].signals[signal].finPower = wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].powerRampType = "nr";
					wave.chan[axis].signals[signal].freqRampType = "nr";
					break;
				}
				/// The case for "gen ?, amp ramp"
				case 1:
				{
					scripts[axis] >> wave.chan[axis].signals[signal].freqInit;
					wave.chan[axis].signals[signal].freqInit.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].powerRampType;
					scripts[axis] >> wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].initPower.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].finPower;
					wave.chan[axis].signals[signal].finPower.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].initPhase;
					wave.chan[axis].signals[signal].initPhase.assertValid( variables );

					wave.chan[axis].signals[signal].freqFin = wave.chan[axis].signals[signal].freqInit;
					wave.chan[axis].signals[signal].freqRampType = "nr";
					break;
				}
				/// The case for "gen ?, freq ramp"
				case 2:
				{
					scripts[axis] >> wave.chan[axis].signals[signal].freqRampType;
					scripts[axis] >> wave.chan[axis].signals[signal].freqInit;
					wave.chan[axis].signals[signal].freqInit.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].freqFin;
					wave.chan[axis].signals[signal].freqFin.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].initPower.assertValid( variables );
					wave.chan[axis].signals[signal].finPower = wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].powerRampType = "nr";
					scripts[axis] >> wave.chan[axis].signals[signal].initPhase;
					wave.chan[axis].signals[signal].initPhase.assertValid( variables );
					break;
				}
				/// The case for "gen ?, freq & amp ramp"
				case 3:
				{
					scripts[axis] >> wave.chan[axis].signals[signal].freqRampType;
					scripts[axis] >> wave.chan[axis].signals[signal].freqInit;
					wave.chan[axis].signals[signal].freqInit.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].freqFin;
					wave.chan[axis].signals[signal].freqFin.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].powerRampType;
					scripts[axis] >> wave.chan[axis].signals[signal].initPower;
					wave.chan[axis].signals[signal].initPower.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].finPower;
					wave.chan[axis].signals[signal].finPower.assertValid( variables );
					scripts[axis] >> wave.chan[axis].signals[signal].initPhase;
					wave.chan[axis].signals[signal].initPhase.assertValid( variables );
					break;
				}			
			}
		}
		// get the common things.
		scripts[axis] >> time[axis];
		try
		{
			time[axis].evaluate( );
		}
		catch ( Error& err )
		{
			thrower( "ERROR: waveform time cannot be varied! Evaluation of time expression failed with error:\n" 
					 + err.whatStr() );
		}
		std::string option;
		scripts[axis] >> option;
		try
		{
			wave.chan[axis].phaseOption = std::stoi( option );
		}
		catch ( std::invalid_argument&)
		{
			thrower( "Error: phase option failed to convert to an integer." );
		}
		scripts[axis] >> wave.chan[axis].delim;
		// check delimiter
		if ( wave.chan[axis].delim != "#" )
		{
			thrower( "ERROR: The delimeter is missing in the " + AXES_NAMES[axis] + " script file for waveform #"
					 + str( output.waveFormInfo.size( ) - 1 ) + "The value placed in the delimeter location was " + wave.chan[axis].delim
					 + " while it should have been '#'. This indicates that either the code is not interpreting the user input "
					 "correctly or that the user has inputted too many parameters for this type of waveform." );
		}
	}
	// make sure the times match to nanosecond precision.
	if ( !(fabs( time[Horizontal].evaluate() - time[Vertical].evaluate( ) ) < 1e-6) )
	{
		thrower( "ERROR: the horizontal and vertical waveforms must have the same time value. They appear to be mismatched for waveform #"
				 + str( output.waveFormInfo.size( ) - 1 ) + "!" );
	}
	wave.time = time[Horizontal];
	//	Handle -1 Phase (start with the phase that the previous waveform ended with)
	for ( auto axis : AXES )
	{
		UINT count = 0;
		// loop through all signals in a the current waveform for a given axis.
		for ( auto signal : wave.chan[axis].signals )
		{
			// If the user used a '-1' for the initial phase, this means the user wants to copy the ending phase of the previous waveform.
			if ( signal.initPhase.evaluate() == -1 )
			{
				if ( count + 1 > output.waveFormInfo[output.waveFormInfo.size( ) - 1].core.chan[axis].signals.size( ) )
				{
					thrower( "ERROR: You are trying to copy the phase of signal " + str( count + 1 ) + "  of "
							 + AXES_NAMES[axis] + " waveform #" + str( output.waveFormInfo.size( ) - 1 )
							 + ", but the previous waveform only had "
							 + str( output.waveFormInfo[output.waveFormInfo.size( ) - 1].core.chan[axis].signals.size( ) ) + " signals!\n" );
				}
			}
			count++;
		}
	}
	output.waveFormInfo.push_back( toWaveInfoForm( wave ) );
}


void NiawgController::loadFullWave( NiawgOutput& output, std::string cmd, ScriptStream& script,
									std::vector<variableType>& variables, simpleWaveForm& wave )
{
	int axis;
	// get axis of first waveform
	std::string axisStr;
	script >> axisStr;
	if ( axisStr == "vertical" )
	{
		axis = Vertical;
	}
	else if ( axisStr == "horizontal" )
	{
		axis = Horizontal;
	}
	loadWaveformParametersFormSingle( output, cmd, script, variables, axis, wave );
	// get cmd, axis of second waveform
	script >> cmd;
	if ( !isStandardWaveform( cmd ) )
	{
		thrower( "ERROR: standard waveform must be followed by another standard waveform, for each of the horizontal and"
				 " vertical axes" );
	}
	std::string newAxisStr;
	script >> newAxisStr;
	if ( newAxisStr == axisStr )
	{
		thrower( "ERROR: horizontal waveform must be followed by vertical or vertical must be followed by horizontal." );
	}

	if ( axisStr == "vertical" )
	{
		axis = Vertical;
	}
	else if ( axisStr == "horizontal" )
	{
		axis = Horizontal;
	}
	loadWaveformParametersFormSingle( output, cmd, script, variables, axis, wave );
	// get the common things.
	loadCommonWaveParams( script, wave );
}


void NiawgController::handleStandardWaveformFormSingle( NiawgOutput& output, std::string cmd, ScriptStream& script, 
														std::vector<variableType>& variables )
{
	/*
	example syntax:

	gen2const
	Horizontal
	80 1 0
	70 1 0
	#
	gen1freqramp
	Vertical
	lin 70 80 1 0
	#
	% time, phase option
	0.1 0
	*/

	simpleWaveForm wave;
	loadFullWave( output, cmd, script, variables, wave );
	output.waveFormInfo.push_back( toWaveInfoForm( wave ) );
	// +1 to avoid the default waveform.
	if ( output.isDefault )
	{
		output.waveFormInfo.back( ).core.name = "Waveform0";
	}
	else
	{
		output.waveFormInfo.back( ).core.name = "Waveform" + str( output.waveFormInfo.size( ) );
	}
	// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
	// have to rewrite the script to insert the new waveform name into it.
	output.niawgLanguageScript += "generate " + output.waveFormInfo.back( ).core.name + "\n";
}


void NiawgController::handleStandardWaveformForm( NiawgOutput& output, profileSettings profile,
												  niawgPair<std::string> command, niawgPair<ScriptStream>& scripts,
												  debugInfo& options, std::vector<variableType>& variables )
{
	loadWaveformParametersForm( output, profile, command, options, scripts, variables );
	// +1 to avoid the default waveform.
	if ( output.isDefault )
	{
		output.waveFormInfo.back( ).core.name = "Waveform0";
	}
	else
	{
		output.waveFormInfo.back( ).core.name = "Waveform" + str( output.waveFormInfo.size( ));
	}
	// append script with the relevant command. This needs to be done even if variable waveforms are used, because I don't want to
	// have to rewrite the script to insert the new waveform name into it.
	output.niawgLanguageScript += "generate " + output.waveFormInfo.back( ).core.name + "\n";
}


void NiawgController::finalizeScript( ULONGLONG repetitions, std::string name, std::vector<std::string> workingUserScripts,
									  std::vector<ViChar>& userScriptSubmit, bool repeatForever )
{
	// format the script to send to the 5451 according to the accumulation number and based on the number of sequences.
	std::string finalUserScriptString = "script " + name + "\n";
	if ( repeatForever )
	{
		finalUserScriptString += "repeat forever\n";
		for ( UINT sequenceInc = 0; sequenceInc < workingUserScripts.size( ); sequenceInc++ )
		{
			finalUserScriptString += workingUserScripts[sequenceInc];
		}
		finalUserScriptString += "end repeat\n";
	}
	else
	{
		// repeat the script once for every accumulation.
		for ( UINT accumCount = 0; accumCount < repetitions; accumCount++ )
		{
			for ( UINT sequenceInc = 0; sequenceInc < workingUserScripts.size( ); sequenceInc++ )
			{
				finalUserScriptString += workingUserScripts[sequenceInc];
			}
		}
	}
	// the NIAWG requires that the script file must end with "end script".
	finalUserScriptString += "end Script";

	// Convert script string to ViConstString. +1 for a null character on the end.
	userScriptSubmit = std::vector<ViChar>( finalUserScriptString.begin( ), finalUserScriptString.end( ) );
}


void NiawgController::flashFormToOutput( waveInfoForm& waveForm, waveInfo& wave, std::vector<variableType>& variables, 
										 UINT variation )
{
	wave.core.time = waveForm.core.time.evaluate( variables, variation ) * 1e-3;
	// convert to MHz
	wave.flash.flashCycleFreq = waveForm.flash.flashCycleFreq.evaluate( variables, variation ) * 1e6;
	wave.flash.deadTime = waveForm.flash.deadTime.evaluate( variables, variation ) * 1e-9;
	wave.flash.flashNumber = waveForm.flash.flashNumber;
	wave.flash.isFlashing = waveForm.flash.isFlashing;
	wave.flash.flashWaves.resize( waveForm.flash.flashWaves.size( ) );
	wave.core.name = waveForm.core.name;
	for ( auto waveInc : range( waveForm.flash.flashWaves.size( ) ) )
	{
		simpleFormToOutput( waveForm.flash.flashWaves[waveInc], wave.flash.flashWaves[waveInc], variables, variation );
	}
}


void NiawgController::rerngFormToOutput( waveInfoForm& waveForm, waveInfo& wave, 
											 std::vector<variableType>& variables, UINT variation )
{
	wave.rearrange.isRearrangement = waveForm.rearrange.isRearrangement;
	wave.rearrange.freqPerPixel = waveForm.rearrange.freqPerPixel;
	wave.rearrange.lowestFreqs = waveForm.rearrange.lowestFreqs;
	wave.rearrange.moveLimit = waveForm.rearrange.moveLimit;
	wave.rearrange.target = waveForm.rearrange.target;
	wave.rearrange.finalPosition = waveForm.rearrange.finalPosition;
	wave.rearrange.timePerMove = waveForm.rearrange.timePerMove.evaluate(variables, variation);
	simpleFormToOutput( waveForm.rearrange.staticWave, wave.rearrange.staticWave, variables, variation );
	simpleFormToOutput( waveForm.rearrange.fillerWave, wave.rearrange.fillerWave, variables, variation );
}



void NiawgController::flashVaries( waveInfoForm& wave )
{
	if ( wave.core.time.varies( ) || wave.flash.flashCycleFreq.varies( ) || wave.flash.deadTime.varies( ) )
	{
		wave.core.varies = true;
	}
	// convert to MHz
	for ( auto& flashWave : wave.flash.flashWaves )
	{
		simpleFormVaries( flashWave );
		if ( flashWave.varies )
		{
			wave.core.varies = true;
		}
	}
}


void NiawgController::writeFlashing( waveInfo& wave, debugInfo& options, UINT variation )
{
	/// get waveforms to flash.
	/// load these waveforms into the flashing info	
	wave.core.time = 0;
	for ( auto& flashWave : wave.flash.flashWaves )
	{
		wave.core.time += flashWave.time;
	}
	createFlashingWave( wave, options );
	if ( variation != 0 )
	{
		fgenConduit.deleteWaveform( cstr( wave.core.name ) );
	}
	fgenConduit.allocateNamedWaveform( cstr( wave.core.name ), long( wave.core.waveVals.size( ) / 2 ) );
	// write named waveform on the device. Now the device knows what "waveform0" refers to when it sees it in 
	// the script. 
	fgenConduit.writeNamedWaveform( cstr( wave.core.name ), long( wave.core.waveVals.size( ) ),
									wave.core.waveVals.data( ) );
	//writeToFile( wave.core.waveVals );

}


void NiawgController::deleteRerngWave( )
{
	fgenConduit.deleteWaveform( cstr(rerngWaveName) );
}


// generic stream.
void NiawgController::streamWaveform()
{
	fgenConduit.writeNamedWaveform( cstr(streamWaveName), streamWaveformVals.size(), streamWaveformVals.data());
}


// expects the rearrangmenet waveform to have already been filled into rearrangeWaveVals.
void NiawgController::streamRerng()
{
	fgenConduit.writeNamedWaveform( cstr( rerngWaveName ), rerngWaveVals.size(), rerngWaveVals.data() );
}


// calculates the data, mixes it, and cleans up the calculated data.
void NiawgController::finalizeStandardWave( simpleWave& wave, debugInfo& options )
{
	// prepare each channel
	generateWaveform( wave.chan[Horizontal], options, wave.sampleNum, wave.time );
	generateWaveform( wave.chan[Vertical], options, wave.sampleNum, wave.time );
	// mix
	mixWaveforms( wave, options.outputNiawgWavesToText );
	// clear channel data, no longer needed.
	wave.chan[Vertical].wave.clear( );
	// not sure if shrink_to_fit is necessary, but might help with data management
	wave.chan[Vertical].wave.shrink_to_fit( );
	wave.chan[Horizontal].wave.clear( );
	wave.chan[Horizontal].wave.shrink_to_fit( );
}


// which should be Horizontal or Vertical.
void NiawgController::setDefaultWaveformScript( )
{
	fgenConduit.setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, cstr("DefaultConfigScript"));
}


void NiawgController::createFlashingWave( waveInfo& wave, debugInfo options )
{
	/// quick check
	if ( !wave.flash.isFlashing )
	{
		thrower( "ERROR: tried to create flashing wave data for a waveform that wasn't flashing!" );
	}
	/// create the data for each wave that's gonna be flashed between.
	for ( UINT waveInc = 0; waveInc < wave.flash.flashNumber; waveInc++ )
	{
		finalizeStandardWave( wave.flash.flashWaves[waveInc], options );
	}
	double staticMovingRatio = 1;
	if ( wave.flash.flashWaves.size( ) <= 2 )
	{
		staticMovingRatio = wave.flash.flashWaves.back( ).time / wave.flash.flashWaves.front( ).time;
	}
	mixFlashingWaves( wave, wave.flash.deadTime, staticMovingRatio );
}


/**
  * this is separated from the above function so that I can call it with pre-written waves.
  * dead time is in seconds!
  * staticMovingRatio is a ratio.
  */
void NiawgController::mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio )
{
	// firstDutyCycle is set to -1 if doing a non-rearranging waveform.
	if ( wave.flash.flashNumber > 2 )
	{
		thrower( "ERROR: firstDutyCycle is set to a non-negative value (negative value is the dummy value for this "
				 "input case), but more than 2 flashing waveforms! This is considered undefined and an error." );
	}
	/// then mix them to create the flashing version.
	// total period time in seconds...
	double period = 1.0 / wave.flash.flashCycleFreq;
	// total period in samples...S
	long totalPeriodInSamples = long( period * NIAWG_SAMPLE_RATE + 0.5 );
	long samplesPerWavePerPeriod = totalPeriodInSamples / wave.flash.flashNumber;
	// *2 because of mixing
	long deadSamples = deadTime * NIAWG_SAMPLE_RATE * 2;

	std::vector<double> samplesInWave( wave.flash.flashNumber );
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
		samplesInWave[1] = staticMovingRatio * totalPeriodInSamples / (1 + staticMovingRatio);
		// moving wave
		samplesInWave[0] = totalPeriodInSamples - samplesInWave[1];
	}
	if ( !(fabs( std::floor( wave.core.time / period ) - wave.core.time / period ) < 1e-9) )
	{
		thrower( "ERROR: flashing cycle time doesn't result in an integer number of flashing cycles during the given waveform time!"
				 " This is not allowed currently." );
	}
	long cycles = long( std::floor( wave.core.time / period ) );
	std::vector<ULONG> sampleNum( wave.flash.flashNumber, 0 );
	ULONG mixedSample( 0 );
	/// mix the waves together
	wave.core.waveVals.resize( 2 * waveformSizeCalc( wave.core.time ) );
	for ( auto cycleInc : range( cycles ) )
	{
		for ( auto waveInc : range( wave.flash.flashNumber ) )
		{
			// samplesPerWavePerPeriod * 2 because need to account for the mixed nature of the waveform I'm adding.
			while ( sampleNum[waveInc] < 2*int(( cycleInc + 1 ) * samplesInWave[waveInc]))
			{
				if ( sampleNum[waveInc] > 2*int(( cycleInc + 1 ) * samplesInWave[waveInc]) - deadSamples )
				{
					// not in duty cycle, NIAWG is to output nothing.
					wave.core.waveVals[mixedSample] = 0;
				}
				else
				{
					
					wave.core.waveVals[mixedSample] = wave.flash.flashWaves[waveInc].waveVals[sampleNum[waveInc]];
				}
				mixedSample++;
				sampleNum[waveInc]++;
			}
		}
	}

	/// cleanup
	// should be good now. Immediately delete the old waveforms. Wait until after all cycles done.
	for ( auto waveInc : range( wave.flash.flashNumber ) )
	{
		wave.flash.flashWaves[waveInc].waveVals.clear( );
		wave.flash.flashWaves[waveInc].waveVals.shrink_to_fit( );
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


void NiawgController::turnOff()
{
	fgenConduit.configureOutputEnabled(VI_FALSE);
	fgenConduit.abortGeneration();
	on = false;
}


void NiawgController::turnOn()
{
	fgenConduit.configureOutputEnabled(VI_TRUE);
	fgenConduit.initiateGeneration();
	on = true;
}


/****
	* A series of sanity checks on the waveform parameters. This is ment to catch user error. The following checks for...
	* - Phase Continuity between waveforms (not checking repeating waveforms (yet))
	* - Amplitude Continuity between waveforms
	* - Frequency Continuity between waveforms
	* - Sensible Ramping Options (initial and final freq/amp values reflect choice of ramp or no ramp).
	* - Sensible Phase Correction Options
	***/
void NiawgController::checkThatWaveformsAreSensible( std::string& warnings, NiawgOutput& output )
{
	for ( auto axis : AXES )
	{
		for ( UINT waveInc = 2; waveInc < output.waves.size( ); waveInc++ )
		{
			// if two waveforms have the same number of parameters... (elsewise 
			if ( output.waves[waveInc].core.chan[axis].signals.size( ) != output.waves[waveInc - 1].core.chan[axis].signals.size( ) )
			{
				// (in this case, where the # of signals is changing, its non-trivial to assume anything about what the
				// user is doing)
				continue;
			}
			for ( auto signalNum : range(output.waves[waveInc].core.chan[axis].signals.size( )) )
			{
				/// shorthands
				waveSignal& currSig = output.waves[waveInc].core.chan[axis].signals[signalNum];
				waveSignal& prevSig = output.waves[waveInc - 1].core.chan[axis].signals[signalNum];
				///
				if ( currSig.initPower != prevSig.finPower )
				{
					warnings += "Warning: Amplitude jump at waveform #" + str( waveInc ) + " in " + AXES_NAMES[axis]
						+ " component detected!\r\n";
				}
				if ( currSig.freqInit != prevSig.freqFin )
				{
					warnings += "Warning: Frequency jump at waveform #" + str( waveInc ) + " in " + AXES_NAMES[axis]
						+ " component detected!\r\n";
				}
				if ( currSig.initPhase - prevSig.finPhase > CORRECTION_WAVEFORM_ERROR_THRESHOLD )
				{
					warnings += "Warning: Phase jump (greater than what's wanted for correction waveforms) at "
						"waveform #" + str( waveInc ) + " in " + AXES_NAMES[axis] + " component detected!\r\n";
				}
				// if there signal is ramping but the beginning and end amplitudes are the same, that's weird. 
				// It's not actually ramping.
				if ( currSig.powerRampType != "nr" && (currSig.initPower == currSig.finPower) )
				{
					warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to amplitude"
						" ramp, but the initial and final amplitudes are the same. This is not a ramp.\r\n";
				}
				// if there signal is ramping but the beginning and end frequencies are the same, that's weird. 
				// It's not actually ramping.
				if ( currSig.freqRampType != "nr" && (currSig.freqInit == currSig.freqFin) )
				{
					warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to frequency"
						" ramp, but the initial and final frequencies are the same. This is not a ramp.\r\n";
				}
				// if there signal is not ramping but the beginning and end amplitudes are different, that's weird. 
				// It's not actually ramping.
				if ( currSig.powerRampType == "nr" && (currSig.initPower != currSig.finPower) )
				{
					warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to no "
						"amplitude ramp, but the initial and final amplitudes are the different. This is not a ramp, "
						"the initial value will be used.\r\n";
				}
				// if there signal is not ramping but the beginning and end frequencies are different, that's weird. 
				// It's not actually ramping.
				if ( currSig.freqRampType == "nr" && (currSig.freqInit != currSig.freqInit) )
				{
					warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + "is set to no "
						"frequency ramp, but the initial and final frequencies are different. This is not a ramp, the"
						" initial value will be used throughout.\r\n";
				}
				if ( output.waves[waveInc].core.chan[axis].phaseOption != 0 )
				{
					warnings += "Warning: " + AXES_NAMES[axis] + " waveform #" + str( waveInc ) + " has a non-zero "
						"phase correction option, but phase correction has been discontinued! This option being set "
						"won't do anything...";
				}
			}
		}
	}
}


waveInfoForm NiawgController::toWaveInfoForm( simpleWaveForm wave )
{
	waveInfoForm formWave;
	formWave.core.chan = wave.chan;
	formWave.core.name = wave.name;
	formWave.core.time = wave.time;
	formWave.isStreamed = false;
	return formWave;
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
			phase = fmod( 2 * PI * freqList[signalInc] * ((double)testSampleNum / NIAWG_SAMPLE_RATE) 
						  + startPhases[signalInc], 2 * PI );
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
 * This function takes ramp-related information as an input and returns the "position" in the ramp (targetInc.e. the 
 ( amount to add to the initial value due to ramping)
 * that the waveform should be at.
 *
 * @return double is the ramp position.
 *
 * @param size is the total size of the waveform, in numbers of samples
 * @param iteration is the sample number that the waveform is currently at.
 * @param initPos is the initial frequency or amplitude of the waveform.
 * @param finPos is the final frequency or amplitude of the waveform.
 * @param type is the type of ramp being executed, as specified by the reader.
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
///								Rearrangement stuffs
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
This function expects the input to have already been initialized and everything. It's ment to be used in the
"start thread" function, after all but the rearrangement moves have been loaded into the input structure.
*/
void NiawgController::preWriteRerngWaveforms( rerngThreadInput* input )
{
	UINT rows = input->sourceRows;
	UINT cols = input->sourceCols;
	rerngContainer<double> calBias( rows, cols );
	if ( input->rerngOptions.useCalibration )
	{
		// find the calibration whose dimensions match the target
		for ( auto& cal : moveBiasCalibrations )
		{
			if ( cal.getRows( ) == rows && cal.getCols( ) == cols )
			{
				calBias = cal;
			}
		}
	}

	for ( auto row : range( rows ) )
	{
		for ( auto col : range( cols ) )
		{
			rerngMove flashMove, noFlashMove;
			flashMove.col = col;
			flashMove.row = row;
			flashMove.staticMovingRatio = input->rerngOptions.staticMovingRatio;
			flashMove.deadTime = input->rerngOptions.deadTime;
			flashMove.moveTime = input->rerngOptions.moveSpeed;
			flashMove.moveBias = input->rerngOptions.moveBias;
			// up
			noFlashMove.direction = flashMove.direction = up;
			if ( row != rows - 1 )
			{
				if ( input->rerngOptions.useCalibration )
				{
					noFlashMove.moveBias = flashMove.moveBias = calBias( row, col, flashMove.direction );
				}
				noFlashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, noFlashMove.direction,
													  noFlashMove.staticMovingRatio, noFlashMove.moveBias, 
													  noFlashMove.deadTime, input->sourceRows, input->sourceCols, 
													  false );
				flashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, flashMove.direction,
											   flashMove.staticMovingRatio, flashMove.moveBias, flashMove.deadTime,
											   input->sourceRows, input->sourceCols, true );
			}
			input->flashMoves( row, col, flashMove.direction ) = flashMove;
			input->noFlashMoves( row, col, noFlashMove.direction ) = noFlashMove;
			noFlashMove.waveVals = flashMove.waveVals = std::vector<double>( );
			// down
			noFlashMove.direction = flashMove.direction = down;
			if ( row != 0 )
			{
				if ( input->rerngOptions.useCalibration )
				{
					noFlashMove.moveBias = flashMove.moveBias = calBias( row, col, flashMove.direction );
				}
				noFlashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, noFlashMove.direction,
													  noFlashMove.staticMovingRatio, noFlashMove.moveBias,
													  noFlashMove.deadTime, input->sourceRows, input->sourceCols,
													  false );
				flashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, flashMove.direction,
											   flashMove.staticMovingRatio, flashMove.moveBias, flashMove.deadTime,
											   input->sourceRows, input->sourceCols, true );
			}
			input->flashMoves( row, col, flashMove.direction ) = flashMove;
			noFlashMove.waveVals = flashMove.waveVals = std::vector<double>( );

			noFlashMove.direction = flashMove.direction = left;
			if ( col != 0 )
			{
				if ( input->rerngOptions.useCalibration )
				{
					noFlashMove.moveBias = flashMove.moveBias = calBias( row, col, flashMove.direction );
				}
				noFlashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, noFlashMove.direction,
													  noFlashMove.staticMovingRatio, noFlashMove.moveBias,
													  noFlashMove.deadTime, input->sourceRows, input->sourceCols,
													  false );
				flashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, flashMove.direction,
													flashMove.staticMovingRatio, flashMove.moveBias, flashMove.deadTime, 
													input->sourceRows, input->sourceCols, true );
			}
			input->flashMoves( row, col, flashMove.direction ) = flashMove;
			noFlashMove.waveVals = flashMove.waveVals = std::vector<double>( );
			
			noFlashMove.direction = flashMove.direction = right;
			if ( col != cols - 1 )
			{
				if ( input->rerngOptions.useCalibration )
				{
					noFlashMove.moveBias = flashMove.moveBias = calBias( row, col, flashMove.direction );
				}
				noFlashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, noFlashMove.direction,
													  noFlashMove.staticMovingRatio, noFlashMove.moveBias,
													  noFlashMove.deadTime, input->sourceRows, input->sourceCols,
													  false );
				flashMove.waveVals = makeRerngWave( input->rerngWave.rearrange, row, col, flashMove.direction,
											   flashMove.staticMovingRatio, flashMove.moveBias, flashMove.deadTime,
											   input->sourceRows, input->sourceCols, true );
			}
			input->flashMoves( row, col, flashMove.direction ) = flashMove;
		}
	}
	input->flashMoves.setFilledFlag( );
}


std::vector<double> NiawgController::makeRerngWave( rerngInfo& info, UINT row, UINT col, directions direction, 
													double staticMovingRatio, double moveBias, double deadTime, 
													UINT sourceRows, UINT sourceCols, bool needsFlash )
{
	
	// program this move.
	double freqPerPixel = info.freqPerPixel;
	// starts from the top left.
	niawgPair<int> initPos = { row, col };
	niawgPair<int> finPos;
	int rowInt = row, colInt = col;
	UINT movingAxis, movingSize, staticAxis;	
	switch ( direction )
	{
		case up:
			finPos = { rowInt + 1, colInt };
			movingAxis = Vertical;
			staticAxis = Horizontal;
			movingSize = sourceRows;
			break;
		case down:
			finPos = { rowInt - 1, colInt };
			movingAxis = Vertical;
			staticAxis = Horizontal;
			movingSize = sourceRows;
			break;
		case left:
			finPos = { rowInt, colInt - 1 };
			movingAxis = Horizontal;
			staticAxis = Vertical;
			movingSize = sourceCols;
			break;
		case right:
			finPos = { rowInt, colInt + 1 };
			movingAxis = Horizontal;
			staticAxis = Vertical;
			movingSize = sourceCols;
			break;
	}
	simpleWave moveWave;
	moveWave.varies = false;
	moveWave.name = "NOT-USED";
	// needs to match correctly the static waveform.
	moveWave.time = info.timePerMove / (staticMovingRatio + 1);
	moveWave.sampleNum = waveformSizeCalc( moveWave.time );

	double movingFrac = moveBias;
	// split the remaining bias between all of the other movingSize-2 signals.
	double nonMovingFrac = (1 - movingFrac) / (movingSize - 2);
	/// handle moving axis
	// 1 less signal because of the two locations that the moving tweezer spans
	moveWave.chan[movingAxis].signals.resize( movingSize - 1 );
	bool foundMoving = false;
	UINT gridLocation = 0;
	for ( auto signalNum : range( movingSize - 1 ) )
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
				sig.freqInit = ((info.target.getCols( ) - initPos[movingAxis] - 1)
								 * freqPerPixel + info.lowestFreqs[movingAxis]) * 1e6;
				sig.freqFin = ((info.target.getCols( ) - finPos[movingAxis] - 1)
								* freqPerPixel + info.lowestFreqs[movingAxis]) * 1e6;
			}
			else
			{
				sig.freqInit = (initPos[movingAxis] * freqPerPixel + info.lowestFreqs[movingAxis]) * 1e6;
				sig.freqFin = (finPos[movingAxis] * freqPerPixel + info.lowestFreqs[movingAxis]) * 1e6;
			}
		}
		else
		{
			sig.initPower = nonMovingFrac;
			sig.finPower = nonMovingFrac;
			sig.freqRampType = "nr";
			if ( movingAxis == Horizontal )
			{
				sig.freqInit = ((info.target.getCols( ) - gridLocation - 1) * freqPerPixel
								 + info.lowestFreqs[movingAxis]) * 1e6;
				sig.freqFin = sig.freqInit;
			}
			else
			{
				sig.freqInit = (gridLocation * freqPerPixel + info.lowestFreqs[movingAxis]) * 1e6;
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
		sig.freqInit = ((info.target.getCols( ) - initPos[staticAxis] - 1) * freqPerPixel
						 + info.lowestFreqs[staticAxis]) * 1e6;
		sig.freqFin = sig.freqInit;
	}
	else
	{
		// convert to Hz
		sig.freqInit = (initPos[staticAxis] * freqPerPixel + info.lowestFreqs[staticAxis])*1e6;
		sig.freqFin = sig.freqInit;
	}
	/// finalize info & calculate things
	finalizeStandardWave( moveWave, debugInfo( ) );
	// now put together into small temporary flashing wave
	waveInfo flashMove;
	flashMove.core.time = info.timePerMove;
	flashMove.flash.isFlashing = true;
	flashMove.flash.flashNumber = 2;
	flashMove.flash.deadTime = deadTime;
	if ( fabs( info.staticWave.time + moveWave.time - info.timePerMove ) > 1e-9 )
	{
		thrower( "ERROR: static wave and moving wave don't add up to the total time of the flashing wave! "
				 "Sizes were " + str( info.staticWave.waveVals.size( ) ) + " and "
				 + str( moveWave.waveVals.size( ) ) + " respectively.\r\n" );
	}
	flashMove.flash.flashWaves.push_back( moveWave );
	flashMove.flash.flashWaves.push_back( info.staticWave );
	flashMove.flash.flashCycleFreq = info.flashingFreq;
	mixFlashingWaves( flashMove, deadTime, staticMovingRatio );

	return flashMove.core.waveVals;
}


void NiawgController::rerngOptionsFormToFinal( rerngOptionsForm& form, rerngOptions& data, 
											   std::vector<variableType>& variables, UINT variation )
{
	data.active = form.active;
	data.deadTime = form.deadTime.evaluate( variables, variation );
	data.flashingRate = form.flashingRate.evaluate( variables, variation );
	data.moveBias = form.moveBias.evaluate( variables, variation );
	data.moveSpeed = form.moveSpeed.evaluate( variables, variation );
	data.staticMovingRatio = form.staticMovingRatio.evaluate( variables, variation );
	//
	data.outputIndv = form.outputIndv;
	data.outputInfo = form.outputInfo;
	data.preprogram = form.preprogram;
	data.useCalibration = form.useCalibration;
}


void NiawgController::startRerngThread( std::vector<std::vector<bool>>* atomQueue, waveInfo wave, Communicator* comm, 
										std::mutex* rearrangerLock, chronoTimes* andorImageTimes, 
										chronoTimes* grabTimes, std::condition_variable* rearrangerConditionWatcher,
										rerngOptions rearrangeInfo, atomGrid grid )
{
	threadStateSignal = true;
	rerngThreadInput* input = new rerngThreadInput( grid.height, grid.width);
	input->sourceRows = grid.height;
	input->sourceCols = grid.width;
	input->rerngOptions = rearrangeInfo;
	input->pictureTimes = andorImageTimes;
	input->grabTimes = grabTimes;
	input->rerngLock = rearrangerLock;
	input->threadActive = &threadStateSignal;
	input->comm = comm;
	input->niawg = this;
	input->atomsQueue = atomQueue;
	input->rerngWave = wave;
	input->rerngConditionWatcher = rearrangerConditionWatcher;
	if ( rearrangeInfo.preprogram )
	{
		preWriteRerngWaveforms( input );
	}
	UINT rearrangerId;
	// start the thread with ~100MB of memory (it may get rounded to some page size)
	rerngThreadHandle = (HANDLE)_beginthreadex( 0, 1e7, NiawgController::rerngThreadProcedure, (void*)input,
													 STACK_SIZE_PARAM_IS_A_RESERVATION, &rearrangerId );
	if ( !rerngThreadHandle )
	{
		errBox( "beginThreadEx error: " + str( GetLastError( ) ) );
	}
	if ( !SetThreadPriority( rerngThreadHandle, THREAD_PRIORITY_TIME_CRITICAL ) )
	{
		errBox( "Set Thread priority error: " + str( GetLastError( ) ) );
	}
}


bool NiawgController::rerngThreadIsActive( )
{
	return threadStateSignal;
}


// calculate (and return) the wave that will take the atoms from the target position to the final position.
std::vector<double> NiawgController::calcFinalPositionMove( niawgPair<ULONG> targetPos, niawgPair<ULONG> finalPos, 
														    double freqSpacing, Matrix<bool> target, 
														    niawgPair<double> cornerFreqs, double moveTime )
{
	if ( target.getRows() == 0 || target.getCols() == 0 )
	{
		thrower( "ERROR: invalid target size in calcFinalPositionMove function. target must be a non-empty 2D Vector." );
	}
	simpleWave moveWave;
	moveWave.varies = false;
	moveWave.name = "NA";
	niawgPair<double> freqChange;
	freqChange[Vertical] = freqSpacing * (double( finalPos[Vertical] ) - double(targetPos[Vertical]));
	freqChange[Horizontal] = freqSpacing * (double( finalPos[Horizontal] ) - double( targetPos[Horizontal] ));
	if ( (fabs(freqChange[Vertical]) < 1e-9) && (fabs( freqChange[Horizontal] ) < 1e-9))
	{
		return std::vector<double>();
	}
	moveWave.chan[Vertical].signals.resize( target.getRows() );
	moveWave.chan[Horizontal].signals.resize( target.getCols() );
	// this is pretty arbitrary right now. In principle can prob be very fast.
	moveWave.time = moveTime;
	moveWave.sampleNum = waveformSizeCalc( moveWave.time );
	simpleWave waitWave = moveWave;
	// fill wave info
	for ( auto axis : AXES )
	{
		UINT count = 0;
		double targetCornerFreq = cornerFreqs[axis] + freqSpacing * targetPos[axis];
		for ( auto sigInc : range(moveWave.chan[axis].signals.size()) )
		{
			auto& sig = moveWave.chan[axis].signals[sigInc];
			auto& waitSig = waitWave.chan[axis].signals[sigInc];
			sig.freqInit = (targetCornerFreq + count * freqSpacing) * 1e6;
			waitSig.freqFin = waitSig.freqInit = sig.freqInit;
			sig.freqFin = sig.freqInit + freqChange[axis] * 1e6;
			if ( sig.freqInit == sig.freqFin )
			{
				sig.freqRampType = "nr";
			}
			else
			{
				sig.freqRampType = "lin";
			}
			waitSig.freqRampType = waitSig.powerRampType = "nr";			
			waitSig.initPower = waitSig.finPower = 1;
			sig.initPower = sig.finPower = 1;
			sig.powerRampType = "nr";
			sig.initPhase = 0;
			waitSig.initPhase = 0;
			count++;
		}
	}
	finalizeStandardWave( moveWave, debugInfo( ) );
	finalizeStandardWave( waitWave, debugInfo( ) );
	std::vector<double> vals( waitWave.waveVals );
	vals.insert( vals.end(), moveWave.waveVals.begin( ), moveWave.waveVals.end( ) );
	return vals;
}


/// things that might make faster:
// preprogram all possible fillers
// 1D rearranging instead of 2D
// faster moves / lower sample rate
// don't use vector
// make rearrangement algorithm work with flattened matrix
UINT __stdcall NiawgController::rerngThreadProcedure( void* voidInput )
{
	rerngThreadInput* input = (rerngThreadInput*)voidInput;
	std::vector<bool> triedRearranging;
	std::vector<double> streamTime, triggerTime, resetPositionTime, picHandlingTime, picGrabTime, rerngCalcTime, 
		moveCalcTime, finishingCalcTime, finMoveCalcTime;
	std::vector<UINT> numberMoves;
	chronoTimes startCalc, stopReset, stopStream, stopTrigger, stopRerngCalc, finMoveCalc, stopMoveCalc, stopAllCalc;
	std::ofstream outFile;
	UINT counter = 0;
	try
	{
		UINT rows = input->rerngWave.rearrange.target.getRows( );
		UINT cols = input->rerngWave.rearrange.target.getCols( );
		rerngContainer<double> calBias( input->sourceRows, input->sourceCols );
		if ( input->rerngOptions.useCalibration )
		{
			// find the calibration whose dimensions match the target
			for ( auto& cal : input->niawg->moveBiasCalibrations )
			{
				if ( cal.getRows( ) == rows && cal.getCols( ) == cols )
				{
					calBias = cal;
				}
			}
		}
		if ( input->rerngOptions.outputInfo )
		{
			outFile.open( DEBUG_OUTPUT_LOCATION + "Rearranging-Event-Info.txt" );
			if ( !outFile.is_open( ) )
			{
				thrower( "ERROR: Info file failed to open!" );
			}
			outFile << "Target:\n";
			outFile << input->rerngWave.rearrange.target.print( ) << "\n";
		}
		// wait for data
		while ( *input->threadActive )
		{
			std::vector<bool> tempAtoms;
			if ( input->atomsQueue->size( ) == 0 )
			{
				// wait for the next image using a condition_variable.
				std::unique_lock<std::mutex> locker( *input->rerngLock );
				input->rerngConditionWatcher->wait( locker );
				if ( !*input->threadActive )
				{
					break;
				}
				if ( input->atomsQueue->size( ) == 0)
				{
					input->comm->sendStatus( "Rearrangement Thread woke up???" );
					continue;
				}
			}
			{
				std::unique_lock<std::mutex> locker( *input->rerngLock );
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
			startCalc.push_back(chronoClock::now( ));			
			rerngInfo& info = input->rerngWave.rearrange;
			info.timePerMove = input->rerngOptions.moveSpeed;
			info.flashingFreq = input->rerngOptions.flashingRate;
			// right now I need to re-shape the atomqueue matrix. I should probably modify Kai's code to work with a 
			// flattened source matrix for speed.
			Matrix<bool> source(input->sourceRows, input->sourceCols, 0);
			UINT count = 0;
			for ( auto colCount : range( source.getCols() ) )
			{
				for ( auto rowCount : range( source.getRows() ) )
				{
					source(source.getRows() - 1 - rowCount, colCount) = tempAtoms[count++];
				}
			}
			std::vector<simpleMove> moveSequence;
			niawgPair<ULONG> finPos;
			try
			{
				smartRearrangement( source, info.target, finPos, info.finalPosition, moveSequence, input->rerngOptions);
			}
			catch ( Error& )
			{
				// as of now, just ignore.
			}
			numberMoves.push_back( moveSequence.size( ) );
			stopRerngCalc.push_back( chronoClock::now( ) );
			input->niawg->rerngWaveVals.clear( );
			/// program niawg
			debugInfo options;
			for ( auto move : moveSequence )
			{
				// program this move.
				directions dir;
				if ( move.initRow - move.finRow == 1 )
				{
					dir = down;
				}
				else if ( move.initRow - move.finRow == -1 )
				{
					dir = up;
				}
				else if ( move.initCol - move.finCol == 1 )
				{
					dir = left;
				}
				else if ( move.initCol - move.finCol == -1 )
				{
					dir = right;
				}
				std::vector<double> vals;
				double bias;
				if ( input->rerngOptions.useCalibration )
				{
					bias = calBias( move.initRow, move.initCol, dir );
				}
				else
				{
					bias = input->rerngOptions.moveBias;
				}
				if ( input->rerngOptions.preprogram )
				{
					vals = input->flashMoves( move.initRow, move.initCol, dir ).waveVals;
				}
				else
				{
					vals = input->niawg->makeRerngWave( info, move.initRow, move.initCol, dir,
														input->rerngOptions.staticMovingRatio, bias,
														input->rerngOptions.deadTime, input->sourceRows,
														input->sourceCols, true );
				}
				input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), vals.begin( ), vals.end( ) );
			}
			stopMoveCalc.push_back( chronoClock::now( ) );
			/// Finishing Move to move the atoms to the desired location.
			std::vector<double> finalMove;
			finalMove = input->niawg->calcFinalPositionMove( finPos, info.finalPosition, info.freqPerPixel, 
															 info.target, info.lowestFreqs, 
															 input->rerngOptions.finalMoveTime );
			input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), finalMove.begin( ),
												finalMove.end( ) );
			finMoveCalc.push_back( chronoClock::now( ) );
			// the filler wave holds the total length of the wave. Add the differnece in size between the filler wave
			// size and the existing size to fill out the rest of the vector.
			input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), info.fillerWave.waveVals.begin( ),
												info.fillerWave.waveVals.begin( ) + info.fillerWave.waveVals.size()
												- input->niawg->rerngWaveVals.size() );
			stopAllCalc.push_back(chronoClock::now( ));
			input->niawg->streamRerng( );
			stopStream.push_back( chronoClock::now( ) );
			input->niawg->fgenConduit.sendSoftwareTrigger( );
			stopTrigger.push_back( chronoClock::now( ));
			input->niawg->fgenConduit.resetWritePosition( );
			stopReset.push_back( chronoClock::now( ));
			if ( moveSequence.size( ) )
			{
				triedRearranging.push_back( true );
			}
			else
			{
				triedRearranging.push_back( false );
			}
			//input->niawg->writeToFile( input->niawg->rerngWaveVals );
			input->niawg->rerngWaveVals.clear( );
			if ( moveSequence.size( ) != 0 )
			{
				if ( input->rerngOptions.outputIndv )
				{
					input->comm->sendStatus( "Tried Moving, " + str( moveSequence.size() ) + " Moves. Move Calc Time:"
											 + str( std::chrono::duration<double>( stopMoveCalc.back()
																				   - startCalc.back()).count()) 
											 +  ", Fin Move Time:"
											 + str( std::chrono::duration<double>( finMoveCalc.back( )
																				   - stopMoveCalc.back( ) ).count( ) )
											 + " Code Time = "
											 + str( std::chrono::duration<double>( stopReset.back() 
																				   - startCalc.back()).count())
											 + "\r\n" );
				}
			}
			if ( input->rerngOptions.outputInfo )
			{
				outFile << "Rep # " << counter << "\n";
				outFile << "Source: ";
				UINT counter = 0;
				for ( auto elem : source )
				{
					outFile << elem << ", ";
					if ( ++counter % source.getCols( ) == 0 )
					{
						outFile << "; ";
					}
				}
				outFile << "\nTarget Location: " + str( finPos[0] ) + ' ' + str( finPos[1] ) + "\n";
				outFile << "Moves:\n";
				UINT moveCount = 0;
				for ( auto move : moveSequence )
				{
					outFile << moveCount++ << " " << move.initRow << " " << move.finRow << " " << move.initCol << " "
						<< move.finCol << "\n";
				}
			}
			counter++;
		}
		for ( auto inc : range( startCalc.size( ) ) )
		{
			finMoveCalcTime.push_back( std::chrono::duration<double>( finMoveCalc[inc] - stopMoveCalc[inc] ).count( ) );
			streamTime.push_back( std::chrono::duration<double>( stopStream[inc] - stopAllCalc[inc] ).count( ) );
			triggerTime.push_back( std::chrono::duration<double>( stopTrigger[inc] - stopStream[inc] ).count( ) );
			rerngCalcTime.push_back( std::chrono::duration<double>( stopRerngCalc[inc] - startCalc[inc] ).count( ) );
			moveCalcTime.push_back( std::chrono::duration<double>( stopMoveCalc[inc] - stopRerngCalc[inc] ).count( ) );
			finishingCalcTime.push_back( std::chrono::duration<double>( stopAllCalc[inc] - stopMoveCalc[inc] ).count( ) );
			resetPositionTime.push_back( std::chrono::duration<double>( stopReset[inc] - stopTrigger[inc] ).count( ) );
			picHandlingTime.push_back( std::chrono::duration<double>( startCalc[inc] - (*input->grabTimes)[inc] ).count( ) );
			picGrabTime.push_back( std::chrono::duration<double>( (*input->grabTimes)[inc] - (*input->pictureTimes)[inc] ).count( ) );
		}
		(*input->pictureTimes).clear( );
		(*input->grabTimes).clear( );

		std::ofstream dataFile( DEBUG_OUTPUT_LOCATION + "rearrangementLog.txt" );
		dataFile
			<< "PicHandlingTime\t"
			<< "PicGrabTime\t"
			<< "Rearrangement-Calc-Time\t"
			<< "Movement-Calc-Time\t"
			<< "Finishing-Calc-Time\t"
			<< "ResetPositionTime\t"
			<< "StreamTime\t"
			<< "TriggerTime\n";
		if ( !dataFile.is_open( ) )
		{
			errBox( "ERROR: data file failed to open for rearrangement log!" );
		}
		for ( auto count : range( triedRearranging.size( ) ) )
		{
			dataFile
				<< picHandlingTime[count] << "\t"
				<< picGrabTime[count] << "\t"
				<< rerngCalcTime[count] << "\t"
				<< moveCalcTime[count] << "\t"
				<< finishingCalcTime[count] << "\t"
				<< resetPositionTime[count] << "\t"
				<< streamTime[count] << "\t"
				<< triggerTime[count] << "\t"
				<< numberMoves[count] << "\n";
		}
		dataFile.close( );
	}
	catch ( Error& err )
	{
		errBox( "ERROR in rearrangement thread! " + err.whatStr( ) );
	}
	if ( outFile.is_open( ) )
	{
		outFile.close( );
	}
	input->comm->sendStatus( "Exiting rearranging thread.\r\n" );
	delete input;
	return 0;
}


void NiawgController::smartRearrangement( Matrix<bool> source, Matrix<bool> target, niawgPair<ULONG>& finTargetPos, 
										  niawgPair<ULONG> finalPos, std::vector<simpleMove> &operationsMatrix, 
										  rerngOptions options )
{
	if ( source.getRows() == target.getRows() && source.getCols() == target.getCols() )
	{
		// dimensions match, no flexibility.
		rearrangement( source, target, operationsMatrix );
		finTargetPos = { 0,0 };
		return;
	}

	switch (options.smartOption)
	{
		case smartRerngOption::none:
		{
			// finTarget is the correct size, has the original target at finalPos, and zeros elsewhere.
			Matrix<bool> finTarget( source.getRows( ), source.getCols( ), 0 );
			for ( auto rowInc : range(target.getRows()))
			{
				for ( auto colInc : range(target.getCols()))
				{
					finTarget(rowInc + finalPos[Vertical], colInc + finalPos[Horizontal]) = target(rowInc, colInc);
				}
			}
			rearrangement( source, finTarget, operationsMatrix );
			finTargetPos = finalPos;
			return;
		}
		case smartRerngOption::convolution:
		{
			//
			finTargetPos = convolve( source, target );
			Matrix<bool> finTarget( source.getRows( ), source.getCols( ), 0 );
			for ( auto rowInc : range( target.getRows( ) ) )
			{
				for ( auto colInc : range( target.getCols( ) ) )
				{
					finTarget( rowInc + finTargetPos[Vertical], colInc + finTargetPos[Horizontal] ) = target( rowInc, colInc );
				}
			}
			rearrangement( source, finTarget, operationsMatrix );
			break;
		}
		case smartRerngOption::full:
		{
			UINT leastMoves = UINT_MAX;
			for ( auto startRowInc : range( source.getRows() - target.getRows() + 1 ) )
			{
				if ( leastMoves == 0 )
				{
					break;
				}
				for ( auto startColInc : range( source.getCols( ) - target.getCols( ) + 1 ) )
				{
					// create the potential target with the correct offset.
					// finTarget is the correct size, has the original target at finalPos, and zeros elsewhere.
					Matrix<bool> potentialTarget( source.getRows( ), source.getCols( ), 0 );
					for ( auto rowInc : range( target.getRows() ) )
					{
						for ( auto colInc : range( target.getCols() ) )
						{
							potentialTarget(rowInc + startRowInc, colInc + startColInc) = target(rowInc, colInc);
						}
					}
					std::string targ = potentialTarget.print( );
					//errBox( targ );
					std::vector<simpleMove> potentialMoves;
					rearrangement( source, potentialTarget, potentialMoves );
					if ( potentialMoves.size( ) < leastMoves )
					{
						// new record.
						operationsMatrix = potentialMoves;
						finTargetPos = { startRowInc, startColInc };
						leastMoves = potentialMoves.size( );
						if ( leastMoves == 0 )
						{
							// not possible to move to final location
							break;
						}
					}
				}
			}
			return;
		}
	}
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


double NiawgController::minCostMatching( Matrix<double> cost, std::vector<int> &sourceMates, std::vector<int> &targetMates )
{
	/// 
	UINT numSources = cost.getRows( );
	UINT numTargets = cost.getCols( );
	// construct dual feasible solution

	// each element of u represents one of the sources, and the value of that element is the distance of that source to 
	// the closest target.
	std::vector<double> u( numSources );
	// v is more complicated.
	std::vector<double> v( numTargets );
	// 
	for (int sourceInc = 0; sourceInc < numSources; sourceInc++)
	{
		u[sourceInc] = cost(sourceInc,0);
		for (int targetInc = 1; targetInc < numTargets; targetInc++)
		{
			u[sourceInc] = min( u[sourceInc], cost(sourceInc, targetInc) );
		}
	}

	for (int targetInc = 0; targetInc < numTargets; targetInc++)
	{
		v[targetInc] = cost(0,targetInc) - u[0];
		for (int sourceInc = 1; sourceInc < numSources; sourceInc++)
		{
			v[targetInc] = min( v[targetInc], cost(sourceInc,targetInc) - u[sourceInc] );
		}
	}

	// construct primal solution satisfying complementary slackness
	// -1 indicates unmatched.
	sourceMates = std::vector<int>( numSources, -1 );
	targetMates = std::vector<int>( numTargets, -1 );
	int numberMated = 0;

	for (int sourceInc = 0; sourceInc < numSources; sourceInc++)
	{
		for (int targetInc = 0; targetInc < numTargets; targetInc++)
		{
			if (targetMates[targetInc] != -1)
			{
				// already matched.
				continue;
			}
			if (fabs( cost(sourceInc,targetInc) - u[sourceInc] - v[targetInc] ) < 1e-10)
			{
				sourceMates[sourceInc] = targetInc;
				targetMates[targetInc] = sourceInc;
				numberMated++;
				break;
			}
		}
	}
	
	std::vector<double> dist( numTargets );
	std::vector<int> dad( numSources );
	std::vector<bool> seen( numTargets, false);

	// repeat until primal solution is feasible
	while (numberMated < numSources)
	{
		// find an unmatched left node
		int currentUnmatchedSource = 0;
		// I think there must be at least one because numberMated < rows.
		while (sourceMates[currentUnmatchedSource] != -1)
		{
			currentUnmatchedSource++;
			if ( currentUnmatchedSource >= sourceMates.size( ) )
			{
				thrower( "ERROR: rearrangement Error! all mateColumn are matched but numberMated < rows!" );
			}
		}

		// initialize Dijkstra ...?
		fill( dad.begin(), dad.end(), -1 );
		fill( seen.begin(), seen.end(), false );
		for (auto targetInc : range(numTargets))
		{
			dist[targetInc] = cost(currentUnmatchedSource,targetInc) - u[currentUnmatchedSource] - v[targetInc];
		}

		int closestTarget = 0;
		while (true)
		{
			// find closest target
			closestTarget = -1;
			for (int targetInc = 0; targetInc < numTargets; targetInc++)
			{
				if (seen[targetInc])
				{
					continue;
				}
				if (closestTarget == -1 || dist[targetInc] < dist[closestTarget])
				{
					closestTarget = targetInc;
				}
			}
			seen[closestTarget] = true;

			// termination condition
			if (targetMates[closestTarget] == -1)
			{
				break;
			}

			// relax neighbors
			const int closestTargetMate = targetMates[closestTarget];

			for (int targetInc = 0; targetInc < numTargets; targetInc++)
			{
				if (seen[targetInc])
				{
					continue;
				}

				const double new_dist = dist[closestTarget] + cost( closestTargetMate,targetInc)
					- u[closestTargetMate] - v[targetInc];
				if (dist[targetInc] > new_dist)
				{
					dist[targetInc] = new_dist;
					dad[targetInc] = closestTarget;
				}
			}
		}

		// update dual variables
		for (auto targetInc : range(numTargets))
		{
			if (targetInc == closestTarget || !seen[targetInc])
			{
				continue;
			}

			const int closestTargetMate = targetMates[targetInc];
			v[targetInc] += dist[targetInc] - dist[closestTarget];
			u[closestTargetMate] -= dist[targetInc] - dist[closestTarget];
		}

		u[currentUnmatchedSource] += dist[closestTarget];
		// augment along path
		while (dad[closestTarget] >= 0)
		{
			const int d = dad[closestTarget];
			targetMates[closestTarget] = targetMates[d];
			sourceMates[targetMates[closestTarget]] = closestTarget;
			closestTarget = d;
		}
		targetMates[closestTarget] = currentUnmatchedSource;
		sourceMates[currentUnmatchedSource] = closestTarget;
		numberMated++;
	}

	double value = 0;
	for (auto sourceInc : range(numSources))
	{
		value += cost(sourceInc, sourceMates[sourceInc]);
	}
	return value;
}


double NiawgController::rearrangement( Matrix<bool> & sourceMatrix, Matrix<bool> & targetMatrix,
									   std::vector<simpleMove>& moveSequence)
{
	// I am sure this might be also included directly after evaluating the image, but for safety I also included it 
	// here. 
	int numberTargets = 0;
	int numberSources = 0;
	std::string sourceStr = sourceMatrix.print( );
	std::string targStr = targetMatrix.print( );
	for (UINT rowInc = 0; rowInc < sourceMatrix.getRows(); rowInc++)
	{
		for (UINT colInc = 0; colInc < sourceMatrix.getCols(); colInc++)
		{
			if (targetMatrix(rowInc, colInc))
			{
				numberTargets++;
			}
			if ( sourceMatrix( rowInc, colInc ))
			{
				numberSources++;
			}
		}
	}
	// Throw, if  less atoms than targets!
	if (numberSources < numberTargets)
	{
		thrower( "Less atoms than targets!\nN source: " + str( numberSources ) + ", N target: " + str( numberTargets ) );
	}

	/// calculate cost matrix from Source and Targetmatrix
	// Cost matrix. Stores path length for each source atom to each target position
	Matrix<double> costMatrix( numberSources, numberSources, 0 );
	// Indices of atoms in initial config
	std::vector<std::vector<int> > sourceCoordinates( numberSources, std::vector<int>( 2, 0 ) );
	// Indices of atoms in final config
	std::vector<std::vector<int> > targetCoordinates( numberTargets, std::vector<int>( 2, 0 ) );
	// Find out the indice
	int sourceCounter = 0;
	int targetCounter = 0;

	for (UINT rowInc = 0; rowInc < sourceMatrix.getRows(); rowInc++)
	{
		for (UINT columnInc = 0; columnInc < sourceMatrix.getCols(); columnInc++)
		{
			if (sourceMatrix(rowInc, columnInc) == 1)
			{
				sourceCoordinates[sourceCounter][0] = rowInc;
				sourceCoordinates[sourceCounter][1] = columnInc;
				sourceCounter++;
			}
			if (targetMatrix( rowInc, columnInc ) == 1)
			{
				targetCoordinates[targetCounter][0] = rowInc;
				targetCoordinates[targetCounter][1] = columnInc;
				targetCounter++;
			}
		}
	}

	// Now compute the pathlengths
	for (int sourceInc = 0; sourceInc < numberSources; sourceInc++)
	{
		for (int targetInc = 0; targetInc < numberTargets; targetInc++)
		{
			costMatrix(sourceInc, targetInc) = abs( sourceCoordinates[sourceInc][0] - targetCoordinates[targetInc][0] )
						+ abs( sourceCoordinates[sourceInc][1] - targetCoordinates[targetInc][1] );
		}
	}

	/// Use MinCostMatching algorithm
	//input for bipartite matching algorithm, Algorithm writes into these vectors
	std::vector<int> left;
	std::vector<int> right;

	//The returned cost is the travelled distance
	double cost = minCostMatching( costMatrix, left, right );

	/// calculate the moveSequence

	//First resize moveSequence, empty in code, but now we now how many entrys: cost!
	std::vector<simpleMove> operationsList;
	operationsList.resize( cost, { 0,0,0,0 } );

	std::vector<std::vector<int> > matching( numberTargets, std::vector<int>( 4, 0 ) );
	
	// matching matrix, numberTargets x 4, Source and Target indice in each row
	for (int targetInc = 0; targetInc < numberTargets; targetInc++)
	{
		matching[targetInc][0] = sourceCoordinates[right[targetInc]][0];
		matching[targetInc][1] = sourceCoordinates[right[targetInc]][1];
		matching[targetInc][2] = targetCoordinates[targetInc][0];
		matching[targetInc][3] = targetCoordinates[targetInc][1];
	}

	int step_x, step_y, init_x, init_y;
	int counter = 0;

	// Setting up the moveSequence (only elementary steps) from the matching matrix (source - target)
	for (int targetInc = 0; targetInc < numberTargets; targetInc++)
	{
		step_x = matching[targetInc][2] - matching[targetInc][0];
		step_y = matching[targetInc][3] - matching[targetInc][1];
		init_x = matching[targetInc][0];
		init_y = matching[targetInc][1];
		for (int xStepInc = 0; xStepInc < abs( step_x ); xStepInc++)
		{
			operationsList[counter].initRow = init_x;
			operationsList[counter].initCol = init_y;
			operationsList[counter].finRow = init_x + sign( step_x );
			operationsList[counter].finCol = init_y;
			init_x = init_x + sign( step_x );
			counter++;
		}
		for (int yStepInc = 0; yStepInc < abs( step_y ); yStepInc++)
		{
			operationsList[counter].initRow = init_x;
			operationsList[counter].initCol = init_y;
			operationsList[counter].finRow = init_x;
			operationsList[counter].finCol = init_y + sign( step_y );
			init_y = init_y + sign( step_y );
			counter++;
		}
	}
	/// now order the operations.
	// this part was written by Mark Brown. The other stuff in the rearrangment handling was written by Kai Niklas.
	// this clear should be unnecessary.
	moveSequence.clear( );
	// systemState keeps track of the state of the system after each move. It's important so that the algorithm can
	// avoid making atoms overlap.
	Matrix<bool> systemState = sourceMatrix;
	UINT moveNum = 0;
	while ( operationsList.size( ) != 0 )
	{
		if ( moveNum >= operationsList.size( ) )
		{
			// it's reached the end, reset this.
			moveNum = 0;
		}
		// make sure that the initial location IS populated and the final location ISN'T.
		if ( systemState(operationsList[moveNum].initRow, operationsList[moveNum].initCol) == false
			 || systemState(operationsList[moveNum].finRow, operationsList[moveNum].finCol) == true )
		{
			moveNum++;
			continue;
		}
		// else it's okay. add this to the list of moves.
		moveSequence.push_back( operationsList[moveNum] );
		// update the system state after this move.
		systemState(operationsList[moveNum].initRow, operationsList[moveNum].initCol) = false;
		systemState(operationsList[moveNum].finRow, operationsList[moveNum].finCol) = true;
		// remove the move from the list of moves.
		operationsList.erase( operationsList.begin( ) + moveNum );
	}
	// at this point operationsList should be zero size and moveSequence should be full of the moves in a sequence that
	// works. return the travelled distance.
	return cost;
} 


void NiawgController::writeToFile( std::vector<double> waveVals )
{
	std::ofstream file( DEBUG_OUTPUT_LOCATION + "Wave_" + str( writeToFileNumber++ ) + ".txt" );
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


// for visualization purposes. note that the returned vector will be one longer than the number of moves because it
// includes the original image.
std::vector<std::string> NiawgController::evolveSource( Matrix<bool> source, std::vector<complexMove> flashMoves )
{
	std::vector<std::string> images;
	images.push_back( source.print( ) );
	for ( auto move : flashMoves )
	{
		for ( auto loc : move.whichAtoms )
		{
			UINT initRow, initCol, finRow, finCol;
			initRow = move.rowOrColumn == "row" ? move.whichRowOrColumn : loc;
			initCol = move.rowOrColumn == "row" ? loc : move.whichRowOrColumn;
			finRow = initRow + (move.rowOrColumn == "row") * move.direction;
			finCol = initCol + !(move.rowOrColumn == "row") * move.direction;
			if ( !source( initRow, initCol ) )
			{
				throw;
			}
			// potentially could move a blank...
			source( finRow, finCol ) = source( initRow, initCol );
			source( initRow, initCol ) = false;
		}
		images.push_back( source.print( ) );
	}
	return images;
}


void NiawgController::optimizeMoves( std::vector<simpleMove> singleMoves, Matrix<bool> origSource, 
									 std::vector<complexMove> &flashMoves, rerngOptions options )
{
	if ( options.parallel == parallelMoveOption::none && options.noFlashOption == nonFlashingOption::none )
	{
		// no optimizations.
		return;
	}
	Matrix<bool> runningSource = origSource;
	while ( singleMoves.size( ) != 0 )
	{
		std::vector<std::vector<int>> dimLoops = { range( (int)origSource.getRows( ) ), range( (int)origSource.getRows( ), 0, -1 ),
			range( (int)origSource.getCols( ) ), range( (int)origSource.getCols( ), 0, -1 ) };
		std::vector<UINT> altSize = { origSource.getCols( ), origSource.getCols( ), origSource.getRows( ), origSource.getRows( ) };
		std::vector<std::string> directions = { "row", "row", "column", "column" };
		std::vector<int> offsets = { 1, -1, 1, -1 };
		for ( auto dim : range( dimLoops.size( ) ) )
		{
			for ( auto dimInc : dimLoops[dim] )
			{
				std::vector<int> moveIndexes;
				std::vector<simpleMove> moveList;
				for ( auto moveInc : range( singleMoves.size( ) ) )
				{
					if ( (options.parallel == parallelMoveOption::partial && moveList.size( ) == PARTIAL_PARALLEL_LIMIT)
						 || (options.parallel == parallelMoveOption::none && moveList.size( ) == 1) )
					{
						// already have all the moves we want for combining.
						break;
					}
					simpleMove& move = singleMoves[moveInc];
					int init = directions[dim] == "row" ? move.initRow : move.initCol;
					int fin = directions[dim] == "row" ? move.finRow : move.finCol;
					if ( init == dimInc && fin == dimInc + offsets[dim] )
					{
						// avoid repeats by checking iff singleMoves is in moveList first
						if ( std::find( moveList.begin( ), moveList.end( ), move ) == moveList.end( ) )
						{
							moveIndexes.push_back( moveInc );
							moveList.push_back( move );
						}
					}
				}
				if ( moveIndexes.size( ) == 0 )
				{
					// no moves in this row in this direction.
					continue;
				}
				// From the moves that go from dim to dim+offset, get which have atom at initial position and have no 
				// atom at the final position
				for ( unsigned k = moveIndexes.size( ); k-- > 0; )
				{
					auto& move = singleMoves[moveIndexes[k]];
					// check that initial spot has atom & final spot is free
					if ( !(runningSource( move.initRow, move.initCol ) && !runningSource( move.finRow, move.finCol )) )
					{
						// can't move this one, remove from list.
						moveIndexes.erase( moveIndexes.begin( ) + k );
						moveList.erase( moveList.begin( ) + k );
					}
				}
				if ( moveList.size( ) == 0 )
				{
					// couldn't move any atoms.
					continue;
				}
				flashMoves.push_back( complexMove( directions[dim], dimInc, offsets[dim] ) );
				/// create complex move objs
				Matrix<bool> tmpSource = runningSource;
				for ( auto indexNumber : range( moveIndexes.size( ) ) )
				{
					// offset from moveIndexes is the # of moves already erased.
					UINT moveIndex = moveIndexes[indexNumber] - indexNumber;
					auto& move = singleMoves[moveIndex];
					flashMoves.back( ).whichAtoms.push_back( directions[dim] == "row" ? move.initCol : move.initRow );
					// update source image with new configuration.
					tmpSource( move.initRow, move.initCol ) = false;
					tmpSource( move.finRow, move.finCol ) = true;
					singleMoves.erase( singleMoves.begin( ) + moveIndex );
				}
				flashMoves.back( ).needsFlash = false;
				/// determine if flashing is needed for this move.
				// loop through all locations in the row/collumn
				for ( auto location : range( altSize[dim] ) )
				{
					UINT initRow, initCol, finRow, finCol, which;
					bool isRow = directions[dim] == "row";
					initRow = isRow ? dimInc : location;
					initCol = isRow ? location : dimInc;
					finRow = initRow + isRow*offsets[dim];
					finCol = initCol + (!isRow)*offsets[dim];
					// if atom in location and location not being moved, always need to flash to not move this atom.
					if ( runningSource( initRow, initCol ) && std::find( flashMoves.back( ).whichAtoms.begin( ),
																		 flashMoves.back( ).whichAtoms.end( ), location )
						 == flashMoves.back( ).whichAtoms.end( ) )
					{
						flashMoves.back( ).needsFlash = true;
					}
					// if being cautious...
					if ( runningSource( finRow, finCol ) )
					{
						flashMoves.back( ).needsFlash = true;
					}
				}
				//
				runningSource = tmpSource;
			}
		}
	}
}

// Finds out the maximum number of moves, by only knowing the Target Matrix configuration
// I added together the furthest distances from each target.
// Therefore it assumes there is no atom on each target
// This is really overestimating the number of moves, but it is a maximum
// Is overestimating the most if you have a very small target in a big lattice.
// If you wanted to scale it down, one idea might be to scale getMaxMoves with the filling fraction!
// Also: Not super fast because of nested for loops
UINT NiawgController::getMaxMoves( Matrix<bool> targetmatrix )
{
	int targetNumber = 0;
	for (auto elem : targetmatrix)
	{
		if (elem == 1)
		{
			targetNumber++;
		}
	}
	std::vector<std::vector<UINT> >targetIndice( targetNumber, std::vector<UINT>( 2, 0 ) );
	UINT targetcounter = 0;
	for (auto rowInc : range(targetmatrix.getRows()))
	{
		for (auto colInc : range(targetmatrix.getCols()))
		{
			if (targetmatrix(rowInc, colInc) == 1)
			{
				targetIndice[targetcounter] = { rowInc, colInc }; 
				targetcounter++;
			}
		}
	}
	UINT maxlength = 0, sumlength = 0, length = 0;
	for (auto k : range(targetcounter))
	{
		for (auto i : range( targetmatrix.getRows()))
		{
			for (auto j : range( targetmatrix.getCols() ) )
			{
				length = abs( int(i - targetIndice[k][0]) ) + abs( int(j - targetIndice[k][1]) );
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

