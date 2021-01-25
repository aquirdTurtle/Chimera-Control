// created by Mark O. Brown
#include "stdafx.h"
#include <algorithm>
#include "Python/SMSTextingControl.h"
#include "commctrl.h"
#include <boost/lexical_cast.hpp>
#include <QHeaderView>
#include <QMenu>
#include "PrimaryWindows/QtMainWindow.h"

SmsTextingControl::SmsTextingControl () {//: smtp("smtp.gmail.com", 465, SmtpClient::SslConnection) {
	//smtp.setUser (qstr(emailAddress));
	//smtp.setPassword (qstr(password));

}

void SmsTextingControl::handleContextMenu (const QPoint& pos){
	QTableWidgetItem* item = peopleListView->itemAt (pos);
	if (!item) { return; }
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete This Item", peopleListView);
	peopleListView->connect (deleteAction, &QAction::triggered, [this, item]() {peopleListView->removeRow (item->row ());});
	auto* newPerson = new QAction ("New Item", peopleListView);
	peopleListView->connect (newPerson, &QAction::triggered, [this]() {addPerson (personInfo ()); });
	menu.addAction (deleteAction);
	menu.addAction (newPerson);
	menu.exec (peopleListView->mapToGlobal (pos));
}

void SmsTextingControl::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();
	title = new QLabel ("TEXTING SERVICES" + PYTHON_SAFEMODE ? " (DISABLED, PYTHON_SAFEMODE=TRUE)" : "", parent);
	title->setGeometry (px, py, 480, 25);
	py += 25;
	peopleListView = new QTableWidget (0, 5, parent);
	QStringList labels;
	labels << "Person" << "Phone #" << "Carrier" << "@ Finish?" << "If No Loading?";
	peopleListView->setHorizontalHeaderLabels (labels);
	peopleListView->horizontalHeader ()->setFixedHeight (25);
	peopleListView->verticalHeader ()->setFixedWidth (25);
	peopleListView->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect ( peopleListView, &QTableWidget::customContextMenuRequested, 
					  [this](const QPoint& pos) {this->handleContextMenu(pos); });
	peopleListView->connect (peopleListView, &QTableWidget::cellDoubleClicked, [this](int clRow, int clCol) {
		if (clCol == 3 || clCol == 4) {
			auto* item = new QTableWidgetItem (peopleListView->item (clRow, clCol)->text () == "No" ? "Yes" : "No");
			item->setFlags (item->flags () & ~Qt::ItemIsEditable);
			peopleListView->setItem (clRow, clCol, item);
		}});

	peopleListView->setGeometry (px, py, 480, 50);
	py += 50;
	peopleListView->setColumnWidth (0, 90);
	peopleListView->setColumnWidth (1, 95);
	peopleListView->setColumnWidth (2, 65);
	peopleListView->setColumnWidth (3, 85);
	peopleListView->setColumnWidth (4, 115);
	peopleListView->setShowGrid (true);
	personInfo me;
	me.name = "Mark Brown";
	me.number = "7206797962";
	me.provider = "googlefi";
	me.textIfLoadingStops = false;
	me.textWhenComplete = false;
	addPerson (me);
}

void SmsTextingControl::addPerson( personInfo person ){
	int row = peopleListView->rowCount ();
	peopleListView->insertRow (row);
	peopleListView->setItem (row, 0, new QTableWidgetItem(person.name.c_str ())); 
	peopleListView->setItem (row, 1, new QTableWidgetItem (person.number.c_str()));
	peopleListView->setItem (row, 2, new QTableWidgetItem (person.provider.c_str ()));

	QTableWidgetItem* txtWhenCompleteItem = new QTableWidgetItem (person.textWhenComplete ? "Yes" : "No");
	txtWhenCompleteItem->setFlags (txtWhenCompleteItem->flags () & ~Qt::ItemIsEditable);
	peopleListView->setItem (row, 3, txtWhenCompleteItem);
	QTableWidgetItem* txtNoLoading = new QTableWidgetItem (person.textIfLoadingStops ? "Yes" : "No");
	txtNoLoading->setFlags (txtNoLoading->flags () & ~Qt::ItemIsEditable);
	peopleListView->setItem (row, 4, txtNoLoading);
}

void SmsTextingControl::deletePersonInfo(QTableWidgetItem* item){
	peopleListView->removeRow (item->row());
}

void SmsTextingControl::sendMessage(std::string message, std::string msgType){
	auto peopleToText = getPeopleFromListview ();
	if (msgType == "Loading"){
		for (personInfo& person : peopleToText){
			if (person.textIfLoadingStops){
				//MimeMessage message;
				//
				//message.setSender (new EmailAddress (qstr(emailAddress), "Chimera"));
				//message.addRecipient (new EmailAddress ("recipient@host.com", "Recipient's Name"));
				//message.setSubject ("SmtpClient for Qt - Demo");


				// send text gives an appropriate error message.
				//pyHandler->sendText( person, message, "Not Loading Atoms", emailAddress, password );
			}
		}
	}
	if ( msgType == "Mot" ){
		for ( personInfo& person : peopleToText ){
			if ( person.textIfLoadingStops ){
				// send text gives an appropriate error message.
				//pyHandler->sendText ( person, message, "Not Loading MOT", emailAddress, password );
			}
		}
	}
	else if (msgType == "Finished"){
		for (personInfo& person : peopleToText){
			if (person.textWhenComplete){
				// send text gives an appropriate error message.
				//pyHandler->sendText(person, message, "Experiment Finished", emailAddress, password);
			}
		}
	}
	else{
		thrower ("unrecognized text message type: " + msgType);
	}
}


std::vector<personInfo> SmsTextingControl::getPeopleFromListview (){
	std::vector<personInfo> people (peopleListView->rowCount ());
	for (auto personCount : range(people.size())){
		auto& person = people[personCount];
		person.name = str(peopleListView->item (personCount, 0)->text ());
		person.number = str(peopleListView->item (personCount, 1)->text ());
		person.provider = str (peopleListView->item (personCount, 2)->text ());
		person.textWhenComplete = (peopleListView->item (personCount, 3)->text () == "Yes" ? true : false);
		person.textIfLoadingStops = (peopleListView->item (personCount, 4)->text () == "Yes" ? true : false);
	}
	return people;
}

