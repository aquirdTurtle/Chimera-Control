
#include "stdafx.h"
#include "DataAnalysisHandler.h"
#include "Control.h"
#include "CameraWindow.h"

void DataAnalysisControl::rearrange(std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts)
{
	updateFrequencyLabel1.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyLabel2.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyEdit.rearrange(cameraMode, trigMode, width, height, fonts);
	header.rearrange(cameraMode, trigMode, width, height, fonts);
	plotListview.rearrange(cameraMode, trigMode, width, height, fonts);
	currentDataSetNumberEdit.rearrange( cameraMode, trigMode, width, height, fonts );
	currentDataSetNumberText.rearrange( cameraMode, trigMode, width, height, fonts );
	setAnalysisLocationsButton.rearrange( cameraMode, trigMode, width, height, fonts );
	// analyzeMostRecentButton.rearrange( cameraMode, trigMode, width, height, fonts );
	// autoAnalyzeCheckBox.rearrange( cameraMode, trigMode, width, height, fonts );
}

std::vector<std::string> DataAnalysisControl::getActivePlotList()
{
	std::vector<std::string> list;
	for ( auto plot : this->allPlots )
	{
		if ( plot.isActive ) 
		{
			list.push_back( plot.name );
		}
	}
	return list;
}

bool DataAnalysisControl::getLocationSettingStatus()
{
	return currentlySettingAnalysisLocations;
}

void DataAnalysisControl::updateDataSetNumberEdit( int number )
{
	if ( number > 0 )
	{
		currentDataSetNumberEdit.SetWindowTextA( cstr( number ) );
	}
	else
	{
		currentDataSetNumberEdit.SetWindowTextA( "None" );
	}
}

void DataAnalysisControl::analyze( std::string date, long runNumber, long accumulations, 
								   EmbeddedPythonHandler* pyHandler, Communicator* comm )
{
	// Note: python is initialized in the constructor for the data handler object. 
	// Get information to send to the python script from inputParam
	pyHandler->runDataAnalysis( date, runNumber, accumulations, this->atomLocations );
}


// handles the pressing of the analysis points button.
// TODO: handle different cases where single locations or pairs of locations are being analyzed. 
void DataAnalysisControl::onButtonPushed()
{	
	if ( setAnalysisLocationsButton.GetCheck() )
	{
		// if pressed currently, then upress it.
		setAnalysisLocationsButton.SetCheck(0);
		setAnalysisLocationsButton.SetWindowTextA( "Set Analysis Points" );
		currentlySettingAnalysisLocations = false;
	}
	else
	{
		// else press it.
		atomLocations.clear();
		setAnalysisLocationsButton.SetCheck( 1 );
		setAnalysisLocationsButton.SetWindowTextA( "Right-Click Relevant Points and Reclick" );
		currentlySettingAnalysisLocations = true;
	}
	int experimentPictureNumber;
	//if ( eRealTimePictures )
	if ( true )
	{
		experimentPictureNumber = 0;
	}
	else
	{
		//experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition);
	}
}

void DataAnalysisControl::setAtomLocation( std::pair<int, int> location )
{
	bool exists = false;
	for ( int locInc = 0; locInc < this->atomLocations.size(); locInc++ )
	{
		if ( location == atomLocations[locInc] )
		{
			exists = true;
		}
	}
	if ( !exists )
	{
		atomLocations.push_back( location );
	}
}

std::vector<std::pair<int, int>> DataAnalysisControl::getAtomLocations()
{
	return atomLocations;
}

void DataAnalysisControl::clearAtomLocations()
{
	atomLocations.clear();
}

void DataAnalysisControl::initialize(cameraPositions& pos, int& id, CWnd* parent, fontMap fonts, 
									 std::vector<CToolTipCtrl*>& tooltips, int isTriggerModeSensitive)
{

	header.seriesPos = { pos.seriesPos.x,  pos.seriesPos.y,  pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	header.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 25 };
	header.amPos = { pos.amPos.x,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y + 25 };
	header.triggerModeSensitive = isTriggerModeSensitive;
	header.ID = id++;
	header.Create("DATA ANALYSIS", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER, header.seriesPos, parent, header.ID);
	header.fontType = Heading;
	pos.seriesPos.y += 25;
	pos.videoPos.y += 25;
	pos.amPos.y += 25;
	
	/// Data analysis stuffs.
	//
	currentDataSetNumberText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y,
		pos.seriesPos.x + 400, pos.seriesPos.y + 25 };
	currentDataSetNumberText.amPos = { pos.amPos.x, pos.amPos.y,
		pos.amPos.x + 400, pos.amPos.y + 25 };
	currentDataSetNumberText.videoPos = { -1,-1,-1,-1 };
	currentDataSetNumberText.triggerModeSensitive = isTriggerModeSensitive;
	currentDataSetNumberText.ID = id++;
	currentDataSetNumberText.Create( "Most Recent Data Set #:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
									 currentDataSetNumberText.seriesPos, parent, currentDataSetNumberText.ID );
	currentDataSetNumberText.fontType = Normal;
	//
	currentDataSetNumberEdit.seriesPos = { pos.seriesPos.x + 400, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	currentDataSetNumberEdit.amPos = { pos.amPos.x + 400, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 25 };
	currentDataSetNumberEdit.videoPos = { -1,-1,-1,-1 };
	currentDataSetNumberEdit.ID = id++;
	currentDataSetNumberEdit.triggerModeSensitive = isTriggerModeSensitive;
	currentDataSetNumberEdit.Create( "?", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, 
									 currentDataSetNumberEdit.seriesPos, parent, currentDataSetNumberEdit.ID );
	currentDataSetNumberEdit.fontType = Normal;
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;

	/*
	autoAnalyzeCheckBox.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 20 };
	autoAnalyzeCheckBox.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 20 };
	autoAnalyzeCheckBox.videoPos = { -1,-1,-1,-1 };
	autoAnalyzeCheckBox.ID = id++;
	autoAnalyzeCheckBox.triggerModeSensitive = isTriggerModeSensitive;
	autoAnalyzeCheckBox.Create( "Automatically Analyze Data at Finish?", WS_CHILD | WS_VISIBLE | ES_RIGHT |
								ES_READONLY | BS_CHECKBOX, autoAnalyzeCheckBox.seriesPos, parent, autoAnalyzeCheckBox.ID );
	autoAnalyzeCheckBox.fontType = Normal;
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	*/
	// 
	setAnalysisLocationsButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y,
		pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	setAnalysisLocationsButton.amPos = { pos.amPos.x, pos.amPos.y,
		pos.amPos.x + 480, pos.amPos.y + 25 };
	setAnalysisLocationsButton.videoPos = { -1,-1,-1,-1 };
	setAnalysisLocationsButton.ID = id++;
	idVerify(setAnalysisLocationsButton.ID, IDC_SET_ANALYSIS_LOCATIONS);
	setAnalysisLocationsButton.triggerModeSensitive = isTriggerModeSensitive;
	setAnalysisLocationsButton.Create( "Set AutoAnalysis Points", WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_CHECKBOX,
									   setAnalysisLocationsButton.seriesPos, parent, setAnalysisLocationsButton.ID );
	setAnalysisLocationsButton.fontType = Normal;
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;

	//
	/*
	analyzeMostRecentButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y,
		pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	analyzeMostRecentButton.amPos = { pos.amPos.x, pos.amPos.y,
		pos.amPos.x + 480, pos.amPos.y + 25 };
	analyzeMostRecentButton.videoPos = { -1, -1, -1, -1 };
	analyzeMostRecentButton.triggerModeSensitive = isTriggerModeSensitive;
	analyzeMostRecentButton.ID = id++;
	analyzeMostRecentButton.triggerModeSensitive = isTriggerModeSensitive;
	analyzeMostRecentButton.Create( "Analyze Most Recent Data", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
									analyzeMostRecentButton.seriesPos, parent, analyzeMostRecentButton.ID );
	analyzeMostRecentButton.fontType = Normal;
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;
	*/
	/// PLOTTING FREQUENCY CONTROLS
	// Set Plotting Frequency
	updateFrequencyLabel1.seriesPos =  { pos.seriesPos.x,  pos.seriesPos.y,  pos.seriesPos.x  + 150, pos.seriesPos.y  + 25 };
	updateFrequencyLabel1.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 150, pos.videoPos.y + 25 };
	updateFrequencyLabel1.amPos =   { pos.amPos.x,   pos.amPos.y,   pos.amPos.x   + 150, pos.amPos.y   + 25 };
	updateFrequencyLabel1.ID = id++;
	updateFrequencyLabel1.triggerModeSensitive = isTriggerModeSensitive;
	
	updateFrequencyLabel1.Create("Update plots every (", WS_CHILD | WS_VISIBLE | WS_BORDER,
		updateFrequencyLabel1.seriesPos, parent, updateFrequencyLabel1.ID);
	updateFrequencyLabel1.fontType = Normal;
	// Plotting Frequency Edit
	updateFrequencyEdit.seriesPos = { pos.seriesPos.x + 150, pos.seriesPos.y,pos.seriesPos.x + 200, pos.seriesPos.y + 25 };
	updateFrequencyEdit.videoPos = { pos.videoPos.x + 150, pos.videoPos.y, pos.videoPos.x + 200, pos.videoPos.y + 25 };
	updateFrequencyEdit.amPos = { pos.amPos.x + 150, pos.amPos.y, pos.amPos.x + 200, pos.amPos.y + 25 };
	updateFrequencyEdit.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, updateFrequencyEdit.seriesPos, parent,
		updateFrequencyEdit.ID);
	updateFrequencyEdit.fontType = Normal;
	updateFrequency = 5; 
	updateFrequencyEdit.SetWindowTextA("5");
	// end of that statement
	updateFrequencyLabel2.seriesPos =  { pos.seriesPos.x + 200,  pos.seriesPos.y,  pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	updateFrequencyLabel2.videoPos = { pos.videoPos.x + 200, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	updateFrequencyLabel2.amPos =   { pos.amPos.x + 200,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y += 25 };
	updateFrequencyLabel2.ID = id++;
	updateFrequencyLabel2.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyLabel2.Create(") repetitions.", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		updateFrequencyLabel2.seriesPos, parent, updateFrequencyLabel2.ID);
	updateFrequencyLabel2.fontType = Normal;
	/// the listview
	plotListview.seriesPos = { pos.seriesPos.x,   pos.seriesPos.y,  pos.seriesPos.x + 480,  pos.seriesPos.y += 100 };
	plotListview.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 100 };
	plotListview.amPos = { pos.amPos.x,     pos.amPos.y,   pos.amPos.x + 480,   pos.amPos.y += 100 };
	plotListview.ID = id++;
	plotListview.triggerModeSensitive = isTriggerModeSensitive;
	idVerify(plotListview.ID, IDC_PLOTTING_LISTVIEW);
	plotListview.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS, plotListview.seriesPos, parent,
		plotListview.ID);
	plotListview.fontType == Normal;
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
	// Text Style
	listViewDefaultItem.mask = LVIF_TEXT;   
	// Max size of test
	listViewDefaultItem.cchTextMax = 256; 
	listViewDefaultItem.pszText = "___";
	// choose item  
	listViewDefaultItem.iItem = 0;          
	// Put in first coluom
	listViewDefaultItem.iSubItem = 0;       
	plotListview.InsertItem(&listViewDefaultItem);
	// Add SubItems in a loop
	for (int itemInc = 1; itemInc <= 3; itemInc++) 
	{
		listViewDefaultItem.iSubItem = itemInc;
		// Enter text to SubItems
		plotListview.SetItem(&listViewDefaultItem);
	}
}

void DataAnalysisControl::handleDoubleClick()
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	plotListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}
	// TODO: update camera configuration saved status.

	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
								   // choose item
								   /// check if adding new variable
	listViewItem.iItem = itemIndicator;
	if (itemIndicator == allPlots.size())
	{
		// open plot creator.
		// ...
		// T.T
		// add a person
		allPlots.resize(allPlots.size() + 1);
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName;
			//std::string newName = (const char*)DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter a name for the variable:");
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			// rename the file 
			// ...
			// ...
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newName.c_str();
			plotListview.SetItem(&listViewItem);
			break;
		}
		case 1:
		{
			/// active?
			break;
		}
		case 2:
		{
			/// view
			break;
		}
		case 3:
		{
			/// edit?
			break;
		}
	}
	return;
}


void DataAnalysisControl::handleRClick()
{
	// delete...
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator = plotListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = plotListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == allPlots.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer = MessageBox(0, ("Delete Plot " + allPlots[itemIndicator].name + "?").c_str(), 0, MB_YESNO);
	if (answer == IDYES)
	{
		plotListview.DeleteItem(itemIndicator);
		allPlots.erase(allPlots.begin() + itemIndicator);
	}
}
