﻿// created by Mark O. Brown
#pragma once

#include "DioSystem.h"
#include "AoSystem.h"
#include "AiSystem.h"
#include "ParameterSystem.h"
#include "RhodeSchwarz.h"
#include "GpibFlume.h"
#include "DebugOptionsControl.h"
#include "Agilent.h"
#include "NiawgController.h"
#include "EmbeddedPythonHandler.h"
#include "TektronicsControl.h"
#include "MainOptionsControl.h"
#include "PiezoCore.h"
#include "Andor.h"

#include "AndorRunSettings.h"
#include "atomGrid.h"
#include "profileSettings.h"
#include "atomCruncherInput.h"
#include "realTimePlotterInput.h"
#include "baslerSettings.h"
#include "DdsSystem.h"
#include "PlotDialog.h"

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
	AndorCamera& andorCamera;
	DdsCore& dds;
	std::vector<PiezoCore*> piezoControllers;
	ScanRangeInfo variableRangeInfo;
	// believe outer layer here is for sequence increment
	std::vector<std::vector<parameterType>> parameters;
	MasterThreadManager* thisObj;
	Communicator& comm;
	RohdeSchwarz& rsg;
	debugInfo debugOptions;
	std::vector<Agilent*> agilents;
	TektronicsAfgControl& topBottomTek;
	TektronicsAfgControl& eoAxialTek;
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
