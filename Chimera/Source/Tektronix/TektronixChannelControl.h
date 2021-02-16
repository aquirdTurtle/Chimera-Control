#pragma once
#include "TektronixStructures.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

class TektronixChannelControl{
	public:
		void initialize (QPoint loc, IChimeraQtWindow* parent, std::string channel1Text, long width);
		tektronixChannelOutput getTekChannelSettings ();
		void setSettings (tektronixChannelOutput info);
		void handleEnabledStatus ();
	private:
		QLabel* channelLabel;
		CQCheckBox* controlButton;
		CQCheckBox* onOffButton;
		CQCheckBox* fskButton;
		CQLineEdit* power;
		CQLineEdit* mainFreq;
		CQLineEdit* fskFreq;
		tektronixChannelOutput currentInfo;
};
