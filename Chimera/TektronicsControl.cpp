#include "stdafx.h"
#include "TektronicsControl.h"


void TektronicsChannelControl::initialize(POINT loc, CWnd* parent, int& id, std::string channelText, LONG width)
{
	channelLabel.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	channelLabel.Create( cstr(channelText), WS_CHILD | WS_VISIBLE | WS_BORDER, channelLabel.sPos, parent, id++ );

	onOffButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	onOffButton.Create( "", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTOCHECKBOX, onOffButton.sPos, parent, id++ );
	idVerify(onOffButton, AXIAL_ON_OFF, EO_ON_OFF, TOP_ON_OFF, BOTTOM_ON_OFF);

	fskButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskButton.Create("", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_AUTOCHECKBOX, fskButton.sPos, parent, id++);
	idVerify(fskButton, AXIAL_FSK, EO_FSK, TOP_FSK, BOTTOM_FSK);

	power.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	power.Create(WS_CHILD | WS_VISIBLE, power.sPos, parent, id++);
	power.EnableWindow(0);

	mainFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	mainFreq.Create(WS_CHILD | WS_VISIBLE, mainFreq.sPos, parent, id++);
	mainFreq.EnableWindow(0);

	fskFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskFreq.Create(WS_CHILD | WS_VISIBLE, fskFreq.sPos, parent, id++);
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
	if (controlID == onOffLabel.GetDlgCtrlID() || controlID == fskLabel.GetDlgCtrlID() 
		|| controlID == mainPowerLabel.GetDlgCtrlID() || controlID == mainFreqLabel.GetDlgCtrlID() 
		|| controlID == fskFreqLabel.GetDlgCtrlID())
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["White"]);
		return *brushes["Medium Grey"];
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
	power.GetWindowTextA(text);
	currentInfo.power = str(text, 12, false, true);

	mainFreq.GetWindowTextA(text);
	currentInfo.mainFreq = str(text, 12, false, true);

	fskFreq.GetWindowTextA(text);
	currentInfo.fskFreq = str(text, 12, false, true);

	return currentInfo;
}


void TektronicsChannelControl::setSettings(tektronicsChannelInfo info)
{
	currentInfo = info;
	onOffButton.SetCheck(currentInfo.on);
	fskButton.SetCheck(currentInfo.fsk);
	power.SetWindowTextA(cstr(currentInfo.power));
	mainFreq.SetWindowTextA(cstr(currentInfo.mainFreq));
	fskFreq.SetWindowTextA(cstr(currentInfo.fskFreq));
}


void TektronicsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "TEKTRONICS\n";
	saveFile << "CHANNEL_1\n";
	tektronicsInfo tekInfo = getSettings();
	saveFile << tekInfo.channels.first.on << "\n" << tekInfo.channels.first.fsk << "\n"
		<< tekInfo.channels.first.power << "\n" << tekInfo.channels.first.mainFreq << "\n"
		<< tekInfo.channels.first.fskFreq << "\n";
	saveFile << "CHANNEL_2\n";
	saveFile << tekInfo.channels.second.on << "\n" << tekInfo.channels.second.fsk << "\n"
		<< tekInfo.channels.second.power << "\n" << tekInfo.channels.second.mainFreq << "\n"
		<< tekInfo.channels.second.fskFreq << "\n";
	saveFile << "END_TEKTRONICS" << "\n";
}


void TektronicsControl::handleOpeningConfig(std::ifstream& configFile, double version)
{
	ProfileSystem::checkDelimiterLine(configFile, "TEKTRONICS");
	ProfileSystem::checkDelimiterLine(configFile, "CHANNEL_1");
	tektronicsInfo tekInfo;
	configFile >> tekInfo.channels.first.on;
	configFile >> tekInfo.channels.first.fsk;
	configFile.get();
	std::getline(configFile, tekInfo.channels.first.power);
	std::getline(configFile, tekInfo.channels.first.mainFreq);
	std::getline(configFile, tekInfo.channels.first.fskFreq);
	ProfileSystem::checkDelimiterLine(configFile, "CHANNEL_2");
	configFile >> tekInfo.channels.second.on;
	configFile >> tekInfo.channels.second.fsk;
	configFile.get();
	std::getline(configFile, tekInfo.channels.second.power);
	std::getline(configFile, tekInfo.channels.second.mainFreq);
	std::getline(configFile, tekInfo.channels.second.fskFreq);
	ProfileSystem::checkDelimiterLine(configFile, "END_TEKTRONICS");
	setSettings(tekInfo);
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
	header.sPos = { loc.x, loc.y, loc.x + width, loc.y += 25 };
	header.Create( cstr("Tektronics " + headerText), WS_CHILD | WS_VISIBLE | WS_BORDER, header.sPos, parent, id++ );
	header.fontType = HeadingFont;

	channel1.initialize({ loc.x + width/3, loc.y }, parent, id, channel1Text, width / 3);
	channel2.initialize({ loc.x + 2*width/3, loc.y }, parent, id, channel2Text, width / 3);

	loc.y += 20;
	
	onOffLabel.sPos = { loc.x, loc.y, loc.x + width/3, loc.y += 20 };
	onOffLabel.Create("On:", WS_CHILD | WS_VISIBLE | WS_BORDER, onOffLabel.sPos, parent, id++);

	fskLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	fskLabel.Create("FSK:", WS_CHILD | WS_VISIBLE | WS_BORDER, fskLabel.sPos, parent, id++);

	mainPowerLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	mainPowerLabel.Create("Power", WS_CHILD | WS_VISIBLE | WS_BORDER, mainPowerLabel.sPos, parent, id++);

	mainFreqLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	mainFreqLabel.Create("Main Freq.", WS_CHILD | WS_VISIBLE | WS_BORDER, mainFreqLabel.sPos, parent, id++);

	fskFreqLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	fskFreqLabel.Create("FSK Freq.", WS_CHILD | WS_VISIBLE | WS_BORDER, fskFreqLabel.sPos, parent, id++);
}


void TektronicsChannelControl::rearrange(int width, int height, fontMap fonts)
{
	channelLabel.rearrange("", "", width, height, fonts);
	onOffButton.rearrange("", "", width, height, fonts);
	fskButton.rearrange("", "", width, height, fonts);
	mainFreq.rearrange("", "", width, height, fonts);
	power.rearrange("", "", width, height, fonts);
	fskFreq.rearrange("", "", width, height, fonts);
}


void TektronicsControl::rearrange(int width, int height, fontMap fonts)
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
