// created by Mark O. Brown
#include "stdafx.h"
#include "Microwave/MicrowaveSystem.h"
#include "GeneralFlumes/GpibFlume.h"
#include "LowLevel/constants.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include <qheaderview.h>
#include <qmenu.h>
#include "PrimaryWindows/QtMainWindow.h"
#include <QTableWidget.h>

MicrowaveSystem::MicrowaveSystem() {}

std::string MicrowaveSystem::getIdentity()
{ 
	return core.queryIdentity();
}

void MicrowaveSystem::handleContextMenu (const QPoint& pos)
{
	QTableWidgetItem* item = uwListListview->itemAt (pos);
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete This Item", uwListListview);
	uwListListview->connect (deleteAction, &QAction::triggered, [this, item]() {uwListListview->removeRow (item->row ()); });
	auto* newPerson = new QAction ("New Item", uwListListview);
	uwListListview->connect (newPerson, &QAction::triggered,
		[this]() {currentList.push_back (microwaveListEntry ()); refreshListview (); });
	if (item) { menu.addAction (deleteAction); }
	menu.addAction (newPerson);
	menu.exec (uwListListview->mapToGlobal (pos));
}

/*
 * The controls in this class only display information about what get's programmed to the RSG. They do not
 * (by design) provide an interface for which the user to change the programming of the RSG directly. The
 * user is to do this by using the "rsg:" command in a script.
 */
void MicrowaveSystem::initialize( POINT& pos, IChimeraWindowWidget* parent )
{
	// controls
	header = new QLabel ("MICROWAVE SYSTEM", parent);
	header->setGeometry (pos.x, pos.y, 480, 25);
	
	controlOptionCheck = new QCheckBox ("Control?", parent);
	controlOptionCheck->setGeometry (pos.x, pos.y += 25, 240, 20);

	programNowPush = new QPushButton ("Program Now", parent);
	programNowPush->setGeometry (pos.x + 240, pos.y, 240, 20);
	parent->connect (programNowPush, &QPushButton::released, [this, parent]() {
		try	{
			programNow (parent->auxWin->getUsableConstants ());
		}
		catch (Error& err) {
			parent->reportErr ("Failed to program microwave system! " + err.trace ());
		}
	});

	uwListListview = new QTableWidget (parent);
	uwListListview->setGeometry (pos.x, pos.y += 20, 480, 100);
	uwListListview->setColumnCount (3);
	QStringList labels;
	labels << "#" << "Frequency (GHz)" << "Power (dBm)";
	uwListListview->setHorizontalHeaderLabels (labels);
	uwListListview->horizontalHeader ()->setFixedHeight (25);
	uwListListview->verticalHeader ()->setFixedWidth (25);
	uwListListview->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (uwListListview, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {this->handleContextMenu (pos); });
	uwListListview->setGeometry (pos.x, pos.y, 480, 205);
	uwListListview->setColumnWidth (0, 80);
	uwListListview->setColumnWidth (1, 200);
	uwListListview->setColumnWidth (2, 180);
	uwListListview->setShowGrid (true);
	refreshListview ();
	pos.y += 100;
}

void MicrowaveSystem::handleReadPress (){
	auto res = core.uwFlume.read ();
	readTxt->setText (cstr (res));
	errBox (res);
}

void MicrowaveSystem::handleWritePress (){
	auto txt = writeTxt->text ();
	core.uwFlume.write(str(txt));
}


void MicrowaveSystem::programNow(std::vector<parameterType> constants){
	microwaveSettings settings;
	// ignore the check if the user literally presses program now.
	settings.control = true;
	settings.list = currentList;
	std::string warnings;
	core.calculateVariations (constants);
	core.programVariation (0, constants);
}


void MicrowaveSystem::handleSaveConfig (ConfigStream& saveFile)
{
	saveFile << core.configDelim
		<< "\n/*Control?*/ " << controlOptionCheck->isChecked ()
		<< "\n/*List Size:*/ " << currentList.size ();
	for (auto listElem : currentList)
	{
		saveFile << "\n/*Freq:*/ " << listElem.frequency 
				 << "\n/*Power:*/ " << listElem.power;
	}
	saveFile << "\nEND_" << core.configDelim << "\n";
}

void MicrowaveSystem::setMicrowaveSettings (microwaveSettings settings)
{
	controlOptionCheck->setChecked (settings.control);
	currentList = settings.list;
	refreshListview ();
}


void MicrowaveSystem::handleListviewRClick ()
{
	/*
	POINT cursorPos;
	GetCursorPos (&cursorPos);
	uwListListview.ScreenToClient (&cursorPos);
	int subitemIndicator = uwListListview.HitTest (cursorPos);
	LVHITTESTINFO myItemInfo;
	memset (&myItemInfo, 0, sizeof (LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = uwListListview.SubItemHitTest (&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == currentList.size ())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if (UINT (itemIndicator) < currentList.size ())
	{
		answer = promptBox ("Delete List Element #" + str(itemIndicator+1) + "?", MB_YESNO);
		if (answer == IDYES)
		{
			uwListListview.DeleteItem (itemIndicator);
			currentList.erase (currentList.begin () + itemIndicator);
		}

	}
	else if (UINT (itemIndicator) > currentList.size ())
	{
		answer = promptBox ("You appear to have found a bug with the listview control... there are too many lines "
			"in this control. Clear this line?", MB_YESNO);
		if (answer == IDYES)
		{
			uwListListview.DeleteItem (itemIndicator);
		}
	}
	*/
	refreshListview ();
}


void MicrowaveSystem::handleListviewDblClick ()
{
	/*
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
		{	// freq
			std::string freqTxt;
			TextPromptDialog dialog (&freqTxt, "Please enter a frequency in Hz for this list element.", listItem.frequency.expressionStr);
			dialog.DoModal ();
			if (freqTxt != "") { listItem.frequency = freqTxt; }
			break;
		}
		case 2:
		{	// power
			std::string powerTxt;
			TextPromptDialog dialog (&powerTxt, "Please enter a power in dBm for this list element.", listItem.power.expressionStr);
			dialog.DoModal ();
			if (powerTxt != "") { listItem.power = powerTxt; }
			break;
		}
	}
	refreshListview ();
	*/
}


void MicrowaveSystem::refreshListview ()
{
	UINT count = 0;
	uwListListview->setRowCount (0);
	for (auto listElem : currentList){
		auto ind = uwListListview->rowCount ();
		uwListListview->insertRow (ind);
		uwListListview->setItem (ind, 0, new QTableWidgetItem (cstr(count)));
		uwListListview->item (ind, 0)->setFlags (uwListListview->item (ind, 0)->flags () ^ Qt::ItemIsEnabled);
		uwListListview->setItem (ind, 1, new QTableWidgetItem (cstr (listElem.frequency.expressionStr)));
		uwListListview->setItem (ind, 2, new QTableWidgetItem (cstr (listElem.power.expressionStr)));
		count++;
	}
}


MicrowaveCore& MicrowaveSystem::getCore ()
{
	return core;
}

