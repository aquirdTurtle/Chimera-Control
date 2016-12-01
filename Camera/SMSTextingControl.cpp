#include "stdafx.h"
#include "SMSTextingControl.h"
#include "externals.h"
#include "commctrl.h"
#include "constants.h"
#include "dialogProcedures.h"
#include <algorithm>
#include "appendText.h"
#include "reorganizeControl.h"
#include <Algorithm>

SMSTextingControl::SMSTextingControl()
{

}

SMSTextingControl::~SMSTextingControl()
{

}

bool SMSTextingControl::promptForEmailAddressAndPassword()
{
	emailAddress = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)std::string("Please enter an email address:").c_str());
	password = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)std::string("Please enter a password:").c_str());
	return false;
}

bool SMSTextingControl::initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous, 
										   HWND parentWindow, bool isTriggerModeSensitive)
{
	title.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 272, topLeftPositionKinetic.y + 25 };
	title.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 272, topLeftPositionAccumulate.y + 25 };
	title.continuousSingleScansModePos = { -1,-1,-1,-1 };
	RECT initPos = title.kineticSeriesModePos;
	title.hwnd = CreateWindowEx(0, "STATIC", "Text Me", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	title.fontType = "Heading";

	peopleListView.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y + 25, topLeftPositionKinetic.x + 272, topLeftPositionKinetic.y + 120 };
	peopleListView.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y + 25, topLeftPositionAccumulate.x + 272, topLeftPositionAccumulate.y + 120 };
	peopleListView.continuousSingleScansModePos = { -1, -1, -1, -1 };
	initPos = peopleListView.kineticSeriesModePos;
	peopleListView.hwnd = CreateWindowEx(0, WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_PEOPLE_LIST_VIEW, GetModuleHandle(NULL), NULL);
	peopleListView.fontType = "Small";

	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// width between each coloum
	listViewDefaultCollumn.cx = 0x28;
	listViewDefaultCollumn.pszText = "Person";
	// Inserting Couloms as much as we want
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 0, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Phone #";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 1, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Carrier";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 2, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.cx = 0x42;
	listViewDefaultCollumn.pszText = "At Finish?";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 3, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "If No Loading?";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 4, (LPARAM)&listViewDefaultCollumn);

	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
	listViewDefaultItem.cchTextMax = 256; // Max size of test
	listViewDefaultItem.pszText = "___";
	listViewDefaultItem.iItem = 0;          // choose item  
	listViewDefaultItem.iSubItem = 0;       // Put in first coluom
	SendMessage(peopleListView.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewDefaultItem);
	for (int itemInc = 1; itemInc < 3; itemInc++) // Add SubItems in a loop
	{
		listViewDefaultItem.iSubItem = itemInc;
		SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewDefaultItem); // Enter text to SubItems
	}
	listViewDefaultItem.iSubItem = 3;
	listViewDefaultItem.pszText = "No";
	SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewDefaultItem); // Enter text to SubItems
	listViewDefaultItem.iSubItem = 4;
	SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewDefaultItem);

	topLeftPositionKinetic.y += 120;
	topLeftPositionAccumulate.y += 120;

	return false;
}

bool SMSTextingControl::updatePersonInfo(HWND parentHandle, LPARAM lparamOfMessage)
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(peopleListView.hwnd, &cursorPos);
	NMITEMACTIVATE itemClicked = *(NMITEMACTIVATE*)lparamOfMessage;
	int subitemIndicator = itemClicked.iSubItem;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = SendMessage(peopleListView.hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM)&myItemInfo);
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return false;
	}
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	
	listViewItem.iItem = itemIndicator;          // choose item  
	/// check if adding new person
	if (itemIndicator == peopleToText.size())
	{
		// add a person
		peopleToText.resize(peopleToText.size() + 1);
		peopleToText.back().textWhenComplete = false;
		// add an item to the control
		listViewItem.pszText = "___";
		listViewItem.iItem = itemIndicator;          // choose item  
		listViewItem.iSubItem = 0;       // Put in first coluom
		SendMessage(peopleListView.hwnd, LVM_INSERTITEM, 0, (LPARAM)&listViewItem);
		for (int itemInc = 1; itemInc < 3; itemInc++) // Add SubItems in a loop
		{
			listViewItem.iSubItem = itemInc;
			// Enter text to SubItems
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); 
		}
		// default texting options is no!
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "No";
		SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); 
		listViewItem.iSubItem = 4;
		SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);

	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)std::string("Please enter a name:").c_str());
			// update the info inside
			peopleToText[itemIndicator].name = newName;
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newName.c_str();
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
		case 1:
		{
			/// number
			// Prompt for a phone number
			std::string phoneNumber = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)std::string("Please enter a phone number (numbers only!):").c_str());
			// test to see if it is a number like it should be (note: not sure if this catches a single decimal or not...)
			try
			{
				long long test = std::stoll(phoneNumber);
			}
			catch (std::invalid_argument& exception)
			{
				MessageBox(0, "Numbers only, please!", 0, 0);
				break;
			}
			catch (std::out_of_range& exception)
			{
				MessageBox(0, "number was too long...", 0, 0);
				break;
			}
			peopleToText[itemIndicator].number = phoneNumber;
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)phoneNumber.c_str();
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
		case 2:
		{
			/// provider
			// Prompt for provider
			std::string newProvider = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
																  (DLGPROC)dialogProcedures::textPromptDialogProcedure, 
																  (LPARAM)std::string("Please enter provider (either \"verizon\", \"tmobile\", \"at&t\", or \"googlefi\"):").c_str());
			std::transform(newProvider.begin(), newProvider.end(), newProvider.begin(), ::tolower);
			if (newProvider != "verizon" && newProvider != "tmobile" && newProvider != "at&t" && newProvider != "googlefi")
			{
				MessageBox(0, "Please enter either \"verizon\", \"tmobile\", \"at&t\", or \"googlefi\" (not case sensitive)", 0, 0);
				break;
			}
			peopleToText[itemIndicator].provider = newProvider;
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newProvider.c_str();
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);
			break;
		}
		case 3:
		{
			/// at finish?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (peopleToText[itemIndicator].textWhenComplete)
			{
				peopleToText[itemIndicator].textWhenComplete = false;
				listViewItem.pszText = "No";
			}
			else
			{
				peopleToText[itemIndicator].textWhenComplete = true;
				listViewItem.pszText = "Yes";
			}
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);

			break;
		}	
		case 4:
		{
			/// at finish?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (peopleToText[itemIndicator].textIfLoadingStops)
			{
				peopleToText[itemIndicator].textIfLoadingStops = false;
				listViewItem.pszText = "No";
			}
			else
			{
				peopleToText[itemIndicator].textIfLoadingStops = true;
				listViewItem.pszText = "Yes";
			}
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem);

			break;
		}
	}

	return false;
}

bool SMSTextingControl::deletePersonInfo(HWND parentHandle, LPARAM lparamOfMessage)
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(peopleListView.hwnd, &cursorPos);
	NMITEMACTIVATE itemClicked = *(NMITEMACTIVATE*)lparamOfMessage;
	int subitemIndicator = itemClicked.iSubItem;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = SendMessage(peopleListView.hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM)&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == peopleToText.size())
	{
		// user didn't click in a deletable item.
		return false;
	}
	int answer = MessageBox(0, ("Delete info for " + peopleToText[itemIndicator].name + "?").c_str(), 0, MB_YESNO);
	if (answer == IDYES)
	{
		SendMessage(peopleListView.hwnd, LVM_DELETEITEM, itemIndicator, 0);
		peopleToText.erase(peopleToText.begin() + itemIndicator);
	}
	return false;
}

bool SMSTextingControl::sendMessage(std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType)
{
	if (msgType == "Loading")
	{
		for (personInfo& person : this->peopleToText)
		{
			if (person.textIfLoadingStops)
			{
				// send text gives an appropriate error message.
				if (pyHandler->sendText(person, message, "Not Loading Atoms", this->emailAddress, this->password))
				{
					return true;
				}
			}
		}
	}
	else if (msgType == "Finished")
	{
		for (personInfo& person : this->peopleToText)
		{
			if (person.textWhenComplete)
			{
				// send text gives an appropriate error message.
				if (pyHandler->sendText(person, message, "Experiment Finished", this->emailAddress, this->password))
				{
					return true;
				}
			}
		}
	}
	else
	{
		errBox("ERROR: unrecognized text message type: " + msgType);
		return true;
	}
	return false;
}

bool SMSTextingControl::reorganizeControls(RECT parentRectangle, std::string mode)
{
	reorganizeControl(title, mode, parentRectangle);
	reorganizeControl(peopleListView, mode, parentRectangle);
	return false;
}
