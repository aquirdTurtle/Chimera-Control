#include "stdafx.h"
#include "DmCore.h"
#include "ParameterSystem/Expression.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "DmProfileCreator.h"
#include "DmOutputForm.h"
#include <iostream>

DmCore::DmCore(std::string Number, bool safemodeOption) : DM(safemodeOption), profile() {
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
	for (auto& val : valueArray) {
		val = 0.5;
	}
	setMap();
	std::vector<double> testArray = std::vector<double>(DM.getActuatorCount(), 0.0);
	DM.setArray(valueArray.data(), map_lut.data());
	readDMArray(testArray);
}

void DmCore::loadArray(double *A){
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

DMOutputForm DmCore::getSettingsFromConfig(ConfigStream& configFile) {
	DMOutputForm Info;
	Info.coma.expressionStr = configFile.getline();
	Info.comaAng.expressionStr = configFile.getline ();
	Info.astig.expressionStr = configFile.getline ();
	Info.astigAng.expressionStr = configFile.getline ();
	Info.trefoil.expressionStr = configFile.getline ();
	Info.trefoilAng.expressionStr = configFile.getline ();
	Info.spherical.expressionStr = configFile.getline ();
	configFile >> Info.base;
	return Info;
}

void DmCore::handleSaveConfig(ConfigStream& saveFile, DMOutputForm out) {
	currentInfo = out;
	//add the new line for the delimeter
	saveFile << getDelim() + "\n";
	saveFile << currentInfo.coma << "\n";
	saveFile << currentInfo.comaAng << "\n";
	saveFile << currentInfo.astig << "\n";
	saveFile << currentInfo.astigAng << "\n";
	saveFile << currentInfo.trefoil << "\n";
	saveFile << currentInfo.trefoilAng << "\n";
	saveFile << currentInfo.spherical << "\n";
	saveFile << currentInfo.base << "\n";
	saveFile << "END_" + getDelim () + "\n";
}

void DmCore::interpretKey(std::vector<std::vector<parameterType>>& variables, DmCore &DM){
	unsigned variations;
	unsigned sequenceNumber;
	if (variables.size() == 0)	{
		thrower("ERROR: variables empty, no sequence fill!");
	}
	else if (variables.front().size() == 0)	{
		variations = 1;
	}
	else {
		variations = variables.front().front().keyValues.size();
	}
	sequenceNumber = variables.size();
	for (auto seqInc : range(sequenceNumber)) {
 		DM.currentInfo.coma.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.comaAng.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.astig.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.astigAng.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.trefoil.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.trefoilAng.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.spherical.assertValid(variables[seqInc], GLOBAL_PARAMETER_SCOPE);
		DM.currentInfo.coma.internalEvaluate(variables[seqInc], variations);
		DM.currentInfo.comaAng.internalEvaluate(variables[seqInc], variations);
		DM.currentInfo.astig.internalEvaluate(variables[seqInc], variations);
		DM.currentInfo.astigAng.internalEvaluate(variables[seqInc], variations);
		DM.currentInfo.trefoil.internalEvaluate(variables[seqInc], variations);
		DM.currentInfo.trefoilAng.internalEvaluate(variables[seqInc], variations);
		DM.currentInfo.spherical.internalEvaluate(variables[seqInc], variations);
	}
}

void DmCore::ProgramNow(unsigned variation) {
	std::string location  = DM_PROFILES_LOCATION + "\\" + currentInfo.base +".txt";
	profile.addComa(currentInfo.coma.getValue(variation), currentInfo.comaAng.getValue(variation));
	profile.addAstigmatism(currentInfo.astig.getValue(variation), currentInfo.astigAng.getValue(variation));
	profile.addTrefoil(currentInfo.trefoil.getValue(variation), currentInfo.trefoilAng.getValue(variation));
	profile.addSpherical(currentInfo.spherical.getValue(variation));
	profile.readZernikeFile(location);
	std::vector<double> temp = profile.createZernikeArray(profile.getCurrAmps(), currentInfo.base, false);
	loadArray(temp.data());
}

DMOutputForm DmCore::getInfo() {
	return currentInfo;
}

void DmCore::setCurrentInfo(DMOutputForm form) {
	currentInfo = form;
}

void DmCore::initialCheck(unsigned variation, std::string& warnings) {
	std::string location = DM_PROFILES_LOCATION + "\\" + currentInfo.base + ".txt";
	profile.addComa(currentInfo.coma.getValue(variation), currentInfo.comaAng.getValue(variation));
	profile.addAstigmatism(currentInfo.astig.getValue(variation), currentInfo.astigAng.getValue(variation));
	profile.addTrefoil(currentInfo.trefoil.getValue(variation), currentInfo.trefoilAng.getValue(variation));
	profile.addSpherical(currentInfo.spherical.getValue(variation));
	profile.readZernikeFile(location);
	std::vector<double> temp = profile.createZernikeArray(profile.getCurrAmps(), currentInfo.base, true);
	for (auto& element : temp) 	{
		if (element < 0 || element > 1) {
			warnings += "Caution, variation " + str(variation) + " will cause one or more pistons in the DM to rail.";
			break;
		}
	}
}

void DmCore::logSettings (DMOutputForm settings, DataLogger& log) {

}