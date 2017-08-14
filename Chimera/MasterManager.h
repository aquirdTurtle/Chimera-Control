#pragma once

#include <string>

#include "nidaqmx2.h"
#include <sstream>
#include "TtlSystem.h"
#include "DacSystem.h"
#include <vector>
#include "VariableSystem.h"
#include "RichEditControl.h"
#include "KeyHandler.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include "DebuggingOptionsControl.h"
#include <mutex>
#include "ScriptStream.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "StatusControl.h"
#include "Repetitions.h"
#include "TektronicsControl.h"

class MasterManager;

struct MasterThreadInput
{

	profileSettings profile;
	TtlSystem* ttls;											
	DacSystem* dacs;
	UINT repetitionNumber;
	std::vector<variable> vars;
	MasterManager* thisObj;
	KeyHandler* key;
	std::string masterScriptAddress;
	Communicator* comm;
	Gpib* gpib;
	RhodeSchwarz* rsg;
	debugInfo debugOptions;
	std::vector<Agilent*> agilents;
	TektronicsControl* tektronics1;
	TektronicsControl* tektronics2;
	VariableSystem* globalControl;
	NiawgController* niawg;
	bool dontActuallyGenerate;
	bool quiet;
	bool programIntensity;
	bool runNiawg;
	bool runMaster;
};

struct niawgCalcInput
{

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

		void analyzeMasterScript( TtlSystem* ttls, DacSystem* dacs, 
								  std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
								  RhodeSchwarz* rsg, std::vector<variable>& vars);

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		void startExperimentThread(MasterThreadInput* input);
		void loadMotSettings(MasterThreadInput* input);
		void loadVariables(std::vector<variable> newVariables);
		bool runningStatus();
		bool isValidWord(std::string word);
		
		bool getAbortStatus();

		static UINT __cdecl experimentThreadProcedure(void* voidInput);
		static void expUpdate(std::string text, Communicator* comm, bool quiet = false);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);

		static UINT __cdecl niawgCalcThread(void* voidInput);

		static UINT determineVariationNumber(std::vector<variable> vars, key tempKey);

	private:
		void callCppCodeFunction();
		// the master script file contents get dumped into this.
		std::string currentFunctionText;
		std::string currentMasterScriptText;
		ScriptStream currentMasterScript;
		std::string functionsFolderLocation;
		// called by analyzeMasterScript functions only.
		void analyzeFunction( std::string function, std::vector<std::string> args, TtlSystem* ttls, DacSystem* dacs,
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
