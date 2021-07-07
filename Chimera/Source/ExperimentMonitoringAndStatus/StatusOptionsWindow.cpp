#include "stdafx.h"
#include "StatusOptionsWindow.h"
#include <boost/lexical_cast.hpp>

StatusOptionsWindow::StatusOptionsWindow(QWidget* parent, StatusControlOptions* optsIn, std::deque<statusMsg> msgHist) : QDialog(parent), opts(optsIn) {
	ui.setupUi(this);
	msgHistory = msgHist;
	initializeWidgets();
}

void StatusOptionsWindow::initializeWidgets() {
	ui.HistoryEdit->setText(qstr(opts->historyLength));
	ui.DebugLevelEdit->setText(qstr(opts->debugLvl));
	ui.ShowMsgOrigin->setChecked(opts->showOrigin);
	ui.okBtn->connect(ui.okBtn, &QPushButton::pressed, [this]() {
		try {
			opts->showOrigin = ui.ShowMsgOrigin->isChecked();
			opts->debugLvl = boost::lexical_cast<unsigned>(str(ui.DebugLevelEdit->text()));
			opts->historyLength = boost::lexical_cast<unsigned>(str(ui.HistoryEdit->text()));
			loadFromTable();
		}
		catch (boost::bad_lexical_cast&) {
			return;
		}
		close();
		});
	ui.cancelBtn->connect(ui.cancelBtn, &QPushButton::pressed, [this]() {
		close();
		});
	//
	ui.SystemSpecificsTable->setColumnCount(3);
	for (auto item : msgHistory) {
		if (!nameAlreadyLoaded(item.systemDelim)) {
			auto ind = ui.SystemSpecificsTable->rowCount();
			ui.SystemSpecificsTable->insertRow(ind);
			bool found = false;
			for (auto setting : opts->indvOptions) {
				if (opts->indvOptions.contains(item.systemDelim)) {
					auto prevItem = opts->indvOptions[item.systemDelim];
					ui.SystemSpecificsTable->setItem(ind, 0, new QTableWidgetItem(qstr(item.systemDelim)));
					ui.SystemSpecificsTable->setItem(ind, 1, new QTableWidgetItem(qstr(prevItem.debugLvlOffset)));
					ui.SystemSpecificsTable->setItem(ind, 2, new QTableWidgetItem(prevItem.show? "true" : "false"));
					found = true;
					break;
				}
			}
			if (!found) {
				ui.SystemSpecificsTable->setItem(ind, 0, new QTableWidgetItem(qstr(item.systemDelim)));
				ui.SystemSpecificsTable->setItem(ind, 1, new QTableWidgetItem("0"));
				ui.SystemSpecificsTable->setItem(ind, 2, new QTableWidgetItem("true"));
			}
		}
	}
	ui.SystemSpecificsTable->setHorizontalHeaderLabels({ "System", "Dbg Lvl", "Show?" });
}

bool StatusOptionsWindow::nameAlreadyLoaded(QString name) {
	for (auto itemn : range(ui.SystemSpecificsTable->rowCount())) {
		if (ui.SystemSpecificsTable->item(itemn, 0)->text() == name) {
			return true;
		}
	}
	return false;
}

void StatusOptionsWindow::loadFromTable() {
	opts->indvOptions.clear();
	for (auto itemNum : range(ui.SystemSpecificsTable->rowCount())) {
		indvOption option;
		try {
			option.debugLvlOffset = boost::lexical_cast<unsigned>(str(ui.SystemSpecificsTable->item(itemNum, 1)->text()));
		}
		catch (boost::bad_lexical_cast& err) {
			// probably shouldn't be quiet...
		}
		option.show = ui.SystemSpecificsTable->item(itemNum, 2)->text() == "true" ? true : false;
		opts->indvOptions[ui.SystemSpecificsTable->item(itemNum, 0)->text()] = option;
	}
}