#include "stdafx.h"
#include "Thrower.h"
#include "ProfileSystem.h"
#include "range.h"
#include "DmControl.h"




void DmControl::initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT control_id) {
	currentInfo.ActuatorCount = count;
	currentInfo.serialNumber = serialNumber;
	std::vector<pistonButton> piston = std::vector<pistonButton>(count);
	auto& cid = control_id;
	if (count != 137) {
		thrower("GUI Only Works with acutator count of 137");
	}
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
	for(; i < 36; i++) {
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
	for(;  i < 75; i++) {
		piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
		piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
		piston[i].Voltage.EnableWindow(0);
		loc.x += width;
	}
	loc.y += width;
	loc.x = (init.x - (4 * width));
	for(; i < 88; i++) {
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
	for(; i < 112; i++) {
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
	loc.x = (init.x );
	for (; i < 137; i++) {
		piston[i].Voltage.sPos = { loc.x, loc.y, loc.x + width, loc.y + width };
		piston[i].Voltage.Create(NORM_CHECK_OPTIONS, piston[i].Voltage.sPos, parent, cid++);
		piston[i].Voltage.EnableWindow(0);
		loc.x += width;
	}
		

}

void DmControl::handleOnPress(int i) {

	piston[i].Voltage.EnableWindow();
}