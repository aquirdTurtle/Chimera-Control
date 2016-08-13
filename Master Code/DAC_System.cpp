#include "stdafx.h"
#include "DAC_System.h"
#include "constants.h"
// for other ni stuff
#include "nidaqmx2.h"
#include "ExperimentManager.h"
#include "MasterWindow.h"

DAC_System::DAC_System(int& startID)
{
	if (startID != ID_DAC_FIRST_EDIT)
	{
		errBox("ERROR: startID value " + std::to_string(startID) + " Didn't match ID_DAC_FIRST_EDIT" + std::to_string(ID_DAC_FIRST_EDIT));
	}
	for (int dacInc = 0; dacInc < this->breakoutBoardEdits.size(); dacInc++)
	{
		breakoutBoardEdits[dacInc].ID = startID;
		startID += 1;
	}
	for (int dacInc = 0; dacInc < this->dacLabels.size(); dacInc++)
	{
		dacLabels[dacInc].ID = startID;
		startID += 1;
	}
	this->dacSetButton.ID = startID;
	if (startID != ID_DAC_SET_BUTTON)
	{
		errBox("ERROR: startID value " + std::to_string(startID) + " Didn't match ID_DAC_SET_BUTTON" + std::to_string(ID_DAC_SET_BUTTON));
	}
	startID += 1;
	
	if (!DAQMX_SAFEMODE)
	{
		// initialize tasks and chanells on the DACs
		long output = 0;
		long sampsPerChanWritten;

		//Create a task for each board
		//assume 3 boards, 8 channels per board. AMK 11/2010, modified for three from 2
		//task names are defined as public variables of type Long in TheMainProgram Declarations
		//This creates the task to output from DAC 2
		output = DAQmxCreateTask("", &this->staticDAC_2);
		if (output != 0)
		{
			// error!
			errBox("Create Task 2 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		//This creates the task to output from DAC 1
		output = DAQmxCreateTask("", &this->staticDAC_1);
		if (output != 0)
		{
			// error!
			errBox("Create Task 1 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		//This creates the task to output from DAC 0
		output = DAQmxCreateTask("", &this->staticDAC_0);
		if (output != 0)
		{
			errBox("Create Task 0 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		/// INPUTS
		//This creates a task to read in a digital input from DAC 0 on port 0 line 0
		output = DAQmxCreateTask("", &this->digitalDAC_0_00);
		if (output != 0)
		{
			errBox("Create Task 00 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		//This creates a task to read in a digital input from DAC 0 on port 1 line 1 (currently unused 11/08)
		output = DAQmxCreateTask("", &this->digitalDAC_0_01);
		if (output != 0)
		{
			errBox("Create Task 01 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		// Configure the output
		output = DAQmxCreateAOVoltageChan(this->staticDAC_2, "PXI1Slot5/ao0:7", "StaticDAC_2", -10, 10, DAQmx_Val_Volts, "");
		if (output != 0)
		{
			errBox("Create Voltage Channel 2 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		//'Not sure why Tara and Debbie chose to switch the labels (for staticDac_0 -> StaticDac_1) here, but I'll stick with it to be consistent everywhere else in the program.AMK, 11 / 2010
		output = DAQmxCreateAOVoltageChan(staticDAC_0, "PXI1Slot3/ao0:7", "StaticDAC_1", -10, 10, DAQmx_Val_Volts, "");
		if (output != 0)
		{
			errBox("Create Voltage Channel 1 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		output = DAQmxCreateAOVoltageChan(staticDAC_1, "PXI1Slot4/ao0:7", "StaticDAC_0", -10, 10, DAQmx_Val_Volts, "");
		if (output != 0)
		{
			errBox("Create Voltage Channel 0 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		//INPUTS
		output = DAQmxCreateDIChan(digitalDAC_0_00, "PXI1Slot3/port0/line0", "DIDAC_0", DAQmx_Val_ChanPerLine);
		if (output != 0)
		{
			errBox("Create Digital DAC 00 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}

		// currently unused 11/08 (<-date copied from VB6. what is the actual full date though T.T)
		output = DAQmxCreateDIChan(digitalDAC_0_01, "PXI1Slot3/port0/line1", "DIDAC_0", DAQmx_Val_ChanPerLine);
		if (output != 0)
		{
			errBox("Create Digital DAC 01 Failed (" + std::to_string(output) + "): " + this->getErrorMessage(output));
		}
	}
	return;
}

DAC_System::DAC_System() {}

DAC_System::~DAC_System() {}

bool DAC_System::handleEditChange(unsigned int dacNumber)
{
	if (dacNumber > this->breakoutBoardEdits.size())
	{
		errBox("ERROR: attempted to handle dac edit change, but the dac number reported doesn't exist!");
		return false;
	}
	// mark this to change color.
	breakoutBoardEdits[dacNumber].colorState = 1;
	breakoutBoardEdits[dacNumber].parent.RedrawWindow();
	return true;
}

bool DAC_System::isValidDACName(std::string name)
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
bool DAC_System::initialize(POINT& upperLeftHandCornerPosition, HWND windowHandle, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	// title
	RECT location;
	location.left = upperLeftHandCornerPosition.x;
	location.top = upperLeftHandCornerPosition.y;
	location.right = location.left + 480;
	location.bottom = location.top + 25;
	dacTitle.position = location;
	dacTitle.parent.Create("DAC Channel Voltage Values", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, location, CWnd::FromHandle(windowHandle), dacTitle.ID);
	// all number labels
	// all row labels
	location.left = upperLeftHandCornerPosition.x;
	location.top = upperLeftHandCornerPosition.y + 25;
	location.right = location.left + 480;
	location.bottom = location.top + 25;

	dacSetButton.position = location;
	dacSetButton.parent.Create("Set New DAC Values", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, location, CWnd::FromHandle(windowHandle), dacSetButton.ID);
	dacSetButton.setToolTip("Press this button to attempt force all DAC values to the values currently recorded in the edits below.", toolTips, master);
	location.top += 25;
	location.right = location.left + 20;
	location.bottom = location.top + 25;
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		if (dacInc == dacLabels.size() / 3 || dacInc == 2 * dacLabels.size() / 3)
		{
			// go to second or third collumn
			location.left += 160;
			location.right += 160;
			location.top -= 25 * dacLabels.size() / 3;
			location.bottom -= 25 * dacLabels.size() / 3;
		}
		// create label
		dacLabels[dacInc].position = location;
		dacLabels[dacInc].parent.Create(std::to_string(dacInc + 1).c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER, location, 
			CWnd::FromHandle(windowHandle), dacLabels[dacInc].ID);
		dacLabels[dacInc].setToolTip(this->dacNames[dacInc], toolTips, master);
		location.left += 20;
		location.right += 140;
		breakoutBoardEdits[dacInc].position = location;
		breakoutBoardEdits[dacInc].parent.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, location, CWnd::FromHandle(windowHandle), breakoutBoardEdits[dacInc].ID);
		breakoutBoardEdits[dacInc].setToolTip(this->dacNames[dacInc], toolTips, master);
		location.left -= 20;
		location.right -= 140;
		location.top += 25;
		location.bottom += 25;
		//dacLabels[dacInc].ID = ID_DAC_EDIT_1 + dacInc;
	}
	upperLeftHandCornerPosition.y = location.top;
	return true;
}

bool DAC_System::handleButtonPress(TTL_System* ttls)
{
	this->dacEventInfoList.clear();

	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		CString text;
		breakoutBoardEdits[dacInc].parent.GetWindowTextA(text);
		try
		{
			double tempDac = _tstof(text);
			this->forceDacChange(dacInc, tempDac, ttls);
		}
		catch (std::exception& exc)
		{
			errBox("ERROR: value entered in DAC #" + std::to_string(dacInc) + " (" + text.GetString() + ") failed to convert to a double!");
		}
	}
	

	// qprep
	// qgo

	return false;
}

bool DAC_System::analyzeDAC_Commands()
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<DAC_Event>>> timeOrganizer;
	/// organize all of the commands.
	for (int commandInc = 0; commandInc < this->dacEventInfoList.size(); commandInc++)
	{
		// if it stays -1 after the following, it's a new time.
		int timeIndex = -1;
		for (int timeInc = 0; timeInc < timeOrganizer.size(); timeInc++)
		{
			if (dacEvents[commandInc].time == timeOrganizer[timeInc].first)
			{
				timeIndex = timeInc;
				break;
			}
		}
		if (timeIndex == -1)
		{
			std::vector<DAC_Event> temp;
			temp.push_back(dacEvents[commandInc]);
			timeOrganizer.push_back({ dacEvents[commandInc].time, temp });
		}
		else
		{
			timeOrganizer[timeIndex].second.push_back(dacEvents[commandInc]);
		}
	}
	// this one will have all of the times ordered in sequence, in case the other doesn't.
	std::vector<std::pair<double, std::vector<DAC_Event>>> orderedOrganizer;
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
	this->dacSnapshots.clear();
	// give it the initial status.
	this->dacSnapshots.push_back({ 0, this->dacValues });
	if (orderedOrganizer.size() == 0)
	{
		errBox("ERROR: no dac commands...?");
		return false;
	}
	if (orderedOrganizer[0].first == 0)
	{
		// handle the zero case specially.
		// for every event at the first time...
		for (int zeroInc = 0; zeroInc < orderedOrganizer[0].second.size(); zeroInc++)
		{
			this->dacSnapshots[0].dacValues[orderedOrganizer[0].second[zeroInc].line] = orderedOrganizer[0].second[zeroInc].value;
			//... setting it to the command's state.
		}
	}
	// 
	for (int commandInc = 1; commandInc < timeOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		this->dacSnapshots.push_back(this->dacSnapshots.back());
		for (int zeroInc = 0; zeroInc < orderedOrganizer[commandInc].second.size(); zeroInc++)
		{
			// see description of this command above... update everything that changed at this time.
			this->dacSnapshots[commandInc].dacValues[orderedOrganizer[commandInc].second[zeroInc].line] = orderedOrganizer[commandInc].second[zeroInc].value;
		}
	}
	// phew.

	return true;
}

std::array<std::string, 24> DAC_System::getAllNames()
{
	return this->dacNames;
}
std::string DAC_System::getErrorMessage(int errorCode)
{
	char* errorChars;
	long bufferSize;
	long status;

	//'Find out the error message length.
	bufferSize = DAQmxGetErrorString(errorCode, 0, 0);
	//'Allocate enough space in the string.
	errorChars = new char[bufferSize];
		//'Get the actual error message.
	status = DAQmxGetErrorString(errorCode, errorChars, bufferSize);
	std::string errorString(errorChars);
	return errorString;
}

bool DAC_System::interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNumber, std::vector<variable> vars)
{
	this->dacEvents.clear();
	for (int eventInc = 0; eventInc < this->dacEventInfoList.size(); eventInc++)
	{
		// interpret ramp time command. I need to know whether it's ramping or not.
		double rampTime;
		try
		{
			rampTime = std::stod(dacEventInfoList[eventInc].rampTime);
		}
		catch (std::invalid_argument&)
		{
			for (int varInc = 0; varInc < vars.size(); varInc++)
			{
				if (vars[varInc].name == dacEventInfoList[eventInc].rampTime)
				{
					rampTime = key[dacEventInfoList[eventInc].rampTime][variationNumber];
				}
			}
		}
		if (rampTime == 0)
		{
			/// single point.
			double value, time;
			DAC_Event tempEvent;
			tempEvent.line = dacEventInfoList[eventInc].line;
			//////////////////////////////////
			// Deal with time.
			if (dacEventInfoList[eventInc].time.first == "")
			{
				// no variable portion of the time.
				tempEvent.time = dacEventInfoList[eventInc].time.second;
			}
			else
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == dacEventInfoList[eventInc].time.first)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: the time string " + dacEventInfoList[eventInc].time.first + " is not a variable!");
					return false;
				}
				double varTime = key[dacEventInfoList[eventInc].time.first][variationNumber];
				tempEvent.time = varTime + dacEventInfoList[eventInc].time.second;
			}
			////////////////
			// deal with value
			try
			{
				tempEvent.value = std::stod(dacEventInfoList[eventInc].initVal);
			}
			catch (std::invalid_argument&)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (dacEventInfoList[eventInc].initVal == vars[varInc].name)
					{
						tempEvent.value = key[dacEventInfoList[eventInc].initVal][variationNumber];
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: the dac value " + dacEventInfoList[eventInc].initVal + " is not a variable or a double!");
					return false;
				}
			}
			this->dacEvents.push_back(tempEvent);
		}
		else
		{
			/// many points to be made.
			double value, time;
			DAC_Event tempEvent;
			tempEvent.line = dacEventInfoList[eventInc].line;
			//////////////////////////////////
			// Deal with time.
			if (dacEventInfoList[eventInc].time.first == "")
			{
				// no variable portion of the time.
				tempEvent.time = dacEventInfoList[eventInc].time.second;
			}
			else
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (vars[varInc].name == dacEventInfoList[eventInc].time.first)
					{
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: the time string " + dacEventInfoList[eventInc].time.first + " is not a variable!");
					return false;
				}
				double varTime = key[dacEventInfoList[eventInc].time.first][variationNumber];
				tempEvent.time = varTime + dacEventInfoList[eventInc].time.second;
			}
			// convert initValue and finalValue to doubles to be used 
			double initValue, finalValue, rampInc;
			try
			{
				initValue = std::stod(dacEventInfoList[eventInc].initVal);
			}
			catch (std::invalid_argument)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (dacEventInfoList[eventInc].initVal == vars[varInc].name)
					{
						initValue = key[dacEventInfoList[eventInc].initVal][variationNumber];
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: the dac initial value " + dacEventInfoList[eventInc].initVal + " is not a variable or a double!");
					return false;
				}
			}
			// deal with final value;
			try
			{
				finalValue = std::stod(dacEventInfoList[eventInc].finalVal);
			}
			catch (std::invalid_argument)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (dacEventInfoList[eventInc].finalVal == vars[varInc].name)
					{
						finalValue = key[dacEventInfoList[eventInc].finalVal][variationNumber];
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: the dac initial value " + dacEventInfoList[eventInc].finalVal + " is not a variable or a double!");
					return false;
				}
			}
			// deal with ramp inc
			try
			{
				rampInc = std::stod(dacEventInfoList[eventInc].rampInc);
			}
			catch (std::invalid_argument)
			{
				bool isVar = false;
				for (int varInc = 0; varInc < vars.size(); varInc++)
				{
					if (dacEventInfoList[eventInc].rampInc == vars[varInc].name)
					{
						rampInc = key[dacEventInfoList[eventInc].rampInc][variationNumber];
						isVar = true;
						break;
					}
				}
				if (!isVar)
				{
					errBox("ERROR: the dac ramp increment value " + dacEventInfoList[eventInc].rampInc + " is not a variable or a double!");
					return false;
				}
			}
			// This might be the first not i++ usage of a for loop I've ever done...
			// calculate the time increment:
			double timeInc = rampTime / int((finalValue - initValue) / rampInc);
			double currentTime = tempEvent.time;
			for (int dacValue = initValue; dacValue < finalValue; dacValue += rampInc)
			{
				tempEvent.value = dacValue;
				tempEvent.time = currentTime;
				this->dacEvents.push_back(tempEvent);
				currentTime += timeInc;
			}
			// and get the final value.
			tempEvent.value = finalValue;
			tempEvent.time = tempEvent.time + rampTime;
			this->dacEvents.push_back(tempEvent);
		}
	}
	return true;
}

// note that this is not directly tied to changing any "current" parameters in the DAC_System object (it of course changes a list parameter). The 
// DAC_System object "current" parameters aren't updated to reflect an experiment, so if this is called for a force out, it should be called in conjuction
// with changing "currnet" parameters in the DAC_System object.
bool DAC_System::setDacEvent(int line, std::pair<std::string, double> time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc, TTL_System* ttls)
{
	DAC_EventInfo eventInfo;
	eventInfo.line = line;
	eventInfo.initVal = initVal;
	eventInfo.finalVal = finalVal;

	eventInfo.rampTime = rampTime;
	eventInfo.time = time;
	eventInfo.rampInc = rampInc;
	this->dacEventInfoList.push_back(eventInfo);

	// you need to set up a corresponding trigger to tell the dacs to change the output at the correct time.
	// I don't understand why three triggers are sent though... Seems like there should only be one depending on the board. This actually feels like it should screw everything up...
	// maybe it's compensated for somewhere.

	// turn them on...
	ttls->ttlOn(dacTriggerLines[0].first, dacTriggerLines[0].second, time);
	ttls->ttlOn(dacTriggerLines[1].first, dacTriggerLines[1].second, time);
	ttls->ttlOn(dacTriggerLines[2].first, dacTriggerLines[2].second, time);
	// turn them off...
	std::pair<std::string, double> triggerOffTime(time);
	triggerOffTime.second += dacTriggerTime;
	ttls->ttlOff(dacTriggerLines[0].first, dacTriggerLines[0].second, triggerOffTime);
	ttls->ttlOff(dacTriggerLines[1].first, dacTriggerLines[1].second, triggerOffTime);
	ttls->ttlOff(dacTriggerLines[2].first, dacTriggerLines[2].second, triggerOffTime);
	return true;
}

// this is a function called in preparation for forcing a dac change. Remember, you need to call ___ to actually change things.
bool DAC_System::forceDacChange(int line, double voltage, TTL_System* ttls)
{
	// change parameters in the DAC_System object so that the object knows what the current settings are.
	std::string volt = std::to_string(voltage);
	volt.erase(volt.find_last_not_of('0') + 1, std::string::npos);
	this->breakoutBoardEdits[line].parent.SetWindowText(volt.c_str());
	this->breakoutBoardEdits[line].colorState = 0;
	this->breakoutBoardEdits[line].parent.RedrawWindow();
	this->dacValues[line] = voltage;
	
	// I'm not sure it's necessary to go throught the procedure of doing this and using the DIO to trigger the dacs for a foce out. I'm guessing it's 
	// possible to tell the DAC to just immediately change without waiting for a trigger.
	this->setDacEvent(line, {"", 0}, volt, "", "", "", ttls);

	return true;
}

bool DAC_System::resetDACEvents()
{
	this->dacEventInfoList.clear();
	return true;
}

bool DAC_System::stopDacs()
{
	return true;
}
bool DAC_System::resetDacs()
{
	return true;
}
bool DAC_System::configureClocks(long sampleNumber)
{	
	int result = DAQmxCfgSampClkTiming(this->staticDAC_0, "/PXI1Slot3/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber);
	if (result != 0)
	{
		errBox("DAQmxCfgSampClkTiming (configuring clocks) Failed! (" + std::to_string(result) + "): " + this->getErrorMessage(result));
	}
	result = DAQmxCfgSampClkTiming(this->staticDAC_1, "/PXI1Slot4/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber);
	if (result != 0)
	{
		errBox("DAQmxCfgSampClkTiming (configuring clocks) Failed! (" + std::to_string(result) + "): " + this->getErrorMessage(result));
	}
	result = DAQmxCfgSampClkTiming(this->staticDAC_2, "/PXI1Slot5/PFI0", 1000000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, sampleNumber);
	if (result != 0)
	{
		errBox("DAQmxCfgSampClkTiming (configuring clocks) Failed! (" + std::to_string(result) + "): " + this->getErrorMessage(result));
	}
	return true;
}
bool DAC_System::writeDacs()
{
	bool32 nothing = 0;
	int32 samplesWritten;
	int output = DAQmxWriteAnalogF64(this->staticDAC_0, this->finalFormattedData[0].size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, this->finalFormattedData[0].data(), &samplesWritten, &nothing);
	if (output != 0)
	{
		errBox("DAQmxWriteAnalogF64 (Writing Data) Failed! (" + std::to_string(output) + "): " + this->getErrorMessage(output));
	}
	output = DAQmxWriteAnalogF64(this->staticDAC_0, this->finalFormattedData[0].size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, this->finalFormattedData[0].data(), &samplesWritten, &nothing);
	if (output != 0)
	{
		errBox("DAQmxWriteAnalogF64 (Writing Data) Failed! (" + std::to_string(output) + "): " + this->getErrorMessage(output));
	}
	output = DAQmxWriteAnalogF64(this->staticDAC_0, this->finalFormattedData[0].size(), false, 0.0001, DAQmx_Val_GroupByScanNumber, this->finalFormattedData[0].data(), &samplesWritten, &nothing);
	if (output != 0)
	{
		errBox("DAQmxWriteAnalogF64 (Writing Data) Failed! (" + std::to_string(output) + "): " + this->getErrorMessage(output));
	}
	return true;
}

bool DAC_System::makeFinalDataFormat()
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
	return true;
}

bool DAC_System::handleDAC_ScriptCommand(std::pair<std::string, long> time, std::string name, std::string initVal, std::string finalVal, std::string rampTime, 
	std::string rampInc, std::vector<unsigned int>& dacShadeLocations, std::vector<variable> vars, TTL_System* ttls)
{
	double value;
	if (!this->isValidDACName(name))
	{
		errBox("ERROR: the name " + name + " is not the name of a dac!");
		return false;
	}
	// work with initVal;
	try
	{
		value = std::stod(initVal);
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
			errBox("ERROR: tried and failed to convert " + initVal + " to a double for a dac voltage value. It's also not a variable.");
			return false;
		}
	}
	// work with finVal
	try
	{
		value = std::stod(finalVal);
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
			errBox("ERROR: tried and failed to convert " + finalVal + " to a double for a dac voltage value. It's also not a variable.");
			return false;
		}
	}
	// work with the ramp time
	try
	{
		value = std::stod(rampTime);
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
			errBox("ERROR: tried and failed to convert " + rampTime + " to a double for a dac time value. It's also not a variable.");
			return false;
		}
	}
	// work with rampInc
	try
	{
		value = std::stod(rampInc);
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
			errBox("ERROR: tried and failed to convert " + rampInc + " to a double for a dac voltage value. It's also not a variable.");
			return false;
		}
	}
	// convert name to corresponding dac line.
	int line = this->getDAC_Identifier(name);
	if (line == -1)
	{
		errBox("ERROR: the name " + name + " is not the name of a dac!");
		return false;
	}
	dacShadeLocations.push_back(line);
	this->setDacEvent(line, time, initVal, finalVal, rampTime, rampInc, ttls);
	return true;
}

int DAC_System::getDAC_Identifier(std::string name)
{
	for (int dacInc = 0; dacInc < this->dacValues.size(); dacInc++)
	{
		// check names set by user.
		if (name == this->dacNames[dacInc])
		{
			return dacInc;
		}
		// check standard names which are always acceptable.
		if (name == "dac" + std::to_string(dacInc + 1))
		{
			return dacInc;
		}
	}
	// not an identifier.
	return -1;
}

bool DAC_System::setName(int dacNumber, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	this->dacNames[dacNumber] = name;
	this->breakoutBoardEdits[dacNumber].setToolTip(name, toolTips, master);
	return true;
}

std::string DAC_System::getName(int dacNumber)
{
	return this->dacNames[dacNumber];
}

HBRUSH DAC_System::handleColorMessage(CWnd* window, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rgbs, CDC* cDC)
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
		int editNum = controlID - this->breakoutBoardEdits[0].ID;
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

unsigned int DAC_System::getNumberOfDACs()
{
	return this->dacValues.size();
}

double DAC_System::getDAC_Value(int dacNumber)
{
	return this->dacValues[dacNumber];
}

bool DAC_System::shadeDacs(std::vector<unsigned int>& dacShadeLocations)
{
	for (int shadeInc = 0; shadeInc < dacShadeLocations.size(); shadeInc++)
	{
		this->breakoutBoardEdits[dacShadeLocations[shadeInc]].colorState = -1;
		this->breakoutBoardEdits[dacShadeLocations[shadeInc]].parent.SetReadOnly(true);
		this->breakoutBoardEdits[dacShadeLocations[shadeInc]].parent.RedrawWindow();
	}
	return true;
}
bool DAC_System::unshadeDacs()
{
	for (int shadeInc = 0; shadeInc < this->breakoutBoardEdits.size(); shadeInc++)
	{
		if (this->breakoutBoardEdits[shadeInc].colorState == -1)
		{
			this->breakoutBoardEdits[shadeInc].colorState = 0;
			this->breakoutBoardEdits[shadeInc].parent.SetReadOnly(false);
			this->breakoutBoardEdits[shadeInc].parent.RedrawWindow();
		}
	}
	return true;
}
