#include "stdafx.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <DataLogging/DataLogger.h>
#include "MicrowaveCore.h"


MicrowaveCore::MicrowaveCore() : uwFlume(UW_SYSTEM_ADDRESS, UW_SYSTEM_SAFEMODE){}

void MicrowaveCore::programVariation (unsigned variationNumber, std::vector<parameterType>& params){
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
			uwFlume.programList (experimentSettings.list, variationNumber);
		}
	}
	catch (ChimeraError&)	{
		// should probably emit a warning here. 
		// errBox ("First attempt to program uw system failed! Will try again. Uw system says: " + uwFlume.read ());
		try	{
			// something in the windfreak seems to need flushing at this point.
			try {
				uwFlume.query ("?");
			}
			catch (ChimeraError & ) {}
			try {
				uwFlume.query ("?");
			}
			catch (ChimeraError & ) {}
			try {
				uwFlume.query ("?");
			}
			catch (ChimeraError & ) {}
			if (experimentSettings.list.size () == 1) {
				uwFlume.programSingleSetting (experimentSettings.list[0], variationNumber);
			}
			else {
				uwFlume.programList (experimentSettings.list, variationNumber);
			}
		}
		catch (ChimeraError & ){
			throwNested ("Failed to program Windfreak!");
		}
	}
}

void MicrowaveCore::logSettings (DataLogger& log){
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
	calculateVariations (params);
}

void MicrowaveCore::calculateVariations (std::vector<parameterType>& params){
	if (!experimentSettings.control)	{
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
}

std::pair<DoRows::which, unsigned> MicrowaveCore::getRsgTriggerLine (){
	return rsgTriggerLine;
}


unsigned MicrowaveCore::getNumTriggers (microwaveSettings settings){
	return settings.list.size () == 1 ? 0 : settings.list.size ();
}

microwaveSettings MicrowaveCore::getSettingsFromConfig (ConfigStream& openFile){
	microwaveSettings settings;
	auto getlineF = ProfileSystem::getGetlineFunc (openFile.ver);
	openFile >> settings.control;
	unsigned numInList = 0;
	openFile >> numInList;
	if (numInList > 100){
		auto res = promptBox ("Detected suspiciously large number of microwave settings in microwave list. Number of list entries"
							  " was " + str (numInList) + ". Is this acceptable?", MB_YESNO);
		if (!res){
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
	ProfileSystem::stdGetFromConfig (stream, *this, experimentSettings);
	experimentActive = experimentSettings.control;
}
