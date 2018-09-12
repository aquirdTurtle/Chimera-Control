#include "stdafx.h"
#include "AoSystem.h"
#include "AuxiliaryWindow.h"
#include "Version.h"
// for other ni stuff
#include "nidaqmx2.h"
#include "Thrower.h"
#include "range.h"
#include <boost/lexical_cast.hpp>

AoSystem::AoSystem(bool aoSafemode) : dacResolution(10.0 / pow(2, 16)), daqmx( aoSafemode )
{
	/// set some constants...
	// Both are 0-INDEXED. D16
	dacTriggerLine = { 3, 15 };
	for ( auto dacInc : range(dacValues.size()))
	{
		dacValues[dacInc] = 0;
		dacMinVals[dacInc] = -10;
		dacMaxVals[dacInc] = 10;
	}
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
		// unused at the moment.
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


std::vector<std::vector<std::vector<AoSnapshot>>> AoSystem::getSnapshots( )
{
	return dacSnapshots;
}

std::vector<std::vector<std::array<std::vector<double>, 3>>> AoSystem::getFinData( )
{
	return finalFormatDacData;
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


std::array<double, 24> AoSystem::getDacStatus()
{
	return dacValues;
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

void AoSystem::handleOpenConfig(std::ifstream& openFile, Version ver, DioSystem* ttls)
{
	ProfileSystem::checkDelimiterLine(openFile, "DACS");
	prepareForce( );
	std::vector<double> values(getNumberOfDacs());
	UINT dacInc = 0;
	for (auto& dac : values)
	{
		std::string dacString;
		openFile >> dacString;
		try
		{
			double dacValue = boost::lexical_cast<double>(dacString);
			prepareDacForceChange(dacInc, dacValue, ttls);
		}
		catch ( boost::bad_lexical_cast&)
		{
			throwNested("failed to convert dac value to voltage. string was " + dacString);
		}
		dacInc++;
	}
	ProfileSystem::checkDelimiterLine(openFile, "END_DACS");
}


void AoSystem::handleNewConfig( std::ofstream& newFile )
{
	newFile << "DACS\n";
	for ( UINT dacInc = 0; dacInc < getNumberOfDacs( ); dacInc++ )
	{
		newFile << 0 << " ";
	}
	newFile << "\nEND_DACS\n";
}


void AoSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "DACS\n";
	for (UINT dacInc = 0; dacInc < getNumberOfDacs(); dacInc++)
	{
		saveFile << getDacValue(dacInc) << " ";
	}
	saveFile << "\nEND_DACS\n";
}




std::string AoSystem::getDacSequenceMessage(UINT variation, UINT seqNum)
{
	std::string message;
	for ( auto snap : dacSnapshots[seqNum][variation] )
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



void AoSystem::handleEditChange(UINT dacNumber)
{
	if (dacNumber >= breakoutBoardEdits.size())
	{
		thrower ("attempted to handle dac edit change, but the dac number reported doesn't exist!");
	}
	CString text;
	breakoutBoardEdits[dacNumber].GetWindowTextA(text);
	bool matches = false;
	std::string textStr(text);
	try
	{
		if (roundToDacPrecision)
		{
			double roundNum = roundToDacResolution(dacValues[dacNumber]);
			if (fabs(roundToDacResolution(dacValues[dacNumber]) - boost::lexical_cast<double>(textStr)) < 1e-8)
			{
				matches = true;
			}
		}
		else
		{
			if (fabs(dacValues[dacNumber] - boost::lexical_cast<double>(str(text))) < 1e-8)
			{
				matches = true;
			}
		}
	}
	catch ( boost::bad_lexical_cast&){ /* failed to convert to double. Effectively, doesn't match. */ }
	if ( matches )
	{
		// mark this to change color.
		breakoutBoardEdits[dacNumber].colorState = 0;
		breakoutBoardEdits[dacNumber].RedrawWindow();
	}
	else
	{
		breakoutBoardEdits[dacNumber].colorState = 1;
		breakoutBoardEdits[dacNumber].RedrawWindow();
	}
}


bool AoSystem::isValidDACName(std::string name)
{
	for (UINT dacInc = 0; dacInc < getNumberOfDacs(); dacInc++)
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
	for (auto& control : dacLabels)
	{
		control.rearrange( width, height, fonts);
	}
	for (auto& control : breakoutBoardEdits)
	{
		control.rearrange( width, height, fonts);
	}
}


void AoSystem::setDefaultValue(UINT dacNum, double val)
{
	defaultVals[dacNum] = val;
}


double AoSystem::getDefaultValue(UINT dacNum)
{
	return defaultVals[dacNum];
}


// this function returns the end location of the set of controls. This can be used for the location for the next control beneath it.
void AoSystem::initialize(POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id, rgbMap rgbs)
{
	// title
	dacTitle.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	dacTitle.Create("DACS", WS_CHILD | WS_VISIBLE | SS_CENTER, dacTitle.sPos, master, id++);
	dacTitle.fontType = fontTypes::HeadingFont;
	// 
	dacSetButton.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 25};
	dacSetButton.Create( "Set New DAC Values", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
						 dacSetButton.sPos, master, ID_DAC_SET_BUTTON );
	dacSetButton.setToolTip("Press this button to attempt force all DAC values to the values currently recorded in the"
							 " edits below.", toolTips, master);
	//
	zeroDacsButton.sPos = { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 };
	zeroDacsButton.Create( "Zero Dacs", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, zeroDacsButton.sPos, master, IDC_ZERO_DACS );
	zeroDacsButton.setToolTip( "Press this button to set all dac values to zero.", toolTips, master );

	int collumnInc = 0;
	
	// there's a single label first, hence the +1.
	UINT dacInc = 0;
	for (auto& edit : breakoutBoardEdits)
	{
		if (dacInc == breakoutBoardEdits.size() / 3 || dacInc == 2 * breakoutBoardEdits.size() / 3)
		{
			collumnInc++;
			// go to second or third collumn
			pos.y -= 25 * breakoutBoardEdits.size( ) / 3;
		}
		edit.sPos = { pos.x + 20 + collumnInc * 160, pos.y, pos.x + 160 + collumnInc * 160, pos.y += 25 };
		edit.colorState = 0;
		edit.Create( WS_CHILD | WS_VISIBLE | WS_BORDER, edit.sPos, master, id++ );
		edit.SetWindowText( "0" );
		edit.setToolTip( dacNames[dacInc] + "\r\n" + dacNotes[dacInc], toolTips, master );
		dacInc++;
	}

	collumnInc = 0;
	pos.y -= 25 * breakoutBoardEdits.size( ) / 3;

	for ( UINT dacInc = 0; dacInc < dacLabels.size( ); dacInc++ )
	{
		if ( dacInc == dacLabels.size( ) / 3 || dacInc == 2 * dacLabels.size( ) / 3 )
		{
			collumnInc++;
			// go to second or third collumn
			pos.y -= 25 * dacLabels.size( ) / 3;
		}
		// create label
		dacLabels[dacInc].sPos = { pos.x + collumnInc * 160, pos.y, pos.x + 20 + collumnInc * 160, pos.y += 25 };
		dacLabels[dacInc].Create( cstr( dacInc ), WS_CHILD | WS_VISIBLE | SS_CENTER,
								  dacLabels[dacInc].sPos, master, ID_DAC_FIRST_EDIT + dacInc );
		dacLabels[dacInc].setToolTip( dacNames[ dacInc ] + "\r\n" + dacNotes[ dacInc ], toolTips, master );
	}
}


void AoSystem::handleRoundToDac( CMenu& menu )
{
	if ( roundToDacPrecision )
	{
		roundToDacPrecision = false;
		menu.CheckMenuItem( ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_UNCHECKED );
	}
	else
	{
		roundToDacPrecision = true;
		menu.CheckMenuItem( ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_CHECKED );
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
	for ( UINT dacInc = 0; dacInc < dacLabels.size( ); dacInc++ )
	{
		CString text;
		breakoutBoardEdits[dacInc].GetWindowTextA( text );
		try
		{
			vals[dacInc] = boost::lexical_cast<double>( str( text ) );
		}
		catch ( boost::bad_lexical_cast& )
		{
			if ( useDefault )
			{
				vals[dacInc] = 0;
			}
			else
			{
				throwNested( "value entered in DAC #" + str( dacInc ) + " (" + text.GetString( )
						 + ") failed to convert to a double!" );
			}
		}
		prepareDacForceChange( dacInc, vals[dacInc], ttls );
	}
	// wait until after all this to actually do this to make sure things get through okay.
	dacValues = vals;
	for ( UINT dacInc = 0; dacInc < dacLabels.size( ); dacInc++ )
	{
		breakoutBoardEdits[dacInc].colorState = 0;
		breakoutBoardEdits[dacInc].RedrawWindow( );
	}
}


void AoSystem::updateEdits( )
{
	for ( auto dacInc : range( dacValues.size( ) ) )
	{
		std::string valStr;
		if ( roundToDacPrecision )
		{
			valStr = str( roundToDacResolution( dacValues[dacInc] ), 13, true );
		}
		else
		{
			valStr = str( dacValues[dacInc] );
		}
		breakoutBoardEdits[dacInc].SetWindowTextA( cstr( valStr ) );
	}
}


void AoSystem::organizeDacCommands(UINT variation, UINT seqNum)
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<AoCommand>>> timeOrganizer;
	std::vector<AoCommand> tempEvents(dacCommandList[seqNum][variation]);
	// sort the events by time. using a lambda here.
	std::sort( tempEvents.begin(), tempEvents.end(), 
			   [](AoCommand a, AoCommand b){return a.time < b.time; });
	for (UINT commandInc = 0; commandInc < tempEvents.size(); commandInc++)
	{
		auto& command = tempEvents[commandInc];
		// because the events are sorted by time, the time organizer will already be sorted by time, and therefore I 
		// just need to check the back value's time. Check that the times are greater than a pico second apart. 
		// pretty arbitrary.
		if (commandInc == 0 || fabs( command.time - timeOrganizer.back().first) > 1e-9)
		{
			// new time
			timeOrganizer.push_back({ command.time, std::vector<AoCommand>({ command }) });
		}
		else
		{
			// old time
			timeOrganizer.back().second.push_back( command );
		}
	}
	/// make the snapshots
	if (timeOrganizer.size() == 0)
	{
		// no commands, that's fine.
		return;
	}
	auto& snap = dacSnapshots[seqNum][variation];
	snap.clear();
	// first copy the initial settings so that things that weren't changed remain unchanged.
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
	auto& snaps = dacSnapshots[seqNum][variation];
	auto& loadSkipSnaps = loadSkipDacSnapshots[seqNum][variation];
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
	if (dacSnapshots.size() != 0)
	{
		if (dacSnapshots.back().size() != 0)
		{
			if ( dacSnapshots.back( ).back( ).size( ) != 0 )
			{
				return dacSnapshots.back( ).back( ).back( ).dacValues;
			}
			else
			{
				thrower ( "No DAC Events" );
			}
		}
		else
		{
			thrower ( "No DAC Events" );
		}
	}
	else
	{
		thrower ("No DAC Events");
	}
}


std::array<std::string, 24> AoSystem::getAllNames()
{
	return dacNames;
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
	dacValues = status;
	// change the edits
	for (UINT dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		std::string valStr;
		if (roundToDacPrecision)
		{
			double val = roundToDacResolution(dacValues[dacInc]);
			valStr = str(val, 13, true);
		}
		else
		{
			valStr = str(dacValues[dacInc], 13, true);
		}
		breakoutBoardEdits[dacInc].SetWindowText(cstr(valStr));
		breakoutBoardEdits[dacInc].colorState = 0;
	}
}


double AoSystem::roundToDacResolution(double num)
{
	return long((num + dacResolution / 2) / dacResolution) * dacResolution;
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
		for ( auto& dacSeqInfo : dacSnapshots )
		{
			if ( dacSeqInfo.size( ) <= variation )
			{
				thrower ( "Attempted to use dac data from variation " + str( variation ) + ", which does not "
						 "exist!" );
			}
			for ( auto& snap : dacSeqInfo[variation] )
			{
				data->push_back( { runningSeqTime + snap.time, double( snap.dacValues[line] ), 0 } );
			}
			runningSeqTime += finTimes[seqInc][variation];
			seqInc++;
		}
	}
}


// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
// readable. I very rarely use things like this.
template<class T> using vec = std::vector<T>;

void AoSystem::interpretKey( std::vector<std::vector<parameterType>>& variables, std::string& warnings )
{
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
	dacCommandList = vec<vec<vec<AoCommand>>>( sequenceLength, vec<vec<AoCommand>>(variations) );
	dacSnapshots = vec<vec<vec<AoSnapshot>>>( sequenceLength, vec<vec<AoSnapshot>>(variations) );
	loadSkipDacSnapshots = vec<vec<vec<AoSnapshot>>>( sequenceLength, vec<vec<AoSnapshot>>( variations ) );
	finalFormatDacData = vec<vec<std::array<vec<double>, 3>>>( sequenceLength, 
															   vec<std::array<vec<double>, 3>>( variations ));
	loadSkipDacFinalFormat = vec<vec<std::array<vec<double>, 3>>>( sequenceLength,
															   vec<std::array<vec<double>, 3>>( variations ) );
	bool resolutionWarningPosted = false;
	bool nonIntegerWarningPosted = false;
	for ( auto seqInc : range( sequenceLength ) )
	{
		for ( UINT variationInc = 0; variationInc < variations; variationInc++ )
		{
			for ( UINT eventInc = 0; eventInc < dacCommandFormList[seqInc].size( ); eventInc++ )
			{
				AoCommand tempEvent;
				auto& formList = dacCommandFormList[seqInc][eventInc];
				auto& seqVariables = variables[seqInc];
				auto& cmdList = dacCommandList[seqInc][variationInc];
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
				if ( formList.commandName == "dac:" )
				{
					/// single point.
					////////////////
					// deal with value
					tempEvent.value = formList.finalVal.evaluate( seqVariables, variationInc );
					cmdList.push_back( tempEvent );
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
						warnings += "Warning: ramp increment of " + str( rampInc ) + " is below the resolution of the aoSys "
							"(which is 10/2^16 = " + str( 10.0 / pow( 2, 16 ) ) + "). It's likely taxing the system to "
							"calculate the ramp unnecessarily.\r\n";
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
				}
				else if ( formList.commandName == "daclinspace:" )
				{
					// interpret ramp time command. I need to know whether it's ramping or not.
					double rampTime = formList.rampTime.evaluate( seqVariables, variationInc );
					/// many points to be made.
					// convert initValue and finalValue to doubles to be used 
					double initValue, finalValue;
					UINT numSteps;
					initValue = formList.initVal.evaluate( seqVariables, variationInc );
					// deal with final value;
					finalValue = formList.finalVal.evaluate( seqVariables, variationInc );
					// deal with numPoints
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
	return dacSnapshots[seqNum][variation].size();
}


void AoSystem::checkTimingsWork(UINT variation, UINT seqInc)
{
	std::vector<double> times;
	// grab all the times.
	for (auto snapshot : dacSnapshots[seqInc][variation])
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
	return dacSnapshots[seqInc][variation].size();
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
void AoSystem::setDacTriggerEvents(DioSystem* ttls, UINT variation, UINT seqInc)
{
	for ( auto snapshot : dacSnapshots[seqInc][variation])
	{
		ttls->ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time, variation, seqInc);
		ttls->ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time + dacTriggerTime, variation, 
							seqInc );
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
	std::string valStr;
	if (roundToDacPrecision)
	{
		valStr = str(roundToDacResolution(voltage), 13);
	}
	else
	{
		valStr = str(voltage, 13);
	}
	if (valStr.find(".") != std::string::npos)
	{
		// then it's a double. kill extra zeros on the end.
		valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
	}
	//breakoutBoardEdits[line].SetWindowText(cstr(valStr));
	dacValues[line] = voltage;
	// I'm not sure it's necessary to go through the procedure of doing this and using the DIO to trigger the aoSys for a foce out. I'm guessing it's 
	// possible to tell the DAC to just immediately change without waiting for a trigger.
	setForceDacEvent( line, voltage, ttls, 0, 0 );
}


void AoSystem::checkValuesAgainstLimits(UINT variation, UINT seqNum)
{
	for (UINT line = 0; line < dacNames.size(); line++)
	{
		for (auto snapshot : dacSnapshots[seqNum][variation])
		{
			if (snapshot.dacValues[line] > dacMaxVals[line] || snapshot.dacValues[line] < dacMinVals[line])
			{
				thrower ("Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
						"value was " + str(snapshot.dacValues[line]) + ", while the minimum accepted value is " +
						str(dacMinVals[line]) + " and the maximum value is " + str(dacMaxVals[line]) + ". "
						"Change the min/max if you actually need to set this value.\r\n");
			}
		}
	}
}


void AoSystem::setForceDacEvent( int line, double val, DioSystem* ttls, UINT variation, UINT seqNum )
{
	if (val > dacMaxVals[line] || val < dacMinVals[line])
	{
		thrower ("Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
				"value was " + str(val) + ", while the minimum accepted value is " +
				str(dacMinVals[line]) + " and the maximum value is " + str(dacMaxVals[line]) + ". "
				"Change the min/max if you actually need to set this value.\r\n");
	}
	AoCommand eventInfo;
	eventInfo.line = line;
	eventInfo.time = 1;	
	eventInfo.value = val;
	dacCommandList[seqNum][variation].push_back( eventInfo );
	// important! need at least 2 states to run the dac board. can't just give it one value. This is how this was done in the VB code,
	// there might be better ways of dealing with this. 
	eventInfo.time = 10;
	dacCommandList[seqNum][variation].push_back( eventInfo );
	// you need to set up a corresponding pulse trigger to tell the aoSys to change the output at the correct time.
	ttls->ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, 1, 0, 0 );
	ttls->ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, 1 + dacTriggerTime, 0, 0 );
}


void AoSystem::prepareForce( )
{
	initializeDataObjects( 1, 1 );
}


void AoSystem::initializeDataObjects( UINT sequenceNum, UINT cmdNum )
{
	dacCommandFormList = vec<vec<AoCommandForm>>( sequenceNum, vec<AoCommandForm>( cmdNum ) );
	dacCommandList = vec<vec<vec<AoCommand>>>( sequenceNum, vec<vec<AoCommand>>( cmdNum ) );
	dacSnapshots = vec<vec<vec<AoSnapshot>>>( sequenceNum, vec<vec<AoSnapshot>>( cmdNum ) );
	loadSkipDacSnapshots = vec<vec<vec<AoSnapshot>>>( sequenceNum, vec<vec<AoSnapshot>>( cmdNum ) );
	finalFormatDacData = vec<vec<std::array<vec<double>, 3>>>( sequenceNum, vec<std::array<vec<double>, 3>>( cmdNum ) );
	loadSkipDacFinalFormat = vec<vec<std::array<vec<double>, 3>>>( sequenceNum, vec<std::array<vec<double>, 3>>( cmdNum ) );
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
		sampleNumber = loadSkipDacSnapshots[seqNum][variation].size( );
	}
	else
	{
		sampleNumber = dacSnapshots[seqNum][variation].size( );
	}
	daqmx.configSampleClkTiming( analogOutTask0, "/Dev2/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqmx.configSampleClkTiming( analogOutTask1, "/Dev3/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqmx.configSampleClkTiming( analogOutTask2, "/Dev4/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
}


void AoSystem::writeDacs(UINT variation, UINT seqNum, bool loadSkip)
{
	std::vector<AoSnapshot>& snapshots = loadSkip ? loadSkipDacSnapshots[seqNum][variation] 
												   : dacSnapshots[seqNum][variation];
	std::array<std::vector<double>, 3>& finalData = loadSkip ? loadSkipDacFinalFormat[seqNum][variation]
															 : finalFormatDacData[seqNum][variation];
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
	daqmx.writeAnalogF64( analogOutTask0, snapshots.size(), false, 0.01, DAQmx_Val_GroupByScanNumber, &finalData[0].front(),
					   &samplesWritten );
	daqmx.writeAnalogF64( analogOutTask1, snapshots.size(), false, 0.01, DAQmx_Val_GroupByScanNumber, &finalData[1].front(),
					   &samplesWritten );
	daqmx.writeAnalogF64( analogOutTask2, snapshots.size(), false, 0.01, DAQmx_Val_GroupByScanNumber, &finalData[2].front(),
					   &samplesWritten );
}


void AoSystem::startDacs()
{
	daqmx.startTask( analogOutTask0 );
	daqmx.startTask( analogOutTask1 );
	daqmx.startTask( analogOutTask2 );
}


void AoSystem::makeFinalDataFormat(UINT variation, UINT seqNum)
{
	auto& finalNormal = finalFormatDacData[seqNum][variation];
	auto& finalLoadSkip = loadSkipDacFinalFormat[seqNum][variation];
	auto& normSnapshots = dacSnapshots[seqNum][variation];
	auto& loadSkipSnapshots = loadSkipDacSnapshots[seqNum][variation];

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
										std::vector<parameterType>& vars, DioSystem* ttls, UINT seqNum )
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
	for (UINT dacInc = 0; dacInc < dacValues.size(); dacInc++)
	{
		// check names set by user.
		std::transform( dacNames[dacInc].begin(), dacNames[dacInc].end(), dacNames[dacInc].begin(), ::tolower );
		if (name == dacNames[dacInc])
		{
			return dacInc;
		}
		// check standard names which are always acceptable.
		if (name == "dac" + str(dacInc))
		{
			return dacInc;
		}
	}
	// not an identifier.
	return -1;
}

void AoSystem::setMinMax(int dacNumber, double min, double max)
{
	if (!(min <= max))
	{
		thrower ("Min dac value must be less than max dac value.");
	}
	if (min < -10 || min > 10 || max < -10 || max > 10)
	{
		thrower ("Min and max dac values must be withing [-10,10].");
	}
	dacMinVals[dacNumber] = min;
	dacMaxVals[dacNumber] = max;
}


std::pair<double, double> AoSystem::getDacRange(int dacNumber)
{
	return { dacMinVals[dacNumber], dacMaxVals[dacNumber] };
}


void AoSystem::setName(int dacNumber, std::string name, cToolTips& toolTips, AuxiliaryWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return; 
	}
	std::transform( name.begin(), name.end(), name.begin(), ::tolower );
	dacNames[dacNumber] = name;
	breakoutBoardEdits[dacNumber].setToolTip( dacNames[ dacNumber ] + "\r\n" + dacNotes[ dacNumber ], toolTips, master);
}

std::string AoSystem::getNote ( int dacNumber )
{
	return dacNotes[ dacNumber ];
}

void AoSystem::setNote( int dacNum, std::string note, cToolTips& toolTips, AuxiliaryWindow* master )
{
	dacNotes[ dacNum ] = note;
	breakoutBoardEdits[ dacNum ].setToolTip ( dacNames[ dacNum ] + "\r\n" + dacNotes[ dacNum ], toolTips, master );
}


std::string AoSystem::getName(int dacNumber)
{
	return dacNames[dacNumber];
}


HBRUSH AoSystem::handleColorMessage( CWnd* window, brushMap brushes, rgbMap rgbs, CDC* cDC)
{
	int controlID = GetDlgCtrlID(*window);
	if (controlID >= dacLabels[0].GetDlgCtrlID() && controlID <= dacLabels.back().GetDlgCtrlID() )
	{
		cDC->SetBkColor(rgbs["Medium Grey"]);
		cDC->SetTextColor(rgbs["Solarized Base1"]);
		return *brushes["Medium Grey"];
	}
	else if (controlID >= breakoutBoardEdits[0].GetDlgCtrlID() && controlID <= breakoutBoardEdits.back().GetDlgCtrlID())
	{
		int editNum = (controlID - breakoutBoardEdits[0].GetDlgCtrlID());
		if (breakoutBoardEdits[editNum].colorState == 0)
		{
			// default.
			cDC->SetTextColor(rgbs["Solarized Base2"]);
			cDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		else if (breakoutBoardEdits[editNum].colorState == 1)
		{
			// in this case, the actuall setting hasn't been changed despite the edit being updated.
			cDC->SetTextColor(rgbs["White"]);
			cDC->SetBkColor(rgbs["Red"]);
			return *brushes["Red"];
		}
		else if (breakoutBoardEdits[editNum].colorState == -1)
		{
			// in use during experiment.
			cDC->SetTextColor(rgbs["Black"]);
			cDC->SetBkColor(rgbs["White"]);
			return *brushes["White"];
		}
	}
	return NULL;
}


UINT AoSystem::getNumberOfDacs()
{
	return dacValues.size();
}


double AoSystem::getDacValue(int dacNumber)
{
	return dacValues[dacNumber];
}


void AoSystem::shadeDacs(std::vector<UINT>& dacShadeLocations)
{
	for (UINT shadeInc = 0; shadeInc < dacShadeLocations.size(); shadeInc++)
	{
		breakoutBoardEdits[dacShadeLocations[shadeInc]].colorState = -1;
		breakoutBoardEdits[dacShadeLocations[shadeInc]].SetReadOnly(true);
		breakoutBoardEdits[dacShadeLocations[shadeInc]].InvalidateRect( NULL );
		//breakoutBoardEdits[dacShadeLocations[shadeInc]].RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE );
	}
	for (auto& ctrl : breakoutBoardEdits)
	{
		ctrl.EnableWindow(0);
	}
}


void AoSystem::unshadeDacs()
{
	for (UINT shadeInc = 0; shadeInc < breakoutBoardEdits.size(); shadeInc++)
	{
		breakoutBoardEdits[shadeInc].EnableWindow();
		if (breakoutBoardEdits[shadeInc].colorState == -1)
		{
			breakoutBoardEdits[shadeInc].colorState = 0;
			breakoutBoardEdits[shadeInc].SetReadOnly(false);
			breakoutBoardEdits[shadeInc].RedrawWindow();
		}		
	}
}

