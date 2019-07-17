#include "stdafx.h"
#include "ftd2xx.h"
#include "ddsSystem.h"
#include "AuxiliaryWindow.h"
#include "TextPromptDialog.h"
#include "multiDimensionalKey.h"
#include "afxcmn.h"
#include <boost/lexical_cast.hpp>

DdsSystem::DdsSystem ( bool ftSafemode ) : core( ftSafemode ) { }

void DdsSystem::initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string title )
{
	ddsHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	ddsHeader.fontType = fontTypes::HeadingFont; 
	ddsHeader.Create ( cstr ( title ), NORM_HEADER_OPTIONS, ddsHeader.sPos, parent, id++ );

	programNowButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	programNowButton.fontType = fontTypes::NormalFont;
	programNowButton.Create ( "Program Now", NORM_PUSH_OPTIONS, programNowButton.sPos, parent, IDC_DDS_PROGRAM_NOW );

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
	rampListview.rearrange ( width, height, fonts );
	ddsHeader.rearrange ( width, height, fonts );
	programNowButton.rearrange ( width, height, fonts );
}

void DdsSystem::redrawListview ( )
{
	rampListview.DeleteAllItems ( );
	for ( auto rampInc : range(currentRamps.size()) )
	{
		auto& ramp = currentRamps[ rampInc ];
		rampListview.InsertItem ( str ( ramp.index ), rampInc, 0 );
		rampListview.SetItem ( str ( ramp.channel ), rampInc, 1 );
		rampListview.SetItem ( str ( ramp.freq1.expressionStr ), rampInc, 2 );
		rampListview.SetItem ( str ( ramp.amp1.expressionStr ), rampInc, 3 );
		rampListview.SetItem ( str ( ramp.freq2.expressionStr ), rampInc, 4 );
		rampListview.SetItem ( str ( ramp.amp2.expressionStr ), rampInc, 5 );
		rampListview.SetItem ( str ( ramp.rampTime.expressionStr ), rampInc, 6 );
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
	if ( itemIndicator < 0  )
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
				newIndex = boost::lexical_cast<USHORT>( newIndexStr );
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
					case 2: ramp.freq1 = valStr;		break;
					case 3: ramp.amp1 = valStr;			break;
					case 4: ramp.freq2 = valStr;		break;
					case 5: ramp.amp2 = valStr;			break;
					case 6: ramp.rampTime = valStr;		break;
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

void DdsSystem::programNow ( )
{
	try
	{
		ExpWrap<std::vector<ddsIndvRampListInfo>> simpleExp;
		simpleExp.resizeSeq ( 1 );
		simpleExp.resizeVariations ( 0, 1 );
		simpleExp ( 0, 0 ) = currentRamps;
		core.updateRampLists ( simpleExp );
		core.evaluateDdsInfo ( );
		core.generateFullExpInfo ( );
		core.writeExperiment ( 0, 0 );
	}
	catch ( Error& )
	{
		throwNested ( "Error seen while programming DDS system via Program Now Button." );
	}
}


std::string DdsSystem::getSystemInfo ( )
{
	return core.getSystemInfo();
}
