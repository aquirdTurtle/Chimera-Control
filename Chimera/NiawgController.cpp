#include "stdafx.h"

#include "NiawgController.h"
#include "NiawgStructures.h"
#include "MasterManager.h"
#include "NiawgWaiter.h"

#include "MasterThreadInput.h"
#include "Matrix.h"
#include "Thrower.h"
#include <boost/algorithm/string/replace.hpp>
#include <chrono>
#include <numeric>
#include "externals.h"
#include "Thrower.h"
#include "miscCommonFunctions.h"
#include "range.h"
#include "Queues.h"
#include <algorithm>
#include <random>

NiawgController::NiawgController( UINT trigRow, UINT trigNumber, bool safemode ) : 
	triggerRow( trigRow ), triggerNumber( trigNumber ), fgenConduit(safemode)
{
	// Contains all of of the names of the files that hold actual data file names.	
	for ( auto number : range( MAX_NIAWG_SIGNALS ) )
	{
		WAVEFORM_NAME_FILES[number] = "gen " + str( number + 1 ) + ", const waveform file names.txt";
		WAVEFORM_NAME_FILES[number + MAX_NIAWG_SIGNALS] = "gen " + str( number + 1 )
			+ ", amp ramp waveform file names.txt";
		WAVEFORM_NAME_FILES[number + 2 * MAX_NIAWG_SIGNALS] = "gen " + str( number + 1 )
			+ ", freq ramp waveform file names.txt";
		WAVEFORM_NAME_FILES[number + 3 * MAX_NIAWG_SIGNALS] = "gen " + str( number + 1 )
			+ ", freq & amp ramp waveform file names.txt";

		WAVEFORM_TYPE_FOLDERS[number] = "gen" + str( number + 1 ) + "const\\";
		WAVEFORM_TYPE_FOLDERS[number + MAX_NIAWG_SIGNALS] = "gen" + str( number + 1 ) + "ampramp\\";
		WAVEFORM_TYPE_FOLDERS[number + 2 * MAX_NIAWG_SIGNALS] = "gen" + str( number + 1 ) + "freqramp\\";
		WAVEFORM_TYPE_FOLDERS[number + 3 * MAX_NIAWG_SIGNALS] = "gen" + str( number + 1 ) + "ampfreqramp\\";
	}

	// initialize rearrangement calibrations.
	// default value for bias calibrations is currently 0.5.
	// 3x6 calibration
	rerngContainer<double> moveBias3x6Cal( 3, 6, 0.45);
	
	moveBias3x6Cal( 1, 4, dir::right ) = 0.38;

	moveBias3x6Cal( 0, 0, dir::up ) = moveBias3x6Cal( 1, 0, dir::down ) = 0.7;
	moveBias3x6Cal( 0, 1, dir::up ) = moveBias3x6Cal( 1, 1, dir::down ) = 0.52;
	moveBias3x6Cal( 0, 2, dir::up ) = moveBias3x6Cal( 1, 2, dir::down ) = 0.48;
	moveBias3x6Cal( 0, 3, dir::up ) = moveBias3x6Cal( 1, 3, dir::down ) = 0.45;
	moveBias3x6Cal( 0, 4, dir::up ) = moveBias3x6Cal( 1, 4, dir::down ) = 0.48;
	moveBias3x6Cal( 0, 5, dir::up ) = moveBias3x6Cal( 1, 5, dir::down ) = 0.75;
	// 
	moveBias3x6Cal( 2, 0, dir::down ) = moveBias3x6Cal( 1, 0, dir::up ) = 0.8;
	moveBias3x6Cal( 2, 1, dir::down ) = moveBias3x6Cal( 1, 1, dir::up ) = 0.5;
	moveBias3x6Cal( 2, 2, dir::down ) = moveBias3x6Cal( 1, 2, dir::up ) = 0.45;
	moveBias3x6Cal( 2, 3, dir::down ) = moveBias3x6Cal( 1, 3, dir::up ) = 0.48;
	moveBias3x6Cal( 2, 4, dir::down ) = moveBias3x6Cal( 1, 4, dir::up ) = 0.35;
	moveBias3x6Cal( 2, 5, dir::down ) = moveBias3x6Cal( 1, 5, dir::up ) = 0.7;
	moveBiasCalibrations.push_back( moveBias3x6Cal );

	rerngContainer<double> moveBias10x10Cal( 10, 10, 0.1 );
	moveBias10x10Cal( 5, 3, dir::right ) = 0.125;
	moveBias10x10Cal( 5, 5, dir::left ) = 0.0975;

	moveBiasCalibrations.push_back( moveBias10x10Cal );
}


void NiawgController::initialize( )
{
	// open up the files and check what I have stored.
	openWaveformFiles( );
	/// Initialize the waveform generator via FGEN.
	// initializes the session handle.
	fgenConduit.init( NI_5451_LOCATION, VI_TRUE, VI_TRUE );
	// tells the niaw where I'm outputting.
	fgenConduit.configureChannels( );
	// Set output mode of the device to scripting mode (defined in constants.h)
	fgenConduit.configureOutputMode( );
	// configure marker event. This is set to output on PFI1, a port on the front of the card.
	fgenConduit.configureMarker( "Marker0", "PFI1" );
	// enable flatness correction. This allows there to be a bit less frequency dependence on the power outputted by 
	// the waveform generator.
	fgenConduit.setViBooleanAttribute( NIFGEN_ATTR_FLATNESS_CORRECTION_ENABLED, VI_TRUE );
	// configure the trigger. Trigger mode doesn't need to be set because I'm using scripting mode.
	fgenConduit.configureDigtalEdgeScriptTrigger( );
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
	for ( UINT startRowInc =0; startRowInc < result.getRows(); startRowInc++ )
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
				targetCoords[Axes::Horizontal] = startColInc;
				targetCoords[Axes::Vertical] = startRowInc;
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
									std::vector<ViChar>& userScriptSubmit, rerngGuiOptionsForm& rerngGuiForm, 
									rerngGuiOptions& rerngGui )
{
	input->comm->sendColorBox( System::Niawg, 'Y' );
	input->niawg->handleVariations( output, input->variables, variation, variedMixedSize, warnings, input->debugOptions,
									totalVariations, rerngGuiForm, rerngGui );
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
	input->comm->sendColorBox( System::Niawg, 'G' );
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

void NiawgController::initForExperiment ( )
{
	triggersInScript = 0;
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
				thrower( "ERROR: Multiple rearrangement waveforms found, but not allowed! Only one rearrangement is "
						 "currently possible per repetition." );
			}
			foundRearrangement = true;
			// start rearrangement thread. Give the thread the queue.
			input->niawg->startRerngThread( input->atomQueueForRearrangement, wave, input->comm, input->rearrangerLock,
											input->andorsImageTimes, input->grabTimes, 
											input->conditionVariableForRerng, input->rerngGui,
											input->analysisGrid );
		}
	}
	if ( input->rerngGui.active && !foundRearrangement )
	{
		thrower( "ERROR: system is primed for rearranging atoms, but no rearrangement waveform was found!" );
	}
	else if ( !input->rerngGui.active && foundRearrangement )
	{
		thrower( "ERROR: System was not primed for rearranging atoms, but a rearrangement waveform was found!" );
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
	std::fstream configFile( str( DEFAULT_SCRIPT_FOLDER_PATH ) + "DEFAULT_SCRIPT.nScript" );
	// check errors
	if ( !configFile.is_open( ) )
	{
		thrower( "FATAL ERROR: Couldn't open default niawg file!" );
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
		ScriptStream script;
		script << configFile.rdbuf( );
		rerngGuiOptionsForm rInfoDummy;
		rInfoDummy.moveSpeed.expressionStr = str(0.00006);
		analyzeNiawgScript( script, output, profile, debug, warnings, rInfoDummy, std::vector<parameterType>() );
		writeStaticNiawg( output, debug, std::vector<parameterType>( ) );
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
									NiawgOutput& output, Communicator* comm, bool dontGenerate )
{
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



void NiawgController::analyzeNiawgScript( ScriptStream& script, NiawgOutput& output, profileSettings profile, 
										  debugInfo& options, std::string& warnings, rerngGuiOptionsForm rerngGuiInfo, 
										  std::vector<parameterType>& variables )
{
	/// Preparation
	output.niawgLanguageScript = "";
	currentScript = script.str( );
	script.clear();
	script.seekg( 0, std::ios::beg );
	std::string command;
	// get the first command
	script >> command;
	/// Analyze!
	while ( script.peek( ) != EOF )
	{
		if ( MasterManager::handleVariableDeclaration ( command, script, variables, "niawg", warnings ) )
		{}
		else if ( isLogic( command ) )
		{
			handleLogic( script, command, output.niawgLanguageScript );
		}
		else if ( isSpecialCommand( command ) )
		{
			handleSpecial( script, output, command, profile, options, warnings );
		}
		else if ( isStandardWaveform( command ) )
		{
			handleStandardWaveform( output, command, script, variables );
		}
		else if ( isSpecialWaveform( command ) )
		{
			handleSpecialWaveform( output, profile, command, script, options, rerngGuiInfo, variables );
		}
		else
		{
			thrower( "ERROR: Input niawg command is unrecognized!\nMust be logic commands, generate commands, or "
					 "special commands. See documentation on the correct format for these commands.\n\n"
					 "The inputted command is: \"" + command + "\" for waveform #" 
					 + str( output.waveFormInfo.size( ) - 1 ) + "!" );
		}
		script >> command;
	}
	output.waves.resize( output.waveFormInfo.size( ) );
}



void NiawgController::writeStaticNiawg( NiawgOutput& output, debugInfo& options, std::vector<parameterType>& constants,
										bool deleteWaveAfterWrite)
{
	for ( auto waveInc : range( output.waveFormInfo.size()) )
	{
		waveInfoForm& waveForm( output.waveFormInfo[ waveInc ] );
		waveInfo& wave( output.waves[waveInc] );
		waveInfo prevWave;
		if ( waveInc != 0 )
		{
			prevWave = output.waves[waveInc - 1];
		}
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
				rerngScriptInfoFormToOutput( waveForm, wave, constants, 0 );
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
				handleMinus1Phase( wave.core, prevWave.core );
				writeStandardWave( wave.core, options, output.isDefault );
				if ( deleteWaveAfterWrite )
				{
					deleteWaveData( wave.core );
				}
			}
		}
	}
}


void NiawgController::deleteWaveData( simpleWave& core )
{
	core.waveVals.clear( );
	core.waveVals.shrink_to_fit( );
}


void NiawgController::handleMinus1Phase( simpleWave& waveCore, simpleWave prevWave )
{
	for ( auto chanInc : range( waveCore.chan.size()) )
	{
		for ( auto sigInc : range( waveCore.chan[chanInc].signals.size()) )
		{
			auto& sig = waveCore.chan[chanInc].signals[sigInc];
			if (sig.initPhase == -1 )
			{
				sig.initPhase = prevWave.chan[chanInc].signals[sigInc].finPhase;
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
										  std::vector<parameterType>& varibles, UINT variation )
{
	try
	{
		wave.varies = formWave.varies;
		wave.time = formWave.time.evaluate( varibles, variation ) * 1e-3;
		wave.sampleNum = waveformSizeCalc( wave.time );
		wave.name = formWave.name;
		for ( auto chanInc : range( wave.chan.size()) )
		{
			wave.chan[chanInc].delim = formWave.chan[chanInc].delim;
			wave.chan[chanInc].initType = formWave.chan[chanInc].initType;
			wave.chan[chanInc].phaseOption = formWave.chan[chanInc].phaseOption;
			wave.chan[chanInc].signals.resize( formWave.chan[chanInc].signals.size( ) );
			for ( auto signalInc : range( wave.chan[chanInc].signals.size()))
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
}


void NiawgController::handleSpecialWaveform( NiawgOutput& output, profileSettings profile, std::string cmd,
													   ScriptStream& script, debugInfo& options, rerngGuiOptionsForm rerngGuiInfo,
													   std::vector<parameterType>& variables )
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
		std::string scope = NO_PARAMETER_SCOPE;
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
			flashingWave.flash.flashCycleFreq.assertValid( variables, scope );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: flashing number failed to convert to an integer! This parameter cannot be varied." );
		}
		script >> flashingWave.core.time;
		script >> flashingWave.flash.deadTime;
		flashingWave.flash.deadTime.assertValid( variables, scope );
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
		target coordinates (row col)
		}
		*/
		waveInfoForm rearrangeWave;
		rearrangeWave.rearrange.timePerMove = rerngGuiInfo.moveSpeed.expressionStr;
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
		rearrangeWave.rearrange.lowestFreqs[Axes::Horizontal] = std::stod( tempStr );
		script >> tempStr;
		rearrangeWave.rearrange.lowestFreqs[Axes::Vertical] = std::stod( tempStr );
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
		rearrangeWave.rearrange.moveLimit = 10;//getMaxMoves( rearrangeWave.rearrange.target );
		rearrangeWave.rearrange.fillerWave = rearrangeWave.rearrange.staticWave;
		// filler move gets the full time of the move. Need to convert the time per move to ms instead of s.
		rearrangeWave.rearrange.fillerWave.time = str( (rearrangeWave.rearrange.moveLimit
														 * rearrangeWave.rearrange.timePerMove.evaluate( )
														 + 2 * rerngGuiInfo.finalMoveTime.evaluate() / 1e3) * 1e3 );
		for ( auto ax : AXES )
		{
			for ( auto sig : rearrangeWave.rearrange.fillerWave.chan[ax].signals )
			{
				rearrangeWave.rearrange.staticBiases[ax].push_back( sig.initPower.evaluate( ) );
				rearrangeWave.rearrange.staticPhases[ax].push_back( sig.initPhase.evaluate( ) );
			}
		}
		output.waveFormInfo.push_back( rearrangeWave );
		long samples = long( (output.waveFormInfo.back( ).rearrange.moveLimit
							   * output.waveFormInfo.back( ).rearrange.timePerMove.evaluate( ) 
							   + 2 * rerngGuiInfo.finalMoveTime.evaluate() / 1e3) * NIAWG_SAMPLE_RATE );
		fgenConduit.allocateNamedWaveform( cstr( rerngWaveName ), samples );
		output.niawgLanguageScript += "generate " + rerngWaveName + "\n";
	}
	else
	{
		thrower( "ERROR: Bad special waveform command! command was \"" + cmd + "\"." );
	}
}


void NiawgController::handleVariations( NiawgOutput& output, std::vector<std::vector<parameterType>>& variables, 
										UINT variation, std::vector<long>& mixedWaveSizes, std::string& warnings, 
										debugInfo& debugOptions, UINT totalVariations, rerngGuiOptionsForm& rerngGuiForm,
										rerngGuiOptions& rerngGui )
{
	rerngGuiOptionsFormToFinal( rerngGuiForm, rerngGui, variables[0], variation );
	int mixedCount = 0;
	// I think waveInc = 0 & 1 are always the default.. should I be handling that at all? shouldn't make a difference 
	// I don't think. 
	/// Why is the seqinc here a for loop??? I think this should be an input to the function...
	for ( auto seqInc : range( variables.size( ) ) )
	{
		for ( auto waveInc : range( output.waveFormInfo.size( ) ) )
		{
			waveInfo& wave = output.waves[waveInc];
			waveInfoForm& waveForm = output.waveFormInfo[waveInc];
			waveInfo prevWave;
			if ( waveInc != 0 )
			{
				prevWave = output.waves[waveInc - 1];
			}
			if ( waveForm.core.varies )
			{
				if ( waveForm.flash.isFlashing )
				{
					flashFormToOutput( waveForm, wave, variables[seqInc], variation );
					writeFlashing( wave, debugOptions, variation );
				}
				else if ( waveForm.rearrange.isRearrangement )
				{
					rerngScriptInfoFormToOutput( waveForm, wave, variables[seqInc], variation );
				}
				else
				{
					simpleFormToOutput( waveForm.core, wave.core, variables[seqInc], variation );
					if ( variation != 0 )
					{
						fgenConduit.deleteWaveform( cstr( wave.core.name ) );
					}
					handleMinus1Phase( wave.core, prevWave.core );
					writeStandardWave( wave.core, debugOptions, output.isDefault );
					deleteWaveData( wave.core );
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
		thrower( "ERROR: waveform input type not found while loading standard input type?!?!? "
				 " (A low level bug, this shouldn't happen)\r\n" );
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
				thrower( "ERROR: Error Creating directory for waveform library system. Error was windows error " 
						 + str( GetLastError()) + ", Path was " + folderPath);
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
				thrower( "ERROR: waveform library file did not open correctly. Name was " + libNameFilePath
						 + " (A low level bug, this shouldn't happen)" );
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
void NiawgController::generateWaveform( channelWave & chanWave, debugInfo& options, long int sampleNum, double waveTime,
										bool powerCap, bool constPower)
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
	waveformFileSpecs += str( waveTime * 1000.0 ) + "; ";
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
		// shouldn't happen.
		thrower( "ERROR: NIAWG Waveform Storage File could not open. Shouldn't happen. File name is too long? "
				 "File name is " + waveformFileName + ", which is " + str( waveformFileName.size( ) ) 
				 + " characters long." );
	}
	else
	{
		// start timer.
		std::chrono::time_point<chronoClock> time1( chronoClock::now( ) );
		// calculate all voltage values and final phases and store them in the readData variable.
		std::vector<ViReal64> readData( sampleNum + chanWave.signals.size( ) );
		calcWaveData( chanWave, readData, sampleNum, waveTime, powerCap, constPower );
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
			thrower( "ERROR! saved waveform file not opening correctly! File name was " + LIB_PATH 
					 + WAVEFORM_TYPE_FOLDERS[chanWave.initType] + WAVEFORM_NAME_FILES[chanWave.initType] + ".\n" );
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

void NiawgController::handleLogic( ScriptStream& script, std::string cmd, std::string &scriptString )
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
			thrower( "ERROR: Sample number inside NIAWAG wait command wasn't an integer! Value was " + temp );
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
			thrower( "ERROR: NIAWG repeat number was not an integer! value was " + temp );
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


void NiawgController::handleSpecial( ScriptStream& script, NiawgOutput& output, std::string cmd, 
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
		thrower( "ERROR: special command not recognized! Command was \"" + cmd + "\"." );
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
void NiawgController::calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, 
									double waveTime, bool powerCap, bool constPower )
{
	// Declarations
	std::vector<double> powerPos, freqRampPos, phasePos( inputData.signals.size( ) );
	std::vector<double*> powerRampFileData, freqRampFileData;
	std::fstream powerRampFile, freqRampFile;
	std::string tempStr;

	/// deal with ramp calibration files. check all signals for files and read if yes.
	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		if ( inputData.signals[signal].initPhase < 0 )
		{
			thrower( "ERROR: initial phase of waveform was negative! This shouldn't happen. At this point, if using -1,"
					 "phase from prev waveform should have been grabbed already." );
		}
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
					thrower( "ERROR: niawg ramp file not the correct size?\nSize of upload is " + str( powerValNumber )
							 + "; size of file is " + str( sampleNum ) );
				}
				// close the file.
				powerRampFile.close( );
			}
			else
			{
				thrower( "ERROR: niawg ramp type " + str( inputData.signals[signal].powerRampType ) + " is unrecognized. If "
						 "this is a file name, make sure the file exists and is in the project folder. " );
			}
		}
		// If the ramp type isn't a standard command...
		if ( inputData.signals[signal].freqRampType != "lin" && inputData.signals[signal].freqRampType != "nr"
			 && inputData.signals[signal].freqRampType != "tanh" && inputData.signals[signal].freqRampType != "fast" )
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
					thrower( "ERROR: niawg freq ramp file not the correct size?\nSize of upload is " + str( freqRampValNum )
							 + "; size of file is " + str( sampleNum ) );
				}
				// close file
				freqRampFile.close( );
			}
			else
			{
				thrower( "ERROR: niawg freq ramp type " + inputData.signals[signal].freqRampType + " is unrecognized. "
						 "If this is a file name, make sure the file exists and is in the project folder." );
			}
		}
	}
	/// Pre-calculate a bunch of parameters for the ramps. 
	// These are not all used, but it's simple one-time calcs so I just od them anyways.
	auto& t_r = waveTime;
	auto t_r2 = t_r / 2;
	std::vector<double> deltaOmega;
	std::vector<double> deltaNu;
	std::vector<double> accel_w0;
	std::vector<double> accel_w1;
	std::vector<double> jerk;
	std::vector<double> freq_1;
	std::vector<double> phi_halfway;
	double deltaTanh = std::tanh( 4 ) - std::tanh( -4 );
	for ( UINT signal = 0; signal < inputData.signals.size( ); signal++ )
	{
		// I try to keep the "auto" aliases here consistent with what's used later in the calculation.
		auto f_0 = inputData.signals[signal].freqInit;
		auto dNu = (inputData.signals[signal].freqFin - f_0);
		deltaNu.push_back( dNu );
		auto dOmega = 2 * PI *  dNu;
		deltaOmega.push_back( dOmega );
		// initial phase acceleration, can modify this to give tweezer an initial velocity. 0 = start from static
		auto a_w0 = 0;//2 * PI * dNu / t_r;
		accel_w0.push_back( a_w0 );
		auto a_w1 = 4 * PI * dNu / t_r - a_w0;
		accel_w1.push_back( a_w1 );
		jerk.push_back( 8 * PI * dNu / (t_r*t_r) - 4 * a_w0 / t_r );
		freq_1.push_back( f_0 + dNu / 2);
		auto phi_0 = inputData.signals[signal].initPhase;
		phi_halfway.push_back( 0.5 * a_w0 * (t_r2*t_r2) + (t_r2 / 6.0) * (2 * PI * dNu - a_w0 * t_r)
						 + 2 * PI * f_0 * t_r2 + phi_0 );
	}
	///		Get Data Points.		///
	int sample = 0;
	/// increment through all samples
	for ( ; sample < sampleNum; sample++ )
	{
		// calculate the time that this sample number refers to
		double t = (double)sample / NIAWG_SAMPLE_RATE;
		/// Calculate Phase and Power Positions. For Every signal...
		for ( auto signal : range( inputData.signals.size( ) ) )
		{
			// these "auto" aliases should match what was used above to calculate constants.
			auto dOmega = deltaOmega[signal];
			auto phi_0 = inputData.signals[signal].initPhase;
			auto f_0 = inputData.signals[signal].freqInit;
			/// Handle Frequency Ramps
			// Frequency ramps are actually a little complex. we have dPhi/dt = omega(t) and we need phi to calculate data points. So in order to get 
			// the phase you need to integrate the omega(t) you want and modify the integration constant to get your initial phase.
			if ( inputData.signals[signal].freqRampType == "lin" )
			{
				// W{t} = Wi + (DeltaW * t) / (Tfin)
				// Phi{t}   = Wi * t + (DeltaW * t ^ 2) / 2 + phi_i
				phasePos[signal] = 2 * PI * f_0 * t + dOmega * pow( t, 2 ) / (2 * t_r) + phi_0;
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
				phasePos[signal] = (2 * PI * f_0 + dOmega / 2.0) * t + (dOmega / deltaTanh) * (t_r / 8.0) 
					* (std::log( std::cosh( 4 - (8 / t_r) * t ) ) - std::log( std::cosh( 4 ) )) + phi_0;
			}
			else if ( inputData.signals[signal].freqRampType == "nr" )
			{
				// omega{t} = omega
				// phi = omega*t
				phasePos[signal] = 2 * PI * f_0 * t + phi_0;
			}
			else if ( inputData.signals[signal].freqRampType == "fast" )
			{
				// these "auto" aliases should match what was used above to calculate constants.
				auto a_w0 = accel_w0[signal];
				auto a_w1 = accel_w1[signal];
				auto dNu = deltaNu[signal];
				auto J = jerk[signal];
				auto f_1 = freq_1[signal];
				auto phi_1 = phi_halfway[signal];
				// constant phase-jerk ramp, except (optionally) an initial phase acceleration. I have a jupyter 
				// notebook about this.
				if ( t < t_r2 )
				{
					phasePos[signal] = (1.0 / 6.0) * J * (t * t * t) + 0.5 * a_w0 * (t * t) + 2.0 * PI * f_0 * t + phi_0;
				}
				else
				{
					auto tp = t - t_r2;
					phasePos[signal] = -(1.0 / 6.0) * J * (tp*tp*tp) + 0.5 * a_w1 * (tp*tp) + 2.0 * PI * f_1 * tp + phi_1;
				}
			}
			else
			{
				// special ramp case. I'm not sure if this is actually useful. 
				// The frequency file would have to be designed very carefully.
				freqRampPos[signal] = freqRampFileData[signal][sample] * (deltaNu[signal]);
				phasePos[signal] = (ViReal64)sample * 2 * PI * (f_0 + freqRampPos[signal]) / NIAWG_SAMPLE_RATE
					+ phi_0;
			}

			/// handle amplitude ramps, which are much simpler.
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
		if ( constPower )
		{
			double currentPower = 0;
			// calculate the total current amplitude.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				currentPower += fabs( inputData.signals[signal].initPower + powerPos[signal] );
				/// modify here for frequency-dependent calibrations!
				/// need current frequency and calibration file.
			}

			// normalize each signal.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				// After this, a "currentPower" calculated the same above will always give TOTAL_POWER. 
				powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal])
					* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
			}
		}
		else if ( powerCap )
		{
			double currentPower = 0;
			// calculate the total current amplitude.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				currentPower += fabs( inputData.signals[signal].initPower + powerPos[signal] );
				/// modify here for frequency-dependent calibrations!
				/// need current frequency and calibration file.
			}
			// normalize each signal only if power is above TOTAL_POWER. hence the word "cap".
			if ( currentPower > TOTAL_POWER )
			{
				for ( auto signal : range( inputData.signals.size( ) ) )
				{
					// After this, a "currentPower" calculated the same above will always give TOTAL_POWER. 
					powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal])
						* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
				}
			}
		}

		///  finally, Calculate voltage data point.
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
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime 
				+ deltaOmega[signal] * pow( curTime, 2 ) * 1 / (2 * waveTime)
				+ inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "tanh" )
		{
			phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
				+ (deltaOmega[signal] / deltaTanh) * (waveTime / 8.0) * std::log( std::cosh( 4 - (8 / waveTime) * curTime ) )
				- (deltaOmega[signal] / deltaTanh) * (waveTime / 8.0) * std::log( std::cosh( 4 ) )
				+ inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "nr" )
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "fast" )
		{
			// these "auto" aliases should match what was used above to calculate constants.
			auto t = curTime;
			auto f_0 = inputData.signals[signal].freqInit;
			auto phi_0 = inputData.signals[signal].initPhase;
			auto a_w0 = accel_w0[signal];
			auto a_w1 = accel_w1[signal];
			auto dNu = deltaNu[signal];
			auto J = jerk[signal];
			auto f_1 = freq_1[signal];
			auto phi_1 = phi_halfway[signal];
			// constant phase-jerk ramp, except (optionally) an initial phase acceleration. I have a jupyter 
			// notebook about this.
			if ( t < t_r2 )
			{
				phasePos[signal] = (1.0 / 6.0) * J * (t*t*t) + 0.5 * a_w0 * (t * 2) + 2 * PI * f_0 * t + phi_0;
			}
			else
			{
				auto tp = t - t_r2;
				phasePos[signal] = -(1.0 / 6.0) * J * (tp*tp*tp) + 0.5 * a_w1 * (tp*tp) + 2.0 * PI * f_1 * tp + phi_1;
			}
		}
		else
		{
			freqRampPos[signal] = freqRampFileData[signal][sample] * (inputData.signals[signal].freqFin 
																	   - inputData.signals[signal].freqInit);
			phasePos[signal] = (ViReal64)sample * 2 * PI * (inputData.signals[signal].freqInit 
															 + freqRampPos[signal]) / (NIAWG_SAMPLE_RATE)
				+ inputData.signals[signal].initPhase;
		}
		// Don't need amplitude info.
	}

	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		// get the final phase of this waveform. Note that this is the phase of the /next/ data point (the last time 
		// signalInc gets incremented, the for loop doesn't run) so that if the next waveform starts at this data 
		// point, it will avoid repeating the same data point. This is used for the option where the user uses this 
		// phase as the starting phase of the next waveform.
		inputData.signals[signal].finPhase = fmod( phasePos[signal], 2 * PI );
		// catch the case in which the final phase is virtually identical to 2*PI, which isn't caught in the above 
		// line because of bad floating point arithmetic.
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
		waveCore.waveVals[2 * sample] = waveCore.chan[Axes::Vertical].wave[sample];
		waveCore.waveVals[2 * sample + 1] = waveCore.chan[Axes::Horizontal].wave[sample];
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
		wave.time = time;
	}
	catch ( Error& err )
	{
		thrower( "ERROR: niawg waveform time cannot be varied! Evaluation of time expression failed with error:\n"
				 + err.whatStr( ) );
	}
	std::string option;
	script >> option;
	int phaseOption;
	try
	{
		phaseOption = std::stoi( option );
		wave.chan[Axes::Horizontal].phaseOption = wave.chan[Axes::Vertical].phaseOption = phaseOption;
	}
	catch ( std::invalid_argument& )
	{
		thrower( "Error: niawg phase management option failed to convert to an integer." );
	}
}


void NiawgController::loadWaveformParametersFormSingle( NiawgOutput& output, std::string cmd, ScriptStream& script,
														std::vector<parameterType>& variables, int axis, 
														simpleWaveForm& wave )
{
	// Don't remember why I have this limitation built in.
	if ( output.isDefault && output.waveFormInfo.size( ) == 1 )
	{
		thrower( "ERROR: The default niawg waveform files contain sequences of waveforms. Right now, the default "
				 "waveforms must be a single waveform, not a sequence.\r\n" );
	}
	std::string scope = "niawg";
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
				sig.freqInit.assertValid( variables, scope );
				// set the initial and final values to be equal, and to not use a ramp, unless variable present.
				script >> sig.initPower;
				sig.initPower.assertValid( variables, scope );
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables, scope );
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
				sig.freqInit.assertValid( variables, scope );
				script >> sig.powerRampType;
				script >> sig.initPower;
				sig.initPower.assertValid( variables, scope );
				script >> sig.finPower;
				sig.finPower.assertValid( variables, scope );
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables, scope );
				sig.freqFin = sig.freqInit;
				sig.freqRampType = "nr";
				break;
			}
			/// The case for "gen ?, freq ramp"
			case 2:
			{
				script >> sig.freqRampType;
				script >> sig.freqInit;
				sig.freqInit.assertValid( variables, scope );
				script >> sig.freqFin;
				sig.freqFin.assertValid( variables, scope );
				script >> sig.initPower;
				sig.initPower.assertValid( variables, scope );
				sig.finPower = sig.initPower;
				sig.powerRampType = "nr";
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables, scope );
				break;
			}
			/// The case for "gen ?, freq & amp ramp"
			case 3:
			{
				script >> sig.freqRampType;
				script >> sig.freqInit;
				sig.freqInit.assertValid( variables, scope );
				script >> sig.freqFin;
				sig.freqFin.assertValid( variables, scope );
				script >> sig.powerRampType;
				script >> sig.initPower;
				sig.initPower.assertValid( variables, scope );
				script >> sig.finPower;
				sig.finPower.assertValid( variables, scope );
				script >> sig.initPhase;
				sig.initPhase.assertValid( variables, scope );
				break;
			}
		}
	}

	script >> wave.chan[axis].delim;
	// check delimiter
	if ( wave.chan[axis].delim != "#" )
	{
		thrower( "ERROR: The delimeter is missing in the " + AXES_NAMES[axis] + " waveform command for waveform #"
				 + str( output.waveFormInfo.size( ) - 1 ) + "The value placed in the delimeter location was "
				 + wave.chan[axis].delim + " while it should have been '#'. This indicates that either the code is "
				 "not interpreting the user input correctly or that the user has inputted too many parameters for this"
				 " type of waveform." );
	}
	//	Handle -1 Phase (start with the phase that the previous waveform ended with)
	UINT count = 0;
	// loop through all signals in a the current waveform for a given axis.
	for ( auto signal : wave.chan[axis].signals )
	{
		// If the user used a '-1' for the initial phase, this means the user wants to copy the ending phase of the 
		// previous waveform. Check to make sure this is valid at this point, will be evaluated later.
		if ( signal.initPhase.evaluate( ) == -1 )
		{
			if ( output.waveFormInfo.size( ) == 0 )
			{
				thrower( "ERROR: You are trying to copy the phase of the previous niawg waveform... in the /first/ "
						 "niawg waveform! Not possible!" );
			}
			UINT prevNum = output.waveFormInfo.size( ) - 1;
			UINT signalNum = output.waveFormInfo[prevNum].core.chan[axis].signals.size( );
			if ( count + 1 > signalNum )
			{
				thrower( "ERROR: In niawg command, You are trying to copy the phase of signal " + str( count + 1 ) + "  of " 
						 + AXES_NAMES[axis] + " waveform #" + str( prevNum ) + ", but the previous waveform only had " 
						 + str( signalNum ) + " signals!\n" );
			}
		}
		count++;
	}
}


void NiawgController::loadFullWave( NiawgOutput& output, std::string cmd, ScriptStream& script,
									std::vector<parameterType>& variables, simpleWaveForm& wave )
{
	int axis;
	std::string axisStr;
	script >> axisStr;
	if ( axisStr == "vertical" )
	{
		axis = Axes::Vertical;
	}
	else if ( axisStr == "horizontal" )
	{
		axis = Axes::Horizontal;
	}
	else
	{
		thrower( "ERROR: unrecognized niawg axis string: " + axisStr + " inside NIAWG script file! axis string must be one of"
				 "\"horizontal\" or \"vertical\"" );
	}
	loadWaveformParametersFormSingle( output, cmd, script, variables, axis, wave );
	// get cmd, axis of second waveform
	script >> cmd;
	if ( !isStandardWaveform( cmd ) )
	{
		thrower( "ERROR: standard niawg waveform must be followed by another standard waveform, for each of the horizontal and"
				 " vertical axes" );
	}
	std::string newAxisStr;
	script >> newAxisStr;
	if ( newAxisStr == axisStr )
	{
		thrower( "ERROR: horizontal niawg waveform must be followed by vertical or vertical must be followed by horizontal." );
	}

	if ( newAxisStr == "vertical" )
	{
		axis = Axes::Vertical;
	}
	else if ( newAxisStr == "horizontal" )
	{
		axis = Axes::Horizontal;
	}
	else
	{
		thrower( "ERROR: Expected either \"vertical\" or \"horizontal\" after waveform type declaration. Instead, found"
				 " \"" + newAxisStr + "\"." );
	}
	loadWaveformParametersFormSingle( output, cmd, script, variables, axis, wave );
	// get the common things.
	loadCommonWaveParams( script, wave );
}


void NiawgController::handleStandardWaveform( NiawgOutput& output, std::string cmd, ScriptStream& script, 
														std::vector<parameterType>& variables )
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


void NiawgController::flashFormToOutput( waveInfoForm& waveForm, waveInfo& wave, std::vector<parameterType>& variables, 
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


void NiawgController::rerngScriptInfoFormToOutput( waveInfoForm& waveForm, waveInfo& wave, 
											 std::vector<parameterType>& variables, UINT variation )
{
	wave.rearrange.isRearrangement = waveForm.rearrange.isRearrangement;
	wave.rearrange.freqPerPixel = waveForm.rearrange.freqPerPixel;
	wave.rearrange.lowestFreqs = waveForm.rearrange.lowestFreqs;
	wave.rearrange.moveLimit = waveForm.rearrange.moveLimit;
	wave.rearrange.target = waveForm.rearrange.target;
	wave.rearrange.finalPosition = waveForm.rearrange.finalPosition;
	wave.rearrange.timePerMove = waveForm.rearrange.timePerMove.evaluate(variables, variation);
	wave.rearrange.staticBiases = waveForm.rearrange.staticBiases;
	wave.rearrange.staticPhases = waveForm.rearrange.staticPhases;
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
void NiawgController::finalizeStandardWave( simpleWave& wave, debugInfo& options, bool powerCap, bool constPower )
{
	// prepare each channel
	generateWaveform( wave.chan[Axes::Horizontal], options, wave.sampleNum, wave.time, powerCap, constPower );
	generateWaveform( wave.chan[Axes::Vertical], options, wave.sampleNum, wave.time, powerCap, constPower );
	mixWaveforms( wave, options.outputNiawgWavesToText );
	// clear channel data, no longer needed.
	wave.chan[Axes::Vertical].wave.clear( );
	// not sure if shrink_to_fit is necessary, but might help with mem management
	wave.chan[Axes::Vertical].wave.shrink_to_fit( );
	wave.chan[Axes::Horizontal].wave.clear( );
	wave.chan[Axes::Horizontal].wave.shrink_to_fit( );
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
		thrower( "ERROR: tried to create niawg flashing wave data for a waveform that wasn't flashing!  "
				 "(A low level bug, this shouldn't happen)" );
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
	if ( wave.flash.flashNumber == 1 )
	{
		// this occurs when not actually flashing.
		wave.core.waveVals = wave.flash.flashWaves.front( ).waveVals;
		return;
	}
	/// then mix them to create the flashing version.
	// total period time in seconds...
	double period = 1.0 / wave.flash.flashCycleFreq;
	// total period in samples...
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
	if ( input->guiOptions.useCalibration )
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
			flashMove.staticMovingRatio = input->guiOptions.staticMovingRatio;
			flashMove.deadTime = input->guiOptions.deadTime;
			flashMove.moveTime = input->guiOptions.moveSpeed;
			flashMove.moveBias = input->guiOptions.moveBias;
			noFlashMove = flashMove;
			complexMove flashMoveInfo;
			flashMoveInfo.locationsToMove.resize( 1 );
			flashMoveInfo.needsFlash = true;
			complexMove noFlashMoveInfo( flashMoveInfo );
			noFlashMoveInfo.needsFlash = false;
			std::array<dir, 4> directions = { dir::up, dir::down, dir::left, dir::right };
			std::array<int, 4> offsets = { -1, 1, -1, 1 };
			std::array<std::string, 4> dirText = { "row", "row", "col", "col" };
			//std::array<UINT, 4> whichRowOrColumn = { row, row, col, col };
			std::array<UINT, 4> whichAtom = { col, col, row, row };
			std::array<bool, 4> conditions = { row != rows - 1, row != 0, col != 0, col != cols - 1 };

			// loop through each possible direction.
 			for ( auto inc : range( 4 ) )
			{
				noFlashMoveInfo.locationsToMove[0] = flashMoveInfo.locationsToMove[0] = { row, col };
				noFlashMove.waveVals = flashMove.waveVals = std::vector<double>( );
				noFlashMoveInfo.moveDir = noFlashMove.direction = flashMoveInfo.moveDir = flashMove.direction 
					= directions[inc];
				if ( conditions[inc] )
				{
					if ( input->guiOptions.useCalibration )
					{
						noFlashMove.moveBias = flashMove.moveBias = calBias( row, col, flashMove.direction );
					}
					if ( input->guiOptions.useFast )
					{
						noFlashMove.waveVals = makeFastRerngWave( input->rerngWave.rearrange, input->sourceRows,
																  input->sourceCols, noFlashMoveInfo, input->guiOptions,
																  noFlashMove.moveBias );
						flashMove.waveVals = makeFastRerngWave( input->rerngWave.rearrange, input->sourceRows,
																input->sourceCols, flashMoveInfo, input->guiOptions,
																flashMove.moveBias );
					}
					else if ( input->guiOptions.auxStatic )
					{
						noFlashMove.waveVals = makeRerngWaveMovePart ( input->rerngWave.rearrange, 
																	   noFlashMove.moveBias, input->sourceRows, 
																	   input->sourceCols, noFlashMoveInfo ).waveVals;
						flashMove.waveVals = makeRerngWaveMovePart ( input->rerngWave.rearrange, 
																	 flashMove.moveBias, input->sourceRows, 
																	 input->sourceCols, flashMoveInfo ).waveVals;
					}
					else
					{
						noFlashMove.waveVals = makeFullRerngWave( input->rerngWave.rearrange, noFlashMove.staticMovingRatio,
															  noFlashMove.moveBias, noFlashMove.deadTime, input->sourceRows,
															  input->sourceCols, noFlashMoveInfo );
						flashMove.waveVals = makeFullRerngWave( input->rerngWave.rearrange, flashMove.staticMovingRatio,
															flashMove.moveBias, flashMove.deadTime, input->sourceRows,
															input->sourceCols, flashMoveInfo );
					}
				}
				input->flashMoves( row, col, flashMove.direction ) = flashMove;
				input->noFlashMoves( row, col, noFlashMove.direction ) = noFlashMove;
			}
		}
	}
	input->flashMoves.setFilledFlag( );
}

/*
	Has not been updated with the off-grid dump functionality.
*/
std::vector<double> NiawgController::makeFastRerngWave( rerngScriptInfo& rerngSettings, UINT sourceRows, UINT sourceCols,
														complexMove moveInfo, rerngGuiOptions options, double moveBias )
{
	double freqPerPixel = rerngSettings.freqPerPixel;
	// starts from the top left.
	bool upOrDown = (moveInfo.moveDir == dir::down || moveInfo.moveDir == dir::up);
	UINT movingAxis = upOrDown ? Axes::Vertical : Axes::Horizontal;
	UINT staticAxis = !upOrDown ? Axes::Vertical : Axes::Horizontal;
	UINT movingSize = upOrDown ? sourceRows : sourceCols;
	auto targetCols = rerngSettings.target.getCols( );
	auto targetRows = rerngSettings.target.getRows( );
	auto& lowFreqs = rerngSettings.lowestFreqs;
	// make the move
	simpleWave moveWave;
	moveWave.varies = false;
	moveWave.name = "NOT-USED";
	// needs to match correctly the static waveform.
	moveWave.time = options.fastMoveTime;
	moveWave.sampleNum = waveformSizeCalc( moveWave.time );

	double movingFrac = moveBias;
	// split the remaining bias between all of the other movingSize-2 signals.
	double nonMovingFrac = (1 - movingFrac) / (movingSize - 2);
	/// get number of static traps in the moving axis
	std::vector<UINT> staticTweezers;
	// for every possible static tweezer
	for ( auto potentialStaticGridLoc : range( movingSize ) )
	{
		bool isUsed = false;
		for ( auto loc : moveInfo.locationsToMove )
		{
			// if location is init or final location of this location's move...
			if ( potentialStaticGridLoc == (upOrDown ? loc.row : loc.column) ||
				 potentialStaticGridLoc == (upOrDown ? loc.row + moveInfo.dirInt( ) : loc.column + moveInfo.dirInt( )) )
			{
				isUsed = true;
			}
		}
		if ( !isUsed )
		{
			// then should be a static tweezer
			staticTweezers.push_back( potentialStaticGridLoc );
		}
	}
	/// set parameters for the static tweezers in the moving axis.
	UINT signalNum = 0;

	for ( auto gridLoc : staticTweezers )
	{
		moveWave.chan[movingAxis].signals.push_back( waveSignal( ) );
		waveSignal& sig = moveWave.chan[movingAxis].signals[signalNum];
		// static
		sig.freqRampType = sig.powerRampType = "nr";
		sig.finPower = sig.initPower = nonMovingFrac;
		sig.initPhase = 0;
		sig.freqInit = (movingAxis == Axes::Vertical) ?
			((targetRows - gridLoc - 1) * freqPerPixel + lowFreqs[movingAxis]) * 1e6 :
			(gridLoc * freqPerPixel + lowFreqs[movingAxis]) * 1e6;
		sig.freqFin = sig.freqInit;
		signalNum++;
	}
	/// /// /// /// handle moving axis /////////////
	/// set parameters for the moving tweezers in the moving axis.
	bool piFound = false;
	for ( auto loc : moveInfo.locationsToMove )
	{
		if ( !moveInfo.isInlineParallel && piFound )
		{
			// if pi-parallel then only one ramp in this line so break.
			break;
		}
		else if ( !moveInfo.isInlineParallel )
		{
			piFound = true;
		}
		moveWave.chan[movingAxis].signals.push_back( waveSignal( ) );
		waveSignal& sig = moveWave.chan[movingAxis].signals[signalNum];
		sig.powerRampType = "nr";
		sig.initPhase = 0;
		sig.finPower = sig.initPower = movingFrac;
		// I found that linear ramps worked best for the ultra-fast moves
		sig.freqRampType = "lin";
		if ( movingAxis == Axes::Horizontal )
		{
			UINT init = loc.column;
			UINT fin = loc.column + moveInfo.dirInt( );
			// convert to Hz
			sig.freqInit = (init * freqPerPixel + lowFreqs[movingAxis]) * 1e6;
			sig.freqFin = (fin * freqPerPixel + lowFreqs[movingAxis]) * 1e6;
		}
		else
		{
			UINT init = loc.row;
			UINT fin = loc.row + moveInfo.dirInt( );
			sig.freqInit = ((targetRows - init - 1) * freqPerPixel + lowFreqs[movingAxis]) * 1e6;
			sig.freqFin =  ((targetRows - fin  - 1) * freqPerPixel + lowFreqs[movingAxis]) * 1e6;
		}
		signalNum++;
	}
	/// handle other axis
	// in this case, flash refers to flashing most of the tweezer off during themove. the semantics of when flash is needed are the same as the other case. 
	if ( moveInfo.needsFlash )
	{
		moveWave.chan[staticAxis].signals.resize( moveInfo.locationsToMove.size( ) );
		UINT sigCount = 0;
		for ( auto atom : moveInfo.locationsToMove )
		{
			waveSignal& sig = moveWave.chan[staticAxis].signals[sigCount];
			// equal power in all of these.
			sig.finPower = sig.initPower = 1;
			sig.freqRampType = sig.powerRampType = "nr";
			sig.initPhase = 0;
			sig.freqInit = (staticAxis == Axes::Vertical) ?
				((targetRows - atom.row - 1) * freqPerPixel + lowFreqs[staticAxis]) * 1e6 :
				(atom.row * freqPerPixel + lowFreqs[staticAxis])*1e6;
			sig.freqFin = sig.freqInit;
			sigCount++;
		}
	}
	else
	{
		// no flash, so static axis must span all 
		auto staticDim = (staticAxis == Axes::Horizontal) ? targetCols : targetRows;
		moveWave.chan[staticAxis].signals.resize( staticDim );
		UINT sigCount = 0;
		for ( auto& sig : moveWave.chan[staticAxis].signals )
		{
			sig.freqRampType = sig.powerRampType = "nr";
			sig.freqFin = sig.freqInit;
			sig.initPhase = 0;
			sig.freqInit = (staticAxis == Axes::Vertical) ?
				((targetRows - sigCount - 1) * freqPerPixel + lowFreqs[staticAxis]) * 1e6 :
				(sigCount * freqPerPixel + lowFreqs[staticAxis])*1e6;
			sig.freqFin = sig.freqInit;
			// even intensity
			sig.initPower = sig.finPower = 1;
			sigCount++;
		}
	}
	/// make ramp into move wave
	simpleWave initRampWave, finRampWave;
	if ( true )
	{
		initRampWave = moveWave;
		initRampWave.time = 1e-6;
		initRampWave.sampleNum = waveformSizeCalc( initRampWave.time );
		// make every signal not freq ramp and make all amplitude ramps.
		for ( auto& chan : initRampWave.chan )
		{
			// I want to ramp up to the proper TOTAL_POWER so that the transition after the move is correct.
			double setPower = 0;
			for ( auto sig : chan.signals )
			{
				setPower += sig.initPower;
			}
			double powerScale = TOTAL_POWER / setPower;
			for ( auto& sig : chan.signals )
			{
				sig.freqRampType = "nr";
				sig.powerRampType = "lin";
				sig.finPower = sig.initPower * powerScale;
				sig.initPower = 0.1 * sig.initPower * powerScale;
			}
		}
		/// make ramp out of move wave
		finRampWave = initRampWave;
		for ( auto& chan : finRampWave.chan )
		{
			for ( auto& sig : chan.signals )
			{
				auto fin = sig.finPower;
				sig.finPower = sig.initPower;
				sig.initPower = fin;
			}
		}
		finalizeStandardWave( initRampWave, debugInfo( ), true, false );
		finalizeStandardWave( finRampWave, debugInfo( ), true, false );
	}
	finalizeStandardWave( moveWave, debugInfo( ) );
	/// Combine waves. 4 parts.
	// if the amp ramp is turned off above (if (false)) then the ramp vectors are empty and inserting them causes no harm.
	auto waveVals = initRampWave.waveVals;
	waveVals.insert( waveVals.end( ), moveWave.waveVals.begin( ), moveWave.waveVals.end( ) );
	waveVals.insert( waveVals.end( ), finRampWave.waveVals.begin( ), finRampWave.waveVals.end( ) );
	waveVals.insert( waveVals.end( ), rerngSettings.staticWave.waveVals.begin( ), rerngSettings.staticWave.waveVals.end( ) );
	return waveVals;
}


simpleWave NiawgController::makeRerngWaveMovePart ( rerngScriptInfo& rerngSettings, double moveBias, UINT sourceRows,
													UINT sourceCols, complexMove moveInfo )
{ 
	double freqPerPixel = rerngSettings.freqPerPixel;

	auto targetCols = rerngSettings.target.getCols ( );
	auto targetRows = rerngSettings.target.getRows ( );
	auto& lowFreqs = rerngSettings.lowestFreqs;

	bool upOrDown = ( moveInfo.moveDir == dir::down || moveInfo.moveDir == dir::up );
	UINT movingAxis = upOrDown ? Axes::Vertical : Axes::Horizontal;
	UINT staticAxis = ( !upOrDown ) ? Axes::Vertical : Axes::Horizontal;
	UINT movingSize = upOrDown ? sourceRows : sourceCols;
	simpleWave moveWave;
	moveWave.varies = false;
	moveWave.name = "NOT-USED";
	// if flashing, this time + staticWave.time needs to be = rerngSettings.timePerMove
	moveWave.time = rerngSettings.timePerMove;
	moveWave.sampleNum = waveformSizeCalc ( moveWave.time );
	bool offGridDump = true;
	double nonMovingFrac = offGridDump ? 1 - moveBias : ( 1 - moveBias ) / ( movingSize - 2 );
	/// handle moving axis /////////////
	/// figure out where to put static "dump" traps in the moving axis
	std::vector<int> staticTweezers;
	if ( !offGridDump || !moveInfo.needsFlash )
	{
		// if not flashing then need to put excess power in the other tweezers to hold them during the move.
		// for every possible static tweezer
		for ( auto potentialStaticGridLoc : range ( movingSize ) )
		{
			bool isUsed = false;
			for ( auto loc : moveInfo.locationsToMove )
			{
				// if location is init or final location of this location's move...
				if ( potentialStaticGridLoc == ( upOrDown ? loc.row : loc.column ) ||
					 potentialStaticGridLoc == ( upOrDown ? loc.row + moveInfo.dirInt ( ) : loc.column + moveInfo.dirInt ( ) ) )
				{
					isUsed = true;
				}
			}
			if ( !isUsed )
			{
				// then should be a static tweezer
				staticTweezers.push_back ( potentialStaticGridLoc );
			}
		}
	}
	else
	{
		// put it off-axis on the low frequency side of things. 
		staticTweezers.push_back ( -1 );
	}
	/// make the static "dump" tweezers in the moving axis
	UINT signalNum = 0;
	for ( auto gridLoc : staticTweezers )
	{
		moveWave.chan[ movingAxis ].signals.push_back ( waveSignal ( ) );
		waveSignal& sig = moveWave.chan[ movingAxis ].signals[ signalNum ];
		// static
		sig.freqRampType = sig.powerRampType = "nr";
		auto staticPos = upOrDown ? rerngSettings.staticBiases[ movingAxis ].size ( ) - gridLoc - 1 : gridLoc;
		sig.finPower = sig.initPower = nonMovingFrac * ( gridLoc == -1 ? 1 : rerngSettings.staticBiases[ movingAxis ][ staticPos ] );
		sig.initPhase = ( gridLoc == -1 ? 0 : rerngSettings.staticPhases[ movingAxis ][ staticPos ] );
		sig.freqInit = ( upOrDown ) ? ( ( targetRows - gridLoc - 1 ) * freqPerPixel + lowFreqs[ movingAxis ] ) :
			( gridLoc * freqPerPixel + lowFreqs[ movingAxis ] );
		sig.freqInit *= 1e6;
		sig.freqFin = sig.freqInit;
		signalNum++;
	}
	/// make the moving traps in the moving axis.
	bool piFound = false;
	for ( auto loc : moveInfo.locationsToMove )
	{
		if ( !moveInfo.isInlineParallel && piFound )
		{
			break;
		}
		else if ( !moveInfo.isInlineParallel )
		{
			piFound = true;
		}
		moveWave.chan[ movingAxis ].signals.push_back ( waveSignal ( ) );
		waveSignal& sig = moveWave.chan[ movingAxis ].signals[ signalNum ];
		sig.powerRampType = "nr";
		sig.initPhase = 0;
		sig.finPower = sig.initPower = moveBias;
		sig.freqRampType = "fast";
		UINT init = upOrDown ? loc.row : loc.column;
		UINT fin = ( upOrDown ? loc.row : loc.column ) + moveInfo.dirInt ( );
		sig.freqInit = ( upOrDown ? ( ( targetRows - init - 1 ) * freqPerPixel + lowFreqs[ movingAxis ] ) :
						 init * freqPerPixel + lowFreqs[ movingAxis ] )* 1e6;
		sig.freqFin = ( upOrDown ? ( ( targetRows - fin - 1 ) * freqPerPixel + lowFreqs[ movingAxis ] ) :
						fin * freqPerPixel + lowFreqs[ movingAxis ] )* 1e6;
		signalNum++;
	}
	/// handle static axis /////////////////
	// much simpler than the moving axis, just 1 branch.
	if ( moveInfo.needsFlash )
	{
		moveWave.chan[ staticAxis ].signals.resize ( moveInfo.locationsToMove.size ( ) );
		UINT sigCount = 0;
		for ( auto atom : moveInfo.locationsToMove )
		{
			waveSignal& sig = moveWave.chan[ staticAxis ].signals[ sigCount ];
			// equal power in all of these.
			sig.finPower = sig.initPower = 1;
			sig.freqRampType = sig.powerRampType = "nr";
			sig.initPhase = 0;
			sig.freqInit = ( upOrDown ? atom.column * freqPerPixel + lowFreqs[ staticAxis ] :
				( targetRows - atom.row - 1 ) * freqPerPixel + lowFreqs[ staticAxis ] ) * 1e6;
			sig.freqFin = sig.freqInit;
			sigCount++;
		}
	}
	else
	{
		// no flash, so static axis must span all tweezers
		moveWave.chan[ staticAxis ].signals.resize ( upOrDown ? targetRows : targetCols );
		UINT sigCount = 0;
		for ( auto& sig : moveWave.chan[ staticAxis ].signals )
		{
			sig.freqRampType = sig.powerRampType = "nr";
			auto staticPos = !upOrDown ? rerngSettings.staticBiases[ staticAxis ].size ( ) - sigCount - 1 : sigCount;
			sig.initPhase = rerngSettings.staticPhases[ staticAxis ][ staticPos ];
			sig.freqInit = ( upOrDown ? sigCount * freqPerPixel + lowFreqs[ staticAxis ] :
				( targetRows - sigCount - 1 ) * freqPerPixel + lowFreqs[ staticAxis ] ) * 1e6;
			sig.freqFin = sig.freqInit;
			// use the calibrated even biases
			sig.initPower = sig.finPower = rerngSettings.staticBiases[ staticAxis ][ staticPos ];
			sigCount++;
		}
	}
	/// finalize info & calculate things
	finalizeStandardWave ( moveWave, debugInfo ( ) );
	return moveWave;
}


std::vector<double> NiawgController::makeFullRerngWave( rerngScriptInfo& rerngSettings, double staticMovingRatio, 
													    double moveBias, double deadTime, UINT sourceRows, UINT sourceCols, 
													    complexMove moveInfo )
{
	/*
	double freqPerPixel = rerngSettings.freqPerPixel;
	// starts from the top left.
	bool upOrDown = (moveInfo.moveDir == dir::down || moveInfo.moveDir == dir::up);
	UINT movingAxis = upOrDown ? Axes::Vertical : Axes::Horizontal;
	UINT staticAxis = (!upOrDown) ? Axes::Vertical : Axes::Horizontal;
	UINT movingSize = upOrDown ? sourceRows : sourceCols;
	auto targetCols = rerngSettings.target.getCols( );
	auto targetRows = rerngSettings.target.getRows( );
	auto& lowFreqs = rerngSettings.lowestFreqs;
	simpleWave moveWave;
	moveWave.varies = false;
	moveWave.name = "NOT-USED";
	// if flashing, this time + staticWave.time needs to be = rerngSettings.timePerMove
	moveWave.time = (moveInfo.needsFlash ? rerngSettings.timePerMove / (staticMovingRatio + 1) : rerngSettings.timePerMove);
	moveWave.sampleNum = waveformSizeCalc( moveWave.time );
	bool offGridDump = true;
	double nonMovingFrac = offGridDump ? 1 - moveBias : (1 - moveBias) / (movingSize - 2);
	/// handle moving axis /////////////
	/// figure out where to put static "dump" traps in the moving axis
	std::vector<int> staticTweezers;
	if ( !offGridDump || !moveInfo.needsFlash )
	{
		// if not flashing then need to put excess power in the other tweezers to hold them during the move.
		// for every possible static tweezer
		for ( auto potentialStaticGridLoc : range( movingSize ) )
		{
			bool isUsed = false;
			for ( auto loc : moveInfo.locationsToMove )
			{
				// if location is init or final location of this location's move...
				if ( potentialStaticGridLoc == (upOrDown ? loc.row : loc.column) ||
					 potentialStaticGridLoc == (upOrDown ? loc.row + moveInfo.dirInt( ) : loc.column + moveInfo.dirInt( )) )
				{
					isUsed = true;
				}
			}
			if ( !isUsed )
			{
				// then should be a static tweezer
				staticTweezers.push_back( potentialStaticGridLoc );
			}
		}
	}
	else
	{
		// put it off-axis on the low frequency side of things. 
		staticTweezers.push_back( -1 );
	}
	/// make the static "dump" tweezers in the moving axis
	UINT signalNum = 0;
	for ( auto gridLoc : staticTweezers )
	{
		moveWave.chan[movingAxis].signals.push_back( waveSignal( ) );
		waveSignal& sig = moveWave.chan[movingAxis].signals[signalNum];
		// static
		sig.freqRampType = sig.powerRampType = "nr";
		auto staticPos = upOrDown ? rerngSettings.staticBiases[movingAxis].size( ) - gridLoc - 1: gridLoc;
		sig.finPower = sig.initPower = nonMovingFrac * (gridLoc==-1 ? 1 : rerngSettings.staticBiases[movingAxis][staticPos]);
		sig.initPhase = (gridLoc==-1 ? 0 : rerngSettings.staticPhases[movingAxis][staticPos]);
		sig.freqInit = (upOrDown) ? ((targetRows - gridLoc - 1) * freqPerPixel + lowFreqs[movingAxis]) :
								     (gridLoc * freqPerPixel + lowFreqs[movingAxis]);
		sig.freqInit *= 1e6;
		sig.freqFin = sig.freqInit;
		signalNum++;
	}
	/// make the moving traps in the moving axis.
	bool piFound = false;
	for ( auto loc : moveInfo.locationsToMove )
	{
		if ( !moveInfo.isInlineParallel && piFound )
		{
			break;
		}
		else if ( !moveInfo.isInlineParallel )
		{
			piFound = true;
		}
		moveWave.chan[movingAxis].signals.push_back( waveSignal( ) );
		waveSignal& sig = moveWave.chan[movingAxis].signals[signalNum];
		sig.powerRampType = "nr";
		sig.initPhase = 0;
		sig.finPower = sig.initPower = moveBias;
		sig.freqRampType = "fast";
		UINT init = upOrDown ? loc.row : loc.column;
		UINT fin = (upOrDown ? loc.row : loc.column) + moveInfo.dirInt( );
		sig.freqInit = (upOrDown ? ((targetRows - init - 1) * freqPerPixel + lowFreqs[movingAxis]) :
						 init * freqPerPixel + lowFreqs[movingAxis])* 1e6;
		sig.freqFin = (upOrDown ? ((targetRows - fin - 1) * freqPerPixel + lowFreqs[movingAxis]) :
						fin * freqPerPixel + lowFreqs[movingAxis])* 1e6;
		signalNum++;
	}
	/// handle static axis /////////////////
	// much simpler than the moving axis, just 1 branch.
	if ( moveInfo.needsFlash )
	{
		moveWave.chan[staticAxis].signals.resize( moveInfo.locationsToMove.size() );
		UINT sigCount = 0;
		for ( auto atom : moveInfo.locationsToMove )
		{
			waveSignal& sig = moveWave.chan[staticAxis].signals[sigCount];
			// equal power in all of these.
			sig.finPower = sig.initPower = 1;
			sig.freqRampType = sig.powerRampType = "nr";
			sig.initPhase = 0;
			sig.freqInit = (upOrDown ? atom.column * freqPerPixel + lowFreqs[staticAxis] :
									   (targetRows - atom.row - 1) * freqPerPixel + lowFreqs[staticAxis] ) * 1e6;
			sig.freqFin = sig.freqInit;
			sigCount++;
		}
	}
	else
	{
		// no flash, so static axis must span all tweezers
		moveWave.chan[staticAxis].signals.resize( upOrDown ? targetRows : targetCols );
		UINT sigCount = 0;
		for ( auto& sig : moveWave.chan[staticAxis].signals )
		{
			sig.freqRampType = sig.powerRampType = "nr";
			auto staticPos = !upOrDown ? rerngSettings.staticBiases[staticAxis].size( ) - sigCount - 1 : sigCount;
			sig.initPhase = rerngSettings.staticPhases[staticAxis][staticPos];
			sig.freqInit = (upOrDown ? sigCount * freqPerPixel + lowFreqs[staticAxis] :
							(targetRows - sigCount - 1) * freqPerPixel + lowFreqs[staticAxis]) * 1e6;
			sig.freqFin = sig.freqInit;
			// use the calibrated even biases
			sig.initPower = sig.finPower = rerngSettings.staticBiases[staticAxis][staticPos];
			sigCount++;
		}
	}

	/// finalize info & calculate things
	finalizeStandardWave( moveWave, debugInfo( ) );
	*/
	auto moveWave = makeRerngWaveMovePart ( rerngSettings, moveBias, sourceRows, sourceCols, moveInfo );
	// now put together into small temporary flashing wave
	waveInfo flashMove;
	flashMove.core.time = rerngSettings.timePerMove;
	flashMove.flash.isFlashing = true;
	flashMove.flash.flashNumber = 1 + int(moveInfo.needsFlash);
	flashMove.flash.deadTime = deadTime;
	if ( moveInfo.needsFlash 
		 && (fabs( rerngSettings.staticWave.time + moveWave.time - rerngSettings.timePerMove ) > 1e-9 ))
	{
		thrower( "ERROR: static wave and moving wave don't add up to the total time of the flashing wave! "
				 "static time was " + str( rerngSettings.staticWave.time ) + ", move time was "
				 + str( moveWave.time ) + ", and total time was " + str(rerngSettings.timePerMove) + ".\r\n" );
	}
	flashMove.flash.flashWaves.push_back( moveWave );
	if ( moveInfo.needsFlash )
	{
		flashMove.flash.flashWaves.push_back( rerngSettings.staticWave );
	}
	flashMove.flash.flashCycleFreq = rerngSettings.flashingFreq;
	mixFlashingWaves( flashMove, deadTime, staticMovingRatio );

	return flashMove.core.waveVals;
}


void NiawgController::rerngGuiOptionsFormToFinal( rerngGuiOptionsForm& form, rerngGuiOptions& data, 
											      std::vector<parameterType>& variables, UINT variation )
{
	data.active = form.active;
	data.deadTime = form.deadTime.evaluate( variables, variation );
	data.flashingRate = form.flashingRate.evaluate( variables, variation );
	data.moveBias = form.moveBias.evaluate( variables, variation );
	data.moveSpeed = form.moveSpeed.evaluate( variables, variation );
	data.staticMovingRatio = form.staticMovingRatio.evaluate( variables, variation );
	data.finalMoveTime = form.finalMoveTime.evaluate( variables, variation ) / 1e3;
	//
	data.outputIndv = form.outputIndv;
	data.outputInfo = form.outputInfo;
	data.preprogram = form.preprogram;
	data.useCalibration = form.useCalibration;
	data.auxStatic = form.auxStatic;
}


void NiawgController::startRerngThread( atomQueue* atomQueue, waveInfo wave, Communicator* comm, 
										std::mutex* rearrangerLock, chronoTimes* andorImageTimes, 
										chronoTimes* grabTimes, std::condition_variable* rearrangerConditionWatcher,
										rerngGuiOptions guiOptions, atomGrid grid )
{
	threadStateSignal = true;
	rerngThreadInput* input = new rerngThreadInput( grid.height, grid.width);
	input->sourceRows = grid.height;
	input->sourceCols = grid.width;
	input->guiOptions = guiOptions;
	input->pictureTimes = andorImageTimes;
	input->grabTimes = grabTimes;
	input->rerngLock = rearrangerLock;
	input->threadActive = &threadStateSignal;
	input->comm = comm;
	input->niawg = this;
	input->atomsQueue = atomQueue;
	input->rerngWave = wave;
	input->rerngConditionWatcher = rearrangerConditionWatcher;
	if ( guiOptions.preprogram )
	{
		preWriteRerngWaveforms( input );
	}
	UINT rearrangerId;
	// start the thread with ~10MB of memory (it may get rounded to some page size)
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
	freqChange[Axes::Vertical] = freqSpacing * (double( finalPos[Axes::Vertical] ) 
												 - double(targetPos[Axes::Vertical]));
	freqChange[Axes::Horizontal] = freqSpacing * (double( finalPos[Axes::Horizontal] ) 
												   - double( targetPos[Axes::Horizontal] ));
	if ( (fabs(freqChange[Axes::Vertical]) < 1e-9) && (fabs( freqChange[Axes::Horizontal] ) < 1e-9))
	{
		return std::vector<double>();
	}
	moveWave.chan[Axes::Vertical].signals.resize( target.getRows() );
	moveWave.chan[Axes::Horizontal].signals.resize( target.getCols() );
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
		if ( input->guiOptions.useCalibration )
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
		if ( input->guiOptions.outputInfo )
		{
			UINT fileNum = getNextFileIndex( DEBUG_OUTPUT_LOCATION + "Rearranging-Event-Info_", ".txt" );
			outFile.open( DEBUG_OUTPUT_LOCATION + "Rearranging-Event-Info_" + str( fileNum ) + ".txt" );
			if ( !outFile.is_open( ) )
			{
				thrower( "ERROR: rearranging Info file failed to open!" );
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
				tempAtoms = (*input->atomsQueue)[0].image;
				if ( tempAtoms.size( ) == 0 )
				{
					// spurious wake-up? This one probably never happens now that I've implemented the 
					// condition_variable.
					continue;
				}
				input->atomsQueue->erase( input->atomsQueue->begin( ) );
			}
			if ( input->atomsQueue->size ( ) != 0 )
			{
				input->comm->sendStatus ( "WARNING: LOOKS LIKE RERNG CODE IS BEHIND IN THE PICTURE QUEUE???" );
			}
			startCalc.push_back(chronoClock::now( ));			
			rerngScriptInfo& info = input->rerngWave.rearrange;
			info.timePerMove = input->guiOptions.moveSpeed;
			info.flashingFreq = input->guiOptions.flashingRate;

			Matrix<bool> source(input->sourceRows, input->sourceCols, 0);
			UINT count = 0;
			for ( auto colCount : range( source.getCols() ) )
			{
				for ( auto rowCount : range( source.getRows() ) )
				{
					source(source.getRows() - 1 - rowCount, colCount) = tempAtoms[count++];
				}
			}
			std::vector<simpleMove> simpleMoveSequence;
			std::vector<complexMove> complexMoveSequence;
			niawgPair<ULONG> finPos;
			try
			{
				smartTargettingRearrangement( source, info.target, finPos, info.finalPosition, simpleMoveSequence, 
									input->guiOptions);
				optimizeMoves( simpleMoveSequence, source, complexMoveSequence, input->guiOptions );
			}
			catch ( Error& )
			{
				// as of now, just ignore. simpleMoveSequence should be empty anyways.
			}
			/// Change this to true to force a specific type of move.
			if ( false )
			{
				complexMoveSequence.clear( );
				complexMove forcedMove;
				forcedMove.isInlineParallel = false;
				coordinate loc;
				loc.row = 5;
				loc.column = 3;
				forcedMove.locationsToMove.push_back(loc);
				forcedMove.moveDir = dir::right;
				forcedMove.needsFlash = false;
				complexMoveSequence.push_back( forcedMove );
			}
			///
			std::string tmpStr = source.print( );
			std::string dum = tmpStr.c_str( );
			numberMoves.push_back( complexMoveSequence.size( ) );
			stopRerngCalc.push_back( chronoClock::now( ) );
			input->niawg->rerngWaveVals.clear( );
			/// program niawg
			debugInfo options;
			UINT moveCount = 0;
			// always start with 10us of the static waveform. Depending on how the NIAWG script is created that does the
			// rearranging, if there is not a buffer waveform before the rearranging, then the NIAWG will preload part of 
			// the yet-to-be-written rearrangement waveform into memory while outputting the previous looping waveform.
			// So really you should always put a buffer waveform for sanity, but this is an automatic little buffer 
			// right here which I think does the job as well.
			//input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), info.fillerWave.waveVals.begin( ),
			//									info.fillerWave.waveVals.begin( ) + 10e-6 * NIAWG_SAMPLE_RATE );
			for ( auto move : complexMoveSequence )
			{
				// program this move.
				std::vector<double> vals;
				UINT row = move.locationsToMove[0].row;
				UINT col = move.locationsToMove[0].column;
				if ( input->guiOptions.preprogram )
				{
					if ( move.needsFlash )
					{
						auto tmpMove = input->flashMoves( row, col, move.moveDir );
						vals = tmpMove.waveVals;
					}
					else
					{
						vals = input->noFlashMoves( row, col, move.moveDir ).waveVals;
					}
				}
				else
				{
					double bias = input->guiOptions.useCalibration ?
						calBias( row, col, move.moveDir ) : input->guiOptions.moveBias;
					if ( move.needsFlash )
					{
						bias *= move.locationsToMove.size( );
					}
					if ( bias > 1 )
					{
						bias = 1;
					}
					if ( input->guiOptions.useFast )
					{
						vals = input->niawg->makeFastRerngWave( info, input->sourceRows, input->sourceCols, move,
																input->guiOptions, bias );
					}
					else
					{
						vals = input->niawg->makeFullRerngWave( info, input->guiOptions.staticMovingRatio, bias,
															input->guiOptions.deadTime, input->sourceRows,
															input->sourceCols, move );
					}
				}
				input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), vals.begin( ), vals.end( ) );
				// put a break statement here to limit the rearranging algorithm to 1 move at a time.
				break;
				if ( moveCount++ > 8 )
				{
					break;
				}
			}
			stopMoveCalc.push_back( chronoClock::now( ) );
			/// Finishing Move to move the atoms to the desired location.
			std::vector<double> finalMove;
			finalMove = input->niawg->calcFinalPositionMove( finPos, info.finalPosition, info.freqPerPixel, 
															 info.target, info.lowestFreqs, 
															 input->guiOptions.finalMoveTime );
			input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), finalMove.begin( ),
												finalMove.end( ) );
			finMoveCalc.push_back( chronoClock::now( ) );
			// the filler wave holds the total length of the wave. Add the differnece in size between the filler wave
			// size and the existing size to fill out the rest of the vector.
			input->niawg->rerngWaveVals.insert( input->niawg->rerngWaveVals.end( ), info.fillerWave.waveVals.begin( ),
												info.fillerWave.waveVals.begin( ) + info.fillerWave.waveVals.size()
												- input->niawg->rerngWaveVals.size() );
			stopAllCalc.push_back(chronoClock::now( ));
			input->niawg->fgenConduit.resetWritePosition( );
			input->niawg->streamRerng( );
			stopStream.push_back( chronoClock::now( ) );
			input->niawg->fgenConduit.sendSoftwareTrigger( );
			stopTrigger.push_back( chronoClock::now( ));
			input->niawg->fgenConduit.resetWritePosition( );
			stopReset.push_back( chronoClock::now( ));
			if ( complexMoveSequence.size( ) )
			{
				triedRearranging.push_back( true );
			}
			else
			{
				triedRearranging.push_back( false );
			}
			//input->niawg->writeToFile( input->niawg->rerngWaveVals );
			input->niawg->rerngWaveVals.clear( );
			if ( complexMoveSequence.size( ) != 0 )
			{
				if ( input->guiOptions.outputIndv )
				{
					input->comm->sendStatus( "Tried Moving, " + str( complexMoveSequence.size() ) 
											 + " Moves. Move Calc Time:"
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
			if ( input->guiOptions.outputInfo )
			{
				outFile << "Rep # " << counter << "\n";
				outFile << "Source: ";
				UINT colcounter = 0;
				for ( auto elem : source )
				{
					outFile << elem << ", ";
					if ( ++colcounter % source.getCols( ) == 0 )
					{
						outFile << "; ";
					}
				}
				outFile << "\nTarget Location: " + str( finPos[0] ) + ' ' + str( finPos[1] ) + "\n";
				outFile << "Moves:\n";
				UINT moveCount = 0;
				for ( auto move : complexMoveSequence )
				{
					outFile << moveCount++ << " " << move.needsFlash << " " << int(move.moveDir) << " " <<
						move.isInlineParallel << "\n";
					for ( auto atom : move.locationsToMove )
					{
						outFile << atom.row << " " << atom.column << "\n";
					}
					outFile << "\n";
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

		std::ofstream dataFile( DEBUG_OUTPUT_LOCATION + "Rearrangement-Time-Log.txt" );
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
			errBox( "ERROR: file failed to open for rearrangement time log!" );
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

Matrix<bool> NiawgController::calculateFinalTarget ( Matrix<bool> target, niawgPair<ULONG> finalPos, UINT rows, UINT cols )
{
	// finTarget is the correct size, has the original target at finalPos, and zeros elsewhere.
	Matrix<bool> finTarget ( rows, cols, 0 );
	for ( auto rowInc : range ( target.getRows ( ) ) )
	{
		for ( auto colInc : range ( target.getCols ( ) ) )
		{
			finTarget ( rowInc + finalPos[ Axes::Vertical ], colInc + finalPos[ Axes::Horizontal ] )
				= target ( rowInc, colInc );
		}
	}
	return finTarget;
}


void NiawgController::smartTargettingRearrangement( Matrix<bool> source, Matrix<bool> target, niawgPair<ULONG>& finTargetPos, 
												    niawgPair<ULONG> finalPos, std::vector<simpleMove> &moveSequence, 
												    rerngGuiOptions options, bool randomize, bool orderMovesByProximityToTarget )
{
	std::vector<simpleMove> moveList;
	Matrix<bool> finTarget(source.getRows(), source.getCols(), 0);
	// The while loop only comes into play if you don't have enough atoms to make the target, in which case
	// an exception is thrown and the catch uses "continue" to redo the contents of the while loop.
	while ( true )
	{
		try
		{
			if ( source.getRows ( ) == target.getRows ( ) && source.getCols ( ) == target.getCols ( ) )
			{
				// dimensions match, no flexibility for smartness
				rearrangement ( source, target, moveList );
				finTargetPos = { 0,0 };
				finTarget = target;
				break;
			}
			switch ( options.smartOption )
			{
				case smartRerngOption::none:
				{
					// finTarget is the correct size, has the original target at finalPos, and zeros elsewhere.
					finTarget = calculateFinalTarget ( target, finalPos, source.getRows ( ), source.getCols ( ) );
					rearrangement ( source, finTarget, moveList );
					finTargetPos = finalPos;
					break;
				}
				case smartRerngOption::convolution:
				{
					finTargetPos = convolve ( source, target );
					finTarget = calculateFinalTarget ( target, finTargetPos, source.getRows ( ), source.getCols ( ) );
					rearrangement ( source, finTarget, moveList );
					break;
				}
				case smartRerngOption::full:
				{
					// calculate the full rearrangement sequence for every possible final position and use the easiest.
					UINT leastMoves = UINT_MAX;
					for ( auto startRowInc : range ( source.getRows ( ) - target.getRows ( ) + 1 ) )
					{
						for ( auto startColInc : range ( source.getCols ( ) - target.getCols ( ) + 1 ) )
						{
							// create the potential target with the correct offset.
							finTarget = calculateFinalTarget ( target, { startRowInc, startColInc },
															   source.getRows ( ), source.getCols ( ) );
							std::vector<simpleMove> potentialMoves;
							rearrangement ( source, finTarget, potentialMoves );
							if ( potentialMoves.size ( ) < leastMoves )
							{
								// new record.
								moveList = potentialMoves;
								finTargetPos = { source.getRows ( ) - target.getRows ( ) - startRowInc, startColInc };
								leastMoves = potentialMoves.size ( );
								if ( leastMoves == 0 )
								{
									// not possible to move to final location
									break;
								}
							}
						}
					}
					break;
				}
				// didn't throw.
				break;
			}
		}
		catch ( Error& err )
		{
			std::string tmpStr = err.whatBare ( ).substr ( 0, 10 );
			if ( tmpStr == "Less atoms" )
			{
				if ( moveList.size ( ) == 0 )
				{
					// flip one atom and try again with less atoms.
					bool found = false;
					for ( auto& atom : target )
					{
						if ( atom )
						{
							atom = false;
							found = true;
							break;
						}
					}
					if ( found )
					{
						continue;
					}
				}
				break;
			}
			else
			{
				throw;
			}
		}
	}
	/// now order the operations.
	// can randomize first, otherwise the previous algorith always ends up filling the bottom left of the array first.
	if ( randomize )
	{
		randomizeMoves ( moveList );
	}

	if ( orderMovesByProximityToTarget )
	{
		auto comPos = calculateTargetCOM ( finTarget, finTargetPos );
		calculateMoveDistancesToTarget ( moveList, comPos );
		sortByDistanceToTarget ( moveList );
	}

	orderMoves ( moveList, moveSequence, source );
}


void NiawgController::calculateMoveDistancesToTarget ( std::vector<simpleMove> &moveList, niawgPair<double> comPos )
{
	for ( auto& move : moveList )
	{
		move.distanceToTarget = std::sqrt ( std::pow ( comPos[ Axes::Horizontal ] - move.initCol, 2 ) 
											+ std::pow ( comPos[ Axes::Vertical ] - move.initRow, 2 ) );
	}
}


void NiawgController::sortByDistanceToTarget ( std::vector<simpleMove> &moveList )
{
	std::sort ( moveList.begin ( ), moveList.end ( ),
				[] ( simpleMove const& a, simpleMove const& b ) { return a.distanceToTarget < b.distanceToTarget; } );
}



niawgPair<double> NiawgController::calculateTargetCOM ( Matrix<bool> target, niawgPair<ULONG> finalPos )
{
	niawgPair<double> avg = { 0,0 };
	UINT totalAtoms = 0;
	for ( auto p : target )
	{
		for ( auto rowInc : range ( target.getRows ( ) ) )
		{
			for ( auto colInc : range ( target.getCols ( ) ) )
			{
				auto pix = int(target ( rowInc, colInc ));
				avg[ Axes::Vertical ] += rowInc * pix;
				avg[ Axes::Horizontal ] += colInc * pix;
				totalAtoms += pix;
			}
		}
	}
	avg[ Axes::Vertical ] /= totalAtoms;
	avg[ Axes::Horizontal ] /= totalAtoms;
	avg[ Axes::Vertical ] -= finalPos[ Axes::Vertical ];
	avg[ Axes::Horizontal ] -= finalPos[ Axes::Horizontal ];
	return avg;
}


/// everything below here is primarily Kai-Niklas Schymik's work, with minor modifications. Some modifications are
/// minor to improve style consistency with my code, some are renaming params so that I can make sense of what's 
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
				thrower( "ERROR: rearrangement Error! all mateColumn are matched but numberMated < rows!  "
						 "(A low level bug, this shouldn't happen)" );
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

		// update dual params
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


/*
	This should be done before orderMoves The default algorithm will make the moves in an order that tends to fill a 
	certain corner of the pattern first.
*/
void NiawgController::randomizeMoves(std::vector<simpleMove>& operationsList)
{
	std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());
	std::shuffle(std::begin(operationsList), std::end(operationsList), rng);
}




/*
	this part was written by Mark O Brown. The other stuff in the rearrangment handling was written by Kai Niklas.
*/
void NiawgController::orderMoves( std::vector<simpleMove> operationsList, std::vector<simpleMove>& moveSequence, 
								  Matrix<bool> sourceMatrix )
{

	// systemState keeps track of the state of the system after each move. It's important so that the algorithm can
	// avoid making atoms overlap.
	Matrix<bool> systemState = sourceMatrix;
	UINT moveNum = 0;
	while (operationsList.size() != 0)
	{
		if (moveNum >= operationsList.size())
		{
			// it's reached the end, reset this.
			moveNum = 0;
		}
		// make sure that the initial location IS populated and the final location ISN'T.
		bool initIsOpen = systemState(operationsList[moveNum].initRow, operationsList[moveNum].initCol) == false;
		bool finIsOccupied = systemState(operationsList[moveNum].finRow, operationsList[moveNum].finCol) == true;
		if (initIsOpen || finIsOccupied)
		{
			moveNum++;
			continue;
		}
		// else it's okay. add this to the list of moves.
		moveSequence.push_back(operationsList[moveNum]);
		// update the system state after this move.
		systemState(operationsList[moveNum].initRow, operationsList[moveNum].initCol) = false;
		systemState(operationsList[moveNum].finRow, operationsList[moveNum].finCol) = true;
		// remove the move from the list of moves.
		operationsList.erase(operationsList.begin() + moveNum);
	}
	// at this point moveList should be zero size and moveSequence should be full of the moves in a sequence that
	// works. return the travelled distance.
}


double NiawgController::rearrangement( Matrix<bool> & sourceMatrix, Matrix<bool> & targetMatrix,
									   std::vector<simpleMove>& moveList, bool randomize )
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
	// input for bipartite matching algorithm, Algorithm writes into these vectors
	std::vector<int> left;
	std::vector<int> right;

	// The returned cost is the travelled distance
	double cost = minCostMatching( costMatrix, left, right );

	/// calculate the move list

	// std::vector<simpleMove> moveList;
	moveList.resize( cost, { 0,0,0,0 } );

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
			moveList[counter].initRow = init_x;
			moveList[counter].initCol = init_y;
			moveList[counter].finRow = init_x + sign( step_x );
			moveList[counter].finCol = init_y;
			init_x = init_x + sign( step_x );
			counter++;
		}
		for (int yStepInc = 0; yStepInc < abs( step_y ); yStepInc++)
		{
			moveList[counter].initRow = init_x;
			moveList[counter].initCol = init_y;
			moveList[counter].finRow = init_x;
			moveList[counter].finCol = init_y + sign( step_y );
			init_y = init_y + sign( step_y );
			counter++;
		}
	}
	return cost;
} 


void NiawgController::writeToFile( std::vector<double> waveVals )
{
	static UINT writeToStaticNumber = 0;
	std::ofstream file( DEBUG_OUTPUT_LOCATION + "Wave_" + str( writeToStaticNumber++ ) + ".txt" );
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


// for debugging / visualization purposes. note that the returned vector will be one longer than the number of moves
// because it includes the original image.
std::vector<std::string> NiawgController::evolveSource( Matrix<bool> source, std::vector<complexMove> flashMoves )
{
	std::vector<std::string> images;
	images.push_back( source.print( ) );
	for ( auto move : flashMoves )
	{
		for ( auto loc : move.locationsToMove )
		{
			if ( !source( loc.row, loc.column) )
			{
				throw;
			}
			// potentially could move a blank...
			bool leftRight = (move.moveDir == dir::right || move.moveDir == dir::left);
			auto finrow = loc.row + (!leftRight)*move.dirInt( ), finCol = loc.row + leftRight*move.dirInt( );
			source( finrow, finCol ) = source( loc.row, loc.column );
			source( loc.row, loc.column ) = false;
		}
		images.push_back( source.print( ) );
	}
	return images;
}


/* 
	Handles parallelizing moves and determining if flashing is necessary for moves or not. The parallelizing part of this is very tricky.
*/
void NiawgController::optimizeMoves( std::vector<simpleMove> singleMoves, Matrix<bool> origSource, 
									 std::vector<complexMove> &flashMoves, rerngGuiOptions options )
{
	Matrix<bool> runningSource = origSource;
	// convert all single moves into complex moves.
	// procedure for combining at the moment (not really optimal...):
	/*
	  Outer loops are over all possible group move configuraitons. I.e. row=2 -> row3 moves, etc.
	  Grab all moves that match the group move.
	  Determine which moves are actually possible to do at this point, based on an atom being in the initial position
		and no atom being in the final position, removing moves that can't be made.
	  At this I've distilled the relevant simple moves that can be combined into a complex move at this point.
	  Check if move can be made without flashing.
	 */
	UINT initMoveNum = 0;
	while ( singleMoves.size( ) != 0 )
	{
		/*
		A word on notation: pi configuration parallel moves means that the moves share the same moving index.
		inline (sometimes denoted - -) configuration parallel moves means that the moves share the same static index.
		
		in the following, and 'o' indicates atom, '->' indicates direction of movement.

		Pi:
		o->
		o->
		Inline:
		o->o->
		*/
		std::vector<int> pi_moveIndexes;
		std::vector<simpleMove> pi_moveList;
		std::vector<int> inline_moveIndexes;
		std::vector<simpleMove> inline_moveList;
		if ( initMoveNum >= singleMoves.size( ) )
		{
			initMoveNum = 0;
		}
		auto baseMove = singleMoves[int(initMoveNum)];
		auto moveDir = baseMove.dir( );
		auto baseMoveIndex = baseMove.movingIndex( );
		auto baseStaticIndex = baseMove.staticIndex( );
		auto altSize = ((moveDir == dir::up || moveDir == dir::down) ? origSource.getCols() : origSource.getRows( ));
		pi_moveIndexes.push_back( int(initMoveNum) );
		inline_moveIndexes.push_back( int( initMoveNum ) );
		pi_moveList.push_back( baseMove );
		inline_moveList.push_back( baseMove );
		// grab all moves that match the initial row(column) and the final row(column).
		for ( auto moveInc : range( singleMoves.size( ) ) )
		{
			if ( moveInc == initMoveNum )
			{
				continue;
			}
			if ( (options.parallel == parallelMoveOption::partial && pi_moveList.size( ) == PARTIAL_PARALLEL_LIMIT)
					|| (options.parallel == parallelMoveOption::none && pi_moveList.size( ) == 1) )
			{
				// already have all the moves we want for combining.
				break;
			}
			simpleMove testMove = singleMoves[moveInc];
			if ( testMove.dir( ) == moveDir )
			{
				if ( testMove.movingIndex( ) == baseMoveIndex )
				{
					// the move is compatible with being parallelized.
					// avoid repeats by checking if singleMoves is in pi_moveList first
					if ( std::find( pi_moveList.begin( ), pi_moveList.end( ), testMove ) == pi_moveList.end( ) )
					{
						pi_moveIndexes.push_back( moveInc );
						pi_moveList.push_back( testMove );
					}
				}
				if ( testMove.staticIndex( ) == baseStaticIndex )
				{
					// the move is compatible with being parallelized.
					// avoid repeats by checking if singleMoves is in pi_moveList first
					if ( std::find( inline_moveList.begin( ), inline_moveList.end( ), testMove ) 
						 == inline_moveList.end( ) )
					{
						inline_moveIndexes.push_back( moveInc );
						inline_moveList.push_back( testMove );
					}
				}
			}
		}

		// From the moves that go from dim to dim+offset, get which have atom at initial position and have no 
		// atom at the final position
		for ( unsigned k = pi_moveIndexes.size( ); k-- > 0; )
		{
			auto& move = singleMoves[pi_moveIndexes[k]];
			// check that initial spot has atom & final spot is free
			if ( !(runningSource( move.initRow, move.initCol ) && !runningSource( move.finRow, move.finCol )) )
			{
				// can't move this one, remove from list.
				pi_moveIndexes.erase( pi_moveIndexes.begin( ) + k );
				pi_moveList.erase( pi_moveList.begin( ) + k );
			}
		}
		// much more tricky for inline moves. Want to be able to allow move to an initially filled
		// location if the atom in the initially filed location can move. i.e. allowing o->o->[] where [] is an empty 
		// site.
		// first index here is location, second is whether cleared (+1), blocked (-1), or unknown (0)
		std::vector<std::pair<UINT, int>> potentialMoveLocations;
		for ( auto move : inline_moveList )
		{
			potentialMoveLocations.push_back( { move.movingIndex( ), 0 } );
		}

		// loop on the following rules until move list reaches static-ness.
		bool changed = true;
		while ( changed )
		{
			changed = false;
			for ( unsigned k = potentialMoveLocations.size( ); k-- > 0; )
			{
				// first case here is just a result of keeping potential MoveKocations the same size for proper 
				// indexing. Second occurs if location's fate has already been decided.
				if ( k > inline_moveIndexes.size( ) || potentialMoveLocations[k].second != 0)
				{
					continue;
				}
				auto& move = singleMoves[inline_moveIndexes[k]];
				// check that initial spot has atom & final spot is free
				if ( runningSource( move.initRow, move.initCol ) )
				{
					if ( !runningSource( move.finRow, move.finCol ) )
					{
						potentialMoveLocations[k].second = 1;
						changed = true;
					}
					else
					{
						for ( auto potentialLoc : potentialMoveLocations )
						{
							if ( potentialLoc.second == 1 )
							{
								potentialMoveLocations[k].second = 1;
								// then blockage is moving out of the way. good.
								changed = true;
							}
							else if ( potentialLoc.second == 0 )
							{
								// unknown, might still be moved. Don't do anything.
							}
							else
							{
								// will not be moved.
								potentialMoveLocations[k].second = -1;
								inline_moveIndexes.erase( inline_moveIndexes.begin( ) + k );
								inline_moveList.erase( inline_moveList.begin( ) + k );
								changed = true;
							}
							break;
						}
					}
				}
				else
				{
					potentialMoveLocations[k].second = -1;
					inline_moveIndexes.erase( inline_moveIndexes.begin( ) + k );
					inline_moveList.erase( inline_moveList.begin( ) + k );
					changed = true;
				}
			}
		}
		// take the better result.
		auto moveIndexes = (inline_moveList.size( ) > pi_moveList.size( )) ? inline_moveIndexes : pi_moveIndexes ;
		// auto moveIndexes = pi_moveIndexes;
		if ( moveIndexes.size( ) == 0 )
		{
			if ( singleMoves.size( ) == 1 )
			{
				thrower( "ERROR: somehow the last single move can't be made..." );
			}
			// couldn't move any atoms. try a different starting point.
			initMoveNum++;
			continue;
		}

		flashMoves.push_back( complexMove( moveDir ) );
		flashMoves.back( ).isInlineParallel = (inline_moveList.size( ) > pi_moveList.size( ));
		/// create complex move objs
		Matrix<bool> tmpSource = runningSource;
		for ( auto indexNumber : range( moveIndexes.size( ) ) )
		{
			// offset from moveIndexes is the # of moves already erased.
			UINT moveIndex = moveIndexes[indexNumber] - indexNumber;
			auto& move = singleMoves[moveIndex];
			flashMoves.back( ).locationsToMove.push_back( { move.initRow, move.initCol } );
			// update source image with new configuration.
			bool prevtofalse = tmpSource( move.initRow, move.initCol );
			tmpSource( move.initRow, move.initCol ) = false;
			bool prevtotrue = tmpSource( move.finRow, move.finCol );
			tmpSource( move.finRow, move.finCol ) = true;
			tmpSource.updateString( );
			singleMoves.erase( singleMoves.begin( ) + moveIndex );
		}
		/// Handle Smart-Flashing
		if ( options.noFlashOption != nonFlashingOption::none )
		{
			flashMoves.back( ).needsFlash = false;
			// loop through all locations in the row / collumn
			for ( auto location : range( altSize ) )
			{
				UINT initRow, initCol, finRow, finCol;
				bool isRow = (moveDir == dir::up || moveDir == dir::down);
				initRow = isRow ? baseMoveIndex : location;
				initCol = isRow ? location : baseMoveIndex;
				// if atom in location and location not being moved, always need to flash to not move this atom.
				if ( runningSource( initRow, initCol ) && std::find( flashMoves.back( ).locationsToMove.begin( ),
													   				 flashMoves.back( ).locationsToMove.end( ), 
																	 coordinate({ initRow, initCol }) )
					 == flashMoves.back( ).locationsToMove.end( ) )
				{
					flashMoves.back( ).needsFlash = true;
				}
				// also check the final locaiton.
				if ( options.noFlashOption == nonFlashingOption::cautious )
				{
					finRow = initRow + isRow * (baseMove.dirInt( ));
					finCol = initCol + (!isRow) * (baseMove.dirInt( ));
					if ( runningSource( finRow, finCol ) )
					{
						flashMoves.back( ).needsFlash = true;
					}
				}
			}
		}
		else
		{
			flashMoves.back( ).needsFlash = true;
		}
		runningSource = tmpSource;
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

