// created by Mark O. Brown
#pragma once

#include "DigitalOutput/DioSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "AnalogInput/AiSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "Microwave/MicrowaveCore.h"
#include "GeneralFlumes/GpibFlume.h"
#include "MiscellaneousExperimentOptions/DebugOptionsControl.h"
#include "Agilent/Agilent.h"
#include "NIAWG/NiawgController.h"
#include "Python/EmbeddedPythonHandler.h"
#include "Tektronix/TektronixAfgControl.h"
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
#include "DirectDigitalSynthesis/DdsSystem.h"
#include "Plotting/PlotDialog.h"
#include "Basler/BaslerCamera.h"
#include "ExperimentType.h"

#include <chrono>
#include <vector>
#include <atomic>

class AuxiliaryWindow;
class MainWindow;
class DataLogger;
class ExperimentThreadManager;

struct ExperimentThreadInput
{
	ExperimentThreadInput ( AuxiliaryWindow* auxWin, MainWindow* mainWin, AndorWindow* andorWin, BaslerWindow* basWin );
	realTimePlotterInput* plotterInput;

	EmbeddedPythonHandler& python;
	// for posting messages only!
	profileSettings profile;
	seqSettings seq;
	DioSystem& ttls;
	AoSystem& aoSys;
	AiSystem& aiSys;
	AndorCameraCore& andorCamera;
	BaslerCameraCore& basCamera;
	DdsCore& dds;
	std::vector<PiezoCore*> piezoControllers;
	// TODO: this should be loaded from config file, not gui thread.
	std::vector<parameterType> globalParameters;
	ExperimentThreadManager* thisObj;
	Communicator& comm;
	MicrowaveCore& rsg;
	debugInfo debugOptions = { 0, 0, 0, 0, 0, 0, 0, "", 0, 0, 0 };
	std::vector<Agilent*> agilents;
	TektronixAfgControl& topBottomTek;
	TektronixAfgControl& eoAxialTek;
	ParameterSystem& globalControl;
	NiawgController& niawg;
	DataLogger& logger;
	UINT intensityAgilentNumber=-1;
	UINT numVariations = 1;
	bool quiet = false;
	expSystemRunList runList;
	UINT numAiMeasurements=0;
	bool updatePlotterXVals = false;
	bool dontActuallyGenerate = false;
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
	std::atomic<bool>* skipNext=NULL;
	atomGrid analysisGrid;

	ExperimentType expType = ExperimentType::Normal;
};


