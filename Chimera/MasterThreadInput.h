#pragma once

#include "DioSystem.h"
#include "AoSystem.h"
#include "AiSystem.h"
#include "VariableSystem.h"
#include "RhodeSchwarz.h"
#include "GpibFlume.h"
#include "DebugOptionsControl.h"
#include "Agilent.h"
#include "EmbeddedPythonHandler.h"
#include "atomGrid.h"
#include <chrono>
#include <vector>
#include <atomic>

class MasterManager;

struct MasterThreadInput
{
	EmbeddedPythonHandler* python;
	DataLogger* logger;
	profileSettings profile;
	seqSettings seq;
	DioSystem* ttls;
	AoSystem* aoSys;
	AiSystem* aiSys;
	UINT repetitionNumber;
	std::vector<std::vector<variableType>> variables;
	std::vector<std::vector<variableType>> constants;
	MasterManager* thisObj;
	Communicator* comm;
	RhodeSchwarz* rsg;
	debugInfo debugOptions;
	std::vector<Agilent*> agilents;
	TektronicsControl* topBottomTek;
	TektronicsControl* eoAxialTek;
	VariableSystem* globalControl;
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
	std::vector<std::vector<bool>>* atomQueueForRearrangement;
	chronoTimes* andorsImageTimes;
	chronoTimes* grabTimes;
	std::condition_variable* conditionVariableForRearrangement;
	rerngOptions rearrangeInfo;
	std::atomic<bool>* skipNext;
	atomGrid analysisGrid;
	
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
