#include "stdafx.h"
#include "DAC_System.h"
#include "constants.h"
// for other ni stuff
#include "nidaqmx2.h"
#include "ExperimentManager.h"
#include "MasterWindow.h"

void DacSystem::abort()
{

}

std::string DacSystem::getDacSequenceMessage()
{
	std::string message;
	for ( auto snap : this->dacSnapshots )
	{
		std::string time = std::to_string( snap.time );
		time.erase( time.find_last_not_of( '0' ) + 1, std::string::npos );
		message += time + ":\r\n";
		int dacCount = 0;
		for ( auto val : snap.dacValues )
		{
			std::string volt = std::to_string( val );
			volt.erase( volt.find_last_not_of( '0' ) + 1, std::string::npos );
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

void DacSystem::daqCreateTask( const char* taskName, TaskHandle& handle )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxCreateTask( taskName, &handle );
		if ( result )
		{
			thrower( "daqCreateTask Failed! (" + std::to_string( result) + "): " 
					 + this->getErrorMessage( result ) );
		}
	}
	return;
}


void DacSystem::daqCreateAOVoltageChan( TaskHandle taskHandle, const char physicalChannel[],
										 const char nameToAssignToChannel[], float64 minVal, float64 maxVal, int32 units,
										 const char customScaleName[] )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxCreateAOVoltageChan( taskHandle, physicalChannel, nameToAssignToChannel, minVal, maxVal, 
											   units, customScaleName );
		if ( result )
		{
			thrower( "daqCreateAOVoltageChan Failed! (" + std::to_string( result ) + "): "
					 + this->getErrorMessage( result ) );
		}
	}
	return;
}


void DacSystem::daqCreateDIChan( TaskHandle taskHandle, const char lines[], const char nameToAssignToLines[],
								  int32 lineGrouping )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxCreateDIChan( taskHandle, lines, nameToAssignToLines, lineGrouping );
		if ( result )
		{
			thrower( "daqCreateDIChan Failed! (" + std::to_string( result ) + "): "
					 + this->getErrorMessage( result ) );
		}
	}
	return;
}


void DacSystem::daqStopTask( TaskHandle handle )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxStopTask(handle);
		// this function is currently meant to be silent.
		if ( result )
		{
			//thrower( "daqStopTask Failed! (" + std::to_string( result ) + "): "
			//		 + this->getErrorMessage( result ) );
			
		}
	}
}


void DacSystem::daqConfigSampleClkTiming( TaskHandle taskHandle, const char source[], float64 rate, int32 activeEdge,
									  int32 sampleMode, uInt64 sampsPerChan )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxCfgSampClkTiming( taskHandle, source, rate, activeEdge, sampleMode, sampsPerChan );
		if ( result )
		{
			thrower( "daqConfigSampleClkTiming Failed! (" + std::to_string( result ) + "): "
					 + this->getErrorMessage( result ) );
		}
	}
}


void DacSystem::daqWriteAnalogF64( TaskHandle handle, int32 numSampsPerChan, bool32 autoStart, float64 timeout,
									bool32 dataLayout, const float64 writeArray[], int32 *sampsPerChanWritten)
{
	if ( !DAQMX_SAFEMODE )
	{
		// the last argument must be null as of the writing of this wrapper. may be used in the future for something else.
		int result = DAQmxWriteAnalogF64( handle, numSampsPerChan, autoStart, timeout, dataLayout, writeArray, 
										  sampsPerChanWritten, NULL);
		if ( result )
		{
			thrower( "daqWriteAnalogF64 Failed! (" + std::to_string( result ) + "): "
					 + this->getErrorMessage( result ) );
		}
	}
}


void DacSystem::daqStartTask( TaskHandle handle )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxStartTask(handle);
		if ( result )
		{
			thrower( "daqStartTask Failed! (" + std::to_string( result ) + "): "
					 + this->getErrorMessage( result ) );
		}
	}
}




/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////




DacSystem::DacSystem()
{
	/// set some constants...
	// starts at 0... 
	// should change this later because only 1 is actually used...
	dacTriggerLines[0] = { 3, 14 };
	dacTriggerLines[1] = { 3, 15 };
	dacTriggerLines[2] = { 3, 13 };
	// paraphrasing adam...
	// "in 100 ns, i.e. 10 = 1 us. Dacs sample at 1 MHz, so 0.5 us is appropriate." .. no longer acurate...

	dacTriggerTime = 0.0005;
	try
	{
		// initialize tasks and chanells on the DACs
		long output = 0;
		long sampsPerChanWritten;

		// Create a task for each board
		// assume 3 boards, 8 channels per board. AMK 11/2010, modified for three from 2
		// task names are defined as public variables of type Long in TheMainProgram Declarations
		//This creates the task to output from DAC 2
		daqCreateTask( "Board 3 Dacs 16-23", this->staticDac2 );
		//This creates the task to output from DAC 1
		daqCreateTask( "Board 2 Dacs 8-15", this->staticDac1 );
		//This creates the task to output from DAC 0
		daqCreateTask( "Board 1 Dacs 0-7", this->staticDac0 );
		/// INPUTS
		//This creates a task to read in a digital input from DAC 0 on port 0 line 0
		daqCreateTask( "", this->digitalDAC_0_00 );
		// currently unused 11/08 (<-date copied from VB6. what is the actual full date though T.T)
		daqCreateTask( "", this->digitalDAC_0_01 );
		// Configure the output
		daqCreateAOVoltageChan( staticDac2, "PXI1Slot5/ao0:7", "StaticDAC_2", -10, 10, DAQmx_Val_Volts, "" );	
		//'Not sure why Tara and Debbie chose to switch the labels (for staticDac_0 -> StaticDac_1) here, but I'll stick with it to be consistent everywhere else in the program.AMK, 11 / 2010
		daqCreateAOVoltageChan( staticDac0, "PXI1Slot3/ao0:7", "StaticDAC_1", -10, 10, DAQmx_Val_Volts, "" );
		daqCreateAOVoltageChan( staticDac1, "PXI1Slot4/ao0:7", "StaticDAC_0", -10, 10, DAQmx_Val_Volts, "" );
		daqCreateDIChan( digitalDAC_0_00, "PXI1Slot3/port0/line0", "DIDAC_0", DAQmx_Val_ChanPerLine );
		// currently unused 11/08 (<-date copied from VB6. what is the actual full date though T.T)
		daqCreateDIChan( digitalDAC_0_01, "PXI1Slot3/port0/line1", "DIDAC_0", DAQmx_Val_ChanPerLine );
	}
	// I catch here because it's the constructor, and catching elsewhere is weird.
	catch (myException& exception)
	{
		errBox( exception.what() );
	}
	return;
}

std::string DacSystem::getDacSystemInfo()
{
	int32 answer = -1;
	// don't think I'm calling this right but there are no fucking examples.
	DAQmxGetDevProductCategory( "Board 1 Dacs 0-7", &answer );
	if ( answer == 12588 )
	{
		return "Dac System: Not connected?";
	}
	else
	{
		return "Dac System: Connected... " + std::to_string( answer );
	}
}

void DacSystem::handleEditChange(unsigned int dacNumber)
{
	if (dacNumber > this->breakoutBoardEdits.size())
	{
		thrower("ERROR: attempted to handle dac edit change, but the dac number reported doesn't exist!");
		return;
	}
	CString text;

	breakoutBoardEdits[dacNumber].GetWindowTextA(text);
	std::string current = std::to_string( this->dacValues[dacNumber] );
	if ( fabs( atof( current.c_str() ) - atof( text ) ) < 1e-8)
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
	return;
}

bool DacSystem::isValidDACName(std::string name)
{
	for (int dacInc = 0; dacInc < this->getNumberOfDACs(); dacInc++)
	{
		if (name == "dac" + std::to_string(dacInc))
		{
			return true;
		}
		else if (this->getDAC_Identifier(name) != -1)
		{
			return true;
		}
	}
	return false;
}

// this function returns the end location of the set of controls. This can be used for the location for the next control beneath it.
void DacSystem::initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id)
{
	// title
	dacTitle.position = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	dacTitle.ID = id++;
	dacTitle.Create("DACS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, dacTitle.position, master, dacTitle.ID);
	dacTitle.SetFont( CFont::FromHandle( sHeadingFont ) );
	pos.y += 25;
	// 
	dacSetButton.position = { pos.x, pos.y, pos.x + 240, pos.y + 25};
	dacSetButton.ID = id++;
	if ( dacSetButton.ID != ID_DAC_SET_BUTTON )
	{
		throw;
	}
	dacSetButton.Create("Set New DAC Values", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
						 dacSetButton.position, master, dacSetButton.ID);
	dacSetButton.setToolTip("Press this button to attempt force all DAC values to the values currently recorded in the"
							 " edits below.", toolTips, master);
	//
	zeroDacs.position = { pos.x + 240, pos.y, pos.x + 480, pos.y + 25 };
	zeroDacs.ID = id++;
	if ( zeroDacs.ID != IDC_ZERO_DACS )
	{
		throw;
	}
	zeroDacs.Create( "Zero Dacs", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, zeroDacs.position, master, 
					 zeroDacs.ID );
	zeroDacs.setToolTip( "Press this button to set all dac values to zero.", toolTips, master );

	pos.y += 25;
	int collumnInc = 0;
	
	// there's a single label first, hence the +1.
	if ( id + 1 != ID_DAC_FIRST_EDIT )
	{
		throw;
	}
	for ( auto& edit : breakoutBoardEdits )
	{
		edit.ID = id++;
	}
	for ( auto& label : dacLabels )
	{
		label.ID = id++;
	}
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{

		if (dacInc == dacLabels.size() / 3 || dacInc == 2 * dacLabels.size() / 3)
		{
			collumnInc++;
			// go to second or third collumn
			pos.y -= 25 * dacLabels.size() / 3;
		}
		// create label
		dacLabels[dacInc].position = { pos.x + collumnInc * 160, pos.y, pos.x + 20 + collumnInc * 160, pos.y + 25 };
		dacLabels[dacInc].Create( std::to_string( dacInc ).c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER,
								  dacLabels[dacInc].position, master, dacLabels[dacInc].ID );
		dacLabels[dacInc].setToolTip( this->dacNames[dacInc], toolTips, master );

		breakoutBoardEdits[dacInc].position = { pos.x + 20 + collumnInc * 160, pos.y, pos.x + 160 + collumnInc * 160,
												pos.y + 25 };
		breakoutBoardEdits[dacInc].colorState = 0;
		breakoutBoardEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_BORDER, breakoutBoardEdits[dacInc].position,
										   master, breakoutBoardEdits[dacInc].ID );
		breakoutBoardEdits[dacInc].setToolTip(this->dacNames[dacInc], toolTips, master);

		pos.y += 25;
	}
	return;
}

void DacSystem::handleButtonPress(TtlSystem* ttls)
{
	this->dacComplexEventsList.clear();

	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		CString text;
		breakoutBoardEdits[dacInc].GetWindowTextA(text);
		try
		{
			double tempDac = _tstof(text);
			this->prepareDacForceChange(dacInc, tempDac, ttls);
		}
		catch (std::exception& exc)
		{
			thrower("ERROR: value entered in DAC #" + std::to_string(dacInc) + " (" + text.GetString() + ") failed to convert to a double!");
		}
	}
	return;
}


void DacSystem::analyzeDAC_Commands()
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<DAC_IndividualEvent>>> timeOrganizer;
	/// organize all of the commands.
	for (int commandInc = 0; commandInc < dacIndividualEvents.size(); commandInc++)
	{
		// if it stays -1 after the following, it's a new time.
		int timeIndex = -1;
		for (int timeInc = 0; timeInc < timeOrganizer.size(); timeInc++)
		{
			if (dacIndividualEvents[commandInc].time == timeOrganizer[timeInc].first)
			{
				timeIndex = timeInc;
				break;
			}
		}
		if (timeIndex == -1)
		{
			std::vector<DAC_IndividualEvent> temp;
			temp.push_back(dacIndividualEvents[commandInc]);
			timeOrganizer.push_back({ dacIndividualEvents[commandInc].time, temp });
		}
		else
		{
			timeOrganizer[timeIndex].second.push_back(dacIndividualEvents[commandInc]);
		}
	}
	// this one will have all of the times ordered in sequence, in case the other doesn't.
	// first entry time, second entry event.
	std::vector<std::pair<double, std::vector<DAC_IndividualEvent>>> orderedOrganizer;
	/// order the commands.
	while (timeOrganizer.size() != 0)
	{
		// find the lowest value time
		int lowestTime = MAXINT32;
		unsigned int index;
		for (int commandInc = 0; commandInc < timeOrganizer.size(); commandInc++)
		{
			if (timeOrganizer[commandInc].first < lowestTime)
			{
				lowestTime = timeOrganizer[commandInc].first;
				index = commandInc;
			}
		}
		orderedOrganizer.push_back(timeOrganizer[index]);
		timeOrganizer.erase(timeOrganizer.begin() + index);
	}
	/// make the snapshots
	if (orderedOrganizer.size() == 0)
	{
		//thrower("ERROR: no dac commands...?");
		return;
	}
	this->dacSnapshots.clear();
	// first copy the initial settings so that things that weren't changed remain unchanged.
	this->dacSnapshots.push_back( { 0, this->dacValues } );
	// handle the zero case specially.
	// for every event at the first time...
	this->dacSnapshots.back().time = orderedOrganizer[0].first;
	for (int zeroInc = 0; zeroInc < orderedOrganizer[0].second.size(); zeroInc++)
	{		
		this->dacSnapshots.back().dacValues[orderedOrganizer[0].second[zeroInc].line] = orderedOrganizer[0].second[zeroInc].value;
		//... setting it to the command's state.
	}
	// 
	for (int commandInc = 1; commandInc < orderedOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		this->dacSnapshots.push_back(this->dacSnapshots.back());
		this->dacSnapshots.back().time = orderedOrganizer[commandInc].first;
		for (int zeroInc = 0; zeroInc < orderedOrganizer[commandInc].second.size(); zeroInc++)
		{
			// see description of this command above... update everything that changed at this time.
			this->dacSnapshots.back().dacValues[orderedOrganizer[commandInc].second[zeroInc].line] = orderedOrganizer[commandInc].second[zeroInc].value;
		}
	}
	// phew.

	return;
}


std::array<std::string, 24> DacSystem::getAllNames()
{
	return this->dacNames;
}


std::string DacSystem::getErrorMessage(int errorCode)
{
	char errorChars[2048];
	//'Get the actual error message. This is much surperior to getErrorString function below, much more info.
	DAQmxGetExtendedErrorInfo( errorChars, 2048 );

	//'Find out the error message length.
	//bufferSize = DAQmxGetErrorString(errorCode, 0, 0);
	//'Allocate enough space in the string.
	//errorChars = new char[bufferSize];
	//'Get the actual error message.
	//status = DAQmxGetErrorString(errorCode, errorChars, bufferSize);
	std::string errorString(errorChars);
	return errorString;
}



void DacSystem::interpretKey(key variationKey, unsigned int variationNumber, std::vector<variable> vars)
{
	this->dacIndividualEvents.clear();
	for (int eventInc = 0; eventInc < this->dacComplexEventsList.size(); eventInc++)
	{
		double value, time;
		DAC_IndividualEvent tempEvent;
		tempEvent.line = dacComplexEventsList[eventInc].line;

		//////////////////////////////////
		// Deal with time.
		if (dacComplexEventsList[eventInc].time.first.size() == 0)
		{
			// no variable portion of the time.
			tempEvent.time = dacComplexEventsList[eventInc].time.second;
		}
		else
		{
			double varTime;
			for (auto variableTimeString : dacComplexEventsList[eventInc].time.first)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{

						if (variableTimeString == vars[varInc].name)
						{
							isVar = true;
							break;
						}
				}
				if (!isVar)
				{
					thrower( "ERROR: the time string " + variableTimeString + " is not a variable!" );
					return;
				}
				varTime += variationKey[variableTimeString].first[variationNumber];
			}			
			tempEvent.time = varTime + dacComplexEventsList[eventInc].time.second;
		}
		// interpret ramp time command. I need to know whether it's ramping or not.
		double rampTime;
		try
		{
			rampTime = std::stod( dacComplexEventsList[eventInc].rampTime );
		}
		catch (std::invalid_argument&)
		{
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				if (vars[varInc].name == dacComplexEventsList[eventInc].rampTime)
				{
					rampTime = variationKey[dacComplexEventsList[eventInc].rampTime].first[variationNumber];
				}
			}
		}

		if (rampTime == 0)
		{
			/// single point.

			////////////////
			// deal with value
			try
			{
				tempEvent.value = std::stod(dacComplexEventsList[eventInc].finalVal);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (dacComplexEventsList[eventInc].finalVal == vars[varInc].name)
					{
						tempEvent.value = variationKey[dacComplexEventsList[eventInc].finalVal].first[variationNumber];
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the dac value " + dacComplexEventsList[eventInc].finalVal + " is not a variable or a double!");
					return;
				}
			}

			this->dacIndividualEvents.push_back(tempEvent);

		}
		else
		{
			/// many points to be made.
			// convert initValue and finalValue to doubles to be used 
			double initValue, finalValue, rampInc;
			try
			{
				initValue = std::stod(dacComplexEventsList[eventInc].initVal);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (dacComplexEventsList[eventInc].initVal == vars[varInc].name)
					{
						initValue = variationKey[dacComplexEventsList[eventInc].initVal].first[variationNumber];
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					thrower("ERROR: the dac initial value " + dacComplexEventsList[eventInc].initVal + " is not a variable or a double!");
					return;
				}
			}
			// deal with final value;
			try
			{
			finalValue = std::stod( dacComplexEventsList[eventInc].finalVal );
			}
			catch ( std::invalid_argument& )
			{
				bool isVar = false;
				for ( int varInc = 0; varInc < vars.size(); varInc++ )
				{
					if ( dacComplexEventsList[eventInc].finalVal == vars[varInc].name )
					{
						finalValue = variationKey[dacComplexEventsList[eventInc].finalVal].first[variationNumber];
						isVar = true;
						break;
					}
				}
				if ( !isVar )
				{
					thrower( "ERROR: the dac final value " + dacComplexEventsList[eventInc].finalVal + " is not a variable or a double!" );
					return;
				}
			}
			// deal with ramp inc
			try
			{
				rampInc = std::stod( dacComplexEventsList[eventInc].rampInc );
			}
			catch ( std::invalid_argument )
			{
				bool isVar = false;
				for ( int varInc = 0; varInc < vars.size(); varInc++ )
				{
					if ( dacComplexEventsList[eventInc].rampInc == vars[varInc].name )
					{
						rampInc = variationKey[dacComplexEventsList[eventInc].rampInc].first[variationNumber];
						isVar = true;
						break;
					}
				}
				if ( !isVar )
				{
					thrower( "ERROR: the dac ramp increment value " + dacComplexEventsList[eventInc].rampInc + " is not a variable or a double!" );
					return;
				}
			}
			// This might be the first not i++ usage of a for loop I've ever done... XD
			// calculate the time increment:
			double timeInc = rampTime / int( fabs(finalValue - initValue) / rampInc );
			double currentTime = tempEvent.time;
			if (initValue < finalValue)
			{
				for (double dacValue = initValue; dacValue < finalValue; dacValue += rampInc)
				{
					tempEvent.value = dacValue;
					tempEvent.time = currentTime;
					this->dacIndividualEvents.push_back( tempEvent );
					currentTime += timeInc;
				}
			}
			else
			{
				for (double dacValue = initValue; dacValue > finalValue; dacValue -= rampInc)
				{
					tempEvent.value = dacValue;
					tempEvent.time = currentTime;
					this->dacIndividualEvents.push_back( tempEvent );
					currentTime += timeInc;
				}
			}
			// and get the final value.
			tempEvent.value = finalValue;
			tempEvent.time = currentTime;
			this->dacIndividualEvents.push_back( tempEvent );
		}
	}
	return;
}

// note that this is not directly tied to changing any "current" parameters in the DacSystem object (it of course changes a list parameter). The 
// DacSystem object "current" parameters aren't updated to reflect an experiment, so if this is called for a force out, it should be called in conjuction
// with changing "currnet" parameters in the DacSystem object.
void DacSystem::setDacComplexEvent( int line, timeType time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc )
{
	DAC_ComplexEvent eventInfo;
	eventInfo.line = line;
	eventInfo.initVal = initVal;
	eventInfo.finalVal = finalVal;

	eventInfo.rampTime = rampTime;
	eventInfo.time = time;
	eventInfo.rampInc = rampInc;
	dacComplexEventsList.push_back( eventInfo );
	// you need to set up a corresponding trigger to tell the dacs to change the output at the correct time. 
	// This is done later on interpretation of ramps etc.
	return;
}


void DacSystem::setDacTtlTriggerEvents(TtlSystem* ttls)
{
	for ( auto snapshot : this->dacSnapshots)
	{
		// turn them on...
		timeType triggerOnTime;
		triggerOnTime.second = snapshot.time;
		ttls->ttlOn( dacTriggerLines[0].first, dacTriggerLines[0].second, triggerOnTime );
		ttls->ttlOn( dacTriggerLines[1].first, dacTriggerLines[1].second, triggerOnTime );
		ttls->ttlOn( dacTriggerLines[2].first, dacTriggerLines[2].second, triggerOnTime );

		// turn them off...
		timeType triggerOffTime;
		triggerOffTime.second = snapshot.time + dacTriggerTime;

		ttls->ttlOff( dacTriggerLines[0].first, dacTriggerLines[0].second, triggerOffTime );
		ttls->ttlOff( dacTriggerLines[1].first, dacTriggerLines[1].second, triggerOffTime );
		ttls->ttlOff( dacTriggerLines[2].first, dacTriggerLines[2].second, triggerOffTime );
	}
}

// this is a function called in preparation for forcing a dac change. Remember, you need to call ___ to actually change things.
void DacSystem::prepareDacForceChange(int line, double voltage, TtlSystem* ttls)
{
	// change parameters in the DacSystem object so that the object knows what the current settings are.
	std::string volt = std::to_string(voltage);
	volt.erase(volt.find_last_not_of('0') + 1, std::string::npos);
	this->breakoutBoardEdits[line].SetWindowText(volt.c_str());
	this->breakoutBoardEdits[line].colorState = 0;
	this->breakoutBoardEdits[line].RedrawWindow();
	this->dacValues[line] = voltage;
	// I'm not sure it's necessary to go through the procedure of doing this and using the DIO to trigger the dacs for a foce out. I'm guessing it's 
	// possible to tell the DAC to just immediately change without waiting for a trigger.
	this->setForceDacEvent( line, voltage, ttls );
	return;
}


void DacSystem::setForceDacEvent( int line, double val, TtlSystem* ttls )
{
	DAC_IndividualEvent eventInfo;
	eventInfo.line = line;
	eventInfo.time = 0;	
	eventInfo.value = val;
	dacIndividualEvents.push_back( eventInfo );
	// important! need at least 2 states to run the dac board. can't just give it one value. This is how this was done in the VB code,
	// there might be better ways of dealing with this. 
	eventInfo.time = 10;
	dacIndividualEvents.push_back( eventInfo );
	// you need to set up a corresponding trigger to tell the dacs to change the output at the correct time.
	// I don't understand why three triggers are sent though... Seems like there should only be one depending on the board. This actually feels like it should screw everything up...
	// maybe it's compensated for somewhere.
	// turn them on...
	ttls->ttlOnDirect( dacTriggerLines[0].first, dacTriggerLines[0].second, 0 );
	ttls->ttlOnDirect( dacTriggerLines[1].first, dacTriggerLines[1].second, 0 );
	ttls->ttlOnDirect( dacTriggerLines[2].first, dacTriggerLines[2].second, 0 );
	// turn them off...
	ttls->ttlOffDirect( dacTriggerLines[0].first, dacTriggerLines[0].second, dacTriggerTime );
	ttls->ttlOffDirect( dacTriggerLines[1].first, dacTriggerLines[1].second, dacTriggerTime );
	ttls->ttlOffDirect( dacTriggerLines[2].first, dacTriggerLines[2].second, dacTriggerTime );
}


void DacSystem::resetDACEvents()
{
	dacComplexEventsList.clear();
	dacIndividualEvents.clear();
	dacSnapshots.clear();
	return;
}


void DacSystem::stopDacs()
{
	daqStopTask( staticDac0 );
	daqStopTask( staticDac1 );
	daqStopTask( staticDac2 );
	return;
}


void DacSystem::resetDacs()
{
	return;
}


void DacSystem::configureClocks()
{	
	long sampleNumber = this->dacSnapshots.size();
	daqConfigSampleClkTiming( this->staticDac0, "/PXI1Slot3/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqConfigSampleClkTiming( this->staticDac1, "/PXI1Slot4/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqConfigSampleClkTiming( this->staticDac2, "/PXI1Slot5/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	return;
}


void DacSystem::writeDacs()
{
	if (dacSnapshots.size() <= 1)
	{
		// need at least 2 events to run dacs.
		return;
	}

	if (finalFormattedData[0].size() != 8 * dacSnapshots.size() || finalFormattedData[1].size() != 8 * dacSnapshots.size() 
		 || finalFormattedData[2].size() != 8 * dacSnapshots.size())
	{
		thrower( "Data array size doesn't match the number of time slices in the experiment!" );
		return;
	}
	bool32 nothing = NULL;
	int32 samplesWritten;
	int output;
	//
	daqWriteAnalogF64( staticDac0, dacSnapshots.size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, &finalFormattedData[0].front(), &samplesWritten );
	daqWriteAnalogF64( staticDac1, dacSnapshots.size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, &finalFormattedData[1].front(), &samplesWritten );
	daqWriteAnalogF64( staticDac2, dacSnapshots.size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, &finalFormattedData[2].front(), &samplesWritten );	
	return;
}


void DacSystem::startDacs()
{
	daqStartTask( staticDac0 );

	daqStartTask( staticDac1 );
	daqStartTask( staticDac2 );
	return;
}


void DacSystem::makeFinalDataFormat()
{
	this->finalFormattedData[0].clear();
	this->finalFormattedData[1].clear();
	this->finalFormattedData[2].clear();
	
	for (DAC_Snapshot snapshot : this->dacSnapshots)
	{
		for (int dacInc = 0; dacInc < 8; dacInc++)
		{
			this->finalFormattedData[0].push_back(snapshot.dacValues[dacInc]);
		}
		for (int dacInc = 8; dacInc < 16; dacInc++)
		{
			this->finalFormattedData[1].push_back(snapshot.dacValues[dacInc]);
		}
		for (int dacInc = 16; dacInc < 24; dacInc++)
		{
			this->finalFormattedData[2].push_back(snapshot.dacValues[dacInc]);
		}
	}
	return;
}


void DacSystem::handleDAC_ScriptCommand(timeType time, std::string name, std::string initVal, 
										 std::string finalVal, std::string rampTime, std::string rampInc, 
										 std::vector<unsigned int>& dacShadeLocations, std::vector<variable> vars, 
										 TtlSystem* ttls)
{
	double value;
	if (!this->isValidDACName(name))
	{
		thrower("ERROR: the name " + name + " is not the name of a dac!");
		return;
	}
	/// final value.
	try
	{
		value = std::stod( finalVal );
	}
	catch (std::invalid_argument& exception)
	{
		bool isVar = false;
		for (int varInc = 0; varInc < vars.size(); varInc++)
		{
			if (vars[varInc].name == finalVal)
			{
				isVar = true;
				break;
			}
		}
		if (!isVar)
		{
			thrower( "ERROR: tried and failed to convert " + finalVal + " to a double for a dac final voltage value. "
					 "It's also not a variable." );
			return;
		}
	}
	if (rampTime != "0")
	{
		// It's a ramp.
		// work with initVal;
		try
		{
			value = std::stod( initVal );
		}
		catch (std::invalid_argument& exception)
		{
			bool isVar = false;
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				if (vars[varInc].name == initVal)
				{
					isVar = true;
					break;
				}
			}
			if (!isVar)
			{
				thrower( "ERROR: tried and failed to convert value of \"" + initVal + "\" to a double for a dac "
						 "initial voltage value. It's also not a variable." );
				return;
			}
		}

		// work with finVal
		// work with the ramp time
		try
		{
			value = std::stod( rampTime );
		}
		catch (std::invalid_argument& exception)
		{
			bool isVar = false;
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				if (vars[varInc].name == rampTime)
				{
					isVar = true;
					break;
				}
			}
			if (!isVar)
			{
				thrower( "ERROR: tried and failed to convert " + rampTime + " to a double for a dac time value. It's "
						 "also not a variable." );
				return;
			}
		}
		// work with rampInc
		try
		{
			value = std::stod( rampInc );
		}
		catch (std::invalid_argument& exception)
		{
			bool isVar = false;
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				if (vars[varInc].name == rampInc)
				{
					isVar = true;
					break;
				}
			}
			if (!isVar)
			{
				thrower( "ERROR: tried and failed to convert " + rampInc + " to a double for a dac ramp increment "
						 "value. It's also not a variable." );
				return;
			}
		}
	}
	// convert name to corresponding dac line.
	int line = this->getDAC_Identifier(name);
	if (line == -1)
	{
		thrower("ERROR: the name " + name + " is not the name of a dac!");
		return;
	}
	dacShadeLocations.push_back(line);
	this->setDacComplexEvent(line, time, initVal, finalVal, rampTime, rampInc);
	return;
}


int DacSystem::getDAC_Identifier(std::string name)
{
	for (int dacInc = 0; dacInc < this->dacValues.size(); dacInc++)
	{
		// check names set by user.
		std::transform( dacNames[dacInc].begin(), dacNames[dacInc].end(), 
						dacNames[dacInc].begin(), ::tolower );
		if (name == dacNames[dacInc])
		{
			return dacInc;
		}
		// check standard names which are always acceptable.
		if (name == "dac" + std::to_string(dacInc))
		{
			return dacInc;
		}
	}
	// not an identifier.
	return -1;
}

void DacSystem::setName(int dacNumber, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return; 
	}
	std::transform( name.begin(), name.end(), name.begin(), ::tolower );
	this->dacNames[dacNumber] = name;
	this->breakoutBoardEdits[dacNumber].setToolTip(name, toolTips, master);
	return;
}

std::string DacSystem::getName(int dacNumber)
{
	return this->dacNames[dacNumber];
}

HBRUSH DacSystem::handleColorMessage(CWnd* window, std::unordered_map<std::string, HBRUSH> brushes, 
									  std::unordered_map<std::string, COLORREF> rgbs, CDC* cDC)
{
	DWORD controlID = GetDlgCtrlID(*window);
	if (controlID >= this->dacLabels[0].ID && controlID <= this->dacLabels.back().ID)
	{
		cDC->SetBkColor(rgbs["Medium Grey"]);
		cDC->SetTextColor(rgbs["Gold"]);
		return brushes["Medium Grey"];
	}
	else if (controlID >= this->breakoutBoardEdits[0].ID && controlID <= this->breakoutBoardEdits.back().ID)
	{
		int editNum = (controlID - this->breakoutBoardEdits[0].ID);
		if (this->breakoutBoardEdits[editNum].colorState == 0)
		{
			// default.
			cDC->SetTextColor(rgbs["White"]);
			cDC->SetBkColor(rgbs["Dark Grey"]);
			return brushes["Dark Grey"];
		}
		else if (this->breakoutBoardEdits[editNum].colorState == 1)
		{
			// in this case, the actuall setting hasn't been changed despite the edit being updated.
			//CString text;
			//breakoutBoardEdits[editNum].GetWindowTextA(text);
			//errBox( std::string(text ));
			cDC->SetTextColor(rgbs["White"]);
			cDC->SetBkColor(rgbs["Red"]);
			return brushes["Red"];
		}
		else if (this->breakoutBoardEdits[editNum].colorState == -1)
		{
			// in use during experiment.
			cDC->SetTextColor(rgbs["Black"]);
			cDC->SetBkColor(rgbs["White"]);
			return brushes["White"];
		}
	}
	else if (controlID == this->dacTitle.ID)
	{
		cDC->SetTextColor(rgbs["Gold"]);
		cDC->SetBkColor(rgbs["Medium Grey"]);
		return brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}

unsigned int DacSystem::getNumberOfDACs()
{
	return this->dacValues.size();
}

double DacSystem::getDAC_Value(int dacNumber)
{
	return this->dacValues[dacNumber];
}

void DacSystem::shadeDacs(std::vector<unsigned int>& dacShadeLocations)
{
	for (int shadeInc = 0; shadeInc < dacShadeLocations.size(); shadeInc++)
	{
		this->breakoutBoardEdits[dacShadeLocations[shadeInc]].colorState = -1;
		this->breakoutBoardEdits[dacShadeLocations[shadeInc]].SetReadOnly(true);
		this->breakoutBoardEdits[dacShadeLocations[shadeInc]].RedrawWindow();
	}
	return;
}
void DacSystem::unshadeDacs()
{
	for (int shadeInc = 0; shadeInc < this->breakoutBoardEdits.size(); shadeInc++)
	{
		if (this->breakoutBoardEdits[shadeInc].colorState == -1)
		{
			this->breakoutBoardEdits[shadeInc].colorState = 0;
			this->breakoutBoardEdits[shadeInc].SetReadOnly(false);
			this->breakoutBoardEdits[shadeInc].RedrawWindow();
		}
	}
	return;
}
