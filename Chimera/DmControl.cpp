#include "stdafx.h"
#include "Thrower.h"
#include "ProfileSystem.h"
#include "range.h"
#include "DmControl.h"
#include "string.h"
#include <boost/lexical_cast.hpp>


DmControl::DmControl(std::string serialNumber, bool safeMode) : defObject(serialNumber, safeMode){
	defObject.initialize();
}

void DmControl::initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT control_id) {
	currentInfo.ActuatorCount = count;
	currentInfo.serialNumber = serialNumber;
	std::vector<pistonButton> piston = std::vector<pistonButton>(count);
	auto& cid = control_id;
	if (count != 137) {
		thrower("GUI Only Works with acutator count of 137");
	}
	else {
		POINT init = loc;
		int i;
		for (i = 0; i < 5; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.x = (init.x - (2 * width));
		loc.y += width;
		for (; i < 14; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.x = (init.x - (3 * width));
		loc.y += width;
		for (; i < 25; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (3 * width));
		for (; i < 36; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (4 * width));
		for (; i < 49; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (4 * width));
		for (; i < 62; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (4 * width));
		for (; i < 75; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (4 * width));
		for (; i < 88; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (4 * width));
		for (; i < 101; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (3 * width));
		for (; i < 112; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (3 * width));
		for (; i < 123; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x - (2 * width));
		for (; i < 132; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
		loc.y += width;
		loc.x = (init.x);
		for (; i < 137; i++) {
			piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
			piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
			piston[i].Voltage.EnableWindow(0);
			loc.x += width;
		}
	}

}

void DmControl::handleOnPress(int i) {

	piston[i].Voltage.EnableWindow();
}

void DmControl::updateButtons() {
	defObject.readDMArray(temp);
	double value;
	for (int i = 0; i < 137; i++) {
	
		value = temp[i];
		double rounded = (int)(value * 1000.0) / 1000.0;
		std::string s1 = boost::lexical_cast<std::string>(rounded);
		piston[i].Voltage.SetWindowTextA(cstr(s1));
	}
}

void DmControl::programNow() {
	std::vector<double> values;
	for (int i = 0; i < 137; i++) {
		CString s1;
		piston[i].Voltage.GetWindowTextA(s1);
		std::string s2 = str(s1, 4, false, true);
		double s3 = boost::lexical_cast<double>(s2);
		values.push_back(s3);
	}
	defObject.loadArray(values);
	updateButtons();
}

void DmControl::setMirror(std::vector<double> A) {
	defObject.loadArray(A);
	updateButtons();

}