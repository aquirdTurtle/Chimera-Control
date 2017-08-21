#include "stdafx.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
// NI's visa file. Also gets indirectly included via #include "nifgen.h".
#include "visa.h"
#include "Agilent.h"
#include "VariableSystem.h"
#include "ScriptStream.h"
#include "ProfileSystem.h"
#include "AuxiliaryWindow.h"


void Agilent::rearrange(UINT width, UINT height, fontMap fonts)
{
	// GUI ELEMENTS
	header.rearrange(width, height, fonts);
	deviceInfoDisplay.rearrange(width, height, fonts);
	channel1Button.rearrange(width, height, fonts);
	channel2Button.rearrange(width, height, fonts);
	syncedButton.rearrange(width, height, fonts);
	settingCombo.rearrange(width, height, fonts);
	optionsFormat.rearrange(width, height, fonts);
	agilentScript.rearrange(width, height, fonts);
	programNow.rearrange( width, height, fonts );
}


std::string Agilent::getName()
{
	return name;
}


/**
 * This function tells the agilent to put out the DC default waveform.
 */
void Agilent::setDefualt( int channel )
{
	visaFlume.open();
	// turn it to the default voltage...
	visaFlume.write( str( "APPLy:DC DEF, DEF, " ) + AGILENT_DEFAULT_DC );
	// and leave...
	visaFlume.close();
	// update current values
	currentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
	currentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);
}


void Agilent::analyzeAgilentScript( scriptedArbInfo& infoObj)
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
	while (!stream.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = infoObj.wave.readIntoSegment( currentSegmentNumber, stream );

		if (leaveTest < 0)
		{
			thrower( "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #"
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

/*
	* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
	*/
void Agilent::selectIntensityProfile(int varNum)
{
	// TODO: add checks for differnet types of programmings.
	// 
	if (varies || varNum == 0)
	{
		visaFlume.open();
		// Load sequence that was previously loaded.
		visaFlume.write("MMEM:LOAD:DATA \"INT:\\seq" + str(varNum) + ".seq\"");
		visaFlume.write( "SOURCE1:FUNC ARB");
		visaFlume.write( "SOURCE1:FUNC:ARB \"INT:\\seq" + str(varNum) + ".seq\"");
		// Set output impedance...
		visaFlume.write( str("OUTPUT1:LOAD ") + AGILENT_LOAD);
		visaFlume.write( str("SOURCE1:VOLT:LOW ") + str(ranges[varNum].min) + " V");
		visaFlume.write( str("SOURCE1:VOLT:HIGH ") + str(ranges[varNum].max) + " V");
		visaFlume.write( "OUTPUT1 ON" );
		// and leave...
		visaFlume.close();
	}
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


void Agilent::initialize( POINT& loc, cToolTips& toolTips, CWnd* parent, int& id, std::string address, 
						  std::string headerText, UINT editHeight, std::array<UINT, 7> ids )
{
	visaFlume.init( address );
	name = headerText;
	try
	{
		visaFlume.open();
		int errCode = 0;
		deviceInfo = visaFlume.identityQuery();
		isConnected = true;
	}
	catch (Error&)
	{
		deviceInfo = "Disconnected";
		isConnected = false;
	}
	
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( cstr(headerText), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++ );
	header.fontType = HeadingFont;

	deviceInfoDisplay.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	deviceInfoDisplay.Create( cstr(deviceInfo), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, deviceInfoDisplay.sPos, 
							 parent, id++ );
	deviceInfoDisplay.fontType = SmallFont;

	channel1Button.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
	channel1Button.Create( "Channel 1", BS_AUTORADIOBUTTON | WS_GROUP | WS_VISIBLE | WS_CHILD, channel1Button.sPos,
						  parent, ids[0] );
	channel1Button.SetCheck( true );
	
	channel2Button.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
	channel2Button.Create( "Channel 2", BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD, channel2Button.sPos, parent, ids[1] );

	syncedButton.sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
	syncedButton.Create( "Synced?", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, syncedButton.sPos, parent, ids[2]);

	programNow.sPos = { loc.x, loc.y, loc.x += 120, loc.y += 20 };
	programNow.Create( "Program Now", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, programNow.sPos, parent, ids[3] );
	
	loc.x -= 480;

	settingCombo.sPos = { loc.x, loc.y, loc.x += 240, loc.y + 200 };
	settingCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,  settingCombo.sPos, 
						 parent, ids[4] );
	settingCombo.AddString( "No Control" );
	settingCombo.AddString( "Output Off" );
	settingCombo.AddString( "DC Output" );
	settingCombo.AddString( "Single Frequency Output" );
	settingCombo.AddString( "Square Output" );
	settingCombo.AddString( "Preloaded Arbitrary Waveform" );
	settingCombo.AddString( "Scripted Arbitrary Waveform" );
	settingCombo.SetCurSel( 0 );

	optionsFormat.sPos = { loc.x, loc.y, loc.x += 240, loc.y += 25 };
	optionsFormat.Create( "---", WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsFormat.sPos, parent, id++ );
	loc.x -= 480;

	agilentScript.initialize( 480, editHeight, loc, toolTips, parent, id, "Agilent", "", { ids[5], ids[6] } );
	
	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
	currentChannel = 1;
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


void Agilent::handleProgramNow()
{
	handleInput();
	setAgilent();
}


void Agilent::handleInput(int chan)
{
	if (chan != 1 && chan != 2)
	{
		thrower( "ERROR: Bad argument for agilent channel in Agilent::handleInput(...)!" );
	}
	// convert to zero-indexed
	chan -= 1;

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
			break;
		case 1:
			// sine wave
			stream >> settings.channel[chan].sine.frequencyInput;
			stream >> settings.channel[chan].sine.amplitudeInput;
			break;
		case 2:
			stream >> settings.channel[chan].square.frequencyInput;
			stream >> settings.channel[chan].square.amplitudeInput;
			stream >> settings.channel[chan].square.offsetInput;
			break;
		case 3:
			stream >> settings.channel[chan].preloadedArb.address;
			break;
		case 4:
			settings.channel[chan].scriptedArb.fileAddress = agilentScript.getScriptPathAndName();
			analyzeAgilentScript( settings.channel[chan].scriptedArb );
			break;
		default:
			thrower( "ERROR: unknown agilent option" );
	}
}


// overload for handling whichever channel is currently selected.
void Agilent::handleInput()
{
	// true -> 0 + 1 = 1
	// false -> 1 + 1 = 2
	handleInput( (!channel1Button.GetCheck()) + 1);
}


void Agilent::updateEdit( std::string currentCategoryPath, RunInfo currentRunInfo )
{
	updateEdit( (!channel1Button.GetCheck()) + 1, currentCategoryPath, currentRunInfo );
}


void Agilent::updateEdit(int chan, std::string currentCategoryPath, RunInfo currentRunInfo)
{
	// convert to zero-indexed.
	chan -= 1;
	std::string tempStr;
	switch ( settings.channel[chan].option )
	{
		case -2:
			tempStr = "";
			settingCombo.SetCurSel( 0 );
			break;
		case -1:
			tempStr = "";
			settingCombo.SetCurSel( 1 );
			break;
		case 0:
			// dc
			tempStr = settings.channel[chan].dc.dcLevelInput;
			settingCombo.SetCurSel( 2 );
			break;
		case 1:
			// sine
			tempStr = settings.channel[chan].sine.frequencyInput + " " + settings.channel[chan].sine.amplitudeInput;
			settingCombo.SetCurSel( 3 );
			break;
		case 2:
			// square
			tempStr = settings.channel[chan].square.frequencyInput + " " + settings.channel[chan].square.amplitudeInput
				+ " " + settings.channel[chan].square.offsetInput;
			settingCombo.SetCurSel( 4 );
			break;
		case 3:
			// preprogrammed
			tempStr = settings.channel[chan].preloadedArb.address;
			settingCombo.SetCurSel( 5 );
			break;
		case 4:
			// scripted
			settingCombo.SetCurSel( 6 );
			agilentScript.openParentScript( settings.channel[chan].scriptedArb.fileAddress, currentCategoryPath, 
											currentRunInfo );
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
	agilentScript.setScriptText(tempStr);
}


void Agilent::handleChannelPress( int chan, std::string currentCategoryPath, RunInfo currentRunInfo )
{
	// convert from channel 1/2 to 0/1 to access the right array entr
	handleInput( currentChannel );
	updateEdit( chan, currentCategoryPath, currentRunInfo );
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
			optionsFormat.SetWindowTextA( "[Frequency(Hz)] [Amplitude(Vpp)] [Offset(]" );
			settings.channel[selectedChannel].option = 2;
			break;
		case 5:
			optionsFormat.SetWindowTextA( "[Address]" );
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


void Agilent::convertInputToFinalSettings( key variableKey, UINT variation )
{
	// iterate between 0 and 1...
	try
	{
		for (auto chan : range( 2 ))
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
					settings.channel[chan].dc.dcLevel = reduce( settings.channel[chan].dc.dcLevelInput,
																variableKey, variation );
					break;
				case 1:
					// single frequency output
					// frequency
					settings.channel[chan].sine.frequency = reduce( settings.channel[chan].sine.frequencyInput, 
																	variableKey, variation );
					// amplitude
					settings.channel[chan].sine.amplitude = reduce( settings.channel[chan].sine.amplitudeInput, 
																	variableKey, variation );
					break;
				case 2:
					// Square Output
					// frequency
					settings.channel[chan].square.frequency = reduce( settings.channel[chan].square.frequencyInput, 
																	  variableKey, variation );
					// amplitude
					settings.channel[chan].square.amplitude = reduce( settings.channel[chan].square.amplitudeInput, 
																	  variableKey, variation );
					settings.channel[chan].square.offset = reduce( settings.channel[chan].square.offsetInput,
																	  variableKey, variation );
					break;
				case 3:
					// Preloaded Arb Output... no variations possible...
					break;
				case 4:
					// Scripted Arb Output... 
					handleScriptVariation( variableKey, variation, settings.channel[chan].scriptedArb);
					break;
				default:
					thrower( "Unrecognized Agilent Setting: " + str( settings.channel[chan].option ) );
			}
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}


// version without variables
void Agilent::convertInputToFinalSettings()
{
	// iterate between 0 and 1...
	try
	{
		for (auto chan : range( 2 ))
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
					settings.channel[chan].dc.dcLevel = reduce( settings.channel[chan].dc.dcLevelInput);
					break;
				case 1:
					// single frequency output
					// frequency
					settings.channel[chan].sine.frequency = reduce( settings.channel[chan].sine.frequencyInput);
					// amplitude
					settings.channel[chan].sine.amplitude = reduce( settings.channel[chan].sine.amplitudeInput);
					break;
				case 2:
					// Square Output
					// frequency
					settings.channel[chan].square.frequency = reduce( settings.channel[chan].square.frequencyInput );
					// amplitude
					settings.channel[chan].square.amplitude = reduce( settings.channel[chan].square.amplitudeInput );
					settings.channel[chan].square.offset = reduce( settings.channel[chan].square.offsetInput );
					break;
				case 3:
					// Preloaded Arb Output... no variations possible...
					break;
				case 4:
					// Scripted Arb Output... 
					handleNoVariations( settings.channel[chan].scriptedArb );
					break;
				default:
					thrower( "Unrecognized Agilent Setting: " + str( settings.channel[chan].option ) );
			}
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}



/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
void Agilent::handleSavingConfig(std::ofstream& saveFile)
{
	// make sure data is up to date.
	//handleInput();
	// start outputting.
	saveFile << "AGILENT\n";
	saveFile << str(settings.synced) << "\n";
	saveFile << "CHANNEL_1\n";
	saveFile << str(settings.channel[0].option) + "\n";
	saveFile << settings.channel[0].dc.dcLevelInput + "\n";
	saveFile << settings.channel[0].sine.amplitudeInput + "\n";
	saveFile << settings.channel[0].sine.frequencyInput + "\n";
	saveFile << settings.channel[0].square.amplitudeInput + "\n";
	saveFile << settings.channel[0].square.frequencyInput + "\n";
	saveFile << settings.channel[0].square.offsetInput + "\n";
	saveFile << settings.channel[0].preloadedArb.address + "\n";
	saveFile << settings.channel[0].scriptedArb.fileAddress + "\n";
	saveFile << "CHANNEL_2\n";
	saveFile << str( settings.channel[1].option ) + "\n";
	saveFile << settings.channel[1].dc.dcLevelInput + "\n";
	saveFile << settings.channel[1].sine.amplitudeInput + "\n";
	saveFile << settings.channel[1].sine.frequencyInput + "\n";	
	saveFile << settings.channel[1].square.amplitudeInput + "\n";
	saveFile << settings.channel[1].square.frequencyInput + "\n";
	saveFile << settings.channel[1].square.offsetInput + "\n";
	saveFile << settings.channel[1].preloadedArb.address + "\n";
	saveFile << settings.channel[1].scriptedArb.fileAddress + "\n";
	saveFile << "END_AGILENT\n";
}


void Agilent::readConfigurationFile( std::ifstream& file )
{
	ProfileSystem::checkDelimiterLine(file, "AGILENT");
	file >> settings.synced;
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_1");
	// the extra step in all of the following is to remove the , at the end of each input.
	std::string input;
	file >> input;
	try
	{
		settings.channel[0].option = std::stoi( input );
	}
	catch (std::invalid_argument&)
	{
		thrower( "ERROR: Bad channel 1 option!" );
	}
	std::getline( file, settings.channel[0].dc.dcLevelInput);
	std::getline( file, settings.channel[0].sine.amplitudeInput );
	std::getline( file, settings.channel[0].sine.frequencyInput);
	std::getline( file, settings.channel[0].square.amplitudeInput);
	std::getline( file, settings.channel[0].square.frequencyInput);
	std::getline( file, settings.channel[0].square.offsetInput);
	std::getline( file, settings.channel[0].preloadedArb.address);
	std::getline( file, settings.channel[0].scriptedArb.fileAddress );
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_2"); 
	file >> input;
	try
	{
		settings.channel[1].option = std::stoi(input);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Bad channel 1 option!");
	}
	std::getline( file, settings.channel[1].dc.dcLevelInput);
	std::getline( file, settings.channel[1].sine.amplitudeInput);
	std::getline( file, settings.channel[1].sine.frequencyInput);
	std::getline( file, settings.channel[1].square.amplitudeInput);
	std::getline( file, settings.channel[1].square.frequencyInput);
	std::getline( file, settings.channel[1].square.offsetInput);
	std::getline( file, settings.channel[1].preloadedArb.address);
	std::getline( file, settings.channel[1].scriptedArb.fileAddress );
	ProfileSystem::checkDelimiterLine(file, "END_AGILENT");
	// default to first channel.
	
}


void Agilent::outputOff( int channel )
{
	channel++;
	visaFlume.open();
	if (channel == 1)
	{
		visaFlume.write( "OUTPUT1 OFF" );
	}
	else if (channel == 2)
	{
		visaFlume.write( "OUTPUT2 OFF" );
	}
	else
	{
		thrower( "ERROR: Attempted to turn off channel " + str( channel ) + " which does not exist! Use channel 1 or 2." );
	}
	visaFlume.close();
}


bool Agilent::connected()
{
	return isConnected;
}


void Agilent::setDC( int channel, dcInfo info )
{
	channel++;
	visaFlume.open();
	if (channel == 1)
	{
		visaFlume.write( "SOURce1:APPLy:DC DEF, DEF, " + str( info.dcLevel ) + " V" );
		chan1Range.min = chan1Range.max = info.dcLevel;
	}
	else if (channel == 2)
	{
		visaFlume.write( "SOURce2:APPLy:DC DEF, DEF, " + str( info.dcLevel ) + " V" );
		chan2Range.min = chan2Range.max = info.dcLevel;
	}
	else
	{
		thrower( "tried to set DC level for \"channel\" " + str( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	// and leave...
	visaFlume.close();
}


void Agilent::setExistingWaveform( int channel, preloadedArbInfo info )
{
	channel++;
	visaFlume.open();
	if (channel == 1)
	{
		// Load sequence that was previously loaded.
		visaFlume.write( "MMEM:LOAD:DATA \"" + info.address + "\"" );
		// tell it that it's outputting something arbitrary (not sure if necessary)
		visaFlume.write( "SOURCE1:FUNC ARB" );
		// tell it what arb it's outputting.
		visaFlume.write( "SOURCE1:FUNC:ARB \"" + info.address + "\"" );
		// Set output impedance...
		visaFlume.write( str( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
		// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
		visaFlume.write( "SOURCE1:BURST::MODE TRIGGERED" );
		visaFlume.write( "SOURCE1:BURST::NCYCLES 1" );
		visaFlume.write( "SOURCE1:BURST::PHASE 0" );
		// 
		visaFlume.write( "SOURCE1:BURST::STATE ON" );
		visaFlume.write( "OUTPUT1 ON" );
	}
	else if (channel == 2)
	{
		// Load sequence that was previously loaded onto the agilent's non-volitile memory.
		visaFlume.write( "MMEM:LOAD:DATA \"" + info.address + "\"" );
		// tell it that it's outputting something arbitrary (not sure if necessary)
		visaFlume.write( "SOURCE2:FUNC ARB" );
		// tell it what arb it's outputting.
		visaFlume.write( "SOURCE2:FUNC:ARB \"" + info.address + "\"" );
		// not really bursting... but this allows us to reapeat on triggers. Probably another way to do this.
		visaFlume.write( "SOURCE2:BURST::MODE TRIGGERED" );
		visaFlume.write( "SOURCE2:BURST::NCYCLES 1" );
		visaFlume.write( "SOURCE2:BURST::PHASE 0" );
		visaFlume.write( "SOURCE2:BURST::STATE ON" );
		// Set output impedance...
		visaFlume.write( str( "OUTPUT2:LOAD " ) + AGILENT_LOAD );
		visaFlume.write( "OUTPUT2 ON" );

	}
	else
	{
		thrower( "tried to set arbitrary function for \"channel\" " + str( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	visaFlume.close();
}

void Agilent::setSquare( int channel, squareInfo info )
{
	channel++;
	visaFlume.open();
	if (channel == 1)
	{
		visaFlume.write( "SOURCE1:APPLY:SQUARE " + str(info.frequency) + " KHZ, "
				   + str(info.amplitude) + " VPP, " + str(info.offset) + " V" );
	}
	else if (channel == 2)
	{
		visaFlume.write( "SOURCE2:APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
				   + str( info.amplitude ) + " VPP, " + str( info.offset ) + " V" );
	}
	else
	{
		thrower( "tried to set square function for \"channel\" " + str( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	visaFlume.close();
}


void Agilent::setSingleFreq( int channel, sineInfo info )
{
	channel++;
	visaFlume.open();
	if (channel == 1)
	{
		visaFlume.write( "SOURCE1:APPLY:SINUSOID " + str( info.frequency ) + " KHZ, "
						 + str( info.amplitude ) + " VPP");
	}
	else if (channel == 2)
	{
		visaFlume.write( "SOURCE2:APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
						 + str( info.amplitude ) + " VPP" );
	}
	else
	{
		thrower( "tried to set square function for \"channel\" " + str( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	visaFlume.close();
}


/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// pilfered from myAgilent.
/// ......................................

/*
void Agilent::analyzeIntensityScript( ScriptStream& intensityFile, IntensityWaveform* intensityWaveformData,
									  UINT& currentSegmentNumber )
{
	while (!intensityFile.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = intensityWaveformData->readIntoSegment( currentSegmentNumber, intensityFile );
		if (leaveTest < 0)
		{
			// Error
			// should I be throwing here?
			errBox( "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #" 
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
*/

// stuff that only has to be done once.
void Agilent::prepAgilentSettings()
{
	visaFlume.open();
	// timout value?
	visaFlume.setAttribute( VI_ATTR_TMO_VALUE, 40000 );
	// Set sample rate
	visaFlume.write( "SOURCE1:FUNC:ARB:SRATE " + str( AGILENT_SAMPLE_RATE ) );
	// Set filtering state
	visaFlume.write( str( "SOURCE1:FUNC:ARB:FILTER " ) + AGILENT_FILTER_STATE );
	// Set Trigger Parameters
	visaFlume.write( "TRIGGER1:SOURCE EXTERNAL" );
	//
	visaFlume.write( "TRIGGER1:SLOPE POSITIVE" );
	visaFlume.close();
}


void Agilent::handleScriptVariation( key varKey, UINT variation, scriptedArbInfo& scriptInfo )
{
	// Initialize stuff
	prepAgilentSettings();


	// if varied
	if (scriptInfo.wave.isVaried())
	{
		//ScriptedAgilentWaveform scriptWave;
		UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber();
		visaFlume.open();
		// replace variable values where found
		scriptInfo.wave.replaceVarValues( varKey, variation );
		// Loop through all segments
		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData( segNumInc );
			}
			catch (Error& err)
			{
				thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
							+ str( totalSegmentNumber ) + ": " + err.what() );
			}
		}
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages();
		scriptInfo.wave.calcMinMax();
		scriptInfo.wave.minsAndMaxes.resize( variation + 1 );
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt();
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt();
		scriptInfo.wave.normalizeVoltages();
		

		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, variation,
																					totalSegmentNumber ) );
			// Select the segment
			visaFlume.write( "SOURCE1:FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
			// Save the segment
			visaFlume.write( "MMEM:STORE:DATA \"INT:\\seg"
								+ str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
			// increment for the next.
			visaFlume.write( "TRIGGER1:SLOPE POSITIVE" );
		}
		// Now handle seqeunce creation / writing.
		scriptInfo.wave.compileSequenceString( totalSegmentNumber, variation );
		// submit the sequence
		visaFlume.write( scriptInfo.wave.returnSequenceString() );
		// Save the sequence
		visaFlume.write( "SOURCE1:FUNC:ARB seq" + str( variation ) );
		visaFlume.write( "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
		// clear temporary memory.
		visaFlume.write( "SOURCE1:DATA:VOL:CLEAR" );
		// loop through # of variable values
		// replace variable values where found
		scriptInfo.wave.replaceVarValues( varKey, variation );
		// Loop through all segments
		for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData( segNumInc );
			}
			catch (Error& err)
			{
				thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
							+ str( totalSegmentNumber ) + ": " + err.what() );
			}
		}
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages();
		scriptInfo.wave.calcMinMax();
		scriptInfo.wave.minsAndMaxes.resize( variation + 1 );
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt();
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt();
		scriptInfo.wave.normalizeVoltages();

		for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, variation,
																					totalSegmentNumber ) );
			// Select the segment
			visaFlume.write( "SOURCE1:FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
			// Save the segment
			visaFlume.write( "MMEM:STORE:DATA \"INT:\\seg"
								+ str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
			// increment for the next.
			visaFlume.write( "TRIGGER1:SLOPE POSITIVE" );
		}
		// Now handle seqeunce creation / writing.
		scriptInfo.wave.compileSequenceString( totalSegmentNumber, variation );
		// submit the sequence
		visaFlume.write( scriptInfo.wave.returnSequenceString() );
		// Save the sequence
		visaFlume.write( "SOURCE1:FUNC:ARB seq" + str( variation ) );
		visaFlume.write( "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
		// clear temporary memory.
		visaFlume.write( "SOURCE1:DATA:VOL:CLEAR" );
		visaFlume.close();

	}
	else
	{
		handleNoVariations(scriptInfo);
	}
}

void Agilent::handleNoVariations(scriptedArbInfo& scriptInfo)
{
	// Initialize stuff
	prepAgilentSettings();

	//ScriptedAgilentWaveform scriptWave;
	UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber();
	visaFlume.open();
	scriptInfo.wave.replaceVarValues();
	// else not varying. Loop through all segments once.
	for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
	{
		// Use that information to write the data.
		try
		{
			scriptInfo.wave.writeData( segNumInc );
		}
		catch (Error& err)
		{
			thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
					 + str( totalSegmentNumber ) + "." + err.what() );
		}
	}

	// no reassignment nessesary, no variables
	scriptInfo.wave.convertPowersToVoltages();
	scriptInfo.wave.calcMinMax();
	scriptInfo.wave.minsAndMaxes.resize( 1 );
	scriptInfo.wave.minsAndMaxes[0].second = scriptInfo.wave.getMaxVolt();
	scriptInfo.wave.minsAndMaxes[0].first = scriptInfo.wave.getMinVolt();
	scriptInfo.wave.normalizeVoltages();
	visaFlume.write( "SOURCE1:DATA:VOL:CLEAR" );
	for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
	{
		visaFlume.write( str( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
		visaFlume.write( str( "SOURCE1:VOLT:LOW " ) + str( scriptInfo.wave.minsAndMaxes[0].first ) + " V" );
		visaFlume.write( str( "SOURCE1:VOLT:HIGH " ) + str( scriptInfo.wave.minsAndMaxes[0].second ) + " V" );
		visaFlume.write( scriptInfo.wave.compileAndReturnDataSendString( segNumInc, 0, totalSegmentNumber ) );
		// don't think I need this line.
		//visaFlume.write( "SOURCE1:FUNC:ARB seg" + str( segNumInc ) );
		visaFlume.write( "MMEM:STORE:DATA \"INT:\\seg" + str( segNumInc ) + ".arb\"" );
	}

	// Now handle seqeunce creation / writing.
	scriptInfo.wave.compileSequenceString( totalSegmentNumber, 0 );
	// submit the sequence
	visaFlume.write( scriptInfo.wave.returnSequenceString() );
	// Save the sequence
	visaFlume.write( "SOURCE1:FUNC:ARB seq" + str( 0 ) );
	visaFlume.write( "MMEM:STORE:DATA \"INT:\\seq" + str( 0 ) + ".seq\"" );
	// clear temporary memory.
	visaFlume.write( "SOURCE1:DATA:VOL:CLEAR" );
	visaFlume.close();
}

/*
* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
*/
void Agilent::setScriptOutput( UINT varNum, scriptedArbInfo scriptInfo )
{
	if (scriptInfo.wave.isVaried() || varNum == 0)
	{
		visaFlume.open();
		// Load sequence that was previously loaded.

		visaFlume.write( "MMEM:LOAD:DATA \"INT:\\seq" + str( varNum ) + ".seq\"" );
		visaFlume.write( "SOURCE1:FUNC ARB" );
		visaFlume.write( "SOURCE1:FUNC:ARB \"INT:\\seq" + str( varNum ) + ".seq\"" );
		visaFlume.write( str( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
		visaFlume.write( str( "SOURCE1:VOLT:OFFSET " ) + str( (scriptInfo.wave.minsAndMaxes[varNum].first + scriptInfo.wave.minsAndMaxes[varNum].second) / 2 ) + " V" );
		visaFlume.write( str( "SOURCE1:VOLT:LOW " ) + str( scriptInfo.wave.minsAndMaxes[varNum].first ) + " V" );
		visaFlume.write( str( "SOURCE1:VOLT:HIGH " ) + str( scriptInfo.wave.minsAndMaxes[varNum].second ) + " V" );
		visaFlume.write( "OUTPUT1 ON" );
		// and leave...
		visaFlume.close();
	}
}


void Agilent::setAgilent( key varKey, UINT variation )
{
	if (!connected())
	{
		return;
	}
	convertInputToFinalSettings( varKey, variation );
	deviceOutputInfo info = getOutputInfo();
	for (auto chan : range( 2 ))
	{
		switch (info.channel[chan].option)
		{
			case -2:
				// don't do anything.
				break;
			case -1:
				outputOff( chan );
				break;
			case 0:
				setDC( chan, info.channel[chan].dc );
				break;
			case 1:
				setSingleFreq( chan, info.channel[chan].sine );
				break;
			case 2:
				setSquare( chan, info.channel[chan].square );
				break;
			case 3:
				setExistingWaveform( chan, info.channel[chan].preloadedArb );
				break;
			case 4:
				setScriptOutput( variation, info.channel[chan].scriptedArb );
				break;
			default:
				thrower( "ERROR: unrecognized channel 1 setting: " + str( info.channel[chan].option ) );
		}
	}
}


void Agilent::setAgilent()
{
	if (!connected())
	{
		return;
	}
	convertInputToFinalSettings();
	deviceOutputInfo info = getOutputInfo();
	for (auto chan : range( 2 ))
	{
		switch (info.channel[chan].option)
		{
			case -2:
				// don't do anything.
				break;
			case -1:
				outputOff( chan );
				break;
			case 0:
				setDC( chan, info.channel[chan].dc );
				break;
			case 1:
				setSingleFreq( chan, info.channel[chan].sine );
				break;
			case 2:
				setSquare( chan, info.channel[chan].square );
				break;
			case 3:
				setExistingWaveform( chan, info.channel[chan].preloadedArb );
				break;
			case 4:
				setScriptOutput( 0, info.channel[chan].scriptedArb );
				break;
			default:
				thrower( "ERROR: unrecognized channel 1 setting: " + str( info.channel[chan].option ) );
		}
	}
}
