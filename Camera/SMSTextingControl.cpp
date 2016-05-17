#include "stdafx.h"
#include "SMSTextingControl.h"
#include "externals.h"
#include "commctrl.h"
#include "constants.h"
#include "dialogProcedures.h"
#include <algorithm>
#include "appendText.h"
#include "reorganizeControl.h"

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
	title.hwnd = CreateWindowEx(0, "STATIC", "Text Me When Finished", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	title.fontType = "Heading";

	peopleListView.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y + 25, topLeftPositionKinetic.x + 272, topLeftPositionKinetic.y + 200 };
	peopleListView.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y + 25, topLeftPositionAccumulate.x + 272, topLeftPositionAccumulate.y + 200 };
	peopleListView.continuousSingleScansModePos = { -1, -1, -1, -1 };
	initPos = peopleListView.kineticSeriesModePos;
	peopleListView.hwnd = CreateWindowEx(0, WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_PEOPLE_LIST_VIEW, GetModuleHandle(NULL), NULL);
	title.fontType = "Normal";

	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	// Type of mask
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// width between each coloum
	listViewDefaultCollumn.cx = 0x28;
	listViewDefaultCollumn.pszText = "Person";
	listViewDefaultCollumn.cx = 0x42;
	// Inserting Couloms as much as we want
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 0, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Phone Number";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 1, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Carrier";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 2, (LPARAM)&listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Text?";
	SendMessage(peopleListView.hwnd, LVM_INSERTCOLUMN, 3, (LPARAM)&listViewDefaultCollumn);
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
			SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); // Enter text to SubItems
		}
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "No";
		SendMessage(peopleListView.hwnd, LVM_SETITEM, 0, (LPARAM)&listViewItem); // Enter text to SubItems
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
																  (LPARAM)std::string("Please enter provider (either \"Verizon\" or \"TMobile\"):").c_str());
			std::transform(newProvider.begin(), newProvider.end(), newProvider.begin(), ::tolower);
			if (newProvider != "verizon" && newProvider != "tmobile")
			{
				MessageBox(0, "Please enter either \"Verizon\" or \"TMobile\" (not case sensitive)", 0, 0);
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
			/// send text?
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

bool SMSTextingControl::sendMessage(std::string message)
{
	::CoInitialize(NULL);
	mailObject = NULL;
	mailObject.CreateInstance("EASendMailObj.Mail");
	// do I need to pay for this at some point...?
	mailObject->LicenseCode = _T("TryIt");
	// Set your sender email address
	mailObject->FromAddr = emailAddress.c_str();
	// Add recipients email address
	std::string recipientString;
	bool moreThanOne = false;
	recipientString = "";
	for (int peopleInc = 0; peopleInc < peopleToText.size(); peopleInc++)
	{
		if (peopleToText[peopleInc].textWhenComplete)
		{
			if (moreThanOne)
			{
				recipientString += ", ";
			}
			moreThanOne = true;
			recipientString += peopleToText[peopleInc].number;
			if (peopleToText[peopleInc].provider == "verizon")
			{
				// for verizon: [number]@vzwpix.com
				recipientString += "@vzwpix.com";
			}
			else if (peopleToText[peopleInc].provider == "tmobile")
			{
				// for tmobile: [number]@tmomail.net
				recipientString += "@tmomail.net";
			}
		}		
	}
	if (recipientString == "")
	{
		// no people to send messages to.
		return false;
	}
	mailObject->AddRecipientEx(recipientString.c_str(), 0);
	// Set email subject
	mailObject->Subject = "";

	// Set email body
	mailObject->BodyText = message.c_str();

	// Your SMTP server address
	mailObject->ServerAddr = "smtp.gmail.com";

	mailObject->UserName = emailAddress.c_str();
	mailObject->Password = password.c_str();

	// Set SSL 465 port
	mailObject->ServerPort = 465;

	// Set direct SSL connection
	mailObject->SSL_starttls = 0;
	mailObject->SSL_init();

	if (mailObject->SendMail() == 0)
	{
		appendText("Users have been notified via text of status update.", IDC_STATUS_EDIT);
	}
	else
	{
		appendText("Failed to notify users via text with error:" + std::string((const TCHAR*)mailObject->GetLastErrDescription()), IDC_ERROR_EDIT);
	}
	return false;
}

bool SMSTextingControl::reorganizeControls(RECT parentRectangle, std::string mode)
{
	reorganizeControl(title, mode, parentRectangle);
	reorganizeControl(peopleListView, mode, parentRectangle);
	return false;
}
