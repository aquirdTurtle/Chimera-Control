#include "stdafx.h"
#include "MyVariableClass.h"	
#include "commctrl.h"
//#include "Windows.h"
#include "constants.h"

MyVariableClass::MyVariableClass()
{

}
MyVariableClass::~MyVariableClass()
{

}
bool MyVariableClass::initializeVariableControls(POINT& upperLeftHandLocation, HWND parentWindow)
{
	RECT location;
	location.left = upperLeftHandLocation.x;
	location.top = upperLeftHandLocation.y;
	location.right = location.left + 480;
	location.bottom = location.top + 25;
	variableTitle.normalPosition = location;
	variableTitle.hwnd = CreateWindowEx(0, "STATIC", "Scripting Variables", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
										location.left, location.top, location.right - location.left, location.bottom - location.top,
										parentWindow, (HMENU)-1, GetModuleHandle(NULL), NULL);
	location.top += 25; 
	location.bottom += 225;
	variableListControl.normalPosition = location;
	variableListControl.hwnd = CreateWindowEx(0, WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
											  location.left, location.top, location.right - location.left, location.bottom - location.top,
											  parentWindow, (HMENU)ID_VAR_LIST, GetModuleHandle(NULL), NULL);
	LVITEM listViewDefaultItem;
	/// Make Columns
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));                  
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;    
	// width between each coloum
	listViewDefaultCollumn.cx = 0x28;                                   
	listViewDefaultCollumn.pszText = "Variable Name";
	listViewDefaultCollumn.cx = 0x42;   
	// Inserting Couloms as much as we want
	SendMessage(variableListControl.hwnd, LVM_INSERTCOLUMN, 0, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Range Start";
	SendMessage(variableListControl.hwnd, LVM_INSERTCOLUMN, 1, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Range End";
	SendMessage(variableListControl.hwnd, LVM_INSERTCOLUMN, 2, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Units (Cosmetic)";
	SendMessage(variableListControl.hwnd, LVM_INSERTCOLUMN, 3, (LPARAM)&listViewDefaultCollumn);
	// Make Items
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	//memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn)); // Zero struct's Members

	//  Setting properties Of members:

	listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
	listViewDefaultItem.cchTextMax = 256; // Max size of test
	listViewDefaultItem.iItem = 0;          // choose item  
	listViewDefaultItem.iSubItem = 0;       // Put in first coluom
	listViewDefaultItem.pszText = "Test Variable 0"; // Text to display (can be from a char variable) (Items)

	SendMessage(variableListControl.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewDefaultItem); // Send info to the Listview

	for (int itemInc = 1; itemInc <= 3; itemInc++) // Add SubItems in a loop
	{
		std::string text = std::to_string(itemInc);
		listViewDefaultItem.iSubItem = itemInc;
		listViewDefaultItem.pszText = (LPSTR)text.c_str();
		SendMessage(variableListControl.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewDefaultItem); // Enter text to SubItems
	}
	location.top += 225; 
	location.bottom += 25;
	location.right = location.left + 240;
	cNewVariableButton.normalPosition = location;
	cNewVariableButton.hwnd = CreateWindowEx(NULL, "BUTTON", "+NEW", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		location.left, location.top, location.right - location.left, location.bottom - location.top,
		parentWindow, (HMENU)ID_VARIABLE_ADD, GetModuleHandle(NULL), NULL);
	
	location.left += 240;
	location.right += 240;
	cDeleteButton.normalPosition = location;
	cDeleteButton.hwnd = CreateWindowEx(NULL, "BUTTON", "-DELETE", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
										location.left, location.top, location.right - location.left, location.bottom - location.top,
										parentWindow, (HMENU)ID_VARIABLE_DELETE, GetModuleHandle(NULL), NULL);
	upperLeftHandLocation.y = location.bottom;
	return true;
}

int MyVariableClass::addVariable(std::string variableName, double initialVal, double finalVal, std::string units)
{
	// get number of items already in the list.
	int itemCount = ListView_GetItemCount(variableListControl.hwnd);
	// prompt user for information;
	

	return 0;
}
int MyVariableClass::deleteVariable()
{
	int position;
	position = ListView_GetNextItem(variableListControl.hwnd, -1, LVNI_SELECTED);
	return 0;
}
