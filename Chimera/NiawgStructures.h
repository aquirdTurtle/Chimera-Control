#pragma once
#include <string>
#include <vector>
#include "nifgen.h"

// order here matches the literal channel number on the 5451. Vertical is actually channel0 and Horizontal is actually channel1.
enum AXES { Vertical = 0, Horizontal = 1 };
// used to pair together info for each channel of the niawg in an easy, iterable way.
template<typename type> using niawgPair = std::array<type, 2>;

/* * * *
* Niawg Data structure objects, in increasing order of complexity. I.e. waveSignals make up channelWaves which make up...
* */


/* * * * *
* A "Signal" structure contains all of the information for a single signal. Vectors of these are included in a "waveInfo" structure.
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
};


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
	int phaseOption;
	int initType;
	// variables for dealing with varied waveforms. These only get set when a varied waveform is used, and they serve the purpose of 
	// carrying relevant info to the end of the program, when these varried waveforms are compiled.
	UINT varNum;
	std::vector<std::string> varNames;
	std::vector<long> varTypes;
	// This should usually just be a single char, but if it's wrong I can use the whole string to debug.
	std::string delim;
	// the actual waveform data.
	std::vector<ViReal64> wave;
};

// used for flashing, rearrangement waves, etc.
struct simpleWave
{
	niawgPair<channelWave> chan;
	double time;
	long int sampleNum;
	bool varies;
	std::string name;
	std::vector<ViReal64> waveVals;
};


struct flashInfo
{
	niawgPair<std::string> flashCycleFreqInput;
	niawgPair<std::string> totalTimeInput;
	std::vector<simpleWave> flashWaves;
	double flashCycleFreq;
	UINT flashNumber;
};



struct rearrangeInfo
{
	// the target picture
	std::vector<std::vector<bool>> target;
	// a bit redundant atm.
	UINT targetRows;
	UINT targetCols;
	double timePerStep = 1e-3;
	double flashingFreq = 5e5;
	// the maixmum number of moves the rearrangement should take.
	UINT moveLimit;
	// these are the frequencies that the niawg would need to output to reach the lower left corner (I think?) of 
	// the picture.
	niawgPair<double> lowestFreq;
	// this is the frequency difference per pixel
	double freqPerPixel;
	simpleWave staticWave;
};



// contains all info for a waveform on the niawg; i.e. info for both channels, special options, time, and waveform data.
struct waveInfo
{
	simpleWave core;
	// may or may not be filled... check the "is flashing variable". Don't have a better way of doing this atm...
	flashInfo flash;
	rearrangeInfo rearrange;
	bool isStreamed;
	bool isFlashing;
	bool isRearrangement = false;
};


/* * * * *
* The largest output structure, contains all info for a script to be outputted. Because this contains a lot of info, it gets passed around
* a lot between functions under a name "output".
* */
struct NiawgOutputInfo
{
	// wave <-> waveform
	//int waveCount;
	//int predefinedWaveCount;
	std::vector<int> predefinedWaveLocs;
	bool isDefault;
	std::string niawgLanguageScript;
	// output channel
	std::vector<waveInfo> waves;
	niawgPair<std::vector<std::string>> predefinedWaveNames;
};
