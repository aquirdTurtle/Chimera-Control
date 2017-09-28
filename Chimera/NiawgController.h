#pragma once

#include "externals.h"
#include "constants.h"
#include "miscellaneousCommonFunctions.h"
#include "ScriptStream.h"
#include "ProfileSystem.h"
#include "DebuggingOptionsControl.h"
#include "Communicator.h"
#include "NiawgStructures.h"
#include "rearrangementStructures.h"
#include "Fgen.h"
#include "profileSettings.h"
#include <algorithm>
#include <memory>
#include <cmath>
#include <mutex>
#include <chrono>
#include "rearrangeParams.h"

struct MasterThreadInput;
class NiawgWaiter;

/**
 * One of the biggest & most complicated classes in the code.
 * Part of this class is effectively an FGEN wrapper. You could extract that if you have other devies which use fgen.
 */
class NiawgController
{
	public:
		void initialize();
		// get info
		void cleanupNiawg( profileSettings profile, bool masterWasRunning, 
						   niawgPair<std::vector<std::fstream>>& niawgFiles, NiawgOutputInfo& output, 
						   Communicator* comm, bool dontGenerate );
		void writeToFile( UINT fileNum, std::vector<double> waveVals );
		bool rearrangementThreadIsActive();
		std::string getCurrentScript();
		bool niawgIsRunning();
		// analysis & numerics
		void handleVariations( NiawgOutputInfo& output, key varKey, const UINT variation, std::vector<long>& mixedWaveSizes,
							   std::string& warnings, debugInfo& debugOptions, UINT totalVariations );
		void analyzeNiawgScripts( niawgPair<ScriptStream>& scripts, NiawgOutputInfo& output, profileSettings profile,
								  debugInfo& options, std::string& warnings, rearrangeParams rInfo );
		void loadWaveformParameters( NiawgOutputInfo& output, profileSettings profile, niawgPair<std::string> command,
									 debugInfo& debug, niawgPair<ScriptStream>& scripts );
		void finalizeScript( ULONGLONG repetitions, std::string name, std::vector<std::string> workingUserScripts,
							 std::vector<ViChar>& userScriptSubmit, bool repeatForever );
		void setDefaultWaveforms( MainWindow* mainWin );
		void varyParam( simpleWave& wave, waveInfo previousWave, int axis, int paramNum, double paramVal, 
						std::string& warnings );
		void deleteRearrangementWave( );
		void checkThatWaveformsAreSensible( std::string& warnings, NiawgOutputInfo& output );
		void prepareNiawg( MasterThreadInput* input, NiawgOutputInfo& output,
						   niawgPair<std::vector<std::fstream>>& niawgFiles, std::string& warnings,
						   std::vector<ViChar>& userScriptSubmit, bool& foundRearrangement,
						   rearrangeParams rInfo );
		void finalizeStandardWave( simpleWave& wave, debugInfo& options );
		void createFlashingWave( waveInfo& wave, debugInfo options );
		void mixFlashingWaves( waveInfo& wave, double deadTime, double staticMovingRatio );
		long waveformSizeCalc( double time );
		static double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );
		template <typename type> static void loadParam( type& dataToAssign, ScriptStream& scriptName, UINT& varCount,
														std::vector<std::string>& varNames, std::vector<long> &varParamTypes,
														std::vector<long> dataTypes );
		// programming the device
		void restartDefault();
		void turnOffRearranger( );
		void waitForRearranger( );
		void programVariations( UINT variation, std::vector<long>& variedMixedSize, NiawgOutputInfo& output );
		
		void programNiawg( MasterThreadInput* input, NiawgOutputInfo& output, std::string& warnings, UINT variation, 
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
		FgenFlume fgenConduit;
		static bool outputVaries(NiawgOutputInfo output);

	private:
		void mixWaveforms( simpleWave& waveCore );
		void calcWaveData( channelWave& inputData, std::vector<ViReal64>& readData, long int sampleNum, double time );
		void loadStandardInputType( std::string inputType, channelWave &wvInfo );
		void openWaveformFiles( );
		void generateWaveform( channelWave & waveInfo, debugInfo& options, long int sampleNum, double time );		
		//
		bool isLogic( std::string command );
		void handleLogic( niawgPair<ScriptStream>& script, niawgPair<std::string> inputs, std::string &scriptString );
		bool isSpecialWaveform( std::string command );
		void handleSpecialWaveform( NiawgOutputInfo& output, profileSettings profile, niawgPair<std::string> command,
									niawgPair<ScriptStream>& scripts, debugInfo& options, rearrangeParams rInfo );
		bool isStandardWaveform( std::string command );
		void handleStandardWaveform( NiawgOutputInfo& output, profileSettings profile, niawgPair<std::string> command,
									 niawgPair<ScriptStream>& scripts, debugInfo& options );
		bool isSpecialCommand( std::string command );
		void handleSpecial( niawgPair<ScriptStream>& script, NiawgOutputInfo& output, niawgPair<std::string> inputTypes,
							profileSettings profile, debugInfo& options, std::string& warnings );
		//
		
		waveInfo toWaveInfo( simpleWave wave );

		/// member variables
		// Important. This can change if you change computers.
		const ViRsrc NI_5451_LOCATION = "Dev6";
		niawgPair<std::string> currentScripts;
		bool runningState;
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

		// not used at the moment		
		double calculateCorrectionTime( channelWave& wvData1, channelWave& wvData2, std::vector<double> startPhases, 
										std::string order, double time, long sampleNum);
		/// Rearrangement stuff
		HANDLE rearrangerThreadHandle;
		static UINT __stdcall rearrangerThreadProcedure( LPVOID input );
		void calculateRearrangingMoves( std::vector<std::vector<bool>> initArrangement );
		// true = active;
		std::atomic<bool> threadStateSignal;
		std::vector<std::vector<bool>> finalState;
		// could set different thresholds for each location in the camera if expect imbalance.
		int threshold;
		void rearrange();

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

