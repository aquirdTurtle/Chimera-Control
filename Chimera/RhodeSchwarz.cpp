#include "stdafx.h"
#include "RhodeSchwarz.h"
#include "GpibFlume.h"
#include "constants.h"
#include "AuxiliaryWindow.h"


RhodeSchwarz::RhodeSchwarz() : gpibFlume(RSG_ADDRESS, RSG_SAFEMODE){}


std::string RhodeSchwarz::getIdentity()
{
	return gpibFlume.queryIdentity();
}


/*
 * The controls in this class only display information about what get's programmed to the RSG. They do not
 * (by design) provide an interface for which the user to change the programming of the RSG directly. The
 * user is to do this by using the "rsg:" command in a script.
 */
void RhodeSchwarz::initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id )
{
	// These are currently just hard-coded.
	triggerTime = 0.01;
	rsgTtl = "a15"; //c3

	// controls
	header.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	header.Create( "RHODE-SHWARTZ GENERATOR INFO (READ-ONLY)", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER 
				  | ES_READONLY, header.sPos, master, id++ );
	header.fontType = HeadingFont;
	pos.y += 25;

	infoControl.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	infoControl.Create( WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER, infoControl.sPos, master, 
					   id++ );
	infoControl.fontType = SmallFont;
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
	listViewDefaultCollumn.cx = 200;
	infoControl.InsertColumn( 2, &listViewDefaultCollumn );
	pos.y += 100;
}

void RhodeSchwarz::rearrange(UINT width, UINT height, fontMap fonts)
{
	header.rearrange( width, height, fonts);
	infoControl.rearrange( width, height, fonts);
}

/*
 * The following function takes the existing list of events (already evaluated for a particular variation) and
 * orders them in time. 
 */
void RhodeSchwarz::orderEvents(UINT variation)
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


void RhodeSchwarz::setInfoDisp(UINT variation)
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

void RhodeSchwarz::interpretKey(std::vector<variableType>& variables)
{
	UINT variations;
	if ( variables.size() == 0)
	{
		variations = 1;
	}
	else
	{
		variations = variables.front().keyValues.size();
	}
	/// imporantly, this sizes the relevant structures.
	events.clear();
	events.resize(variations);
	for (UINT variationNumber = 0; variationNumber < variations; variationNumber++)
	{
		for (UINT freqInc = 0; freqInc < eventStructures.size(); freqInc++)
		{
			rsgEvent event;
			// convert freq
			event.frequency = eventStructures[freqInc].frequency.evaluate( variables, variationNumber);
			// convert power
			event.power = eventStructures[freqInc].power.evaluate( variables, variationNumber );
			/// deal with time!
			if (eventStructures[freqInc].time.first.size() == 0)
			{
				event.time = eventStructures[freqInc].time.second;
			}
			else
			{
				event.time = 0;
				for (auto timeStr : eventStructures[freqInc].time.first)
				{
					event.time += timeStr.evaluate( variables, variationNumber );
				}
				event.time += eventStructures[freqInc].time.second;
			}
			events[variationNumber].push_back(event);
		}	
	}
}


// Essentially gets called by a script command.
void RhodeSchwarz::addFrequency(rsgEventForm info)
{
	eventStructures.push_back( info );
}


void RhodeSchwarz::programRsg( UINT variationNumber )
{
	if (events[variationNumber].size() == 0)
	{
		// nothing to do.
		return;
	}
	else if (events[variationNumber].size() == 1)
	{
		gpibFlume.send( "OUTPUT ON" );
		gpibFlume.send( "SOURce:FREQuency:MODE CW" );
		gpibFlume.send( "FREQ " + str( events[variationNumber][0].frequency, 13 ) + " GHz" );
		gpibFlume.send( "POW " + str( events[variationNumber][0].power, 13 ) + " dBm" );
		gpibFlume.send( "OUTP ON" );
	}
	else
	{
		gpibFlume.send( "OUTP ON" );
		gpibFlume.send( "SOURce:LIST:SEL 'freqList" + str( events.size() ) + "'" );
		std::string frequencyList = "SOURce:LIST:FREQ " + str( events[variationNumber][0].frequency, 13 ) + " GHz";
		std::string powerList = "SOURce:LIST:POW " + str( events[variationNumber][0].power, 13 ) + "dBm";
		for (UINT eventInc = 1; eventInc < events[variationNumber].size(); eventInc++)
		{
			frequencyList += ", ";
			frequencyList += str( events[variationNumber][eventInc].frequency, 13 ) + " GHz";
			powerList += ", ";
			powerList += str( events[variationNumber][eventInc].power, 13 ) + "dBm";
		}
		gpibFlume.send( cstr(frequencyList) );
		gpibFlume.send( cstr(powerList));
		gpibFlume.send( "SOURce:LIST:MODE STEP" );
		gpibFlume.send( "SOURce:LIST:TRIG:SOURce EXT" );
		gpibFlume.send( "SOURce:FREQ:MODE LIST" );
	}
}


void RhodeSchwarz::clearFrequencies()
{
	eventStructures.clear();
	events.clear();
}


std::vector<rsgEventForm> RhodeSchwarz::getFrequencyForms()
{
	return eventStructures;
}


std::string RhodeSchwarz::getRsgTtl()
{
	return rsgTtl;
}

double RhodeSchwarz::getTriggerLength()
{
	return triggerTime;
}

