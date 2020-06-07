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

struct ExpRuntimeData
{
	UINT repetitions = 1;
	mainOptions mainOpts;
	ScriptStream masterScript;
	std::vector<parameterType> expParams;
};

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
		static void checkTriggerNumbers ( std::unique_ptr<ExperimentThreadInput>& input, 
										  std::vector<parameterType>& expParams);
		static void analyzeMasterScript( DoCore& ttls, AoSystem& aoSys, std::vector<parameterType>& vars, 
										 ScriptStream& currentMasterScript, bool expectsLoadSkip,
										 std::string& warnings, timeType& operationTime, 
										 timeType& loadSkipTime);

		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		void startExperimentThread(ExperimentThreadInput* input, IChimeraWindowWidget* parent);
		bool runningStatus();
		bool isValidWord(std::string word);
		bool getAbortStatus();
		static bool handleTimeCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
										std::string scope, timeType& operationTime);
		static bool handleDoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
									   DoCore& ttls, std::string scope, timeType& operationTime);
		static bool handleAoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
									  AoSystem& aoSys, DoCore& ttls, std::string scope, 
									  timeType& operationTime);
		static bool handleFunctionCall( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
										DoCore& ttls, AoSystem& aoSys, 
										std::string& warnings, std::string callingFunction, timeType& operationTime);
		static void updatePlotX_vals (std::unique_ptr<ExperimentThreadInput>& input, std::vector<parameterType>& expParams);
		static bool handleVariableDeclaration( std::string word, ScriptStream& stream, std::vector<parameterType>& params,
											   std::string scope, std::string& warnings );
		static bool handleVectorizedValsDeclaration ( std::string word, ScriptStream& stream, 
												std::vector<vectorizedNiawgVals>& constVecs, std::string& warnings );
		static unsigned int __stdcall experimentThreadProcedure(void* voidInput);
		static void analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static UINT determineVariationNumber(std::vector<parameterType> vars);
		static void handleDebugPlots( debugInfo debugOptions, ExpThreadWorker* worker, DoCore& ttls, AoSystem& aoSys,
									  unsigned variation);
		static double convertToTime( timeType time, std::vector<parameterType> variables, UINT variation );
		static void calculateAdoVariations (std::unique_ptr<ExperimentThreadInput>& input, ExpRuntimeData& runtime);
		static std::vector<parameterType> getLocalParameters (ScriptStream& stream);
		static void runConsistencyChecks (std::unique_ptr<ExperimentThreadInput>& input, std::vector<parameterType> expParams );
		static void handlePause (Communicator& comm, std::atomic<bool>& isPaused, std::atomic<bool>& isAborting, ExpThreadWorker* worker);
		static void initVariation ( std::unique_ptr<ExperimentThreadInput>& input, UINT variationInc, 
									std::vector<parameterType> expParams );
		static void normalFinish ( Communicator& comm, ExperimentType& expType, bool runMaster,
								   std::chrono::time_point<chronoClock> startTime, AoSystem& aoSys, ExpThreadWorker* worker);
		static void errorFinish ( Communicator& comm, std::atomic<bool>& isAborting, Error& exception,
								  std::chrono::time_point<chronoClock> startTime, ExpThreadWorker* worker);
		static void startRep (std::unique_ptr<ExperimentThreadInput>& input, UINT repInc, UINT variationInc, bool skip);
		static std::string abortString; 
		static void loadExperimentRuntime ( ConfigStream& config, ExpRuntimeData& runtime, 
											std::unique_ptr<ExperimentThreadInput>& input );
		
		/* IDeviceCore functionality wrappers */
		static void deviceLoadExpSettings (IDeviceCore& device, std::unique_ptr<ExperimentThreadInput>& input, ConfigStream& cStream);
		static void deviceProgramVariation ( IDeviceCore& device, std::unique_ptr<ExperimentThreadInput>& input,
									         std::vector<parameterType>& expParams, UINT variationInc);
		static void deviceCalculateVariations (IDeviceCore& device, std::unique_ptr<ExperimentThreadInput>& input,
											   std::vector<parameterType>& expParams);
		static void deviceNormalFinish (IDeviceCore& device, std::unique_ptr<ExperimentThreadInput>& input);
		
	//private:
		// I've forgotten why there are two of these. 
		timeType loadSkipTime;
		std::vector<double> loadSkipTimes;
		static void callCppCodeFunction();
		// the master script file contents get dumped into this.
		const std::string functionsFolderLocation = FUNCTIONS_FOLDER_LOCATION;
		// called by analyzeMasterScript functions only.
		static void analyzeFunction( std::string function, std::vector<std::string> args, DoCore& ttls, AoSystem& aoSys,
									 std::vector<parameterType>& vars, std::string& warnings,
									 timeType& operationTime, std::string callingScope);
		timeType operationTime;
		HANDLE runningThread;
		std::atomic<bool> experimentIsRunning = false;
		std::atomic<bool> isPaused = false;
		std::atomic<bool> isAborting = false;
};