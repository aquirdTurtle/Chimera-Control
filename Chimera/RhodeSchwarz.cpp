// created by Mark O. Brown
#include "stdafx.h"
#include "RhodeSchwarz.h"
#include "GpibFlume.h"
#include "constants.h"
#include "AuxiliaryWindow.h"

RohdeSchwarz::RohdeSchwarz() : gpibFlume(RSG_ADDRESS, RSG_SAFEMODE){}


std::string RohdeSchwarz::getIdentity()
{
	return gpibFlume.queryIdentity();
}

/*
 * The controls in this class only display information about what get's programmed to the RSG. They do not
 * (by design) provide an interface for which the user to change the programming of the RSG directly. The
 * user is to do this by using the "rsg:" command in a script.
 */
void RohdeSchwarz::initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id )
{
	// controls
	header.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	header.Create( "RHODE-SHWARTZ GENERATOR INFO (READ-ONLY)", NORM_HEADER_OPTIONS, header.sPos, master, id++ );
	header.fontType = fontTypes::HeadingFont;

	infoControl.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	infoControl.Create( NORM_LISTVIEW_OPTIONS, infoControl.sPos, master, id++ );
	infoControl.fontType = fontTypes::SmallFont;
	infoControl.SetBkColor( RGB( 15, 15, 15 ) );
	infoControl.SetTextBkColor( RGB( 15, 15, 15 ) );
	infoControl.SetTextColor( RGB( 255, 255, 255 ) );
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset( &listViewDefaultCollumn, 0, sizeof( listViewDefaultCollumn ) );
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "#";
	listViewDefaultCollumn.cx = 80;
	infoControl.InsertColumn( 0, &listViewDefaultCollumn );
	listViewDefaultCollumn.pszText = "Frequency (GHz)";
	listViewDefaultCollumn.cx = 200;
	infoControl.InsertColumn( 1, &listViewDefaultCollumn );
	listViewDefaultCollumn.pszText = "Power (dBm)";
	listViewDefaultCollumn.cx = 180;
	infoControl.InsertColumn( 2, &listViewDefaultCollumn );
	pos.y += 100;
}


void RohdeSchwarz::rearrange(UINT width, UINT height, fontMap fonts)
{
	header.rearrange( width, height, fonts);
	infoControl.rearrange( width, height, fonts);
}

/*
 * The following function takes the existing list of events (already evaluated for a particular variation) and
 * orders them in time. 
 */
void RohdeSchwarz::orderEvents(UINT variation)
{
	std::vector<rsgEvent> newOrder;
	for (auto event : events[variation])
	{
		bool set = false;
		int count = 0;
		// deal with the first case.
		if (newOrder.size() == 0)
		{
			newOrder.push_back( event );
			continue;
		}

		for (UINT eventInc = 0; eventInc < newOrder.size(); eventInc++)
		{
			if (newOrder[eventInc].time > event.time)
			{
				newOrder.insert( newOrder.begin() + count, event );
				set = true;
				break;
			}
		}

		if (!set)
		{
			newOrder.push_back( event );
		}
	}
	events[variation] = newOrder;
}


void RohdeSchwarz::setInfoDisp(UINT variation)
{
	infoControl.DeleteAllItems();
	int count = 0;
	for (auto event : events[variation])
	{
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		std::string text = str( count + 1 );
		listViewDefaultItem.pszText = (LPSTR)text.c_str();
		listViewDefaultItem.iItem = count;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		infoControl.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		text = str( event.frequency );
		listViewDefaultItem.pszText = (LPSTR)text.c_str();
		infoControl.SetItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 2;
		text = str( event.power );
		listViewDefaultItem.pszText = (LPSTR)text.c_str();
		infoControl.SetItem( &listViewDefaultItem );
		count++;
	}
}

void RohdeSchwarz::interpretKey( std::vector<std::vector<parameterType>>& variables)
{
	UINT variations;
	UINT sequencNumber;
	if ( variables.size() == 0)
	{
		thrower ( "ERROR: empty variables! no sequence size!" );
	}
	else if ( variables[0].size( ) == 0 )
	{
		variations = 1;
	}
	else
	{
		variations = variables.front().front().keyValues.size();
	}
	sequencNumber = variables.size();
	/// imporantly, this sizes the relevant structures.
	events.clear();
	events.resize(variations);
	for ( auto seqNum : range( sequencNumber ) )
	{
		for ( UINT variationNumber = 0; variationNumber < variations; variationNumber++ )
		{
			for ( UINT freqInc = 0; freqInc < eventForms.size( ); freqInc++ )
			{
				rsgEvent event;
				event.frequency = eventForms[freqInc].frequency.evaluate( variables[seqNum], variationNumber );
				event.power = eventForms[freqInc].power.evaluate( variables[seqNum], variationNumber );
				/// deal with time!
				if ( eventForms[freqInc].time.first.size( ) == 0 )
				{
					event.time = eventForms[freqInc].time.second;
				}
				else
				{
					event.time = 0;
					for ( auto timeStr : eventForms[freqInc].time.first )
					{
						event.time += timeStr.evaluate( variables[seqNum], variationNumber );
					}
					event.time += eventForms[freqInc].time.second;
				}
				events[variationNumber].push_back( event );
			}
		}
	}
}


// Essentially gets called by a script command.
void RohdeSchwarz::addFrequency(rsgEventForm info)
{
	eventForms.push_back( info );
}

void RohdeSchwarz::setFmSettings ( )
{
	gpibFlume.send ( "SOURCE:PM1:STATe OFF" );
	gpibFlume.send ( "SOURCE:FM:MODE NORMal" );
	gpibFlume.send ( "SOURCE:FM:RATio 100PCT" );
	gpibFlume.send ( "SOURCE:FM1:SOURce EXT1" );
	gpibFlume.send ( "SOURCE:FM1:DEViation 20kHz" ); 
	gpibFlume.send ( "SOURCE:FM1:STATe ON" );
}


void RohdeSchwarz::setPmSettings ( )
{
	gpibFlume.send ( "SOURCE:FM1:STATe OFF" );
	gpibFlume.send ( "SOURCE:PM:MODE HDEViation" );
	gpibFlume.send ( "SOURCE:PM:RATio 100PCT" );
	gpibFlume.send ( "SOURCE:PM1:SOURce EXT1" );
	gpibFlume.send ( "SOURCE:PM1:DEViation 6.28RAD" );
	gpibFlume.send ( "SOURCE:PM1:STATe ON" );
}


void RohdeSchwarz::programRsg( UINT variationNumber )
{
	if (events[variationNumber].size() == 0)
	{
		// nothing to do.
		return;
	}
	else
	{
		//setPmSettings ( );
		if ( events[ variationNumber ].size ( ) == 1 )
		{
			gpibFlume.send ( "OUTPUT ON" );
			gpibFlume.send ( "SOURce:FREQuency:MODE CW" );
			gpibFlume.send ( "FREQ " + str ( events[ variationNumber ][ 0 ].frequency, 13 ) + " GHz" );
			gpibFlume.send ( "POW " + str ( events[ variationNumber ][ 0 ].power, 13 ) + " dBm" );
			gpibFlume.send ( "OUTP ON" );
		}
		else
		{
			gpibFlume.send ( "OUTP ON" );
			gpibFlume.send ( "SOURce:LIST:SEL 'freqList" + str ( events.size ( ) ) + "'" );
			std::string freqList = "SOURce:LIST:FREQ " + str ( events[ variationNumber ][ 0 ].frequency, 13 ) + " GHz";
			std::string powerList = "SOURce:LIST:POW " + str ( events[ variationNumber ][ 0 ].power, 13 ) + "dBm";
			for ( UINT eventInc = 1; eventInc < events[ variationNumber ].size ( ); eventInc++ )
			{
				freqList += ", ";
				freqList += str ( events[ variationNumber ][ eventInc ].frequency, 13 ) + " GHz";
				powerList += ", ";
				powerList += str ( events[ variationNumber ][ eventInc ].power, 13 ) + "dBm";
			}
			gpibFlume.send ( cstr ( freqList ) );
			gpibFlume.send ( cstr ( powerList ) );
			gpibFlume.send ( "SOURce:LIST:MODE STEP" );
			gpibFlume.send ( "SOURce:LIST:TRIG:SOURce EXT" );
			gpibFlume.send ( "SOURce:FREQ:MODE LIST" );
		}
	}
}


void RohdeSchwarz::clearFrequencies()
{
	eventForms.clear();
	events.clear();
}


std::vector<rsgEventForm> RohdeSchwarz::getFrequencyForms()
{
	return eventForms;
}


std::pair<DioRows::which, UINT> RohdeSchwarz::getRsgTriggerLine ()
{
	return rsgTriggerLine;
}


UINT RohdeSchwarz::getNumTriggers (UINT variationNumber )
{
	return events[ variationNumber ].size ( ) == 1 ? 0 : events[ variationNumber ].size ( );
}
