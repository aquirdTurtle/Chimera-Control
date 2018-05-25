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

#include "AndorRunSettings.h"
#include "atomGrid.h"
#include "profileSettings.h"
#include "atomCruncherInput.h"
#include "realTimePlotterInput.h"

#include <chrono>
#include <vector>
#include <atomic>

class MasterManager;

struct MasterThreadInput
{
	EmbeddedPythonHandler* python;
	// for posting messages only!
	AuxiliaryWindow* auxWin;
	profileSettings profile;
	seqSettings seq;
	DioSystem* ttls;
	AoSystem* aoSys;
	AiSystem* aiSys;
	UINT repetitionNumber;
	// believe outer layer here is for sequence increment
	std::vector<std::vector<parameterType>> variables;
	std::vector<std::vector<parameterType>> constants;
	//std::vector<funcVarMap> functionVars;
	MasterManager* thisObj;
	Communicator* comm;
	RhodeSchwarz* rsg;
	debugInfo debugOptions;
	std::vector<Agilent*> agilents;
	TektronicsControl* topBottomTek;
	TektronicsControl* eoAxialTek;
	ParameterSystem* globalControl;
	NiawgController* niawg;
	UINT intensityAgilentNumber;
	bool quiet;
	mainOptions settings;
	bool runNiawg;
	bool runMaster;
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

	bool isLoadMot = false;
	bool isCameraCal = false;
};


struct ExperimentInput
{
	ExperimentInput::ExperimentInput( ) :
		includesCameraRun( false ), masterInput( NULL ), plotterInput( NULL ), cruncherInput( NULL ) { }
	MasterThreadInput* masterInput;
	realTimePlotterInput* plotterInput;
	atomCruncherInput* cruncherInput;
	AndorRunSettings camSettings;
	bool includesCameraRun;
};
