#include "stdafx.h"
#include "StatusOptionsWindow.h"
#include <boost/lexical_cast.hpp>

StatusOptionsWindow::StatusOptionsWindow(QWidget* parent, StatusControlOptions* optsIn) : QDialog(parent), opts(optsIn) {
	ui.setupUi(this);
	initializeWidgets();
}

void StatusOptionsWindow::initializeWidgets() {
	ui.HistoryEdit->setText(qstr(opts->historyLength));
	ui.DebugLevelEdit->setText(qstr(opts->debugLvl));
	ui.ShowMsgOrigin->setChecked(opts->showOrigin);
	opts->debugLvl;
	ui.okBtn->connect(ui.okBtn, &QPushButton::pressed, [this]() {
		try {
			opts->showOrigin = ui.ShowMsgOrigin->isChecked();
			opts->debugLvl = boost::lexical_cast<unsigned>(str(ui.DebugLevelEdit->text()));
			opts->historyLength = boost::lexical_cast<unsigned>(str(ui.HistoryEdit->text()));
		}
		catch (boost::bad_lexical_cast&) {
			return;
		}
		close();
		});
	ui.cancelBtn->connect(ui.cancelBtn, &QPushButton::pressed, [this]() {
		close();
		});
}

