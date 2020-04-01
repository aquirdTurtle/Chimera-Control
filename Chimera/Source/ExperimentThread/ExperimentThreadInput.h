// created by Mark O. Brown
#pragma once

#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "AnalogInput/AiSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "Microwave/MicrowaveCore.h"
#include "MiscellaneousExperimentOptions/DebugOptionsControl.h"
#include "Agilent/AgilentCore.h"
#include "NIAWG/NiawgCore.h"
#include "Python/EmbeddedPythonHandler.h"
#include "Tektronix/TekCore.h"
#include "MiscellaneousExperimentOptions/MainOptionsControl.h"
#include "Piezo/PiezoCore.h"
#include "Andor/AndorCameraCore.h"
#include "expSystemRunList.h" 
#include "Andor/AndorRunSettings.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "ConfigurationSystems/profileSettings.h"
#include "RealTimeDataAnalysis/atomCruncherInput.h"
#include "RealTimeDataAnalysis/realTimePlotterInput.h"
#include "Basler/BaslerCamera.h"
#include "Basler/baslerSettings.h"
#include "Plotting/PlotDialog.h"
#include "ExperimentType.h"
#include "DeviceList.h"
#include "DirectDigitalSynthesis/DdsCore.h"

#include <chrono>
#include <vector>
#include <atomic>
#include <functional>

class AuxiliaryWindow;
class MainWindow;
class DataLogger;
class IChimeraWindow;
class ScriptingWindow;
class ExperimentThreadManager;

struct ExperimentThreadInput
{
	ExperimentThreadInput ( IChimeraWindow* win );
	realTimePlotterInput* plotterInput;
	EmbeddedPythonHandler& python;
	profileSettings profile;

	DoCore& ttls;
	AoSystem& aoSys;

	DeviceList devices;

	std::vector<parameterType> globalParameters;
	ExperimentThreadManager* thisObj;
	Communicator& comm;
	debugInfo debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
	ParameterSystem& globalControl;

	DataLogger& logger;
	UINT numVariations = 1;
	bool quiet = false;
	expSystemRunList runList;
	UINT numAiMeasurements=0;
	bool updatePlotterXVals = false;
	std::atomic<bool>* skipNext = NULL;
	atomGrid analysisGrid;
	ExperimentType expType = ExperimentType::Normal;
	// outermost vector is for each dac or ttl plot. next level is for each line.
	std::vector<std::vector<pPlotDataVec>> ttlData;
	std::vector<std::vector<pPlotDataVec>> dacData;
	// only for rearrangement.
	std::mutex* rearrangerLock;
	atomQueue* atomQueueForRearrangement;
	chronoTimes* andorsImageTimes;
	chronoTimes* grabTimes;
	std::condition_variable* conditionVariableForRerng;
	rerngGuiOptionsForm rerngGuiForm;
	//rerngGuiOptions rerngGui;
};


