#pragma once

#include <string>

#include "nidaqmx2.h"
#include <sstream>
#include "TTL_System.h"
#include "DAC_System.h"
#include <vector>
#include "VariableSystem.h"
#include "RichEditControl.h"
#include "KeyHandler.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include "Debugger.h"
#include <mutex>
#include "ScriptStream.h"
#include "Agilent.h"
#include "commonTypes.h"

struct ExperimentThreadInput
{
	TtlSystem* ttls;
	DacSystem* dacs;
	unsigned int repetitions;
	std::vector<variable> vars;
	ExperimentManager* thisObj;
	RichEditControl* status;
	RichEditControl* error;
	KeyHandler* key;
	bool connectToNIAWG;
	SocketWrapper* niawgSocket;
	std::string masterScriptAddress;
	Gpib* gpibHandler;
	RhodeSchwarz* rsg;
	debuggingOptions debugOptions;
	std::vector<Agilent*> agilents;
	// first = top, second = bottom, third = axial.
	std::array<std::string, 3> ramanFreqs;
};



class ExperimentManager
{
	public:
		ExperimentManager();
		void pause();
		void unPause();
		bool getIsPaused();
		void abort();
		std::string getErrorMessage(int errorCode);
		void loadMasterScript(std::string scriptAddress);

		void analyzeCurrentMasterScript( TtlSystem* ttls, DacSystem* dacs, 
										 std::vector<std::pair<unsigned int, unsigned int>>& ttlShades,
										 std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, 
										 std::array<std::string, 3>& ramanFreqs );

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		void startExperimentThread(MasterWindow* master);
		void loadMotSettings(MasterWindow* master);
		static UINT __cdecl experimentThreadProcedure(LPVOID input);
		void loadVariables(std::vector<variable> newVariables);
		bool runningStatus();
		bool isValidWord(std::string word);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);

	private:
		void callCppCodeFunction();
		// the master script file contents get dumped into this.
		std::string currentFunctionText;
		std::string currentMasterScriptText;
		std::vector<variable> variables;
		ScriptStream currentMasterScript;
		std::string functionsFolderLocation;
		// called by analyzeCurrentMasterScript functions only.
		void analyzeFunction( std::string function, std::vector<std::string> args, TtlSystem* ttls, DacSystem* dacs,
							  std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg,
							  std::array<std::string, 3>& ramanFreqs );

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
