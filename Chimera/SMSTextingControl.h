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

class SmsTextingControl
{
	public:
		void promptForEmailAddressAndPassword();
		void initialize(POINT& pos, CWnd* parent, bool isTriggerModeSensitive, int& id, fontMap fonts,
								cToolTips& tooltips);
		void sendMessage(std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType);
		void updatePersonInfo();
		void deletePersonInfo();
		void rearrange(int width, int height, fontMap fonts);
	private:
		std::string emailAddress;
		std::string password;
		Control<CListCtrl> peopleListView;
		Control<CStatic> title;
		std::vector<personInfo> peopleToText;
};