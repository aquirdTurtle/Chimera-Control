#pragma once

#include <string>
#include "easendmailobj.tlh"
#include "Control.h"

struct personInfo
{
	std::string name;
	std::string number;
	std::string provider;
	bool textWhenComplete;
};

class SMSTextingControl
{
	public:
		SMSTextingControl();
		~SMSTextingControl();
		bool promptForEmailAddressAndPassword();
		bool initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous, HWND parentWindow, 
								bool isTriggerModeSensitive);
		bool reorganizeControls(RECT parentRectangle, std::string mode);
		bool sendMessage(std::string message);
		bool updatePersonInfo(HWND parentHandle, LPARAM lparamOfMessage);
		bool deletePersonInfo(HWND parentHandle, LPARAM lparamOfMessage);
	private:
		std::string emailAddress;
		std::string password;
		EASendMailObjLib::IMailPtr mailObject;	
		Control peopleListView;
		Control title;
		std::vector<personInfo> peopleToText;
};