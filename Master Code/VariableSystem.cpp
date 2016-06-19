#include "stdafx.h"
#include "VariableSystem.h"
#include "fonts.h"
#include "textPromptDialogProcedure.h"
#include <iomanip>
#include <sstream>
#include "Script.h"
#include "constants.h"

bool VariableSystem::updateVariableInfo(LPARAM lParamOfMessage, std::vector<Script*> scripts, MasterWindow* Master)
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(variablesListview.hwnd, &cursorPos);
	NMITEMACTIVATE itemClicked = *(NMITEMACTIVATE*)lParamOfMessage;
	int subitemIndicator = itemClicked.iSubItem;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = SendMessage(variablesListview.hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM)&myItemInfo);
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return false;
	}
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
		currentVariables.back().initialValue = 0;
		currentVariables.back().finalValue = 0;
		currentVariables.back().variations = 1;
		// add an item to the control
		// choose ite m
		listViewItem.iItem = itemIndicator;
		// initialize this row.
		listViewItem.pszText = "___";
		listViewItem.iSubItem = 0;       // Put in first coluom
		SendMessage(variablesListview.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewItem);
		listViewItem.iSubItem = 1;
		listViewItem.pszText = "Singleton";
		SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); 
		listViewItem.iSubItem = 2;
		listViewItem.pszText = "No";
		SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "0";
		SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
		listViewItem.iSubItem = 4;
		listViewItem.pszText = "---";
		SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
		listViewItem.iSubItem = 4;
		listViewItem.pszText = "1";
		SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);

	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName = (const char*)DialogBoxParam(this->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter a name for the variable:");
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			// update the info inside
			currentVariables[itemIndicator].name = newName;
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newName.c_str();
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			// recolor to catch any new variable names needing to change color.
			for (int scriptInc = 0; scriptInc < scripts.size(); scriptInc++)
			{
				scripts[scriptInc]->colorEntireScript(*Master);
			}
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
				listViewItem.iSubItem = subitemIndicator;
				listViewItem.pszText = "Variable";				
			}
			else
			{
				currentVariables[itemIndicator].singleton = true;
				// set the value to be dashes on the screen. no value for "from master".
				listViewItem.pszText = "---";
				listViewItem.iSubItem = 4;
				SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
				listViewItem.pszText = "---";
				listViewItem.iSubItem = 5;
				SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
				listViewItem.iSubItem = subitemIndicator;
				listViewItem.pszText = "Singleton";
			}
			
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);

			break;
		}
		case 2:
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
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
		case 3:
		{
			std::string newValue = (const char*)DialogBoxParam(this->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)"Please enter an initial value for this variable. Value will be formatted as a double.");
			if (newValue == "")
			{
				// probably canceled.
				break;
			}
			// else there's something there.
			try 
			{
				currentVariables[itemIndicator].initialValue = std::stod(newValue);
			}
			catch (std::invalid_argument &exception)
			{
				MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check for invalid characters.").c_str(), 0, 0);
				break;
			}
			// update the listview
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			std::ostringstream out;
			out << std::setprecision(12) << currentVariables[itemIndicator].initialValue;
			std::string tempstr = out.str();
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
		case 4:
		{
			if (currentVariables[itemIndicator].singleton)
			{
				// then no final value to be set.
				break;
			}
			// else singleton.
			std::string newValue = (const char*)DialogBoxParam(this->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)"Please enter a final value for this variable. Value will be formatted as a double.");
			if (newValue == "")
			{
				// probably canceled.
				break;
			}
			// else there's something there.
			try
			{
				currentVariables[itemIndicator].finalValue = std::stod(newValue);
			}
			catch (std::invalid_argument &exception)
			{
				MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check for invalid characters.").c_str(), 0, 0);
				break;
			}
			// update the listview
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			std::ostringstream out;
			out << std::setprecision(12) << currentVariables[itemIndicator].finalValue;
			std::string tempstr = out.str();
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
		case 5:
		{
			if (currentVariables[itemIndicator].singleton)
			{
				// then must be 1.
				break;
			}
			// else singleton.
			std::string newValue = (const char*)DialogBoxParam(this->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)"Please enter the number of variations of this variable. Value will be formatted as an integer.");
			if (newValue == "")
			{
				// probably canceled.
				break;
			}
			// else there's something there.
			try
			{
				currentVariables[itemIndicator].variations = std::stoi(newValue);
			}
			catch (std::invalid_argument &exception)
			{
				MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check for invalid characters.").c_str(), 0, 0);
				break;
			}
			// update the listview
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			std::ostringstream out;
			out << std::setprecision(12) << currentVariables[itemIndicator].variations;
			std::string tempstr = out.str();
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
	}
	return false;
}

bool VariableSystem::deleteVariable(LPARAM lParamOfMessage)
{

	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(variablesListview.hwnd, &cursorPos);
	NMITEMACTIVATE itemClicked = *(NMITEMACTIVATE*)lParamOfMessage;
	int subitemIndicator = itemClicked.iSubItem;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = SendMessage(variablesListview.hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM)&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == currentVariables.size())
	{
		// user didn't click in a deletable item.
		return false;
	}
	int answer = MessageBox(0, ("Delete variable " + currentVariables[itemIndicator].name + "?").c_str(), 0, MB_YESNO);
	if (answer == IDYES)
	{
		SendMessage(variablesListview.hwnd, LVM_DELETEITEM, itemIndicator, 0);
		currentVariables.erase(currentVariables.begin() + itemIndicator);
	}
	return false;
}

variable VariableSystem::getVariableInfo(int varNumber)
{
	return currentVariables[varNumber];
}

unsigned int VariableSystem::getCurrentNumberOfVariables()
{
	return currentVariables.size();
}



bool VariableSystem::clearVariables()
{
	// clear the internal info holder;
	currentVariables.clear();
	// clear the listview
	int itemCount = SendMessage(variablesListview.hwnd, LVM_GETITEMCOUNT, 0, 0);
	// take the bottoms out of the variable listview
	for (int itemInc = 0; itemInc < itemCount; itemInc++)
	{
		SendMessage(variablesListview.hwnd, LVM_DELETEITEM, 0, 0);
	}
	// get the extra item too.
	//SendMessage(variablesListview.hwnd, LVM_DELETEITEM, itemCount, 0);
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
		SendMessage(variablesListview.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewDefaultItem);
		for (int itemInc = 1; itemInc < 6; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewDefaultItem); // Enter text to SubItems
		}
		return false;
	}
	/// add it to the internal structure that keeps track of variables
	variable tempVar;
	tempVar.name = name;
	tempVar.singleton = singleton;
	tempVar.timelike = timelike;
	tempVar.initialValue = value;
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
	SendMessage(variablesListview.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewItem);
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
	SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); 
	listViewItem.iSubItem = 2;
	if (singleton)
	{
		std::string tempValue = std::to_string(value).c_str();
		listViewItem.pszText = (LPSTR)tempValue.c_str();
	}
	else
	{
		listViewItem.pszText = "---";
	}
	// Enter text to SubItems
	SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);

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
	SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); 
	return false;
}

bool VariableSystem::initialize(POINT topLeftCorner, HWND parentWindow)
{
	RECT initPos = variablesHeader.normalPos = { topLeftCorner.x, topLeftCorner.y, topLeftCorner.x + 480, topLeftCorner.y + 25 };
	variablesHeader.hwnd = CreateWindowEx(0, "STATIC", "Variables", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, this->programInstance, NULL);
	SendMessage(variablesHeader.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	topLeftCorner.y += 25;
	initPos = variablesListview.normalPos = { topLeftCorner.x, topLeftCorner.y, topLeftCorner.x + 480, topLeftCorner.y + 195 };
	variablesListview.hwnd = CreateWindowEx(0, WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_VARIABLES_LISTVIEW, GetModuleHandle(NULL), NULL);
	SendMessage(variablesListview.hwnd, WM_SETFONT, WPARAM(sSmallFont), TRUE);

	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// width between each coloum
	listViewDefaultCollumn.pszText = "Symbol";
	listViewDefaultCollumn.cx = 0x42;
	// Inserting Couloms as much as we want
	SendMessage(variablesListview.hwnd, LVM_INSERTCOLUMN, 0, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Type";
	SendMessage(variablesListview.hwnd, LVM_INSERTCOLUMN, 1, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Timelike?";
	SendMessage(variablesListview.hwnd, LVM_INSERTCOLUMN, 2, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Initial Value";
	SendMessage(variablesListview.hwnd, LVM_INSERTCOLUMN, 3, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Final Value";
	SendMessage(variablesListview.hwnd, LVM_INSERTCOLUMN, 4, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Variations";
	SendMessage(variablesListview.hwnd, LVM_INSERTCOLUMN, 5, (LPARAM)&listViewDefaultCollumn);

	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
	listViewDefaultItem.cchTextMax = 256; // Max size of test
	listViewDefaultItem.pszText = "___";
	listViewDefaultItem.iItem = 0;          // choose item  
	listViewDefaultItem.iSubItem = 0;       // Put in first coluom
	SendMessage(variablesListview.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewDefaultItem);
	for (int itemInc = 1; itemInc < 6; itemInc++) // Add SubItems in a loop
	{
		listViewDefaultItem.iSubItem = itemInc;
		SendMessage(variablesListview.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewDefaultItem); // Enter text to SubItems
	}
	topLeftCorner.y += 400;
	return false;
}