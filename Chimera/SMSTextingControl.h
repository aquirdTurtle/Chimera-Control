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
		// void promptForEmailAddressAndPassword();
		void initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips, 
						 rgbMap rgbs );
		void sendMessage( std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType );
		void addPerson( personInfo person );
		void updatePersonInfo();
		void deletePersonInfo();
		void rearrange( int width, int height, fontMap fonts );
	private:
		const std::string emailAddress = "quantumGasAssemblyControl@gmail.com";
		const std::string password = "rubidium85HasStrongerInteractions";
		Control<CListCtrl> peopleListView;
		Control<CButton> enterEmailInfoButton;
		Control<CStatic> title;
		std::vector<personInfo> peopleToText;
};