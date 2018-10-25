#pragma once
#include "Expression.h"
#include "Matrix.h"
#include "visa.h"
#include "nifgen.h"
#include "constants.h"
#include "afxwin.h"
#include <string>
#include <vector>
#include <array>


struct niawgWavePower
{
	enum class mode
	{
		// power is held constant.
		constant,
		// power can float beneath cap.
		capped,
		// no restrictions or modifications.
		unrestricted
	};
	static const std::array<mode, 4> allModes;
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
	static const mode defaultMode;
};


struct niawgLibOption
{
	enum class mode
	{
		// i.e. used if available
		allowed,
		// always calculate the wave
		banned,
		// forces use of library wave, throws if can't. Mostly used for testing.
		forced		
	};
	static const std::array<mode, 4> allModes;
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
	static const mode defaultMode;
};



struct niawgWaveCalcOptions
{
	niawgWavePower::mode powerOpt = niawgWavePower::defaultMode;
	niawgLibOption::mode libOpt = niawgLibOption::defaultMode;
};


// order here matches the literal channel number on the 5451. Vertical is actually channel0 and Horizontal is actually 
// channel1. putting the enum in the struct here is a trick that makes you have to use the Axes:: scope but allows 
// automatic int conversion unlike enum class, which is useful for this.
struct Axes
{
	enum type { Vertical = 0, Horizontal = 1 };
};
// used to pair together info for each channel of the niawg in an easy, iterable way.
template<typename type> using niawgPair = std::array<type, 2>;

/* * * *
* Niawg Data structure objects in increasing order of complexity. I.e. waveSignals make up channelWaves which make up...
* */

struct waveSignalForm
{
	Expression freqInit;
	Expression freqFin;
	std::string freqRampType="";

	Expression initPower;
	Expression finPower;
	std::string powerRampType="";

	Expression initPhase;
};;


/* * * * *
* A "Signal" structure contains all of the information for a single signal. Vectors of these are included in a 
* "waveInfo" structure.
* */
struct waveSignal
{
	double freqInit;
	double freqFin;
	std::string freqRampType;

	double initPower;
	double finPower;
	std::string powerRampType;

	double initPhase;
	// Asssigned only after a waveform is calculated or read.
	double finPhase;
};;


struct channelWaveForm
{
	std::vector<waveSignalForm> signals;
	// should be 0 until this option is re-implemented!
	int phaseOption = 0;
	int initType = 0;
	// This should usually just be a single char, but if it's wrong I can use the whole string to debug.
	std::string delim;
	// the actual waveform data.
	std::vector<ViReal64> wave;
};;


/* * * * * * *
* info for a single channel's output.
* note that this structure does not contain any information about the time of a waveform because these always come with pairs in a waveInfo
* struct, and I think it's better to just have a single copy of the time in the waveInfo struct rather than two (potentially conflicting)
* copies in each channel.
* */
struct channelWave
{
	std::vector<waveSignal> signals;
	// should be 0 until this option is re-implemented!
	int phaseOption = 0;
	int initType = 0;
	// This should usually just be a single char, but if it's wrong I can use the whole string to debug.
	std::string delim;
	// the actual waveform data.
	std::vector<ViReal64> wave;
};;


struct simpleWaveForm
{
	niawgPair<channelWaveForm> chan;
	Expression time;
	// whether the whole thing varies. This is true if any expression varies in the wave.
	bool varies = false;
	std::string name="";
};;


// used for flashing, rearrangement waves, etc.
struct simpleWave
{
	niawgPair<channelWave> chan;
	double time = 0;
	long int sampleNum ( )
	{
		double waveSize = time * NIAWG_SAMPLE_RATE;
		// round to an integer.
		return (long) ( waveSize + 0.5 );
		// return waveformSizeCalc ( time );
	}
	//long int sampleNum = 0;
	bool varies = false;
	std::string name;
	std::vector<ViReal64> waveVals;
};;


struct flashInfoForm
{
	bool isFlashing = false;
	niawgPair<Expression> flashCycleFreqInput;
	niawgPair<Expression> totalTimeInput;
	niawgPair<Expression> deadTimeInput;
	Expression flashCycleFreqInputSingle, totalTimeInputSingle, deadTimeInputSingle;

	std::vector<simpleWaveForm> flashWaves;

	Expression flashCycleFreq;
	Expression deadTime;
	UINT flashNumber = 0;
};;


struct flashInfo
{
	bool isFlashing = false;
	std::vector<simpleWave> flashWaves;
	double deadTime = 0;
	double flashCycleFreq = 0;
	UINT flashNumber = 0;
};;

// rerng = rearrange
struct rerngScriptInfoForm
{
	bool isRearrangement = false;
	Expression timePerMove;
	Expression flashingFreq;
	simpleWaveForm staticWave;
	simpleWaveForm fillerWave;
	// the target picture
	Matrix<bool> target = Matrix<bool>(0,0);
	// the maixmum number of moves the rearrangement should take.
	UINT moveLimit = 0;
	// the location that the array will be moved to at the end.
	niawgPair<ULONG> finalPosition = { 0,0 };
	// these are the frequencies that the niawg would need to output to reach the lower left corner (I think?) of 
	// the picture.
	niawgPair<double> lowestFreqs = { 0,0 };
	niawgPair<std::vector<double>> staticPhases;
	niawgPair<std::vector<double>> staticBiases;
	// this is the frequency difference per pixel
	double freqPerPixel = 0;
};;

// rerng = rearrange
struct rerngScriptInfo
{
	bool isRearrangement = false;
	// the target picture
	Matrix<bool> target = Matrix<bool>( 0, 0 );
	niawgPair<ULONG> finalPosition = { 0,0 };
	// hard-coded currently. Should probably add some control for this.
	double timePerMove = 60e-6;
	double flashingFreq = 1e6;
	// the maixmum number of moves the rearrangement should take.
	UINT moveLimit = 0;
	// these are the frequencies that the niawg would need to output to reach the lower left corner (I think?) of 
	// the picture.
	niawgPair<double> lowestFreqs = { 0,0 };
	// this is the frequency difference per pixel
	double freqPerPixel = 0;
	niawgPair<std::vector<double>> staticPhases;
	niawgPair<std::vector<double>> staticBiases;
	// the wave that gets flashed with the moving tweezer
	simpleWave staticWave;
	simpleWave fillerWave;
};;


struct waveInfoForm
{
	simpleWaveForm core;
	flashInfoForm flash;
	rerngScriptInfoForm rearrange;
	bool isStreamed = false;
};;


// contains all info for a waveform on the niawg; i.e. info for both channels, special options, time, and waveform data.
struct waveInfo
{
	simpleWave core;
	flashInfo flash;
	rerngScriptInfo rearrange;
	bool isStreamed = false;
};;


struct NiawgOutput
{
	bool isDefault=false;
	std::string niawgLanguageScript;
	// information directly out of the 
	std::vector<waveInfoForm> waveFormInfo;
	std::vector<waveInfo> waves;
};;
