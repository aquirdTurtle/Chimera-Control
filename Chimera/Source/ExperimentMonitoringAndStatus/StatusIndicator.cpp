// created by Mark O. Brown
#include "stdafx.h"
#include "StatusIndicator.h"
#include <PrimaryWindows/IChimeraQtWindow.h>

void StatusIndicator::initialize(QPoint &loc, IChimeraQtWindow* parent ){
	status = new QLabel ("Passively Outputing Default Waveform", parent);
	status->setGeometry (loc.x(), loc.y(), 960, 100);
	status->setStyleSheet ("QLabel {font: bold 30pt;}");
}

void StatusIndicator::setText(std::string text){
	status->setText (cstr (text));
}

void StatusIndicator::setColor(std::string color){
	currentColor = color;
}
