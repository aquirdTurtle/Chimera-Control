// created by Mark O. Brown
#include "stdafx.h"
#include "MicrowaveSystem.h"
#include "GpibFlume.h"
#include "constants.h"
#include "AuxiliaryWindow.h"

MicrowaveSystem::MicrowaveSystem() {}

std::string MicrowaveSystem::getIdentity()
{
	return core.queryIdentity();
}

/*
 * The controls in this class only display information about what get's programmed to the RSG. They do not
 * (by design) provide an interface for which the user to change the programming of the RSG directly. The
 * user is to do this by using the "rsg:" command in a script.
 */
void MicrowaveSystem::initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id )
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
	infoControl.InsertColumn (0, "#", 80);
	infoControl.InsertColumn (1, "Frequency (GHz)", 200);
	infoControl.InsertColumn (2, "Power (dBm)", 180);
	pos.y += 100;
}


void MicrowaveSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	header.rearrange( width, height, fonts);
	infoControl.rearrange( width, height, fonts);
}


void MicrowaveSystem::setInfoDisp(UINT variation)
{
	infoControl.DeleteAllItems();
	int count = 0;
	/*
	for (auto event : events[variation])
	{
		infoControl.InsertItem (str (count + 1), count, 0);
		infoControl.SetItem (str (event.frequency), count, 1);
		infoControl.SetItem (str (event.power), count, 2);
		count++;
	}
	*/
}

MicrowaveCore& MicrowaveSystem::getCore ()
{
	return core;
}
