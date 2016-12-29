#pragma once

#include <string>
//#include "easendmailobj.tlh"
#include "Control.h"
#include "EmbeddedPythonHandler.h"

struct personInfo
{
	std::string name;
	std::string number;
	std::string provider;
	bool textWhenComplete;
	bool textIfLoadingStops;
};

class SMSTextingControl
{
	public:
		SMSTextingControl();
		~SMSTextingControl();
		bool promptForEmailAddressAndPassword();
		bool initializeControls(POINT& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, std::unordered_map<std::string, CFont*> fonts,
			std::vector<CToolTipCtrl*>& tooltips);
		bool rearrange(RECT parentRectangle, std::string mode);
		bool sendMessage(std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType);
		bool updatePersonInfo(HWND parentHandle, LPARAM lparamOfMessage);
		bool deletePersonInfo(HWND parentHandle, LPARAM lparamOfMessage);
	private:
		std::string emailAddress;
		std::string password;
		Control<CListCtrl> peopleListView;
		Control<CStatic> title;
		std::vector<personInfo> peopleToText;
};