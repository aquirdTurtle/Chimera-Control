#pragma once

#include "externals.h"
#include "constants.h"
#include "miscellaneousCommonFunctions.h"
#include "ScriptStream.h"
#include "ProfileSystem.h"
#include "DebugOptionsControl.h"
#include "Communicator.h"
#include "NiawgStructures.h"
#include "rearrangementStructures.h"
#include "Fgen.h"
#include "profileSettings.h"
#include "directions.h"
#include <algorithm>
#include <memory>
#include <cmath>
#include <mutex>
#include <chrono>
#include "rearrangeParams.h"
#include "rearrangementThreadInput.h"

struct MasterThreadInput;
class NiawgWaiter;

/**
 * One of the biggest & most complicated classes in the code.
 * Part of this class is effectively an FGEN wrapper. You could extract that if you have other devies which use fgen.
 */
class NiawgController
{
	public:
		NiawgController( UINT trigRow, UINT trigNumber );
		void initialize();
		void cleanupNiawg( profileSettings profile, bool masterWasRunning,
							  niawgPair<std::vector<std::fstream>>& niawgFiles, NiawgOutput& output,
							  Communicator* comm, bool dontGenerate );
		void preWriteRearrangementWaveforms( rearrangementThreadInput* input );
		void writeToFile( std::vector<double> waveVals );
		bool rearrangementThreadIsActive();
		std::string getCurrentScript();
		bool niawgIsRunning();
		// analysis & numerics
		void simpleFormVaries( simpleWaveForm& wave );
		void simpleFormToOutput( simpleWaveForm& formWave, simpleWave& wave, 
								 std::vector<variableType>& varibles=std::vector<variableType>(), UINT variation=-1 );
		void finalizeStandardWave( simpleWave& wave, debugInfo& options );
		void writeStandardWave( simpleWave& wave, debugInfo options, bool isDefault );
		void writeFlashing( waveInfo& wave, debugInfo& options, UINT variation );
		void generateWaveform( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time );
		void mixWaveforms( simpleWave& waveCore, bool writeThisToFile );
		void calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, 
							  double time );
		void handleStartingRearrangement( MasterThreadInput* input, NiawgOutput& output );
		void prepareNiawg( MasterThreadInput* input, NiawgOutput& output, 
							  niawgPair<std::vector<std::fstream>>& niawgFiles, std::string& warnings, 
							  std::vector<ViChar>& userScriptSubmit, bool& foundRearrangement, rearrangeParams rInfo,
							  std::vector<variableType>& variables );
		bool outputVaries( NiawgOutput output );
		void checkThatWaveformsAreSensible( std::string& warnings, NiawgOutput& output );
		void createFlashingWave( waveInfo& wave, debugInfo options );
		void mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio );
		void handleVariations( NiawgOutput& output, std::vector<variableType>& variables, UINT variation, 
								  std::vector<long>& mixedWaveSizes, std::string& warnings, debugInfo& debugOptions, 
								  UINT totalVariations );
		void analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, NiawgOutput& output, profileSettings profile, 
								  debugInfo& options, std::string& warnings, rearrangeParams rInfo, 
								  std::vector<variableType>& variables );
		void flashFormToOutput( waveInfoForm& waveForm, waveInfo& wave, 
								std::vector<variableType>& varibles = std::vector<variableType>( ), 
								UINT variation = -1 );
		void flashVaries( waveInfoForm& wave );
		void rearrangeFormToOutput( waveInfoForm& waveForm, waveInfo& wave, std::vector<variableType>& varibles,
									UINT variation );
		void writeStaticNiawg( NiawgOutput& output, debugInfo& options, std::vector<variableType>& variables );
		void loadWaveformParametersForm( NiawgOutput& output, profileSettings profile, 
										 niawgPair<std::string> command, debugInfo& debug,
										 niawgPair<ScriptStream>& scripts, std::vector<variableType> variables );
		void finalizeScript( ULONGLONG repetitions, std::string name, std::vector<std::string> workingUserScripts,
							 std::vector<ViChar>& userScriptSubmit, bool repeatForever );
		void preProgramRearrangingMoves( );
		void setDefaultWaveforms( MainWindow* mainWin );
		void deleteRearrangementWave( );
		static long waveformSizeCalc( double time );
		static double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		template <typename type> static void loadParam( type& dataToAssign, ScriptStream& scriptName, UINT& varCount,
														std::vector<std::string>& varNames, std::vector<long> &varParamTypes,
														std::vector<long> dataTypes );
		// programming the device
		void restartDefault();
		void turnOffRearranger( );
		void waitForRearranger( );
		void programVariations( UINT variation, std::vector<long>& variedMixedSize, NiawgOutput& output );
		void programNiawg( MasterThreadInput* input, NiawgOutput& output, std::string& warnings, UINT variation, 
							  UINT totalVariations, std::vector<long>& variedMixedSize, 
							  std::vector<ViChar>& userScriptSubmit );
		void streamWaveform();
		void streamRearrangement();
		void setDefaultWaveformScript( );
		void turnOff();
		void turnOn();
		// Other
		void setRunningState( bool newRunningState );
		void startRearrangementThread( std::vector<std::vector<bool>>* atomQueue, waveInfo wave, Communicator* comm,
										  std::mutex* rearrangerLock, chronoTimes* andorImageTimes, chronoTimes* grabTimes,
										  std::condition_variable* rearrangerConditionWatcher,
										  rearrangeParams rearrangeInfo );
		std::pair<UINT, UINT> getTrigLines( );
		UINT getNumberTrigsInScript( );
		
		waveInfoForm toWaveInfoForm( simpleWaveForm wave );

		bool isOn( );

		std::vector<std::vector<long>> convolve( std::vector<std::vector<bool>> atoms, 
												 std::vector<std::vector<bool>> target );
		FgenFlume fgenConduit;
	private:
		UINT writeToFileNumber = 0;
		void loadStandardInputFormType( std::string inputType, channelWaveForm &wvInfo );
		void openWaveformFiles( );
		bool isLogic( std::string command );
		void handleLogic( niawgPair<ScriptStream>& script, niawgPair<std::string> inputs, std::string &scriptString );
		bool isSpecialWaveform( std::string command );
		void handleSpecialWaveformForm( NiawgOutput& output, profileSettings profile, niawgPair<std::string> command,
										niawgPair<ScriptStream>& scripts, debugInfo& options, rearrangeParams rInfo, 
										std::vector<variableType>& variables );
		bool isStandardWaveform( std::string command );
		void handleStandardWaveformForm( NiawgOutput& output, profileSettings profile, niawgPair<std::string> command,
										 niawgPair<ScriptStream>& scripts, debugInfo& options, 
										 std::vector<variableType>& variables );
		bool isSpecialCommand( std::string command );
		void handleSpecialForm( niawgPair<ScriptStream>& scripts, NiawgOutput& output, niawgPair<std::string> inputTypes,
							profileSettings profile, debugInfo& options, std::string& warnings );
		/// member variables
 		// Important. This can change if you change computers.
 		const ViRsrc NI_5451_LOCATION = "Dev6";
 		niawgPair<std::string> currentScripts;		
		bool runningState;
		bool on;
		// don't take the word "library" too seriously... it's just a listing of all of the waveforms that have been 
		// already created.
		std::array<std::vector<std::string>, MAX_NIAWG_SIGNALS * 4> waveLibrary;
		//
		ViInt32 streamWaveHandle;
		std::string streamWaveName = "streamedWaveform";
		std::string rearrangeWaveName = "rearrangeWaveform";
		std::vector<double> streamWaveformVals;
		std::vector<double> rearrangeWaveVals;
		// pair is of horizontal and vertical configurations.
		std::vector<ViReal64> defaultMixedWaveform;
		std::string defaultWaveName;
		std::vector<ViChar> defaultScript;
		ViSession sessionHandle;

		// not used at the moment, but could revive the phase correction feature back later.
		double calculateCorrectionTime( channelWave& wvData1, channelWave& wvData2, std::vector<double> startPhases, 
										std::string order, double time, long sampleNum);
		/// Rearrangement stuff
		HANDLE rearrangerThreadHandle;
		static UINT __stdcall rearrangerThreadProcedure( LPVOID input );
		void calculateRearrangingMoves( std::vector<std::vector<bool>> initArrangement );
		
		const UINT triggerRow;
		const UINT triggerNumber;
		UINT triggersInScript;
		// true = active;
		std::atomic<bool> threadStateSignal;
		std::vector<std::vector<bool>> finalState;
		// could set different thresholds for each location in the camera if expect imbalance.
		int threshold;
		void rearrange();
		std::vector<double> makeRearrangementWave( rearrangeInfo& info, UINT row, UINT col, directions direction,
												   double staticMovingRatio, double moveBias, double deadTime );
		// returns sign of x.
		static int sign( int );
		// returns cost, which is total travel distance. Algorithm from: 
		// http://cs.stanford.edu/group/acm/SLPC/notebook.pdf
		// You have to give it the cost matrix, and to empty vectors, in which it will write
		static double minCostMatching( const std::vector<std::vector<double>> & cost, std::vector<int> & Lmate,
									   std::vector<int> & Rmate );
		// returns a list of single elementary (left,right,up,down) moves. Size is 4 x n_moves: Initialx,Initialy,Finalx,Finaly
		static double rearrangement( const std::vector<std::vector<bool>> &sourceMatrix,
									 const std::vector<std::vector<bool>> &targetMatrix,
									 std::vector<simpleMove> &operationsMatrix );
		// From the single moves operationsmatrix, this function calculates parallel moves (rows and columns)
		static double parallelMoves( std::vector<std::vector<int>> operationsMatrix, std::vector<std::vector<int>> source,
									 double matrixSize, std::vector<parallelMovesContainer> &moves );
		// returns maximal number of moves given a targetmatrix.
		static UINT getMaxMoves( const std::vector<std::vector<bool>> targetMatrix );
};


template <typename type> static void NiawgController::loadParam( type& dataToAssign, ScriptStream& file, UINT& varCount,
																 std::vector<std::string>& varNames, std::vector<long> &varParamTypes,
																 std::vector<long> dataTypes )
{
	std::string tempInput;
	file >> tempInput;
	if (tempInput.size() > 0)
	{
		if (tempInput[0] == '\'' || tempInput[0] == '#' || tempInput[0] == '}' || tempInput[0] == '{')
		{
			thrower( "ERROR: Detected reserved character out of place inside niawg script string \"" + tempInput + "\"" );
		}
	}
	type val;
	std::stringstream stream;
	stream << tempInput;
	stream >> val;
	// check if it was able to convert text to type correctly.
	if (stream.fail())
	{
		// load variable name into structure.
		for (auto type : dataTypes)
		{
			varNames.push_back( tempInput );
			varParamTypes.push_back( type );
			varCount++;
		}
	}
	else
	{
		// this should happen most of the time.
		dataToAssign = val;
	}
}

