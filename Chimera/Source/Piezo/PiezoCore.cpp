#include "stdafx.h"
#include "Piezo/PiezoCore.h"
#include "Piezo/PiezoType.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigSystem.h"

std::string PiezoCore::systemScope = "piezo";

PiezoCore::PiezoCore (piezoSetupInfo info) :
	controllerType ( info.type),
	flume ( info.type != PiezoType::B, info.addr),
	serFlume ( info.type != PiezoType::A, info.addr),
	configDelim( info.name )
{}

void PiezoCore::programAll ( piezoChan<double> vals ){
	programXNow ( vals.x );
	programYNow ( vals.y );
	programZNow ( vals.z );
}

void PiezoCore::programVariation ( unsigned variationNumber, std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	if (experimentActive){
		programXNow (expSettings.pztValues.x.getValue (variationNumber));
		programYNow (expSettings.pztValues.y.getValue (variationNumber));
		programZNow (expSettings.pztValues.z.getValue (variationNumber));
	}
}

void PiezoCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	size_t totalVariations = (params.size () == 0) ? 1 : params.front ().keyValues.size ();
	if (experimentActive){
		try{
			expSettings.pztValues.x.assertValid (params, systemScope);
			expSettings.pztValues.y.assertValid (params, systemScope);
			expSettings.pztValues.z.assertValid (params, systemScope);
			expSettings.pztValues.x.internalEvaluate (params, totalVariations);
			expSettings.pztValues.y.internalEvaluate (params, totalVariations);
			expSettings.pztValues.z.internalEvaluate (params, totalVariations);
		}
		catch (ChimeraError&){
			throwNested ("Failed to evaluate piezo expression varations!");
		}
	}
}

piezoSettings PiezoCore::getSettingsFromConfig ( ConfigStream& file ){
	piezoSettings tempSettings;
	auto getlineF = ConfigSystem::getGetlineFunc (file.ver);
	file.get ( );
	getlineF ( file, tempSettings.pztValues.x.expressionStr );
	getlineF ( file, tempSettings.pztValues.y.expressionStr);
	getlineF ( file, tempSettings.pztValues.z.expressionStr);
	file >> tempSettings.ctrlPzt;
	file.get ( );
	return tempSettings;
}

void PiezoCore::initialize ( ){
	switch ( controllerType ){
		case PiezoType::A:
			serFlume.open ( );
			break;
		case PiezoType::B:
			flume.open ( );
	}
}

double PiezoCore::getCurrentXVolt ( ){
	switch (controllerType){
		case PiezoType::A:
			return serFlume.getXAxisVoltage ();
		case PiezoType::B:
			return flume.getXAxisVoltage ();
	}
	return 0;
}

double PiezoCore::getCurrentYVolt (){
	switch (controllerType){
	case PiezoType::A:
		return serFlume.getYAxisVoltage ();
	case PiezoType::B:
		return flume.getYAxisVoltage ();
	}
	return 0;
}

double PiezoCore::getCurrentZVolt (){
	switch (controllerType){
		case PiezoType::A: return serFlume.getZAxisVoltage (); break;
		case PiezoType::B: return flume.getZAxisVoltage (); break;
	}
	return 0;
}

void PiezoCore::programXNow ( double val ){
	switch (controllerType){
		case PiezoType::A: serFlume.setXAxisVoltage (val); break;
		case PiezoType::B: flume.setXAxisVoltage (val); break;
	}
} 
 
void PiezoCore::programYNow ( double val ){
	switch (controllerType){
		case PiezoType::A: serFlume.setYAxisVoltage (val); break;
		case PiezoType::B: flume.setYAxisVoltage (val); break;
	}
}

void PiezoCore::programZNow ( double val ){
	switch (controllerType){
		case PiezoType::A: serFlume.setZAxisVoltage (val); break;
		case PiezoType::B: flume.setZAxisVoltage (val); break;
	}
}

std::string PiezoCore::getDeviceInfo ( ){
	switch ( controllerType ){
		case PiezoType::A:
			return serFlume.getDeviceInfo ( );
		case PiezoType::B:
			return flume.getDeviceInfo ( );
		case PiezoType::NONE:
			return "NONE";
		default:
			return "PIEZO TYPE NOT RECOGNIZED!";
	}
}

std::string PiezoCore::getDeviceList ( ){
	return flume.list ( );
}

void PiezoCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
}

void PiezoCore::loadExpSettings (ConfigStream& stream){
	ConfigSystem::stdGetFromConfig (stream, *this, expSettings);
	experimentActive = expSettings.ctrlPzt;
}

