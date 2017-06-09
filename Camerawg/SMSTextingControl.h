#pragma once

#include <string>
#include "Control.h"
#include "EmbeddedPythonHandler.h"
#include "commonTypes.h"

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
		void promptForEmailAddressAndPassword();
		void initializeControls(POINT& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, fontMap fonts,
								std::vector<CToolTipCtrl*>& tooltips);
		void rearrange(RECT parentRectangle, std::string mode);
		void sendMessage(std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType);
		void updatePersonInfo(HWND parentHandle, LPARAM lparamOfMessage);
		void deletePersonInfo(HWND parentHandle, LPARAM lparamOfMessage);
		void rearrange(int width, int height, fontMap fonts);
	private:
		std::string emailAddress;
		std::string password;
		Control<CListCtrl> peopleListView;
		Control<CStatic> title;
		std::vector<personInfo> peopleToText;
};