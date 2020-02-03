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
#include "Andor/Andor.h"
 
#include "Andor/AndorRunSettings.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "ConfigurationSystems/profileSettings.h"
#include "RealTimeDataAnalysis/atomCruncherInput.h"
#include "RealTimeDataAnalysis/realTimePlotterInput.h"
#include "Basler/baslerSettings.h"
#include "DirectDigitalSynthesis/DdsSystem.h"
#include "Plotting/PlotDialog.h"

#include <chrono>
#include <vector>
#include <atomic>

class AuxiliaryWindow;
class MainWindow;
class DataLogger;

enum class ExperimentType
{
	// normal experiments
	Normal,
	// simple load mot, F1
	LoadMot,
	// camera background calibration
	CameraCal,
	// Calibration for determining Mot # & temperature
	MotSize,
	MotTemperature,
	PgcTemperature,
	GreyTemperature,

	// is part of machine optimization procedure
	MachineOptimization
};

class MasterThreadManager;

struct ExperimentThreadInput
{
	ExperimentThreadInput ( AuxiliaryWindow* auxWin, MainWindow* mainWin, AndorWindow* andorWin );
	realTimePlotterInput* plotterInput;

	EmbeddedPythonHandler& python;
	// for posting messages only!
	// AuxiliaryWindow* auxWin;
	profileSettings profile;
	seqSettings seq;
	DioSystem& ttls;
	AoSystem& aoSys;
	AiSystem& aiSys;
	AndorCameraCore& andorCamera;
	DdsCore& dds;
	std::vector<PiezoCore*> piezoControllers;
	ScanRangeInfo variableRangeInfo;
	// believe outer layer here is for sequence increment. 
	// TODO: this should be loaded from config file, not gui thread.
	std::vector<std::vector<parameterType>> parameters;
	MasterThreadManager* thisObj;
	Communicator& comm;
	MicrowaveCore& rsg;
	debugInfo debugOptions;
	std::vector<Agilent*> agilents;
	TektronixAfgControl& topBottomTek;
	TektronixAfgControl& eoAxialTek;
	ParameterSystem& globalControl;
	NiawgController& niawg;
	DataLogger& logger;
	UINT intensityAgilentNumber;
	UINT numVariations = 1;
	bool quiet=false;
	bool runNiawg;
	UINT numAiMeasurements=0;
	bool runMaster;
	bool runAndor;
	bool logBaslerPics;
	bool updatePlotterXVals=false;
	bool dontActuallyGenerate=false;
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
	std::atomic<bool>* skipNext;	
	atomGrid analysisGrid;

	ExperimentType expType = ExperimentType::Normal;
};


struct AllExperimentInput
{
	AllExperimentInput::AllExperimentInput( ) :
		includesAndorRun( false ), masterInput( NULL ), cruncherInput( NULL ) { }
	ExperimentThreadInput* masterInput;
	
	atomCruncherInput* cruncherInput;
	AndorRunSettings AndorSettings;
	baslerSettings baslerRunSettings;
	bool includesAndorRun;
};
