#include "stdafx.h"

#include "Agilent.h"
#include "VariableSystem.h"
#include "ScriptStream.h"
#include "ProfileSystem.h"
#include "AuxiliaryWindow.h"

#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>

// NI's visa file. Also gets indirectly included via #include "nifgen.h".
#include "visa.h"

Agilent::Agilent( const agilentSettings& settings ) : visaFlume( settings.safemode, settings.address ),
													  sampleRate( settings.sampleRate ),
													  load( settings.outputImpedance ),
													  filterState( settings.filterState ),
													  initSettings( settings ),
													  triggerRow(settings.triggerRow ), 
													  triggerNumber( settings.triggerNumber ),
													  memoryLocation(settings.memoryLocation )

{
	visaFlume.open(); 
}


Agilent::~Agilent()
{
	visaFlume.close();
}



void Agilent::initialize( POINT& loc, cToolTips& toolTips, CWnd* parent, int& id, std::string headerText,
						  UINT editHeight, COLORREF color )
{
	name = headerText;
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

	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( cstr( headerText ), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++ );
	header.fontType = HeadingFont;

	deviceInfoDisplay.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	deviceInfoDisplay.Create( cstr( deviceInfo ), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, deviceInfoDisplay.sPos,
							  parent, id++ );
	deviceInfoDisplay.fontType = SmallFont;

	channel1Button.sPos = { loc.x, loc.y, loc.x += 240, loc.y + 20 };
	channel1Button.Create( "Channel 1 - No Control", BS_AUTORADIOBUTTON | WS_GROUP | WS_VISIBLE | WS_CHILD, 
						   channel1Button.sPos, parent, initSettings.chan1ButtonId );
	channel1Button.SetCheck( true );

	channel2Button.sPos = { loc.x, loc.y, loc.x += 240, loc.y += 20 };
	channel2Button.Create( "Channel 2 - No Control", BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD, channel2Button.sPos, 
						   parent, initSettings.chan2ButtonId );
	loc.x -= 480;

	syncedButton.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 20 };
	syncedButton.Create( "Synced?", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, syncedButton.sPos, parent,
						 initSettings.syncButtonId );

	calibratedButton.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 20 };
	calibratedButton.Create( "Use Cal?", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, calibratedButton.sPos, 
							 parent, initSettings.calibrationButtonId );
	calibratedButton.SetCheck( true );
	
	programNow.sPos = { loc.x, loc.y, loc.x += 160, loc.y += 20 };
	programNow.Create( "Program", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, programNow.sPos, parent, 
					   initSettings.programButtonId );

	loc.x -= 480;

	settingCombo.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 200 };
	settingCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, settingCombo.sPos,
						 parent, initSettings.agilentComboId );
	settingCombo.AddString( "No Control" );
	settingCombo.AddString( "Output Off" );
	settingCombo.AddString( "DC" );
	settingCombo.AddString( "Sine" );
	settingCombo.AddString( "Square" );
	settingCombo.AddString( "Preloaded" );
	settingCombo.AddString( "Scripted" );
	settingCombo.SetCurSel( 0 );

	optionsFormat.sPos = { loc.x, loc.y, loc.x += 360, loc.y += 25 };
	optionsFormat.Create( "---", WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsFormat.sPos, parent, id++ );
	loc.x -= 480;

	agilentScript.initialize( 480, editHeight, loc, toolTips, parent, id, "Agilent", "",
							  { initSettings.functionComboId, initSettings.editId }, color );

	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
	currentChannel = 1;
}


void Agilent::checkSave( std::string categoryPath, RunInfo info )
{
	if ( settings.channel[currentChannel-1].option == 4 )
	{
		agilentScript.checkSave( categoryPath, info );
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


std::string Agilent::getName()
{
	return name;
}


/**
 * This function tells the agilent to put out the DC default waveform.
 */
void Agilent::setDefault( int channel )
{
	// turn it to the default voltage...
	std::string setPointString = str(convertPowerToSetPoint(AGILENT_DEFAULT_POWER, true));
	visaFlume.write( "SOURce" + str(channel) + ":APPLy:DC DEF, DEF, " + setPointString + " V" );
}



/**
 * expects the inputted power to be in -MILI-WATTS! 
 * returns set point in VOLTS
 */
double Agilent::convertPowerToSetPoint(double powerInMilliWatts, bool conversionOption)
{
	/// IMPORTANT CONVENTION NOTE:
	// the log-PD calibrations were all done with the power in microwatts. This goes against all other conventions in 
	// the code however, so as I calibrate the linPD, I'm changin to MILLI-watts.

	/// OLD LOG-PD CALIBRATIONS
	// this calibration is 
	// TODO:make a structure and a front panel option. 
	// need to implement using calibrations, which haven't been done yet.
	// HARD CODED right now.
	// Expecting a calibration in terms of /MICROWATTS/!
	/// double newValue = -a * log(y * b);
	// (February 1st, 2016 calibrations)
	// double a = 0.245453772102427, b = 1910.3567515711145;
	// (February 2st, 2016 calibrations)
	// double a = 0.2454742248, b = 1684.849955;
	// (April 14th, 2016 calibrations);
	// double a = 0.24182, b = 1943.25;
	// (June 16th, 2016 calibrations (NE10 filter in front of log pd)
	//double a = 0.247895, b = 218.559;
	// June 18th, 2016 calibrations (NE20 filter in front of log pd)
	// double a = 0.262771, b = 11.2122;
	// June 22nd AM, 2016 calibrations (No filter in front of log pd)
	//double a = 0.246853, b = 1330.08;
	// June 22nd PM, 2016 calibrations (NE10 filter in front of log pd, after tweaking servo parameters)
	//double a = 0.246862, b = 227.363;

	/// CLIBRATIOND WITH DIGITAL LOCK BOX
	/// newValue = a +  b * log(y - info); // here info is treated as a background light level, and the voltage output 
	/// should be positive
	// July 14 2016, NE10 filter in front of log pd
	// double a = 0.479262, b = 0.215003, c = 0.018189;

	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// TERRY'S LINEAR PHOTODIODE CALIBRATIONS
	// August 30th, 2017
	//double a = -0.040063228;
	//double b = 0.000153625;
	// September 12th, 2017
	double a = -0.04090619;
	double b = 0.00641603;
	if ( conversionOption )
	{
		double setPointInVolts = a * powerInMilliWatts + b;
		return setPointInVolts;
	}
	else
	{
		// no conversion
		return powerInMilliWatts;
	}
}


void Agilent::analyzeAgilentScript( scriptedArbInfo& infoObj, std::vector<variableType>& variables)
{
	// open the file
	std::ifstream scriptFile( infoObj.fileAddress );
	if (!scriptFile.is_open())
	{
		thrower( "ERROR: Scripted Agilent File \"" + infoObj.fileAddress + "\" failed to open!" );
	}
	ScriptStream stream;
	stream << scriptFile.rdbuf();
	stream.seekg( 0 );
	int currentSegmentNumber = 0;

	infoObj.wave.resetNumberOfTriggers( );
	while (!stream.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = infoObj.wave.analyzeAgilentScriptCommand( currentSegmentNumber, stream, variables );

		if (leaveTest < 0)
		{
			thrower( "ERROR: IntensityWaveform.analyzeAgilentScriptCommand threw an error! Error occurred in segment #"
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


std::pair<UINT, UINT> Agilent::getTriggerLine( )
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
		msg == err.what();
	}
	if ( msg == "" )
	{
		msg = "Disconnected...\n";
	}
	return msg;
}

HBRUSH Agilent::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	DWORD id = window->GetDlgCtrlID();
	if ( id == deviceInfoDisplay.GetDlgCtrlID() || id == channel1Button.GetDlgCtrlID()  
		 || id == channel2Button.GetDlgCtrlID() || id == syncedButton.GetDlgCtrlID() 
		 || id == settingCombo.GetDlgCtrlID() || id == optionsFormat.GetDlgCtrlID() )
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Solarized Base2"]);
		return *brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


void Agilent::handleInput(int chan, std::string categoryPath, RunInfo info)
{
	if (chan != 1 && chan != 2)
	{
		thrower( "ERROR: Bad argument for agilent channel in Agilent::handleInput(...)!" );
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
		case -2:
			// no control.
			break;
		case -1:
			// output off.
			break;
		case 0:
			// DC.
			stream >> settings.channel[chan].dc.dcLevelInput;
			settings.channel[chan].dc.useCalibration = calibratedButton.GetCheck( );
			break;
		case 1:
			// sine wave
			stream >> settings.channel[chan].sine.frequencyInput;
			stream >> settings.channel[chan].sine.amplitudeInput;
			settings.channel[chan].sine.useCalibration = calibratedButton.GetCheck( );
			break;
		case 2:
			stream >> settings.channel[chan].square.frequencyInput;
			stream >> settings.channel[chan].square.amplitudeInput;
			stream >> settings.channel[chan].square.offsetInput;
			settings.channel[chan].square.useCalibration = calibratedButton.GetCheck( );
			break;
		case 3:
			stream >> settings.channel[chan].preloadedArb.address;
			settings.channel[chan].preloadedArb.useCalibration = calibratedButton.GetCheck( );
			break;
		case 4:
			agilentScript.checkSave( categoryPath, info );
			settings.channel[chan].scriptedArb.fileAddress = agilentScript.getScriptPathAndName();
			settings.channel[chan].scriptedArb.useCalibration = calibratedButton.GetCheck( );
			break;
		default:
			thrower( "ERROR: unknown agilent option" );
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
	if ( chan == 1 )
	{
		channelText = "Channel 1 - ";
	}
	else
	{
		channelText = "Channel 2 - ";
	}
	switch ( settings.channel[chan-1].option )
	{
		case -2:
			channelText += "No Control";
			break;
		case -1:
			channelText += "Output Off";
			break;
		case 0:
			channelText += "DC";
			break;
		case 1:
			channelText += "Sine";
			break;
		case 2:
			channelText += "Square";
			break;
		case 3:
			channelText += "Pre-programmed";
			break;
		case 4:
			channelText += "Scripted";
			break;
		default:
			channelText += "Unrecognized Option???";
	}
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
		case -2:
			agilentScript.setScriptText("");
			settingCombo.SetCurSel( 0 );
			break;
		case -1:
			agilentScript.setScriptText("");
			settingCombo.SetCurSel( 1 );
			break;
		case 0:
			// dc
			agilentScript.setScriptText(settings.channel[chan].dc.dcLevelInput.expressionStr);
			settingCombo.SetCurSel( 2 );
			calibratedButton.SetCheck( settings.channel[chan].dc.useCalibration );
			break;
		case 1:
			// sine
			agilentScript.setScriptText(settings.channel[chan].sine.frequencyInput.expressionStr + " " 
										 + settings.channel[chan].sine.amplitudeInput.expressionStr);
			settingCombo.SetCurSel( 3 );
			calibratedButton.SetCheck( settings.channel[chan].sine.useCalibration );
			break;
		case 2:
			// square
			agilentScript.setScriptText( settings.channel[chan].square.frequencyInput.expressionStr + " " 
										 + settings.channel[chan].square.amplitudeInput.expressionStr + " " 
										 + settings.channel[chan].square.offsetInput.expressionStr );
			settingCombo.SetCurSel( 4 );
			calibratedButton.SetCheck( settings.channel[chan].square.useCalibration );
			break;
		case 3:
			// preprogrammed
			agilentScript.setScriptText(settings.channel[chan].preloadedArb.address);
			settingCombo.SetCurSel( 5 );
			calibratedButton.SetCheck( settings.channel[chan].preloadedArb.useCalibration );
			break;
		case 4:
			// scripted
			settingCombo.SetCurSel( 6 );
			// clear it in case the file fails to open.
			agilentScript.setScriptText( "" );
			agilentScript.openParentScript( settings.channel[chan].scriptedArb.fileAddress, currentCategoryPath, 
											currentRunInfo );
			calibratedButton.SetCheck( settings.channel[chan].scriptedArb.useCalibration );
			break;
		default:
			thrower( "ERROR: unrecognized agilent setting: " + settings.channel[chan].option);
	}
	if ( chan == 0 )
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
	if (channel1Button.GetCheck())
	{
		currentChannel = 1;
	}
	else
	{
		currentChannel = 2;
	}
}


void Agilent::handleCombo()
{
	int selection = settingCombo.GetCurSel();
	int selectedChannel = int( !channel1Button.GetCheck() );
	switch (selection)
	{
		case 0:
			// do nothing
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = -2;
			break;
		case 1:
			// do nothing
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = -1;
			break;
		case 2:
			optionsFormat.SetWindowTextA( "[DC Level]" );
			settings.channel[selectedChannel].option = 0;
			break;
		case 3:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)]" );
			settings.channel[selectedChannel].option = 1;
			break;
		case 4:
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)] [Offset(V)]" );
			settings.channel[selectedChannel].option = 2;
			break;
		case 5:
			optionsFormat.SetWindowTextA( "[File Address]" );
			settings.channel[selectedChannel].option = 3;
			break;
		case 6:
			optionsFormat.SetWindowTextA( "Hover over \"?\"" );
			settings.channel[selectedChannel].option = 4;
			break;
	}
}


deviceOutputInfo Agilent::getOutputInfo()
{
	return settings;
}


void Agilent::convertInputToFinalSettings( UINT chan, UINT variation, std::vector<variableType>& variables)
{
	// iterate between 0 and 1...
	channelInfo& channel( settings.channel[chan] );
	try
	{
		switch (channel.option)
		{
			case -2:
				// no control
				break;
			case -1:
				// no ouput
				break;
			case 0:
				// DC output
				channel.dc.dcLevel = channel.dc.dcLevelInput.evaluate( variables, variation );
				break;
			case 1:
				// single frequency output
				// frequency
				channel.sine.frequency = channel.sine.frequencyInput.evaluate( variables, variation );
				// amplitude
				channel.sine.amplitude = channel.sine.amplitudeInput.evaluate( variables, variation );
				break;
			case 2:
				// Square Output
				// frequency
				channel.square.frequency = channel.square.frequencyInput.evaluate( variables, variation );
				// amplitude
				channel.square.amplitude = channel.square.amplitudeInput.evaluate( variables, variation );
				channel.square.offset = channel.square.offsetInput.evaluate( variables, variation );
				break;
			case 3:
				// Preloaded Arb Output... no variations possible...
				break;
			case 4:
				// Scripted Arb Output... 
				handleScriptVariation( variation, channel.scriptedArb, chan+1, variables );
				break;
			default:
				thrower( "Unrecognized Agilent Setting: " + str( channel.option ) );
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}


// version without variables
void Agilent::convertInputToFinalSettings(UINT chan)
{
	try
	{
		switch (settings.channel[chan].option)
		{
			case -2:
				// no control
				break;
			case -1:
				// no ouput
				break;
			case 0:
				// DC output
				settings.channel[chan].dc.dcLevel = settings.channel[chan].dc.dcLevelInput.evaluate( );
				break;
			case 1:
				// single frequency output
				settings.channel[chan].sine.frequency = settings.channel[chan].sine.frequencyInput.evaluate();
				settings.channel[chan].sine.amplitude = settings.channel[chan].sine.amplitudeInput.evaluate();
				break;
			case 2:
				// Square Output
				settings.channel[chan].square.frequency = settings.channel[chan].square.frequencyInput.evaluate( );
				settings.channel[chan].square.amplitude = settings.channel[chan].square.amplitudeInput.evaluate( );
				settings.channel[chan].square.offset = settings.channel[chan].square.offsetInput.evaluate( );
				break;
			case 3:
				// Preloaded Arb Output... no variations possible...
				break;
			case 4:
				// Scripted Arb Output... 
				handleNoVariations( settings.channel[chan].scriptedArb, chan+1 );
				break;
			default:
				thrower( "Unrecognized Agilent Setting: " + str( settings.channel[chan].option ) );
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}


void Agilent::handleNewConfig( std::ofstream& newFile )
{
	// make sure data is up to date.
	//handleInput();
	// start outputting.
	newFile << "AGILENT\n";
	newFile << "0\n";
	newFile << "CHANNEL_1\n";
	newFile << "-2\n";
	newFile << "0\n";
	newFile << "0\n";
	
	newFile << "0\n";
	newFile << "1\n";
	newFile << "0\n";

	newFile << "0\n";
	newFile << "1\n";
	newFile << "0\n";
	newFile << "0\n";
	newFile << "NONE\n";
	newFile << "0\n";
	newFile << "NONE\n";
	newFile << "0\n";
	newFile << "CHANNEL_2\n";
	newFile << "-2\n";
	newFile << "0\n";
	newFile << "0\n";

	newFile << "0\n";
	newFile << "1\n";
	newFile << "0\n";

	newFile << "0\n";
	newFile << "1\n";
	newFile << "0\n";
	newFile << "0\n";

	newFile << "NONE\n";
	newFile << "0\n";
	newFile << "NONE\n";
	newFile << "0\n";	
	newFile << "END_AGILENT\n";
}


/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
void Agilent::handleSavingConfig(std::ofstream& saveFile, std::string categoryPath, RunInfo info)
{	
	// make sure data is up to date.
	handleInput( currentChannel, categoryPath, info);
	// start outputting.
	saveFile << "AGILENT\n";
	saveFile << str(settings.synced) << "\n";
	saveFile << "CHANNEL_1\n";
	saveFile << str(settings.channel[0].option) + "\n";
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
	saveFile << str( settings.channel[1].option ) + "\n";
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
	saveFile << "END_AGILENT\n";
}


void Agilent::readConfigurationFile( std::ifstream& file, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine(file, "AGILENT");
	file >> settings.synced;
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_1");
	// the extra step in all of the following is to remove the , at the end of each input.
	std::string input;
	file >> input;
	file.get();
	try
	{
		settings.channel[0].option = std::stoi( input );
	}
	catch (std::invalid_argument&)
	{
		thrower( "ERROR: Bad channel 1 option!" );
	}
	std::string calibratedOption;
	std::getline( file, settings.channel[0].dc.dcLevelInput.expressionStr );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[0].dc.useCalibration = calOption;
	}
	std::getline( file, settings.channel[0].sine.amplitudeInput.expressionStr );
	std::getline( file, settings.channel[0].sine.frequencyInput.expressionStr );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[0].sine.useCalibration = calOption;
	}
	std::getline( file, settings.channel[0].square.amplitudeInput.expressionStr );
	std::getline( file, settings.channel[0].square.frequencyInput.expressionStr );
	std::getline( file, settings.channel[0].square.offsetInput.expressionStr );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[0].square.useCalibration = calOption;
	}
	std::getline( file, settings.channel[0].preloadedArb.address);
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[0].preloadedArb.useCalibration = calOption;
	}
	std::getline( file, settings.channel[0].scriptedArb.fileAddress );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[0].scriptedArb.useCalibration = calOption;
	}
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_2"); 
	file >> input;
	file.get( );
	try
	{
		settings.channel[1].option = std::stoi(input);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Bad channel 1 option!");
	}
	std::getline( file, settings.channel[1].dc.dcLevelInput.expressionStr );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[1].dc.useCalibration = calOption;
	}
	std::getline( file, settings.channel[1].sine.amplitudeInput.expressionStr );
	std::getline( file, settings.channel[1].sine.frequencyInput.expressionStr );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[1].sine.useCalibration = calOption;
	}
	std::getline( file, settings.channel[1].square.amplitudeInput.expressionStr );
	std::getline( file, settings.channel[1].square.frequencyInput.expressionStr );
	std::getline( file, settings.channel[1].square.offsetInput.expressionStr );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[1].square.useCalibration = calOption;
	}
	std::getline( file, settings.channel[1].preloadedArb.address);
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[1].preloadedArb.useCalibration = calOption;
	}
	std::getline( file, settings.channel[1].scriptedArb.fileAddress );
	if ( (versionMajor == 2 && versionMinor > 3) || versionMajor > 2 )
	{
		std::getline( file, calibratedOption );
		bool calOption;
		try
		{
			calOption = bool( std::stoi( calibratedOption ) );
		}
		catch ( std::invalid_argument& )
		{
			calOption = false;
		}
		settings.channel[1].scriptedArb.useCalibration = calOption;
	}
	ProfileSystem::checkDelimiterLine(file, "END_AGILENT");

	updateButtonDisplay( 1 );
	updateButtonDisplay( 2 );
}


void Agilent::outputOff( int channel )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: bad value for channel inside outputOff!" );
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
		thrower( "ERROR: Bad value for channel inside setDC!" );
	}
	visaFlume.write( "SOURce" + str( channel ) + ":APPLy:DC DEF, DEF, " + str( convertPowerToSetPoint(info.dcLevel, info.useCalibration) ) + " V" );
}


void Agilent::setExistingWaveform( int channel, preloadedArbInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel in setExistingWaveform!" );
	}
	visaFlume.write( "SOURCE" + str(channel) + ":DATA:VOL:CLEAR" );
	// Load sequence that was previously loaded.
	visaFlume.write( "MMEM:LOAD:DATA \"" + info.address + "\"" );
	// tell it that it's outputting something arbitrary (not sure if necessary)
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC ARB" );
	// tell it what arb it's outputting.
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB \"" + memoryLocation + ":\\" + info.address + "\"" );
	// Set output impedance...
	visaFlume.write( str( "OUTPUT" + str( channel ) + ":LOAD " ) + load );
	// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::MODE TRIGGERED" );
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::NCYCLES 1" );
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::PHASE 0" );
	visaFlume.write( "SOURCE" + str( channel ) + ":BURST::STATE ON" );
	visaFlume.write( "OUTPUT" + str( channel ) + " ON" );
}


// set the agilent to output a square wave.
void Agilent::setSquare( int channel, squareInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad Value for Channel in setSquare!" );
	}
	visaFlume.write( "SOURCE" + str(channel) + ":APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
					 + str( convertPowerToSetPoint(info.amplitude, info.useCalibration ) ) + " VPP, "
					 + str( convertPowerToSetPoint(info.offset, info.useCalibration )) + " V" );
}


void Agilent::setSine( int channel, sineInfo info )
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel in setSine" );
	}
	visaFlume.write( "SOURCE" + str(channel) + ":APPLY:SINUSOID " + str( info.frequency ) + " KHZ, "
					 + str( convertPowerToSetPoint(info.amplitude, info.useCalibration ) ) + " VPP" );
}

// stuff that only has to be done once.
void Agilent::prepAgilentSettings(UINT channel)
{
	if (channel != 1 && channel != 2)
	{
		thrower( "ERROR: Bad value for channel in prepAgilentSettings!" );
	}
	// Set timout, sample rate, filter parameters, trigger settings.
	visaFlume.setAttribute( VI_ATTR_TMO_VALUE, 40000 );	
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB:FILTER " + filterState );
	visaFlume.write( "SOURCE" + str(channel) + ":FUNC:ARB:SRATE " + str( sampleRate ) );
	visaFlume.write( "TRIGGER" + str( channel ) + ":SOURCE EXTERNAL" );
	visaFlume.write( "TRIGGER" + str( channel ) + ":SLOPE POSITIVE" );
	visaFlume.write( "OUTPUT" + str( channel ) + ":LOAD " + load );
}


void Agilent::handleScriptVariation( UINT variation, scriptedArbInfo& scriptInfo, UINT channel,  
									 std::vector<variableType>& variables)
{
	// Initialize stuff
	prepAgilentSettings( channel );
	// if varied
	if ( scriptInfo.wave.isVaried( ) || variation == 0 )
	{
		UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber( );
		// replace variable values where found
		scriptInfo.wave.replaceVarValues( variation, variables );
		// Loop through all segments
		for ( UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++ )
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData( segNumInc, sampleRate );
			}
			catch ( Error& err )
			{
				thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
						 + str( totalSegmentNumber ) + ": " + err.what( ) );
			}
		}
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages( scriptInfo.useCalibration );
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
			visaFlume.write( "MMEM:STORE:DATA" + str( channel ) + " \"" + memoryLocation + ":\\segment"
								+ str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
			//visaFlume.write( "MMEM:STORE:DATA" + str( channel ) + " \"" + memoryLocation + ":\\segment"
			//				 + str( segNumInc ) + ".arb\"" );
		}
		// Now handle seqeunce creation / writing.
		scriptInfo.wave.compileSequenceString( totalSegmentNumber, variation, channel );
		// submit the sequence
		visaFlume.write( scriptInfo.wave.returnSequenceString( ) );
		// Save the sequence
		visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB sequence" + str( variation ) );
		visaFlume.write( "MMEM:STORE:DATA" + str( channel ) + " \"" + memoryLocation + ":\\sequence" 
						 + str( variation ) + ".seq\"" );
		// clear temporary memory.
		visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
	}	
}


void Agilent::handleNoVariations(scriptedArbInfo& scriptInfo, UINT channel)
{
	// Initialize stuff
	prepAgilentSettings(channel);

	UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber();
	scriptInfo.wave.replaceVarValues();
	// else not varying. Loop through all segments once.
	for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
	{
		// Use that information to write the data.
		try
		{
			scriptInfo.wave.writeData( segNumInc, sampleRate );
		}
		catch (Error& err)
		{
			thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
					 + str( totalSegmentNumber ) + "." + err.what() );
		}
	}
	// no reassignment nessesary, no variables
	scriptInfo.wave.convertPowersToVoltages( scriptInfo.useCalibration );
	scriptInfo.wave.calcMinMax();
	scriptInfo.wave.minsAndMaxes.resize( 1 );
	scriptInfo.wave.minsAndMaxes[0].second = scriptInfo.wave.getMaxVolt();
	scriptInfo.wave.minsAndMaxes[0].first = scriptInfo.wave.getMinVolt();
	scriptInfo.wave.normalizeVoltages();
	visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
	/// new line here
	prepAgilentSettings( channel );
	for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
	{
		visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, 0, totalSegmentNumber, channel ) );
		visaFlume.write( "MMEM:STORE:DATA" + str( channel ) + " \"" + memoryLocation + ":\\chan" + str(channel) + "arb" + str( segNumInc ) + ".arb\"" );
	}
	// Now handle seqeunce creation / writing.
	scriptInfo.wave.compileSequenceString( totalSegmentNumber, 0, channel );
	// submit the sequence
	visaFlume.write( scriptInfo.wave.returnSequenceString() );
	// Save the sequence
	visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB sequence0");
	visaFlume.write( "MMEM:STORE:DATA" + str(channel) + " \"" + memoryLocation + ":\\sequence0.seq\"" );
	// clear temporary memory.
	visaFlume.write( "SOURCE" + str( channel ) + ":DATA:VOL:CLEAR" );
}


/*
 * This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
 */
void Agilent::setScriptOutput( UINT varNum, scriptedArbInfo scriptInfo, UINT channel )
{
	if (scriptInfo.wave.isVaried() || varNum == 0)
	{
		prepAgilentSettings( channel );
		// check if effectively dc
		if ( fabs( scriptInfo.wave.minsAndMaxes[varNum].first - scriptInfo.wave.minsAndMaxes[varNum].second ) < 1e-6 )
		{
			dcInfo tempDc;
			tempDc.dcLevel = scriptInfo.wave.minsAndMaxes[varNum].first;
			tempDc.useCalibration = scriptInfo.useCalibration;
			setDC( channel, tempDc );
		}
		else
		{
			// Load sequence that was previously loaded.
			visaFlume.write( "MMEM:LOAD:DATA" + str( channel ) + " \"" + memoryLocation + ":\\sequence" 
							 + str( varNum ) + ".seq\"" );
			visaFlume.write( "SOURCE" + str( channel ) + ":FUNC ARB" );
			visaFlume.write( "SOURCE" + str( channel ) + ":FUNC:ARB \"" + memoryLocation + ":\\sequence" 
							 + str( varNum ) + ".seq\"" );
			// set the offset and then the low & high. this prevents accidentally setting low higher than high or high 
			// higher than low, which causes agilent to throw annoying errors.
			visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:OFFSET "
							 + str( (scriptInfo.wave.minsAndMaxes[varNum].first
									  + scriptInfo.wave.minsAndMaxes[varNum].second) / 2 ) + " V" );
			visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:LOW " + str( scriptInfo.wave.minsAndMaxes[varNum].first ) + " V" );
			visaFlume.write( "SOURCE" + str( channel ) + ":VOLT:HIGH " + str( scriptInfo.wave.minsAndMaxes[varNum].second ) + " V" );
			visaFlume.write( "OUTPUT" + str( channel ) + " ON" );
		}
	}
}


void Agilent::setAgilent( UINT variation, std::vector<variableType>& variables)
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
	for (auto chan : range( 2 ))
	{
		try
		{
			if ( settings.channel[chan].option == 4 )
			{
				// need to do this before converting to final settings
				analyzeAgilentScript( settings.channel[chan].scriptedArb, variables );
			}
			convertInputToFinalSettings( chan, variation, variables );
			switch ( settings.channel[chan].option )
				{
				case -2:
					// don't do anything.
					break;
				case -1:
					outputOff( chan + 1 );
					break;
				case 0:
					setDC( chan + 1, settings.channel[chan].dc );
					break;
				case 1:
					setSine( chan + 1, settings.channel[chan].sine );
					break;
				case 2:
					setSquare( chan + 1, settings.channel[chan].square );
					break;
				case 3:
					setExistingWaveform( chan + 1, settings.channel[chan].preloadedArb );
					break;
				case 4:
					setScriptOutput( variation, settings.channel[chan].scriptedArb, chan + 1 );
					break;
				default:
					thrower( "ERROR: unrecognized channel 1 setting: " + str( settings.channel[chan].option ) );
			}
		}
		catch ( Error& err )
		{
			thrower( "Error seen while programming agilent output for " + name + " agilent channel " 
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
	for (auto chan : range( 2 ))
	{
		if (settings.channel[chan].option == 4)
		{
			analyzeAgilentScript(settings.channel[chan].scriptedArb, std::vector<variableType>());
		}
		convertInputToFinalSettings(chan);
		switch (settings.channel[chan].option)
		{
			case -2:
				// don't do anything.
				break;
			case -1:
				outputOff( chan+1 );
				break;
			case 0:
				setDC( chan+1, settings.channel[chan].dc );
				break;
			case 1:
				setSine( chan+1, settings.channel[chan].sine );
				break;
			case 2:
				setSquare( chan+1, settings.channel[chan].square );
				break;
			case 3:
				setExistingWaveform( chan+1, settings.channel[chan].preloadedArb );
				break;
			case 4:
				setScriptOutput( 0, settings.channel[chan].scriptedArb, chan+1 );
				break;
			default:
				thrower( "ERROR: unrecognized channel " + str(chan+1) + " setting: " + str(settings.channel[chan].option ) );
		}
	}
}
