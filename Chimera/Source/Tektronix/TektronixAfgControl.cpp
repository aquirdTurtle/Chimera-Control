// created by Mark O. Brown

#include "stdafx.h"
#include "Tektronix/TektronixAfgControl.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "GeneralUtilityFunctions/range.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
TektronixAfgControl::TektronixAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter ) 
	: core(safemode, address, configurationFileDelimiter ) {}

void TektronixAfgControl::handleSaveConfig(ConfigStream& saveFile){
	saveFile << core.configDelim;
	tektronixInfo tekInfo = getTekSettings ();
	for (auto chanInc : range (tekInfo.channels.size ()))
	{
		auto& channel = tekInfo.channels[chanInc];
		saveFile << "\nCHANNEL_" + str(chanInc+1);
		saveFile << "\n/*Control?*/\t"<< channel.control 
				 << "\n/*On?*/\t\t\t" << channel.on 
			     << "\n/*Fsk?*/\t\t" << channel.fsk 
			     << "\n/*Power:*/\t\t" << channel.power 
				 << "\n/*Main Freq:*/\t" << channel.mainFreq 
				 << "\n/*FSK Freq:*/\t" << channel.fskFreq;
	}
	saveFile << "\nEND_" + core.configDelim + "\n";
}


void TektronixAfgControl::handleOpenConfig(ConfigStream& configFile ){
	setSettings(core.getSettingsFromConfig(configFile));
}

void TektronixAfgControl::handleProgram(std::vector<parameterType> constants){
	// this makes sure that what's in the current edits is stored in the currentInfo object.
	getTekSettings();
	core.calculateVariations (constants);
	core.programVariation( 0, constants );
}

std::string TektronixAfgControl::getDelim (){
	return core.configDelim;
}

TekCore& TektronixAfgControl::getCore (){
	return core;
}


void TektronixAfgControl::initialize( POINT& loc, IChimeraWindowWidget* parent, std::string headerText, 
									  std::string channel1Text, std::string channel2Text, LONG width)
{
	header = new QLabel (("Tektronixs " + headerText).c_str(), parent);
	header->setGeometry (loc.x, loc.y, width, 25);
	loc.y += 25;

	programNow = new QPushButton ("Program Now", parent);
	programNow->setGeometry (loc.x, loc.y, width/3, 20);
	parent->connect (programNow, &QPushButton::released, [this, parent]() {
		try	{
			handleProgram (parent->auxWin->getUsableConstants ());
			parent->reportStatus ("Programmed Top/Bottom Tektronix Generator.\r\n");
		}
		catch (Error& exception) {
			parent->reportErr("Error while programing Top/Bottom Tektronix generator: " + exception.trace () + "\r\n");
		}});

	channel1.initialize( { loc.x + width / 3, loc.y }, parent, "Channel 1:" + channel1Text, width / 3 );
	channel2.initialize( { loc.x + 2 * width / 3, loc.y }, parent, "Channel 2:" + channel2Text, width / 3 );

	controlLabel = new QLabel ("Control:", parent);
	controlLabel->setGeometry (loc.x, loc.y+=20, width / 3, 20);

	onOffLabel = new QLabel ("On:", parent);
	onOffLabel->setGeometry (loc.x, loc.y += 20, width / 3, 20);

	fskLabel = new QLabel ("FSK:", parent);
	fskLabel->setGeometry (loc.x, loc.y += 20, width / 3, 20);

	mainPowerLabel = new QLabel ("Power:", parent);
	mainPowerLabel->setGeometry (loc.x, loc.y += 20, width / 3, 20);

	mainFreqLabel = new QLabel ("Main Freq:", parent);
	mainFreqLabel->setGeometry (loc.x, loc.y += 20, width / 3, 20);

	fskFreqLabel = new QLabel ("FSK Freq:", parent);
	fskFreqLabel->setGeometry (loc.x, loc.y += 20, width / 3, 20);
	loc.y += 20;
}


std::string TektronixAfgControl::queryIdentity() {
	return core.queryIdentity ();
}


tektronixInfo TektronixAfgControl::getTekSettings(){
	currentInfo.channels[0] = channel1.getTekChannelSettings();
	currentInfo.channels[1] = channel2.getTekChannelSettings();
	return currentInfo;
}

// does not set the address, that's permanent.
void TektronixAfgControl::setSettings(tektronixInfo info){
	currentInfo.channels = info.channels;
	channel1.setSettings(currentInfo.channels[0]);
	channel2.setSettings(currentInfo.channels[1]);
	// update the controls to reflect what is now selected.
	channel1.handleEnabledStatus ();
	channel2.handleEnabledStatus ();
}
