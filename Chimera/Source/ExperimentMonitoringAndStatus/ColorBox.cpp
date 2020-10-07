// created by Mark O. Brown
#include "stdafx.h"
#include "ColorBox.h"
#include <tuple>
#include <PrimaryWindows/IChimeraQtWindow.h>

void ColorBox::initialize(QPoint& pos, IChimeraQtWindow* parent, int length, DeviceList devices, unsigned numrows) {
	int& px = pos.rx (), & py = pos.ry ();
	auto numCtrls = devices.list.size ()+1;
	boxes.resize (numCtrls);
	int itemsPerRow = ceil (float (numCtrls) / numrows);
	int indvLength = length / itemsPerRow;
	for (auto devInc : range (numCtrls-1)) {
		if (((devInc % itemsPerRow) == 0) && (devInc != 0)) {
			py += 20;
		}
		auto& box = boxes[devInc];
		auto& dev = devices.list[devInc].get ();
		box.delim = dev.getDelim ();
		box.ctrl = new QLabel (box.delim.substr(0,5).c_str(), parent);
		box.ctrl->setToolTip (box.delim.c_str ());
		box.ctrl->setGeometry (px + (devInc % itemsPerRow) * indvLength, py, indvLength, 20);
		box.ctrl->setStyleSheet ("QLabel { font: 8pt; }");
	}
	auto& box = boxes.back();
	box.delim = "Other";
	box.ctrl = new QLabel (qstr(box.delim), parent);
	box.ctrl->setToolTip (box.delim.c_str ());
	box.ctrl->setGeometry (px + ((numCtrls - 1)%itemsPerRow) * indvLength, py, indvLength, 20);
	box.ctrl->setStyleSheet ("QLabel { font: 8pt; }");

	py += 20;
	initialized = true;
}

void ColorBox::changeColor( std::string delim, std::string color ){
	bool foundMatch = false;
	for (auto& device : boxes){
		if (device.delim == delim){
			foundMatch = true;
			device.color = color;
			device.ctrl->setStyleSheet (("QLabel {background-color: \"" + str(color) + "\"; font: 8pt;}").c_str());
		}
	}
	if (!foundMatch) {
		boxes.back ().color = color;
		boxes.back ().ctrl->setStyleSheet (("QLabel {background-color: \"" + str (color) + "\"; font: 8pt;}").c_str ());
	}
}