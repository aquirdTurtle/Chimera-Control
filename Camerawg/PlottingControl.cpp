
#include "stdafx.h"
#include "PlottingControl.h"
#include "Control.h"
#include "CameraWindow.h"

void PlottingControl::initialize(cameraPositions& pos, int& id, CWnd* parent)
{
	header.ksmPos = { pos.ksmPos.x,  pos.ksmPos.y,  pos.ksmPos.x + 480, pos.ksmPos.y + 25 };
	header.cssmPos = { pos.cssmPos.x, pos.cssmPos.y, pos.cssmPos.x + 480, pos.cssmPos.y + 25 };
	header.amPos = { pos.amPos.x,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y + 25 };
	header.ID = id++;
	header.Create("PLOTTING CONTROLS", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER, header.ksmPos, parent, header.ID);
	header.fontType = "Normal";
	pos.ksmPos.y += 25;
	pos.cssmPos.y += 25;
	pos.amPos.y += 25;
	/// PLOTTING FREQUENCY CONTROLS
	// Set Plotting Frequency
	updateFrequencyLabel1.ksmPos =  { pos.ksmPos.x,  pos.ksmPos.y,  pos.ksmPos.x  + 140, pos.ksmPos.y  + 25 };
	updateFrequencyLabel1.cssmPos = { pos.cssmPos.x, pos.cssmPos.y, pos.cssmPos.x + 140, pos.cssmPos.y + 25 };
	updateFrequencyLabel1.amPos =   { pos.amPos.x,   pos.amPos.y,   pos.amPos.x   + 140, pos.amPos.y   + 25 };
	updateFrequencyLabel1.ID = id++;
	updateFrequencyLabel1.triggerModeSensitive = true;
	updateFrequencyLabel1.Create("Update plots every (", WS_CHILD | WS_VISIBLE | WS_BORDER,
		updateFrequencyLabel1.ksmPos, parent, updateFrequencyLabel1.ID);
	updateFrequencyLabel1.fontType = "Normal";
	// Plotting Frequency Edit
	updateFrequencyEdit.ksmPos = { pos.ksmPos.x + 140, pos.ksmPos.y,pos.ksmPos.x + 200, pos.ksmPos.y + 25 };
	updateFrequencyEdit.cssmPos = { pos.cssmPos.x + 140, pos.cssmPos.y, pos.cssmPos.x + 200, pos.cssmPos.y + 25 };
	updateFrequencyEdit.amPos = { pos.amPos.x + 140, pos.amPos.y, pos.amPos.x + 200, pos.amPos.y + 25 };
	updateFrequencyEdit.triggerModeSensitive = 1;
	updateFrequencyEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, updateFrequencyEdit.ksmPos, parent,
		updateFrequencyEdit.ID);
	updateFrequencyEdit.fontType = "Normal";
	// end of that statement
	updateFrequencyLabel2.ksmPos =  { pos.ksmPos.x + 200,  pos.ksmPos.y,  pos.ksmPos.x + 480, pos.ksmPos.y + 25 };
	updateFrequencyLabel2.cssmPos = { pos.cssmPos.x + 200, pos.cssmPos.y, pos.cssmPos.x + 480, pos.cssmPos.y + 25 };
	updateFrequencyLabel2.amPos =   { pos.amPos.x + 200,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y + 25 };
	updateFrequencyLabel2.ID = id++;
	updateFrequencyLabel2.triggerModeSensitive = true;
	updateFrequencyLabel2.Create(") repetitions.", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		updateFrequencyLabel2.ksmPos, parent, updateFrequencyLabel2.ID);
	updateFrequencyLabel2.fontType = "Normal";
	pos.ksmPos.y += 25;
	pos.cssmPos.y += 25;
	pos.amPos.y += 25;
	/// the listview
	plotListview.ksmPos = { pos.ksmPos.x,   pos.ksmPos.y,  pos.ksmPos.x + 480,  pos.ksmPos.y + 100 };
	plotListview.cssmPos = { pos.cssmPos.x, pos.cssmPos.y, pos.cssmPos.x + 480, pos.cssmPos.y + 100 };
	plotListview.amPos = { pos.amPos.x,     pos.amPos.y,   pos.amPos.x + 480,   pos.amPos.y + 100 };
	plotListview.ID = id++;
	plotListview.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS, plotListview.ksmPos, parent,
		plotListview.ID);
	plotListview.fontType == "Normal";
	// initialize the listview
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "Name";
	// width between each coloum
	listViewDefaultCollumn.cx = 0x62;
	// Inserting Collumbs as much as we want
	plotListview.InsertColumn(0, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Active?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "View Details?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Edit?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
	listViewDefaultItem.cchTextMax = 256; // Max size of test
	listViewDefaultItem.pszText = "___";
	listViewDefaultItem.iItem = 0;          // choose item  
	listViewDefaultItem.iSubItem = 0;       // Put in first coluom
	plotListview.InsertItem(&listViewDefaultItem);
	for (int itemInc = 1; itemInc <= 1; itemInc++) // Add SubItems in a loop
	{
		listViewDefaultItem.iSubItem = itemInc;
		// Enter text to SubItems
		plotListview.SetItem(&listViewDefaultItem);
	}
	return;
}

void PlottingControl::handleClick()
{

}

void PlottingControl::addPlot()
{

}

void PlottingControl::openPlotter()
{

}
