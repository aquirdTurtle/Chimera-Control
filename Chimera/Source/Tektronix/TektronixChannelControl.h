#pragma once
#include "TektronixStructures.h"
#include "afxwin.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

class TektronixChannelControl
{
	public:
		void initialize (POINT loc, IChimeraWindowWidget* parent, std::string channel1Text, LONG width);
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
