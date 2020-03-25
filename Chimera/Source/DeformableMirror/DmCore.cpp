#include "stdafx.h"
#include "DmCore.h"
#include "ParameterSystem/Expression.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "DmProfileCreator.h"
#include "DmOutputForm.h"
#include <iostream>

DmCore::DmCore(std::string Number, bool safemodeOption) : DM(safemodeOption), profile() 
{
	serial = Number;
	boolOfSerial = true;
	valueArray = std::vector<double>(DM.getActuatorCount(), 0.0);
}


void DmCore::setSerial(std::string Number) 
{
	serial = Number;
	boolOfSerial = true;
}

std::string DmCore::getDeviceInfo() 
{
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

void DmCore::setMap() 
{
	DM.LoadMap(map_lut.data());
}

void DmCore::pokePiston(unsigned int piston, double value) 
{
	DM.setSingle(piston, value);
}
void DmCore::zeroPistons() 
{
	DM.reset();
}

void DmCore::readDMArray(std::vector<double> &testArray) 
{
	DM.getArray(testArray.data(), DM.getActuatorCount());
}

void DmCore::testMirror() 
{
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

void DmCore::loadArray(double *A)
{
	setMap();
	DM.setArray(A, map_lut.data());
}

int DmCore::getActCount() 
{
	return DM.getActuatorCount();
}

std::vector<double> DmCore::getActuatorValues() 
{
	readDMArray(valueArray);
	return valueArray;
}

void DmCore::handleNewConfig(std::ofstream& newFile) 
{
	newFile << "DM\n";
	// nothing at the moment.
	newFile << "END_DM\n";
}

DMOutputForm DmCore::handleGetConfig(ScriptStream& configFile, Version ver) 
{
	DMOutputForm Info;
	configFile.get();
	std::getline(configFile, Info.coma.expressionStr);
	std::getline(configFile, Info.comaAng.expressionStr);
	std::getline(configFile, Info.astig.expressionStr);
	std::getline(configFile, Info.astigAng.expressionStr);
	std::getline(configFile, Info.trefoil.expressionStr);
	std::getline(configFile, Info.trefoilAng.expressionStr);
	std::getline(configFile, Info.spherical.expressionStr);
	configFile >> Info.base;
	return Info;
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

void DmCore::ProgramNow(UINT variation) 
{
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

void DmCore::initialCheck(UINT variation, std::string& warnings) 
{
	std::string location = DM_PROFILES_LOCATION + "\\" + currentInfo.base + ".txt";
	profile.addComa(currentInfo.coma.getValue(variation), currentInfo.comaAng.getValue(variation));
	profile.addAstigmatism(currentInfo.astig.getValue(variation), currentInfo.astigAng.getValue(variation));
	profile.addTrefoil(currentInfo.trefoil.getValue(variation), currentInfo.trefoilAng.getValue(variation));
	profile.addSpherical(currentInfo.spherical.getValue(variation));
	profile.readZernikeFile(location);
	std::vector<double> temp = profile.createZernikeArray(profile.getCurrAmps(), currentInfo.base, true);
	for (auto& element : temp) {
		if (element < 0 || element > 1) {
			warnings += "Caution, variation " + str(variation) + " will cause one or more pistons in the DM to rail.";
			break;
		}
	}
}