// created by Mark O. Brown
#include "stdafx.h"
#include "MicrowaveSystem.h"
#include "TextPromptDialog.h"
#include "GpibFlume.h"
#include "constants.h"
#include "AuxiliaryWindow.h"

MicrowaveSystem::MicrowaveSystem() {}

const std::string MicrowaveSystem::delim = "MICROWAVE_SYSTEM";

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
	header.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	header.Create( "Microwave System", NORM_HEADER_OPTIONS, header.sPos, master, id++ );
	header.fontType = fontTypes::HeadingFont;

	controlOption.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 20 };
	controlOption.Create ("Control?", NORM_CHECK_OPTIONS, controlOption.sPos, master, id++);

	programNow.sPos = { pos.x+240, pos.y, pos.x + 480, pos.y += 20 };
	programNow.Create ("Program Now", NORM_PUSH_OPTIONS, controlOption.sPos, master, IDC_UW_SYSTEM_PROGRAM_NOW);

	uwListListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	uwListListview.Create( NORM_LISTVIEW_OPTIONS, uwListListview.sPos, master, IDC_UW_SYSTEM_LISTVIEW );
	uwListListview.fontType = fontTypes::SmallFont;
	uwListListview.SetBkColor( RGB( 15, 15, 15 ) );
	uwListListview.SetTextBkColor( RGB( 15, 15, 15 ) );
	uwListListview.SetTextColor( RGB( 255, 255, 255 ) );
	uwListListview.InsertColumn (0, "#", 80);
	uwListListview.InsertColumn (1, "Frequency (GHz)", 200);
	uwListListview.InsertColumn (2, "Power (dBm)", 180);
	refreshListview ();
	pos.y += 100;
}


void MicrowaveSystem::handleSaveConfig (std::ofstream& saveFile)
{
	saveFile << delim << "\n";
	saveFile << controlOption.GetCheck () << "\n";
	saveFile << currentList.size () << "\n";
	for (auto listElem : currentList)
	{
		saveFile << listElem.frequency.expressionStr << "\n";
		saveFile << listElem.power.expressionStr << "\n";
	}
	saveFile << "END_" << delim << "\n";
}


microwaveSettings MicrowaveSystem::getMicrowaveSettingsFromConfig (std::ifstream& openFile, Version ver)
{
	microwaveSettings settings;
	openFile >> settings.control;
	UINT numInList = 0;
	openFile >> numInList;
	if (numInList > 100) 
	{
		auto res = promptBox ("Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
			" was " + str(numInList) + ". Is this acceptable?", MB_YESNO );
		if (!res)
		{
			thrower ("Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
					" was " + str(numInList) + ".");
		}
	}
	settings.list.resize (numInList);
	if (numInList > 0)
	{
		openFile.get ();
	}
	for (auto num : range (numInList))
	{
		std::getline ( openFile, settings.list[num].frequency.expressionStr );
		std::getline ( openFile, settings.list[num].power.expressionStr );
	}
	return settings;
}


void MicrowaveSystem::setMicrowaveSettings (microwaveSettings settings)
{
	controlOption.SetCheck (settings.control);
	currentList = settings.list;
	refreshListview ();
}


void MicrowaveSystem::handleListviewDblClick ()
{
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos (&myItemInfo.pt);
	uwListListview.ScreenToClient (&myItemInfo.pt);
	uwListListview.SubItemHitTest (&myItemInfo);
	int subitem, itemIndicator;
	itemIndicator = myItemInfo.iItem;
	if (itemIndicator == -1)
	{
		return;
	}
	CString text = uwListListview.GetItemText (itemIndicator, 0);
	if (text == "___")
	{	// add a step in the list
		currentList.resize (currentList.size () + 1);
		refreshListview ();
		return;
	}
	auto& listItem = currentList[itemIndicator];
	subitem = myItemInfo.iSubItem;
	/// Handle different subitem clicks
	switch (subitem)
	{
		case 0:
		{	/// Number in list, non-responsive
			break;
		}
		case 1:
		{	// power
			std::string freqTxt;
			TextPromptDialog dialog (&freqTxt, "Please enter a frequency in Hz for this list element.");
			dialog.DoModal ();
			listItem.frequency = freqTxt;
			break;
		}
		case 2:
		{	// power
			std::string powerTxt;
			TextPromptDialog dialog (&powerTxt, "Please enter a power in dBm for this list element.");
			dialog.DoModal ();
			listItem.power = powerTxt;
			break;
		}
	}
	refreshListview ();
}

void MicrowaveSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	header.rearrange( width, height, fonts);
	controlOption.rearrange (width, height, fonts);
	programNow.rearrange (width, height, fonts);
	uwListListview.rearrange( width, height, fonts);
}


void MicrowaveSystem::refreshListview ()
{
	UINT count = 0;
	uwListListview.DeleteAllItems ();
	for (auto listElem : currentList)
	{
		uwListListview.InsertItem (str (count + 1), count, 0);
		uwListListview.SetItem (listElem.frequency.expressionStr, count, 1);
		uwListListview.SetItem (listElem.power.expressionStr, count, 2);
		count++;
	}
	uwListListview.insertBlankRow ();
}


MicrowaveCore& MicrowaveSystem::getCore ()
{
	return core;
}

