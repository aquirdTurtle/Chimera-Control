#pragma once

#include "nidaqmx2.h"
#include "DioSystem.h"
#include "DacSystem.h"
#include "VariableSystem.h"
#include "RichEditControl.h"
#include "KeyHandler.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include "DebuggingOptionsControl.h"
#include "ScriptStream.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "StatusControl.h"
#include "Repetitions.h"
#include "TektronicsControl.h"
#include "DataLogger.h"
#include "atomCruncherInput.h"
#include "commonTypes.h"
#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include "realTimePlotterInput.h"

class MasterManager;

struct MasterThreadInput
{
	DataLogger* logger;
	profileSettings profile;
	DioSystem* ttls;											
	DacSystem* dacs;
	UINT repetitionNumber;
	std::vector<variable> variables;
	MasterManager* thisObj;
	KeyHandler* key;
	std::string masterScriptAddress;
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
	// only for rearrangement.
	std::mutex* rearrangerLock;
	std::vector<std::vector<bool>>* atomQueueForRearrangement;
	chronoTimes* andorsImageTimes;
	chronoTimes* grabTimes;
	std::condition_variable* conditionVariableForRearrangement;
};


struct ExperimentInput
{
	ExperimentInput::ExperimentInput() : 
		includesCameraRun(false), masterInput(NULL), plotterInput(NULL), cruncherInput(NULL){ }
	MasterThreadInput* masterInput;
	realTimePlotterInput* plotterInput;
	atomCruncherInput* cruncherInput;
	AndorRunSettings camSettings;
	bool includesCameraRun;
};


class MasterManager
{
	public:
		MasterManager();
		void pause();
		void unPause();
		bool getIsPaused();
		void abort();
		std::string getErrorMessage(int errorCode);
		void loadMasterScript(std::string scriptAddress);

		void analyzeMasterScript( DioSystem* ttls, DacSystem* dacs, 
								  std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
								  RhodeSchwarz* rsg, std::vector<variable>& vars);

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		void startExperimentThread(MasterThreadInput* input);
		void loadMotSettings(MasterThreadInput* input);
		void loadVariables(std::vector<variable> newVariables);
		bool runningStatus();
		bool isValidWord(std::string word);
		bool getAbortStatus();
		bool handleTimeCommands( std::string word, ScriptStream& stream, std::vector<variable>& vars );

		static UINT __cdecl experimentThreadProcedure(void* voidInput);
		static void expUpdate(std::string text, Communicator* comm, bool quiet = false);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static UINT determineVariationNumber(std::vector<variable> vars, key tempKey);

	private:
		void callCppCodeFunction();
		// the master script file contents get dumped into this.
		std::string currentFunctionText;
		std::string currentMasterScriptText;
		ScriptStream currentMasterScript;
		std::string functionsFolderLocation;
		// called by analyzeMasterScript functions only.
		void analyzeFunction( std::string function, std::vector<std::string> args, DioSystem* ttls, DacSystem* dacs,
							  std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
							  RhodeSchwarz* rsg, std::vector<variable>& vars);
		// 
		timeType operationTime;
		bool experimentIsRunning;
		/// task handles
		CWinThread* runningThread;

		// Important, these should only be written to by the pause and aborting functions...
		std::mutex pauseLock;
		bool isPaused;
		std::mutex abortLock;
		bool isAborting;
};
