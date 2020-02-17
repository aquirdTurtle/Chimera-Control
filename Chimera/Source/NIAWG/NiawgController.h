// created by Mark O. Brown
#pragma once

#include "ExperimentThread/Communicator.h"

#include "MiscellaneousExperimentOptions/debugInfo.h"
#include "ConfigurationSystems/profileSettings.h"
#include "NIAWG/NiawgStructures.h"
#include "NIAWG/Rearrangement/rerngOptionStructures.h"
#include "directions.h"
#include "NIAWG/Rearrangement/rerngMoveStructures.h"
#include "NIAWG/Rearrangement/rerngThreadInput.h"
#include "GeneralObjects/Matrix.h"
#include "Scripts/ScriptStream.h"
#include "LowLevel/constants.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "DigitalOutput/DioRows.h"

#include "Fgen.h"
#include <algorithm>
#include <memory>
#include <cmath>
#include <mutex>
#include <chrono>
#include <vector>
#include <array>

#include <boost/container/vector.hpp>

struct ExperimentThreadInput;
struct seqInfo;
class NiawgWaiter;

/** 
  * One of the biggest & most complicated classes in the code.
  * Part of this class is effectively an FGEN wrapper. You could extract that if you have other devies which use fgen.
  */
class NiawgController
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		NiawgController& operator=(const NiawgController&) = delete;
		NiawgController (const NiawgController&) = delete;

		NiawgController( DioRows::which trigRow, UINT trigNumber, bool safemode );
		void initialize();
		void cleanupNiawg( profileSettings profile, bool masterWasRunning, NiawgOutput& output, Communicator& comm, 
						   bool dontGenerate );
		void initForExperiment ( );
		bool rerngThreadIsActive();
		std::string getCurrentScript();
		bool niawgIsRunning();
		void handleStartingRerng( ExperimentThreadInput* input, NiawgOutput& output );
		bool outputVaries( NiawgOutput output );
		void checkThatWaveformsAreSensible( std::string& warnings, NiawgOutput& output );		
		void handleVariations( NiawgOutput& output, std::vector<std::vector<parameterType>>& variables, UINT variation, 
							   std::vector<long>& mixedWaveSizes, std::string& warnings, debugInfo& debugOptions, 
							   UINT totalVariations, rerngGuiOptionsForm& rerngGuiForm, rerngGuiOptions& rerngGui );
		void analyzeNiawgScript( ScriptStream& script, NiawgOutput& output, profileSettings profile,
								 debugInfo& options, std::string& warnings, rerngGuiOptionsForm rInfo,
								 std::vector<parameterType>& variables );
		void flashVaries( waveInfoForm& wave );
		///
		void rerngScriptInfoFormToOutput( waveInfoForm& waveForm, waveInfo& wave, std::vector<parameterType>& varibles,
								UINT variation );

		void writeStaticNiawg( NiawgOutput& output, debugInfo& options, std::vector<parameterType>& variables,
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
		void setDefaultWaveforms( MainWindow* mainWin );
		//static long waveformSizeCalc( double time );
		static double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		// programming the device
		void restartDefault();
		void turnOffRerng( );
		void waitForRerng( bool andClearWvfm );
		void programVariations( UINT variation, std::vector<long>& variedMixedSize, NiawgOutput& output );
		void programNiawg( ExperimentThreadInput* input, NiawgOutput& output, std::string& warnings, UINT variation, 
						   UINT totalVariations, std::vector<long>& variedMixedSize, 
						   std::vector<ViChar>& userScriptSubmit, rerngGuiOptionsForm& rerngGuiForm, 
						   rerngGuiOptions& rerngGui, std::vector<std::vector<parameterType>>& expParams);
		void setDefaultWaveformScript( );
		void turnOff();
		void turnOn();
		// Other
		void setRunningState( bool newRunningState );
		std::pair<DioRows::which, UINT> getTrigLines( );
		UINT getNumberTrigsInScript( );
		bool isOn( );
		void streamWaveform( );
		FgenFlume fgenConduit;
		static void smartTargettingRearrangement ( Matrix<bool> source, Matrix<bool> target, niawgPair<ULONG>& finTargetPos,
												   niawgPair<ULONG> finalPos, std::vector<simpleMove> &operationsMatrix,
												   rerngGuiOptions options, bool randomize = true,
												   bool orderMovesByProximityToTarget = false );
		// From the single moves operationsmatrix, this function calculates parallel moves (rows and columns)
		static void optimizeMoves( std::vector<simpleMove> singleMoves, Matrix<bool> source,
								   std::vector<complexMove> &flashMoves, rerngGuiOptions options );
		void finalizeScript ( ULONGLONG repetitions, std::string name, std::vector<std::string> workingUserScripts,
							  std::vector<ViChar>& userScriptSubmit, bool repeatForever );

		static void generateWaveform ( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time,
									   std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4>& waveLibrary,
									   niawgWaveCalcOptions calcOpts = niawgWaveCalcOptions ( ) );
		static niawgPair<std::vector<UINT>> findLazyPosition ( Matrix<bool> source, UINT targetDim, Communicator& comm );
		static int increment ( std::vector<UINT>& ind, UINT currentLevel, UINT maxVal, bool reversed=false );
	private:
		void preWriteRerngWaveforms( rerngThreadInput* input );
		static void writeToFile( std::vector<double> waveVals );
		///
		void rerngGuiOptionsFormToFinal( rerngGuiOptionsForm& form, rerngGuiOptions& data, std::vector<parameterType>& variables,
									     UINT variation );
		///

		void mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio );
		std::vector<double> calcFinalPositionMove( niawgPair<ULONG> targetPos, niawgPair<ULONG> finalPos, 
												   double freqSpacing, Matrix<bool> target, 
												   niawgPair<double> cornerFreqs, double moveTime );
		void streamRerng( );
		waveInfoForm toWaveInfoForm( simpleWaveForm wave );
		void simpleFormVaries( simpleWaveForm& wave );
		void simpleFormToOutput( simpleWaveForm& formWave, simpleWave& wave,
								 std::vector<parameterType>& varibles = std::vector<parameterType>( ), UINT variation = -1 );
		void flashFormToOutput( waveInfoForm& waveForm, waveInfo& wave,
								std::vector<parameterType>& varibles = std::vector<parameterType>( ),
								UINT variation = -1 );
		void deleteRerngWave( );
		void startRerngThread( atomQueue* atomQueue, waveInfo& wave, Communicator& comm,
							   std::mutex* rerngLock, chronoTimes* andorImageTimes, chronoTimes* grabTimes,
							   std::condition_variable* rerngConditionWatcher, rerngGuiOptions guiInfo, atomGrid grid );
		static niawgPair<ULONG> convolve( Matrix<bool> atoms, Matrix<bool> target );
		void writeStandardWave( simpleWave& wave, debugInfo& options, bool isDefault, 
								niawgLibOption::mode libOption = niawgLibOption::defaultMode);
		void writeFlashing( waveInfo& wave, debugInfo& options, UINT variation );
		void generateWaveform ( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time,
								niawgWaveCalcOptions calcOpts = niawgWaveCalcOptions() );
		void mixWaveforms( simpleWave& waveCore, bool writeThisToFile );
		static void calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, 
								  double time, niawgWavePower::mode powerMode = niawgWavePower::defaultMode );

		void handleMinus1Phase( simpleWave& waveCore, simpleWave& prevWave );
		void createFlashingWave( waveInfo& wave, debugInfo& options );
		void loadStandardInputFormType( std::string inputType, channelWaveForm &wvInfo );
		void openWaveformFiles( );
		bool isLogic( std::string command );
		void handleLogic( ScriptStream& script, std::string inputs, std::string &scriptString );
		bool isSpecialWaveform( std::string command );
		void handleSpecialWaveform( NiawgOutput& output, profileSettings profile, std::string cmd, 
									ScriptStream& scripts, debugInfo& options, rerngGuiOptionsForm guiInfo,
									std::vector<parameterType>& variables, std::vector<vectorizedNiawgVals>& vectorizedVals );
		bool isStandardWaveform( std::string command );
		bool isSpecialCommand( std::string command );
		void handleSpecial( ScriptStream& scripts, NiawgOutput& output, std::string inputTypes, 
									  profileSettings profile, debugInfo& options, std::string& warnings );
		void finalizeStandardWave( simpleWave& wave, debugInfo& options, niawgWaveCalcOptions calcOpts = niawgWaveCalcOptions ( ) );
		/// member variables
 		// Important. This can change if you change computers.
 		const ViRsrc NI_5451_LOCATION = "Dev1";
 		niawgPair<std::string> currentScripts;
		std::string currentScript;
		bool runningState;
		bool on;
		// don't take the word "library" too seriously... it's just a listing of all of the waveforms that have been 
		// already created.
		std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4> waveLibrary;
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
		const DioRows::which triggerRow;
		const UINT triggerNumber;
		/// Rearrangement stuff
		std::vector<rerngContainer<double>> moveBiasCalibrations;
		std::string rerngWaveName = "rearrangeWaveform";
		std::vector<double> rerngWaveVals;
		HANDLE rerngThreadHandle;
		static UINT __stdcall rerngThreadProcedure( LPVOID input );
		UINT triggersInScript=0;
		// true = active;
		std::atomic<bool> threadStateSignal;
		std::vector<std::vector<bool>> finalState;
		// could set different thresholds for each location in the camera if expect imbalance.
		int threshold;
		simpleWave makeRerngWaveMovePart ( rerngScriptInfo& rerngSettings, double moveBias, UINT sourceRows, 
										   UINT sourceCols, complexMove moveInfo );
		std::vector<double> makeFullRerngWave( rerngScriptInfo& info, double staticMovingRatio, double moveBias, double deadTime, 
										   UINT sourceRows, UINT sourceCols, complexMove move );
		std::vector<double> makeFastRerngWave( rerngScriptInfo& rerngSettings, UINT sourceRows, UINT sourceCols, 
											   complexMove moveInfo, rerngGuiOptions options, double moveBias );
		// returns sign of x.
		static int sign( int );
		// returns cost, which is total travel distance. Algorithm from: 
		// http://cs.stanford.edu/group/acm/SLPC/notebook.pdf
		// You have to give it the cost matrix, and to empty vectors, in which it will write
		static double minCostMatching( Matrix<double> cost, std::vector<int> & Lmate, std::vector<int> & Rmate );
		// returns a list of single elementary (left,right,up,down) moves. Size is 4 x n_moves: Initialx,Initialy,Finalx,Finaly
		static double rearrangement( Matrix<bool> & sourceMatrix, Matrix<bool> & targetMatrix,
									 std::vector<simpleMove>& moveSequence, bool randomize=true );
		static void randomizeMoves(std::vector<simpleMove>& operationsList);
		static void orderMoves ( std::vector<simpleMove> operationsList, std::vector<simpleMove>& moveSequence,
								 Matrix<bool> sourceMatrix );
		static void NiawgController::calculateMoveDistancesToTarget ( std::vector<simpleMove> &moveList, niawgPair<double> comPos );
		static niawgPair<double> calculateTargetCOM ( Matrix<bool> target, niawgPair<ULONG> finalPos);
		static Matrix<bool> calculateFinalTarget ( Matrix<bool> target, niawgPair<ULONG> finalPos, UINT rows, UINT cols );
		static void sortByDistanceToTarget ( std::vector<simpleMove> &moveList );


		std::vector<std::string> evolveSource( Matrix<bool> source, std::vector<complexMove> flashMoves );
		// returns maximal number of moves given a targetmatrix.
		static UINT getMaxMoves( Matrix<bool> targetMatrix );

};


