#include "stdafx.h"
#include "DacSystem.h"
#include "DeviceWindow.h"
// for other ni stuff
#include "nidaqmx2.h"

std::array<double, 24> DacSystem::getDacStatus()
{
	return dacValues;
}


void DacSystem::handleOpenConfig(std::ifstream& openFile, double version, TtlSystem* ttls)
{
	ProfileSystem::checkDelimiterLine(openFile, "DACS");

	std::vector<double> dacValues(getNumberOfDacs());
	UINT dacInc = 0;
	for (auto& dac : dacValues)
	{
		std::string dacString;
		openFile >> dacString;
		try
		{
			double dacValue = std::stod(dacString);
			prepareDacForceChange(dacInc, dacValue, ttls);
		}
		catch (std::invalid_argument& exception)
		{
			thrower("ERROR: failed to convert dac value to voltage. string was " + dacString);
		}
		dacInc++;
	}
	ProfileSystem::checkDelimiterLine(openFile, "END_DACS");
}


void DacSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "DACS\n";
	for (int dacInc = 0; dacInc < getNumberOfDacs(); dacInc++)
	{
		saveFile << getDacValue(dacInc) << "\n";
	}
	saveFile << "END_DACS\n";
}


void DacSystem::abort()
{
	// TODO...?
}

std::string DacSystem::getDacSequenceMessage(UINT var)
{
	std::string message;
	for ( auto snap : dacSnapshots[var] )
	{
		std::string time = str( snap.time );
		time.erase( time.find_last_not_of( '0' ) + 1, std::string::npos );
		message += time + ":\r\n";
		int dacCount = 0;
		for ( auto val : snap.dacValues )
		{
			std::string volt = str( val );
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
			thrower( "daqCreateTask Failed! (" + str( result) + "): " 
					 + getErrorMessage( result ) );
		}
	}
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
			thrower( "daqCreateAOVoltageChan Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DacSystem::daqCreateDIChan( TaskHandle taskHandle, const char lines[], const char nameToAssignToLines[],
								  int32 lineGrouping )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxCreateDIChan( taskHandle, lines, nameToAssignToLines, lineGrouping );
		if ( result )
		{
			thrower( "daqCreateDIChan Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DacSystem::daqStopTask( TaskHandle handle )
{
	if ( !DAQMX_SAFEMODE )
	{
		int result = DAQmxStopTask(handle);
		// this function is currently meant to be silent.
		if ( result )
		{
			//thrower( "daqStopTask Failed! (" + str( result ) + "): "
			//		 + getErrorMessage( result ) );
			
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
			thrower( "daqConfigSampleClkTiming Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
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
			thrower( "daqWriteAnalogF64 Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
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
			thrower( "daqStartTask Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DacSystem::DacSystem() : dacResolution(10.0 / pow(2,16))
{
	/// set some constants...
	// Both are 0-INDEXED. D16
	dacTriggerLine = { 3, 15 };
	// paraphrasing adam...
	// Dacs sample at 1 MHz, so 0.5 us is appropriate.
	// in ms.
	// ?? I thought it was 10 MHz...
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
		daqCreateTask( "Board 3 Dacs 16-23", staticDac2 );
		//This creates the task to output from DAC 1
		daqCreateTask( "Board 2 Dacs 8-15", staticDac1 );
		//This creates the task to output from DAC 0
		daqCreateTask( "Board 1 Dacs 0-7", staticDac0 );
		/// INPUTS
		//This creates a task to read in a digital input from DAC 0 on port 0 line 0
		daqCreateTask( "", digitalDac_0_00 );
		// currently unused 11/08 (<-date copied from VB6. what is the actual full date though T.T)
		daqCreateTask( "", digitalDac_0_01 );
		// Configure the output
		daqCreateAOVoltageChan( staticDac2, "PXI1Slot5/ao0:7", "StaticDAC_2", -10, 10, DAQmx_Val_Volts, "" );	
		//
		daqCreateAOVoltageChan( staticDac0, "PXI1Slot3/ao0:7", "StaticDAC_1", -10, 10, DAQmx_Val_Volts, "" );
		daqCreateAOVoltageChan( staticDac1, "PXI1Slot4/ao0:7", "StaticDAC_0", -10, 10, DAQmx_Val_Volts, "" );
		daqCreateDIChan( digitalDac_0_00, "PXI1Slot3/port0/line0", "DIDAC_0", DAQmx_Val_ChanPerLine );
		// currently unused 11/08 (<-date copied from VB6. what is the actual full date though T.T)
		daqCreateDIChan( digitalDac_0_01, "PXI1Slot3/port0/line1", "DIDAC_0", DAQmx_Val_ChanPerLine );
	}
	// I catch here because it's the constructor, and catching elsewhere is weird.
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
}


std::string DacSystem::getDacSystemInfo()
{
	int32 answer = -1;
	int32 errCode = DAQmxGetDevProductCategory( "Board 1 Dacs 0-7", &answer );
	
	// TODO: interpret the number which is the answer to the query.
	if (errCode != 0)
	{
		std::string err = getErrorMessage(0);
		return "DAC System: Error! " + err;
	}
	else if ( answer == 12588 )
	{
		return "Dac System: Unknown Category?";
	}
	else if (answer == -1)
	{
		return "Dac System: no response... " + str(answer);
	}
	else
	{
		return "Dac System: Connected... result = " + str( answer );
	}
}


void DacSystem::handleEditChange(UINT dacNumber)
{
	if (dacNumber >= breakoutBoardEdits.size())
	{
		thrower("ERROR: attempted to handle dac edit change, but the dac number reported doesn't exist!");
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
			if (fabs(roundToDacResolution(dacValues[dacNumber]) - std::stod(textStr)) < 1e-8)
			{
				matches = true;
			}
		}
		else
		{
			if (fabs(dacValues[dacNumber] - std::stod(str(text))) < 1e-8)
			{
				matches = true;
			}
		}
	}
	catch (std::invalid_argument&){ /* failed to convert to double. Effectively, doesn't match. */ }
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


bool DacSystem::isValidDACName(std::string name)
{
	for (int dacInc = 0; dacInc < getNumberOfDacs(); dacInc++)
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

void DacSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	dacTitle.rearrange("", "", width, height, fonts);
	dacSetButton.rearrange("", "", width, height, fonts);
	zeroDacs.rearrange("", "", width, height, fonts);
	for (auto& control : dacLabels)
	{
		control.rearrange("", "", width, height, fonts);
	}
	for (auto& control : breakoutBoardEdits)
	{
		control.rearrange("", "", width, height, fonts);
	}
}


void DacSystem::setDefaultValue(UINT dacNum, double val)
{
	defaultVals[dacNum] = val;
}


double DacSystem::getDefaultValue(UINT dacNum)
{
	return defaultVals[dacNum];
}


// this function returns the end location of the set of controls. This can be used for the location for the next control beneath it.
void DacSystem::initialize(POINT& pos, cToolTips& toolTips, DeviceWindow* master, int& id)
{
	// title
	dacTitle.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	dacTitle.Create("DACS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, dacTitle.sPos, master, id++);
	dacTitle.fontType = HeadingFont;
	// 
	dacSetButton.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 25};
	dacSetButton.Create( "Set New DAC Values", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
						 dacSetButton.sPos, master, id++ );
	idVerify(dacSetButton, ID_DAC_SET_BUTTON);
	dacSetButton.setToolTip("Press this button to attempt force all DAC values to the values currently recorded in the"
							 " edits below.", toolTips, master);
	//
	zeroDacs.sPos = { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 };
	zeroDacs.Create( "Zero Dacs", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, zeroDacs.sPos, master, id++ );
	idVerify(zeroDacs, IDC_ZERO_DACS);
	zeroDacs.setToolTip( "Press this button to set all dac values to zero.", toolTips, master );
	int collumnInc = 0;
	
	// there's a single label first, hence the +1.
	for (int dacInc = 0; dacInc < breakoutBoardEdits.size(); dacInc++)
	{
		if (dacInc == breakoutBoardEdits.size() / 3 || dacInc == 2 * breakoutBoardEdits.size() / 3)
		{
			collumnInc++;
			// go to second or third collumn
			pos.y -= 25 * breakoutBoardEdits.size() / 3;
		}

		breakoutBoardEdits[dacInc].sPos = { pos.x + 20 + collumnInc * 160, pos.y, pos.x + 160 + collumnInc * 160,
												pos.y += 25 };
		breakoutBoardEdits[dacInc].colorState = 0;
		breakoutBoardEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_BORDER, breakoutBoardEdits[dacInc].sPos,
										   master, id++ );
		breakoutBoardEdits[dacInc].SetWindowText("0");
		breakoutBoardEdits[dacInc].setToolTip(dacNames[dacInc], toolTips, master);
	}

	collumnInc = 0;
	pos.y -= 25 * breakoutBoardEdits.size() / 3;

	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		if (dacInc == dacLabels.size() / 3 || dacInc == 2 * dacLabels.size() / 3)
		{
			collumnInc++;
			// go to second or third collumn
			pos.y -= 25 * dacLabels.size() / 3;
		}
		// create label
		dacLabels[dacInc].sPos = { pos.x + collumnInc * 160, pos.y, pos.x + 20 + collumnInc * 160, pos.y += 25 };
		dacLabels[dacInc].Create(cstr(dacInc), WS_CHILD | WS_VISIBLE | SS_CENTER,
								 dacLabels[dacInc].sPos, master, id++);
		dacLabels[dacInc].setToolTip(dacNames[dacInc], toolTips, master);
	}

	idVerify(breakoutBoardEdits.front(), ID_DAC_FIRST_EDIT);
}


void DacSystem::handleRoundToDac(CMenu& menu)
{
	if (roundToDacPrecision)
	{
		roundToDacPrecision = false;
		menu.CheckMenuItem(ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_UNCHECKED);
	}
	else
	{
		roundToDacPrecision = true;
		menu.CheckMenuItem(ID_MASTER_ROUNDDACVALUESTODACPRECISION, MF_CHECKED);
	}
}


/*
 * get the text from every edit and prepare a change.
 */
void DacSystem::handleButtonPress(TtlSystem* ttls)
{
	dacComplexEventsList.clear();
	prepareForce();
	ttls->prepareForce();
	std::array<double, 24> vals;
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		CString text;
		breakoutBoardEdits[dacInc].GetWindowTextA(text);
		try
		{
			vals[dacInc] = std::stod(str(text));
			std::string valStr;
			if (roundToDacPrecision)
			{
				valStr = str(roundToDacResolution(vals[dacInc]), 12, true);
			}
			else
			{
				valStr = str(vals[dacInc]);
			}
			breakoutBoardEdits[dacInc].SetWindowTextA(cstr(valStr));
			prepareDacForceChange(dacInc, vals[dacInc], ttls);
		}
		catch (std::invalid_argument& err)
		{
			thrower("ERROR: value entered in DAC #" + str(dacInc) + " (" + text.GetString() + ") failed to convert to a double!");
		}
	}
	// wait until after all this to actually do this to make sure things get through okay.
	dacValues = vals;
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		breakoutBoardEdits[dacInc].colorState = 0;
		breakoutBoardEdits[dacInc].RedrawWindow();
	}
}


void DacSystem::analyzeDacCommands(UINT var)
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<DacIndividualEvent>>> timeOrganizer;
	std::vector<DacIndividualEvent> tempEvents(dacIndividualEvents[var]);
	// sort the events by time. using a lambda
	std::sort( tempEvents.begin(), tempEvents.end(), 
			   [](DacIndividualEvent a, DacIndividualEvent b){return a.time < b.time; });
	for (int commandInc = 0; commandInc < tempEvents.size(); commandInc++)
	{
		// because the events are sorted by time, the time organizer will already be sorted by time, and therefore I 
		// just need to check the back value's time.
		if (commandInc == 0 || fabs(tempEvents[commandInc].time - timeOrganizer.back().first) > 2 * DBL_EPSILON)
		{
			// new time
			timeOrganizer.push_back({ tempEvents[commandInc].time,
									std::vector<DacIndividualEvent>({ tempEvents[commandInc] }) });
		}
		else
		{
			// old time
			timeOrganizer.back().second.push_back(tempEvents[commandInc]);
		}
	}
	/// make the snapshots
	if (timeOrganizer.size() == 0)
	{
		//thrower("ERROR: no dac commands...?");
		return;
	}
	dacSnapshots[var].clear();
	// first copy the initial settings so that things that weren't changed remain unchanged.
	dacSnapshots[var].push_back({ 0, dacValues });
	for (int commandInc = 0; commandInc < timeOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		dacSnapshots[var].push_back(dacSnapshots[var].back());
		dacSnapshots[var].back().time = timeOrganizer[commandInc].first;
		for (int zeroInc = 0; zeroInc < timeOrganizer[commandInc].second.size(); zeroInc++)
		{
			// see description of this command above... update everything that changed at this time.
			dacSnapshots[var].back().dacValues[timeOrganizer[commandInc].second[zeroInc].line]
				= timeOrganizer[commandInc].second[zeroInc].value;
		}
	}
}


std::array<double, 24> DacSystem::getFinalSnapshot()
{
	if (dacSnapshots.size() != 0)
	{
		if (dacSnapshots.back().size() != 0)
		{
			return dacSnapshots.back().back().dacValues;
		}
		else
		{
			thrower("No DAC Events");
		}
	}
	else
	{
		thrower("No DAC Events");
	}
}


std::array<std::string, 24> DacSystem::getAllNames()
{
	return dacNames;
}

/*
 * IMPORTANT: this does not actually change any of the outputs of the board. It is meant to be called when things have
 * happened such that the control doesn't know what it's own status is, e.g. at the end of an experiment, since the 
 * program doesn't change it's internal memory of all of the status of the dacs as the experiment runs. (it can't, 
 * besides it would intensive to keep track of that in real time).
 */
void DacSystem::setDacStatusNoForceOut(std::array<double, 24> status)
{
	// set the internal values
	dacValues = status;
	// change the edits
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		std::string valStr;
		if (roundToDacPrecision)
		{
			double val = roundToDacResolution(dacValues[dacInc]);
			valStr = str(val, 12, true);
		}
		else
		{
			valStr = str(dacValues[dacInc], 12, true);
		}
		breakoutBoardEdits[dacInc].SetWindowText(cstr(valStr));
		breakoutBoardEdits[dacInc].colorState = 0;
	}
}


double DacSystem::roundToDacResolution(double num)
{
	return long((num + dacResolution / 2) / dacResolution) * dacResolution;
}


std::string DacSystem::getErrorMessage(int errorCode)
{
	char errorChars[2048];
	// Get the actual error message. This is much surperior to getErrorString function.
	DAQmxGetExtendedErrorInfo( errorChars, 2048 );
	std::string errorString(errorChars);
	return errorString;
}


void DacSystem::prepareForce()
{
	dacIndividualEvents.resize(1);
	dacSnapshots.resize(1);
	finalFormattedData.resize(1);
}


void DacSystem::interpretKey( key variationKey, std::vector<variable>& vars, std::string& warnings )
{
	UINT variations;
	if (vars.size() == 0)
	{
		variations = 1;
	}
	else
	{
		variations = variationKey[vars[0].name].first.size();
	}
	/// imporantly, this sizes the relevant structures.
	dacIndividualEvents.clear();
	dacIndividualEvents.resize(variations);
	dacSnapshots.clear();
	dacSnapshots.resize(variations);
	finalFormattedData.clear();
	finalFormattedData.resize(variations);
	for (UINT var = 0; var < variations; var++)
	{
	//
		for (int eventInc = 0; eventInc < dacComplexEventsList.size(); eventInc++)
		{
			double value, time;
			DacIndividualEvent tempEvent;
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
				double varTime = 0;
				for (auto variableTimeString : dacComplexEventsList[eventInc].time.first)
				{
					varTime += reduce(variableTimeString, variationKey, var, vars);
					//varTime += variationKey[variableTimeString].first[var];
				}
				tempEvent.time = varTime + dacComplexEventsList[eventInc].time.second;
			}
			// interpret ramp time command. I need to know whether it's ramping or not.
			double rampTime = reduce(dacComplexEventsList[eventInc].rampTime, variationKey, var, vars);
			if (rampTime == 0)
			{
				/// single point.
				////////////////
				// deal with value
				tempEvent.value = reduce(dacComplexEventsList[eventInc].finalVal, variationKey, var, vars);
				dacIndividualEvents[var].push_back(tempEvent);
			}
			else
			{
				/// many points to be made.
				// convert initValue and finalValue to doubles to be used 
				double initValue, finalValue, rampInc;
				initValue = reduce(dacComplexEventsList[eventInc].initVal, variationKey, var, vars);
				// deal with final value;
				finalValue = reduce(dacComplexEventsList[eventInc].finalVal, variationKey, var, vars);
				// deal with ramp inc
				rampInc = reduce(dacComplexEventsList[eventInc].rampInc, variationKey, var, vars);
				if (rampInc < 10.0 / pow(2, 16))
				{
					warnings += "Warning: ramp increment of " + str(rampInc) + " is below the resolution of the dacs (which"
								" is 10/2^16 = " + str(10.0 / pow(2, 16)) + "). It's likely taxing the system to calculate the ramp "
								"unnecessarily.\r\n";
				}
				// This might be the first not i++ usage of a for loop I've ever done... XD
				// calculate the time increment:
				int steps = int(fabs(finalValue - initValue) / rampInc + 0.5);
				double stepsFloat = fabs(finalValue - initValue) / rampInc;
				double diff = fabs(steps - fabs(finalValue - initValue) / rampInc);
				if (diff > 100 * DBL_EPSILON)
				{
					warnings += "Warning: Ideally your spacings for a dacramp would result in a non-integer number of steps."
						" The code will attempt to compensate by making a last step to the final value which is not the"
						" same increment in voltage or time as the other steps to take the dac to the final value at the"
						" right time.\r\n";
				}
				double timeInc = rampTime / steps;
				// 0.017543859649122806
				// 0.017241379310344827
				double initTime = tempEvent.time;
				double currentTime = tempEvent.time;
				// handle the two directions seperately.
				if (initValue < finalValue)
				{
					for (double dacValue = initValue; (dacValue - finalValue) < -steps * 2 * DBL_EPSILON; dacValue += rampInc)
					{
						tempEvent.value = dacValue;
						tempEvent.time = currentTime;
						dacIndividualEvents[var].push_back(tempEvent);
						currentTime += timeInc;
					}
				}
				else
				{
					for (double dacValue = initValue; dacValue - finalValue > 100 * DBL_EPSILON; dacValue -= rampInc)
					{
						tempEvent.value = dacValue;
						tempEvent.time = currentTime;
						dacIndividualEvents[var].push_back(tempEvent);
						currentTime += timeInc;
					}
				}
				// and get the final value.
				tempEvent.value = finalValue;
				tempEvent.time = initTime + rampTime;
				dacIndividualEvents[var].push_back(tempEvent);
			}
		}
	}
}


UINT DacSystem::getNumberSnapshots(UINT var)
{
	return dacSnapshots[var].size();
}

void DacSystem::checkTimingsWork(UINT var)
{
	std::vector<double> times;
	// grab all the times.
	for (auto snapshot : dacSnapshots[var])
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
				thrower("ERROR: timings are such that the dac system would have to get triggered too fast to follow the"
						" programming! ");
			}
			countInner++;
		}
		count++;
	}
}

ULONG DacSystem::getNumberEvents(UINT var)
{
	return dacSnapshots[var].size();
}


// note that this is not directly tied to changing any "current" parameters in the DacSystem object (it of course changes a list parameter). The 
// DacSystem object "current" parameters aren't updated to reflect an experiment, so if this is called for a force out, it should be called in conjuction
// with changing "currnet" parameters in the DacSystem object.
void DacSystem::setDacComplexEvent( int line, timeType time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc )
{
	DacComplexEvent eventInfo;
	eventInfo.line = line;
	eventInfo.initVal = initVal;
	eventInfo.finalVal = finalVal;

	eventInfo.rampTime = rampTime;
	eventInfo.time = time;
	eventInfo.rampInc = rampInc;
	dacComplexEventsList.push_back( eventInfo );
	// you need to set up a corresponding trigger to tell the dacs to change the output at the correct time. 
	// This is done later on interpretation of ramps etc.
}


// add a ttl trigger event for every unique dac snapshot.
void DacSystem::setDacTriggerEvents(TtlSystem* ttls, UINT var)
{
	for ( auto snapshot : dacSnapshots[var])
	{
		// turn them on...
		ttls->ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time, var);
		// turn them off...
		ttls->ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, snapshot.time + dacTriggerTime, var);
	}
}


// this is a function called in preparation for forcing a dac change. Remember, you need to call ___ to actually change things.
void DacSystem::prepareDacForceChange(int line, double voltage, TtlSystem* ttls)
{
	// change parameters in the DacSystem object so that the object knows what the current settings are.
	//std::string volt = str(roundToDacResolution(voltage));
	std::string valStr;
	if (roundToDacPrecision)
	{
		valStr = str(roundToDacResolution(voltage), 12);
	}
	else
	{
		valStr = str(voltage, 12);
	}
	if (valStr.find(".") != std::string::npos)
	{
		// then it's a double. kill extra zeros on the end.
		valStr.erase(valStr.find_last_not_of('0') + 1, std::string::npos);
	}
	breakoutBoardEdits[line].SetWindowText(cstr(valStr));
	// I'm not sure it's necessary to go through the procedure of doing this and using the DIO to trigger the dacs for a foce out. I'm guessing it's 
	// possible to tell the DAC to just immediately change without waiting for a trigger.
	setForceDacEvent( line, voltage, ttls, 0 );
}


void DacSystem::checkValuesAgainstLimits(UINT var)
{
	for (int line = 0; line < dacNames.size(); line++)
	{
		for (auto snapshot : dacSnapshots[var])
		{
			if (snapshot.dacValues[line] > dacMaxVals[line] || snapshot.dacValues[line] < dacMinVals[line])
			{
				thrower("ERROR: Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
						"value was " + str(snapshot.dacValues[line]) + ", while the minimum accepted value is " +
						str(dacMinVals[line]) + " and the maximum value is " + str(dacMaxVals[line]) + ". "
						"Change the min/max if you actually need to set this value.\r\n");
			}
		}
	}
}


void DacSystem::setForceDacEvent( int line, double val, TtlSystem* ttls, UINT var )
{
	if (val > dacMaxVals[line] || val < dacMinVals[line])
	{
		thrower("ERROR: Attempted to set Dac" + str(line) + " value outside min/max range for this line. The "
				"value was " + str(val) + ", while the minimum accepted value is " +
				str(dacMinVals[line]) + " and the maximum value is " + str(dacMaxVals[line]) + ". "
				"Change the min/max if you actually need to set this value.\r\n");
	}
	DacIndividualEvent eventInfo;
	eventInfo.line = line;
	eventInfo.time = 0;	
	eventInfo.value = val;
	dacIndividualEvents[var].push_back( eventInfo );
	// important! need at least 2 states to run the dac board. can't just give it one value. This is how this was done in the VB code,
	// there might be better ways of dealing with this. 
	eventInfo.time = 10;
	dacIndividualEvents[var].push_back( eventInfo );
	// you need to set up a corresponding pulse trigger to tell the dacs to change the output at the correct time.
	ttls->ttlOnDirect( dacTriggerLine.first, dacTriggerLine.second, 0, 0 );
	ttls->ttlOffDirect( dacTriggerLine.first, dacTriggerLine.second, dacTriggerTime, 0 );
}


void DacSystem::resetDacEvents()
{
	dacComplexEventsList.clear();
	dacIndividualEvents.clear();
	dacSnapshots.clear();
}


void DacSystem::stopDacs()
{
	daqStopTask( staticDac0 );
	daqStopTask( staticDac1 );
	daqStopTask( staticDac2 );
}


void DacSystem::configureClocks(UINT var)
{	
	long sampleNumber = dacSnapshots[var].size();
	daqConfigSampleClkTiming( staticDac0, "/PXI1Slot3/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqConfigSampleClkTiming( staticDac1, "/PXI1Slot4/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
	daqConfigSampleClkTiming( staticDac2, "/PXI1Slot5/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber );
}


void DacSystem::writeDacs(UINT var)
{
	if (dacSnapshots[var].size() <= 1)
	{
		// need at least 2 events to run dacs.
		return;
	}

	if (finalFormattedData[var][0].size() != 8 * dacSnapshots[var].size() || finalFormattedData[var][1].size() != 8 * dacSnapshots[var].size()
		 || finalFormattedData[var][2].size() != 8 * dacSnapshots[var].size())
	{
		thrower( "Data array size doesn't match the number of time slices in the experiment!" );
	}

	int32 samplesWritten;
	int output;
	//
	daqWriteAnalogF64( staticDac0, dacSnapshots[var].size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, 
					  &finalFormattedData[var][0].front(), &samplesWritten );
	daqWriteAnalogF64( staticDac1, dacSnapshots[var].size(), false, 0.0001, DAQmx_Val_GroupByScanNumber,
					  &finalFormattedData[var][1].front(), &samplesWritten );
	daqWriteAnalogF64( staticDac2, dacSnapshots[var].size(), false, 0.0001, DAQmx_Val_GroupByScanNumber,
					  &finalFormattedData[var][2].front(), &samplesWritten );	
}


void DacSystem::startDacs()
{
	daqStartTask( staticDac0 );
	daqStartTask( staticDac1 );
	daqStartTask( staticDac2 );
}


void DacSystem::makeFinalDataFormat(UINT var)
{
	finalFormattedData[var][0].clear();
	finalFormattedData[var][1].clear();
	finalFormattedData[var][2].clear();
	
	for (DacSnapshot snapshot : dacSnapshots[var])
	{
		for (int dacInc = 0; dacInc < 8; dacInc++)
		{
			finalFormattedData[var][0].push_back(snapshot.dacValues[dacInc]);
		}
		for (int dacInc = 8; dacInc < 16; dacInc++)
		{
			finalFormattedData[var][1].push_back(snapshot.dacValues[dacInc]);
		}
		for (int dacInc = 16; dacInc < 24; dacInc++)
		{
			finalFormattedData[var][2].push_back(snapshot.dacValues[dacInc]);
		}
	}
}


void DacSystem::handleDacScriptCommand( timeType time, std::string name, std::string initVal, 
										 std::string finalVal, std::string rampTime, std::string rampInc, 
										 std::vector<UINT>& dacShadeLocations, std::vector<variable>& vars, 
										 TtlSystem* ttls )
{
	double value;
	if (!isValidDACName(name))
	{
		thrower("ERROR: the name " + name + " is not the name of a dac!");
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
				vars[varInc].active = true;
				isVar = true;
				break;
			}
		}
		if (!isVar)
		{
			thrower( "ERROR: tried and failed to convert " + finalVal + " to a double for a dac final voltage value. "
					 "It's also not a variable." );
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
			}
		}
	}
	// convert name to corresponding dac line.
	int line = getDacIdentifier(name);
	if (line == -1)
	{
		thrower("ERROR: the name " + name + " is not the name of a dac!");
	}
	dacShadeLocations.push_back(line);
	setDacComplexEvent(line, time, initVal, finalVal, rampTime, rampInc);
}


int DacSystem::getDacIdentifier(std::string name)
{
	for (int dacInc = 0; dacInc < dacValues.size(); dacInc++)
	{
		// check names set by user.
		std::transform( dacNames[dacInc].begin(), dacNames[dacInc].end(), 
						dacNames[dacInc].begin(), ::tolower );
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

void DacSystem::setMinMax(int dacNumber, double min, double max)
{
	if (!(min <= max))
	{
		thrower("ERROR: Min dac value must be less than max dac value.");
	}
	if (min < -10 || min > 10 || max < -10 || max > 10)
	{
		thrower("ERROR: Min and max dac values must be withing [-10,10].");
	}
	dacMinVals[dacNumber] = min;
	dacMaxVals[dacNumber] = max;
}


std::pair<double, double> DacSystem::getDacRange(int dacNumber)
{
	return { dacMinVals[dacNumber], dacMaxVals[dacNumber] };
}


void DacSystem::setName(int dacNumber, std::string name, cToolTips& toolTips, DeviceWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return; 
	}
	std::transform( name.begin(), name.end(), name.begin(), ::tolower );
	dacNames[dacNumber] = name;
	breakoutBoardEdits[dacNumber].setToolTip(name, toolTips, master);
}


std::string DacSystem::getName(int dacNumber)
{
	return dacNames[dacNumber];
}


HBRUSH DacSystem::handleColorMessage( CWnd* window, brushMap brushes, rgbMap rgbs, CDC* cDC)
{
	DWORD controlID = GetDlgCtrlID(*window);
	if (controlID >= dacLabels[0].GetDlgCtrlID() && controlID <= dacLabels.back().GetDlgCtrlID() )
	{
		cDC->SetBkColor(rgbs["Medium Grey"]);
		cDC->SetTextColor(rgbs["White"]);
		return *brushes["Medium Grey"];
	}
	else if (controlID >= breakoutBoardEdits[0].GetDlgCtrlID() && controlID <= breakoutBoardEdits.back().GetDlgCtrlID())
	{
		int editNum = (controlID - breakoutBoardEdits[0].GetDlgCtrlID());
		if (breakoutBoardEdits[editNum].colorState == 0)
		{
			// default.
			cDC->SetTextColor(rgbs["White"]);
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
	else if (controlID == dacTitle.GetDlgCtrlID())
	{
		cDC->SetTextColor(rgbs["Gold"]);
		cDC->SetBkColor(rgbs["Medium Grey"]);
		return *brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


UINT DacSystem::getNumberOfDacs()
{
	return dacValues.size();
}


double DacSystem::getDacValue(int dacNumber)
{
	return dacValues[dacNumber];
}


void DacSystem::shadeDacs(std::vector<UINT>& dacShadeLocations)
{
	for (int shadeInc = 0; shadeInc < dacShadeLocations.size(); shadeInc++)
	{
		breakoutBoardEdits[dacShadeLocations[shadeInc]].colorState = -1;
		breakoutBoardEdits[dacShadeLocations[shadeInc]].SetReadOnly(true);
		breakoutBoardEdits[dacShadeLocations[shadeInc]].RedrawWindow();
	}
	for (auto& ctrl : breakoutBoardEdits)
	{
		ctrl.EnableWindow(0);
	}
}


void DacSystem::unshadeDacs()
{
	for (int shadeInc = 0; shadeInc < breakoutBoardEdits.size(); shadeInc++)
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

