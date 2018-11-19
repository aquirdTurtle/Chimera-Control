#include "stdafx.h"
#include <algorithm>
#include "SMSTextingControl.h"
#include "commctrl.h"
#include "TextPromptDialog.h"
#include "Thrower.h"
#include <boost/lexical_cast.hpp>


void SmsTextingControl::initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips, rgbMap rgbs )
{
	title.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	std::string titletxt = "TEXT ME";
	if ( PYTHON_SAFEMODE )
	{
		titletxt += " (DISABLED, PYTHON_SAFEMODE=TRUE)";
	}
	title.Create( titletxt.c_str(), NORM_HEADER_OPTIONS, title.sPos, parent, id++ );
	title.fontType = fontTypes::HeadingFont;

	peopleListView.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 120 };
	peopleListView.Create( NORM_LISTVIEW_OPTIONS, peopleListView.sPos, parent, IDC_SMS_TEXTING_LISTVIEW );
	peopleListView.fontType = fontTypes::SmallFont;
	peopleListView.InsertColumn( 0, "Person", 0x42);
	peopleListView.InsertColumn( 1, "Phone #" );
	peopleListView.InsertColumn( 2, "Carrier");
	peopleListView.InsertColumn( 3, "At Finish?", 0x62 );
	peopleListView.InsertColumn( 4, "If No Loading?" );
	peopleListView.SetBkColor( rgbs["Solarized Base02"] );
	peopleListView.SetTextBkColor( rgbs["Solarized Base02"] );
	peopleListView.SetTextColor( rgbs["Solarized Base2"] );
	peopleListView.insertBlankRow ( );
	pos.y += 120;
	if ( !PYTHON_SAFEMODE )
	{
		// initialize myself;
		personInfo me;
		me.name = "Mark Brown";
		me.number = "7032544981";
		me.provider = "verizon";
		me.textIfLoadingStops = false;
		me.textWhenComplete = false;
		addPerson( me );
		personInfo tobias;
		tobias.name = "Tobias Thiele";
		tobias.number = "7207252841";
		tobias.provider = "googlefi";
		tobias.textIfLoadingStops = false;
		tobias.textWhenComplete = false;
		addPerson ( tobias );
	}
}




void SmsTextingControl::rearrange(int width, int height, fontMap fonts)
{
	title.rearrange( width, height, fonts);
	peopleListView.rearrange( width, height, fonts);	
}


void SmsTextingControl::addPerson( personInfo person )
{
	peopleListView.InsertItem( person.name, peopleToText.size ( ), 0 );
	peopleListView.SetItem ( person.number, peopleToText.size ( ), 1 );
	peopleListView.SetItem( person.provider, peopleToText.size ( ), 2 );
	peopleListView.SetItem ( person.textWhenComplete ? "Yes" : "No" , peopleToText.size ( ), 3 );
	peopleListView.SetItem ( person.textIfLoadingStops ? "Yes" : "No", peopleToText.size ( ), 4 );
	peopleToText.push_back( person );
}


void SmsTextingControl::updatePersonInfo()
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	peopleListView.ScreenToClient(&cursorPos);

	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	int subitemIndicator;
	peopleListView.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}	
	/// check if adding new person
	if (itemIndicator == peopleToText.size())
	{
		// add a person
		peopleToText.resize(peopleToText.size() + 1);
		peopleToText.back().textWhenComplete = false;
		peopleListView.insertBlankRow ( );
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName;
			TextPromptDialog dialog(&newName, "Please enter a name:");
			dialog.DoModal();
			if ( newName == "" )
			{
				newName = "noname";
			}
			// update the info inside
			peopleToText[itemIndicator].name = newName;
			peopleListView.SetItem(newName, itemIndicator, subitemIndicator );
			break;
		}
		case 1:
		{
			/// number
			// Prompt for a phone number			
			std::string phoneNumber;
			TextPromptDialog dialog(&phoneNumber, "Please enter a phone number (numbers only!):");
			dialog.DoModal();
			// test to see if it is a number like it should be (note: not sure if this catches a single decimal or not...)
			try
			{
				long long test = boost::lexical_cast<long long>(phoneNumber);
			}
			catch ( boost::bad_lexical_cast&)
			{
				throwNested ("Numbers only, please!");
			}
			peopleToText[itemIndicator].number = phoneNumber;
			peopleListView.SetItem( phoneNumber, itemIndicator, subitemIndicator );
			break;
		}
		case 2:
		{
			/// provider
			// Prompt for provider
			std::string newProvider;
			TextPromptDialog dialog(&newProvider, 
									"Please enter provider (either \"verizon\", \"tmobile\", \"at&t\", or "
									"\"googlefi\"):");
			dialog.DoModal();
			std::transform(newProvider.begin(), newProvider.end(), newProvider.begin(), ::tolower);
			if (newProvider != "verizon" && newProvider != "tmobile" && newProvider != "at&t" && newProvider != "googlefi")
			{
				errBox("Please enter either \"verizon\", \"tmobile\", \"at&t\", or \"googlefi\" (not case sensitive)");
				break;
			}
			peopleToText[itemIndicator].provider = newProvider;
			peopleListView.SetItem(newProvider, itemIndicator, subitemIndicator);
			break;
		}
		case 3:
		{
			/// at finish?
			// this is just a binary switch.
			auto& twc = peopleToText[ itemIndicator ].textWhenComplete;
			twc = !twc;
			peopleListView.SetItem ( twc ? "Yes" : "No", itemIndicator, subitemIndicator );
			break;
		}	
		case 4:
		{
			/// at finish?
			// this is just a binary switch.
			auto& tils = peopleToText[ itemIndicator ].textIfLoadingStops;
			tils = !tils;
			peopleListView.SetItem ( tils ? "Yes" : "No", itemIndicator, subitemIndicator );
			break;
		}
	}
}


void SmsTextingControl::deletePersonInfo()
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	peopleListView.ScreenToClient(&cursorPos);

	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	peopleListView.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	if (itemIndicator == -1 || itemIndicator == peopleToText.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer = promptBox("Delete info for " + peopleToText[itemIndicator].name + "?", MB_YESNO);
	if (answer == IDYES)
	{
		peopleListView.DeleteItem(itemIndicator);
		peopleToText.erase(peopleToText.begin() + itemIndicator);
	}
}


void SmsTextingControl::sendMessage(std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType)
{
	if (msgType == "Loading")
	{
		for (personInfo& person : peopleToText)
		{
			if (person.textIfLoadingStops)
			{
				// send text gives an appropriate error message.
				pyHandler->sendText( person, message, "Not Loading Atoms", emailAddress, password );
			}
		}
	}
	if ( msgType == "Mot" )
	{
		for ( personInfo& person : peopleToText )
		{
			if ( person.textIfLoadingStops )
			{
				// send text gives an appropriate error message.
				pyHandler->sendText ( person, message, "Not Loading MOT", emailAddress, password );
			}
		}
	}
	else if (msgType == "Finished")
	{
		for (personInfo& person : peopleToText)
		{
			if (person.textWhenComplete)
			{
				// send text gives an appropriate error message.
				pyHandler->sendText(person, message, "Experiment Finished", emailAddress, password);
			}
		}
	}
	else
	{
		thrower ("unrecognized text message type: " + msgType);
	}
}

