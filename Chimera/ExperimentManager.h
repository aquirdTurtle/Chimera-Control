#pragma once
#include "Windows.h"
#include <string>
#include "DebuggingOptionsControl.h"
#include "NiawgController.h"
#include "externals.h"
#include "constants.h"
#include <algorithm>
#include "myAgilent.h"
#include "experimentThreadInputStructure.h"
#include <sstream>
#include "NiawgWaiter.h"
#include "boost/cast.hpp"
#include "VariableSystem.h"
#include <boost/algorithm/string/replace.hpp>
#include <afxsock.h>
#include "SocketWrapper.h"
#include "ExperimentLogger.h"


struct niawgIntensityThreadInput
{
	bool dontActuallyGenerate;
	debugInfo debugInfo;
	mainOptions settings;
	profileSettings profile;
	Communicator* comm;
	NiawgController* niawg;
};


/*
 * This runs the experiment. It calls analyzeNiawgScripts and then procedurally goes through all variable values. It also communicates with the other computer
 * throughout the process.
 * inputParam is the list of all of the relevant parameters to be used during this run of the experiment.
 */
class NiawgAndIntensityManager
{
	public:
		void startThread(niawgIntensityThreadInput* inputParam);
		bool isRunning();
		void waitUntilDone();
		void pauseExperiment();
		void unpauseExperiment();

	private:
		static unsigned __stdcall niawgIntensityProgrammingThread( LPVOID inputParam );
		bool threadRunningStatus;
};

/*
// extra thread that programs niawg waveforms, started and watched by main experiment thread.

// variation loop
// wait until prev niawg finished
// start programming next niawg
// start variation
// finish variation

*/