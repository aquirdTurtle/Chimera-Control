// created by Mark O. Brown
#pragma once

#include "DioSystem.h"
#include "AoSystem.h"
#include "MicrowaveSystem.h"
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

class MasterThreadManager;

class MasterThreadManager
{
	public:
		MasterThreadManager();
		void pause();
		void unPause(); 
		bool getIsPaused();
		void abort();
		void loadMasterScript(std::string scriptAddress, ScriptStream& script );
		static void loadNiawgScript ( std::string scriptAddress, ScriptStream& niawgScript );
		static void loadAgilentScript ( std::string scriptAddress, ScriptStream& agilentScript );
		static void checkTriggerNumbers ( ExperimentThreadInput* input, bool useAuxDevices, std::string& warnings,
										  UINT variations );
		static void analyzeMasterScript( DioSystem& ttls, AoSystem& aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades, 
								  std::vector<UINT>& dacShades, MicrowaveCore& rsg, std::vector<parameterType>& vars, 
								  ScriptStream& currentMasterScript, UINT seqNum, bool expectsLoadSkip,
								  std::string& warnings, timeType& operationTime, std::vector<timeType>& loadSkipTime);

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		HANDLE startExperimentThread(ExperimentThreadInput* input);
		void loadMotSettings(ExperimentThreadInput* input);
		bool runningStatus();
		bool isValidWord(std::string word);
		bool getAbortStatus();
		static bool handleTimeCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
										std::string scope, timeType& operationTime);
		static bool handleDioCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
									   DioSystem& ttls, std::vector<std::pair<UINT, UINT>>& ttlShades, UINT seqNum,
									   std::string scope, timeType& operationTime);
		static bool handleAoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
									  AoSystem& aoSys, std::vector<UINT>& dacShades, DioSystem& ttls, UINT seqNum,
									  std::string scope, timeType& operationTime);
		static bool handleFunctionCall( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
										DioSystem& ttls, AoSystem& aoSys, std::vector<std::pair<UINT, UINT>>& ttlShades, 
										std::vector<UINT>& dacShades, MicrowaveCore& rsg, UINT seqNum, std::string& warnings,
										std::string callingFunction, timeType& operationTime);
		static void updatePlotX_vals (ExperimentThreadInput* input );
		static bool handleVariableDeclaration( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
											   std::string scope, std::string& warnings );
		static bool handleVectorizedValsDeclaration ( std::string word, ScriptStream& stream, std::vector<vectorizedNiawgVals>& 
											   constVecs, std::string& warnings );
		static unsigned int __stdcall experimentThreadProcedure(void* voidInput);
		static void expUpdate(std::string text, Communicator& comm, bool quiet = false);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static UINT determineVariationNumber(std::vector<parameterType> vars);
		static void handleDebugPlots( debugInfo debugOptions, Communicator& comm, DioSystem& ttls, AoSystem& aoSys,
									  std::vector<std::vector<pPlotDataVec>> ttlData,
									  std::vector<std::vector<pPlotDataVec>> dacData );
		static double convertToTime( timeType time, std::vector<parameterType> variables, UINT variation );
	private:
		// I've forgotten why there are two of these. 
		std::vector<timeType> loadSkipTime;
		std::vector<std::vector<double>> loadSkipTimes;
		static void callCppCodeFunction();
		// the master script file contents get dumped into this.
		const std::string functionsFolderLocation = FUNCTIONS_FOLDER_LOCATION;
		// called by analyzeMasterScript functions only.
		static void analyzeFunction( std::string function, std::vector<std::string> args, DioSystem& ttls, AoSystem& aoSys,
									 std::vector<std::pair<UINT, UINT>>& ttlShades, std::vector<UINT>& dacShades, 
									 MicrowaveCore& rsg, std::vector<parameterType>& vars, UINT seqNum, std::string& warnings,
									 timeType& operationTime, std::string callingScope);
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
	std::string niawgScript;
	ScriptStream niawgStream;
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

