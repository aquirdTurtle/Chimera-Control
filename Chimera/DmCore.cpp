#include "stdafx.h"
#include "DmCore.h"
#include "Expression.h"
#include "ProfileSystem.h"
#include "DmProfileCreator.h"
#include "DmOutputForm.h"
#include <iostream>

DmCore::DmCore(std::string Number, bool safemodeOption) : DM(safemodeOption){
	serial = Number;
	boolOfSerial = true;
	valueArray = std::vector<double>(DM.getActuatorCount(), 0.0);
}


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

void DmCore::handleNewConfig(std::ofstream& newFile) {
	newFile << "DM\n";
	// nothing at the moment.
	newFile << "END_DM\n";
}

DMOutputForm DmCore::handleGetConfig(std::ifstream& configFile, Version ver) {
	DMOutputForm Info;
	configFile >> Info.base;
	configFile.get();
	std::getline(configFile, Info.coma.expressionStr);
	std::getline(configFile, Info.comaAng.expressionStr);
	std::getline(configFile, Info.astig.expressionStr);
	std::getline(configFile, Info.astigAng.expressionStr);
	std::getline(configFile, Info.trefoil.expressionStr);
	std::getline(configFile, Info.trefoilAng.expressionStr);
	std::getline(configFile, Info.spherical.expressionStr);
	return Info;
}

void DmCore::handleOpenConfig(std::ifstream &openFile, Version Ver) {
	openFile.get();
	std::getline(openFile, currentInfo.coma.expressionStr);
	std::getline(openFile, currentInfo.comaAng.expressionStr);
	std::getline(openFile, currentInfo.astig.expressionStr);
	std::getline(openFile, currentInfo.astigAng.expressionStr);
	std::getline(openFile, currentInfo.trefoil.expressionStr);
	std::getline(openFile, currentInfo.trefoilAng.expressionStr);
	std::getline(openFile, currentInfo.spherical.expressionStr);
	openFile >> currentInfo.base;
}

void DmCore::handleSaveConfig(std::ofstream& saveFile, DMOutputForm out) {
	currentInfo = out;
	//add the new line for the delimeter
	saveFile << delimeter + "\n";
	saveFile << currentInfo.coma.expressionStr << "\n";
	saveFile << currentInfo.comaAng.expressionStr << "\n";
	saveFile << currentInfo.astig.expressionStr << "\n";
	saveFile << currentInfo.astigAng.expressionStr << "\n";
	saveFile << currentInfo.trefoil.expressionStr << "\n";
	saveFile << currentInfo.trefoilAng.expressionStr << "\n";
	saveFile << currentInfo.spherical.expressionStr << "\n";
	saveFile << currentInfo.base << "\n";
	saveFile << "END_" + delimeter + "\n";
}

void DmCore::interpretKey(std::vector<std::vector<parameterType>>& variables, DmCore &DM)
{
	UINT variations;
	UINT sequenceNumber;
	if (variables.size() == 0)
	{
		thrower("ERROR: variables empty, no sequence fill!");
	}
	else if (variables.front().size() == 0)
	{
		variations = 1;
	}
	else
	{
		variations = variables.front().front().keyValues.size();
	}
	sequenceNumber = variables.size();
	for (auto seqInc : range(sequenceNumber))
	{
		if (!DM_SAFEMODE)
		{
			DM.currentInfo.coma.internalEvaluate(variables[seqInc], variations);
			DM.currentInfo.comaAng.internalEvaluate(variables[seqInc], variations);
			DM.currentInfo.astig.internalEvaluate(variables[seqInc], variations);
			DM.currentInfo.astigAng.internalEvaluate(variables[seqInc], variations);
			DM.currentInfo.trefoil.internalEvaluate(variables[seqInc], variations);
			DM.currentInfo.trefoilAng.internalEvaluate(variables[seqInc], variations);
			DM.currentInfo.spherical.internalEvaluate(variables[seqInc], variations);
		}
		
	}
}

void DmCore::ProgramNow(UINT variation) {
    DmProfileCreator Profile;
	Profile.addComa(currentInfo.coma.getValue(variation), currentInfo.comaAng.getValue(variation));
	Profile.addAstigmatism(currentInfo.astig.getValue(variation), currentInfo.astigAng.getValue(variation));
	Profile.addTrefoil(currentInfo.trefoil.getValue(variation), currentInfo.trefoilAng.getValue(variation));
	Profile.addSpherical(currentInfo.spherical.getValue(variation));
	std::vector<double> temp = Profile.createZernikeArray(Profile.getCurrAmps(), currentInfo.base, false);
	loadArray(temp.data());
}

DMOutputForm DmCore::getInfo() {
	return currentInfo;
}