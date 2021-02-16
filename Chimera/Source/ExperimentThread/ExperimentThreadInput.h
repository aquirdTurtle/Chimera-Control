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
#include "ExperimentType.h"
#include "DeviceList.h"
#include "DirectDigitalSynthesis/DdsCore.h"

#include <chrono>
#include <vector>
#include <atomic>
#include <functional>

class MainWindow;
class DataLogger;
class IChimeraQtWindow;
class ExpThreadWorker;

struct ExperimentThreadInput{
	ExperimentThreadInput ( IChimeraQtWindow* win );
	realTimePlotterInput* plotterInput;
	profileSettings profile;

	DoCore& ttls;
	AoSystem& aoSys;

	DeviceList devices;
	std::vector<calResult> calibrations;
	std::vector<parameterType> globalParameters;
	unsigned sleepTime = 0;
	DataLogger& logger;
	unsigned numVariations = 1;
	bool quiet = false;
	//expSystemRunList runList;
	unsigned numAiMeasurements=0;
	bool updatePlotterXVals = false;
	std::atomic<bool>* skipNext = nullptr;
	atomGrid analysisGrid;
	ExperimentType expType = ExperimentType::Normal;
	// only for rearrangement.
	std::mutex* rearrangerLock;
	atomQueue* atomQueueForRearrangement;
	chronoTimes* andorsImageTimes;
	chronoTimes* grabTimes;
	std::condition_variable* conditionVariableForRerng;
};


