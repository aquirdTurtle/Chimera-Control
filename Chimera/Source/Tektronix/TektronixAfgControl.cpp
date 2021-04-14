// created by Mark O. Brown

#include "stdafx.h"
#include "Tektronix/TektronixAfgControl.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "GeneralUtilityFunctions/range.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>

TektronixAfgControl::TektronixAfgControl(IChimeraQtWindow* parent_in, bool safemode, std::string address, std::string configurationFileDelimiter )
	: core(safemode, address, configurationFileDelimiter ), IChimeraSystem(parent_in) {

}

void TektronixAfgControl::handleSaveConfig(ConfigStream& saveFile){
	saveFile << core.configDelim;
	tektronixInfo tekInfo = getTekSettings ();
	for (auto chanInc : range (tekInfo.channels.size ())){
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
	core.setSettings (currentInfo);
	core.experimentActive = true;
	core.calculateVariations (constants);
	core.programVariation( 0, constants, nullptr);
}

std::string TektronixAfgControl::getDelim (){
	return core.configDelim;
}

TekCore& TektronixAfgControl::getCore (){
	return core;
}


void TektronixAfgControl::initialize (QPoint& loc, IChimeraQtWindow* parent, std::string headerText,
	std::string channel1Text, std::string channel2Text, long width) {
	auto& px = loc.rx (), & py = loc.ry ();
	header = new QLabel (("Tektronix " + headerText).c_str (), parent);
	header->setGeometry (px, py, width, 25);
	py += 25;

	programNow = new QPushButton ("Program Now", parent);
	programNow->setGeometry (px, py, width / 3, 20);

	parent->connect (programNow, &QPushButton::released, [this, parent]() {
		try	{
			handleProgram (parent->auxWin->getUsableConstants ());
			emit notification({ "Programmed Top/Bottom Tektronix Generator.\r\n", 0, core.getDelim() });
		}
		catch (ChimeraError& exception) {
			emit error({ "Error while programing Top/Bottom Tektronix generator: " + exception.trace() + "\r\n", 0, 
				core.getDelim() });
		}});

	channel1.initialize( { px + width / 3, py }, parent, "Channel 1:" + channel1Text, width / 3 );
	channel2.initialize( { px + 2 * width / 3, py }, parent, "Channel 2:" + channel2Text, width / 3 );

	controlLabel = new QLabel ("Control:", parent);
	controlLabel->setGeometry (px, py+=20, width / 3, 20);

	onOffLabel = new QLabel ("On:", parent);
	onOffLabel->setGeometry (px, py += 20, width / 3, 20);

	fskLabel = new QLabel ("FSK:", parent);
	fskLabel->setGeometry (px, py += 20, width / 3, 20);

	mainPowerLabel = new QLabel ("Power:", parent);
	mainPowerLabel->setGeometry (px, py += 20, width / 3, 20);

	mainFreqLabel = new QLabel ("Main Freq:", parent);
	mainFreqLabel->setGeometry (px, py += 20, width / 3, 20);

	fskFreqLabel = new QLabel ("FSK Freq:", parent);
	fskFreqLabel->setGeometry (px, py += 20, width / 3, 20);
	py += 20;

	emit notification({ core.queryIdentity(), 0, core.getDelim() });
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
