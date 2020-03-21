// created by Mark O. Brown
#pragma once

#include "DigitalOutput/DoCore.h"
#include "AnalogOutput/AoSystem.h"
#include "Microwave/MicrowaveSettings.h"
#include "MiscellaneousExperimentOptions/debugInfo.h"
#include "Scripts/ScriptStream.h"
#include "Agilent/Agilent.h"
#include "GeneralObjects/commonTypes.h"
#include "ExperimentThreadInput.h"
#include "Communicator.h"
#include "ParameterSystem/ParameterSystemStructures.h"
#include <string>
#include <vector>
#include <mutex>

class ExperimentThreadManager
{
	public:
		ExperimentThreadManager();
		void pause();
		void unPause(); 
		bool getIsPaused();
		void abort();
		void loadMasterScript(std::string scriptAddress, ScriptStream& script );
		static void loadNiawgScript ( std::string scriptAddress, ScriptStream& niawgScript );
		static void loadAgilentScript ( std::string scriptAddress, ScriptStream& agilentScript );
		static void checkTriggerNumbers ( ExperimentThreadInput* input, std::string& warnings,
										  UINT variations, microwaveSettings settings,
										  std::vector<std::vector<parameterType>>& expParams,
										  std::vector<deviceOutputInfo>& agRunInfo, bool runNiawg);
		static void analyzeMasterScript( DoCore& ttls, AoSystem& aoSys, std::vector<parameterType>& vars, 
										 ScriptStream& currentMasterScript, UINT seqNum, bool expectsLoadSkip,
										 std::string& warnings, timeType& operationTime, 
										 std::vector<timeType>& loadSkipTime);

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		HANDLE startExperimentThread(ExperimentThreadInput* input);
		void loadMotSettings(ExperimentThreadInput* input);
		bool runningStatus();
		bool isValidWord(std::string word);
		bool getAbortStatus();
		static bool handleTimeCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
										std::string scope, timeType& operationTime);
		static bool handleDoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
									   DoCore& ttls, UINT seqNum, std::string scope, timeType& operationTime);
		static bool handleAoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
									  AoSystem& aoSys, DoCore& ttls, UINT seqNum, std::string scope, 
									  timeType& operationTime);
		static bool handleFunctionCall( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
										DoCore& ttls, AoSystem& aoSys, UINT seqNum, 
										std::string& warnings, std::string callingFunction, timeType& operationTime);
		static void updatePlotX_vals (ExperimentThreadInput* input, std::vector<std::vector<parameterType>>& expParams);
		static bool handleVariableDeclaration( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
											   std::string scope, std::string& warnings );
		static bool handleVectorizedValsDeclaration ( std::string word, ScriptStream& stream, 
												std::vector<vectorizedNiawgVals>& constVecs, std::string& warnings );
		static unsigned int __stdcall experimentThreadProcedure(void* voidInput);
		static void expUpdate(std::string text, Communicator& comm, bool quiet = false);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static UINT determineVariationNumber(std::vector<parameterType> vars);
		static void handleDebugPlots( debugInfo debugOptions, Communicator& comm, DoCore& ttls, AoSystem& aoSys,
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
		static void analyzeFunction( std::string function, std::vector<std::string> args, DoCore& ttls, AoSystem& aoSys,
									 std::vector<parameterType>& vars, UINT seqNum, std::string& warnings,
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

