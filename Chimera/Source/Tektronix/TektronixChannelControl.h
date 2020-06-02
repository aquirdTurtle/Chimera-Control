#pragma once
#include "TektronixStructures.h"
#include "afxwin.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

class TektronixChannelControl
{
	public:
		void initialize (POINT loc, IChimeraWindowWidget* parent, std::string channel1Text, LONG width);
		tektronixChannelOutput getTekChannelSettings ();
		void setSettings (tektronixChannelOutput info);
		void rearrange (int width, int height, fontMap fonts);
		void handleEnabledStatus ();
	private:
		QLabel* channelLabel;
		QCheckBox* controlButton;
		QCheckBox* onOffButton;
		QCheckBox* fskButton;
		QLineEdit* power;
		QLineEdit* mainFreq;
		QLineEdit* fskFreq;
		tektronixChannelOutput currentInfo;
};
