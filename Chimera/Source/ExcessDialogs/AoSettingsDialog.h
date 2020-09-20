// created by Mark O. Brown
#pragma once

#include "AnalogOutput/AoSystem.h"
#include <unordered_map>
#include <QLabel.h>
#include <QLineEdit.h>
#include <QObject.h>

struct aoInputStruct {
	AoSystem* aoSys;
};

/*
 * This control is an interface for extra Analog-Output settings. In particular, at the moment, the names of the 
 * different channels, and the min/max voltages allowed for each channel.
 */
class AoSettingsDialog : public QDialog {
	Q_OBJECT;
	public:
		AoSettingsDialog (aoInputStruct* inputPtr);
	public Q_SLOTS:
		void handleOk ();
		void handleCancel ();
	private:
		std::array<QLabel*, 24> numberLabels;
		std::array<QLineEdit*, 24> nameEdits;
		std::array < QLineEdit*, 24> noteEdits;
		std::array<QLineEdit*, 24> minValEdits;
		std::array<QLineEdit*, 24> maxValEdits;
		std::array<QLabel*, 3> dacNumberHeaders;
		std::array<QLabel*, 3> dacNameHeaders;
		std::array<QLabel*, 3> dacMinValHeaders;
		std::array<QLabel*, 3> dacMaxValHeaders;
		std::array<QLabel*, 3> noteHeaders;
		QPushButton* okBtn;
		QPushButton* cancelBtn;
		aoInputStruct* input;
};
