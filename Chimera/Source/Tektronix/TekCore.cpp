#include "stdafx.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "TekCore.h"
#include "TektronixStructures.h"

TekCore::TekCore (bool safemode, std::string address, std::string configurationFileDelimiter) : 
	visaFlume (safemode, address), configDelim (configurationFileDelimiter){
	try{
		visaFlume.open ();
	}
	catch (ChimeraError & err){
		errBox ("Failed to initialize tektronics visa connection! " + err.trace ());
	}
}

TekCore::~TekCore (){
	visaFlume.close ();
}

void TekCore::programVariation (unsigned variation, std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	if (experimentActive){
		if (experimentInfo.channels[0].control || experimentInfo.channels[1].control){
			for (auto channelInc : range (experimentInfo.channels.size ())){
				auto& channel = experimentInfo.channels[channelInc];
				auto ch_s = str (channelInc + 1);
				if (channel.control){
					if (channel.on){
						// SCPI 
						visaFlume.write ("SOURCE" + ch_s + ":FREQ " + str (channel.mainFreq.getValue (variation)));
						visaFlume.write ("SOURCE" + ch_s + ":VOLT:UNIT DBM");
						visaFlume.write ("SOURCE" + ch_s + ":VOLT " + str (channel.power.getValue (variation)));
						visaFlume.write ("SOURCE" + ch_s + ":VOLT:OFFS 0");
						if (channel.fsk){
							visaFlume.write ("SOURCE" + ch_s + ":FSKey:STATe On");
							visaFlume.write ("SOURCE" + ch_s + ":FSKey:FREQ " + str (channel.fskFreq.getValue (variation)));
							visaFlume.write ("SOURCE" + ch_s + ":FSKey:SOURce External");
						}
						else{
							visaFlume.write ("SOURCE" + ch_s + ":FSKey:STATe Off");
						}
						visaFlume.write ("OUTput" + ch_s + ":STATe ON");
					}
					else{
						visaFlume.write ("OUTput" + ch_s + ":STATe OFF");
					}
				}
			}
		}
	}
}

std::string TekCore::queryIdentity (){
	try{
		auto res = visaFlume.identityQuery ();
		return res;
	}
	catch (ChimeraError & err){
		return err.trace ();
	}
}

void TekCore::calculateVariations (std::vector<parameterType>& parameters, ExpThreadWorker* threadworker){
	calculateVariations (parameters);
}

void TekCore::calculateVariations (std::vector<parameterType>& parameters) {
	if (experimentActive){
		unsigned variations = (parameters.size() == 0 ? 1 : parameters.front ().keyValues.size ());
		for (auto& channel : experimentInfo.channels){
			if (channel.on)	{
				channel.mainFreq.internalEvaluate (parameters, variations);
				channel.power.internalEvaluate (parameters, variations);
				if (channel.fsk) { channel.fskFreq.internalEvaluate (parameters, variations); }
			}
		}
	}
}

tektronixInfo TekCore::getSettingsFromConfig (ConfigStream& configFile){
	auto getlineF = ConfigSystem::getGetlineFunc (configFile.ver);
	tektronixInfo tekInfo;
	for (auto chanInc : range (tekInfo.channels.size ())){
		ConfigSystem::checkDelimiterLine (configFile, "CHANNEL_" + str (chanInc + 1));
		auto& channel = tekInfo.channels[chanInc];
		configFile >> channel.control >> channel.on >> channel.fsk;
		configFile.get ();
		getlineF (configFile, channel.power.expressionStr);
		getlineF (configFile, channel.mainFreq.expressionStr);
		getlineF (configFile, channel.fskFreq.expressionStr);
	}
	return tekInfo;
}

void TekCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
	try{
		H5::Group tektronixGroup;
		try	{
			tektronixGroup = H5::Group (log.file.createGroup ("/Tektronics"));
		}
		catch (H5::Exception err){
			// probably has just already been created.
			tektronixGroup = H5::Group (log.file.openGroup ("/Tektronics"));
		}
		H5::Group thisTek (tektronixGroup.createGroup (getDelim()));
		log.writeDataSet (experimentInfo.machineAddress, "Machine-Address", thisTek);
		unsigned channelCount = 1;
		for (auto& channel : experimentInfo.channels){
			H5::Group thisChannel (thisTek.createGroup ("Channel_" + str (channelCount++)));
			log.writeDataSet (channel.control, "Controlled_Option", thisChannel);
			log.writeDataSet (channel.on, "Output_On", thisChannel);
			log.writeDataSet (channel.power.expressionStr, "Power", thisChannel);
			log.writeDataSet (channel.mainFreq.expressionStr, "Main_Frequency", thisChannel);
			log.writeDataSet (channel.fsk, "FSK_Option", thisChannel);
			log.writeDataSet (channel.fskFreq.expressionStr, "FSK_Frequency", thisChannel);
		}
	}
	catch (H5::Exception err){
		log.logError (err);
		throwNested ("Failed to write tektronics settings to the HDF5 data file!");
	}
}

void TekCore::loadExpSettings (ConfigStream& stream){
	ConfigSystem::stdGetFromConfig (stream, *this, experimentInfo);
	experimentActive = (experimentInfo.channels[0].control || experimentInfo.channels[1].control);
}


void TekCore::setSettings (tektronixInfo newInfo) {
	experimentInfo = newInfo;
}
