#pragma once

#include <ExperimentThread/ExperimentThreadInput.h>
#include <MiscellaneousExperimentOptions/debugInfo.h>
#include <qobject.h>
#include <string>
#include <memory>

struct baslerSettings;
struct ExpRuntimeData {
	unsigned repetitions = 1;
	mainOptions mainOpts;
	ScriptStream masterScript;
	std::vector<parameterType> expParams;
};


class ExpThreadWorker : public QObject {
    Q_OBJECT

    public:
		static constexpr auto OSCILLOSCOPE_TRIGGER = "C11";

        ExpThreadWorker (ExperimentThreadInput* input_, std::atomic<bool>& expRunning);
        ~ExpThreadWorker ();
		void pause ();
		void unPause ();
		bool getIsPaused ();
		void abort ();
		void loadMasterScript (std::string scriptAddress, ScriptStream& script);
		static void loadNiawgScript (std::string scriptAddress, ScriptStream& niawgScript);
		static void loadAgilentScript (std::string scriptAddress, ScriptStream& agilentScript);
		void checkTriggerNumbers (std::vector<parameterType>& expParams);
		void analyzeMasterScript (DoCore& ttls, AoSystem& aoSys, std::vector<parameterType>& vars,
			ScriptStream& currentMasterScript, bool expectsLoadSkip,
			std::string& warnings, timeType& operationTime,
			timeType& loadSkipTime);
		void waitForAndorFinish ();
		// this function needs the mastewindow in order to gather the relevant parameters for the experiment.
		void startExperimentThread (ExperimentThreadInput* input, IChimeraQtWindow* parent);
		bool runningStatus ();
		bool isValidWord (std::string word);
		bool getAbortStatus ();
		const std::string abortString = "\r\nABORTED!\r\n";
		bool handleTimeCommands (std::string word, ScriptStream& stream, std::vector<parameterType>& params,
			std::string scope, timeType& operationTime);
		bool handleDoCommands (std::string word, ScriptStream& stream, std::vector<parameterType>& params,
			DoCore& ttls, std::string scope, timeType& operationTime);
		bool handleAoCommands (std::string word, ScriptStream& stream, std::vector<parameterType>& params,
			AoSystem& aoSys, DoCore& ttls, std::string scope,
			timeType& operationTime);
		bool handleFunctionCall (std::string word, ScriptStream& stream, std::vector<parameterType>& params,
			DoCore& ttls, AoSystem& aoSys, std::string& warnings, std::string callingFunction, timeType& operationTime);
		static bool handleVariableDeclaration (std::string word, ScriptStream& stream, std::vector<parameterType>& params,
			std::string scope, std::string& warnings);
		static bool handleVectorizedValsDeclaration (std::string word, ScriptStream& stream,
			std::vector<vectorizedNiawgVals>& constVecs, std::string& warnings);
		void experimentThreadProcedure ();
		void analyzeFunctionDefinition (std::string defLine, std::string& functionName, std::vector<std::string>& args);
		static unsigned determineVariationNumber (std::vector<parameterType> vars);
		void handleDebugPlots (DoCore& ttls, AoSystem& aoSys, unsigned variation);
		double convertToTime (timeType time, std::vector<parameterType> variables, unsigned variation);
		void calculateAdoVariations (ExpRuntimeData& runtime);
		static std::vector<parameterType> getLocalParameters (ScriptStream& stream);
		void runConsistencyChecks (std::vector<parameterType> expParams);
		void handlePause (std::atomic<bool>& isPaused, std::atomic<bool>& isAborting);
		void initVariation (unsigned variationInc,
			std::vector<parameterType> expParams);
		void normalFinish (ExperimentType& expType, bool runMaster,
			std::chrono::time_point<chronoClock> startTime);
		void errorFinish (std::atomic<bool>& isAborting, ChimeraError& exception,
			std::chrono::time_point<chronoClock> startTime);
		void startRep (unsigned repInc, unsigned variationInc, bool skip);
		//std::string abortString;
		void loadExperimentRuntime (ConfigStream& config, ExpRuntimeData& runtime);

		/* IDeviceCore functionality wrappers */
		void deviceLoadExpSettings (IDeviceCore& device, ConfigStream& cStream);
		void deviceProgramVariation (IDeviceCore& device, std::vector<parameterType>& expParams, unsigned variationInc);
		void deviceCalculateVariations (IDeviceCore& device, std::vector<parameterType>& expParams);
		void deviceNormalFinish (IDeviceCore& device);
		
		// I've forgotten why there are two of these. 
		timeType loadSkipTime;
		std::vector<double> loadSkipTimes;
		void callCppCodeFunction ();
		// the master script file contents get dumped into this.
		const std::string functionsFolderLocation = FUNCTIONS_FOLDER_LOCATION;
		// called by analyzeMasterScript functions only.
		void analyzeFunction (std::string function, std::vector<std::string> args, DoCore& ttls, AoSystem& aoSys,
			std::vector<parameterType>& vars, std::string& warnings, timeType& operationTime, std::string callingScope);
		timeType operationTime;
		std::atomic<bool> isPaused = false;
		std::atomic<bool> isAborting = false;
		std::atomic<bool>& experimentIsRunning;
		void notify(statusMsg msg);

    public Q_SLOTS:
        void process ();
    Q_SIGNALS:
        void updateBoxColor (QString, QString);
        void notification (statusMsg msg);
        void warn (statusMsg msg);
        void repUpdate (unsigned int);
        void prepareAndor (AndorRunSettings*, analysisSettings);
        void prepareBasler (baslerSettings* settings);
        void plot_Xvals_determined (std::vector<double>);
        void doAoData (const std::vector<std::vector<plotDataVec>>& doData,
                       const std::vector<std::vector<plotDataVec>>& aoData);
        void normalExperimentFinish (QString, profileSettings);
		void calibrationFinish (QString, profileSettings);
        void errorExperimentFinish (QString, profileSettings);
		void expParamsSet (std::vector<parameterType> expParams);
        void mainProcessFinish (); 

    private:
        // add your variables here
		std::unique_ptr< ExperimentThreadInput > input;
};

