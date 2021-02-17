#include "stdafx.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include <DataLogging/DataLogger.h>
#include <ExperimentThread/ExpThreadWorker.h>
#include "MicrowaveCore.h"

MicrowaveCore::MicrowaveCore() : uwFlume(UW_SYSTEM_ADDRESS, UW_SYSTEM_SAFEMODE){}

void MicrowaveCore::setTrigTime (double time) {
	triggerTime = time;
}

void MicrowaveCore::programVariation (unsigned variationNumber, std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	if (!experimentActive) { return; }
	if (!experimentSettings.control || experimentSettings.list.size () == 0)	{
		// Nothing to program.
		return;
	}
	//setPmSettings ();
	try	{
		if (experimentSettings.list.size () == 1)	{
			uwFlume.programSingleSetting (experimentSettings.list[0], variationNumber);
		}
		else{
			uwFlume.programList (experimentSettings.list, variationNumber, triggerTime);
		}
	}
	catch (ChimeraError&)	{
		// I don't think this really happens much anymore, have fixed some small bugs in windfreak programming which
		// were probably causing this. 
		if (threadworker != nullptr) {
			emit threadworker->warn ("Failed to program windfreak first time! Trying again...");
		}
		// should probably emit a warning here. 
		try	{
			// something in the windfreak seems to need flushing at this point.
			try {
				uwFlume.query ("?");
			}
			catch (ChimeraError & ) {}
			if (experimentSettings.list.size () == 1) {
				uwFlume.programSingleSetting (experimentSettings.list[0], variationNumber);
			}
			else {
				uwFlume.programList (experimentSettings.list, variationNumber, triggerTime);
			}
		}
		catch (ChimeraError & ){
			throwNested ("Failed to program Windfreak!");
		}
	}
	if (threadworker != nullptr) {
		notify({ "Windfreak list setting programmed: " + qstr(getCurrentList()), 2 }, threadworker);
	}
}

void MicrowaveCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
	try {
		H5::Group microwaveGroup;
		try {
			microwaveGroup = log.file.createGroup ("/Microwave");
		}
		catch (H5::Exception&) {
			microwaveGroup = log.file.openGroup ("/Microwave");
		}
		log.writeDataSet (experimentSettings.control, str ("Control"), microwaveGroup);
		unsigned count = 0;
		for (auto& listSetting : experimentSettings.list) {
			auto listElemGroup = microwaveGroup.createGroup ("List Elem #" + str (count++));
			log.writeDataSet (listSetting.frequency.expressionStr, "Frequency", listElemGroup);
			log.writeDataSet (listSetting.power.expressionStr, "Power", listElemGroup);
		}
	}
	catch (H5::Exception&) {
		throwNested ("Failed to save microwave settings to H5 File!");
	}
}

std::string MicrowaveCore::queryIdentity (){
	return uwFlume.queryIdentity ();
}

void MicrowaveCore::setFmSettings (){
	uwFlume.setFmSettings ();
}

void MicrowaveCore::setPmSettings (){
	uwFlume.setPmSettings ();
}

void MicrowaveCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	if (!experimentSettings.control) {
		return;
	}
	unsigned variations;
	if (params.size () == 0){
		variations = 1;
	}
	else{
		variations = params.front ().keyValues.size ();
	}
	for (auto freqInc : range(experimentSettings.list.size())){
		experimentSettings.list[freqInc].frequency.internalEvaluate (params, variations);
		experimentSettings.list[freqInc].power.internalEvaluate (params, variations);
	}
	notify({ qstr("Microwave List Setting: " + getCurrentList()), 1 }, threadworker);
}

std::pair<DoRows::which, unsigned> MicrowaveCore::getUWaveTriggerLine() {
	return uwaveTriggerLine;
}

unsigned MicrowaveCore::getNumTriggers (microwaveSettings settings){
	return settings.list.size () == 1 ? 0 : settings.list.size ();
}

microwaveSettings MicrowaveCore::getSettingsFromConfig (ConfigStream& openFile){
	microwaveSettings settings;
	auto getlineF = ConfigSystem::getGetlineFunc (openFile.ver);
	openFile >> settings.control;
	unsigned numInList = 0;
	openFile >> numInList;
	if (numInList > 100){
		auto res = QMessageBox::question (nullptr, "Suspicious...",
			"Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
			" was " + qstr (numInList) + ". Is this acceptable?");
		if (res == QMessageBox::No){
			thrower ("Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
					 " was " + str (numInList) + ".");
		}
	}
	settings.list.resize (numInList);
	if (numInList > 0){
		openFile.get ();
	}
	for (auto num : range (numInList)){
		getlineF (openFile, settings.list[num].frequency.expressionStr);
		getlineF (openFile, settings.list[num].power.expressionStr);
	}
	return settings;
}

void MicrowaveCore::loadExpSettings (ConfigStream& stream){
	ConfigSystem::stdGetFromConfig (stream, *this, experimentSettings);
	experimentActive = experimentSettings.control;
}

std::string MicrowaveCore::getCurrentList () {
	return uwFlume.getListString ();
}
