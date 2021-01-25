// created by Mark O. Brown
#pragma once

#include <string>
#include "Control.h"
#include "GeneralObjects/commonTypes.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <QLabel>
#include <QTableWidget>

//#include "../../3rd_Party/SmtpClient-for-Qt-1.1/src/SmtpMime";
//#include "../src/SmtpMime"

struct personInfo{
	std::string name="";
	std::string number="";
	std::string provider="";
	bool textWhenComplete=false;
	bool textIfLoadingStops=false;
};


class SmsTextingControl{
	public:
		SmsTextingControl ();
		void initialize( QPoint& pos, IChimeraQtWindow* parent );
		void handleContextMenu (const QPoint& pos);
		void sendMessage( std::string message, std::string msgType );
		void addPerson( personInfo person );
		void deletePersonInfo(QTableWidgetItem* item);
	private:
		const std::string emailAddress = "quantumGasAssemblyControl@gmail.com";
		const std::string password = "theLaughingGoatHasBetterCoffee";
		std::vector<personInfo> getPeopleFromListview ();
		QLabel* title;
		QTableWidget* peopleListView;
		//SmtpClient smtp;
		//std::vector<personInfo> peopleToText;
};