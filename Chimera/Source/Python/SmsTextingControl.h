// created by Mark O. Brown
#pragma once

#include <string>
#include "Control.h"
#include "Python/EmbeddedPythonHandler.h"
#include "GeneralObjects/commonTypes.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"


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
		void initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips );
		void sendMessage( std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType );
		void addPerson( personInfo person );
		void updatePersonInfo();
		void deletePersonInfo();
		void rearrange( int width, int height, fontMap fonts );
	private:
		const std::string emailAddress = "quantumGasAssemblyControl@gmail.com";
		const std::string password = "theLaughingGoatHasBetterCoffee";
		Control<MyListCtrl> peopleListView;
		Control<CStatic> title;
		std::vector<personInfo> peopleToText;
};