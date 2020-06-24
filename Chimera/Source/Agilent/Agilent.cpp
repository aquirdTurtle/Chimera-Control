// created by Mark O. Brown
#include "stdafx.h"

#include "Agilent/Agilent.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "ExperimentThread/ExperimentThreadManager.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
#include "GeneralUtilityFunctions/range.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <PrimaryWindows/QtAuxiliaryWindow.h>
#include "boost/lexical_cast.hpp"


Agilent::Agilent( const agilentSettings& settings ) : core(settings),  initSettings(settings){}


void Agilent::programAgilentNow (std::vector<parameterType> constants)
{
	readGuiSettings ();
	std::string warnings_;
	if (currentGuiInfo.channel[0].scriptedArb.fileAddress.expressionStr != "")
	{
		core.analyzeAgilentScript (currentGuiInfo.channel[0].scriptedArb, constants, warnings_);
	}
	if (currentGuiInfo.channel[1].scriptedArb.fileAddress.expressionStr != "")
	{
		core.analyzeAgilentScript (currentGuiInfo.channel[1].scriptedArb, constants, warnings_);
	}
	core.convertInputToFinalSettings (0, currentGuiInfo, constants);
	core.convertInputToFinalSettings (1, currentGuiInfo, constants);
	core.setAgilent (0, constants, currentGuiInfo);
}


std::string Agilent::getDeviceIdentity (){
	return core.getDeviceIdentity ();
}

std::string Agilent::getConfigDelim (){
	return core.configDelim;
}

bool Agilent::getSavedStatus (){
	return agilentScript.savedStatus ();
}

void Agilent::updateSavedStatus (bool isSaved){
	agilentScript.updateSavedStatus (isSaved);
}

void Agilent::initialize( POINT& loc, std::string headerText, UINT editHeight, IChimeraWindowWidget* win, UINT width ){
	LONG w = LONG( width );
	core.initialize ();
	auto deviceInfo = core.getDeviceInfo ();
	header = new QLabel (cstr (headerText), win);
	header->setGeometry (loc.x, loc.y, w, 25);
	loc.y += 25;
	deviceInfoDisplay = new QLabel (cstr (deviceInfo), win);
	deviceInfoDisplay->setGeometry (loc.x, loc.y, w, 20);

	channel1Button = new CQRadioButton ("Channel 1 - No Control", win);
	channel1Button->setGeometry (loc.x, loc.y+=20, w / 2, 20);
	channel1Button->setChecked( true );
	win->connect (channel1Button, &QRadioButton::toggled, [win, this]() {
		auto channel = (channel2Button->isChecked () ? 2 : 1);
		handleChannelPress (channel, win->mainWin->getProfileSettings ().configLocation, win->mainWin->getRunInfo ());
		});

	channel2Button = new CQRadioButton ("Channel 2 - No Control", win);
	channel2Button->setGeometry (loc.x+w/2, loc.y, w / 2, 20);
	channel2Button->setChecked (false);
	win->connect (channel2Button, &QRadioButton::toggled, [win, this]() {
		auto channel = (channel2Button->isChecked () ? 2 : 1);
		handleChannelPress (channel, win->mainWin->getProfileSettings ().configLocation, win->mainWin->getRunInfo ());
	});

	syncedButton = new CQCheckBox ("Synced?", win);
	syncedButton->setGeometry (loc.x, loc.y+=20, w / 3, 20);

	calibratedButton = new CQCheckBox ("Use Cal?", win);
	calibratedButton->setGeometry (loc.x + w / 3, loc.y, w / 3, 20);
	calibratedButton->setChecked( true );

	programNow = new CQPushButton ("Program", win);
	programNow->setGeometry (loc.x + 2 * w / 3, loc.y, w / 3, 20);
	win->connect (programNow, &QPushButton::released, [win, this]() {
		try	{ 
			checkSave (win->mainWin->getProfileSettings ().configLocation, win->mainWin->getRunInfo ()); 
			programAgilentNow (win->auxWin->getUsableConstants ()); 
			win->reportStatus ("Programmed Agilent " + getConfigDelim () + ".\r\n"); 
		}
		catch (Error& err) {
			win->reportErr ("Error while programming agilent " + getConfigDelim () + ": " + err.trace () + "\r\n");
		}
	});

	settingCombo = new CQComboBox (win);
	settingCombo->setGeometry (loc.x, loc.y += 20, w / 4, 25);
	win->connect ( settingCombo, qOverload<int> (&QComboBox::currentIndexChanged), [win, this](int) {
		try	{
			checkSave (win->mainWin->getProfileSettings ().configLocation, win->mainWin->getRunInfo ());
			readGuiSettings ();
			handleModeCombo ();
			updateSettingsDisplay (win->mainWin->getProfileSettings ().configLocation, win->mainWin->getRunInfo ());
		}
		catch (Error& err){
			win->reportErr ("Error while handling agilent combo change: " + err.trace ());
		}
	} );
	settingCombo->addItem ("No Control");
	settingCombo->addItem ("Output Off");
	settingCombo->addItem ("DC");
	settingCombo->addItem ("Sine");
	settingCombo->addItem ("Square");
	settingCombo->addItem ("Preloaded");
	settingCombo->addItem ("Scripted");
	settingCombo->setCurrentIndex (0);

	optionsFormat = new QLabel ("---", win);
	optionsFormat->setGeometry (loc.x + w / 4, loc.y, 3 * w / 4, 25);
	loc.y += 25;
	agilentScript.initialize( width, editHeight, loc, win, "Agilent", "" );

	currentGuiInfo.channel[0].option = AgilentChannelMode::which::No_Control;
	currentGuiInfo.channel[1].option = AgilentChannelMode::which::No_Control;
	agilentScript.setEnabled ( false, false );

	core.programSetupCommands ( );
}


AgilentCore& Agilent::getCore (){
	return core;
}


void Agilent::checkSave( std::string configPath, RunInfo info ){
	if ( currentGuiInfo.channel[currentChannel-1].option == AgilentChannelMode::which::Script ){
		agilentScript.checkSave( configPath, info );
	}
}


void Agilent::verifyScriptable ( ){
	if ( currentGuiInfo.channel[ currentChannel-1 ].option != AgilentChannelMode::which::Script ){
		thrower ( "Agilent is not in scripting mode!" );
	}
}

void Agilent::setDefault (UINT chan){
	core.setDefault (chan);
}


void Agilent::readGuiSettings(int chan ){
	if (chan != 1 && chan != 2){
		thrower ( "Bad argument for agilent channel in Agilent::handleInput(...)!" );
	}
	// convert to zero-indexed
	chan -= 1;
	currentGuiInfo.synced = syncedButton->isChecked( );
	std::string textStr( agilentScript.getScriptText() );
	ConfigStream stream;
	stream << textStr;
	stream.seekg( 0 );
	switch (currentGuiInfo.channel[chan].option){
		case AgilentChannelMode::which::No_Control:
		case AgilentChannelMode::which::Output_Off:
			break;
		case AgilentChannelMode::which::DC:
			stream >> currentGuiInfo.channel[chan].dc.dcLevel;
			currentGuiInfo.channel[chan].dc.useCal = calibratedButton->isChecked ( );
			break;
		case AgilentChannelMode::which::Sine:
			stream >> currentGuiInfo.channel[chan].sine.frequency;
			stream >> currentGuiInfo.channel[chan].sine.amplitude;
			currentGuiInfo.channel[chan].sine.useCal = calibratedButton->isChecked ( );
			break;
		case AgilentChannelMode::which::Square:
			stream >> currentGuiInfo.channel[chan].square.frequency;
			stream >> currentGuiInfo.channel[chan].square.amplitude;
			stream >> currentGuiInfo.channel[chan].square.offset;
			currentGuiInfo.channel[chan].square.useCal = calibratedButton->isChecked ( );
			break;
		case AgilentChannelMode::which::Preloaded:
			stream >> currentGuiInfo.channel[chan].preloadedArb.address;
			currentGuiInfo.channel[chan].preloadedArb.useCal = calibratedButton->isChecked ( );
			break;
		case AgilentChannelMode::which::Script:
			currentGuiInfo.channel[chan].scriptedArb.fileAddress = agilentScript.getScriptPathAndName();
			currentGuiInfo.channel[chan].scriptedArb.useCal = calibratedButton->isChecked ( );
			break;
		default:
			thrower ( "unknown agilent option" );
	}
}


// overload for handling whichever channel is currently selected.
void Agilent::readGuiSettings(  ){
	// true -> 0 + 1 = 1
	// false -> 1 + 1 = 2
	readGuiSettings( (!channel1Button->isChecked ()) + 1 );
}


void Agilent::updateSettingsDisplay( std::string configPath, RunInfo currentRunInfo ){
	updateSettingsDisplay( (!channel1Button->isChecked ()) + 1, configPath, currentRunInfo );
}


void Agilent::updateButtonDisplay( int chan ){
	std::string channelText;
	channelText = chan == 1 ? "Channel 1 - " : "Channel 2 - ";
	channelText += AgilentChannelMode::toStr ( currentGuiInfo.channel[ chan - 1 ].option );
	if ( chan == 1 ){
		channel1Button->setText ( cstr(channelText) );
	}
	else{
		channel2Button->setText ( cstr( channelText ) );
	}
}


void Agilent::updateSettingsDisplay(int chan, std::string configPath, RunInfo currentRunInfo){
	updateButtonDisplay( chan ); 
	// convert to zero-indexed.
	chan -= 1;
	switch ( currentGuiInfo.channel[chan].option ){
		case AgilentChannelMode::which::No_Control:
			agilentScript.reset ( );
			agilentScript.setScriptText("");
			agilentScript.setEnabled ( false, false );
			settingCombo->setCurrentIndex( 0 );
			break;
		case AgilentChannelMode::which::Output_Off:
			agilentScript.reset ( );
			agilentScript.setScriptText("");
			agilentScript.setEnabled ( false, false );
			settingCombo->setCurrentIndex ( 1 );
			break;
		case AgilentChannelMode::which::DC:
			agilentScript.reset ( );
			agilentScript.setScriptText(currentGuiInfo.channel[chan].dc.dcLevel.expressionStr);
			settingCombo->setCurrentIndex ( 2 );
			calibratedButton->setChecked( currentGuiInfo.channel[chan].dc.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Sine:
			agilentScript.reset ( );
			agilentScript.setScriptText(currentGuiInfo.channel[chan].sine.frequency.expressionStr + " " 
										 + currentGuiInfo.channel[chan].sine.amplitude.expressionStr);
			settingCombo->setCurrentIndex ( 3 );
			calibratedButton->setChecked( currentGuiInfo.channel[chan].sine.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Square:
			agilentScript.reset ( );
			agilentScript.setScriptText( currentGuiInfo.channel[chan].square.frequency.expressionStr + " " 
										 + currentGuiInfo.channel[chan].square.amplitude.expressionStr + " " 
										 + currentGuiInfo.channel[chan].square.offset.expressionStr );
			calibratedButton->setChecked( currentGuiInfo.channel[chan].square.useCal );
			agilentScript.setEnabled ( true, false );
			settingCombo->setCurrentIndex (4);
			break;
		case AgilentChannelMode::which::Preloaded:
			agilentScript.reset ( );
			agilentScript.setScriptText(currentGuiInfo.channel[chan].preloadedArb.address.expressionStr);
			calibratedButton->setChecked( currentGuiInfo.channel[chan].preloadedArb.useCal );
			agilentScript.setEnabled ( true, false );
			settingCombo->setCurrentIndex (5);
			break;
		case AgilentChannelMode::which::Script:
			// clear it in case the file fails to open.
			agilentScript.setScriptText( "" );
			agilentScript.openParentScript( currentGuiInfo.channel[chan].scriptedArb.fileAddress.expressionStr, configPath,
											currentRunInfo );
			calibratedButton->setChecked( currentGuiInfo.channel[chan].scriptedArb.useCal );
			agilentScript.setEnabled ( true, false );
			settingCombo->setCurrentIndex (6);
			break;
		default:
			thrower ( "unrecognized agilent setting: " + AgilentChannelMode::toStr(currentGuiInfo.channel[chan].option));
	}
	currentChannel = chan+1;
}


void Agilent::handleChannelPress( int chan, std::string configPath, RunInfo currentRunInfo ){
	// convert from channel 1/2 to 0/1 to access the right array entr
	readGuiSettings( currentChannel );
	updateSettingsDisplay( chan, configPath, currentRunInfo );
	currentChannel = channel1Button->isChecked( ) ? 1 : 2;
}


void Agilent::handleModeCombo(){
	if (!optionsFormat) {
		return;
	}
	int selection = settingCombo->currentIndex();
	int selectedChannel = int( !channel1Button->isChecked() );
	switch (selection)
	{
		case 0:
			optionsFormat->setText( "---" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::No_Control;
			agilentScript.setEnabled ( false, false );
			break;
		case 1:
			optionsFormat->setText ( "---" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Output_Off;
			agilentScript.setEnabled ( false, false );
			break;
		case 2:
			optionsFormat->setText ( "[DC Level]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::DC;
			agilentScript.setEnabled ( true, false );
			break;
		case 3:
			optionsFormat->setText ( "[Frequency(Hz)] [Amplitude(Vpp)]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Sine;
			agilentScript.setEnabled ( true, false );
			break;
		case 4:
			optionsFormat->setText ( "[Frequency(Hz)] [Amplitude(Vpp)] [Offset(V)]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Square;
			agilentScript.setEnabled ( true, false );
			break;
		case 5:
			optionsFormat->setText ( "[File Address]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Preloaded;
			agilentScript.setEnabled ( true, false );
			break;
		case 6:
			optionsFormat->setText ( "Hover over \"?\"" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Script;
			agilentScript.setEnabled ( true, false );
			break;
	}
}


deviceOutputInfo Agilent::getOutputInfo(){
	return currentGuiInfo;
}

/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
void Agilent::handleSavingConfig(ConfigStream& saveFile, std::string configPath, RunInfo info){	
	// make sure data is up to date.
	readGuiSettings (currentChannel);
	// start outputting.
	saveFile << core.configDelim+"\n";
	saveFile << "/*Synced Option:*/ " << str (currentGuiInfo.synced);
	std::vector<std::string> channelStrings = { "\nCHANNEL_1", "\nCHANNEL_2" };
	for (auto chanInc : range (2))
	{
		auto& channel = currentGuiInfo.channel[chanInc];
		saveFile << channelStrings[chanInc];
		saveFile << "\n/*Channel Mode:*/\t\t\t\t" << AgilentChannelMode::toStr (channel.option);
		saveFile << "\n/*DC Level:*/\t\t\t\t\t" << channel.dc.dcLevel;
		saveFile << "\n/*DC Calibrated:*/\t\t\t\t" << channel.dc.useCal;
		saveFile << "\n/*Sine Amplitude:*/\t\t\t\t" << channel.sine.amplitude;
		saveFile << "\n/*Sine Freq:*/\t\t\t\t\t" << channel.sine.frequency;
		saveFile << "\n/*Sine Calibrated:*/\t\t\t" << channel.sine.useCal;
		saveFile << "\n/*Square Amplitude:*/\t\t\t" << channel.square.amplitude;
		saveFile << "\n/*Square Freq:*/\t\t\t\t" << channel.square.frequency;
		saveFile << "\n/*Square Offset:*/\t\t\t\t" << channel.square.offset;
		saveFile << "\n/*Square Calibrated:*/\t\t\t" << channel.square.useCal;
		saveFile << "\n/*Preloaded Arb Address:*/\t\t" << channel.preloadedArb.address;
		saveFile << "\n/*Preloaded Arb Calibrated:*/\t" << channel.preloadedArb.useCal;
		saveFile << "\n/*Scripted Arb Address:*/\t\t" << channel.scriptedArb.fileAddress;
		saveFile << "\n/*Scripted Arb Calibrated:*/\t" << channel.scriptedArb.useCal;
	}
	saveFile << "\nEND_" + core.configDelim + "\n";
}

void Agilent::setOutputSettings (deviceOutputInfo info){
	currentGuiInfo = info;
	updateButtonDisplay (1);
	updateButtonDisplay (2);
}


void Agilent::handleOpenConfig( ConfigStream& file ){
	setOutputSettings (core.getSettingsFromConfig (file));
}


bool Agilent::scriptingModeIsSelected (){
	return currentGuiInfo.channel[currentChannel - 1].option == AgilentChannelMode::which::Script;
}
