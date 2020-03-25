// created by Mark O. Brown

#include "stdafx.h"
#include "Tektronix/TektronixAfgControl.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "GeneralUtilityFunctions/range.h"

TektronixAfgControl::TektronixAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter ) 
	: core(safemode, address, configurationFileDelimiter ) {}


HBRUSH TektronixAfgControl::handleColorMessage(CWnd* window, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID == onOffLabel.GetDlgCtrlID() || controlID == fskLabel.GetDlgCtrlID() 
		|| controlID == mainPowerLabel.GetDlgCtrlID() || controlID == mainFreqLabel.GetDlgCtrlID() 
		|| controlID == fskFreqLabel.GetDlgCtrlID())
	{
		cDC->SetBkColor(_myRGBs["Static-Bkgd"]);
		cDC->SetTextColor( _myRGBs["Text"]);
		return *_myBrushes["Static-Bkgd"];
	}
	else
	{
		return NULL;
	}
}


void TektronixAfgControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << core.configDelim + "\n";
	newFile << "CHANNEL_1\n";
	newFile << 0 << "\n" << 0 << "\n" << 0 << "\n" << -30 << "\n" << 1 << "\n" << 1 << "\n";
	newFile << "CHANNEL_2\n";
	newFile << 0 << "\n" << 0 << "\n" << 0 << "\n" << -30 << "\n" << 1 << "\n" << 1 << "\n";
	newFile << "END_" + core.configDelim + "\n";
}


void TektronixAfgControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << core.configDelim + "\n";
	tektronixInfo tekInfo = getTekSettings ();
	for (auto chanInc : range (tekInfo.channels.size ()))
	{
		auto& channel = tekInfo.channels[chanInc];
		saveFile << "CHANNEL_" + str(chanInc+1) + "\n";
		saveFile << channel.control << "\n" << channel.on << "\n" << channel.fsk << "\n" << channel.power.expressionStr 
			<< "\n" << channel.mainFreq.expressionStr << "\n" << channel.fskFreq.expressionStr << "\n";
	}
	saveFile << "END_" + core.configDelim + "\n";
}


void TektronixAfgControl::handleOpenConfig(ScriptStream& configFile, Version ver )
{
	setSettings(getTekInfo(configFile, ver));
}

tektronixInfo TektronixAfgControl::getTekInfo (ScriptStream& configFile, Version ver)
{
	tektronixInfo tekInfo;
	for (auto chanInc : range (tekInfo.channels.size ()))
	{
		ProfileSystem::checkDelimiterLine (configFile, "CHANNEL_" + str (chanInc+1));
		auto& channel = tekInfo.channels[chanInc];
		configFile >> channel.control >> channel.on >> channel.fsk;
		configFile.get ();
		std::getline (configFile, channel.power.expressionStr);
		std::getline (configFile, channel.mainFreq.expressionStr);
		std::getline (configFile, channel.fskFreq.expressionStr);
	}
	return tekInfo;
}


void TektronixAfgControl::handleProgram(std::vector<parameterType> constants)
{
	// this makes sure that what's in the current edits is stored in the currentInfo object.
	getTekSettings();
	core.interpretKey (constants, currentInfo);
	core.programMachine( 0, currentInfo );
}

std::string TektronixAfgControl::getDelim ()
{
	return core.configDelim;
}

TekCore& TektronixAfgControl::getCore ()
{
	return core;
}


void TektronixAfgControl::initialize( POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
								    std::string channel2Text, LONG width, UINT control_id )
{
	header.sPos = { loc.x, loc.y, loc.x + width, loc.y += 25 };
	header.Create( cstr("Tektronics " + headerText), NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;
	
	programNow.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y + 20 };
	programNow.Create( "Program Now", NORM_PUSH_OPTIONS, programNow.sPos, parent, control_id );

	channel1.initialize( { loc.x + width / 3, loc.y }, parent, id, "Channel 1:" + channel1Text, width / 3, 
						 control_id+1 );
	channel2.initialize( { loc.x + 2 * width / 3, loc.y }, parent, id, "Channel 2:" + channel2Text, width / 3, 
						 control_id+50 );

	loc.y += 20;
	controlLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	controlLabel.Create( "Control:", NORM_STATIC_OPTIONS, onOffLabel.sPos, parent, id++ );

	onOffLabel.sPos = { loc.x, loc.y, loc.x + width/3, loc.y += 20 };
	onOffLabel.Create("On:", NORM_STATIC_OPTIONS, onOffLabel.sPos, parent, id++);

	fskLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	fskLabel.Create("FSK:", NORM_STATIC_OPTIONS, fskLabel.sPos, parent, id++);

	mainPowerLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	mainPowerLabel.Create("Power", NORM_STATIC_OPTIONS, mainPowerLabel.sPos, parent, id++);

	mainFreqLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	mainFreqLabel.Create("Main Freq.", NORM_STATIC_OPTIONS, mainFreqLabel.sPos, parent, id++);

	fskFreqLabel.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y += 20 };
	fskFreqLabel.Create("FSK Freq.", NORM_STATIC_OPTIONS, fskFreqLabel.sPos, parent, id++);
}


std::string TektronixAfgControl::queryIdentity()
{
	return core.queryIdentity ();
}


void TektronixAfgControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange( width, height, fonts);
	controlLabel.rearrange( width, height, fonts );
	onOffLabel.rearrange( width, height, fonts);
	fskLabel.rearrange( width, height, fonts);
	mainPowerLabel.rearrange( width, height, fonts);
	mainFreqLabel.rearrange( width, height, fonts);
	fskFreqLabel.rearrange(width, height, fonts);
	channel1.rearrange(width, height, fonts);
	channel2.rearrange(width, height, fonts);
	programNow.rearrange( width, height, fonts );
}


tektronixInfo TektronixAfgControl::getTekSettings()
{
	currentInfo.channels[0] = channel1.getTekChannelSettings();
	currentInfo.channels[1] = channel2.getTekChannelSettings();
	return currentInfo;
}

// does not set the address, that's permanent.
void TektronixAfgControl::setSettings(tektronixInfo info)
{
	currentInfo.channels = info.channels;
	channel1.setSettings(currentInfo.channels[0]);
	channel2.setSettings(currentInfo.channels[1]);
	// update the controls to reflect what is now selected.
	channel1.handleOnPress();
	channel1.handleFskPress();
	channel2.handleOnPress();
	channel2.handleFskPress();
}


void TektronixAfgControl::handleButtons(UINT indicator)
{
	channel1.handleButton ( indicator );
	channel2.handleButton ( indicator );
}
