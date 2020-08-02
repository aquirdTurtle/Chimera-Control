// created by Mark O. Brown
#pragma once

#include <string>
#include "Control.h"
#include "Python/EmbeddedPythonHandler.h"
#include "GeneralObjects/commonTypes.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <QLabel>
#include <QTableWidget>

struct personInfo{
	std::string name="";
	std::string number="";
	std::string provider="";
	bool textWhenComplete=false;
	bool textIfLoadingStops=false;
};


class SmsTextingControl{
	public:
		void initialize( POINT& pos, IChimeraQtWindow* parent );
		void handleContextMenu (const QPoint& pos);
		void sendMessage( std::string message, EmbeddedPythonHandler* pyHandler, std::string msgType );
		void addPerson( personInfo person );
		void deletePersonInfo(QTableWidgetItem* item);
	private:
		const std::string emailAddress = "quantumGasAssemblyControl@gmail.com";
		const std::string password = "theLaughingGoatHasBetterCoffee";
		std::vector<personInfo> getPeopleFromListview ();
		QLabel* title;
		QTableWidget* peopleListView;
		//std::vector<personInfo> peopleToText;
};