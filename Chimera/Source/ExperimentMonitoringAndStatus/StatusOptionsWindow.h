#pragma once
#include <qdialog.h>
#include "ui_StatusControlOptionDlg.h"
#include "StatusControlOptions.h"

class StatusOptionsWindow : public QDialog {
	Q_OBJECT;
	public:
		StatusOptionsWindow(QWidget* parent, StatusControlOptions* optsIn);

	private:
		void initializeWidgets();
		Ui::StatusControlOptionsDlg ui;
		StatusControlOptions* opts;

};
