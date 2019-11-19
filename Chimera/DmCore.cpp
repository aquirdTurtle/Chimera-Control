#include "stdafx.h"
#include "DmCore.h"
#include "Expression.h"
#include <iostream>

DmCore::DmCore(std::string Number, bool safemodeOption) : DM(safemodeOption){
	serial = Number;
	boolOfSerial = true;
	
	valueArray = std::vector<double>(DM.getActuatorCount(), 0.0);
}

//DmCore::DmCore(bool safemodeOption) : DM(safemodeOption) {
//	serial = "";
//	boolOfSerial = false;
//}

void DmCore::setSerial(std::string Number) {
	serial = Number;
	boolOfSerial = true;
}

std::string DmCore::getDeviceInfo() {
	if (!boolOfSerial) {
		thrower("No Device");
	}
	else {
		return DM.getDeviceInfo(serial);
	}
}

void DmCore::initialize() {
	if (!boolOfSerial) {
		thrower("serial number not set for DM");
	}
	else {
		DM.open(serial);
	}
}

void DmCore::setMap() {
	DM.LoadMap(map_lut.data());
}

void DmCore::pokePiston(unsigned int piston, double value) {
	DM.setSingle(piston, value);
}
void DmCore::zeroPistons() {
	DM.reset();
}

void DmCore::readDMArray(std::vector<double> &testArray) {
	

	DM.getArray(testArray.data(), DM.getActuatorCount());

	
}

void DmCore::testMirror() {
	for (auto& val : valueArray)
	{
		val = 0.5;
	}
	//initialize();
	setMap();
	std::vector<double> testArray = std::vector<double>(DM.getActuatorCount(), 0.0);
	DM.setArray(valueArray.data(), map_lut.data());
	readDMArray(testArray);
}

void DmCore::loadArray(double *A) {
	setMap();
	DM.setArray(A, map_lut.data());
}

int DmCore::getActCount() {
	return DM.getActuatorCount();
}

std::vector<double> DmCore::getActuatorValues() {
	readDMArray(valueArray);
	return valueArray;
}
