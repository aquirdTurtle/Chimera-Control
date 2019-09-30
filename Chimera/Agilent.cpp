// created by Mark O. Brown
#include "stdafx.h"

#include "Agilent.h"
#include "ParameterSystem.h"
#include "ScriptStream.h"
#include "ProfileSystem.h"
#include "AuxiliaryWindow.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
#include "Thrower.h"
#include "range.h"
#include <boost/lexical_cast.hpp>


// NI's visa file. Also gets indirectly included via #include "nifgen.h".
Agilent::Agilent( const agilentSettings& settings ) : 
	visaFlume( settings.safemode, settings.address ),
	sampleRate( settings.sampleRate ),
	initSettings( settings ),
	triggerRow(settings.triggerRow ), 
	triggerNumber( settings.triggerNumber ),
	memoryLoc(settings.memoryLocation ),
	configDelim(settings.configurationFileDelimiter),
	calibrationCoefficients(settings.calibrationCoeff ),
	agilentName( settings.deviceName ),
	setupCommands( settings.setupCommands )
{
	try
	{
		visaFlume.open ( );
	}
	catch ( Error& )
	{
		throwNested ( "Error seen while initializing "+ agilentName + " Agilent" );
	}
}


Agilent::~Agilent()
{
	visaFlume.close();
}


void Agilent::initialize( POINT& loc, cToolTips& toolTips, CWnd* parent, int& id, std::string headerText,
						  UINT editHeight, COLORREF color, UINT width )
{
	LONG w = LONG( width );
	try
	{
		int errCode = 0;
		deviceInfo = visaFlume.identityQuery( );
		isConnected = true;
	}
	catch ( Error& )
	{
		deviceInfo = "Disconnected";
		isConnected = false;
	}

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

	settings.channel[0].option = AgilentChannelMode::which::No_Control;
	settings.channel[1].option = AgilentChannelMode::which::No_Control;
	currentChannel = 1;
	agilentScript.setEnabled ( false, false );

	programSetupCommands ( );
}

std::vector<std::string> Agilent::getStartupCommands ( )
{
	return setupCommands;
}

void Agilent::programSetupCommands()
{
	try
	{
		for ( auto cmd : setupCommands )
		{
			visaFlume.write ( cmd );
		}
	}
	catch ( Error& )
	{
		throwNested ( "Failed to program setup commands for " + agilentName + " Agilent!" );
	}
}


void Agilent::checkSave( std::string categoryPath, RunInfo info )
{
	if ( settings.channel[currentChannel-1].option == AgilentChannelMode::which::Script )
	{
		agilentScript.checkSave( categoryPath, info );
	}
}


void Agilent::verifyScriptable ( )
{
	if ( settings.channel[ currentChannel-1 ].option != AgilentChannelMode::which::Script )
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

/**
 * This function tells the agilent to put out the DC default waveform.
 */
void Agilent::setDefault( int channel )
{
	// turn it to the default voltage...
	std::string setPointString = str(convertPowerToSetPoint(AGILENT_DEFAULT_POWER, true, calibrationCoefficients));
	visaFlume.write( "SOURce" + str(channel) + ":APPLy:DC DEF, DEF, " + setPointString + " V" );
}
/**
 * expects the inputted power to be in -MILI-WATTS! 
 * returns set point in VOLTS
 */
double Agilent::convertPowerToSetPoint(double powerInMilliWatts, bool conversionOption, std::vector<double> calibCoeff)
{
	if ( conversionOption )
	{
		double setPointInVolts = 0;
		if ( calibCoeff.size ( ) == 0 )
		{
			thrower("Wanted agilent calibration but no calibration given to conversion function!" );
		}
		// build the polynomial calibration.
		UINT polyPower = 0;
		for ( auto coeff : calibCoeff )
		{
			setPointInVolts += coeff * std::pow ( powerInMilliWatts, polyPower++ );
		}
		//double setPointInVolts = slope * powerInMilliWatts + offset;
		return setPointInVolts;
	}
	else
	{
		// no conversion
		return powerInMilliWatts;
	}
}


void Agilent::analyzeAgilentScript ( UINT chan, std::vector<parameterType>& vars )
{
	if ( settings.channel[ chan ].option == AgilentChannelMode::which::Script )
	{
		analyzeAgilentScript ( settings.channel[ chan ].scriptedArb, vars );
	}
}


void Agilent::analyzeAgilentScript( scriptedArbInfo& infoObj, std::vector<parameterType>& variables)
{
	ScriptStream stream;
	MasterThreadManager::loadAgilentScript ( infoObj.fileAddress, stream );
	int currentSegmentNumber = 0;
	infoObj.wave.resetNumberOfTriggers( );
	// Procedurally read lines into segment objects.
	while (!stream.eof())
	{
		int leaveTest;
		try
		{
			leaveTest = infoObj.wave.analyzeAgilentScriptCommand ( currentSegmentNumber, stream, variables );
		}
		catch ( Error& )
		{
			throwNested ( "Error seen while analyzing agilent script command for agilent " + this->configDelim );
		}
		if (leaveTest < 0)
		{
			thrower ( "IntensityWaveform.analyzeAgilentScriptCommand threw an error! Error occurred in segment #"
					 + str( currentSegmentNumber ) + "." );
		}
		if (leaveTest == 1)
		{
			// read function is telling this function to stop reading the file because it's at its end.
			break;
		}
		currentSegmentNumber++;
	}
}


std::pair<DioRows::which, UINT> Agilent::getTriggerLine( )
{
	return { triggerRow, triggerNumber };
}


std::string Agilent::getDeviceIdentity()
{
	std::string msg;
	try
	{
		msg = visaFlume.identityQuery();
	}
	catch (Error& err)
	{
		msg == err.trace();
	}
	if ( msg == "" )
	{
		msg = "Disconnected...\n";
	}
	return msg;
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


void Agilent::handleInput(int chan, std::string categoryPath, RunInfo info )
{
	if (chan != 1 && chan != 2)
	{
		thrower ( "Bad argument for agilent channel in Agilent::handleInput(...)!" );
	}
	// convert to zero-indexed
	chan -= 1;
	settings.synced = syncedButton.GetCheck( );
	std::string textStr( agilentScript.getScriptText() );
	ScriptStream stream; 
	stream << textStr; 
	stream.seekg( 0 );
	switch (settings.channel[chan].option)
	{
		case AgilentChannelMode::which::No_Control:
		case AgilentChannelMode::which::Output_Off:
			break;
		case AgilentChannelMode::which::DC:
			stream >> settings.channel[chan].dc.dcLevelInput;
			settings.channel[chan].dc.useCalibration = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Sine:
			stream >> settings.channel[chan].sine.frequencyInput;
			stream >> settings.channel[chan].sine.amplitudeInput;
			settings.channel[chan].sine.useCalibration = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Square:
			stream >> settings.channel[chan].square.frequencyInput;
			stream >> settings.channel[chan].square.amplitudeInput;
			stream >> settings.channel[chan].square.offsetInput;
			settings.channel[chan].square.useCalibration = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Preloaded:
			stream >> settings.channel[chan].preloadedArb.address;
			settings.channel[chan].preloadedArb.useCalibration = calibratedButton.GetCheck( );
			break;
		case AgilentChannelMode::which::Script:
			agilentScript.checkSave( categoryPath, info );
			settings.channel[chan].scriptedArb.fileAddress = agilentScript.getScriptPathAndName();
			settings.channel[chan].scriptedArb.useCalibration = calibratedButton.GetCheck( );
			break;
		default:
			thrower ( "unknown agilent option" );
	}
}


// overload for handling whichever channel is currently selected.
void Agilent::handleInput( std::string categoryPath, RunInfo info )
{
	// true -> 0 + 1 = 1
	// false -> 1 + 1 = 2
	handleInput( (!channel1Button.GetCheck()) + 1, categoryPath, info );
}


void Agilent::updateSettingsDisplay( std::string currentCategoryPath, RunInfo currentRunInfo )
{
	updateSettingsDisplay( (!channel1Button.GetCheck()) + 1, currentCategoryPath, currentRunInfo );
}


void Agilent::updateButtonDisplay( int chan )
{
	std::string channelText;
	channelText = chan == 1 ? "Channel 1 - " : "Channel 2 - ";
	channelText += AgilentChannelMode::toStr ( settings.channel[ chan - 1 ].option );
	if ( chan == 1 )
	{
		channel1Button.SetWindowTextA( cstr(channelText) );
	}
	else
	{
		channel2Button.SetWindowTextA( cstr( channelText ) );
	}
}


void Agilent::updateSettingsDisplay(int chan, std::string currentCategoryPath, RunInfo currentRunInfo)
{
	updateButtonDisplay( chan ); 
	// convert to zero-indexed.
	chan -= 1;
	switch ( settings.channel[chan].option )
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
			agilentScript.setScriptText(settings.channel[chan].dc.dcLevelInput.expressionStr);
			settingCombo.SetCurSel( 2 );
			calibratedButton.SetCheck( settings.channel[chan].dc.useCalibration );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Sine:
			agilentScript.reset ( );
			agilentScript.setScriptText(settings.channel[chan].sine.frequencyInput.expressionStr + " " 
										 + settings.channel[chan].sine.amplitudeInput.expressionStr);
			settingCombo.SetCurSel( 3 );
			calibratedButton.SetCheck( settings.channel[chan].sine.useCalibration );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Square:
			agilentScript.reset ( );
			agilentScript.setScriptText( settings.channel[chan].square.frequencyInput.expressionStr + " " 
										 + settings.channel[chan].square.amplitudeInput.expressionStr + " " 
										 + settings.channel[chan].square.offsetInput.expressionStr );
			settingCombo.SetCurSel( 4 );
			calibratedButton.SetCheck( settings.channel[chan].square.useCalibration );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Preloaded:
			agilentScript.reset ( );
			agilentScript.setScriptText(settings.channel[chan].preloadedArb.address);
			settingCombo.SetCurSel( 5 );
			calibratedButton.SetCheck( settings.channel[chan].preloadedArb.useCalibration );
			agilentScript.setEnabled ( true, false );
			break;
		case AgilentChannelMode::which::Script:
			settingCombo.SetCurSel( 6 );
			// clear it in case the file fails to open.
			agilentScript.setScriptText( "" );
			agilentScript.openParentScript( settings.channel[chan].scriptedArb.fileAddress, currentCategoryPath, 
											currentRunInfo );
			calibratedButton.SetCheck( settings.channel[chan].scriptedArb.useCalibration );
			agilentScript.setEnabled ( true, false );
			break;
		default:
			thrower ( "unrecognized agilent setting: " + AgilentChannelMode::toStr(settings.channel[chan].option));
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


void Agilent::handleChannelPress( int chan, std::string currentCategoryPath, RunInfo currentRunInfo )
{
	// convert from channel 1/2 to 0/1 to access the right array entr
	handleInput( currentChannel, currentCategoryPath, currentRunInfo );
	updateSettingsDisplay( chan, currentCategoryPath, currentRunInfo );
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
			settings.channel[selectedChannel].option = AgilentChannelMode::which::No_Control;
			agilentScript.setEnabled ( false, false );
			break;
		case 1:
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = AgilentChannelMode::which::Output_Off;
			agilentScript.setEnabled ( false, false );
			break;
		case 2:
			optionsFormat.SetWindowTextA( "[DC Level]" );
			settings.channel[selectedChannel].option = AgilentChannelMode::which::DC;
			agilentScript.setEnabled ( true, false );
			break;
		case 3:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)]" );
			settings.channel[selectedChannel].option = AgilentChannelMode::which::Sine;
			agilentScript.setEnabled ( true, false );
			break;
		case 4:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)] [Offset(V)]" );
			settings.channel[selectedChannel].option = AgilentChannelMode::which::Square;
			agilentScript.setEnabled ( true, false );
			break;
		case 5:
			optionsFormat.SetWindowTextA( "[File Address]" );
			settings.channel[selectedChannel].option = AgilentChannelMode::which::Preloaded;
			agilentScript.setEnabled ( true, false );
			break;
		case 6:
			optionsFormat.SetWindowTextA( "Hover over \"?\"" );
			settings.channel[selectedChannel].option = AgilentChannelMode::which::Script;
			agilentScript.setEnabled ( true, false );
			break;
	}
}


deviceOutputInfo Agilent::getOutputInfo()
{
	return settings;
}


void Agilent::convertInputToFinalSettings( UINT chan, std::vector<parameterType>& variables, UINT variation )
{
	// iterate between 0 and 1...
	channelInfo& channel( settings.channel[chan] );
	try
	{
		switch (channel.option)
		{
			case AgilentChannelMode::which::No_Control:
			case AgilentChannelMode::which::Output_Off:
				break;
			case AgilentChannelMode::which::DC:
				channel.dc.dcLevel = channel.dc.dcLevelInput.evaluate( variables, variation );
				break;
			case AgilentChannelMode::which::Sine:
				channel.sine.frequency = channel.sine.frequencyInput.evaluate( variables, variation );
				channel.sine.amplitude = channel.sine.amplitudeInput.evaluate( variables, variation );
				break;
			case AgilentChannelMode::which::Square:
				channel.square.frequency = channel.square.frequencyInput.evaluate( variables, variation );
				channel.square.amplitude = channel.square.amplitudeInput.evaluate( variables, variation );
				channel.square.offset = channel.square.offsetInput.evaluate( variables, variation );
				break;
			case AgilentChannelMode::which::Preloaded:
				break;
			case AgilentChannelMode::which::Script:
				handleScriptVariation( variation, channel.scriptedArb, chan+1, variables );
				break;
			default:
				thrower ( "Unrecognized Agilent Setting: " + AgilentChannelMode::toStr( channel.option ) );
		}
	}
	catch (std::out_of_range&)
	{
		throwNested( "unrecognized variable!" );
	}
}


void Agilent::handleNewConfig( std::ofstream& newFile )
{
	newFile << configDelim+"\n";
	newFile << "0\n";
	newFile << "CHANNEL_1\n";
	newFile << "-2\n0\n0\n0\n1\n0\n0\n1\n0\n0\nNONE\n0\nNONE\n0\n";
	newFile << "CHANNEL_2\n";
	newFile << "-2\n0\n0\n0\n1\n0\n0\n1\n0\n0\nNONE\n0\nNONE\n0\n";
	newFile << "END_" + configDelim + "\n";
}

/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
void Agilent::handleSavingConfig(std::ofstream& saveFile, std::string categoryPath, RunInfo info)
{	
	// make sure data is up to date.
	handleInput( currentChannel, categoryPath, info);
	// start outputting.
	saveFile << configDelim+"\n";
	saveFile << str(settings.synced) << "\n";
	saveFile << "CHANNEL_1\n";
	saveFile << AgilentChannelMode::toStr (settings.channel[0].option) + "\n";
	saveFile << settings.channel[0].dc.dcLevelInput.expressionStr + "\n";
	saveFile << int(settings.channel[0].dc.useCalibration) << "\n";
	saveFile << settings.channel[0].sine.amplitudeInput.expressionStr + "\n";
	saveFile << settings.channel[0].sine.frequencyInput.expressionStr + "\n";
	saveFile << int(settings.channel[0].sine.useCalibration) << "\n";
	saveFile << settings.channel[0].square.amplitudeInput.expressionStr + "\n";
	saveFile << settings.channel[0].square.frequencyInput.expressionStr + "\n";
	saveFile << settings.channel[0].square.offsetInput.expressionStr + "\n";
	saveFile << int(settings.channel[0].square.useCalibration) << "\n";
	saveFile << settings.channel[0].preloadedArb.address + "\n";
	saveFile << int(settings.channel[0].preloadedArb.useCalibration) << "\n";
	saveFile << settings.channel[0].scriptedArb.fileAddress + "\n";
	saveFile << int(settings.channel[0].scriptedArb.useCalibration) << "\n";
	saveFile << "CHANNEL_2\n";
	saveFile << AgilentChannelMode::toStr( settings.channel[1].option ) + "\n";
	saveFile << settings.channel[1].dc.dcLevelInput.expressionStr + "\n";
	saveFile << int(settings.channel[1].dc.useCalibration) << "\n";
	saveFile << settings.channel[1].sine.amplitudeInput.expressionStr + "\n";
	saveFile << settings.channel[1].sine.frequencyInput.expressionStr + "\n";
	saveFile << int(settings.channel[1].sine.useCalibration) << "\n";
	saveFile << settings.channel[1].square.amplitudeInput.expressionStr + "\n";
	saveFile << settings.channel[1].square.frequencyInput.expressionStr + "\n";
	saveFile << settings.channel[1].square.offsetInput.expressionStr + "\n";
	saveFile << int(settings.channel[1].square.useCalibration) << "\n";
	saveFile << settings.channel[1].preloadedArb.address + "\n";
	saveFile << int(settings.channel[1].preloadedArb.useCalibration) << "\n";
	saveFile << settings.channel[1].scriptedArb.fileAddress + "\n";
	saveFile << int(settings.channel[1].scriptedArb.useCalibration) << "\n";
	saveFile << "END_" + configDelim + "\n";
}

void Agilent::handleOpenConfig( std::ifstream& file, Version ver )
{
	file >> settings.synced;
	std::array<std::string, 2> channelNames = { "CHANNEL_1", "CHANNEL_2" };
	UINT chanInc = 0;
	for ( auto& channel : settings.channel )
	{
		ProfileSystem::checkDelimiterLine ( file, channelNames[chanInc] );
		// the extra step in all of the following is to remove the , at the end of each input.
		std::string input;
		file >> input;
		file.get ( );
		try
		{
			channel.option = ver < Version ( "4.2" ) ? 
				AgilentChannelMode::which(boost::lexical_cast<int>(input) + 2) : AgilentChannelMode::fromStr(input);
		}
		catch ( boost::bad_lexical_cast& )
		{
			throwNested ( "Bad channel " + str(chanInc + 1) + " option!" );
		}
		std::string calibratedOption;
		std::getline ( file, channel.dc.dcLevelInput.expressionStr );
		if ( ver > Version ( "2.3" ) )
		{
			std::getline ( file, calibratedOption );
			bool calOption;
			try
			{
				calOption = bool ( boost::lexical_cast<int>( calibratedOption ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				calOption = false;
			}
			channel.dc.useCalibration = calOption;
		}
		std::getline ( file, channel.sine.amplitudeInput.expressionStr );
		std::getline ( file, channel.sine.frequencyInput.expressionStr );
		if ( ver > Version ( "2.3" ) )
		{
			std::getline ( file, calibratedOption );
			bool calOption;
			try
			{
				calOption = bool ( boost::lexical_cast<int>( calibratedOption ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				calOption = false;
			}
			channel.sine.useCalibration = calOption;
		}
		std::getline ( file, channel.square.amplitudeInput.expressionStr );
		std::getline ( file, channel.square.frequencyInput.expressionStr );
		std::getline ( file, channel.square.offsetInput.expressionStr );
		if ( ver > Version ( "2.3" ) )
		{
			std::getline ( file, calibratedOption );
			bool calOption;
			try
			{
				calOption = bool ( boost::lexical_cast<int>( calibratedOption ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				calOption = false;
			}
			channel.square.useCalibration = calOption;
		}
		std::getline ( file, channel.preloadedArb.address );
		if ( ver > Version ( "2.3" ) )
		{
			std::getline ( file, calibratedOption );
			bool calOption;
			try
			{
				calOption = bool ( boost::lexical_cast<int>( calibratedOption ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				calOption = false;
			}
			channel.preloadedArb.useCalibration = calOption;
		}
		std::getline ( file, channel.scriptedArb.fileAddress );
		if ( ver > Version ( "2.3" ) )
		{
			std::getline ( file, calibratedOption );
			bool calOption;
			try
			{
				calOption = bool ( boost::lexical_cast<int>( calibratedOption ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				calOption = false;
			}
			channel.scriptedArb.useCalibration = calOption;
		}
		chanInc++;
	}
	updateButtonDisplay( 1 );
	updateButtonDisplay( 2 );
}


void Agilent::outputOff( int channel )
{
	if (channel != 1 && channel != 2)
	{
		thrower ( "bad value for channel inside outputOff!" );
	}
	channel++;
	visaFlume.write( "OUTPUT" + str( channel ) + " OFF" );
}


bool Agilent::connected()
{
	return isConnected;
}


void Agilent::setDC( int channel, dcInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower ( "Bad value for channel inside setDC!" );
	}
	visaFlume.write( "SOURce" + str( channel ) + ":APPLy:DC DEF, DEF, " 
					 + str( convertPowerToSetPoint(info.dcLevel, info.useCalibration, calibrationCoefficients) ) + " V" );
}


void Agilent::setExistingWaveform( int channel, preloadedArbInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower ( "Bad value for channel in setExistingWaveform!" );
	}
	auto sStr = "SOURCE" + str ( channel );
	visaFlume.write( sStr + ":DATA:VOL:CLEAR" );
	// Load sequence that was previously loaded.
	visaFlume.write( "MMEM:LOAD:DATA \"" + info.address + "\"" );
	// tell it that it's outputting something arbitrary (not sure if necessary)
	visaFlume.write( sStr + ":FUNC ARB" );
	// tell it what arb it's outputting.
	visaFlume.write( sStr + ":FUNC:ARB \"" + memoryLoc + ":\\" + info.address + "\"" );
	// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
	visaFlume.write( sStr + ":BURST::MODE TRIGGERED" );
	visaFlume.write( sStr + ":BURST::NCYCLES 1" );
	visaFlume.write( sStr + ":BURST::PHASE 0" );
	visaFlume.write( sStr + ":BURST::STATE ON" );
	visaFlume.write( "OUTPUT" + str( channel ) + " ON" );
}


// set the agilent to output a square wave.
void Agilent::setSquare( int channel, squareInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower ( "Bad Value for Channel in setSquare!" );
	}
	visaFlume.write( "SOURCE" + str(channel) + ":APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
					 + str( convertPowerToSetPoint(info.amplitude, info.useCalibration, calibrationCoefficients ) ) + " VPP, "
					 + str( convertPowerToSetPoint(info.offset, info.useCalibration, calibrationCoefficients )) + " V" );
}


void Agilent::setSine( int channel, sineInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower ( "Bad value for channel in setSine" );
	}
	visaFlume.write( "SOURCE" + str(channel) + ":APPLY:SINUSOID " + str( info.frequency ) + " KHZ, "
					 + str( convertPowerToSetPoint(info.amplitude, info.useCalibration, calibrationCoefficients ) ) + " VPP" );
}

// stuff that only has to be done once.
void Agilent::prepAgilentSettings(UINT channel)
{
	if (channel != 1 && channel != 2)
	{
		thrower ( "Bad value for channel in prepAgilentSettings!" );
	}
	// Set timout, sample rate, filter parameters, trigger settings.
	visaFlume.setAttribute( VI_ATTR_TMO_VALUE, 40000 );	
	visaFlume.write ( "SOURCE1:FUNC:ARB:SRATE " + str ( sampleRate ) );
	visaFlume.write ( "SOURCE2:FUNC:ARB:SRATE " + str ( sampleRate ) );
}


void Agilent::handleScriptVariation( UINT variation, scriptedArbInfo& scriptInfo, UINT channel,  
									 std::vector<parameterType>& variables)
{
	prepAgilentSettings( channel );
	programSetupCommands ( );

	if ( scriptInfo.wave.isVaried( ) || variation == 0 )
	{
		UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber( );
		scriptInfo.wave.replaceVarValues( variation, variables );
		// Loop through all segments
		for ( UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++ )
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData( segNumInc, sampleRate );
			}
			catch ( Error& )
			{
				throwNested( "IntensityWaveform.writeData threw an error! Error occurred in segment #" 
							 + str( totalSegmentNumber ) );
			}
		}
		// order matters.
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages( scriptInfo.useCalibration, calibrationCoefficients );
		scriptInfo.wave.calcMinMax( );
		scriptInfo.wave.minsAndMaxes.resize( variation + 1 );
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt( );
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt( );
		scriptInfo.wave.normalizeVoltages( );
		visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
		/// new line here:
		prepAgilentSettings( channel );
		for ( UINT segNumInc : range( totalSegmentNumber ) )
		{
			visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, variation, 
																			 totalSegmentNumber, channel ) );
			// Save the segment
			visaFlume.write( "MMEM:STORE:DATA" + str( channel ) + " \"" + memoryLoc + ":\\segment"
								+ str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
		}
		scriptInfo.wave.compileSequenceString( totalSegmentNumber, variation, channel );
		// submit the sequence
		visaFlume.write( scriptInfo.wave.returnSequenceString( ) );
		// Save the sequence
		visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB sequence" + str( variation ) );
		visaFlume.write( "MMEM:STORE:DATA" + str( channel ) + " \"" + memoryLoc + ":\\sequence" 
						 + str( variation ) + ".seq\"" );
		// clear temporary memory.
		visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
	}	
}


/*
 * This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
 */
void Agilent::setScriptOutput( UINT varNum, scriptedArbInfo scriptInfo, UINT chan )
{
	if (scriptInfo.wave.isVaried() || varNum == 0)
	{
		prepAgilentSettings( chan );
		// check if effectively dc
		if ( scriptInfo.wave.minsAndMaxes.size ( ) == 0 )
		{
			thrower ( "script wave min max size is zero???" );
		}
		auto & minMaxs = scriptInfo.wave.minsAndMaxes[varNum];
		if ( fabs( minMaxs.first - minMaxs.second ) < 1e-6 )
		{
			dcInfo tempDc;
			tempDc.dcLevel = minMaxs.first;
			tempDc.useCalibration = scriptInfo.useCalibration;
			setDC( chan, tempDc );
		}
		else
		{
			auto schan = "SOURCE" + str ( chan );
			// Load sequence that was previously loaded.
			visaFlume.write( "MMEM:LOAD:DATA" + str( chan ) + " \"" + memoryLoc + ":\\sequence" + str( varNum ) + ".seq\"" );
			visaFlume.write( schan + ":FUNC ARB" );
			visaFlume.write( schan + ":FUNC:ARB \"" + memoryLoc + ":\\sequence" + str( varNum ) + ".seq\"" );
			// set the offset and then the low & high. this prevents accidentally setting low higher than high or high 
			// higher than low, which causes agilent to throw annoying errors.
			visaFlume.write( schan + ":VOLT:OFFSET " + str( (minMaxs.first + minMaxs.second) / 2 ) + " V" );
			visaFlume.write( schan + ":VOLT:LOW " + str( minMaxs.first ) + " V" );
			visaFlume.write( schan + ":VOLT:HIGH " + str( minMaxs.second ) + " V" );
			visaFlume.write( "OUTPUT" + str( chan ) + " ON" );
		}
	}
}


bool Agilent::scriptingModeIsSelected( ) 
{
	return settings.channel[currentChannel-1].option == AgilentChannelMode::which::Script;
}


void Agilent::setAgilent( UINT variation, std::vector<parameterType>& params)
{
	if ( !connected( ) )
	{
		return;
	}
	try
	{
		visaFlume.write( "OUTPut:SYNC " + str( settings.synced ) );
	}
	catch ( Error& )
	{
		visaFlume.write( "OUTPut:SYNC " + str( settings.synced ) );
	}
	for (auto chan : range( UINT(2) ))
	{
		auto& channel = settings.channel[chan];
		try
		{
			convertInputToFinalSettings( chan, params, variation );
			switch ( channel.option )
			{
				case AgilentChannelMode::which::No_Control:
					break;
				case AgilentChannelMode::which::Output_Off:
					outputOff( chan + 1 );
					break;
				case AgilentChannelMode::which::DC:
					setDC( chan + 1, channel.dc );
					break;
				case AgilentChannelMode::which::Sine:
					setSine( chan + 1, channel.sine );
					break;
				case AgilentChannelMode::which::Square:
					setSquare( chan + 1, channel.square );
					break;
				case AgilentChannelMode::which::Preloaded:
					setExistingWaveform( chan + 1, channel.preloadedArb );
					break;
				case AgilentChannelMode::which::Script:
					setScriptOutput( variation, channel.scriptedArb, chan + 1 );
					break;
				default:
					thrower ( "unrecognized channel " + str(chan) + " setting: " 
							  + AgilentChannelMode::toStr( channel.option ) );
			}
		}
		catch ( Error& err )
		{
			throwNested( "Error seen while programming agilent output for " + configDelim + " agilent channel " 
						  + str( chan+1) + ": " + err.whatBare( ) );
		}
	}
}


void Agilent::setAgilent()
{
	if (!connected())
	{
		return;
	}
	visaFlume.write( "OUTPut:SYNC " + str( settings.synced ) );
	for (auto chan : range( UINT(2) ))
	{
		if (settings.channel[chan].option == AgilentChannelMode::which::Script )
		{
			analyzeAgilentScript(settings.channel[chan].scriptedArb, std::vector<parameterType>());
		}
		convertInputToFinalSettings(chan);
		switch (settings.channel[chan].option)
		{
			case AgilentChannelMode::which::No_Control:
				break;
			case AgilentChannelMode::which::Output_Off:
				outputOff( chan+1 );
				break;
			case AgilentChannelMode::which::DC:
				setDC( chan+1, settings.channel[chan].dc );
				break;
			case AgilentChannelMode::which::Sine:
				setSine( chan+1, settings.channel[chan].sine );
				break;
			case AgilentChannelMode::which::Square:
				setSquare( chan+1, settings.channel[chan].square );
				break;
			case AgilentChannelMode::which::Preloaded:
				setExistingWaveform( chan+1, settings.channel[chan].preloadedArb );
				break;
			case AgilentChannelMode::which::Script:
				setScriptOutput( 0, settings.channel[chan].scriptedArb, chan+1 );
				break;
			default:
				thrower ( "unrecognized channel " + str(chan+1) + " setting: " + AgilentChannelMode::toStr(settings.channel[chan].option ) );
		}
	}
}


