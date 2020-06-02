#include "stdafx.h"
#include "TektronixChannelControl.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

void TektronixChannelControl::initialize (POINT loc, IChimeraWindowWidget* parent, std::string channelText, LONG width)
{
	channelLabel = new QLabel (cstr (channelText), parent);
	channelLabel->setGeometry (loc.x, loc.y, width, 20);
	
	controlButton = new QCheckBox ("", parent);
	controlButton->setGeometry (loc.x, loc.y+=20, width, 20);
	parent->connect (controlButton, &QCheckBox::stateChanged, [parent]() {parent->configUpdated (); });
	parent->connect (controlButton, &QCheckBox::stateChanged, [this]() {handleEnabledStatus (); });

	onOffButton = new QCheckBox ("", parent);
	onOffButton->setGeometry (loc.x, loc.y += 20, width, 20);
	parent->connect (onOffButton, &QCheckBox::stateChanged, [parent]() {parent->configUpdated (); });
	parent->connect (onOffButton, &QCheckBox::stateChanged, [this]() {handleEnabledStatus (); });

	fskButton = new QCheckBox ("", parent);
	fskButton->setGeometry (loc.x, loc.y += 20, width, 20);
	parent->connect (fskButton, &QCheckBox::stateChanged, [parent]() {parent->configUpdated (); });
	parent->connect (fskButton, &QCheckBox::stateChanged, [this]() {handleEnabledStatus (); });

	power = new QLineEdit ("", parent);
	power->setGeometry (loc.x, loc.y += 20, width, 20);
	parent->connect (power, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	power->setEnabled (false);

	mainFreq = new QLineEdit ("", parent);
	mainFreq->setGeometry (loc.x, loc.y += 20, width, 20);
	parent->connect (mainFreq, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	mainFreq->setEnabled (false);

	fskFreq = new QLineEdit ("", parent);
	fskFreq->setGeometry (loc.x, loc.y += 20, width, 20);
	parent->connect (fskFreq, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	fskFreq->setEnabled (false);
}


void TektronixChannelControl::handleEnabledStatus ()
{
	if (controlButton->isChecked ())
	{
		onOffButton->setEnabled (false);
		fskButton->setEnabled (false);
		power->setEnabled (false);
		mainFreq->setEnabled (false);
		fskFreq->setEnabled (false);
	}
	if (onOffButton->isChecked ())
	{
		fskButton->setEnabled (true);
		power->setEnabled (true);
		mainFreq->setEnabled (true);
		if (fskButton->isChecked())
		{
			fskFreq->setEnabled (true);
		}
		else
		{
			fskFreq->setEnabled (false);
		}
	}
	else
	{
		fskButton->setEnabled (false);
		power->setEnabled (false);
		mainFreq->setEnabled (false);
		fskFreq->setEnabled (false);
	}
}

tektronixChannelOutput TektronixChannelControl::getTekChannelSettings ()
{
	currentInfo.control = controlButton->isChecked();
	currentInfo.on = onOffButton->isChecked ();
	currentInfo.fsk = fskButton->isChecked ();
	currentInfo.power = str (power->text(), 13, false, true);
	currentInfo.mainFreq = str (mainFreq->text(), 13, false, true);
	currentInfo.fskFreq = str (fskFreq->text(), 13, false, true);
	return currentInfo;
}

void TektronixChannelControl::setSettings (tektronixChannelOutput info)
{
	currentInfo = info;
	controlButton->setChecked (currentInfo.control);
	onOffButton->setChecked (currentInfo.on);
	fskButton->setChecked (currentInfo.fsk);
	power->setText(cstr (currentInfo.power.expressionStr));
	mainFreq->setText (cstr (currentInfo.mainFreq.expressionStr));
	fskFreq->setText (cstr (currentInfo.fskFreq.expressionStr));
}

void TektronixChannelControl::rearrange (int width, int height, fontMap fonts){}
