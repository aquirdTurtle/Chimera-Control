// created by Mark O. Brown
#pragma once

#include "MiscellaneousExperimentOptions/debugInfo.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ConfigurationSystems/Version.h"
#include "GeneralObjects/Matrix.h"
#include "Scripts/ScriptStream.h"
#include "LowLevel/constants.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "DigitalOutput/DoRows.h"
#include "GeneralObjects/IDeviceCore.h"

#include "directions.h"
#include "NiawgStructures.h"
#include "Rearrangement/rerngOptionStructures.h"
#include "Rearrangement/rerngMoveStructures.h"
#include "Rearrangement/rerngThreadInput.h"
#include <GeneralObjects/commonTypes.h>
#include "NiawgConstants.h"

#include "Fgen.h"

#include <boost/container/vector.hpp>

#include <algorithm>
#include <memory>
#include <cmath>
#include <mutex>
#include <chrono>
#include <vector>
#include <array>


struct ExperimentThreadInput;
struct seqInfo;
/** 
  * One of the biggest & most complicated classes in the code.
  * Part of this class is effectively an FGEN wrapper. You could extract that if you have other devies which use fgen.
  */
class NiawgCore : public IDeviceCore {
	public:
		// THIS CLASS IS NOT COPYABLE.
		NiawgCore& operator=(const NiawgCore&) = delete;
		NiawgCore (const NiawgCore&) = delete;

		NiawgCore( DoRows::which trigRow, unsigned trigNumber, bool safemode );
		void initialize();
		void cleanupNiawg(  );
		void initForExperiment ( );
		bool rerngThreadIsActive();
		std::string getCurrentScript();
		bool niawgIsRunning();
		std::string getDelim () { return configDelim; }
		void handleStartingRerng( ExperimentThreadInput* input );
		bool outputVaries( NiawgOutput output );
		void checkThatWaveformsAreSensible( std::string& warnings, NiawgOutput& output );		
		void handleVariations( NiawgOutput& output, std::vector<parameterType>& variables, unsigned variation, 
							   std::vector<long>& mixedWaveSizes, std::string& warnings, rerngGuiOptions& rerngGuiForm );
		void analyzeNiawgScript ( NiawgOutput& output, std::string& warnings, rerngGuiOptions rerngGuiInfo,
								  std::vector<parameterType>& variables);
		void flashVaries( waveInfoForm& wave );
		///
		bool getSettingsFromConfig (ConfigStream& openfile);
		void rerngScriptInfoFormToOutput( waveInfoForm& waveForm, waveInfo& wave, std::vector<parameterType>& varibles,
								unsigned variation );
		std::string configDelim = "NIAWG_INFORMATION";
		void writeStaticNiawg( NiawgOutput& output, std::vector<parameterType>& variables,
							   bool deleteWaveAfterWrite=true, niawgLibOption::mode libOption = niawgLibOption::defaultMode );
		void deleteWaveData( simpleWave& core );
		bool isVectorizedCmd ( std::string cmd );
		void loadCommonWaveParams( ScriptStream& script, simpleWaveForm& wave );
		void handleStandardWaveform( NiawgOutput& output, std::string cmd, ScriptStream& script,
									 std::vector<parameterType>& variables,
									 std::vector<vectorizedNiawgVals>& vectorizedVals );
		void loadWaveformParametersFormSingle( NiawgOutput& output, std::string cmd, ScriptStream& script,
											   std::vector<parameterType>& variables, int axis, simpleWaveForm& wave,
											   std::vector<vectorizedNiawgVals>& vectorizedVals );
		void readTraditionalSimpleWaveParams ( ScriptStream& script,
											   std::vector<parameterType>& variables, int axis,
											   simpleWaveForm& wave );
		void readVectorizedSimpleWaveParams ( ScriptStream& script, std::vector<vectorizedNiawgVals>& constVecs,
											  int axis, simpleWaveForm& wave, std::vector<parameterType>& parameters );
		void assertAllValid ( waveSignalForm& signal, std::vector<parameterType>& parameters );
		void loadFullWave( NiawgOutput& output, std::string cmd, ScriptStream& script, 
						   std::vector<parameterType>& variables, simpleWaveForm& wave,
						   std::vector<vectorizedNiawgVals>& vectorizedVals );
		void setDefaultWaveforms( );
		//static long waveformSizeCalc( double time );
		static double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		// programming the device
		void restartDefault();
		std::string getOutputSummary (const NiawgOutput& output);
		void turnOffRerng( );
		void waitForRerng( bool andClearWvfm );
		void programVariations( unsigned variation, std::vector<long>& variedMixedSize, NiawgOutput& output );
		void programNiawg( std::string& warnings, unsigned variation, rerngGuiOptions& rerngGuiForm, 
						   std::vector<parameterType>& expParams );
		void setDefaultWaveformScript( );
		void turnOff();
		void turnOn();
		// Other
		void setRunningState( bool newRunningState );
		std::pair<DoRows::which, unsigned> getTrigLines( );
		unsigned getNumberTrigsInScript( );
		bool isOn( );
		void streamWaveform( );
		FgenFlume fgenFlume;
		static void smartTargettingRearrangement ( Matrix<bool> source, Matrix<bool> target, niawgPair<unsigned long>& finTargetPos,
												   niawgPair<unsigned long> finalPos, std::vector<simpleMove> &operationsMatrix,
												   rerngGuiOptions options, bool randomize = true,
												   bool orderMovesByProximityToTarget = false );
		// From the single moves operationsmatrix, this function calculates parallel moves (rows and columns)
		static void optimizeMoves( std::vector<simpleMove> singleMoves, Matrix<bool> source,
								   std::vector<complexMove> &flashMoves, rerngGuiOptions options );
		void finalizeScript ( unsigned __int64 repetitions, std::string name, std::string workingUserScripts,
							  std::vector<ViChar>& userScriptSubmit, bool repeatForever );

		void generateWaveform ( channelWave & waveInfo, long int sampleNum, double time,
									   std::array<std::vector<std::string>, NiawgConstants::MAX_NIAWG_SIGNALS * 4>& waveLibrary,
									   niawgWaveCalcOptions calcOpts = niawgWaveCalcOptions ( ) );
		static niawgPair<std::vector<unsigned>> findLazyPosition ( Matrix<bool> source, unsigned targetDim );
		static int increment ( std::vector<unsigned>& ind, unsigned currentLevel, unsigned maxVal, bool reversed=false );

		bool outputNiawgMachineScript = false;
		bool outputNiawgHumanScript = false;
		bool outputWriteTimes = false;
		bool outputReadTimes = false;
		bool outputAsText = false;
		unsigned expRepetitions = 0;
		rerngGuiOptions expRerngOptions;
		std::vector<ViChar> niawgMachineScript;
		std::string debugMsg = "";
		NiawgOutput expOutput;
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void programVariation (unsigned varInc, std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		void logSettings (DataLogger& log, ExpThreadWorker* threadworker);
		void loadExpSettings (ConfigStream& stream);
		void normalFinish ();
		void errorFinish ();

	private:
		/// a couple points taken on Friday, Sep 11th 2020, fairly low power coming from the Ti-Saph at the moment.
		/// 1.64: ~71 mW
		/// 0.82: ~9 mW
		/// 0.123 ~31 mW
		/// ORIGINAL NIAWG GAIN: 1.64
		const float NIAWG_GAIN = 1.64f;
		// NEW GAIN: 1.23f
		// const float NIAWG_GAIN = 1.23f;

		// We calibrated this. // NIAWG_GAIN = 1.64.
		//constexpr float NIAWG_GAIN = float(1.64);
		// NIAWG_GAIN = 1.64.
		
		// This is the minimum time (in seconds) that a correction waveform will be allowed to have. Without this, the algorithm might decide that the 1/2 of a period 
		// of a waveform might be enough, even though this would probably be far below the amount of time an individual waveform is allowed to have according to the 
		// NIAWG (the NIAWG has a minimum waveform size).
		// Initial Value: 0.00000025 (250 ns)
		const double MIN_CORRECTION_WAVEFORM_TIME = 0.00000025;
		// this is a time in seconds.
		const double MAX_CORRECTION_WAVEFORM_TIME = 0.001;
		// this is a phase in radians.
		const double CORRECTION_WAVEFORM_GOAL = 0.001;
		// this is a phase in radians.
		const double CORRECTION_WAVEFORM_ERROR_THRESHOLD = 0.5;
		// in hertz
		const double NIAWG_FILTER_FREQENCY = 80000000;
		const unsigned NUMBER_OF_LIBRARY_FILES = NiawgConstants::MAX_NIAWG_SIGNALS * 4;
		/// September 15th, 2017: Changed value from 0.2 to 0.1 in preparation of doing 10 traps, also attempting to lower
		/// going to aom for debugging purposes on the Rearrangement algorithms.
		/// April 26th, 2018" Changed from 0.1 to 1/12 to do 12 traps. 
		// if doing less than 12 traps, effectively this results in the niawg's dac being underutilized. Generally this 
		// is fine. 
		/// IF CHANGE MAKE SURE TO CHANGE LIBRARY FILE ADDRESS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// this was picked because this is the maximum power that can be outputted with 12 signals 
		// given the voltage restrictions.
		const double TOTAL_POWER = (1.0 / 12.0) - 1e-6; //0.1 - 1e-10;

		void preWriteRerngWaveforms( rerngThreadInput* input );
		static void writeToFile( std::vector<double> waveVals );
		void rerngGuiOptionsFormToFinal( rerngGuiOptions& form, std::vector<parameterType>& variables, unsigned var );

		void mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio );
		std::vector<double> calcFinalPositionMove( niawgPair<unsigned long> targetPos, niawgPair<unsigned long> finalPos, 
												   double freqSpacing, Matrix<bool> target, 
												   niawgPair<double> cornerFreqs, double moveTime );
		void streamRerng( );
		waveInfoForm toWaveInfoForm( simpleWaveForm wave );
		void simpleFormVaries( simpleWaveForm& wave );
		void simpleFormToOutput( simpleWaveForm& formWave, simpleWave& wave,
								 std::vector<parameterType>& varibles = std::vector<parameterType>( ), unsigned variation = -1 );
		void flashFormToOutput( waveInfoForm& waveForm, waveInfo& wave,
								std::vector<parameterType>& varibles = std::vector<parameterType>( ),
								unsigned variation = -1 );
		void deleteRerngWave( );
		void startRerngThread( atomQueue* atomQueue, waveInfo& wave, 
							   std::mutex* rerngLock, chronoTimes* andorImageTimes, chronoTimes* grabTimes,
							   std::condition_variable* rerngConditionWatcher, rerngGuiOptions guiInfo, atomGrid grid );
		static niawgPair<unsigned long> convolve( Matrix<bool> atoms, Matrix<bool> target );
		void writeStandardWave( simpleWave& wave, bool isDefault, 
								niawgLibOption::mode libOption = niawgLibOption::defaultMode);
		void writeFlashing( waveInfo& wave, unsigned variation );
		void generateWaveform ( channelWave & waveInfo, long int sampleNum, double time,
								niawgWaveCalcOptions calcOpts = niawgWaveCalcOptions() );
		void mixWaveforms( simpleWave& waveCore, bool writeThisToFile );
		void calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, 
						   double time, niawgWavePower::mode powerMode = niawgWavePower::defaultMode );
		void handleMinus1Phase( simpleWave& waveCore, simpleWave& prevWave );
		void createFlashingWave( waveInfo& wave);
		void loadStandardInputFormType( std::string inputType, channelWaveForm &wvInfo );
		void openWaveformFiles( );
		bool isLogic( std::string command );
		void handleLogic( ScriptStream& script, std::string inputs, std::string &scriptString );
		bool isSpecialWaveform( std::string command );
		void handleSpecialWaveform( NiawgOutput& output, std::string cmd, ScriptStream& scripts, 
									rerngGuiOptions guiInfo, std::vector<parameterType>& variables, 
									std::vector<vectorizedNiawgVals>& vectorizedVals );
		bool isStandardWaveform( std::string command );
		bool isSpecialCommand( std::string command );
		void handleSpecial (ScriptStream& script, NiawgOutput& output, std::string cmd);
		void finalizeStandardWave( simpleWave& wave, niawgWaveCalcOptions calcOpts = niawgWaveCalcOptions ( ) );
		/// member variables
 		// Important. This can change if you change computers.
 		const ViRsrc NI_5451_LOCATION = "Dev1";
 		niawgPair<std::string> currentScripts;
		std::string currentScript;
		bool runningState;
		bool on;
		// don't take the word "library" too seriously... it's just a listing of all of the waveforms that have been 
		// already created.
		std::array<std::vector<std::string>, NiawgConstants::MAX_NIAWG_SIGNALS * 4> waveLibrary;
		//
		ViInt32 streamWaveHandle;
		std::string streamWaveName = "streamedWaveform";
		std::vector<double> streamWaveformVals;
		std::vector<ViReal64> defaultMixedWaveform;
		std::string defaultWaveName;
		std::vector<ViChar> defaultScript;
		ViSession sessionHandle;

		// not used at the moment, but could revive the phase correction feature back later.
		double calculateCorrectionTime( channelWave& wvData1, channelWave& wvData2, std::vector<double> startPhases, 
										std::string order, double time, long sampleNum);
		const DoRows::which triggerRow;
		const unsigned triggerNumber;
		/// Rearrangement stuff
		std::vector<rerngContainer<double>> moveBiasCalibrations;
		std::string rerngWaveName = "rearrangeWaveform";
		std::vector<double> rerngWaveVals;
		HANDLE rerngThreadHandle;
		static unsigned __stdcall rerngThreadProcedure( LPVOID input );
		unsigned triggersInScript=0;
		// true = active;
		std::atomic<bool> threadStateSignal;
		std::vector<std::vector<bool>> finalState;
		// could set different thresholds for each location in the camera if expect imbalance.
		int threshold;
		simpleWave makeRerngWaveMovePart ( rerngScriptInfo& rerngSettings, double moveBias, unsigned sourceRows, 
										   unsigned sourceCols, complexMove moveInfo );
		std::vector<double> makeFullRerngWave( rerngScriptInfo& info, double staticMovingRatio, double moveBias, double deadTime, 
										       unsigned sourceRows, unsigned sourceCols, complexMove move );
		std::vector<double> makeFastRerngWave( rerngScriptInfo& rerngSettings, unsigned sourceRows, unsigned sourceCols, 
											   complexMove moveInfo, rerngGuiOptions options, double moveBias );
		// returns sign of x.
		static int sign( int );
		// returns cost, which is total travel distance. Algorithm from: 
		// http://cs.stanford.edu/group/acm/SLPC/notebook.pdf
		// You have to give it the cost matrix, and to empty vectors, in which it will writebtn
		static double minCostMatching( Matrix<double> cost, std::vector<int> & Lmate, std::vector<int> & Rmate );
		// returns a list of single elementary (left,right,up,down) moves. Size is 4 x n_moves: Initialx,Initialy,Finalx,Finaly
		static double rearrangement( Matrix<bool> & sourceMatrix, Matrix<bool> & targetMatrix,
									 std::vector<simpleMove>& moveSequence, bool randomize=true );
		static void randomizeMoves(std::vector<simpleMove>& operationsList);
		static void orderMoves ( std::vector<simpleMove> operationsList, std::vector<simpleMove>& moveSequence,
								 Matrix<bool> sourceMatrix );
		static void calculateMoveDistancesToTarget ( std::vector<simpleMove> &moveList, niawgPair<double> comPos );
		static niawgPair<double> calculateTargetCOM ( Matrix<bool> target, niawgPair<unsigned long> finalPos);
		static Matrix<bool> calculateFinalTarget ( Matrix<bool> target, niawgPair<unsigned long> finalPos, unsigned rows, unsigned cols );
		static void sortByDistanceToTarget ( std::vector<simpleMove> &moveList );


		std::vector<std::string> evolveSource( Matrix<bool> source, std::vector<complexMove> flashMoves );
		// returns maximal number of moves given a targetmatrix.
		static unsigned getMaxMoves( Matrix<bool> targetMatrix );

		std::string expNiawgScript;
		ScriptStream expNiawgStream;

		//extern std::vector<std::string> WAVEFORM_NAME_FILES;
		//extern std::vector<std::string> WAVEFORM_TYPE_FOLDERS;
		/*/// some globals for niawg stuff, only for niawg stuff so I keep it here because...?
extern const std::array<int, 2> AXES;
// the following is used to receive the index of whatever axis is not your current axis.
extern const std::array<int, 2> ALT_AXES;
extern const std::array<std::string, 2> AXES_NAMES;
extern const niawgPair<std::string> ORIENTATION;
*/
		std::vector<std::string> WAVEFORM_NAME_FILES = std::vector<std::string> (4 * NiawgConstants::MAX_NIAWG_SIGNALS);
		std::vector<std::string> WAVEFORM_TYPE_FOLDERS = std::vector<std::string> (4 * NiawgConstants::MAX_NIAWG_SIGNALS);

};


