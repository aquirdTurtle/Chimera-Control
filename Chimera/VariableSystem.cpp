#include "stdafx.h"
#include "VariableSystem.h"
#include "Script.h"
#include "DioSystem.h"
#include "AuxiliaryWindow.h"
#include "TextPromptDialog.h"
#include "multiDimensionalKey.h"

#include <iomanip>
#include <unordered_map>
#include <random>
#include "afxcmn.h"


UINT VariableSystem::getTotalVariationNumber()
{
	return currentVariations;
}


void VariableSystem::initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id, std::string title,
								 rgbMap rgbs, UINT listviewId )
{
	if ( title == "GLOBAL VARIABLES" )
	{
		isGlobal = true;
	}
	else
	{
		isGlobal = false;
	}
	// initialize these numbers to reflect the initial status of the window.
	scanDimensions = 1;
	variableRanges = 1;
	// controls
	variablesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	variablesHeader.Create( cstr( title ), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
							variablesHeader.sPos, master, id++ );
	variablesHeader.fontType = HeadingFont;
	pos.y += 25;

	LONG listViewSize = 0;
	if ( isGlobal )
	{
		listViewSize = 600;
	}
	else
	{
		listViewSize = 250;
	}

	variablesListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + listViewSize };
	variablesListview.Create( WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER, variablesListview.sPos, 
							  master, listviewId );
	
	variablesListview.fontType = SmallFont;
	variablesListview.SetBkColor( RGB( 15, 15, 15 ) );
	variablesListview.SetTextBkColor( RGB( 15, 15, 15 ) );
	variablesListview.SetTextColor( RGB( 255, 255, 255 ) );
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset( &listViewDefaultCollumn, 0, sizeof( listViewDefaultCollumn ) );
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "Symbol";
	if ( isGlobal )
	{
		listViewDefaultCollumn.cx = 200;
		variablesListview.InsertColumn( 0, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Value";
		listViewDefaultCollumn.cx = 200;
		variablesListview.InsertColumn( 1, &listViewDefaultCollumn );
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = 0;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		variablesListview.SetItem( &listViewDefaultItem );
	}
	else
	{
		listViewDefaultCollumn.cx = 50;
		variablesListview.InsertColumn( 0, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Type";
		listViewDefaultCollumn.cx = 0x40;
		variablesListview.InsertColumn( 1, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Dim";
		listViewDefaultCollumn.cx = 0x40;
		variablesListview.InsertColumn( 2, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "1:(";
		listViewDefaultCollumn.cx = 0x35;
		variablesListview.InsertColumn( 3, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "]";
		variablesListview.InsertColumn( 4, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "#";
		variablesListview.InsertColumn( 5, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "+()";
		listViewDefaultCollumn.cx = 0x30;
		variablesListview.InsertColumn( 6, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "-()";
		variablesListview.InsertColumn( 7, &listViewDefaultCollumn );
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = 0;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem( &listViewDefaultItem );
		for ( int itemInc = 1; itemInc < 6; itemInc++ ) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			variablesListview.SetItem( &listViewDefaultItem );
		}
	}
	variablesListview.SetBkColor( rgbs["Solarized Base02"] );

	pos.y += listViewSize;
}


void VariableSystem::handleOpenConfig(std::ifstream& configFile, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine(configFile, "VARIABLES");
	// handle variables
	clearVariables();
	// Number of Variables
	UINT varNum;
	configFile >> varNum;
	if (varNum > 100)
	{
		int answer = promptBox("ERROR: variable number retrieved from file appears suspicious. The number is "
								+ str(varNum) + ". Is this accurate?", MB_YESNO);
		if (answer == IDNO)
		{
			// don't try to load anything.
			varNum = 0;
		}
	}
int rangeNumber = 1;
for ( const UINT varInc : range( varNum ) )
{
	variableType tempVar;
	std::string varName, typeText, valueString;
	bool constant;
	configFile >> varName;
	std::transform( varName.begin( ), varName.end( ), varName.begin( ), ::tolower );
	tempVar.name = varName;
	configFile >> typeText;
	if ( typeText == "Constant" )
	{
		constant = true;
		tempVar.constant = true;
	}
	else if ( typeText == "Variable" )
	{
		constant = false;
		tempVar.constant = false;
	}
	else
	{
		thrower( "ERROR: unknown variable type option: " + typeText + ". Check the formatting of the configuration"
				 " file." );
	}
	if ( (versionMajor == 2 && versionMinor > 7) || versionMajor > 2 )
	{
		configFile >> tempVar.scanDimension;
	}
	else
	{
		tempVar.scanDimension = 1;
	}
	configFile >> rangeNumber;
	// I think it's unlikely to ever need more than 2 or 3 ranges.
	if ( rangeNumber < 1 || rangeNumber > 100 )
	{
		errBox( "ERROR: Bad range number! setting it to 1, but found " + str( rangeNumber ) + " in the file." );
		rangeNumber = 1;
	}
	setVariationRangeNumber( rangeNumber, 1 );
	// check if the range is actually too small.
	UINT totalVariations = 0;
	for ( auto rangeInc : range( rangeNumber ) )
	{
		double initValue = 0, finValue = 0;
		unsigned int variations = 0;
		bool leftInclusive = 0, rightInclusive = 0;
		configFile >> initValue;
		configFile >> finValue;
		configFile >> variations;
		totalVariations += variations;
		configFile >> leftInclusive;
		configFile >> rightInclusive;
		tempVar.ranges.push_back( { initValue, finValue, variations, leftInclusive, rightInclusive } );
	}
	// shouldn't be because of 1 forcing earlier.
	if ( tempVar.ranges.size( ) == 0 )
	{
		// make sure it has at least one entry.
		tempVar.ranges.push_back( { 0,0,1, false, true } );
	}
	addConfigVariable( tempVar, varInc );
}

for ( auto rangeInc : range( rangeNumber ) )
{
	bool leftInclusivity = false, rightInclusivity = true;
	if ( currentVariables.size( ) != 0 )
	{
		leftInclusivity = currentVariables.front( ).ranges[rangeInc].leftInclusive;
		rightInclusivity = currentVariables.front( ).ranges[rangeInc].rightInclusive;
	}
	setRangeInclusivity( rangeInc, true, leftInclusivity, preRangeColumns + rangeInc * 3 );
	setRangeInclusivity( rangeInc, false, rightInclusivity, preRangeColumns + 1 + rangeInc * 3 );
}

// add a blank line
variableType var;
var.name = "";
var.constant = false;
var.ranges.push_back( { 0, 0, 1, false, true } );
addConfigVariable( var, varNum );

ProfileSystem::checkDelimiterLine( configFile, "END_VARIABLES" );
updateVariationNumber( );
}


void VariableSystem::updateVariationNumber( )
{
	// if no variables, or all are constants, it will stay at 1. else, it will get set to the # of variations
	// of the first variable that it finds.

	std::vector<ULONG> dimVariations;
	std::vector<bool> dimsSeen;
	for ( auto tempVariable : currentVariables )
	{
		if ( !tempVariable.constant )
		{
			if ( dimsSeen.size( ) < tempVariable.scanDimension )
			{
				dimVariations.resize( tempVariable.scanDimension, 0 );
				dimsSeen.resize( tempVariable.scanDimension, false );
			}
			if ( dimsSeen[tempVariable.scanDimension-1] )
			{
				// already seen.
				continue;
			}
			else
			{
				// now it's been seen, don't add it again.
				dimsSeen[tempVariable.scanDimension - 1] = true;
			}
			for ( auto range : tempVariable.ranges )
			{
				dimVariations[tempVariable.scanDimension - 1] += range.variations;
			}
		}
	}
	currentVariations = 1;
	for ( auto val : dimVariations )
	{
		currentVariations *= val;
	}
}

 
void VariableSystem::handleNewConfig( std::ofstream& newFile )
{
	newFile << "VARIABLES\n";
	// Number of Variables
	newFile << 0 << "\n";
	newFile << "END_VARIABLES\n";
}


void VariableSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "VARIABLES\n";
	// Number of Variables
	saveFile << getCurrentNumberOfVariables() << "\n";
	/// Variable Names
	for (UINT varInc = 0; varInc < getCurrentNumberOfVariables(); varInc++)
	{
		variableType info = getVariableInfo(varInc);
		saveFile << info.name << " ";
		if (info.constant)
		{
			saveFile << "Constant ";
		}
		else
		{
			saveFile << "Variable ";
		}
		saveFile << info.scanDimension << "\n";
		saveFile << info.ranges.size() << "\n";
		for (auto& range : info.ranges )
		{
			saveFile << range.initialValue << "\n";
			saveFile << range.finalValue << "\n";
			saveFile << range.variations << "\n";
			saveFile << range.leftInclusive << "\n";
			saveFile << range.rightInclusive << "\n";
		}
	}
	saveFile << "END_VARIABLES\n";
}


void VariableSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	variablesHeader.rearrange( width, height, fonts);
	variablesListview.rearrange( width, height, fonts);
}


void VariableSystem::addVariableDimension()
{

}


void VariableSystem::removeVariableDimension()
{
	if (scanDimensions == 1)
	{
		thrower("ERROR: Can't delete last variable scan dimension.");
	}
	// change all variables in the last dimension to be in the second-to-last dimension.
	// TODO: I'm gonna have to check variation numbers here or change them to be compatible.
	for (auto& variable : currentVariables)
	{
		if (variable.scanDimension == scanDimensions)
		{
			variable.scanDimension--;
		}
	}
	// find the last such dimension border item.
	UINT itemNumber = variablesListview.GetItemCount();
	for (UINT item = itemNumber; item >= 0; item--)
	{
		CString text;
		text = variablesListview.GetItemText(item, 0);
		if (text == "Symbol")
		{
			// delete "new" for this border range
			variablesListview.DeleteItem(variablesListview.GetItemCount()-1);
			variablesListview.DeleteItem(item);
			break;
		}
	}
	scanDimensions--;
}


void VariableSystem::setVariationRangeNumber(int num, USHORT dimNumber)
{
	int currentVariableRangeNumber = (variablesListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
	if (variableRanges != currentVariableRangeNumber)
	{
		errBox( "ERROR: somehow, the number of ranges the VariableSystem object thinks there are and the actual number "
				"are off! The numbes are " + str(variableRanges) + " and "
				+ str(currentVariableRangeNumber) + " respectively. The program will attempt to fix this, but " 
				"data may be lost." );
		variableRanges = currentVariableRangeNumber;
		for (auto& variable : currentVariables)
		{
			variable.ranges.resize(currentVariableRangeNumber);
		}
	}
	if (currentVariableRangeNumber < num)
	{
		while (currentVariableRangeNumber < num)
		{
			/// add a range.
			LV_COLUMN listViewDefaultCollumn;
			// Zero Members
			memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
			// Type of mask
			listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			// width between each coloum
			std::string temp(str(str(variableRanges + 1) + ":("));
			listViewDefaultCollumn.pszText = &temp[0];
			listViewDefaultCollumn.cx = 0x20;
			variablesListview.InsertColumn( preRangeColumns + 3 * currentVariableRangeNumber, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "]";
			variablesListview.InsertColumn( preRangeColumns+1 + 3 * currentVariableRangeNumber, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "#";
			variablesListview.InsertColumn( preRangeColumns+2 + 3 * currentVariableRangeNumber, &listViewDefaultCollumn);
			// edit all variables
			LVITEM listViewItem;
			memset(&listViewItem, 0, sizeof(listViewItem));
			listViewItem.mask = LVIF_TEXT; 
			listViewItem.cchTextMax = 256; 
			for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
			{
				variationRangeInfo tempInfo{ 0,0,0, false, true };
				currentVariables[varInc].ranges.push_back( tempInfo );
				if (currentVariables[varInc].constant)
				{
					listViewItem.pszText = "---";
				}
				else
				{
					listViewItem.pszText = "0";
				}
				listViewItem.iItem = varInc;
				listViewItem.iSubItem = preRangeColumns + 3 * currentVariableRangeNumber;
				variablesListview.SetItem(&listViewItem);
				listViewItem.iSubItem = preRangeColumns + 1 + 3 * currentVariableRangeNumber;
				variablesListview.SetItem(&listViewItem);
				listViewItem.iSubItem = preRangeColumns + 2+ 3 * currentVariableRangeNumber;
				variablesListview.SetItem(&listViewItem);
			}
			int newRangeNum = (variablesListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			// make sure this makes sense.
			if (currentVariableRangeNumber != newRangeNum - 1)
			{
				errBox("Error! Range numbers after new range don't make sense!");
			}
			currentVariableRangeNumber = newRangeNum;
			variableRanges = currentVariableRangeNumber;
		}
	}
	else if (currentVariableRangeNumber > num)
	{
		while (currentVariableRangeNumber > num)
		{
			// delete a range.
			if (variableRanges == 1)
			{
				// can't delete last set...
				return;
			}

			variablesListview.DeleteColumn( preRangeColumns + 3 * (currentVariableRangeNumber - 1));
			variablesListview.DeleteColumn( preRangeColumns + 3 * (currentVariableRangeNumber - 1));
			variablesListview.DeleteColumn( preRangeColumns + 3 * (currentVariableRangeNumber - 1));
			// edit all variables
			for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
			{
				currentVariables[varInc].ranges.pop_back();
			}
			// account for the first 2 columns (name, constant) and the last three (currently the extra dimension stuff
			int newRangeNum = (variablesListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			currentVariableRangeNumber = newRangeNum;
			variableRanges = currentVariableRangeNumber;
		}
	}
	// if equal, nothing needs to be done.
}


void VariableSystem::handleColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	variablesListview.ScreenToClient(&cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	variablesListview.SubItemHitTest(&myItemInfo);
	if (myItemInfo.iSubItem == preRangeColumns + 3 * variableRanges)
	{
		// add a range.
		setVariationRangeNumber(variableRanges + 1, 1);
	}
	else if (myItemInfo.iSubItem == preRangeColumns + 1 + 3 * variableRanges )
	{
		// delete a range.
		setVariationRangeNumber(variableRanges - 1, 1);
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem - preRangeColumns) % 3 == 0)
	{
		// switch between [ and (
		if ( currentVariables.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, true, !currentVariables.front( ).ranges[rangeNum].leftInclusive, 
							 myItemInfo.iSubItem );
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem- preRangeColumns) % 3 == 1)
	{
		// switch between ] and )
		if ( currentVariables.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, false, !currentVariables.front( ).ranges[rangeNum].rightInclusive, 
							 myItemInfo.iSubItem );
	}
}


void VariableSystem::setRangeInclusivity( UINT rangeNum, bool leftBorder, bool inclusive, UINT column )
{
	for ( auto& variable : currentVariables )
	{
		if ( leftBorder )
		{
			variable.ranges[rangeNum].leftInclusive = inclusive;
		}
		else
		{
			// it's the right border then.
			variable.ranges[rangeNum].rightInclusive = inclusive;
		}
	}

	if ( leftBorder )
	{
		LVCOLUMNA colInfo;
		ZeroMemory( &colInfo, sizeof( colInfo ) );
		colInfo.cchTextMax = 100;
		colInfo.mask = LVCF_TEXT;
		variablesListview.GetColumn( column, &colInfo );
		std::string text;
		if ( inclusive )
		{
			text = str( rangeNum + 1 ) + ". [";
		}
		else
		{
			text = str( rangeNum + 1 ) + ". (";
		}
		colInfo.pszText = &text[0];
		variablesListview.SetColumn( column, &colInfo );
	}
	else
	{
		// it's the right border then.
		LVCOLUMNA colInfo;
		colInfo.cchTextMax = 100;
		ZeroMemory( &colInfo, sizeof( colInfo ) );
		colInfo.mask = LVCF_TEXT;
		variablesListview.GetColumn( column, &colInfo );
		if ( inclusive )
		{
			colInfo.pszText = "]";
		}
		else
		{
			colInfo.pszText = ")";
		}
		variablesListview.SetColumn( column, &colInfo );
	}
}


void VariableSystem::handleDraw(NMHDR* pNMHDR, LRESULT* pResult, rgbMap rgbs)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// Take the default processing unless we 
	// set this to something else below.
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		int item = pLVCD->nmcd.dwItemSpec;
		if (item < 0)
		{
			return;
		}

		if (isGlobal)
		{
			if (item == currentVariables.size())
			{
				pLVCD->clrText = RGB(255, 255, 255);
				pLVCD->clrTextBk = rgbs["Solarized Base02"];
			}
			else
			{
				if (currentVariables[item].active)
				{
					pLVCD->clrTextBk = rgbs["Solarized Blue"];
				}
				else
				{
					pLVCD->clrTextBk = rgbs["Solarized Base04"];
				}

				if (currentVariables[item].overwritten)
				{
					pLVCD->clrText = rgbs["Solarized Red"];
				}
				else
				{
					pLVCD->clrText = rgbs["Solarized Base2"];
				}
			}
		}
		else
		{
			CString text;
			text = variablesListview.GetItemText(item, 0);
			if (text == "Symbol")
			{
				// then its a range divider.
				pLVCD->clrText = RGB(0, 0, 0);
				pLVCD->clrTextBk = RGB(255, 255, 255);
			}
		}
		// Tell Windows to paint the control itself.
		*pResult = CDRF_DODEFAULT;
	}
}


void VariableSystem::updateVariableInfo( std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin,
										 DioSystem* ttls, DacSystem* dacs )
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	variablesListview.ScreenToClient(&cursorPos);
	//NMITEMACTIVATE itemClicked = *(NMITEMACTIVATE*)lParamOfMessage;
	int subitem;// = variablesListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	variablesListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	if (itemIndicator == -1)
	{
		return;
	}
	UINT borderCount = 0;
	for (auto rowCount : range(itemIndicator))
	{
		if (variablesListview.GetItemText(rowCount, 0) == "___" ||variablesListview.GetItemText(rowCount, 0) == "Symbol"  )
		{
			borderCount++;
		}
	}
	UINT scanDim = (borderCount + 1) / 2;
	UINT varNumber = itemIndicator - borderCount;
	subitem = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	// choose item
	/// check if adding new variable
	listViewItem.iItem = itemIndicator;
	CString text = variablesListview.GetItemText(itemIndicator, 0);
	if (text == "___")
	{
		// add a variable
		currentVariables.resize(currentVariables.size() + 1);
		currentVariables.back().name = "";
		currentVariables.back().constant = true;
		currentVariables.back().active = false;
		currentVariables.back().overwritten = false;
		currentVariables.back( ).scanDimension = 1;
		currentVariables.back().ranges.push_back({0,0,1, false, true});
		// ranges are 1-indexed.
		for (int rangeInc = 1; rangeInc < variableRanges; rangeInc++)
		{
			currentVariables.back().ranges.push_back({ 0,0,0, false, true });
		}
		// make a new "new" row.
		// choose item
		listViewItem.iItem = itemIndicator;
		// initialize this row.
		listViewItem.pszText = "___";
		listViewItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem(&listViewItem);
		if (isGlobal)
		{
			listViewItem.iSubItem = 1;
			listViewItem.pszText = "0";
			variablesListview.SetItem( &listViewItem );
		}
		else
		{
			listViewItem.iSubItem = 1;
			listViewItem.pszText = "Constant";
			variablesListview.SetItem( &listViewItem );
			for (int rangeInc = 0; rangeInc < variableRanges; rangeInc++)
			{
				listViewItem.iSubItem = preRangeColumns + 3 * rangeInc;
				if (rangeInc == 0)
				{
					listViewItem.pszText = "0";
				}
				else
				{
					listViewItem.pszText = "---";
				}
				variablesListview.SetItem( &listViewItem );
				listViewItem.iSubItem = preRangeColumns+1 + 3 * rangeInc;
				listViewItem.pszText = "---";
				variablesListview.SetItem( &listViewItem );
				listViewItem.iSubItem = preRangeColumns+2 + 3 * rangeInc;
				listViewItem.pszText = "---";
				variablesListview.SetItem( &listViewItem );
			}
		}
		std::string s( "A" );
		listViewItem.pszText = &s[0];
		listViewItem.iSubItem = 2;
		variablesListview.SetItem( &listViewItem );
	}
	/// Handle different subitem clicks
	switch (subitem)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName;
			TextPromptDialog dialog(&newName, "Please enter a name for the variable:");
			dialog.DoModal();
			// make name lower case
			std::transform( newName.begin(), newName.end(), newName.begin(), ::tolower );
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			for (auto variable : currentVariables)
			{
				if (variable.name == newName)
				{
					thrower( "ERROR: A varaible with name " + newName + " already exists!" );
				}
			}
			if ( ttls->isValidTTLName( newName ) )
			{
				thrower( "ERROR: the name " + newName + " is already a ttl Name!" );
			}
			if ( dacs->isValidDACName( newName ) )
			{
				thrower( "ERROR: the name " + newName + " is already a dac name!" );
			}
			// update the info inside
			currentVariables[varNumber].name = newName;
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitem;
			listViewItem.pszText = &newName[0];
			variablesListview.SetItem(&listViewItem);
			// recolor to catch any new variable names needing to change color.
			for (UINT scriptInc = 0; scriptInc < scripts.size(); scriptInc++)
			{
				//scripts[scriptInc]->colorEntireScript();
			}
			break;
		}
		case 1:
		{
			if (isGlobal)
			{
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitem;
				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter a value for the global variable "
										+ currentVariables[varNumber].name + ". Value will be formatted as a double.");
				dialog.DoModal();

				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					currentVariables[varNumber].ranges.front().initialValue =
						currentVariables[varNumber].ranges.front().finalValue = std::stod( newValue );
				}
				catch (std::invalid_argument&)
				{
					thrower( "ERROR: the value entered, " + newValue + ", failed to convert to a double! "
							"Check for invalid characters." );
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitem;
				std::string temp(str(currentVariables[varNumber].ranges.front().initialValue, 13, true));
				listViewItem.pszText = &temp[0];
				variablesListview.SetItem( &listViewItem );
				break;
			}
			else
			{
				/// constant or variable?
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitem;
				// this is just a binary switch.
				if (currentVariables[varNumber].constant)
				{
					// switch to variable.
					currentVariables[varNumber].constant = false;
					listViewItem.iSubItem = subitem;
					listViewItem.pszText = "Variable";
					variablesListview.SetItem( &listViewItem );
					for (UINT rangeInc = 0; rangeInc < currentVariables[varNumber].ranges.size(); rangeInc++)
					{
						// set lower end of range
						std::string temp(str(currentVariables[varNumber].ranges[rangeInc].initialValue, 13));
						listViewItem.pszText = &temp[0];
						listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						// set higher end of range
						temp = str(currentVariables[varNumber].ranges[rangeInc].finalValue, 13);
						listViewItem.pszText = &temp[0];
						listViewItem.iSubItem = preRangeColumns+1 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						// set number of variations in this range
						temp = str(currentVariations);
						listViewItem.pszText = &temp[0];
						currentVariables[varNumber].ranges[rangeInc].variations = currentVariations;
						// TODO: Handle this better. 
						UINT totalVariations = 0;
						for (auto range : currentVariables[varNumber].ranges)
						{
							totalVariations += range.variations;
						}
						if (totalVariations == 0)
						{
							errBox("WARNING: variable has zero variations in a certain range! "
								   "There needs to be at least one.");
						}
						
						listViewItem.iSubItem = preRangeColumns+2 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
					}
				}
				else
				{
					/// switch to constant.
					currentVariables[varNumber].constant = true;
					for (int rangeInc = 0; rangeInc < variableRanges; rangeInc++)
					{
						if (rangeInc == 0)
						{
							std::string temp(str(currentVariables[varNumber].ranges[0].initialValue, 13, true));
							listViewItem.pszText = &temp[0];
							listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
							variablesListview.SetItem( &listViewItem );
						}
						else
						{
							listViewItem.pszText = "---";
							listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
							variablesListview.SetItem( &listViewItem );
						}
						// set the value to be dashes on the screen. no value for "Variable".
						listViewItem.pszText = "---";
						listViewItem.iSubItem = preRangeColumns+1 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						listViewItem.pszText = "---";
						listViewItem.iSubItem = preRangeColumns+2 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
					}
					listViewItem.iSubItem = subitem;
					listViewItem.pszText = "Constant";
					variablesListview.SetItem( &listViewItem );
				}
			}
			break;
		}
		case 2:
		{
			if ( currentVariables[varNumber].constant )
			{
				break;
			}
			// handle dimension;
			UINT maxDim = 0;
			for ( auto& variable : currentVariables )
			{
				if ( variable.name == currentVariables[varNumber].name || variable.constant )
				{
					// don't count the one being changed.
					continue;
				}
				if ( variable.scanDimension > maxDim )
				{
					maxDim = variable.scanDimension;
				}
			}
			currentVariables[varNumber].scanDimension++;
			// handle "wrapping".
			if ( currentVariables[varNumber].scanDimension > maxDim + 1 )
			{
				currentVariables[varNumber].scanDimension = 1;
			}
			reorderVariableDimensions( );
		}
		default:
		{
			if ((subitem - preRangeColumns) % 3 == 0)
			{
				UINT rangeNum = (subitem - preRangeColumns) / 3;
				// if it's a constant, you can only set the first range initial value.
				if (currentVariables[varNumber].constant && (subitem - preRangeColumns) != 0)
				{
					// then no final value to be set. In this case the extra boxes are unresponsive.
					break;
				}
				std::string newValue;
				TextPromptDialog dialog( &newValue, "Please enter an initial value for the variable " 
										 + currentVariables[varNumber].name + ". Value will be formatted as a double.");
				dialog.DoModal();

				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try 
				{
					currentVariables[varNumber].ranges[rangeNum].initialValue = std::stod(newValue);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: the value entered, " + newValue + ", failed to convert to a double! "
									"Check for invalid characters.");
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitem;
				std::string tempStr(str(currentVariables[varNumber].ranges[rangeNum].initialValue, 13));
				listViewItem.pszText = &tempStr[0];
				variablesListview.SetItem(&listViewItem);
				break;
			}
			else if ((subitem - preRangeColumns) % 3 == 1)
			{
				UINT rangeNum = (subitem - preRangeColumns) / 3;
				if (currentVariables[varNumber].constant)
				{
					// then no final value to be set. in this case the box is unresponsive.
					break;
				}
				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter a final value for the "
										"variable " + currentVariables[varNumber].name
										+ ". Value will be formatted as a double.");
				dialog.DoModal();
				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					currentVariables[varNumber].ranges[rangeNum].finalValue = std::stod(newValue);
				}
				catch (std::invalid_argument&)
				{
					errBox("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.");
					break;
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitem;
				std::string tempStr(str(currentVariables[varNumber].ranges[rangeNum].finalValue, 13));
				listViewItem.pszText = &tempStr[0];
				variablesListview.SetItem(&listViewItem);
				break;
			}
			else if((subitem - preRangeColumns) % 3 == 2)
			{
				UINT rangeNum = (subitem - preRangeColumns) / 3;
				if (currentVariables[varNumber].constant)
				{
					// then must be 1. In this case this box is unresponsive.
					break;
				}
				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter the number of variations of the variable range " 
										 + str((subitem - 3) / 3) + ". Value will be formatted as an integer.");
				dialog.DoModal();

				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					for (auto& variable : currentVariables)
					{
						if (!variable.constant)
						{
							// make sure all variables have the same number of variations.
							if ( variable.scanDimension != currentVariables[varNumber].scanDimension )
							{
								continue;
							}
							variable.ranges[rangeNum].variations = std::stoi(newValue);
							UINT totalVariations = 0;
							for (auto range : variable.ranges)
							{
								totalVariations += range.variations;
							}
							if (totalVariations == 0)
							{
								errBox("WARNING: there needs to be at least one variation for a variable.");
							}
						}
					}
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.");
				}
				// update the listview
				listViewItem.iSubItem = subitem;
				std::string tempStr(str(currentVariables[varNumber].ranges[rangeNum].variations));
				listViewItem.pszText = &tempStr[0];
				for (auto varInc : range(currentVariables.size()))
				{
					if (!currentVariables[varInc].constant 
						 && (currentVariables[varInc].scanDimension == currentVariables[varNumber].scanDimension))
					{
						listViewItem.iItem = varInc;
						variablesListview.SetItem(&listViewItem);
					}
				}
				break;
			}
		}
	}
	updateVariationNumber( );
}


void VariableSystem::setActive(bool active)
{
	if (active)
	{
		variablesListview.EnableWindow();
	}
	else
	{
		variablesListview.EnableWindow(false);
	}
}


void VariableSystem::deleteVariable()
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	variablesListview.ScreenToClient(&cursorPos);
	int subitemIndicator = variablesListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = variablesListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == currentVariables.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if (UINT(itemIndicator) < currentVariables.size())
	{
		answer = promptBox("Delete variable " + currentVariables[itemIndicator].name + "?", MB_YESNO);
		if (answer == IDYES)
		{
			variablesListview.DeleteItem(itemIndicator);
			currentVariables.erase(currentVariables.begin() + itemIndicator);
		}

	}
	else if (UINT(itemIndicator) > currentVariables.size())
	{
		answer = promptBox("You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO);
		if (answer == IDYES)
		{
			variablesListview.DeleteItem(itemIndicator);
		}
	}
	updateVariationNumber( );
}


variableType VariableSystem::getVariableInfo(int varNumber)
{
	return currentVariables[varNumber];
}


UINT VariableSystem::getCurrentNumberOfVariables()
{
	return currentVariables.size();
}

// takes as input variables, but just looks at the name and usage stats. When it finds matches between the variables,
// it takes the usage of the input and saves it as the usage of the real inputVar. 
void VariableSystem::setUsages(std::vector<variableType> vars)
{
	for (auto inputVar : vars)
	{
		for (auto& realVar : currentVariables)
		{
			if (inputVar.name == realVar.name)
			{
				realVar.overwritten = inputVar.overwritten;
				realVar.active = inputVar.active;
				break;
			}
		}
	}
	variablesListview.RedrawWindow();
}


void VariableSystem::clearVariables()
{
	// clear the internal info holder;
	currentVariables.clear();
	// clear the listview
	int itemCount = variablesListview.GetItemCount();
	// take the bottoms out of the variable listview
	for (int itemInc = 0; itemInc < itemCount; itemInc++)
	{
		variablesListview.DeleteItem(0);
	}
}


std::vector<variableType> VariableSystem::getEverything()
{
	return currentVariables;
}


std::vector<variableType> VariableSystem::getAllConstants()
{
	std::vector<variableType> constants;
	for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
	{
		if (currentVariables[varInc].constant)
		{
			constants.push_back(currentVariables[varInc]);
		}
	}
	return constants;
}

// this function returns the compliment of the variables that "getAllConstants" returns.
std::vector<variableType> VariableSystem::getAllVariables()
{
	std::vector<variableType> varyingParameters;
	for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
	{
		// opposite of get constants.
		if (!currentVariables[varInc].constant)
		{
			varyingParameters.push_back(currentVariables[varInc]);
		}
	}
	return varyingParameters;
}


void VariableSystem::addGlobalVariable( variableType variable, UINT item )
{
	// convert name to lower case.
	std::transform( variable.name.begin(), variable.name.end(), variable.name.begin(), ::tolower );
	if (isdigit(variable.name[0]))
	{
		thrower("ERROR: " + variable.name + " is an invalid name; names cannot start with numbers.");
	}

	if (variable.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower("ERROR: Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}

	if (variable.name == "")
	{
		// then add empty variable slot
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		if (item == -1)
		{
			// at end.
			listViewDefaultItem.iItem = variablesListview.GetItemCount();          // choose item  
		}
		else
		{
			listViewDefaultItem.iItem = item;          // choose item  
		}
		listViewDefaultItem.iSubItem = 0;       // Put in first collumn
		variablesListview.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		variablesListview.SetItem( &listViewDefaultItem );
		variablesListview.RedrawWindow();
		return;
	}
	/// else...
	if (variable.constant == false)
	{
		thrower( "ERROR: attempted to add a non-constant to the global variable control!" );
	}
	for (auto currentVar : currentVariables)
	{
		if (currentVar.name == variable.name)
		{
			thrower( "ERROR: A variable with the name " + variable.name + " already exists!" );
		}
	}
	// add it to the internal structure that keeps track of variables
	currentVariables.push_back( variable );
	/// add the entry to the listview.
	LVITEM listViewItem;
	memset( &listViewItem, 0, sizeof( listViewItem ) );
	// Text Style
	listViewItem.mask = LVIF_TEXT;   
	// Max size of test
	listViewItem.cchTextMax = 256; 
	// choose item  
	// initialize this row.
	listViewItem.iItem = item;
	std::string tempStr(str(variable.name));
	listViewItem.pszText = &tempStr[0];
	// Put in first column
	listViewItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	tempStr = str(variable.ranges.front().initialValue, 13, true);
	listViewItem.pszText = &tempStr[0];
	variablesListview.SetItem(&listViewItem);
}


void VariableSystem::addConfigVariable(variableType variableToAdd, UINT item)
{
	// make name lower case.
	std::transform(variableToAdd.name.begin(), variableToAdd.name.end(), variableToAdd.name.begin(), ::tolower);
	if (isdigit(variableToAdd.name[0]))
	{
		thrower("ERROR: " + variableToAdd.name + " is an invalid name; names cannot start with numbers.");
	}
	// check for forbidden (math) characters
	if (variableToAdd.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower("ERROR: Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}
	if (variableToAdd.name == "")
	{
		// then add empty variable slot
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		if (item == -1)
		{
			// at end.
			listViewDefaultItem.iItem = variablesListview.GetItemCount();
		}
		else
		{
			// choose item  
			listViewDefaultItem.iItem = item;
		}
		// Put in first collumn
		listViewDefaultItem.iSubItem = 0;
		// initialize as a constant.
		variablesListview.InsertItem(&listViewDefaultItem);
		for (int itemInc = 1; itemInc < 6; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			variablesListview.SetItem(&listViewDefaultItem);
		}
		variablesListview.RedrawWindow();
		return;
	}

	/// else...
	// check if varible already exists.
	for (auto currentVar : currentVariables)
	{
		if (currentVar.name == variableToAdd.name)
		{
			thrower("ERROR: A variable with the name " + variableToAdd.name + " already exists!");
		}
	}
	// add it to the internal structure that keeps track of variables
	currentVariables.push_back(variableToAdd);
	// add the entry to the listview.
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	// Text Style
	listViewItem.mask = LVIF_TEXT;
	// Max size of test
	listViewItem.cchTextMax = 256;
	// choose item  
	// initialize this row.
	listViewItem.iItem = item;
	std::string tempStr(str(variableToAdd.name));
	listViewItem.pszText = &tempStr[0];
	// Put in first column
	listViewItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	if (variableToAdd.constant)
	{
		listViewItem.pszText = "Constant";
	}
	else
	{
		listViewItem.pszText = "Variable";
	}
	variablesListview.SetItem(&listViewItem);
	listViewItem.iSubItem = 2;
	std::string s( str( char('A' + variableToAdd.scanDimension-1) ) );
	listViewItem.pszText = &s[0];
	variablesListview.SetItem( &listViewItem );
	// make sure there are enough currentRanges.
	UINT columns = variablesListview.GetHeaderCtrl()->GetItemCount();
	UINT currentRanges = (columns - preRangeColumns - 2) / 3;
	for (auto rangeAddInc : range(variableToAdd.ranges.size() - currentRanges))
	{
		// add a range.
		LV_COLUMN listViewDefaultCollumn;
		// Zero Members
		memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
		// Type of mask
		listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		// width between each coloum
		std::string tempStr(str(variableRanges + 1) + ":(");
		listViewDefaultCollumn.pszText = &tempStr[0];
		listViewDefaultCollumn.cx = 0x20;
		variablesListview.InsertColumn( preRangeColumns + 3 * variableRanges, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "]";
		variablesListview.InsertColumn( preRangeColumns + 1 + 3 * variableRanges, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "#";
		variablesListview.InsertColumn( preRangeColumns + 2 + 3 * variableRanges, &listViewDefaultCollumn);
		// edit all variables
		LVITEM listViewItem;
		memset(&listViewItem, 0, sizeof(listViewItem));
		// Text Style
		listViewItem.mask = LVIF_TEXT;   
		// Max size of test
		listViewItem.cchTextMax = 256; 
		// make sure all variables have some text for the new columns.
		for (auto varInc : range(currentVariables.size()))
		{
			currentVariables[varInc].ranges.push_back({0,0,0, false, true});
			if (currentVariables[varInc].constant)
			{
				listViewItem.pszText = "---";
			}
			else
			{
				listViewItem.pszText = "0";
			}
			listViewItem.iItem = varInc;
			listViewItem.iSubItem = preRangeColumns + 3 * variableRanges;
			variablesListview.SetItem(&listViewItem);
			listViewItem.iSubItem = preRangeColumns + 1 + 3 * variableRanges;
			variablesListview.SetItem(&listViewItem);
			listViewItem.iSubItem = preRangeColumns + 2 + 3 * variableRanges;
			variablesListview.SetItem(&listViewItem);
		}
		variableRanges++;
	}


	for (auto rangeInc : range(variableToAdd.ranges.size()))
	{
		if (!variableToAdd.constant)
		{
			// variable case.
			std::string tempStr(str(currentVariables[item].ranges[rangeInc].initialValue, 13, true));
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = preRangeColumns + rangeInc * 3;

			variablesListview.SetItem(&listViewItem);
			tempStr = str(currentVariables[item].ranges[rangeInc].finalValue, 13, true);
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = preRangeColumns + 1 + rangeInc * 3;

			variablesListview.SetItem(&listViewItem);
			tempStr = str(currentVariables[item].ranges[rangeInc].variations, 13, true);
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = preRangeColumns + 2 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
		}
		else
		{
			// constant case.
			if (rangeInc == 0)
			{
				// get the first value, this is the value of the constant.
				std::string tempStr(str(currentVariables[item].ranges[rangeInc].initialValue, 13, true));
				listViewItem.pszText = &tempStr[0];
				listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
				variablesListview.SetItem(&listViewItem);
			}
			else
			{
				listViewItem.pszText = "---";
				listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
				variablesListview.SetItem(&listViewItem);
			}
			listViewItem.pszText = "---";
			listViewItem.iSubItem = preRangeColumns + 1 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
			listViewItem.pszText = "---";
			listViewItem.iSubItem = preRangeColumns + 2 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
		}
	}
	variablesListview.RedrawWindow();
	updateVariationNumber( );
}


void VariableSystem::reorderVariableDimensions( )
{
	/// find the maximum dimension
	UINT maxDim = 0;
	for ( auto& variable : currentVariables )
	{
		if ( variable.scanDimension > maxDim )
		{
			maxDim = variable.scanDimension;
		}
	}
	/// flatten the dimension numbers.
	UINT flattenNumber = 0;
	for ( auto dimInc : range( maxDim ) )
	{
		bool found = false;
		for ( auto& variable : currentVariables )
		{
			if ( variable.scanDimension == dimInc+1 )
			{
				variable.scanDimension = dimInc + 1 - flattenNumber;
				found = true;
			}
		}
		if ( !found )
		{
			flattenNumber++;
		}
	}
	/// reset variables
	std::vector<variableType> varCopy = currentVariables;
	clearVariables( );
	UINT variableInc=0;
	for ( auto& variable : varCopy )
	{
		addConfigVariable( variable, variableInc++ );
	}
	addConfigVariable( {}, -1 );
}


INT_PTR VariableSystem::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, 
											brushMap brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == variablesHeader.GetDlgCtrlID())
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(30, 30, 30));
		return (LRESULT)brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


std::vector<double> VariableSystem::getKeyValues( std::vector<variableType> variables )
{
	for ( auto variable : variables )
	{
		if ( variable.valuesVary )
		{
			return variable.keyValues;
		}
	}
	// no varying variables found.
	return std::vector<double>( 1, 0 );
}


void VariableSystem::generateKey( std::vector<variableType>& variables, bool randomizeVariablesOption )
{
	// get information from variables.
	for ( auto& variable : variables )
	{
		variable.keyValues.clear( );
	}
	// get maximum dimension.
	UINT maxDim = 0;
	for ( auto variable : variables )
	{
		if ( variable.scanDimension > maxDim )
		{
			maxDim = variable.scanDimension;
		}
	}
	// each element of the vector refers to the number of variations within a given variation range.
	// variations[dimNumber][rangeNumber]
	std::vector<std::vector<int>> variations(maxDim);
	std::vector<int> variableIndexes;
	for ( UINT dimInc : range( maxDim ) )
	{
		variations[dimInc].resize( variables.front( ).ranges.size( ) );
		for ( UINT varInc = 0; varInc < variables.size( ); varInc++ )
		{
			if ( variables[varInc].scanDimension != dimInc + 1 )
			{
				continue;
			}
			// find a varying parameter.
			if ( variables[varInc].constant )
			{
				continue;
			}
			// then this variable varies in this dimension. 
			variableIndexes.push_back( varInc );
			// variations.size is the number of ranges currently.

			if ( variations[dimInc].size( ) != variables[varInc].ranges.size( ) )
			{
				// if its zero its just the initial size on the initial variable.
				if ( variations.size( ) != 0 )
				{
					thrower( "ERROR: Not all variables seem to have the same number of ranges for their parameters!" );
				}
				variations[dimInc].resize( variables[varInc].ranges.size( ) );
			}
			// make sure the variations number is consistent between
			for ( auto rangeInc : range( variations[dimInc].size( ) ) )
			{
				if ( variables[varInc].scanDimension != dimInc+1 )
				{
					continue;
				}
				// avoid the case of zero as this just hasn't been set yet.
				if ( variations[dimInc][rangeInc] != 0 )
				{
					if ( variables[varInc].ranges[rangeInc].variations != variations[dimInc][rangeInc] )
					{
						thrower( "ERROR: not all ranges of variables have the same number of variations!" );
					}
				}
				variations[dimInc][rangeInc] = variables[varInc].ranges[rangeInc].variations;
			}
		}
	}

	std::vector<UINT> totalVariations( maxDim );
	for ( auto dimInc : range(variations.size()) )
	{
		totalVariations[dimInc] = 0;
		for ( auto variationsInRange : variations[dimInc] )
		{
			totalVariations[dimInc] += variationsInRange;
		}
	}

	// create a key which will be randomized and then used to randomize other things the same way.
	multiDimensionalKey<int> randomizerMultiKey( maxDim );
	randomizerMultiKey.resize( totalVariations );

	UINT count = 0;
	for ( auto& keyElem : randomizerMultiKey.values )
	{
		keyElem = count++;
	}
	if ( randomizeVariablesOption )
	{
		// initialize rng
		std::random_device rng;
		std::mt19937 urng( rng( ) );
		// and shuffle.
		std::shuffle( randomizerMultiKey.values.begin( ), randomizerMultiKey.values.end( ), urng );
		// we now have a random key for the shuffling which every variable will follow
		// initialize this to one so that constants always get at least one value.
	}

	int totalSize = 1;
	
	for ( auto variableInc : range(variableIndexes.size( )) )
	{
		int varIndex = variableIndexes[variableInc];
		// calculate all values for a given variable
		multiDimensionalKey<double> tempKey(maxDim), tempKeyRandomized( maxDim );
		tempKey.resize( totalVariations );
		tempKeyRandomized.resize( totalVariations );
		std::vector<UINT> rangeOffset (totalVariations.size(), 0);
		std::vector<UINT> indexes( maxDim );
		while ( true )
		{
			UINT rangeIndex = 0, variationInc = 0;
			UINT varDim = variables[varIndex].scanDimension - 1;
			UINT relevantIndex = indexes[varDim];
			UINT tempShrinkingIndex = relevantIndex;
			UINT rangeCount = 0, rangeOffset = 0;
			// calculate which range it is and what the index offset should be as a result.
			for ( auto range : variables[varIndex].ranges )
			{
				if ( tempShrinkingIndex >= range.variations )
				{
					tempShrinkingIndex -= range.variations;
					rangeOffset += range.variations;
				}
				else
				{
					rangeIndex = rangeCount;
					break;
				}
				rangeCount++;
			}
			// calculate the parameters for the variation range
			double valueRange = ( variables[varIndex].ranges[rangeIndex].finalValue
								  - variables[varIndex].ranges[rangeIndex].initialValue );
			int spacings;
			if ( variables[varIndex].ranges[rangeIndex].leftInclusive
					&& variables[varIndex].ranges[rangeIndex].rightInclusive )
			{
				spacings = variations[variables[varIndex].scanDimension - 1][rangeIndex] - 1;
			}
			else if ( variables[varIndex].ranges[rangeIndex].leftInclusive
						&& variables[varIndex].ranges[rangeIndex].rightInclusive )
			{
				spacings = variations[varDim][rangeIndex] + 1;
			}
			else
			{
				spacings = variations[varDim][rangeIndex];
			}
			double initVal;
			if ( variables[varIndex].ranges[rangeIndex].leftInclusive )
			{
				initVal = variables[varIndex].ranges[rangeIndex].initialValue;
			}
			else
			{
				initVal = variables[varIndex].ranges[rangeIndex].initialValue + valueRange / spacings;
			}
			// calculate values.
			variationInc = indexes[varDim];
			double value = valueRange * (variationInc - rangeOffset) / spacings + initVal;
			tempKey.setValue( indexes, value );
			// increment. This part effectively makes this infinite while an arbitrary-dimensional loop.
			bool isAtEnd = true;
			for ( auto& indexInc : range(indexes.size()) )
			{
				// if at end of cycle for this index in this range
				if ( indexes[indexInc] == totalVariations[indexInc]-1 )
				{
					indexes[indexInc] = 0;
					continue;
				}
				else
				{
					indexes[indexInc]++;
					isAtEnd = false;
					break;
				}
			}
			if ( isAtEnd )
			{
				break;
			}
		}


		for ( auto keyInc : range(randomizerMultiKey.values.size()) )
		{
			tempKeyRandomized.values[keyInc] = tempKey.values[randomizerMultiKey.values[keyInc]];
		}
		variables[varIndex].keyValues = tempKeyRandomized.values;
		variables[varIndex].valuesVary = true;
		totalSize = tempKeyRandomized.values.size( );
	}
	// now add all constant objects.
	for ( variableType& variable : variables )
	{
		if ( variable.constant )
		{
			variable.keyValues.clear( );
			variable.keyValues.resize( totalSize );
			for ( auto& val : variable.keyValues )
			{
				// the only constant value is stored as the initial value here.
				val = variable.ranges[0].initialValue;
			}
			variable.valuesVary = false;
		}
	}
}



