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

class MasterWindow;
class ExperimentManager;

struct ExperimentThreadInput
{
	TTL_System* ttls;
	DAC_System* dacs;
	unsigned int repetitions;
	std::vector<variable> vars;
	ExperimentManager* thisObj;
	RichEditControl* status;
	RichEditControl* error;
	KeyHandler* key;
	bool connectToNIAWG;
	SocketWrapper* niawgSocket;
	std::string masterScriptAddress;
	GPIB* gpibHandler;
	RhodeSchwarz* rsg;
	// first = top, second = bottom, third = axial.
	std::array<std::string, 3> ramanFreqs;
};

class ExperimentManager
{
	public:
		ExperimentManager();
		bool pause();
		bool stop();
		bool clear();
		bool start(TTL_System* ttls, DAC_System* dacs);
		std::string getErrorMessage(int errorCode);
		bool loadMasterScript(std::string scriptAddress);
		bool analyzeCurrentMasterScript(TTL_System* ttls, DAC_System* dacs, std::vector<std::pair<unsigned int, unsigned int>>& ttlShades,
			std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg, std::array<std::string, 3>& ramanFreqs);
		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		bool startExperimentThread(MasterWindow* master);
		static UINT __cdecl experimentThreadProcedure(LPVOID input);
		bool loadVariables(std::vector<variable> newVariables);

		static bool analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static bool eatComments(std::stringstream* stream);

	private:
		// the master script file contents get dumped into this.
		std::vector<variable> variables;
		std::stringstream currentMasterScript;
		std::string functionsFolderLocation;
		// called by analyzeScript functions only.
		bool analyzeFunction(std::string function, std::vector<std::string> args, TTL_System* ttls, DAC_System* dacs,
			std::vector<std::pair<unsigned int, unsigned int>>& ttlShades, std::vector<unsigned int>& dacShades, RhodeSchwarz* rsg,
			std::array<std::string, 3>& ramanFreqs);
		std::pair<std::string, long> operationTime;
		bool experimentIsRunning;
		/// task handles
		CWinThread* runningThread;
};

