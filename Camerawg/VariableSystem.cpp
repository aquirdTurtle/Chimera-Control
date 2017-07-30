#include "stdafx.h"
#include "VariableSystem.h"
#include "externals.h"
#include "fonts.h"
#include "textPromptDialogProcedure.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "ScriptingWindow.h"
#include <memory>

void VariableSystem::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange("", "", width, height, fonts);
	listview.rearrange("", "", width, height, fonts);
}

bool VariableSystem::updateVariableInfo(MainWindow* mainWin, ScriptingWindow* scriptWin)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	listview.ScreenToClient(&cursorPos);
	int subitemIndicator;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	listview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return false;
	}
	// update the configuration saved status. variables are stored in the configuration-level file.
	mainWin->updateConfigurationSavedStatus(false);
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	// choose item
	/// check if adding new variable
	listViewItem.iItem = itemIndicator;
	if (itemIndicator == currentVariables.size())
	{
		// add a person
		currentVariables.resize(currentVariables.size() + 1);
		currentVariables.back().name = "";
		currentVariables.back().timelike = false;
		currentVariables.back().singleton = true;
		currentVariables.back().value = 0;
		// add an item to the control
		// choose ite m
		listViewItem.iItem = itemIndicator;
		// initialize this row.
		listViewItem.pszText = "___";
		listViewItem.iSubItem = 0;       // Put in first coluom
		listview.InsertItem(&listViewItem);
		listViewItem.iSubItem = 1;
		listViewItem.pszText = "Singleton";
		listview.SetItem(&listViewItem);
		listViewItem.iSubItem = 2;
		listViewItem.pszText = "0";
		listview.SetItem(&listViewItem);
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "No";
		listview.SetItem(&listViewItem);
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName;// = (const char*)DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM));
			TextPromptDialog dialog(&newName, "Please enter a name for the variable:");
			dialog.DoModal();
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			// make it lowercase.
			std::transform(newName.begin(), newName.end(), newName.begin(), ::tolower);
			// update the info inside
			currentVariables[itemIndicator].name = newName;
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newName.c_str();
			listview.SetItem(&listViewItem);
			// recolor to catch any new variable names needing to change color.
			scriptWin->recolorScripts();
			break;
		}
		case 1:
		{
			/// singleton or from master?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (currentVariables[itemIndicator].singleton)
			{
				currentVariables[itemIndicator].singleton = false;
				// set the value to be dashes on the screen. no value for "from master".
				listViewItem.pszText = "---";
				listViewItem.iSubItem = 2;
				listview.SetItem(&listViewItem);
				listViewItem.iSubItem = subitemIndicator;
				listViewItem.pszText = "From Master";				
			}
			else
			{
				currentVariables[itemIndicator].singleton = true;
				listViewItem.pszText = "Singleton";
			}
			listview.SetItem(&listViewItem);

			break;
		}
		case 2:
		{
			if (!currentVariables[itemIndicator].singleton)
			{
				// then the value is gotten from the master computer. No value to set here.
				break;
			}
			// else singleton.
			std::string newValue;
			TextPromptDialog dialog(&newValue, "Please enter a value for this variable. Value will be formatted as a double.");
			dialog.DoModal();
			if (newValue == "")
			{
				// probably canceled.
				break;
			}
			// else there's something there.
			try 
			{
				currentVariables[itemIndicator].value = std::stod(newValue);
			}
			catch (std::invalid_argument)
			{
				MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check for invalid characters.").c_str(), 0, 0);
				break;
			}
			// update the listview
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			std::ostringstream out;
			out << std::setprecision(12) << currentVariables[itemIndicator].value;
			std::string tempstr = out.str();
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			listview.SetItem(&listViewItem);
			break;
		}
		case 3:
		{
			/// timelike?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (currentVariables[itemIndicator].timelike)
			{
				currentVariables[itemIndicator].timelike = false;
				listViewItem.pszText = "No";
			}
			else
			{
				currentVariables[itemIndicator].timelike = true;
				listViewItem.pszText = "Yes";
			}
			listview.SetItem(&listViewItem);
			break;
		}
	}
	return false;
}

bool VariableSystem::deleteVariable()
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	listview.ScreenToClient(&cursorPos);
	int subitemIndicator =listview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = listview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == currentVariables.size())
	{
		// user didn't click in a deletable item.
		return false;
	}
	int answer = MessageBox(0, ("Delete variable " + currentVariables[itemIndicator].name + "?").c_str(), 0, MB_YESNO);
	if (answer == IDYES)
	{
		listview.DeleteItem(itemIndicator);
		currentVariables.erase(currentVariables.begin() + itemIndicator);
	}
	return false;
}


variable VariableSystem::getVariableInfo(int varNumber)
{
	return currentVariables[varNumber];
}


unsigned long long VariableSystem::getCurrentNumberOfVariables()
{
	return currentVariables.size();
}


bool VariableSystem::clearVariables()
{
	// clear the internal info holder;
	currentVariables.clear();
	// clear the listview
	listview.DeleteAllItems();
	return false;
}

std::vector<variable> VariableSystem::getAllSingletons()
{
	std::vector<variable> singletons;
	for (int varInc = 0; varInc < currentVariables.size(); varInc++)
	{
		if (currentVariables[varInc].singleton)
		{
			singletons.push_back(currentVariables[varInc]);
		}
	}
	return singletons;
}

// this function returns the compliment of the variables that "getAllSingletons" returns.
std::vector<variable> VariableSystem::getAllVaryingParameters()
{
	std::vector<variable> varyingParameters;
	for (int varInc = 0; varInc < currentVariables.size(); varInc++)
	{
		// opposite of get singletons.
		if (!currentVariables[varInc].singleton)
		{
			varyingParameters.push_back(currentVariables[varInc]);
		}
	}
	return varyingParameters;
}

bool VariableSystem::addVariable(std::string name, bool timelike, bool singleton, double value, int item)
{
	if (name == "")
	{
		// then add empty variable slot
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = item;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		listview.InsertItem(&listViewDefaultItem);
		for (int itemInc = 1; itemInc < 3; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			listview.SetItem(&listViewDefaultItem);
		}
		return false;
	}
	/// add it to the internal structure that keeps track of variables
	variable tempVar;
	tempVar.name = name;
	tempVar.singleton = singleton;
	tempVar.timelike = timelike;
	tempVar.value = value;
	currentVariables.push_back(tempVar);
	/// add the entry to the listview.
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	// choose item  
	// initialize this row.
	listViewItem.iItem = item;          
	listViewItem.pszText = (LPSTR)name.c_str();
	// Put in first column
	listViewItem.iSubItem = 0;       
	listview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	if (singleton)
	{
		listViewItem.pszText = "Singleton";
	}
	else
	{
		listViewItem.pszText = "From Master";
	}
	// Enter text to SubItes
	listview.SetItem(&listViewItem);
	listViewItem.iSubItem = 2;
	std::string tempstr;
	if (singleton)
	{
		std::ostringstream out;
		out << std::setprecision(12) << value;
		tempstr = out.str();
		listViewItem.pszText = (LPSTR)tempstr.c_str();
	}
	else
	{
		listViewItem.pszText = "---";
	}
	// Enter text to SubItems
	listview.SetItem(&listViewItem);

	if (timelike)
	{
		listViewItem.pszText = "Yes";
	}
	else
	{
		listViewItem.pszText = "No";
	}

	// Enter text to SubItems
	listViewItem.iSubItem = 3;
	listview.SetItem(&listViewItem);
	return false;
}

bool VariableSystem::initializeControls(POINT &topLeftCorner, CWnd* parent, int& id, fontMap fonts, 
										std::vector<CToolTipCtrl*>& tooltips)
{
	header.ID = id++;
	header.sPos = { topLeftCorner.x, topLeftCorner.y, topLeftCorner.x + 480, topLeftCorner.y + 20 };
	header.Create("VARIABLES", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, header.sPos, parent, header.ID);
	header.SetFont(fonts["Heading Font"]);
	topLeftCorner.y += 20;
	listview.ID = id++;
	idVerify(listview.ID, IDC_VARIABLES_LISTVIEW);
	listview.sPos = { topLeftCorner.x, topLeftCorner.y, topLeftCorner.x + 480, topLeftCorner.y + 195 };
	listview.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS, listview.sPos, parent, listview.ID);
	listview.SetFont(fonts["Normal Font"]);
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// width between each coloum
	listViewDefaultCollumn.pszText = "Symbol";
	listViewDefaultCollumn.cx = 0x62;
	// Inserting Couloms as much as we want
	listview.InsertColumn(0, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Type";
	listview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Value";
	listview.InsertColumn(2, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Timelike?";
	listview.InsertColumn(3, &listViewDefaultCollumn);
	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
	listViewDefaultItem.cchTextMax = 256; // Max size of test
	listViewDefaultItem.pszText = "___";
	listViewDefaultItem.iItem = 0;          // choose item  
	listViewDefaultItem.iSubItem = 0;       // Put in first coluom
	listview.InsertItem(&listViewDefaultItem);
	for (int itemInc = 1; itemInc <= 3; itemInc++) // Add SubItems in a loop
	{
		listViewDefaultItem.iSubItem = itemInc;
		// Enter text to SubItems
		listview.SetItem(&listViewDefaultItem);
	}
	topLeftCorner.y += 195;
	return false;
}

std::vector<variable> VariableSystem::getAllVariables()
{
	return this->currentVariables;
}