// created by Mark O. Brown
#include "stdafx.h"
#include "AoSystem.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"
#include "Version.h"
// for other ni stuff
#include "nidaqmx2.h"
#include "Thrower.h"
#include "range.h"
#include "CodeTimer.h"
#include <boost/lexical_cast.hpp>

AoSystem::AoSystem(bool aoSafemode) : daqmx( aoSafemode )
{
	/// set some constants...
	// Both are 0-INDEXED. D16
	dacTriggerLine = { 3, 15 };
	// paraphrasing adam...
	// Dacs sample at 1 MHz, so 0.5 us is appropriate.
	// in ms.
	// ?? I thought it was 10 MHz...
	dacTriggerTime = 0.0005;
	//try
	//{
		// initialize tasks and chanells on the DACs
		long output = 0;
		// Create a task for each board
		// assume 3 boards, 8 channels per board. AMK 11/2010, modified for three from 2
		// task names are defined as public variables of type Long in TheMainProgram Declarations
		daqmx.createTask("Board 3 Dacs 16-23", analogOutTask2);
		daqmx.createTask("Board 2 Dacs 8-15", analogOutTask1);
		daqmx.createTask("Board 1 Dacs 0-7", analogOutTask0);
		daqmx.createAoVoltageChan(analogOutTask0, "dev2/ao0:7", "StaticDAC_1", -10, 10, DAQmx_Val_Volts, "");
		daqmx.createAoVoltageChan(analogOutTask1, "dev3/ao0:7", "StaticDAC_0", -10, 10, DAQmx_Val_Volts, "");
		daqmx.createAoVoltageChan(analogOutTask2, "dev4/ao0:7", "StaticDAC_2", -10, 10, DAQmx_Val_Volts, "");
		// This creates a task to read in a digital input from DAC 0 on port 0 line 0
		daqmx.createTask("", digitalDac_0_00);
		daqmx.createTask("", digitalDac_0_01);
		/// unused at the moment.
		daqmx.createDiChan(digitalDac_0_00, "dev2/port0/line0", "DIDAC_0", DAQmx_Val_ChanPerLine);
		daqmx.createDiChan(digitalDac_0_01, "dev2/port0/line1", "DIDAC_0", DAQmx_Val_ChanPerLine);
		// new
	//}
	// I catch here because it's the constructor, and catching elsewhere is weird.
	//catch (Error& exception)
	//{
	//	errBox(exception.trace());
	//}
}


bool AoSystem::handleArrow ( CWnd* focus, bool up )
{
	if ( quickChange.GetCheck ( ) )
	{
		for ( auto& output : outputs )
		{
			if ( output.handleArrow ( focus, up ) )
			{
				return true;
			}
		}
	}
	return false;
}


void AoSystem::standardNonExperiemntStartDacsSequence( )
{
	updateEdits( );
	organizeDacCommands( 0, 0 );
	makeFinalDataFormat( 0, 0 );
	stopDacs( );
	configureClocks( 0, false, 0 );
	writeDacs( 0, false, 0 );
	startDacs( );
}


void AoSystem::forceDacs( DioSystem* ttls )
{
	ttls->resetTtlEvents( );
	resetDacEvents( );
	handleSetDacsButtonPress( ttls );
	standardNonExperiemntStartDacsSequence( );
	ttls->standardNonExperimentStartDioSequence( );
}


void AoSystem::zeroDacs( DioSystem* ttls )
{
	resetDacEvents( );
	ttls->resetTtlEvents( );
	prepareForce( );
	ttls->prepareForce( );
	for ( int dacInc : range( 24 ) )
	{
		prepareDacForceChange( dacInc, 0, ttls );
	}
	standardNonExperiemntStartDacsSequence( );
	ttls->standardNonExperimentStartDioSequence( );
}


std::array<AoInfo, 24> AoSystem::getDacInfo( )
{
	std::array<AoInfo, 24> info;
	for ( auto dacNum : range(outputs.size()) )
	{
		info[ dacNum ] = outputs[ dacNum ].info;
	}
	return info;
}


void AoSystem::setSingleDac( UINT dacNumber, double val, DioSystem* ttls )
{
	ttls->resetTtlEvents( );
	resetDacEvents( );
	/// 
	dacCommandFormList.clear( );
	prepareForce( );
	ttls->prepareForce( );
	prepareDacForceChange( dacNumber, val, ttls );
	checkValuesAgainstLimits( 0, 0 );
	///
	standardNonExperiemntStartDacsSequence( );
	ttls->standardNonExperimentStartDioSequence( );
	updateEdits( );
}


void AoSystem::handleOpenConfig(std::ifstream& openFile, Version ver)
{
	UINT dacInc = 0;
	if ( ver < Version ( "3.7" ) )
	{
		for ( auto i : range ( 24 ) )
		{
			std::string trash;
			openFile >> trash;
		}
	}
}


void AoSystem::handleNewConfig( std::ofstream& newFile )
{
	newFile << "DACS\n";
	// nothing at the moment.
	newFile << "\nEND_DACS\n";
}


void AoSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "DACS\n";
	// nothing at the moment.
	saveFile << "\nEND_DACS\n";
}


std::string AoSystem::getDacSequenceMessage(UINT variation, UINT seqNum)
{
	std::string message;
	for ( auto snap : dacSnapshots(seqNum,variation) )
	{
		std::string time = str( snap.time, 12, true );
		message += time + ":\r\n";
		int dacCount = 0;
		for ( auto val : snap.dacValues )
		{
			std::string volt = str( val, true );
			message += volt + ", ";
			dacCount++;
			if ( dacCount % 8 == 0 )
			{
				message += "\r\n";
			}
		}
		message += "\r\n---\r\n";
	}
	return message;
}

/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AoSystem::handleEditChange ( UINT dacNumber )
{
	if ( dacNumber >= outputs.size ( ) )
	{
		thrower ( "attempted to handle dac edit change, but the dac number reported doesn't exist!" );
	}
	outputs[ dacNumber ].handleEdit ( roundToDacPrecision );
}


bool AoSystem::isValidDACName(std::string name)
{
	for (auto dacInc : range(getNumberOfDacs()) )
	{
		if (name == "dac" + str(dacInc))
		{
			return true;
		}
		else if (getDacIdentifier(name) != -1)
		{
			return true;
		}
	}
	return false;
}


void AoSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	dacTitle.rearrange( width, height, fonts);
	dacSetButton.rearrange( width, height, fonts);
	zeroDacsButton.rearrange( width, height, fonts);
	quickChange.rearrange ( width, height, fonts );
	for ( auto& out : outputs )
	{
		out.rearrange ( width, height, fonts );
	}
}


void AoSystem::setDefaultValue(UINT dacNum, double val)
{
	outputs[ dacNum ].info.defaultVal = val;
}


double AoSystem::getDefaultValue(UINT dacNum)
{
	return outputs[dacNum].info.defaultVal;
}


// this function returns the end location of the set of controls. This can be used for the location for the next control beneath it.
void AoSystem::initialize(POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id )
{
	// title
	dacTitle.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	dacTitle.Create("DACS", WS_CHILD | WS_VISIBLE | SS_CENTER, dacTitle.sPos, master, id++);
	dacTitle.fontType = fontTypes::HeadingFont;
	// 
	dacSetButton.sPos = { pos.x, pos.y, pos.x + 160, pos.y + 25};
	dacSetButton.Create( "Set New DAC Values", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
						 dacSetButton.sPos, master, ID_DAC_SET_BUTTON );
	dacSetButton.setToolTip("Press this button to attempt force all DAC values to the values currently recorded in the"
							 " edits below.", toolTips, master);
	//
	zeroDacsButton.sPos = { pos.x + 160, pos.y, pos.x + 320, pos.y + 25 };
	zeroDacsButton.Create( "Zero Dacs", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, zeroDacsButton.sPos, master, IDC_ZERO_DACS );
	zeroDacsButton.setToolTip( "Press this button to set all dac values to zero.", toolTips, master );

	// 
	quickChange.sPos = { pos.x + 320, pos.y, pos.x + 480, pos.y += 25 };
	quickChange.Create ( "Quick-Change", NORM_CHECK_OPTIONS, quickChange.sPos, master, id++ );
	quickChange.setToolTip ( "With this checked, you can quickly change a DAC's value by using the arrow keys while "
							 "having the cursor before the desired digit selected in the DAC's edit.", toolTips, master );

	int collumnInc = 0;
	
	UINT dacInc = 0;
	for ( auto& out : outputs )
	{
		if ( dacInc == outputs.size ( ) / 3 || dacInc == 2 * outputs.size ( ) / 3 )
		{
			collumnInc++;
			// go to second or third collumn
			pos.y -= 25 * outputs.size ( ) / 3;
			pos.x += 160;
		}
		out.initialize ( pos, master, id, toolTips, dacInc );
		dacInc++;
	}
	pos.x -= 320;
}


void AoSystem::handleRoundToDac( MainWindow* mainWin )
{
	if ( roundToDacPrecision )
	{
		roundToDacPrecision = false;
		mainWin->checkAllMenus ( ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_UNCHECKED );
	}
	else
	{
		roundToDacPrecision = true;
		mainWin->checkAllMenus ( ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_CHECKED );
	}
}


/*
 * get the text from every edit and prepare a change. If fails to get text from edit, if useDefalt this will set such
 * dacs to zero.
 */
void AoSystem::handleSetDacsButtonPress( DioSystem* ttls, bool useDefault )
{
	dacCommandFormList.clear( );
	prepareForce( );
	ttls->prepareForce( );
	std::array<double, 24> vals;
	for (auto dacInc : range(outputs.size()) )
	{
		vals[ dacInc ] = outputs[ dacInc ].getVal ( useDefault );
		prepareDacForceChange( dacInc, vals[dacInc], ttls );
	}
	// wait until after all this to actually do this to make sure things get through okay.
	for ( auto outputNum : range ( outputs.size() ) )
	{
		outputs[ outputNum ].info.currVal = vals[ outputNum ];
		outputs[ outputNum ].setEditColorState ( 0 );
	}
}


void AoSystem::updateEdits( )
{
	for ( auto& dac : outputs )
	{
		dac.updateEdit ( roundToDacPrecision );
	}
}


void AoSystem::organizeDacCommands(UINT variation, UINT seqNum)
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<AoCommand>>> timeOrganizer;
	std::vector<AoCommand> tempEvents(dacCommandList(seqNum,variation));
	// sort the events by time. using a lambda here.
	std::sort( tempEvents.begin(), tempEvents.end(), 
			   [](AoCommand a, AoCommand b){ return a.time < b.time; });
	for (UINT commandInc : range(tempEvents.size()))
	{
		auto& command = tempEvents[commandInc];
		// because the events are sorted by time, the time organizer will already be sorted by time, and therefore I 
		// just need to check the back value's time. Check that the times are greater than a pico second apart. 
		// pretty arbitrary.
		if (commandInc == 0 || fabs( command.time - timeOrganizer.back().first) > 1e-9)
		{
			// new time
			std::vector<AoCommand> quickVec = { command };
			timeOrganizer.push_back ( { command.time, quickVec } );
		}
		else
		{
			// old time
			timeOrganizer.back().second.push_back( command );
		}
	}
	/// make the snapshots
	if ( timeOrganizer.size ( ) == 0 )
	{
		// no commands, that's fine.
		return;
	}
	auto& snap = dacSnapshots(seqNum,variation);
	snap.clear();
	// first copy the initial settings so that things that weren't changed remain unchanged.
	std::array<double, 24> dacValues;
	for ( auto i : range ( outputs.size ( ) ) )
	{
		dacValues[ i ] = outputs[ i ].info.currVal;
	}
	snap.push_back({ 0, dacValues });
	for (auto& command : timeOrganizer)
	{
		// auto& command = timeOrganizer[commandInc];
		// first copy the last set so that things that weren't changed remain unchanged.
		snap.push_back(snap.back());
		snap.back().time = command.first;
		for ( auto& change : command.second )
		{
			// see description of this command above... update everything that changed at this time.
			snap.back().dacValues[change.line] = change.value;
		}
	}
}


void AoSystem::findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, UINT variation, UINT seqNum )
{
	// find the splitting time and set the loadSkip snapshots to have everything after that time.
	auto& snaps = dacSnapshots(seqNum,variation);
	auto& loadSkipSnaps = loadSkipDacSnapshots(seqNum,variation);
	if ( snaps.size( ) == 0 )
	{
		return;
	}
	for ( auto snapshotInc : range( snaps.size( ) - 1 ) )
	{
		if ( snaps[snapshotInc].time < time && snaps[snapshotInc + 1].time >= time )
		{
			loadSkipSnaps = std::vector<AoSnapshot>( snaps.begin( ) + snapshotInc + 1, snaps.end( ) );
			break;
		}
	}
}


std::array<double, 24> AoSystem::getFinalSnapshot()
{
	auto numSeq = dacSnapshots.getNumSequences ( );
	if (numSeq != 0)
	{
		auto numVar = dacSnapshots.getNumVariations ( numSeq - 1 );
		if (numVar != 0)
		{
			if ( dacSnapshots( numSeq - 1, numVar-1 ).size( ) != 0 )
			{
				return dacSnapshots ( numSeq - 1, numVar - 1 ).back( ).dacValues;
			}
		}
	}
	thrower ("No DAC Events");
}


/*
 * IMPORTANT: this does not actually change any of the outputs of the board. It is meant to be called when things have
 * happened such that the control doesn't know what it's own status is, e.g. at the end of an experiment, since the 
 * program doesn't change it's internal memory of all of the status of the aoSys as the experiment runs. (it can't, 
 * besides it would intensive to keep track of that in real time).
 */
void AoSystem::setDacStatusNoForceOut(std::array<double, 24> status)
{
	// set the internal values
	for ( auto outInc : range(outputs.size()) )
	{
		outputs[outInc].info.currVal = status[ outInc ];
		outputs[ outInc ].updateEdit ( roundToDacPrecision );
	}
}


template <typename T> using vec = std::vector<T>;


void AoSystem::fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> dacData, 
							  std::vector<std::vector<double>> finTimes )
{
	std::string message;
	// each element of ttlData should be one ttl line.
	UINT linesPerPlot = 24 / dacData.size( );

	for ( auto line : range( 24 ) )
	{
		auto& data = dacData[line / linesPerPlot][line % linesPerPlot];
		data->clear( );
		UINT seqInc = 0;
		UINT runningSeqTime = 0;
		for (auto seqNum : range(dacSnapshots.getNumSequences( ) ) )
		//for ( auto& dacSeqInfo : dacSnapshots )
		{
			if ( dacSnapshots.getNumVariations(seqNum) <= variation )
			{
				thrower ( "Attempted to use dac data from variation " + str( variation ) + ", which does not "
						 "exist!" );
			}
			for ( auto& snap : dacSnapshots(seqNum, variation) )
			{
				data->push_back( { runningSeqTime + snap.time, double( snap.dacValues[line] ), 0 } );
			}
			runningSeqTime += finTimes[seqInc][variation];
			seqInc++;
		}
	}
}


// an "alias template". effectively a local "using std::vector;" declaration. makes these declarations much more
// readable. I very rarely use things like this.
template<class T> using vec = std::vector<T>;

void AoSystem::interpretKey( std::vector<std::vector<parameterType>>& variables, std::string& warnings )
{
	CodeTimer sTimer;
	sTimer.tick ( "Ao-Sys-Interpret-Start" );
	UINT sequenceLength = variables.size( );
	if ( sequenceLength == 0 )
	{
		sequenceLength = 1;
	}
	UINT variations = variables.front( ).size( ) == 0 ? 1 : variables.front().front( ).keyValues.size( );
	if (variations == 0)
	{
		variations = 1;
	}
	/// imporantly, this sizes the relevant structures.
	dacCommandList.uniformSizeReset ( sequenceLength, variations );
	dacSnapshots.uniformSizeReset ( sequenceLength, variations );
	loadSkipDacSnapshots.uniformSizeReset ( sequenceLength, variations );
	finalFormatDacData.uniformSizeReset ( sequenceLength, variations );
	loadSkipDacFinalFormat.uniformSizeReset ( sequenceLength, variations );
	bool resolutionWarningPosted = false;
	bool nonIntegerWarningPosted = false;
	sTimer.tick ( "After-init" );
	for ( auto seqInc : range( sequenceLength ) )
	{
		for (auto variationInc : range(variations) )
		{
			if ( variationInc == 0 )
			{
				sTimer.tick ( "Variation-" + str ( variationInc ) + "-Start" );
			}
			auto& seqVariables = variables[ seqInc ];
			auto& cmdList = dacCommandList(seqInc, variationInc);
			for (auto eventInc : range( dacCommandFormList[ seqInc ].size ( ) ) )
			{
				AoCommand tempEvent;
				auto& formList = dacCommandFormList[seqInc][eventInc];
				tempEvent.line = formList.line;
				// Deal with time.
				if ( formList.time.first.size( ) == 0 )
				{
					// no variable portion of the time.
					tempEvent.time = formList.time.second;
				}
				else
				{
					double varTime = 0;
					for ( auto variableTimeString : formList.time.first )
					{
						varTime += variableTimeString.evaluate( seqVariables, variationInc );
					}
					tempEvent.time = varTime + formList.time.second;
				}
				if ( variationInc == 0 )
				{
					sTimer.tick ( "Time-Handled" );
				}
				/// deal with command
				if ( formList.commandName == "dac:" )
				{
					/// single point.
					tempEvent.value = formList.finalVal.evaluate( seqVariables, variationInc );
					if ( variationInc == 0 )
					{
						sTimer.tick ( "val-evaluated" );
					}
					cmdList.push_back( tempEvent );
					if ( variationInc == 0 )
					{
						sTimer.tick ( "Dac:-Handled" );
					}
				}
				else if ( formList.commandName == "dacarange:" )
				{
					// interpret ramp time command. I need to know whether it's ramping or not.
					double rampTime = formList.rampTime.evaluate( seqVariables, variationInc );
					/// many points to be made.
					// convert initValue and finalValue to doubles to be used 
					double initValue, finalValue, rampInc;
					initValue = formList.initVal.evaluate( seqVariables, variationInc );
					// deal with final value;
					finalValue = formList.finalVal.evaluate( seqVariables, variationInc );
					// deal with ramp inc
					rampInc = formList.rampInc.evaluate( seqVariables, variationInc );
					if ( rampInc < 10.0 / pow( 2, 16 ) && resolutionWarningPosted )
					{
						resolutionWarningPosted = true;
						warnings += "Warning: ramp increment of " + str( rampInc ) + " in dac command number " 
							+ str(eventInc) + " is below the resolution of the aoSys (which is 10/2^16 = " 
							+ str( 10.0 / pow( 2, 16 ) ) + "). These ramp points are unnecessary.\r\n";
					}
					// This might be the first not i++ usage of a for loop I've ever done... XD
					// calculate the time increment:
					int steps = int( fabs( finalValue - initValue ) / rampInc + 0.5 );
					double stepsFloat = fabs( finalValue - initValue ) / rampInc;
					double diff = fabs( steps - fabs( finalValue - initValue ) / rampInc );
					if ( diff > 100 * DBL_EPSILON && nonIntegerWarningPosted )
					{
						nonIntegerWarningPosted = true;
						warnings += "Warning: Ideally your spacings for a dacArange would result in a non-integer number "
							"of steps. The code will attempt to compensate by making a last step to the final value which"
							" is not the same increment in voltage or time as the other steps to take the dac to the final"
							" value at the right time.\r\n";
					}
					double timeInc = rampTime / steps;
					double initTime = tempEvent.time;
					double currentTime = tempEvent.time;
					// handle the two directions seperately.
					if ( initValue < finalValue )
					{
						for ( double dacValue = initValue; (dacValue - finalValue) < -steps * 2 * DBL_EPSILON; dacValue += rampInc )
						{
							tempEvent.value = dacValue;
							tempEvent.time = currentTime;
							cmdList.push_back( tempEvent );
							currentTime += timeInc;
						}
					}
					else
					{
						for ( double dacValue = initValue; dacValue - finalValue > 100 * DBL_EPSILON; dacValue -= rampInc )
						{
							tempEvent.value = dacValue;
							tempEvent.time = currentTime;
							cmdList.push_back( tempEvent );
							currentTime += timeInc;
						}
					}
					// and get the final value.
					tempEvent.value = finalValue;
					tempEvent.time = initTime + rampTime;
					cmdList.push_back( tempEvent );
					if ( variationInc == 0 )
					{
						sTimer.tick ( "dacarange:-Handled" );
					}
				}
				else if ( formList.commandName == "daclinspace:" )
				{
					// interpret ramp time command. I need to know whether it's ramping or not.
					double rampTime = formList.rampTime.evaluate( seqVariables, variationInc );
					/// many points to be made.
					double initValue, finalValue;
					UINT numSteps;
					initValue = formList.initVal.evaluate( seqVariables, variationInc );
					finalValue = formList.finalVal.evaluate( seqVariables, variationInc );
					numSteps = formList.numSteps.evaluate( seqVariables, variationInc );
					double rampInc = (finalValue - initValue) / numSteps;
					if ( (fabs( rampInc ) < 10.0 / pow( 2, 16 )) && !resolutionWarningPosted )
					{
						resolutionWarningPosted = true;
						warnings += "Warning: numPoints of " + str( numSteps ) + " results in a ramp increment of "
							+ str( rampInc ) + " is below the resolution of the aoSys (which is 10/2^16 = "
							+ str( 10.0 / pow( 2, 16 ) ) + "). It's likely taxing the system to "
							"calculate the ramp unnecessarily.\r\n";
					}
					// This might be the first not i++ usage of a for loop I've ever done... XD
					// calculate the time increment:
					double timeInc = rampTime / numSteps;
					double initTime = tempEvent.time;
					double currentTime = tempEvent.time;
					double val = initValue;
					// handle the two directions seperately.
					for ( auto stepNum : range( numSteps ) )
					{
						tempEvent.value = val;
						tempEvent.time = currentTime;
						cmdList.push_back( tempEvent );
						currentTime += timeInc;
						val += rampInc;
					}
					// and get the final value. Just use the nums explicitly to avoid rounding error I guess.
					tempEvent.value = finalValue;
					tempEvent.time = initTime + rampTime;
					cmdList.push_back( tempEvent );
					if ( variationInc == 0 )
					{
						sTimer.tick ( "daclinspace:-Handled" );
					}
				}
				else
				{
					thrower ( "Unrecognized dac command name: " + formList.commandName );
				}
			}
		}
	}
}


UINT AoSystem::getNumberSnapshots(UINT variation, UINT seqNum)
{
	return dacSnapshots(seqNum,variation).size();
}


void AoSystem::checkTimingsWork(UINT variation, UINT seqInc)
{
	std::vector<double> times;
	// grab all the times.
	for (auto snapshot : dacSnapshots(seqInc,variation))
	{
		times.push_back(snapshot.time);
	}

	int count = 0;
	for (auto time : times)
	{
		int countInner = 0;
		for (auto secondTime : times)
		{
			// don't check against itself.
			if (count == countInner)
			{
				countInner++;
				continue;
			}
			// can't trigger faster than the trigger time.
			if (fabs(time - secondTime) < dacTriggerTime)
			{
				thrower ("timings are such that the dac system would have to get triggered too fast to follow the"
						" programming! ");
			}
			countInner++;
		}
		count++;
	}
}

ULONG AoSystem::getNumberEvents(UINT variation, UINT seqInc)
{
	return dacSnapshots(seqInc,variation).size();
}


// note that this is not directly tied to changing any "current" parameters in the AoSystem object (it of course changes a list parameter). The 
// AoSystem object "current" parameters aren't updated to reflect an experiment, so if this is called for a force out, it should be called in conjuction
// with changing "currnet" parameters in the AoSystem object.
void AoSystem::setDacCommandForm( AoCommandForm command, UINT seqNum )
{
	dacCommandFormList[seqNum].push_back( command );
	// you need to set up a corresponding trigger to tell the aoSys to change the output at the correct time. 
	// This is done later on interpretation of ramps etc.
}


// add a ttl trigger command for every unique dac snapshot.
// MUST interpret key for dac and organize dac commands before setting the trigger events.
void AoSystem::setDacTriggerEvents(DioSystem& ttls, UINT variation, UINT seqInc, UINT totalVariations)
{
	for ( auto snapshot : dacSnapshots(seqInc,variation))
	{
		ttls.ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time, variation, seqInc, totalVariations );
		ttls.ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time + dacTriggerTime, variation, 
							seqInc, totalVariations );
	}
}


std::string AoSystem::getSystemInfo( )
{
	return daqmx.getDacSystemInfo( );
}


// this is a function called in preparation for forcing a dac change. Remember, you need to call ___ to actually change things.
void AoSystem::prepareDacForceChange(int line, double voltage, DioSystem* ttls)
{
	// change parameters in the AoSystem object so that the object knows what the current settings are.
	//std::string volt = str(roundToDacResolution(voltage));
	std::string valStr = roundToDacPrecision? str ( AnalogOutput::roundToDacResolution ( voltage ), 13 ) : str ( voltage, 13 );
	if (valStr.find(".") != std::string::npos)
	{
		// then it's a double. kill extra zeros on the end.
		valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
	}
	outputs[ line ].info.currVal = voltage;
	// I'm not sure it's necessary to go through the procedure of doing this and using the DIO to trigger the aoSys
	// for a foce out. I'm guessing it's possible to tell the DAC to just immediately change without waiting for a 
	// trigger.
	setForceDacEvent ( line, voltage, ttls, 0, 0 );
}


void AoSystem::checkValuesAgainstLimits(UINT variation, UINT seqNum)
{
	for (auto line : range(outputs.size()))
	{
		for (auto snapshot : dacSnapshots(seqNum,variation))
		{
			if (snapshot.dacValues[line] > outputs[line].info.maxVal || snapshot.dacValues[line] <outputs[ line ].info.minVal )
			{
				thrower ("Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
						"value was " + str(snapshot.dacValues[line]) + ", while the minimum accepted value is " +
						str( outputs[ line ].info.minVal) + " and the maximum value is " + str( outputs[ line ].info.maxVal ) + ". "
						"Change the min/max if you actually need to set this value.\r\n");
			}
		}
	}
}


void AoSystem::setForceDacEvent( int line, double val, DioSystem* ttls, UINT variation, UINT seqNum )
{
	if (val > outputs[ line ].info.maxVal || val < outputs[ line ].info.minVal )
	{
		thrower ("Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
				"value was " + str(val) + ", while the minimum accepted value is " +
				str( outputs[ line ].info.minVal ) + " and the maximum value is " + str( outputs[ line ].info.maxVal ) + ". "
				"Change the min/max if you actually need to set this value.\r\n");
	}
	AoCommand eventInfo;
	eventInfo.line = line;
	eventInfo.time = 1;	
	eventInfo.value = val;
	dacCommandList(seqNum,variation).push_back( eventInfo );
	// important! need at least 2 states to run the dac board. can't just give it one value. This is how this was done in the VB code,
	// there might be better ways of dealing with this. 
	eventInfo.time = 10;
	dacCommandList(seqNum,variation).push_back( eventInfo );
	// you need to set up a corresponding pulse trigger to tell the aoSys to change the output at the correct time.
	ttls->ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, 1, 0, 0, 1 );
	ttls->ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, 1 + dacTriggerTime, 0, 0, 1 );
}


ExpWrap<std::vector<AoSnapshot>> AoSystem::getSnapshots ( )
{
	/* used by the unit testing suite. */
	return dacSnapshots;
}

ExpWrap<std::array<std::vector<double>, 3>> AoSystem::getFinData ( )
{
	/* used by the unit testing suite. */
	return finalFormatDacData;
}


void AoSystem::prepareForce( )
{
	initializeDataObjects( 1, 1 );
}


void AoSystem::initializeDataObjects( UINT sequenceNum, UINT cmdNum )
{
	dacCommandFormList = vec<vec<AoCommandForm>>( sequenceNum, vec<AoCommandForm>( cmdNum ) );
	dacCommandList.uniformSizeReset ( sequenceNum, cmdNum );
	dacSnapshots.uniformSizeReset ( sequenceNum, cmdNum );
	loadSkipDacSnapshots.uniformSizeReset ( sequenceNum, cmdNum );
	finalFormatDacData.uniformSizeReset ( sequenceNum, cmdNum );
	loadSkipDacFinalFormat.uniformSizeReset ( sequenceNum, cmdNum );
}


void AoSystem::resetDacEvents()
{
	initializeDataObjects( 0, 0 );
}


void AoSystem::stopDacs()
{
	daqmx.stopTask( analogOutTask0 );
	daqmx.stopTask( analogOutTask1 );
	daqmx.stopTask( analogOutTask2 );
}


void AoSystem::configureClocks(UINT variation, UINT seqNum, bool loadSkip)
{
	long sampleNumber;
	if ( loadSkip )
	{
		sampleNumber = loadSkipDacSnapshots(seqNum,variation).size( );
	}
	else
	{
		sampleNumber = dacSnapshots(seqNum,variation).size( );
	}
	daqmx.configSampleClkTiming( analogOutTask0, "/Dev2/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqmx.configSampleClkTiming( analogOutTask1, "/Dev3/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqmx.configSampleClkTiming( analogOutTask2, "/Dev4/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
}


void AoSystem::writeDacs(UINT variation, UINT seqNum, bool loadSkip)
{
	std::vector<AoSnapshot>& snapshots = loadSkip ? loadSkipDacSnapshots(seqNum,variation) 
												   : dacSnapshots(seqNum,variation);
	std::array<std::vector<double>, 3>& finalData = loadSkip ? loadSkipDacFinalFormat(seqNum,variation)
															 : finalFormatDacData(seqNum,variation);
	if (snapshots.size() <= 1)
	{
		// need at least 2 events to run aoSys.
		return;
	}
	if (finalData[0].size() != 8 * snapshots.size() 
		 || finalData[1].size() != 8 * snapshots.size()
		 || finalData[2].size() != 8 * snapshots.size())
	{
		thrower ( "Data array size doesn't match the number of time slices in the experiment!" );
	}
	// Should probably run a check on samples written.
	int32 samplesWritten;
	//
	daqmx.writeAnalogF64( analogOutTask0, snapshots.size(), false, 0.01, DAQmx_Val_GroupByScanNumber, 
						  &finalData[0].front(), &samplesWritten );
	daqmx.writeAnalogF64( analogOutTask1, snapshots.size(), false, 0.01, DAQmx_Val_GroupByScanNumber, 
						  &finalData[1].front(), &samplesWritten );
	daqmx.writeAnalogF64( analogOutTask2, snapshots.size(), false, 0.01, DAQmx_Val_GroupByScanNumber, 
						  &finalData[2].front(), &samplesWritten );
}


void AoSystem::startDacs()
{
	daqmx.startTask( analogOutTask0 );
	daqmx.startTask( analogOutTask1 );
	daqmx.startTask( analogOutTask2 );
}


void AoSystem::makeFinalDataFormat(UINT variation, UINT seqNum)
{
	auto& finalNormal = finalFormatDacData(seqNum,variation);
	auto& finalLoadSkip = loadSkipDacFinalFormat(seqNum,variation);
	auto& normSnapshots = dacSnapshots(seqNum,variation);
	auto& loadSkipSnapshots = loadSkipDacSnapshots(seqNum,variation);

	for ( auto& data : finalNormal )
	{
		data.clear( );
	}
	for ( auto& data : finalLoadSkip )
	{
		data.clear( );
	}
	for (AoSnapshot snapshot : normSnapshots)
	{
		for ( auto dacInc : range( 24 ) )
		{
			finalNormal[dacInc / 8].push_back( snapshot.dacValues[dacInc] );
		}
	}
	for ( AoSnapshot snapshot : loadSkipSnapshots )
	{
		for ( auto dacInc : range( 24 ) )
		{
			finalLoadSkip[dacInc/8].push_back( snapshot.dacValues[dacInc] );
		}
	}
}


void AoSystem::handleDacScriptCommand( AoCommandForm command, std::string name, std::vector<UINT>& dacShadeLocations,
										std::vector<parameterType>& vars, DioSystem& ttls, UINT seqNum )
{
	if ( command.commandName != "dac:" && command.commandName != "dacarange:" && command.commandName != "daclinspace:" )
	{
		thrower ( "dac commandName not recognized!" );
	}
	if (!isValidDACName( name))
	{
		thrower ("the name " + name + " is not the name of a dac!");
	}
	// convert name to corresponding dac line.
	command.line = getDacIdentifier(name);
	if ( command.line == -1)
	{
		thrower ("the name " + name + " is not the name of a dac!");
	}
	dacShadeLocations.push_back( command.line );
	setDacCommandForm( command, seqNum );
}


int AoSystem::getDacIdentifier(std::string name)
{
	for (auto dacInc : range(outputs.size()))
	{
		auto& dacName = str(outputs[ dacInc ].info.name,13,false,true);
		// check names set by user and check standard names which are always acceptable
		if (name == dacName || name == "dac" + str ( dacInc ) )
		{
			return dacInc;
		}
	}
	// not an identifier.
	return -1;
}


void AoSystem::setMinMax(int dacNumber, double minv, double maxv)
{
	if (!(minv <= maxv))
	{
		thrower ("Min dac value must be less than max dac value.");
	}
	if (minv < -10 || minv > 10 || maxv < -10 || maxv > 10)
	{
		thrower ("Min and max dac values must be withing [-10,10].");
	}
	outputs[dacNumber].info.minVal = minv;
	outputs[ dacNumber ].info.maxVal = maxv;
}


std::pair<double, double> AoSystem::getDacRange(int dacNumber)
{
	return { outputs[ dacNumber ].info.minVal, outputs[ dacNumber ].info.maxVal };
}


void AoSystem::setName(int dacNumber, std::string name, cToolTips& toolTips, AuxiliaryWindow* master)
{
	outputs[ dacNumber ].setName ( name, toolTips, master );
}


std::string AoSystem::getNote ( int dacNumber )
{
	return outputs[dacNumber].info.note;
}


void AoSystem::setNote( int dacNum, std::string note, cToolTips& toolTips, AuxiliaryWindow* master )
{
	outputs[ dacNum ].setNote ( note, toolTips, master );
}


std::string AoSystem::getName(int dacNumber)
{
	return outputs[dacNumber].info.name;
}


HBRUSH AoSystem::handleColorMessage( CWnd* window, CDC* cDC)
{
	int controlID = GetDlgCtrlID(*window);
	for ( auto& out : outputs )
	{
		auto res = out.handleColorMessage ( controlID, window, cDC );
		if ( res != NULL )
		{
			return res;
		}
	}
	return NULL;
}


UINT AoSystem::getNumberOfDacs()
{
	return outputs.size ( );
}


double AoSystem::getDacValue(int dacNumber)
{
	return outputs[dacNumber].info.currVal;
}


void AoSystem::shadeDacs(std::vector<UINT>& dacShadeLocations)
{
	for ( auto shadeLoc : dacShadeLocations )
	{
		outputs[ shadeLoc ].shade ( );
	}
	for (auto& output : outputs)
	{
		output.disable ( );
	}
}


void AoSystem::unshadeDacs()
{
	for (auto& output : outputs)
	{
		output.unshade ( );
	}
}

