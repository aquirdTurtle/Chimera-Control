#pragma once

#include "DioSystem.h"
#include "AoSystem.h"
#include "RhodeSchwarz.h"
#include "DebugOptionsControl.h"
#include "ScriptStream.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "EmbeddedPythonHandler.h"
#include "MasterThreadInput.h"
#include "Communicator.h"
#include "VariableStructures.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <mutex>

class MasterManager;

class MasterManager
{
	public:
		MasterManager();
		void pause();
		void unPause();
		bool getIsPaused();
		void abort();
		void loadMasterScript(std::string scriptAddress, ScriptStream& script );
		void analyzeMasterScript( DioSystem* ttls, AoSystem* aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades, 
								  std::vector<UINT>& dacShades, RhodeSchwarz* rsg, std::vector<variableType>& vars, 
								  ScriptStream& currentMasterScript, UINT seqNum, bool expectsLoadSkip );

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		void startExperimentThread(MasterThreadInput* input);
		void loadMotSettings(MasterThreadInput* input);
		bool runningStatus();
		bool isValidWord(std::string word);
		bool getAbortStatus();
		bool handleTimeCommands( std::string word, ScriptStream& stream, std::vector<variableType>& vars );
		bool handleDioCommands( std::string word, ScriptStream& stream, std::vector<variableType>& vars,
								DioSystem* ttls, std::vector<std::pair<UINT, UINT>>& ttlShades, UINT seqNum );
		bool handleAoCommands( std::string word, ScriptStream& stream, std::vector<variableType>& vars,
							   AoSystem* aoSys, std::vector<UINT>& dacShades, DioSystem* ttls, UINT seqNum );

		static unsigned int __stdcall experimentThreadProcedure(void* voidInput);
		static void expUpdate(std::string text, Communicator* comm, bool quiet = false);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static UINT determineVariationNumber(std::vector<variableType> vars);
		static void handleDebugPlots( debugInfo debugOptions, Communicator* comm, DioSystem* ttls, AoSystem* aoSys,
									  bool quiet, EmbeddedPythonHandler* python, 
									  std::vector<std::vector<pPlotDataVec>> ttlData,
									  std::vector<std::vector<pPlotDataVec>> dacData );
		static double convertToTime( timeType time, std::vector<variableType> variables, UINT variation );
	private:
		std::vector<timeType> loadSkipTime;
		std::vector<std::vector<double>> loadSkipTimes;
		void callCppCodeFunction();
		// the master script file contents get dumped into this.
		std::string functionsFolderLocation;
		// called by analyzeMasterScript functions only.
		void analyzeFunction( std::string function, std::vector<std::string> args, DioSystem* ttls, AoSystem* aoSys,
							  std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
							  RhodeSchwarz* rsg, std::vector<variableType>& vars, UINT seqNum );
		timeType operationTime;
		bool experimentIsRunning = false;
		/// task handles
		HANDLE runningThread;
		// Important, these should only be written to by the pause and aborting functions...
		std::mutex pauseLock;
		bool isPaused = false;
		std::mutex abortLock;
		bool isAborting = false;
};


struct indvSeqElem
{
	std::string config;
	std::fstream niawgScript;
	std::string masterScript;
	ScriptStream masterStream;
	std::string agilentScript;
};


struct seqInfo
{
	seqInfo( UINT seqSize ) : sequence(seqSize)
	{}
	std::string seqName;
	std::vector<indvSeqElem> sequence;
};

