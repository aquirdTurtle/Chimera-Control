// created by Mark O. Brown

#include "stdafx.h"
#include "TektronicsControl.h"
#include "Thrower.h"
#include "ProfileSystem.h"
#include "range.h"

TektronicsAfgControl::TektronicsAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter ) 
	: visaFlume(safemode, address), configDelim(configurationFileDelimiter)
{
	
}

void TektronicsChannelControl::initialize( POINT loc, CWnd* parent, int& id, std::string channelText, LONG width, 
										   UINT control_id )
{
	auto& cid = control_id;
	channelLabel.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	channelLabel.Create( cstr(channelText), NORM_STATIC_OPTIONS, channelLabel.sPos, parent, id++ );

	controlButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	controlButton.Create( "", NORM_CHECK_OPTIONS, controlButton.sPos, parent, cid++ );

	onOffButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	onOffButton.Create( "", NORM_CHECK_OPTIONS, onOffButton.sPos, parent, cid++ );
	
	fskButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskButton.Create("", NORM_CHECK_OPTIONS, fskButton.sPos, parent, cid++ );
	
	power.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	power.Create(NORM_EDIT_OPTIONS, power.sPos, parent, cid++);
	power.EnableWindow(0);

	mainFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	mainFreq.Create(NORM_EDIT_OPTIONS, mainFreq.sPos, parent, cid++ );
	mainFreq.EnableWindow(0);
	
	fskFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskFreq.Create(NORM_EDIT_OPTIONS, fskFreq.sPos, parent, cid++ );
	fskFreq.EnableWindow(0);
}


void TektronicsAfgControl::interpretKey(std::vector<std::vector<parameterType>>& variables)
{
	UINT variations;
	UINT sequenceNumber;
	if ( variables.size( ) == 0 )
	{
		thrower ( "ERROR: variables empty, no sequence fill!" );
	}
	else if ( variables.front( ).size( ) == 0 )
	{
		variations = 1;
	}
	else
	{
		variations = variables.front().front().keyValues.size();
	}
	sequenceNumber = variables.size( );
	/// imporantly, this sizes the relevant structures.
	currentNums.clear();
	currentNums.resize(variations);
	for ( auto seqInc : range( sequenceNumber ) )
	{
		for ( UINT variationInc : range( variations ) )
		{
			/// deal with first channel.
			if ( currentInfo.channels.first.on )
			{
				tektronicsChannelOutput& channel = currentNums[variationInc].channels.first;
				channel.mainFreqVal = currentInfo.channels.first.mainFreq.evaluate( variables[seqInc], variationInc );
				channel.powerVal = currentInfo.channels.first.power.evaluate( variables[seqInc], variationInc );
				// handle FSK options
				if ( currentInfo.channels.first.fsk )
				{
					channel.fskFreqVal = currentInfo.channels.first.fskFreq.evaluate( variables[seqInc], variationInc );
				}
			}
			// if off don't worry about trying to convert anything, user can not-enter things and it can be fine.
			/// handle second channel.
			if ( currentInfo.channels.second.on )
			{
				tektronicsChannelOutput& channel = currentNums[variationInc].channels.second;
				channel.mainFreqVal = currentInfo.channels.second.mainFreq.evaluate( variables[seqInc], variationInc );
				channel.powerVal = currentInfo.channels.second.power.evaluate( variables[seqInc], variationInc );
				// handle FSK options
				if ( currentInfo.channels.second.fsk )
				{
					channel.fskFreqVal = currentInfo.channels.second.fskFreq.evaluate( variables[seqInc], variationInc );
				}
			}
		}
	}
}


HBRUSH TektronicsAfgControl::handleColorMessage(CWnd* window, CDC* cDC)
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


void TektronicsChannelControl::handleButton ( UINT indicator )
{
	if ( indicator == onOffButton.GetDlgCtrlID ( ) )
	{
		handleOnPress ( );
	}
	else if ( indicator == fskButton.GetDlgCtrlID ( ) )
	{
		handleFskPress ( );
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
tektronicsChannelOutputForm TektronicsChannelControl::getTekChannelSettings()
{
	currentInfo.control = controlButton.GetCheck();
	currentInfo.on = onOffButton.GetCheck();
	currentInfo.fsk = fskButton.GetCheck();

	CString text;
	power.GetWindowTextA(text);
	currentInfo.power = str(text, 13, false, true);

	mainFreq.GetWindowTextA(text);
	currentInfo.mainFreq = str(text, 13, false, true);

	fskFreq.GetWindowTextA(text);
	currentInfo.fskFreq = str(text, 13, false, true);

	return currentInfo;
}


void TektronicsChannelControl::setSettings(tektronicsChannelOutputForm info)
{
	currentInfo = info;
	controlButton.SetCheck ( currentInfo.control );
	onOffButton.SetCheck(currentInfo.on);
	fskButton.SetCheck(currentInfo.fsk);
	power.SetWindowTextA(cstr(currentInfo.power.expressionStr));
	mainFreq.SetWindowTextA(cstr(currentInfo.mainFreq.expressionStr ));
	fskFreq.SetWindowTextA(cstr(currentInfo.fskFreq.expressionStr ));
}


void TektronicsAfgControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << configDelim + "\n";
	newFile << "CHANNEL_1\n";
	newFile << 0 << "\n" << 0 << "\n" << 0 << "\n" << -30 << "\n" << 1 << "\n" << 1 << "\n";
	newFile << "CHANNEL_2\n";
	newFile << 0 << "\n" << 0 << "\n" << 0 << "\n" << -30 << "\n" << 1 << "\n" << 1 << "\n";
	newFile << "END_" + configDelim + "\n";
}


void TektronicsAfgControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << configDelim + "\n";
	saveFile << "CHANNEL_1\n";
	tektronicsInfo tekInfo = getTekSettings();
	saveFile << tekInfo.channels.first.control << "\n" << tekInfo.channels.first.on << "\n" << tekInfo.channels.first.fsk << "\n"
		<< tekInfo.channels.first.power.expressionStr << "\n" << tekInfo.channels.first.mainFreq.expressionStr << "\n"
		<< tekInfo.channels.first.fskFreq.expressionStr << "\n";
	saveFile << "CHANNEL_2\n";
	saveFile << tekInfo.channels.second.control << "\n" << tekInfo.channels.second.on << "\n" << tekInfo.channels.second.fsk << "\n"
		<< tekInfo.channels.second.power.expressionStr << "\n" << tekInfo.channels.second.mainFreq.expressionStr << "\n"
		<< tekInfo.channels.second.fskFreq.expressionStr << "\n";
	saveFile << "END_" + configDelim + "\n";
}


void TektronicsAfgControl::handleOpenConfig(std::ifstream& configFile, Version ver )
{
	std::string tempStr;
	tektronicsInfo tekInfo;
	ProfileSystem::checkDelimiterLine(configFile, "CHANNEL_1");	
	configFile >> tekInfo.channels.first.control;
	configFile >> tekInfo.channels.first.on;
	configFile >> tekInfo.channels.first.fsk;
	configFile.get();
	std::getline(configFile, tekInfo.channels.first.power.expressionStr );
	std::getline(configFile, tekInfo.channels.first.mainFreq.expressionStr );
	std::getline(configFile, tekInfo.channels.first.fskFreq.expressionStr );
	ProfileSystem::checkDelimiterLine(configFile, "CHANNEL_2");
	configFile >> tekInfo.channels.second.control;
	configFile >> tekInfo.channels.second.on;
	configFile >> tekInfo.channels.second.fsk;
	configFile.get();
	std::getline(configFile, tekInfo.channels.second.power.expressionStr );
	std::getline(configFile, tekInfo.channels.second.mainFreq.expressionStr );
	std::getline(configFile, tekInfo.channels.second.fskFreq.expressionStr );
	setSettings(tekInfo);
}



void TektronicsAfgControl::programMachine(UINT variation)
{
	if ( currentInfo.channels.first.control || currentInfo.channels.second.control )
	{
		for ( auto i : range ( 5 ) )
		{
			try
			{
				visaFlume.open ( );
				break;
			}
			catch ( Error& )
			{
				// seems to fail occasionally
			}
		}
		if ( currentInfo.channels.first.control )
		{
			if ( currentInfo.channels.first.on )
			{
				visaFlume.write( "SOURCE1:FREQ " + str( currentNums[variation].channels.first.mainFreqVal ) );
				visaFlume.write( "SOURCE1:VOLT:UNIT DBM" );
				visaFlume.write( "SOURCE1:VOLT " + str( currentNums[variation].channels.first.powerVal ) );
				visaFlume.write( "SOURCE1:VOLT:OFFS 0" );

				if ( currentInfo.channels.first.fsk )
				{
					visaFlume.write( "SOURCE1:FSKey:STATe On" );
					visaFlume.write( "SOURCE1:FSKey:FREQ " + str( currentNums[variation].channels.first.fskFreqVal ) );
					visaFlume.write( "SOURCE1:FSKey:SOURce External" );
				}
				else
				{
					visaFlume.write( "SOURCE1:FSKey:STATe Off" );
				}
				visaFlume.write( "OUTput1:STATe ON" );
			}
			else
			{
				visaFlume.write( "OUTput1:STATe OFF" );
			}
		}
		/// second channel
		if ( currentInfo.channels.second.control )
		{
			if ( currentInfo.channels.second.on )
			{
				visaFlume.write( "SOURCE2:FREQ " + str( currentNums[variation].channels.second.mainFreqVal ) );
				visaFlume.write( "SOURCE2:VOLT:UNIT DBM" );
				visaFlume.write( "SOURCE2:VOLT " + str( currentNums[variation].channels.second.powerVal ) );
				visaFlume.write( "SOURCE2:VOLT:OFFS 0" );
				if ( currentInfo.channels.second.fsk )
				{
					visaFlume.write( "SOURCE2:FSKey:STATe On" );
					visaFlume.write( "SOURCE2:FSKey:FREQ " + str( currentNums[variation].channels.second.fskFreqVal ) );
					visaFlume.write( "SOURCE2:FSKey:SOURce External" );
				}
				else
				{
					visaFlume.write( "SOURCE2:FSKey:STATe Off" );
				}
				visaFlume.write( "OUTput2:STATe ON" );
			}
			else
			{
				visaFlume.write( "OUTput2:STATe OFF" );
			}
		}
		visaFlume.close( );
	}
}

void TektronicsAfgControl::handleProgram()
{
	// this makes sure that what's in the current edits is stored in the currentInfo object.
	getTekSettings();
	// similar to the handling in interpret key except no key or variations, just try to reduce any raw math that the 
	// user enters into the edits.
	currentNums.clear();
	currentNums.resize( 1 );
	/// deal with first channel.
	if (currentInfo.channels.first.on)
	{
		currentNums[0].channels.first.mainFreqVal = currentInfo.channels.first.mainFreq.evaluate();
		currentNums[0].channels.first.powerVal = currentInfo.channels.first.power.evaluate();
		// handle FSK options
		if (currentInfo.channels.first.fsk)
		{
			currentNums[0].channels.first.fskFreqVal = currentInfo.channels.first.fskFreq.evaluate();
		}
	}
	/// handle second channel.
	if (currentInfo.channels.second.on)
	{
		currentNums[0].channels.second.mainFreqVal = currentInfo.channels.second.mainFreq.evaluate(  );
		currentNums[0].channels.second.powerVal = currentInfo.channels.second.power.evaluate();
		// handle FSK options
		if (currentInfo.channels.second.fsk)
		{
			currentNums[0].channels.second.fskFreqVal = currentInfo.channels.second.fskFreq.evaluate();
		}
	}
	// and program just these settings.
	programMachine( 0 );
}


void TektronicsAfgControl::initialize( POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
								    std::string channel2Text, LONG width, UINT control_id )
{

	header.sPos = { loc.x, loc.y, loc.x + width, loc.y += 25 };
	header.Create( cstr("Tektronics " + headerText), NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;
	
	programNow.sPos = { loc.x, loc.y, loc.x + width / 3, loc.y + 20 };
	programNow.Create( "Program Now", NORM_PUSH_OPTIONS, programNow.sPos, parent, control_id );

	channel1.initialize( { loc.x + width / 3, loc.y }, parent, id, channel1Text, width / 3, control_id+1 );
	channel2.initialize( { loc.x + 2 * width / 3, loc.y }, parent, id, channel2Text, width / 3, control_id+50 );

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


void TektronicsChannelControl::rearrange(int width, int height, fontMap fonts)
{
	channelLabel.rearrange(width, height, fonts);
	controlButton.rearrange( width, height, fonts );
	onOffButton.rearrange(width, height, fonts);
	fskButton.rearrange(width, height, fonts);
	mainFreq.rearrange(width, height, fonts);
	power.rearrange(width, height, fonts);
	fskFreq.rearrange(width, height, fonts);
}


std::string TektronicsAfgControl::queryIdentity()
{
	try
	{
		visaFlume.open ( );
		auto res = visaFlume.identityQuery ( );
		visaFlume.close ( );
		return res;
	}
	catch (Error& err)
	{
		return err.trace();
	}
}


void TektronicsAfgControl::rearrange(int width, int height, fontMap fonts)
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


tektronicsInfo TektronicsAfgControl::getTekSettings()
{
	currentInfo.channels.first = channel1.getTekChannelSettings();
	currentInfo.channels.second = channel2.getTekChannelSettings();
	return currentInfo;
}

// does not set the address, that's permanent.
void TektronicsAfgControl::setSettings(tektronicsInfo info)
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


void TektronicsAfgControl::handleButtons(UINT indicator)
{
	channel1.handleButton ( indicator );
	channel2.handleButton ( indicator );
}
