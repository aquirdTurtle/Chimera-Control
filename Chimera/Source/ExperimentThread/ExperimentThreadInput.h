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
#include "Basler/baslerSettings.h"
#include "Plotting/PlotDialog.h"
#include "Basler/BaslerCamera.h"
#include "ExperimentType.h"
#include "DirectDigitalSynthesis/DdsCore.h"
#include <chrono>
#include <vector>
#include <atomic>
#include <functional>

class AuxiliaryWindow;
class MainWindow;
class DataLogger;
class ScriptingWindow;
class ExperimentThreadManager;

struct ExperimentThreadInput
{
	ExperimentThreadInput ( AuxiliaryWindow* auxWin, MainWindow* mainWin, AndorWindow* andorWin, BaslerWindow* basWin,
							ScriptingWindow* scriptWin);
	realTimePlotterInput* plotterInput;
	EmbeddedPythonHandler& python;
	profileSettings profile;
	seqSettings seq;
	DoCore& ttls;
	AoSystem& aoSys;
	AiSystem& aiSys;
	AndorCameraCore& andorCamera;
	BaslerCameraCore& basCamera;
	DdsCore& dds;
	std::vector<std::reference_wrapper<PiezoCore>> piezoCores;
	// TODO: this should be loaded from config file, not gui thread.
	std::vector<parameterType> globalParameters;
	ExperimentThreadManager* thisObj;
	Communicator& comm;
	MicrowaveCore& rsg;
	debugInfo debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
	std::vector<std::reference_wrapper<AgilentCore>> agilents;
	TekCore& topBottomTek;
	TekCore& eoAxialTek;
	ParameterSystem& globalControl;
	NiawgCore& niawg;
	DataLogger& logger;
	UINT intensityAgilentNumber=-1;
	UINT numVariations = 1;
	bool quiet = false;
	expSystemRunList runList;
	UINT numAiMeasurements=0;
	bool updatePlotterXVals = false;
	bool dontActuallyGenerate = false;
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
	rerngGuiOptions rerngGui;
};


