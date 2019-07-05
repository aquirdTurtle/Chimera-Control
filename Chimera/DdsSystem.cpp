#include "stdafx.h"
#include "ftd2xx.h"
#include "ddsSystem.h"
#include "AuxiliaryWindow.h"
#include "TextPromptDialog.h"
#include "multiDimensionalKey.h"
#include "afxcmn.h"
#include <boost/lexical_cast.hpp>

DdsSystem::DdsSystem ( bool ftSafemode ) : ftFlume ( ftSafemode ) { }

void DdsSystem::initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string title )
{
	ddsHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	ddsHeader.fontType = fontTypes::HeadingFont; 
	ddsHeader.Create ( cstr ( title ), NORM_HEADER_OPTIONS, ddsHeader.sPos, parent, id++ );

	RECT r1;
	parent->GetClientRect ( &r1 );
	rampListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 160 };
	rampListview.fontType = fontTypes::SmallFont; 
	rampListview.Create ( NORM_LISTVIEW_OPTIONS, rampListview.sPos, parent, IDC_DDS_LISTVIEW );
	rampListview.SetTextBkColor ( _myRGBs[ "Interactable-Bkgd" ] );
	rampListview.SetTextColor ( _myRGBs[ "AuxWin-Text" ] );
	rampListview.SetBkColor ( _myRGBs[ "Interactable-Bkgd" ] );

	rampListview.InsertColumn ( 0, "Index", r1.right / 12 );
	rampListview.InsertColumn ( 1, "Channel", r1.right / 12 );
	rampListview.InsertColumn ( 2, "Freq 1", r1.right / 8 );
	rampListview.InsertColumn ( 3, "Amp 1" );
	rampListview.InsertColumn ( 4, "Freq 2" );
	rampListview.InsertColumn ( 5, "Amp 2" );
	rampListview.InsertColumn ( 6, "Time" );

	rampListview.insertBlankRow ( );
}

void DdsSystem::rearrange ( UINT width, UINT height, fontMap fonts )
{
	ddsHeader.rearrange ( width, height, fonts );
	rampListview.rearrange ( width, height, fonts );
}

void DdsSystem::writeExperiment ( std::vector<ddsBox<ddsRampInfo>> rampInfo )
{
	if ( rampInfo.size ( ) == 0 )
	{
		return;
	}
	// why do I reconnect here?
	connectasync ( );
	lockPLLs ( );
	for (auto boardNum : range( rampInfo.front().numBoards() ) )
	{
		for ( auto channelNum : range ( rampInfo.front ( ).numChannels ( ) ) )
		{
			writeArrResetFreq ( boardNum, channelNum, rampInfo.front ( ) ( boardNum, channelNum ).freq1 );
			writeArrResetAmp ( boardNum, channelNum, rampInfo.front ( ) ( boardNum, channelNum ).amp1 );
		}
	}
	for (auto rampIndex : range(rampInfo.size()) ) 
	{
		if ( rampIndex > 0 )
		{
			for ( auto boardNum : range ( rampInfo[ rampIndex ].numBoards ( ) ) )
			{
				for (auto channelNum : range ( rampInfo[ rampIndex ].numChannels ( ) ) )
				{
					auto& thisRamp = rampInfo[ rampIndex ] ( boardNum, channelNum );
					auto& prevRamp = rampInfo[ rampIndex-1 ] ( boardNum, channelNum );
					if ( thisRamp.freq1 != prevRamp.freq2 || thisRamp.amp1 != prevRamp.amp2 )
					{
						thrower ( "Initial ramp conditions should match the final ramp conditions of the previous ramp."
									"This wasn't the case for ramp Index " + str ( rampIndex ) + " for Board number "
									+ str ( boardNum ) + ", Channel number " + str ( channelNum ) );
					}
				}
			}
		}
		writeOneRamp ( rampInfo[ rampIndex ], rampIndex );
	}
	longUpdate ( );
}

void DdsSystem::redrawListview ( )
{
	rampListview.DeleteAllItems ( );
	for ( auto rampInc : range(currentRamps.size()) )
	{
		auto& ramp = currentRamps[ rampInc ];
		rampListview.InsertItem ( str ( ramp.index ), rampInc, 0 );
		rampListview.SetItem ( str ( ramp.channel ), rampInc, 1 );
		rampListview.SetItem ( str ( ramp.freq1Form.expressionStr ), rampInc, 2 );
		rampListview.SetItem ( str ( ramp.amp1Form.expressionStr ), rampInc, 3 );
		rampListview.SetItem ( str ( ramp.freq2Form.expressionStr ), rampInc, 4 );
		rampListview.SetItem ( str ( ramp.amp2Form.expressionStr ), rampInc, 5 );
		rampListview.SetItem ( str ( ramp.rampTimeForm.expressionStr ), rampInc, 6 );
	}
	rampListview.insertBlankRow ( );
}

void DdsSystem::handleRampClick (  )
{
	if ( !controlActive )
	{
		return;
	}
	/// get the item and subitem
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos ( &myItemInfo.pt );
	rampListview.ScreenToClient ( &myItemInfo.pt );
	rampListview.SubItemHitTest ( &myItemInfo );
	int subitem, itemIndicator;
	itemIndicator = myItemInfo.iItem;
	if ( itemIndicator == -1 )
	{
		return;
	}
	subitem = myItemInfo.iSubItem;
	/// check if adding new variable
	CString text = rampListview.GetItemText ( itemIndicator, 0 );
	if ( text == "___" )
	{
		currentRamps.resize ( currentRamps.size ( ) + 1 );
		redrawListview ( );
	}
	auto& ramp = currentRamps[ itemIndicator ];
	/// Handle different subitem clicks
	switch ( subitem )
	{
		case 0:
		{
			std::string newIndexStr;
			TextPromptDialog dialog ( &newIndexStr, "Please enter a ramp index (0-255):" );
			dialog.DoModal ( );
			if ( newIndexStr == "" )
			{
				// probably canceled.
				break;
			}
			USHORT newIndex;
			try
			{
				newIndex = boost::lexical_cast<USHORT>( newIndex );
			}
			catch ( boost::bad_lexical_cast& )
			{
				thrower ( "Failed to Convert input to unsigned integer!" );
			}
			if ( newIndex > 255 || newIndex < 0 )
			{
				thrower ( "Index" + str(newIndex) + " needs to be between 0 and 255" );
			}
			ramp.index = newIndex;
			rampListview.SetItem ( str( ramp.index ), itemIndicator, subitem );
			break;
		}
		case 1:
		{
			std::string newChannelStr;
			TextPromptDialog dialog ( &newChannelStr, "Please enter a channel number (0-7):" );
			dialog.DoModal ( );
			if ( newChannelStr == "" )
			{
				// probably canceled.
				break;
			}
			USHORT newChannel;
			try
			{
				newChannel = boost::lexical_cast<USHORT>( newChannelStr );
			}
			catch ( boost::bad_lexical_cast& )
			{
				thrower ( "Failed to Convert input to unsigned integer!" );
			}
			if ( newChannel > 255 || newChannel < 0 )
			{
				thrower ( "Index" + str ( newChannel ) + " needs to be between 0 and 7 (inclusive)" );
			}
			ramp.channel = newChannel;
			break;
		}
		default:
		{
			if ( subitem < 7 )
			{
				std::string valStr;
				TextPromptDialog dialog ( &valStr, "Please enter an Expression for this value:" );
				dialog.DoModal ( );
				if ( valStr == "" )
				{
					// probably canceled.
					break;
				}
				switch ( subitem )
				{
					case 2: ramp.freq1Form = valStr;		break;
					case 3: ramp.amp1Form = valStr;			break;
					case 4: ramp.freq2Form = valStr;		break;
					case 5: ramp.amp2Form = valStr;			break;
					case 6: ramp.rampTimeForm = valStr;		break;
				}
			}
		}
	}
	redrawListview ( );
}


void DdsSystem::deleteRampVariable ( )
{
	if ( !controlActive )
	{
		return;
	}
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos ( &cursorPos );
	rampListview.ScreenToClient ( &cursorPos );
	int subitemIndicator = rampListview.HitTest ( cursorPos );
	LVHITTESTINFO myItemInfo = { 0 };
	myItemInfo.pt = cursorPos;
	int itemIndicator = rampListview.SubItemHitTest ( &myItemInfo );
	if ( itemIndicator == -1 || itemIndicator == currentRamps.size ( ) )
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if ( UINT ( itemIndicator ) < currentRamps.size ( ) )
	{
		answer = promptBox ( "Delete Ramp # " + str(itemIndicator+1) + "?", MB_YESNO );
		if ( answer == IDYES )
		{
			currentRamps.erase ( currentRamps.begin ( ) + itemIndicator );
		}
	}
	redrawListview ( );
}


// this probably needs an overload with a default value for the empty parameters case...
std::vector<ddsBox<ddsRampInfo>> DdsSystem::evaluateDdsInfoForm ( std::vector<ddsBox<ddsRampInfoForm>> rampInfoForm )
{
	std::vector<ddsBox<ddsRampInfo>> rampInfo( rampInfoForm.size() );
	std::vector<parameterType> emptyParamVec;
	for ( auto seqInc : range( rampInfoForm.size() ) )
	{
		for ( auto whichBoard : range(2) )
		{
			for (auto whichChannel : range(4) )
			{
				auto& channel = rampInfo[seqInc](whichBoard , whichChannel );
				auto& channelForm = rampInfoForm[ seqInc ] ( whichBoard, whichChannel );
				channel.freq1 = channelForm.freq1Form.evaluate ( emptyParamVec, -1 );
				channel.freq2 = channelForm.freq2Form.evaluate ( emptyParamVec, -1 );
				channel.amp1 = channelForm.amp1Form.evaluate ( emptyParamVec, -1 );
				channel.amp2 = channelForm.amp2Form.evaluate ( emptyParamVec, -1 );
				channel.rampTime = channelForm.rampTimeForm.evaluate ( emptyParamVec, -1 );
			}
		}
	}
	return rampInfo;
}

// a low level write wrapper around the ftFlume write
void DdsSystem::writeDDS ( UINT8 DEVICE, UINT16 ADDRESS, UINT8 dat1, UINT8 dat2, UINT8 dat3, UINT8 dat4 )
{
	if ( this->connType == ddsConnectionType::type::Async )
	{
		// None of these should be possible based on the types of these args. 
		if ( DEVICE > 255 || ADDRESS > 65535 || dat1 > 255 || dat2 > 255 || dat3 > 255 || dat4 > 255 )
		{
			thrower ( "Error: DDS write out of range." );
		}
		UINT8 ADDRESS_LO = ADDRESS & 0x00ffUL;
		UINT8 ADDRESS_HI = ( ADDRESS & 0xff00UL ) >> 8;
		std::vector<unsigned char> input = { unsigned char ( WBWRITE + DEVICE ), ADDRESS_HI, ADDRESS_LO, dat1, dat2, dat3, dat4 };
		ftFlume.write ( input, MSGLENGTH );
	}
	else
	{
		thrower ( "Incorrect connection type, should be ASYNC" );
	}

}

// a wrapper around the ftFlume open
void DdsSystem::connectasync()
{
	DWORD numDevs = ftFlume.getNumDevices();
	if (numDevs > 0)
	{
		ftFlume.open("FT1FJ8PEB");
		ftFlume.setUsbParams();
		connType = ddsConnectionType::type::Async;
	}
	else
	{
		thrower("No devices found.");
	}
}

void DdsSystem::disconnect ( )
{
	ftFlume.close ( );
}

INT DdsSystem::getFTW(double freq) 
{ 
	// units expected???

	// Negative ints, Nyquist resetFreq, works out.
	if (freq > INTERNAL_CLOCK / 2) 
	{
		thrower("DDS frequency out of range. Must be < 250MHz.");
		return 0;
	}
	return (INT) round ( ( freq * pow ( 2, 32 ) ) / ( INTERNAL_CLOCK ) );;
}

UINT DdsSystem::getATW(double amp) 
{
	// input is a percentage (/100) of the maximum amplitude
	if (amp > 100) 
	{
		thrower("DDS amplitude out of range, should be < 100 %");
	}
	return (UINT) round ( amp * ( pow ( 2, 10 ) - 1 ) / 100.0 );
}

INT DdsSystem::get32bitATW(double amp) 
{
	// why do we need this and the getATW function?
	//SIGNED
	if (abs(amp) > 100) 
	{
		thrower("ERROR: DDS amplitude out of range, should be < 100%.");
	}
	return (INT) round ( amp * ( pow ( 2, 32 ) - pow ( 2, 22 ) ) / 100.0 );
}

void DdsSystem::longUpdate() 
{
	// what's this??? 
	writeDDS(0, 0x1d, 0, 0, 0, 1);
	writeDDS(1, 0x1d, 0, 0, 0, 1);
}

void DdsSystem::lockPLLs() 
{
	writeDDS(0, 1, 0, 0b10101000, 0, 0);
	writeDDS(1, 1, 0, 0b10101000, 0, 0);
	longUpdate();
	Sleep(100); //This delay is critical, need to give the PLL time to lock.
}

void DdsSystem::channelSelect(UINT8 device, UINT8 channel) 
{
	// ??? this is hard-coded...
	UINT8 CW = 0b11100000;
	//CW |= 1 << (channel + 4);
	writeDDS(device, 0, 0, 0, 0, CW);
}

void DdsSystem::writeFreq(UINT8 device, UINT8 channel, double freq) 
{
	UINT FTW = getFTW(freq);
	UINT8 byte4 = FTW & 0x000000ffUL;
	UINT8 byte3 = (FTW & 0x0000ff00UL) >> 8;
	UINT8 byte2 = (FTW & 0x00ff0000UL) >> 16;
	UINT8 byte1 = (FTW & 0xff000000UL) >> 24;
	//TODO: Fix this. Currently sets FTW to zero and then back (to what?) to force a rewrite, 
	// solving issue with inactive channels. (??? looks like chris might have already fixed this?)
	//write(device, 4, 0, 0, 0, 0);
	writeDDS(device, 4, byte1, byte2, byte3, byte4);
	//longupdate();
}

void DdsSystem::writeAmp(UINT8 device, UINT8 channel, double amp) 
{
	UINT ATW = getATW(amp);
	UINT8 byte2 = ATW & 0x000000ffUL;
	UINT8 byte1 = ATW >> 8;

	//Necessary to turn on amplitude multiplier.
	byte1 |= 1 << 4; 
	// only needs two bytes?
	writeDDS(device, 6, 0, 0, byte1, byte2);
}

void DdsSystem::writeArrResetFreq(UINT8 device, UINT8 channel, double freq) 
{
	UINT16 address = 4 * device + 3 - channel;
	// This looks strange. FTW is an insigned int but it's being bitwise compared to an unsigned long. (the UL suffix)
	UINT FTW = getFTW(freq);
	UINT8 byte4 = FTW & 0x000000ffUL;
	UINT8 byte3 = (FTW & 0x0000ff00UL) >> 8;
	UINT8 byte2 = (FTW & 0x00ff0000UL) >> 16;
	UINT8 byte1 = (FTW & 0xff000000UL) >> 24;
	writeDDS(WBWRITE_ARRAY, address, byte1, byte2, byte3, byte4);
}

void DdsSystem::writeArrResetAmp(UINT8 device, UINT8 channel, double amp) 
{
	UINT16 address = 0x100 + 4 * device + 3 - channel;
	UINT32 ATW = getATW(amp);
	ATW = ATW << 22;
	UINT8 byte4 = ATW & 0x000000ffUL;
	UINT8 byte3 = (ATW & 0x0000ff00UL) >> 8;
	UINT8 byte2 = (ATW & 0x00ff0000UL) >> 16;
	UINT8 byte1 = (ATW & 0xff000000UL) >> 24;

	writeDDS(WBWRITE_ARRAY, address, byte1, byte2, byte3, byte4);
}

void DdsSystem::writeArrReps(UINT8 index, UINT16 reps) 
{

	UINT16 address = 0x200 + index;

	UINT8 byte4 = reps & 0x000000ffUL;
	UINT8 byte3 = (reps & 0x0000ff00UL) >> 8;
	UINT8 byte2 = 0;
	UINT8 byte1 = 0;

	writeDDS(WBWRITE_ARRAY, address, byte1, byte2, byte3, byte4);
}

double DdsSystem::calcDeltaFreq(double freq1, double freq2, int reps) 
{
	double deltaFreq = (freq2 - freq1) / (reps);
	return(deltaFreq);	
}

double DdsSystem::calcDeltaAmp(double amp1, double amp2, int reps) 
{
	double deltaAmp = (amp2 - amp1) / (reps);
	return(deltaAmp);
}

INT DdsSystem::getRepsFromTime(double time) 
{
	// units of time is in milliseconds
	double deltaTime = 8e-3; //8 usec
	INT repNum = (int)((time / deltaTime) + 0.5);
	return(repNum);
}

void DdsSystem::writeArrDeltaFreq(UINT8 device, UINT8 channel, UINT8 index, double deltafreq) 
{
	UINT16 address = 0x400 + 0x200 * (4 * device + 3 - channel) + index;
	UINT FTW = getFTW(deltafreq);
	UINT8 byte4 = FTW & 0x000000ffUL;
	UINT8 byte3 = (FTW & 0x0000ff00UL) >> 8;
	UINT8 byte2 = (FTW & 0x00ff0000UL) >> 16;
	UINT8 byte1 = (FTW & 0xff000000UL) >> 24;
	writeDDS(WBWRITE_ARRAY, address, byte1, byte2, byte3, byte4);
}

void DdsSystem::writeArrDeltaAmp(UINT8 device, UINT8 channel, UINT8 index, double deltaamp) 
{
	UINT16 address = 0x1400 + 0x200 * (4 * device + 3 - channel) + index;
	INT ATW = get32bitATW(deltaamp);
	UINT8 byte4 = ATW & 0x000000ffUL;
	UINT8 byte3 = (ATW & 0x0000ff00UL) >> 8;
	UINT8 byte2 = (ATW & 0x00ff0000UL) >> 16;
	UINT8 byte1 = (ATW & 0xff000000UL) >> 24;
	writeDDS(WBWRITE_ARRAY, address, byte1, byte2, byte3, byte4);
}

void DdsSystem::writeOneRamp(ddsBox<ddsRampInfo> boxRamp, UINT8 rampIndex) 
{
	for (auto boardNum : range(boxRamp.numBoards() ) ) 
	{
		for (auto channelNum : range(boxRamp.numChannels()) )
		{
			auto& channel = boxRamp(boardNum, channelNum);
			auto reps = getRepsFromTime( channel.rampTime );
			writeArrReps( rampIndex,reps);
			writeArrDeltaFreq( boardNum, channelNum, rampIndex, calcDeltaFreq ( channel.freq1, channel.freq2, reps ) );
			writeArrDeltaAmp( boardNum, channelNum, rampIndex, calcDeltaAmp ( channel.amp1, channel.amp2, reps ) );
		}
	}
}

