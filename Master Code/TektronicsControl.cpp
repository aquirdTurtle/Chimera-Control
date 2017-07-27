#include "stdafx.h"
#include "TektronicsControl.h"



void TektronicsChannelControl::initialize(POINT loc, CWnd* parent, int& id, std::string channelText, LONG width)
{
	channelLabel.ID = id++;;
	channelLabel.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	channelLabel.Create( channelText.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER, channelLabel.sPos, parent,
						 channelLabel.ID );

	onOffButton.ID = id++;
	if (onOffButton.ID != AXIAL_ON_OFF && onOffButton.ID != EO_ON_OFF && onOffButton.ID != TOP_ON_OFF
		&& onOffButton.ID != BOTTOM_ON_OFF)
	{
		throw;
	}
	onOffButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	onOffButton.Create( "", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTOCHECKBOX, onOffButton.sPos, parent, 
					    onOffButton.ID );
	
	fskButton.ID = id++;
	if (fskButton.ID != AXIAL_FSK && fskButton.ID != EO_FSK && fskButton.ID != TOP_FSK
		&& fskButton.ID != BOTTOM_FSK)
	{
		throw;
	}

	fskButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskButton.Create("", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTOCHECKBOX, fskButton.sPos, parent,
					 fskButton.ID);

	power.ID = id++;;
	power.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	power.Create(WS_CHILD | WS_VISIBLE, power.sPos, parent, power.ID);
	power.EnableWindow(0);

	mainFreq.ID = id++;;
	mainFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	mainFreq.Create(WS_CHILD | WS_VISIBLE, mainFreq.sPos, parent, mainFreq.ID);
	mainFreq.EnableWindow(0);

	fskFreq.ID = id++;;
	fskFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskFreq.Create(WS_CHILD | WS_VISIBLE, fskFreq.sPos, parent, fskFreq.ID);
	fskFreq.EnableWindow(0);
}


void TektronicsControl::interpretKey(key variationKey, std::vector<variable>& vars)
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
	currentNums.clear();
	currentNums.resize(variations);
	for (UINT var = 0; var < variations; var++)
	{
		/// deal with first channel.
		if (currentInfo.channels.first.on)
		{
			currentNums[var].channels.first.mainFreqVal = reduce(currentInfo.channels.first.mainFreq, variationKey, var, vars);
			currentNums[var].channels.first.powerVal = reduce(currentInfo.channels.first.power, variationKey, var, vars);
			// handle FSK options
			if (currentInfo.channels.first.fsk)
			{
				currentNums[var].channels.first.fskFreqVal = reduce(currentInfo.channels.first.fskFreq, variationKey, var, vars);
			}
		}
		// if off don't worry about trying to convert anything, user can not-enter things and it can be fine.
		/// handle second channel.
		if (currentInfo.channels.second.on)
		{
			currentNums[var].channels.second.mainFreqVal = reduce(currentInfo.channels.second.mainFreq, variationKey, var, vars);
			currentNums[var].channels.second.powerVal = reduce(currentInfo.channels.second.power, variationKey, var, vars);
			// handle FSK options
			if (currentInfo.channels.second.fsk)
			{
				currentNums[var].channels.second.fskFreqVal = reduce(currentInfo.channels.second.fskFreq, variationKey, var, vars);
			}
		}
	}
}


HBRUSH TektronicsControl::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID == onOffLabel.ID || controlID == fskLabel.ID || controlID == mainPowerLabel.ID
		|| controlID == mainFreqLabel.ID || controlID == fskFreqLabel.ID )
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["White"]);
		return brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


void TektronicsChannelControl::handleOnPress()
{
	if (onOffButton.GetCheck())
	{
		fskButton.EnableWindow();
		power.EnableWindow();
		mainFreq.EnableWindow();
		if (fskButton.GetCheck())
		{
			fskFreq.EnableWindow();
		}
		else
		{
			fskFreq.EnableWindow(0);
		}
	}
	else
	{
		fskButton.EnableWindow(0);
		power.EnableWindow(0);
		mainFreq.EnableWindow(0);
		fskFreq.EnableWindow(0);
	}
}


void TektronicsChannelControl::handleFskPress()
{
	bool status = fskButton.GetCheck();
	if (status)
	{
		fskFreq.EnableWindow();
	}
	else
	{
		fskFreq.EnableWindow(0);
	}	
}


// TODO: Gonna need to add a check if what gets returned is a double or a variable.
tektronicsChannelInfo TektronicsChannelControl::getSettings()
{
	currentInfo.on = onOffButton.GetCheck();
	currentInfo.fsk = fskButton.GetCheck();

	CString text;
	std::string textStr(text);

	power.GetWindowTextA(text);
	textStr = std::string(text);
	std::transform(textStr.begin(), textStr.end(), textStr.begin(), ::tolower);
	currentInfo.power = std::string(textStr);

	mainFreq.GetWindowTextA(text);
	textStr = std::string(text);
	std::transform(textStr.begin(), textStr.end(), textStr.begin(), ::tolower);
	currentInfo.mainFreq = std::string(textStr);

	fskFreq.GetWindowTextA(text);
	textStr = std::string(text);
	std::transform(textStr.begin(), textStr.end(), textStr.begin(), ::tolower);
	currentInfo.fskFreq = std::string(textStr);
	return currentInfo;
}


void TektronicsChannelControl::setSettings(tektronicsChannelInfo info)
{
	currentInfo = info;
	onOffButton.SetCheck(currentInfo.on);
	fskButton.SetCheck(currentInfo.fsk);
	power.SetWindowTextA(currentInfo.power.c_str());
	mainFreq.SetWindowTextA(currentInfo.mainFreq.c_str());
	fskFreq.SetWindowTextA(currentInfo.fskFreq.c_str());
}

void TektronicsControl::programMachine(Gpib* gpib, UINT var)
{
	if (currentInfo.channels.first.on)
	{
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:FREQ " + str(currentNums[var].channels.first.mainFreqVal));
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:VOLT:UNIT DBM");
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:VOLT " + str(currentNums[var].channels.first.powerVal));
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:VOLT:OFFS 0");

		if (currentInfo.channels.first.fsk)
		{
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:FSKey:STATe On");
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:FSKey:FREQ " + str(currentNums[var].channels.first.fskFreqVal));
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:FSKey:SOURce External");
		}
		else
		{
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE1:FSKey:STATe Off");
		}
		gpib->gpibSend(currentInfo.machineAddress, "OUTput1:STATe ON");
	}
	else
	{
		gpib->gpibSend(currentInfo.machineAddress, "OUTput1:STATe OFF");
	}
	/// second channel
	if (currentInfo.channels.second.on)
	{
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:FREQ " + str(currentNums[var].channels.second.mainFreqVal));
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:VOLT:UNIT DBM");
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:VOLT " + str(currentNums[var].channels.second.powerVal));
		gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:VOLT:OFFS 0");
		if (currentInfo.channels.second.fsk)
		{
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:FSKey:STATe On");
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:FSKey:FREQ " + str(currentNums[var].channels.second.fskFreqVal));
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:FSKey:SOURce External");
		}
		else
		{
			gpib->gpibSend(currentInfo.machineAddress, "SOURCE2:FSKey:STATe Off");
		}
		gpib->gpibSend(currentInfo.machineAddress, "OUTput2:STATe ON");
	}
	else
	{
		gpib->gpibSend(currentInfo.machineAddress, "OUTput2:STATe OFF");
	}
}
/// 

// initialize the control by designating the GPIB address of the machine.
TektronicsControl::TektronicsControl(int address) : machineAddress(address)
{
	currentInfo.machineAddress = address;
}


void TektronicsControl::initialize( POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
								    std::string channel2Text, LONG width )
{
	header.ID = id++;
	header.sPos = { loc.x, loc.y, loc.x + width, loc.y += 25 };
	header.Create( ("Tektronics " + headerText).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER, header.sPos, parent, 
				    header.ID );
	header.fontType = Heading;

	channel1.initialize({ loc.x + width/3, loc.y }, parent, id, channel1Text, width / 3);
	channel2.initialize({ loc.x + 2*width/3, loc.y }, parent, id, channel2Text, width / 3);

	loc.y += 20;
	
	onOffLabel.ID = id++;
	onOffLabel.sPos = { loc.x, loc.y, loc.x + width/3, loc.y += 20 };
	onOffLabel.Create("On:", WS_CHILD | WS_VISIBLE | WS_BORDER, onOffLabel.sPos, parent, onOffLabel.ID);

	fskLabel.ID = id++;
	fskLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	fskLabel.Create("FSK:", WS_CHILD | WS_VISIBLE | WS_BORDER, fskLabel.sPos, parent, fskLabel.ID);

	mainPowerLabel.ID = id++;
	mainPowerLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	mainPowerLabel.Create("Power", WS_CHILD | WS_VISIBLE | WS_BORDER, mainPowerLabel.sPos, parent, mainPowerLabel.ID);

	mainFreqLabel.ID = id++;;
	mainFreqLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	mainFreqLabel.Create("Main Freq.", WS_CHILD | WS_VISIBLE | WS_BORDER, mainFreqLabel.sPos, parent, mainFreqLabel.ID);

	fskFreqLabel.ID = id++;;
	fskFreqLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	fskFreqLabel.Create("FSK Freq.", WS_CHILD | WS_VISIBLE | WS_BORDER, fskFreqLabel.sPos, parent, fskFreqLabel.ID);
}


void TektronicsChannelControl::rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	channelLabel.rearrange("", "", width, height, fonts);
	onOffButton.rearrange("", "", width, height, fonts);
	fskButton.rearrange("", "", width, height, fonts);
	mainFreq.rearrange("", "", width, height, fonts);
	power.rearrange("", "", width, height, fonts);
	fskFreq.rearrange("", "", width, height, fonts);
}


void TektronicsControl::rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	header.rearrange("", "", width, height, fonts);
	onOffLabel.rearrange("", "", width, height, fonts);
	fskLabel.rearrange("", "", width, height, fonts);
	mainPowerLabel.rearrange("", "", width, height, fonts);
	mainFreqLabel.rearrange("", "", width, height, fonts);
	fskFreqLabel.rearrange("", "", width, height, fonts);
	channel1.rearrange(width, height, fonts);
	channel2.rearrange(width, height, fonts);
}

tektronicsInfo TektronicsControl::getSettings()
{
	currentInfo.channels.first = channel1.getSettings();
	currentInfo.channels.second = channel2.getSettings();
	return currentInfo;
}

// does not set the address, that's permanent.
void TektronicsControl::setSettings(tektronicsInfo info)
{
	currentInfo.channels = info.channels;
	channel1.setSettings(currentInfo.channels.first);
	channel2.setSettings(currentInfo.channels.second);
	// update the controls to reflect what is now selected.
	channel1.handleOnPress();
	channel1.handleFskPress();
	channel2.handleOnPress();
	channel2.handleFskPress();
}


void TektronicsControl::handleButtons(UINT indicator)
{
	switch (indicator)
	{
		case 0:
			channel1.handleOnPress();
			break;
		case 1:
			channel1.handleFskPress();
			break;
		case 6:
			channel2.handleOnPress();
			break;
		case 7:
			channel2.handleFskPress();
			break;
	}
}
