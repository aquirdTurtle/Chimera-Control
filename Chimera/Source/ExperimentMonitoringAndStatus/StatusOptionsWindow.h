#pragma once

#include <qdialog.h>
#include "ui_StatusControlOptionDlg.h"
#include "StatusControlOptions.h"
#include <deque>

class StatusOptionsWindow : public QDialog {
	Q_OBJECT;
	public:
		StatusOptionsWindow(QWidget* parent, StatusControlOptions* optsIn, std::deque<statusMsg> msgHistory);
		bool nameAlreadyLoaded(QString name);
		void loadFromTable();
	private:
		void initializeWidgets();
		Ui::StatusControlOptionsDlg ui;
		StatusControlOptions* opts;
		std::deque<statusMsg> msgHistory;
};
