#include "stdafx.h"
#include "TektronixChannelControl.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

void TektronixChannelControl::initialize (QPoint loc, IChimeraQtWindow* parent, std::string channelText, long width){
	auto& px = loc.rx (), & py = loc.ry ();
	channelLabel = new QLabel (cstr (channelText), parent);
	channelLabel->setGeometry (px, py, width, 20);
	
	controlButton = new CQCheckBox ("", parent);
	controlButton->setGeometry (px, py+=20, width, 20);
	parent->connect (controlButton, &QCheckBox::stateChanged, [parent]() {parent->configUpdated (); });
	parent->connect (controlButton, &QCheckBox::stateChanged, [this]() {handleEnabledStatus (); });

	onOffButton = new CQCheckBox ("", parent);
	onOffButton->setGeometry (px, py += 20, width, 20);
	parent->connect (onOffButton, &QCheckBox::stateChanged, [parent]() {parent->configUpdated (); });
	parent->connect (onOffButton, &QCheckBox::stateChanged, [this]() {handleEnabledStatus (); });

	fskButton = new CQCheckBox ("", parent);
	fskButton->setGeometry (px, py += 20, width, 20);
	parent->connect (fskButton, &QCheckBox::stateChanged, [parent]() {parent->configUpdated (); });
	parent->connect (fskButton, &QCheckBox::stateChanged, [this]() {handleEnabledStatus (); });

	power = new CQLineEdit ("", parent);
	power->setGeometry (px, py += 20, width, 20);
	parent->connect (power, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	power->setEnabled (false);

	mainFreq = new CQLineEdit ("", parent);
	mainFreq->setGeometry (px, py += 20, width, 20);
	parent->connect (mainFreq, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	mainFreq->setEnabled (false);

	fskFreq = new CQLineEdit ("", parent);
	fskFreq->setGeometry (px, py += 20, width, 20);
	parent->connect (fskFreq, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	fskFreq->setEnabled (false);
	py += 20;
	handleEnabledStatus ();
}


void TektronixChannelControl::handleEnabledStatus (){
	if (!controlButton->isChecked ()){
		onOffButton->setEnabled (false);
		fskButton->setEnabled (false);
		power->setEnabled (false);
		mainFreq->setEnabled (false);
		fskFreq->setEnabled (false);
		return;
	}
	else{
		onOffButton->setEnabled (true);
	}
	if (onOffButton->isChecked ()){
		fskButton->setEnabled (true);
		power->setEnabled (true);
		mainFreq->setEnabled (true);
		if (fskButton->isChecked()){
			fskFreq->setEnabled (true);
		}
		else{
			fskFreq->setEnabled (false);
		}
	}
	else{
		fskButton->setEnabled (false);
		power->setEnabled (false);
		mainFreq->setEnabled (false);
		fskFreq->setEnabled (false);
	}
}

tektronixChannelOutput TektronixChannelControl::getTekChannelSettings (){
	currentInfo.control = controlButton->isChecked();
	currentInfo.on = onOffButton->isChecked ();
	currentInfo.fsk = fskButton->isChecked ();
	currentInfo.power = str (power->text(), 13, false, true);
	currentInfo.mainFreq = str (mainFreq->text(), 13, false, true);
	currentInfo.fskFreq = str (fskFreq->text(), 13, false, true);
	return currentInfo;
}

void TektronixChannelControl::setSettings (tektronixChannelOutput info){
	currentInfo = info;
	controlButton->setChecked (currentInfo.control);
	onOffButton->setChecked (currentInfo.on);
	fskButton->setChecked (currentInfo.fsk);
	power->setText(cstr (currentInfo.power.expressionStr));
	mainFreq->setText (cstr (currentInfo.mainFreq.expressionStr));
	fskFreq->setText (cstr (currentInfo.fskFreq.expressionStr));
}
