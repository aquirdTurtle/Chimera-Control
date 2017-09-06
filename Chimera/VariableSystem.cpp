#include "stdafx.h"
#include <iomanip>
#include <unordered_map>
#include "afxcmn.h"

#include "VariableSystem.h"
#include "Script.h"
#include "DioSystem.h"
#include "AuxiliaryWindow.h"
#include "TextPromptDialog.h"

UINT VariableSystem::getTotalVariationNumber()
{
	return currentVariations;
}

void VariableSystem::handleOpenConfig(std::ifstream& configFile, double version)
{
	ProfileSystem::checkDelimiterLine(configFile, "VARIABLES");
	// handle variables
	clearVariables();
	// Number of Variables
	UINT varNum;
	configFile >> varNum;
	if (varNum > 10)
	{
		int answer = promptBox("ERROR: variable number retrieved from file appears suspicious. The number is "
								+ str(varNum) + ". Is this accurate?", MB_YESNO);
		if (answer == IDNO)
		{
			// don't try to load anything.
			varNum = 0;
		}
	}
	// if no variables to get incremented over here, it will stay at 1.
	currentVariations = 1;
	for (UINT varInc = 0; varInc < varNum; varInc++)
	{
		variable tempVar;
		std::string varName, typeText, valueString;
		bool constant;
		configFile >> varName;
		std::transform(varName.begin(), varName.end(), varName.begin(), ::tolower);
		tempVar.name = varName;
		configFile >> typeText;
		if (typeText == "Constant")
		{
			constant = true;
			tempVar.constant = true;
		}
		else if (typeText == "Variable")
		{
			constant = false;
			tempVar.constant = false;
		}
		else
		{
			thrower("ERROR: unknown variable type option: " + typeText + ". Check the formatting of the configuration file.");
		}
		int rangeNumber;
		configFile >> rangeNumber;
		// I think it's unlikely to ever need more than 2 or 3 ranges.
		if (rangeNumber < 1 || rangeNumber > 1000)
		{
			errBox("ERROR: Bad range number! setting it to 1, but found " + str(rangeNumber) + " in the file.");
			rangeNumber = 1;
		}
		setVariationRangeNumber(rangeNumber, 0);
		// check if the range is actually too small.
		UINT totalVariations = 0;
		for (int rangeInc = 0; rangeInc < rangeNumber; rangeInc++)
		{
			double initValue=0, finValue=0;
			unsigned int variations=0;
			bool leftInclusive=0, rightInclusive=0;
			configFile >> initValue;
			configFile >> finValue;
			configFile >> variations;
			totalVariations += variations;
			configFile >> leftInclusive;
			configFile >> rightInclusive;
			tempVar.ranges.push_back({ initValue, finValue, variations, leftInclusive, rightInclusive });
		}
		if (!constant)
		{
			currentVariations = totalVariations;
		}
		// shouldn't be because of 1 forcing earlier.
		if (tempVar.ranges.size() == 0)
		{
			// make sure it has at least one entry.
			tempVar.ranges.push_back({ 0,0,1, false, true });
		}
		addConfigVariable(tempVar, varInc);
	}

	// add a blank line
	variable var;
	var.name = "";
	var.constant = false;
	var.ranges.push_back({ 0, 0, 1, false, true });
	addConfigVariable(var, varNum);

	ProfileSystem::checkDelimiterLine(configFile, "END_VARIABLES");
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
		variable info = getVariableInfo(varInc);
		saveFile << info.name << " ";
		if (info.constant)
		{
			saveFile << "Constant ";
		}
		else
		{
			saveFile << "Variable ";
		}
		saveFile << info.ranges.size() << "\n";
		for (UINT rangeInc = 0; rangeInc < info.ranges.size(); rangeInc++)
		{
			saveFile << info.ranges[rangeInc].initialValue << "\n";
			saveFile << info.ranges[rangeInc].finalValue << "\n";
			saveFile << info.ranges[rangeInc].variations << "\n";
			saveFile << info.ranges[rangeInc].leftInclusive << "\n";
			saveFile << info.ranges[rangeInc].rightInclusive << "\n";
		}
		saveFile << "\n";
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
	// then add empty variable slot
	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	// Text Style
	listViewDefaultItem.mask = LVIF_TEXT; 
	// Max size of test
	listViewDefaultItem.cchTextMax = 256; 
	listViewDefaultItem.pszText = "Symbol";
	// at end of items.
	listViewDefaultItem.iItem = variablesListview.GetItemCount();
	// initialize each column.
	listViewDefaultItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewDefaultItem);
	listViewDefaultItem.iSubItem = 1;
	listViewDefaultItem.pszText = "Type";
	variablesListview.SetItem(&listViewDefaultItem);
	// 
	listViewDefaultItem.iSubItem = 2;
	listViewDefaultItem.pszText = "1. (";
	variablesListview.SetItem(&listViewDefaultItem);

	listViewDefaultItem.iSubItem = 3;
	listViewDefaultItem.pszText = "]";
	variablesListview.SetItem(&listViewDefaultItem);
	
	listViewDefaultItem.iSubItem = 4;
	listViewDefaultItem.pszText = "#";
	variablesListview.SetItem(&listViewDefaultItem);

	listViewDefaultItem.iSubItem = 5;
	listViewDefaultItem.pszText = "+(]";
	variablesListview.SetItem(&listViewDefaultItem);
	listViewDefaultItem.iSubItem = 6;
	listViewDefaultItem.pszText = "-(]";
	variablesListview.SetItem(&listViewDefaultItem);
	listViewDefaultItem.iSubItem = variablesListview.GetHeaderCtrl()->GetItemCount() - 1;
	std::string tempStr("Dim " + str(scanDimensions+1));
	listViewDefaultItem.pszText = &tempStr[0];
	variablesListview.SetItem(&listViewDefaultItem);
	/// add the blank line.
	listViewDefaultItem.pszText = "___";
	// at end of items.
	listViewDefaultItem.iItem = variablesListview.GetItemCount();
	// initialize each column.
	listViewDefaultItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewDefaultItem);
	for (int colInc = 1; colInc < variablesListview.GetHeaderCtrl()->GetItemCount(); colInc++)
	{
		listViewDefaultItem.iSubItem = colInc;
		variablesListview.SetItem(&listViewDefaultItem);
	}
	variablesListview.RedrawWindow();
	scanDimensions++;
}


void VariableSystem::removeVariableDimension()
{
	if (scanDimensions == 1)
	{
		thrower("ERROR: Can't delete last variable scan dimension.");
	}
	// change all variables in the last dimension to be in the second-to-last dimension.
	// TODO: I'm gonna have to check variation numbers here or change them to be compatible.
	for (auto& var : currentVariables)
	{
		if (var.scanDimension == scanDimensions)
		{
			var.scanDimension--;
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
	// if you add more currentRanges to the control, you need to change the 5 here to reflect the number of auxiliary 
	// columns.
	while (variableRanges.size() < USHORT(dimNumber + 1))
	{
		variableRanges.push_back(1);
	}
	int currentRanges = (variablesListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
	if (variableRanges[dimNumber] != currentRanges)
	{
		errBox("ERROR: somehow, the number of ranges the VariableSystem object thinks there are and the actual number "
				"are off! The numbes are " + str(variableRanges[dimNumber]) + " and "
				+ str(currentRanges) + " respectively. The program will attempt to fix this, but " 
				"data may be lost.");
		variableRanges[dimNumber] = currentRanges;
		for (UINT variableInc = 0; variableInc < currentVariables.size(); variableInc++)
		{
			currentVariables[variableInc].ranges.resize(currentRanges);
		}
	}
	if (currentRanges < num)
	{
		while (currentRanges < num)
		{
			/// add a range.
			LV_COLUMN listViewDefaultCollumn;
			// Zero Members
			memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
			// Type of mask
			listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			// width between each coloum
			std::string temp(str(str(variableRanges[dimNumber] + 1) + ":("));
			listViewDefaultCollumn.pszText = &temp[0];
			listViewDefaultCollumn.cx = 0x20;
			variablesListview.InsertColumn(3 + 3 * currentRanges, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "]";
			variablesListview.InsertColumn(4 + 3 * currentRanges, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "#";
			variablesListview.InsertColumn(5 + 3 * currentRanges, &listViewDefaultCollumn);
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
				listViewItem.iSubItem = 3 + 3 * currentRanges;
				variablesListview.SetItem(&listViewItem);
				listViewItem.iSubItem = 4 + 3 * currentRanges;
				variablesListview.SetItem(&listViewItem);
				listViewItem.iSubItem = 5 + 3 * currentRanges;
				variablesListview.SetItem(&listViewItem);
			}
			int newRangeNum = (variablesListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			// make sure this makes sense.
			if (currentRanges != newRangeNum - 1)
			{
				errBox("Error! Range numbers after new range don't make sense!");
			}
			currentRanges = newRangeNum;
			variableRanges[dimNumber] = currentRanges;
		}
	}
	else if (currentRanges > num)
	{
		while (currentRanges > num)
		{
			// delete a range.
			if (variableRanges[dimNumber] == 1)
			{
				// can't delete last set...
				return;
			}

			variablesListview.DeleteColumn(3 + 3 * (currentRanges - 1));
			variablesListview.DeleteColumn(3 + 3 * (currentRanges - 1));
			variablesListview.DeleteColumn(3 + 3 * (currentRanges - 1));
			// edit all variables
			for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
			{
				currentVariables[varInc].ranges.pop_back();
			}
			int newRangeNum = (variablesListview.GetHeaderCtrl()->GetItemCount() - 3) / 3;
			// make sure this makes sense.
			if (currentRanges != newRangeNum + 1)
			{
				errBox("Error!");
			}
			// continue...
			currentRanges = newRangeNum;
			variableRanges[dimNumber] = currentRanges;
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
	if (myItemInfo.iSubItem == 2 + 3 * variableRanges[0])
	{
		// add a range.
		setVariationRangeNumber(variableRanges[0] + 1, 0);
	}
	else if (myItemInfo.iSubItem == 3 + 3 * variableRanges[0])
	{
		// delete a range.
		setVariationRangeNumber(variableRanges[0] - 1, 0);
	}
	else if (myItemInfo.iSubItem == 4 + 3 * variableRanges[0])
	{
		// add a variable scan dimension
		addVariableDimension();
	}
	else if (myItemInfo.iSubItem == 5 + 3 * variableRanges[0])
	{
		// remove a variable scan dimension
		removeVariableDimension();
	}
	else if (myItemInfo.iSubItem > 1 && myItemInfo.iSubItem % 3 == 0)
	{
		// switch between [ and (
		for (auto& var : currentVariables)
		{
			UINT rangeNum = (myItemInfo.iSubItem - 3) / 3;
			// toggle
			if (var.ranges[rangeNum].leftInclusive)
			{
				var.ranges[rangeNum].leftInclusive = false;
				LVCOLUMNA colInfo;
				variablesListview.GetColumn(myItemInfo.iSubItem, &colInfo);
				std::string text((str(rangeNum+1) + ". ("));
				colInfo.pszText = &text[0];
				variablesListview.SetColumn(myItemInfo.iSubItem, &colInfo);
			}
			else													   
			{
				var.ranges[rangeNum].leftInclusive = true;
				LVCOLUMNA colInfo;
				variablesListview.GetColumn(myItemInfo.iSubItem, &colInfo);
				std::string text((str(rangeNum+1) + ". ["));
				colInfo.pszText = &text[0];
				variablesListview.SetColumn(myItemInfo.iSubItem, &colInfo);
			}
		}
	}
	else if (myItemInfo.iSubItem > 1 && myItemInfo.iSubItem % 3 == 1)
	{
		// switch between ] and )
		for (auto& var : currentVariables)
		{
			UINT rangeNum = (myItemInfo.iSubItem -3) / 3;
			// toggle
			if (var.ranges[rangeNum].rightInclusive)
			{
				var.ranges[rangeNum].rightInclusive = false;
				LVCOLUMNA colInfo;
				variablesListview.GetColumn(myItemInfo.iSubItem, &colInfo);
				colInfo.pszText = ")";
				variablesListview.SetColumn(myItemInfo.iSubItem, &colInfo);
			}
			else
			{
				var.ranges[rangeNum].rightInclusive = true;
				LVCOLUMNA colInfo;
				variablesListview.GetColumn(myItemInfo.iSubItem, &colInfo);
				colInfo.pszText = "]";
				variablesListview.SetColumn(myItemInfo.iSubItem, &colInfo);
			}
		}
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


void VariableSystem::updateVariableInfo(std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin)
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
	for (UINT rowCount = 0; rowCount < UINT(itemIndicator); rowCount++)
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
		currentVariables.back().ranges.push_back({0,0,1, false, true});
		// it's 1-indexed.
		
		currentVariables.back().scanDimension = scanDim;
		for (int rangeInc = 1; rangeInc < variableRanges[scanDim]; rangeInc++)
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
			for (int rangeInc = 0; rangeInc < variableRanges[0]; rangeInc++)
			{
				listViewItem.iSubItem = 2 + 3 * rangeInc;
				if (rangeInc == 0)
				{
					listViewItem.pszText = "0";
				}
				else
				{
					listViewItem.pszText = "---";
				}
				variablesListview.SetItem( &listViewItem );
				listViewItem.iSubItem = 3 + 3 * rangeInc;
				listViewItem.pszText = "---";
				variablesListview.SetItem( &listViewItem );
				listViewItem.iSubItem = 4 + 3 * rangeInc;
				listViewItem.pszText = "---";
				variablesListview.SetItem( &listViewItem );
			}
		}
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
			for (auto var : currentVariables)
			{
				if (var.name == newName)
				{
					thrower( "ERROR: A varaible with name " + newName + " already exists!" );
				}
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
				std::string temp(str(currentVariables[varNumber].ranges.front().initialValue, 12, true));
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
						std::string temp(str(currentVariables[varNumber].ranges[(subitem - 3) / 3].initialValue, 12));
						listViewItem.pszText = &temp[0];
						listViewItem.iSubItem = 2 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						// set higher end of range
						temp = str(currentVariables[varNumber].ranges[(subitem - 3) / 3].finalValue, 12);
						listViewItem.pszText = &temp[0];
						listViewItem.iSubItem = 3 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						// set number of variations in this range
						temp = str(currentVariations);
						listViewItem.pszText = &temp[0]; 
						currentVariables[varNumber].ranges[(subitem - 3) / 3].variations = currentVariations;
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
						
						listViewItem.iSubItem = 4 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
					}
				}
				else
				{
					/// switch to constant.
					currentVariables[varNumber].constant = true;
					for (int rangeInc = 0; rangeInc < variableRanges[scanDim]; rangeInc++)
					{
						if (rangeInc == 0)
						{
							std::string temp(str(currentVariables[varNumber].ranges[0].initialValue, 12, true));
							listViewItem.pszText = &temp[0];
							listViewItem.iSubItem = 2 + rangeInc * 3;
							variablesListview.SetItem( &listViewItem );
						}
						else
						{
							listViewItem.pszText = "---";
							listViewItem.iSubItem = 2 + rangeInc * 3;
							variablesListview.SetItem( &listViewItem );
						}
						// set the value to be dashes on the screen. no value for "Variable".
						listViewItem.pszText = "---";
						listViewItem.iSubItem = 3 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						listViewItem.pszText = "---";
						listViewItem.iSubItem = 4 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
					}
					listViewItem.iSubItem = subitem;
					listViewItem.pszText = "Constant";
					variablesListview.SetItem( &listViewItem );
				}
			}
			break;
		}
		default:
		{
			if ((subitem - 2) % 3 == 0)
			{
				// if it's a constant, you can only set the first range initial value.
				if (currentVariables[varNumber].constant && (subitem - 2) != 0)
				{
					// then no final value to be set.
					break;
				}

				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter an initial value for the "
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
					currentVariables[varNumber].ranges[(subitem - 3) / 3].initialValue = std::stod(newValue);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: the value entered, " + newValue + ", failed to convert to a double! "
									"Check for invalid characters.");
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitem;
				std::string tempStr(str(currentVariables[varNumber].ranges[(subitem - 3) / 3].initialValue));
				listViewItem.pszText = &tempStr[0];
				variablesListview.SetItem(&listViewItem);
				break;
			}
			else if ((subitem - 2) % 3 == 1)
			{
				if (currentVariables[varNumber].constant)
				{
					// then no final value to be set.
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
					currentVariables[varNumber].ranges[(subitem - 3) / 3].finalValue = std::stod(newValue);
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
				std::string tempStr(str(currentVariables[varNumber].ranges[(subitem - 3) / 3].finalValue));
				listViewItem.pszText = &tempStr[0];
				variablesListview.SetItem(&listViewItem);
				break;
			}
			else if((subitem - 2) % 3 == 2)
			{
				if (currentVariables[varNumber].constant)
				{
					// then must be 1.
					break;
				}

				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter the number of variations of "
										"the variable range " + str((subitem - 3) / 3)
										+ ". Value will be formatted as an integer.");
				dialog.DoModal();

				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
					{
						if (!currentVariables[varInc].constant)
						{
							// make sure all variables have the same number of variations.
							currentVariables[varInc].ranges[(subitem - 3) / 3].variations = std::stoi(newValue);
							UINT totalVariations = 0;
							for (auto range : currentVariables[varInc].ranges)
							{
								totalVariations += range.variations;
							}
							if (totalVariations == 0)
							{
								errBox("WARNING: there needs to be at least one variation for a variable.");
							}
						}
					}
					currentVariations = std::stoi(newValue);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.");
				}
				// update the listview
				
				listViewItem.iSubItem = subitem;
				std::string tempStr(str(currentVariables[varNumber].ranges[(subitem - 3) / 3].variations));
				listViewItem.pszText = &tempStr[0];
				for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
				{
					if (!currentVariables[varInc].constant)
					{
						listViewItem.iItem = varInc;
						variablesListview.SetItem(&listViewItem);
					}
				}
				break;
			}
		}
	}
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
}


variable VariableSystem::getVariableInfo(int varNumber)
{
	return currentVariables[varNumber];
}


UINT VariableSystem::getCurrentNumberOfVariables()
{
	return currentVariables.size();
}

// takes as input variables, but just looks at the name and usage stats. When it finds matches between the variables,
// it takes the usage of the input and saves it as the usage of the real inputVar. 
void VariableSystem::setUsages(std::vector<variable> vars)
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


std::vector<variable> VariableSystem::getEverything()
{
	return currentVariables;
}


std::vector<variable> VariableSystem::getAllConstants()
{
	std::vector<variable> constants;
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
std::vector<variable> VariableSystem::getAllVariables()
{
	std::vector<variable> varyingParameters;
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


void VariableSystem::addGlobalVariable( variable var, UINT item )
{
	// convert name to lower case.
	std::transform( var.name.begin(), var.name.end(), var.name.begin(), ::tolower );
	if (isdigit(var.name[0]))
	{
		thrower("ERROR: " + var.name + " is an invalid name; names cannot start with numbers.");
	}

	if (var.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower("ERROR: Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}

	if (var.name == "")
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
	if (var.constant == false)
	{
		thrower( "ERROR: attempted to add a non-constant to the global variable control!" );
	}
	for (auto currentVar : currentVariables)
	{
		if (currentVar.name == var.name)
		{
			thrower( "ERROR: A variable with the name " + var.name + " already exists!" );
		}
	}
	// add it to the internal structure that keeps track of variables
	currentVariables.push_back( var );
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
	std::string tempStr(str(var.name));
	listViewItem.pszText = &tempStr[0];
	// Put in first column
	listViewItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	tempStr = str(var.ranges.front().initialValue, 12, true);
	listViewItem.pszText = &tempStr[0];
	variablesListview.SetItem(&listViewItem);
}


void VariableSystem::addConfigVariable(variable var, UINT item)
{
	// make name lower case.
	std::transform(var.name.begin(), var.name.end(), var.name.begin(), ::tolower);
	if (isdigit(var.name[0]))
	{
		thrower("ERROR: " + var.name + " is an invalid name; names cannot start with numbers.");
	}
	// check for forbidden (math) characters
	if (var.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower("ERROR: Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}
	if (var.name == "")
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
		if (currentVar.name == var.name)
		{
			thrower("ERROR: A variable with the name " + var.name + " already exists!");
		}
	}
	// add it to the internal structure that keeps track of variables
	currentVariables.push_back(var);
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
	std::string tempStr(str(var.name));
	listViewItem.pszText = &tempStr[0];
	// Put in first column
	listViewItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	if (var.constant)
	{
		listViewItem.pszText = "Constant";
	}
	else
	{
		listViewItem.pszText = "Variable";
	}
	variablesListview.SetItem(&listViewItem);

	UINT borderCount = 0;
	for (UINT rowCount = 0; rowCount < item; rowCount++)
	{
		if (variablesListview.GetItemText(rowCount, 0) == "___" || variablesListview.GetItemText(rowCount, 0) == "Symbol")
		{
			borderCount++;
		}
	}
	UINT scanDim = (borderCount + 1) / 2;
	while(variableRanges.size() < scanDim+1)
	{
		variableRanges.push_back(1);
	}
	UINT varNumber = item - borderCount;

	// make sure there are enough currentRanges.
	UINT columns = variablesListview.GetHeaderCtrl()->GetItemCount();
	UINT currentRanges = (columns - 6) / 3;
	for (UINT rangeAddInc = 0; rangeAddInc < var.ranges.size() - currentRanges; rangeAddInc++)
	{
		// add a range.
		LV_COLUMN listViewDefaultCollumn;
		// Zero Members
		memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
		// Type of mask
		listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		// width between each coloum
		std::string tempStr(str(variableRanges[scanDim] + 1) + ":(");
		listViewDefaultCollumn.pszText = &tempStr[0];
		listViewDefaultCollumn.cx = 0x20;
		variablesListview.InsertColumn(2 + 3 * variableRanges[scanDim], &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "]";
		variablesListview.InsertColumn(3 + 3 * variableRanges[scanDim], &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "#";
		variablesListview.InsertColumn(4 + 3 * variableRanges[scanDim], &listViewDefaultCollumn);
		// edit all variables
		LVITEM listViewItem;
		memset(&listViewItem, 0, sizeof(listViewItem));
		// Text Style
		listViewItem.mask = LVIF_TEXT;   
		// Max size of test
		listViewItem.cchTextMax = 256; 
		// make sure all variables have some text for the new columns.
		for (UINT varInc = 0; varInc < currentVariables.size(); varInc++)
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
			listViewItem.iSubItem = 2 + 3 * variableRanges[scanDim];
			variablesListview.SetItem(&listViewItem);
			listViewItem.iSubItem = 3 + 3 * variableRanges[scanDim];
			variablesListview.SetItem(&listViewItem);
			listViewItem.iSubItem = 4 + 3 * variableRanges[scanDim];
			variablesListview.SetItem(&listViewItem);
		}
		variableRanges[scanDim]++;
	}


	for (UINT rangeInc = 0; rangeInc < var.ranges.size(); rangeInc++)
	{
		if (!var.constant)
		{
			// variable case.
			std::string tempStr(str(currentVariables[item].ranges[rangeInc].initialValue, 12, true));
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = 3 + rangeInc * 3;

			variablesListview.SetItem(&listViewItem);
			tempStr = str(currentVariables[item].ranges[rangeInc].finalValue, 12, true);
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = 4 + rangeInc * 3;

			variablesListview.SetItem(&listViewItem);
			tempStr = str(currentVariables[item].ranges[rangeInc].variations, 12, true);
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = 5 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
		}
		else
		{
			// constant case.
			if (rangeInc == 0)
			{
				// get the first value, this is the value of the constant.
				std::string tempStr(str(currentVariables[item].ranges[rangeInc].initialValue, 12, true));
				listViewItem.pszText = &tempStr[0];
				listViewItem.iSubItem = 3 + rangeInc * 3;
				variablesListview.SetItem(&listViewItem);
			}
			else
			{
				listViewItem.pszText = "---";
				listViewItem.iSubItem = 3 + rangeInc * 3;
				variablesListview.SetItem(&listViewItem);
			}
			listViewItem.pszText = "---";
			listViewItem.iSubItem = 4 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
			listViewItem.pszText = "---";
			listViewItem.iSubItem = 5 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
		}
	}
	variablesListview.RedrawWindow();
}


void VariableSystem::initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id, std::string title, 
								 rgbMap rgbs, UINT listviewId )
{
	if (title == "GLOBAL VARIABLES")
	{
		isGlobal = true;
	}
	else
	{
		isGlobal = false;
	}
	// initialize these numbers to reflect the initial status of the window.
	scanDimensions = 1;
	variableRanges.resize(1);
	variableRanges[0] = 1;
	// controls
	variablesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	variablesHeader.Create( cstr(title), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
							variablesHeader.sPos, master, id++ );
	variablesHeader.fontType = HeadingFont;
	pos.y += 25;

	LONG listViewSize = 0;
	if (isGlobal)
	{
		listViewSize = 600;
	}
	else
	{
		listViewSize = 125;
	}

	variablesListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + listViewSize };
	variablesListview.Create( WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER, 
							  variablesListview.sPos, master, listviewId );

	variablesListview.fontType = SmallFont;
	variablesListview.SetBkColor(RGB(15, 15, 15));
	variablesListview.SetTextBkColor(RGB(15, 15, 15));
	variablesListview.SetTextColor(RGB(255, 255, 255));
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "Symbol";
	if (isGlobal)
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
		listViewDefaultCollumn.pszText = "1:(";
		listViewDefaultCollumn.cx = 0x35;
		variablesListview.InsertColumn( 2, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "]";
		variablesListview.InsertColumn( 3, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "#";
		variablesListview.InsertColumn( 4, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "+()";
		listViewDefaultCollumn.cx = 0x30;
		variablesListview.InsertColumn( 5, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "-()";
		variablesListview.InsertColumn( 6, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "+ Dim";
		variablesListview.InsertColumn(7, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "- Dim";
		variablesListview.InsertColumn(8, &listViewDefaultCollumn);
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = 0;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem( &listViewDefaultItem );
		for (int itemInc = 1; itemInc < 6; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			variablesListview.SetItem( &listViewDefaultItem );
		}
	}
	variablesListview.SetBkColor( rgbs["Solarized Base02"] );

	pos.y += listViewSize;
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

