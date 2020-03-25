// created by Mark O. Brown
#include "stdafx.h"

#include "Agilent/Agilent.h"
#include "ParameterSystem/ParameterSystem.h"
#include "Scripts/ScriptStream.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "ExperimentThread/ExperimentThreadManager.h"
//#include "PrimaryWindows/AuxiliaryWindow.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
#include "GeneralUtilityFunctions/range.h"
#include "boost/lexical_cast.hpp"


Agilent::Agilent( const agilentSettings& settings ) : core(settings),  initSettings(settings){}


void Agilent::programAgilentNow (std::vector<parameterType> constants)
{
	readGuiSettings ();
	std::string warnings_;
	if (currentGuiInfo.channel[0].scriptedArb.fileAddress != "")
	{
		core.analyzeAgilentScript (currentGuiInfo.channel[0].scriptedArb, constants, warnings_);
	}
	if (currentGuiInfo.channel[1].scriptedArb.fileAddress != "")
	{
		core.analyzeAgilentScript (currentGuiInfo.channel[1].scriptedArb, constants, warnings_);
	}
	core.convertInputToFinalSettings (1, 0, currentGuiInfo, constants);
	core.convertInputToFinalSettings (1, 1, currentGuiInfo, constants);
	core.setAgilent (0, constants, currentGuiInfo);
}


std::string Agilent::getDeviceIdentity ()
{
	return core.getDeviceIdentity ();
}

std::string Agilent::getConfigDelim ()
{
	return core.configDelim;
}

bool Agilent::getSavedStatus ()
{
	return agilentScript.savedStatus ();
}

void Agilent::updateSavedStatus (bool isSaved)
{
	agilentScript.updateSavedStatus (isSaved);
}


void Agilent::initialize( POINT& loc, cToolTips& toolTips, CWnd* parent, int& id, std::string headerText,
						  UINT editHeight, COLORREF color, UINT width )
{
	LONG w = LONG( width );
	core.initialize ();
	auto deviceInfo = core.getDeviceInfo ();
	header.sPos = { loc.x, loc.y, loc.x + w, loc.y += 25 };
	header.Create( cstr( headerText ), NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;

	deviceInfoDisplay.sPos = { loc.x, loc.y, loc.x + w, loc.y += 20 };
	deviceInfoDisplay.Create( cstr( deviceInfo ), NORM_STATIC_OPTIONS, deviceInfoDisplay.sPos, parent, id++ );
	deviceInfoDisplay.fontType = fontTypes::SmallFont;

	channel1Button.sPos = { loc.x, loc.y, loc.x += w/2, loc.y + 20 };
	channel1Button.Create( "Channel 1 - No Control", BS_AUTORADIOBUTTON | WS_GROUP | WS_VISIBLE | WS_CHILD, 
						   channel1Button.sPos, parent, initSettings.chan1ButtonId );
	channel1Button.SetCheck( true );

	channel2Button.sPos = { loc.x, loc.y, loc.x += w/2, loc.y += 20 };
	channel2Button.Create( "Channel 2 - No Control", BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD, channel2Button.sPos, 
						   parent, initSettings.chan2ButtonId );
	loc.x -= w;

	syncedButton.sPos = { loc.x, loc.y, loc.x += w/3, loc.y + 20 };
	syncedButton.Create( "Synced?", NORM_CHECK_OPTIONS | BS_RIGHT, syncedButton.sPos, parent,
						 initSettings.syncButtonId );

	calibratedButton.sPos = { loc.x, loc.y, loc.x += w/3, loc.y + 20 };
	calibratedButton.Create( "Use Cal?", NORM_CHECK_OPTIONS | BS_RIGHT, calibratedButton.sPos,
							 parent, initSettings.calibrationButtonId );
	calibratedButton.SetCheck( true );
	
	programNow.sPos = { loc.x, loc.y, loc.x += w/3, loc.y += 20 };
	programNow.Create( "Program", NORM_PUSH_OPTIONS, programNow.sPos, parent, 
					   initSettings.programButtonId );
	programNow.SetFaceColor( _myRGBs["Dark Grey"], true );
	programNow.SetTextColor( _myRGBs["Solarized Base1"] );
	loc.x -= w;

	settingCombo.sPos = { loc.x, loc.y, loc.x += w/4, loc.y + 200 };
	settingCombo.Create( NORM_COMBO_OPTIONS, settingCombo.sPos, parent, initSettings.agilentComboId );
	settingCombo.AddString( "No Control" );
	settingCombo.AddString( "Output Off" );
	settingCombo.AddString( "DC" );
	settingCombo.AddString( "Sine" );
	settingCombo.AddString( "Square" );
	settingCombo.AddString( "Preloaded" );
	settingCombo.AddString( "Scripted" );
	settingCombo.SetCurSel( 0 );

	optionsFormat.sPos = { loc.x, loc.y, loc.x += 3* w/4, loc.y += 25 };
	optionsFormat.Create( "---", NORM_STATIC_OPTIONS, optionsFormat.sPos, parent, id++ );
	loc.x -= w;

	agilentScript.initialize( width, editHeight, loc, toolTips, parent, id, "Agilent", "",
							  { initSettings.functionComboId, initSettings.editId }, color );

	currentGuiInfo.channel[0].option = AgilentChannelMode::which::No_Control;
	currentGuiInfo.channel[1].option = AgilentChannelMode::which::No_Control;
	currentChannel = 1;
	agilentScript.setEnabled ( false, false );

	core.programSetupCommands ( );
}


AgilentCore& Agilent::getCore ()
{
	return core;
}


void Agilent::checkSave( std::string configPath, RunInfo info )
{
	if ( currentGuiInfo.channel[currentChannel-1].option == AgilentChannelMode::which::Script )
	{
		agilentScript.checkSave( configPath, info );
	}
}


void Agilent::verifyScriptable ( )
{
	if ( currentGuiInfo.channel[ currentChannel-1 ].option != AgilentChannelMode::which::Script )
	{
		thrower ( "Agilent is not in scripting mode!" );
	}
}


void Agilent::rearrange(UINT width, UINT height, fontMap fonts)
{
	header.rearrange(width, height, fonts);
	deviceInfoDisplay.rearrange(width, height, fonts);
	channel1Button.rearrange(width, height, fonts);
	channel2Button.rearrange(width, height, fonts);
	syncedButton.rearrange(width, height, fonts);
	settingCombo.rearrange(width, height, fonts);
	optionsFormat.rearrange(width, height, fonts);
	agilentScript.rearrange(width, height, fonts);
	programNow.rearrange( width, height, fonts );
	calibratedButton.rearrange( width, height, fonts );
}


HBRUSH Agilent::handleColorMessage(CWnd* window, CDC* cDC)
{

	DWORD id = window->GetDlgCtrlID();
	if ( id == deviceInfoDisplay.GetDlgCtrlID() || id == channel1Button.GetDlgCtrlID()  
		 || id == channel2Button.GetDlgCtrlID() || id == syncedButton.GetDlgCtrlID() 
		 || id == settingCombo.GetDlgCtrlID() || id == optionsFormat.GetDlgCtrlID() )
	{
		cDC->SetTextColor( _myRGBs["Text"]);
		cDC->SetBkColor	( _myRGBs[ "Static-Bkgd" ] );
		return *_myBrushes["Static-Bkgd"];
	}
	auto res = agilentScript.handleColorMessage ( window, cDC );
	if ( res )
	{
		return res;
	}
	else
	{
		return NULL;
	}
}


void Agilent::setDefault (UINT chan)
{
	core.setDefault (chan);
}


void Agilent::readGuiSettings(int chan )
{
	if (chan != 1 && chan != 2)
	{
		thrower ( "Bad argument for agilent channel in Agilent::handleInput(...)!" );
	}
	// convert to zero-indexed
	chan -= 1;
	currentGuiInfo.synced = syncedButton.GetCheck( );
	std::string textStr( agilentScript.getScriptText() );
	ScriptStream stream;
	stream << textStr;
	stream.seekg( 0 );
	switch (currentGuiInfo.channel[chan].option)
	{
		case AgilentChannelMode::which::No_Control:
		case AgilentChannelMode::which::Output_Off:
			break;
		case AgilentChannelMode::which::DC:
			stream >> currentGuiInfo.channel[chan].dc.dcLevel;
			currentGuiInfo.channel[chan].dc.useCal = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Sine:
			stream >> currentGuiInfo.channel[chan].sine.frequency;
			stream >> currentGuiInfo.channel[chan].sine.amplitude;
			currentGuiInfo.channel[chan].sine.useCal = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Square:
			stream >> currentGuiInfo.channel[chan].square.frequency;
			stream >> currentGuiInfo.channel[chan].square.amplitude;
			stream >> currentGuiInfo.channel[chan].square.offset;
			currentGuiInfo.channel[chan].square.useCal = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Preloaded:
			stream >> currentGuiInfo.channel[chan].preloadedArb.address;
			currentGuiInfo.channel[chan].preloadedArb.useCal = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Script:
			currentGuiInfo.channel[chan].scriptedArb.fileAddress = agilentScript.getScriptPathAndName();
			currentGuiInfo.channel[chan].scriptedArb.useCal = calibratedButton.GetCheck( );
			break;
		default:
			thrower ( "unknown agilent option" );
	}
}


// overload for handling whichever channel is currently selected.
void Agilent::readGuiSettings(  )
{
	// true -> 0 + 1 = 1
	// false -> 1 + 1 = 2
	readGuiSettings( (!channel1Button.GetCheck()) + 1 );
}


void Agilent::updateSettingsDisplay( std::string configPath, RunInfo currentRunInfo )
{
	updateSettingsDisplay( (!channel1Button.GetCheck()) + 1, configPath, currentRunInfo );
}


void Agilent::updateButtonDisplay( int chan )
{
	std::string channelText;
	channelText = chan == 1 ? "Channel 1 - " : "Channel 2 - ";
	channelText += AgilentChannelMode::toStr ( currentGuiInfo.channel[ chan - 1 ].option );
	if ( chan == 1 )
	{
		channel1Button.SetWindowTextA( cstr(channelText) );
	}
	else
	{
		channel2Button.SetWindowTextA( cstr( channelText ) );
	}
}


void Agilent::updateSettingsDisplay(int chan, std::string configPath, RunInfo currentRunInfo)
{
	updateButtonDisplay( chan ); 
	// convert to zero-indexed.
	chan -= 1;
	switch ( currentGuiInfo.channel[chan].option )
	{
		case AgilentChannelMode::which::No_Control:
			agilentScript.reset ( );
			agilentScript.setScriptText("");
			agilentScript.setEnabled ( false, false );
			settingCombo.SetCurSel( 0 );
			break;
		case AgilentChannelMode::which::Output_Off:
			agilentScript.reset ( );
			agilentScript.setScriptText("");
			agilentScript.setEnabled ( false, false );
			settingCombo.SetCurSel( 1 );
			break;
		case AgilentChannelMode::which::DC:
			agilentScript.reset ( );
			agilentScript.setScriptText(currentGuiInfo.channel[chan].dc.dcLevel.expressionStr);
			settingCombo.SetCurSel( 2 );
			calibratedButton.SetCheck( currentGuiInfo.channel[chan].dc.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Sine:
			agilentScript.reset ( );
			agilentScript.setScriptText(currentGuiInfo.channel[chan].sine.frequency.expressionStr + " " 
										 + currentGuiInfo.channel[chan].sine.amplitude.expressionStr);
			settingCombo.SetCurSel( 3 );
			calibratedButton.SetCheck( currentGuiInfo.channel[chan].sine.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Square:
			agilentScript.reset ( );
			agilentScript.setScriptText( currentGuiInfo.channel[chan].square.frequency.expressionStr + " " 
										 + currentGuiInfo.channel[chan].square.amplitude.expressionStr + " " 
										 + currentGuiInfo.channel[chan].square.offset.expressionStr );
			settingCombo.SetCurSel( 4 );
			calibratedButton.SetCheck( currentGuiInfo.channel[chan].square.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Preloaded:
			agilentScript.reset ( );
			agilentScript.setScriptText(currentGuiInfo.channel[chan].preloadedArb.address);
			settingCombo.SetCurSel( 5 );
			calibratedButton.SetCheck( currentGuiInfo.channel[chan].preloadedArb.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Script:
			settingCombo.SetCurSel( 6 );
			// clear it in case the file fails to open.
			agilentScript.setScriptText( "" );
			agilentScript.openParentScript( currentGuiInfo.channel[chan].scriptedArb.fileAddress, configPath,
											currentRunInfo );
			calibratedButton.SetCheck( currentGuiInfo.channel[chan].scriptedArb.useCal );
			agilentScript.setEnabled ( true, false );
			break;
		default:
			thrower ( "unrecognized agilent setting: " + AgilentChannelMode::toStr(currentGuiInfo.channel[chan].option));
	}
	currentChannel = chan+1;
	if ( currentChannel == 1 )
	{
		channel1Button.SetCheck( true );
		channel2Button.SetCheck( false );
	} 
	else
	{
		channel1Button.SetCheck( false );
		channel2Button.SetCheck( true );
	}	
}


void Agilent::handleChannelPress( int chan, std::string configPath, RunInfo currentRunInfo )
{
	// convert from channel 1/2 to 0/1 to access the right array entr
	readGuiSettings( currentChannel );
	updateSettingsDisplay( chan, configPath, currentRunInfo );
	currentChannel = channel1Button.GetCheck ( ) ? 1 : 2;
}


void Agilent::handleModeCombo()
{
	int selection = settingCombo.GetCurSel();
	int selectedChannel = int( !channel1Button.GetCheck() );
	switch (selection)
	{
		case 0:
			optionsFormat.SetWindowTextA( "---" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::No_Control;
			agilentScript.setEnabled ( false, false );
			break;
		case 1:
			optionsFormat.SetWindowTextA( "---" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Output_Off;
			agilentScript.setEnabled ( false, false );
			break;
		case 2:
			optionsFormat.SetWindowTextA( "[DC Level]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::DC;
			agilentScript.setEnabled ( true, false );
			break;
		case 3:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Sine;
			agilentScript.setEnabled ( true, false );
			break;
		case 4:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)] [Offset(V)]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Square;
			agilentScript.setEnabled ( true, false );
			break;
		case 5:
			optionsFormat.SetWindowTextA( "[File Address]" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Preloaded;
			agilentScript.setEnabled ( true, false );
			break;
		case 6:
			optionsFormat.SetWindowTextA( "Hover over \"?\"" );
			currentGuiInfo.channel[selectedChannel].option = AgilentChannelMode::which::Script;
			agilentScript.setEnabled ( true, false );
			break;
	}
}


deviceOutputInfo Agilent::getOutputInfo()
{
	return currentGuiInfo;
}



void Agilent::handleNewConfig( std::ofstream& newFile )
{
	newFile << core.configDelim+"\n";
	newFile << "0\n";
	newFile << "CHANNEL_1\n";
	newFile << "-2\n0\n0\n0\n1\n0\n0\n1\n0\n0\nNONE\n0\nNONE\n0\n";
	newFile << "CHANNEL_2\n";
	newFile << "-2\n0\n0\n0\n1\n0\n0\n1\n0\n0\nNONE\n0\nNONE\n0\n";
	newFile << "END_" + core.configDelim + "\n";
}

/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
void Agilent::handleSavingConfig(std::ofstream& saveFile, std::string configPath, RunInfo info)
{	
	// make sure data is up to date.
	readGuiSettings( currentChannel );
	// start outputting.
	saveFile << core.configDelim+"\n";
	saveFile << str(currentGuiInfo.synced) << "\n";
	saveFile << "CHANNEL_1\n";
	saveFile << AgilentChannelMode::toStr (currentGuiInfo.channel[0].option) + "\n";
	saveFile << currentGuiInfo.channel[0].dc.dcLevel.expressionStr + "\n";
	saveFile << int(currentGuiInfo.channel[0].dc.useCal) << "\n";
	saveFile << currentGuiInfo.channel[0].sine.amplitude.expressionStr + "\n";
	saveFile << currentGuiInfo.channel[0].sine.frequency.expressionStr + "\n";
	saveFile << int(currentGuiInfo.channel[0].sine.useCal) << "\n";
	saveFile << currentGuiInfo.channel[0].square.amplitude.expressionStr + "\n";
	saveFile << currentGuiInfo.channel[0].square.frequency.expressionStr + "\n";
	saveFile << currentGuiInfo.channel[0].square.offset.expressionStr + "\n";
	saveFile << int(currentGuiInfo.channel[0].square.useCal) << "\n";
	saveFile << currentGuiInfo.channel[0].preloadedArb.address + "\n";
	saveFile << int(currentGuiInfo.channel[0].preloadedArb.useCal) << "\n";
	saveFile << currentGuiInfo.channel[0].scriptedArb.fileAddress + "\n";
	saveFile << int(currentGuiInfo.channel[0].scriptedArb.useCal) << "\n";
	saveFile << "CHANNEL_2\n";
	saveFile << AgilentChannelMode::toStr( currentGuiInfo.channel[1].option ) + "\n";
	saveFile << currentGuiInfo.channel[1].dc.dcLevel.expressionStr + "\n";
	saveFile << int(currentGuiInfo.channel[1].dc.useCal) << "\n";
	saveFile << currentGuiInfo.channel[1].sine.amplitude.expressionStr + "\n";
	saveFile << currentGuiInfo.channel[1].sine.frequency.expressionStr + "\n";
	saveFile << int(currentGuiInfo.channel[1].sine.useCal) << "\n";
	saveFile << currentGuiInfo.channel[1].square.amplitude.expressionStr + "\n";
	saveFile << currentGuiInfo.channel[1].square.frequency.expressionStr + "\n";
	saveFile << currentGuiInfo.channel[1].square.offset.expressionStr + "\n";
	saveFile << int(currentGuiInfo.channel[1].square.useCal) << "\n";
	saveFile << currentGuiInfo.channel[1].preloadedArb.address + "\n";
	saveFile << int(currentGuiInfo.channel[1].preloadedArb.useCal) << "\n";
	saveFile << currentGuiInfo.channel[1].scriptedArb.fileAddress + "\n";
	saveFile << int(currentGuiInfo.channel[1].scriptedArb.useCal) << "\n";
	saveFile << "END_" + core.configDelim + "\n";
}

void Agilent::setOutputSettings (deviceOutputInfo info)
{
	currentGuiInfo = info;
	updateButtonDisplay (1);
	updateButtonDisplay (2);
}


void Agilent::handleOpenConfig( ScriptStream& file, Version ver )
{
	setOutputSettings (getOutputSettingsFromConfigFile (file, ver));
}


deviceOutputInfo Agilent::getOutputSettingsFromConfigFile (ScriptStream& file, Version ver)
{
	std::function<void(ScriptStream&, std::string&)>  readFunc;
	if (ver >= Version ("5.0"))
	{
		readFunc = [](ScriptStream& fid, std::string& expr) {expr = fid.getline (); };
	}
	else
	{
		readFunc = [](ScriptStream& fid, std::string& expr) {std::getline (fid, expr);};
	}
	deviceOutputInfo tempSettings;
	file >> tempSettings.synced;
	std::array<std::string, 2> channelNames = { "CHANNEL_1", "CHANNEL_2" };
	UINT chanInc = 0;
	for (auto& channel : tempSettings.channel)
	{
		ProfileSystem::checkDelimiterLine (file, channelNames[chanInc]);
		// the extra step in all of the following is to remove the , at the end of each input.
		std::string input;
		file >> input;
		try
		{
			channel.option = ver < Version ("4.2") ?
				AgilentChannelMode::which (boost::lexical_cast<int>(input) + 2) : AgilentChannelMode::fromStr (input);
		}
		catch (boost::bad_lexical_cast&)
		{
			throwNested ("Bad channel " + str (chanInc + 1) + " option!");
		}
		std::string calibratedOption;
		readFunc (file, channel.dc.dcLevel.expressionStr);
		if (ver > Version ("2.3"))
		{
			file >> channel.dc.useCal;
		}
		readFunc (file, channel.sine.amplitude.expressionStr);
		readFunc (file, channel.sine.frequency.expressionStr);
		if (ver > Version ("2.3"))
		{
			file >> channel.sine.useCal;
		}
		readFunc (file, channel.square.amplitude.expressionStr);
		readFunc (file, channel.square.frequency.expressionStr);
		readFunc (file, channel.square.offset.expressionStr);
		if (ver > Version ("2.3"))
		{
			file >> channel.square.useCal;
		}
		readFunc (file, channel.preloadedArb.address);
		if (ver > Version ("2.3"))
		{
			file >> channel.preloadedArb.useCal;
		}
		readFunc (file, channel.scriptedArb.fileAddress);
		if (ver > Version ("2.3"))
		{
			file >> channel.scriptedArb.useCal;
		}
		chanInc++;
	}
	return tempSettings;
}


bool Agilent::scriptingModeIsSelected ()
{
	return currentGuiInfo.channel[currentChannel - 1].option == AgilentChannelMode::which::Script;
}
