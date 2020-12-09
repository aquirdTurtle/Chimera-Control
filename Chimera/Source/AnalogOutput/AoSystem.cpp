// created by Mark O. Brown
#include "stdafx.h"
#include "AnalogOutput/AoSystem.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "ConfigurationSystems/Version.h"
// for other ni stuff
#include "nidaqmx2.h"
#include "GeneralUtilityFunctions/range.h"
#include "GeneralObjects/CodeTimer.h"
#include <boost/lexical_cast.hpp>
#include <ExperimentThread/ExpThreadWorker.h>

AoSystem::AoSystem(IChimeraQtWindow* parent, bool aoSafemode) : IChimeraSystem(parent), daqmx( aoSafemode ){
	/// set some constants...
	// Both are 0-INDEXED. D16
	dacTriggerLine = { 3, 15 };
	// paraphrasing adam...
	// Dacs sample at 1 MHz, so 0.5 us is appropriate.
	// in ms.
	// ?? I thought it was 10 MHz...
	dacTriggerTime = 0.0005;
	try	{
		// initialize tasks and chanells on the DACs
		long output = 0;
		// Create a task for each board
		// assume 3 boards, 8 channels per board. AMK 11/2010, modified for three from 2
		// task names are defined as public variables of type Long in TheMainProgram Declarations
		daqmx.createTask("Board 3 Dacs 16-23", analogOutTask2);
		daqmx.createTask("Board 2 Dacs 8-15", analogOutTask1);
		daqmx.createTask("Board 1 Dacs 0-7", analogOutTask0);
		// SLOT 3
		daqmx.createAoVoltageChan(analogOutTask0, cstr(board0Name + "/ao0:7"), "StaticDAC_1", -10, 10, DAQmx_Val_Volts, "");
		// SLOT 4
		daqmx.createAoVoltageChan(analogOutTask1, cstr(board1Name + "/ao0:7"), "StaticDAC_0", -10, 10, DAQmx_Val_Volts, "");
		// SLOT 5
		daqmx.createAoVoltageChan(analogOutTask2, cstr(board2Name + "/ao0:7"), "StaticDAC_2", -10, 10, DAQmx_Val_Volts, "");
		// This creates a task to readbtn in a digital input from DAC 0 on port 0 line 0
		daqmx.createTask("", digitalDac_0_00);
		daqmx.createTask("", digitalDac_0_01);
		/// unused at the moment.
		daqmx.createDiChan(digitalDac_0_00, cstr(board0Name + "/port0/line0"), "DIDAC_0", DAQmx_Val_ChanPerLine);
		daqmx.createDiChan(digitalDac_0_01, cstr (board0Name + "/port0/line1"), "DIDAC_0", DAQmx_Val_ChanPerLine);
		// new
	}
	// I catch here because it's the constructor, and catching elsewhere is weird.
	catch (ChimeraError& exception)
	{
		errBox(exception.trace());
		// should fail.
		throw;
	}
}


void AoSystem::standardNonExperiemntStartDacsSequence( ){
	updateEdits( );
	organizeDacCommands( 0 );
	makeFinalDataFormat( 0 );
	resetDacs (0, false);
}


void AoSystem::forceDacs( DoCore& ttls, DoSnapshot initSnap ){
	ttls.resetTtlEvents( );
	resetDacEvents( );
	handleSetDacsButtonPress( ttls );
	standardNonExperiemntStartDacsSequence( );
	ttls.standardNonExperimentStartDoSequence(initSnap);
	emit notification ("Forced Analog Output Values Complete.\n");
}


void AoSystem::zeroDacs( DoCore& ttls, DoSnapshot initSnap){
	resetDacEvents( );
	ttls.resetTtlEvents( );
	prepareForce( );
	ttls.prepareForce( );
	for ( int dacInc : range( 24 ) ){
		prepareDacForceChange( dacInc, 0, ttls );
	}
	standardNonExperiemntStartDacsSequence( );
	ttls.standardNonExperimentStartDoSequence( initSnap );
	emit notification ("Zero'd Analog Outputs.\n", 2);
}


std::array<AoInfo, 24> AoSystem::getDacInfo( ){
	std::array<AoInfo, 24> info;
	for ( auto dacNum : range(outputs.size()) ){
		info[ dacNum ] = outputs[ dacNum ].info;
	}
	return info;
}


void AoSystem::setSingleDac( unsigned dacNumber, double val, DoCore& ttls, DoSnapshot initSnap){
	ttls.resetTtlEvents( );
	resetDacEvents( );
	/// 
	dacCommandFormList.clear( );
	prepareForce( );
	ttls.prepareForce( );
	prepareDacForceChange( dacNumber, val, ttls );
	checkValuesAgainstLimits( 0 );
	///
	standardNonExperiemntStartDacsSequence( );
	ttls.standardNonExperimentStartDoSequence( initSnap );
	updateEdits( );
	emit notification ("Set single dac #" + qstr(dacNumber) + " to value " + qstr(val) + "\n", 2);
}


void AoSystem::handleOpenConfig(ConfigStream& openFile){
	unsigned dacInc = 0;
	if ( openFile.ver < Version ( "3.7" ) ){
		for ( auto i : range ( 24 ) ){
			std::string trash;
			openFile >> trash;
		}
	}
	emit notification ("AO system finished opening config.\n", 2);
}

void AoSystem::standardExperimentPrep ( unsigned variationInc, DoCore& ttls, std::vector<parameterType>& expParams, 
										double currLoadSkipTime ){
	organizeDacCommands (variationInc);
	setDacTriggerEvents (ttls, variationInc);
	findLoadSkipSnapshots (currLoadSkipTime, expParams, variationInc);
	makeFinalDataFormat (variationInc);
}

void AoSystem::handleSaveConfig(ConfigStream& saveFile){
	saveFile << "DACS\nEND_DACS\n";
}

std::string AoSystem::getDacSequenceMessage( unsigned variation ){
	std::string message;
	for ( auto snap : dacSnapshots(variation) ){
		std::string time = str( snap.time, 12, true );
		message += time + ":\r\n";
		int dacCount = 0;
		for ( auto val : snap.dacValues ){
			std::string volt = str( val, true );
			message += volt + ", ";
			dacCount++;
			if ( dacCount % 8 == 0 ){
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

void AoSystem::handleEditChange ( unsigned dacNumber ){
	if ( dacNumber >= outputs.size ( ) ){
		thrower ( "attempted to handle dac edit change, but the dac number reported doesn't exist!" );
	}
	outputs[ dacNumber ].handleEdit ( roundToDacPrecision );
}


bool AoSystem::isValidDACName(std::string name){
	for (auto dacInc : range(getNumberOfDacs()) ){
		if (name == "dac" + str(dacInc)){
			return true;
		}
		else if (getDacIdentifier(name) != -1){
			return true;
		}
	}
	return false;
}


void AoSystem::setDefaultValue(unsigned dacNum, double val){
	outputs[ dacNum ].info.defaultVal = val;
}


double AoSystem::getDefaultValue(unsigned dacNum){
	return outputs[dacNum].info.defaultVal;
}

// this function returns the end location of the set of controls. This can be used for the location for the next control beneath it.
void AoSystem::initialize(QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();
	// title
	dacTitle = new QLabel ("ANALOG OUTPUT", parent);
	dacTitle->setGeometry ({ QPoint{px, py},QPoint{px+480, py += 25} });

	dacSetButton = new CQPushButton ("Set New DAC Values", parent);
	dacSetButton->setGeometry ({ QPoint{px, py},QPoint{px+160, py+25} });
	dacSetButton->setToolTip("Press this button to attempt force all DAC values to the values currently recorded in the"
							 " edits below.");
	parent->connect (dacSetButton, &QPushButton::released, [parent]() {parent->auxWin->SetDacs (); });
	zeroDacsButton = new CQPushButton ("Zero DACs", parent);
	zeroDacsButton->setGeometry ({ QPoint{px+160, py},QPoint{px+320, py+25} });
	zeroDacsButton->setToolTip( "Press this button to set all dac values to zero." );
	parent->connect (zeroDacsButton, &QPushButton::released, [parent]() { parent->auxWin->zeroDacs(); });
	// 
	quickChange = new CQCheckBox ("Quick-Change", parent);
	quickChange->setGeometry ({ QPoint{px + 320, py},QPoint{px + 480, py += 25} });
	quickChange->setToolTip ( "With this checked, you can quickly change a DAC's value by using the arrow keys while "
							 "having the cursor before the desired digit selected in the DAC's edit.");

	int collumnInc = 0;
	
	unsigned dacInc = 0;
	for ( auto& out : outputs )	{
		if ( dacInc == outputs.size ( ) / 3 || dacInc == 2 * outputs.size ( ) / 3 )	{
			collumnInc++;
			// go to second or third collumn
			py -= 20 * outputs.size ( ) / 3;
			px += 160;
		}
		out.initialize ( pos, parent, dacInc );
		dacInc++;
	}
	px -= 320;
}

bool AoSystem::eventFilter (QObject* obj, QEvent* event){
	for (auto& out : outputs) {
		if ( out.eventFilter (obj, event) ) {
			return true;
		}
	}
	return false;
}

void AoSystem::handleRoundToDac( )
{
	if ( roundToDacPrecision )
	{
		roundToDacPrecision = false;
		//mainWin->checkAllMenus ( ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_UNCHECKED );
	}
	else
	{
		roundToDacPrecision = true;
		//mainWin->checkAllMenus ( ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_CHECKED );
	}
}


/*
 * get the text from every edit and prepare a change. If fails to get text from edit, if useDefalt this will set such
 * dacs to zero.
 */
void AoSystem::handleSetDacsButtonPress(DoCore& ttls, bool useDefault ){
	dacCommandFormList.clear( );
	prepareForce( );
	ttls.prepareForce( );
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
	}
}


void AoSystem::updateEdits( )
{
	for ( auto& dac : outputs )
	{
		dac.updateEdit ( roundToDacPrecision );
	}
}


void AoSystem::organizeDacCommands(unsigned variation)
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<AoCommand>>> timeOrganizer;
	std::vector<AoCommand> tempEvents(dacCommandList(variation));
	// sort the events by time. using a lambda here.
	std::sort( tempEvents.begin(), tempEvents.end(), 
			   [](AoCommand a, AoCommand b){ return a.time < b.time; });
	for (unsigned commandInc : range(tempEvents.size()))
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
	auto& snap = dacSnapshots(variation);
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


void AoSystem::findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, unsigned variation )
{
	// find the splitting time and set the loadSkip snapshots to have everything after that time.
	auto& snaps = dacSnapshots(variation);
	auto& loadSkipSnaps = loadSkipDacSnapshots(variation);
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
	auto numVar = dacSnapshots.getNumVariations ();
	if (numVar != 0)
	{
		if ( dacSnapshots( numVar-1 ).size( ) != 0 )
		{
			return dacSnapshots ( numVar - 1 ).back( ).dacValues;
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


void AoSystem::resetDacs (unsigned varInc, bool skipOption){
	stopDacs ();
	// it's important to grab the skipoption from input->skipNext only once because in principle
	// if the cruncher thread was running behind, it could change between writing and configuring the 
	// aoSys and configuring the TTLs, resulting in some funny behavior;
	configureClocks (varInc, skipOption);
	writeDacs (varInc, skipOption);
	startDacs ();
}

std::vector<std::vector<plotDataVec>> AoSystem::getPlotData( unsigned var ){
	std::vector<std::vector<plotDataVec>> dacData(3, std::vector<plotDataVec>(8));
	std::string message;
	// each element of dacData should be one ttl line.
	unsigned linesPerPlot = 24 / 3;

	for ( auto line : range( 24 ) )	{
		auto& data = dacData[line / linesPerPlot][line % linesPerPlot];
		data.clear( );
		if ( dacSnapshots.getNumVariations() <= var ){
			thrower ( "Attempted to use dac data from variation " + str( var ) + ", which does not exist!" );
		}

		for ( auto snapn : range(dacSnapshots(var).size()) ){
			if (snapn != 0) {
				data.push_back ({ dacSnapshots (var)[snapn].time, double (dacSnapshots (var)[snapn-1].dacValues[line]), 0 });
			}
			data.push_back ({ dacSnapshots (var)[snapn].time, double (dacSnapshots (var)[snapn].dacValues[line]), 0 });
		}
	}
	return dacData;
}


// an "alias template". effectively a local "using std::vector;" declaration. makes these declarations much more
// readable. I very rarely use things like this.
template<class T> using vec = std::vector<T>;

void AoSystem::calculateVariations( std::vector<parameterType>& params, ExpThreadWorker* threadworker, 
									std::vector<calResult> calibrations ){
	CodeTimer sTimer;
	sTimer.tick ( "Ao-Sys-Interpret-Start" );
	unsigned variations = params.size( ) == 0 ? 1 : params.front( ).keyValues.size( );
	if (variations == 0){
		variations = 1;
	}
	/// imporantly, this sizes the relevant structures.
	dacCommandList.uniformSizeReset (  variations );
	dacSnapshots.uniformSizeReset ( variations );
	loadSkipDacSnapshots.uniformSizeReset ( variations );
	finalFormatDacData.uniformSizeReset ( variations );
	loadSkipDacFinalFormat.uniformSizeReset ( variations );
	bool resolutionWarningPosted = false;
	bool nonIntegerWarningPosted = false;
	sTimer.tick ( "After-init" );
	for (auto variationInc : range(variations) ){
		if ( variationInc == 0 ){
			sTimer.tick ( "Variation-" + str ( variationInc ) + "-Start" );
		}
		auto& cmdList = dacCommandList(variationInc);
		for (auto eventInc : range( dacCommandFormList.size ( ) ) )	{
			AoCommand tempEvent;
			auto& formList = dacCommandFormList[eventInc];
			tempEvent.line = formList.line;
			// Deal with time.
			if ( formList.time.first.size( ) == 0 )	{
				// no variable portion of the time.
				tempEvent.time = formList.time.second;
			}
			else{
				double varTime = 0;
				for ( auto variableTimeString : formList.time.first ){
					varTime += variableTimeString.evaluate( params, variationInc, calibrations);
				}
				tempEvent.time = varTime + formList.time.second;
			}
			if ( variationInc == 0 ){
				sTimer.tick ( "Time-Handled" );
			}
			/// deal with command
			if ( formList.commandName == "dac:" ){
				/// single point.
				tempEvent.value = formList.finalVal.evaluate( params, variationInc, calibrations);
				if ( variationInc == 0 ){
					sTimer.tick ( "val-evaluated" );
				}
				cmdList.push_back( tempEvent );
				if ( variationInc == 0 ){
					sTimer.tick ( "Dac:-Handled" );
				}
			}
			else if ( formList.commandName == "dacarange:" ){
				// interpret ramp time command. I need to know whether it's ramping or not.
				double rampTime = formList.rampTime.evaluate( params, variationInc, calibrations);
				/// many points to be made.
				// convert initValue and finalValue to doubles to be used 
				double initValue, finalValue, rampInc;
				initValue = formList.initVal.evaluate( params, variationInc, calibrations);
				// deal with final value;
				finalValue = formList.finalVal.evaluate( params, variationInc, calibrations);
				// deal with ramp inc
				rampInc = formList.rampInc.evaluate( params, variationInc, calibrations);
				if ( rampInc < 10.0 / pow( 2, 16 ) && resolutionWarningPosted )	{
					resolutionWarningPosted = true;
					emit threadworker->warn (cstr("Warning: ramp increment of " + str (rampInc) + " in dac command number "
						+ str (eventInc) + " is below the resolution of the aoSys (which is 10/2^16 = "
						+ str (10.0 / pow (2, 16)) + "). These ramp points are unnecessary.\r\n"));
				}
				// This might be the first not i++ usage of a for loop I've ever done... XD
				// calculate the time increment:
				int steps = int( fabs( finalValue - initValue ) / rampInc + 0.5 );
				double stepsFloat = fabs( finalValue - initValue ) / rampInc;
				double diff = fabs( steps - fabs( finalValue - initValue ) / rampInc );
				if ( diff > 100 * DBL_EPSILON && nonIntegerWarningPosted ){
					nonIntegerWarningPosted = true;
					emit threadworker->warn (cstr ("Warning: Ideally your spacings for a dacArange would result in a non-integer number "
						"of steps. The code will attempt to compensate by making a last step to the final value which"
						" is not the same increment in voltage or time as the other steps to take the dac to the final"
						" value at the right time.\r\n"));
				}
				double timeInc = rampTime / steps;
				double initTime = tempEvent.time;
				double currentTime = tempEvent.time;
				// handle the two directions seperately.
				if ( initValue < finalValue ){
					for ( double dacValue = initValue; 
						(dacValue - finalValue) < -steps * 2 * DBL_EPSILON; dacValue += rampInc ){
						tempEvent.value = dacValue;
						tempEvent.time = currentTime;
						cmdList.push_back( tempEvent );
						currentTime += timeInc;
					}
				}
				else
				{
					for ( double dacValue = initValue; 
						dacValue - finalValue > 100 * DBL_EPSILON; dacValue -= rampInc ){
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
				if ( variationInc == 0 ){
					sTimer.tick ( "dacarange:-Handled" );
				}
			}
			else if ( formList.commandName == "daclinspace:" ){
				// interpret ramp time command. I need to know whether it's ramping or not.
				double rampTime = formList.rampTime.evaluate( params, variationInc, calibrations);
				/// many points to be made.
				double initValue, finalValue;
				unsigned numSteps;
				initValue = formList.initVal.evaluate( params, variationInc, calibrations);
				finalValue = formList.finalVal.evaluate(params, variationInc, calibrations);
				numSteps = formList.numSteps.evaluate(params, variationInc, calibrations);
				double rampInc = (finalValue - initValue) / numSteps;
				// this warning isn't actually very useful. very rare that actually run into issues with overtaxing ao 
				// or do systems like this and these circumstances often happen when something is ramped.
				/*if ( (fabs( rampInc ) < 10.0 / pow( 2, 16 )) && !resolutionWarningPosted ){
					resolutionWarningPosted = true;
					emit threadworker->warn (cstr ("Warning: numPoints of " + str (numSteps) + " results in a ramp increment of "
						+ str (rampInc) + " is below the resolution of the aoSys (which is 10/2^16 = "
						+ str (10.0 / pow (2, 16)) + "). It's likely taxing the system to "
						"calculate the ramp unnecessarily.\r\n"));
				}*/
				// This might be the first not i++ usage of a for loop I've ever done... XD
				// calculate the time increment:
				double timeInc = rampTime / numSteps;
				double initTime = tempEvent.time;
				double currentTime = tempEvent.time;
				double val = initValue;
				// handle the two directions seperately.
				for ( auto stepNum : range( numSteps ) ){
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
				if ( variationInc == 0 ){
					sTimer.tick ( "daclinspace:-Handled" );
				}
			}
			else{
				thrower ( "Unrecognized dac command name: " + formList.commandName );
			}
		}
	}
}


unsigned AoSystem::getNumberSnapshots(unsigned variation){
	return dacSnapshots(variation).size();
}


void AoSystem::checkTimingsWork(unsigned variation){
	std::vector<double> times;
	// grab all the times.
	for (auto snapshot : dacSnapshots(variation)){
		times.push_back(snapshot.time);
	}

	int count = 0;
	for (auto time : times){
		int countInner = 0;
		for (auto secondTime : times){
			// don't check against itself.
			if (count == countInner){
				countInner++;
				continue;
			}
			// can't trigger faster than the trigger time.
			if (fabs(time - secondTime) < dacTriggerTime){
				thrower ("timings are such that the dac system would have to get triggered too fast to follow the"
						" programming! ");
			}
			countInner++;
		}
		count++;
	}
}

unsigned long AoSystem::getNumberEvents(unsigned variation){
	return dacSnapshots(variation).size();
}


// note that this is not directly tied to changing any "current" parameters in the AoSystem object (it of course changes a list parameter). The 
// AoSystem object "current" parameters aren't updated to reflect an experiment, so if this is called for a force out, it should be called in conjuction
// with changing "currnet" parameters in the AoSystem object.
void AoSystem::setDacCommandForm( AoCommandForm command){
	dacCommandFormList.push_back( command );
	// you need to set up a corresponding trigger to tell the aoSys to change the output at the correct time. 
	// This is done later on interpretation of ramps etc.
}


// add a ttl trigger command for every unique dac snapshot.
// MUST interpret key for dac and organize dac commands before setting the trigger events.
void AoSystem::setDacTriggerEvents(DoCore& ttls, unsigned variation){
	for ( auto snapshot : dacSnapshots(variation)){
		ttls.ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time, variation );
		ttls.ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time + dacTriggerTime, variation );
	}
}


std::string AoSystem::getSystemInfo( ){
	return daqmx.getDacSystemInfo ({ board0Name, board1Name, board2Name } );
}


// this is a function called in preparation for forcing a dac change. Remember, you need to call ___ to actually change things.
void AoSystem::prepareDacForceChange(int line, double voltage, DoCore& ttls){
	// change parameters in the AoSystem object so that the object knows what the current settings are.
	//std::string volt = str(roundToDacResolution(voltage));
	std::string valStr = roundToDacPrecision? str ( AnalogOutput::roundToDacResolution ( voltage ), 13 ) : str ( voltage, 13 );
	if (valStr.find(".") != std::string::npos)	{
		// then it's a double. kill extra zeros on the end.
		valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
	}
	outputs[ line ].info.currVal = voltage;
	// I'm not sure it's necessary to go through the procedure of doing this and using the DIO to trigger the aoSys
	// for a foce out. I'm guessing it's possible to tell the DAC to just immediately change without waiting for a 
	// trigger.
	setForceDacEvent ( line, voltage, ttls, 0 );
}


void AoSystem::checkValuesAgainstLimits(unsigned variation){
	for (auto line : range(outputs.size())){
		for (auto snapshot : dacSnapshots(variation)){
			if (snapshot.dacValues[line] > outputs[line].info.maxVal || snapshot.dacValues[line] <outputs[ line ].info.minVal )	{
				thrower ("Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
						"value was " + str(snapshot.dacValues[line]) + ", while the minimum accepted value is " +
						str( outputs[ line ].info.minVal) + " and the maximum value is " + str( outputs[ line ].info.maxVal ) + ". "
						"Change the min/max if you actually need to set this value.\r\n");
			}
		}
	}
}


void AoSystem::setForceDacEvent( int line, double val, DoCore& ttls, unsigned variation ){
	if (val > outputs[ line ].info.maxVal || val < outputs[ line ].info.minVal ){
		thrower ("Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
				"value was " + str(val) + ", while the minimum accepted value is " +
				str( outputs[ line ].info.minVal ) + " and the maximum value is " + str( outputs[ line ].info.maxVal ) + ". "
				"Change the min/max if you actually need to set this value.\r\n");
	}
	AoCommand eventInfo;
	eventInfo.line = line;
	eventInfo.time = 1;	
	eventInfo.value = val;
	dacCommandList(variation).push_back( eventInfo );
	// important! need at least 2 states to run the dac board. can't just give it one value. This is how this was done in the VB code,
	// there might be better ways of dealing with this. 
	eventInfo.time = 10;
	dacCommandList(variation).push_back( eventInfo );
	// you need to set up a corresponding pulse trigger to tell the aoSys to change the output at the correct time.
	ttls.ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, 1, 0 );
	ttls.ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, 1 + dacTriggerTime, 0 );
}


ExpWrap<std::vector<AoSnapshot>> AoSystem::getSnapshots ( ){
	/* used by the unit testing suite. */
	return dacSnapshots;
}

ExpWrap<std::array<std::vector<double>, 3>> AoSystem::getFinData ( ){
	/* used by the unit testing suite. */
	return finalFormatDacData;
}


void AoSystem::prepareForce( ){
	initializeDataObjects( 1 );
}


void AoSystem::initializeDataObjects( unsigned cmdNum ){
	dacCommandFormList = vec<AoCommandForm>( cmdNum );
	dacCommandList.uniformSizeReset (  cmdNum );
	dacSnapshots.uniformSizeReset ( cmdNum );
	loadSkipDacSnapshots.uniformSizeReset ( cmdNum );
	finalFormatDacData.uniformSizeReset ( cmdNum );
	loadSkipDacFinalFormat.uniformSizeReset ( cmdNum );
}


void AoSystem::resetDacEvents(){
	initializeDataObjects( 0 );
}


void AoSystem::stopDacs(){
	daqmx.stopTask( analogOutTask0 );
	daqmx.stopTask( analogOutTask1 );
	daqmx.stopTask( analogOutTask2 );
}


void AoSystem::configureClocks(unsigned variation, bool loadSkip){
	long sampleNumber;
	if ( loadSkip ){
		sampleNumber = loadSkipDacSnapshots(variation).size( );
	}
	else{
		sampleNumber = dacSnapshots(variation).size( );
	}
	daqmx.configSampleClkTiming( analogOutTask0, cstr("/" + board0Name + "/PFI0"), 1000000, DAQmx_Val_Rising, 
								 DAQmx_Val_FiniteSamps, sampleNumber );
	daqmx.configSampleClkTiming( analogOutTask1, cstr("/" + board1Name + "/PFI0"), 1000000, DAQmx_Val_Rising, 
								 DAQmx_Val_FiniteSamps, sampleNumber );
	daqmx.configSampleClkTiming( analogOutTask2, cstr("/" + board2Name + "/PFI0"), 1000000, DAQmx_Val_Rising, 
								 DAQmx_Val_FiniteSamps, sampleNumber );
}


void AoSystem::writeDacs(unsigned variation, bool loadSkip){
	std::vector<AoSnapshot>& snapshots = loadSkip ? loadSkipDacSnapshots(variation) : dacSnapshots(variation);
	std::array<std::vector<double>, 3>& finalData = loadSkip ? loadSkipDacFinalFormat(variation)
															 : finalFormatDacData(variation);
	if (snapshots.size() <= 1){
		// need at least 2 events to run aoSys.
		return;
	}
	if (finalData[0].size() != 8 * snapshots.size() 
		 || finalData[1].size() != 8 * snapshots.size()
		 || finalData[2].size() != 8 * snapshots.size()){
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


void AoSystem::startDacs(){
	daqmx.startTask( analogOutTask0 );
	daqmx.startTask( analogOutTask1 );
	daqmx.startTask( analogOutTask2 );
}


void AoSystem::makeFinalDataFormat(unsigned variation){
	auto& finalNormal = finalFormatDacData(variation);
	auto& finalLoadSkip = loadSkipDacFinalFormat(variation);
	auto& normSnapshots = dacSnapshots(variation);
	auto& loadSkipSnapshots = loadSkipDacSnapshots(variation);

	for ( auto& data : finalNormal ){
		data.clear( );
	}
	for ( auto& data : finalLoadSkip ){
		data.clear( );
	}
	for (AoSnapshot snapshot : normSnapshots){
		for ( auto dacInc : range( 24 ) ){
			finalNormal[dacInc / 8].push_back( snapshot.dacValues[dacInc] );
		}
	}
	for ( AoSnapshot snapshot : loadSkipSnapshots ){
		for ( auto dacInc : range( 24 ) ){
			finalLoadSkip[dacInc/8].push_back( snapshot.dacValues[dacInc] );
		}
	}
}


void AoSystem::handleDacScriptCommand( AoCommandForm command, std::string name, std::vector<parameterType>& vars, 
									   DoCore& ttls ){
	if ( command.commandName != "dac:" && command.commandName != "dacarange:" && command.commandName != "daclinspace:" ){
		thrower ( "dac commandName not recognized!" );
	}
	if ( !isValidDACName( name ) ){
		thrower ("the name " + name + " is not the name of a dac!");
	}
	// convert name to corresponding dac line.
	command.line = getDacIdentifier( name );
	if ( command.line == -1){
		thrower ("the name " + name + " is not the name of a dac!");
	}
	setDacCommandForm( command );
}


int AoSystem::getDacIdentifier(std::string name){
	for (auto dacInc : range(outputs.size())){
		auto& dacName = str(outputs[ dacInc ].info.name,13,false,true);
		// check names set by user and check standard names which are always acceptable
		if (name == dacName || name == "dac" + str ( dacInc ) ){
			return dacInc;
		}
	}
	// not an identifier.
	return -1;
}

/* only handles basic names like dac5.*/
int AoSystem::getBasicDacIdentifier (std::string name){
	for (auto dacInc : range (24)){
		if (name == "dac" + str (dacInc)){
			return dacInc;
		}
	}
	// not an identifier.
	return -1;
}

void AoSystem::setMinMax(int dacNumber, double minv, double maxv){
	if (!(minv <= maxv)){
		thrower ("Min dac value must be less than max dac value.");
	}
	if (minv < -10 || minv > 10 || maxv < -10 || maxv > 10)	{
		thrower ("Min and max dac values must be withing [-10,10].");
	}
	outputs[dacNumber].info.minVal = minv;
	outputs[ dacNumber ].info.maxVal = maxv;
}


std::pair<double, double> AoSystem::getDacRange(int dacNumber){
	return { outputs[ dacNumber ].info.minVal, outputs[ dacNumber ].info.maxVal };
}


void AoSystem::setName(int dacNumber, std::string name){
	outputs[ dacNumber ].setName ( name );
}


std::string AoSystem::getNote ( int dacNumber ){
	return outputs[dacNumber].info.note;
}


void AoSystem::setNote( int dacNum, std::string note ){
	outputs[ dacNum ].setNote ( note );
}


std::string AoSystem::getName(int dacNumber){
	return outputs[dacNumber].info.name;
}


unsigned AoSystem::getNumberOfDacs(){
	return outputs.size ( );
}


double AoSystem::getDacValue(int dacNumber){
	return outputs[dacNumber].info.currVal;
}



