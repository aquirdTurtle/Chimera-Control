#include "stdafx.h"
#include "ftd2xx.h"
#include "DirectDigitalSynthesis/DdsSystem.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "GeneralObjects/multiDimensionalKey.h"
#include "afxcmn.h"
#include <boost/lexical_cast.hpp>
#include <qheaderview.h>
#include <qmenu.h>
#include <PrimaryWindows/QtMainWindow.h>

DdsSystem::DdsSystem ( bool ftSafemode ) : core( ftSafemode ) { }

void DdsSystem::handleContextMenu (const QPoint& pos){
	QTableWidgetItem* item = rampListview->itemAt (pos);
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete This Item", rampListview);
	rampListview->connect (deleteAction, &QAction::triggered, [this, item]() {
		getDataFromTable ();
		currentRamps.erase(currentRamps.begin()+item->row());
		redrawListview ();
		});
	auto* newPerson = new QAction ("New Item", rampListview);
	rampListview->connect (newPerson, &QAction::triggered,
		[this]() {
			getDataFromTable ();
			currentRamps.push_back (ddsIndvRampListInfo ());
			redrawListview ();
		});
	if (item) { menu.addAction (deleteAction); }
	menu.addAction (newPerson);
	menu.exec (rampListview->mapToGlobal (pos));
}

void DdsSystem::initialize ( POINT& pos, IChimeraQtWindow* parent, std::string title ){
	ddsHeader = new QLabel (cstr (title), parent);
	ddsHeader->setGeometry (pos.x, pos.y, 480, 25);

	programNowButton = new QPushButton ("Program Now", parent);
	programNowButton->setGeometry (pos.x, pos.y + 25, 360, 25);
	parent->connect (programNowButton, &QPushButton::released, [this, parent]() {
		try	{
			programNow (parent->auxWin->getUsableConstants ());
		}
		catch (ChimeraError& err) {
			parent->reportErr (err.qtrace ());
		}
	});
	controlCheck = new CQCheckBox ("Control?", parent);
	controlCheck->setGeometry (pos.x + 360, pos.y += 25, 120, 25);

	rampListview = new QTableWidget (parent);
	rampListview->setGeometry (pos.x, pos.y+= 25, 480, 160);
	pos.y += 160;
	rampListview->horizontalHeader ()->setFixedHeight (30);
	rampListview->setColumnWidth (0, 60);
	rampListview->setColumnWidth (1, 60);
	rampListview->setColumnWidth (2, 60);
	rampListview->setColumnWidth (3, 60);
	rampListview->setColumnWidth (4, 60);
	rampListview->setColumnWidth (5, 60);
	rampListview->setColumnWidth (6, 120);
	//rampListview->verticalHeader ()->setDefaultSectionSize (60);

	rampListview->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (rampListview, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {handleContextMenu (pos); });
	rampListview->setShowGrid (true);

	QStringList labels;
	labels << "Index" << "Channel" << "Freq 1" << "Amp 1" << "Freq 2" << "Amp 2" << "Time";
	rampListview->setColumnCount (labels.size());
	rampListview->setHorizontalHeaderLabels (labels);
}

void DdsSystem::redrawListview ( ){
	rampListview->setRowCount (0);
	for (auto rampInc : range (currentRamps.size ()))
	{
		rampListview->insertRow (rampListview->rowCount ());
		auto rowN = rampListview->rowCount ()-1;
		auto& ramp = currentRamps[rampInc];
		rampListview->setItem (rowN, 0, new QTableWidgetItem (cstr (ramp.index)));
		rampListview->setItem (rowN, 1, new QTableWidgetItem (cstr (ramp.channel)));
		rampListview->setItem (rowN, 2, new QTableWidgetItem (cstr (ramp.freq1.expressionStr)));
		rampListview->setItem (rowN, 3, new QTableWidgetItem (cstr (ramp.amp1.expressionStr)));
		rampListview->setItem (rowN, 4, new QTableWidgetItem (cstr (ramp.freq2.expressionStr)));
		rampListview->setItem (rowN, 5, new QTableWidgetItem (cstr (ramp.amp2.expressionStr)));
		rampListview->setItem (rowN, 6, new QTableWidgetItem (cstr (ramp.rampTime.expressionStr)));
	}
}

void DdsSystem::handleRampClick (  )
{
	/*
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
			TextPromptDialog dialog ( &newIndexStr, "Please enter a ramp index (0-255):", str(ramp.index));
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
			TextPromptDialog dialog ( &newChannelStr, "Please enter a channel number (0-7):", str(ramp.channel) );
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
				Expression* expr = NULL;
				switch (subitem)
				{
					case 2: expr = &ramp.freq1;		break;
					case 3: expr = &ramp.amp1;		break;
					case 4: expr = &ramp.freq2;		break;
					case 5: expr = &ramp.amp2;		break;
					case 6: expr = &ramp.rampTime;	break;
					default:
						thrower ("Bad Subitem in DDS system?!?!");
				}

				TextPromptDialog dialog ( &valStr, "Please enter an Expression for this value:", expr->expressionStr);
				dialog.DoModal ( );
				valStr = str (valStr, 13, false, true);
				if ( valStr == "" )
				{
					// probably canceled.
					break;
				}
				*expr = valStr;
			}
		}
	}
	redrawListview ( );
	*/
}

void DdsSystem::deleteRampVariable ( )
{
	/*
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
	if ( unsigned ( itemIndicator ) < currentRamps.size ( ) )
	{
		answer = promptBox ( "Delete Ramp # " + str(itemIndicator+1) + "?", MB_YESNO );
		if ( answer == IDYES )
		{
			currentRamps.erase ( currentRamps.begin ( ) + itemIndicator );
		}
	}
	redrawListview ( );
	*/
}


void DdsSystem::programNow ( std::vector<parameterType>& constants ){
	try{
		std::vector<ddsIndvRampListInfo> simpleExp;
		simpleExp = currentRamps;
		core.evaluateDdsInfo ( );
		core.generateFullExpInfo ( 1 );
		core.programVariation ( 0, constants);
	}
	catch ( ChimeraError& ){
		throwNested ( "Error seen while programming DDS system via Program Now Button." );
	}
}


void DdsSystem::handleSaveConfig (ConfigStream& file ){
	getDataFromTable ();
	file << getDelim() << "\n";
	file << "/*Control?*/ " << controlCheck->isChecked () << "\n";
	core.writeRampListToConfig ( currentRamps, file );
	file << "\nEND_" + getDelim ( ) << "\n";
}


void DdsSystem::handleOpenConfig ( ConfigStream& file ){
	if ( file.ver >= Version ( "4.5" ) ){
		auto res = core.getSettingsFromConfig (file);
		currentRamps = res.ramplist;
		controlCheck->setChecked (res.control);
	}
	redrawListview ( );
}


std::string DdsSystem::getSystemInfo ( ){
	return core.getSystemInfo();
}


std::string DdsSystem::getDelim ( ){
	return core.configDelim;
}

DdsCore& DdsSystem::getCore ( ){
	return core;
}

void DdsSystem::getDataFromTable () {
	currentRamps.resize (rampListview->rowCount ());
	for (auto rowI : range(rampListview->rowCount ())) {
		try {
			currentRamps[rowI].index = boost::lexical_cast<int>(cstr(rampListview->item (rowI, 0)->text ()));
			currentRamps[rowI].channel = boost::lexical_cast<int>(cstr (rampListview->item (rowI, 1)->text ()));
			currentRamps[rowI].freq1 = str (rampListview->item (rowI, 2)->text ());
			currentRamps[rowI].amp1 = str(rampListview->item (rowI, 3)->text ());
			currentRamps[rowI].freq2 = str(rampListview->item (rowI, 4)->text ());
			currentRamps[rowI].amp2 = str(rampListview->item (rowI, 5)->text ());
			currentRamps[rowI].rampTime = str(rampListview->item (rowI, 6)->text ());
		}
		catch (ChimeraError &) {
			throwNested ("Failed to convert dds table data to ramp structure!");
		}
	}
}