#include "stdafx.h"
#include "myMath.h"
#include <vector>
#include <string>

#include "waveformData.h"
#include "constants.h"
#include "externals.h"
#include "boost/math/common_factor.hpp"

/*
 * Contains a couple of math functions that I use. Specifically:
 * double rampCalc(int size, int iteration, double initPos, double finPos, std::string rampType)
 * double calculateCorrectionTime(waveData& wvData1, waveData& wvData2, std::vector<double> startPhases, std::string order)
 */
namespace myMath
{
	/**
	* This function takes ramp-related information as an input and returns the "position" in the ramp (i.e. the amount to add to the initial value due to ramping)
	* that the waveform should be at.
	*
	* @return double is the ramp position.
	*
	* @param size is the total size of the waveform, in numbers of samples
	* @param iteration is the sample number that the waveform is currently at.
	* @param initPos is the initial frequency or amplitude of the waveform.
	* @param finPos is the final frequency or amplitude of the waveform.
	* @param rampType is the type of ramp being executed, as specified by the reader.
	*/
	double rampCalc(int size, int iteration, double initPos, double finPos, std::string rampType)
	{
		// for linear ramps
		if (rampType == "lin")
		{
			return iteration * (finPos - initPos) / size;
		}
		// for no ramp
		else if (rampType == "nr")
		{
			return 0;
		}
		// for hyperbolic tangent ramps
		else if (rampType == "tanh")
		{
			return (finPos - initPos) * (tanh(-4 + 8 * (double)iteration / size) + 1) / 2;
		}
		// error message. I've already checked (outside this function) whether the ramp-type is a filename.
		else
		{
			std::string errMsg = "ERROR: ramp type " + rampType + " is unrecognized. If this is a file name, make sure the file exists and is in the project folder.\r\n";
			MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
			return 0;
		}
	}

	/**
	* This function is designed to replace getVartime in function. It uses a numerical, rather than mostly analytical, algorithm for finding a good time for a
	* correction waveform. The algorithm increments through all possible sample numbers for a waveform (multiples of 4 and between the min and max values) and
	* checks whether the phases of the waveforms are near zero to some tolerance.
	* The algorithm:
	* Take input data, create array of frequencies and array of phases.
	* For all possible data points...
	*	For all signals...
	*	  Calculate phases at this point
	*	For all signals...
	*	  Check if phases are all near zero
	*	  Throw flags if bad or terrible matching
	*	If bad flag not thrown, break, a good time has been found.
	*	Else, check if terrible flag thrown
	*	If not, check if this is the best time so far, and if so, set best.
	* If bad flag not thrown, continue happily. Exited well.
	* Else if terrible flag not thrown, report best time and throw warning.
	* Else throw error.
	* Return time.
	*/
	double calculateCorrectionTime(waveData& wvData1, waveData& wvData2, std::vector<double> startPhases, std::string order)
	{
		std::vector<double> freqList;
		for (int signalInc = 0; signalInc < wvData1.signalNum; signalInc++)
		{
			freqList.push_back(wvData1.signals[signalInc].freqInit);
		}
		for (int signalInc = 0; signalInc < wvData2.signalNum; signalInc++)
		{
			freqList.push_back(wvData2.signals[signalInc].freqInit);
		}
		int testSampleNum;
		if (order == "before")
		{
			testSampleNum = 2;
		}
		else
		{
			testSampleNum = 0;
		}
		bool matchIsGood = true;
		bool matchIsOkay = true;
		bool foundOkayMatch = false;
		double bestTime = DBL_MAX;
		int bestSampleNum;
		double totalPhaseMismatch = PI;
		for (; (double)testSampleNum / SAMPLE_RATE < MAX_CORRECTION_WAVEFORM_TIME; testSampleNum += 4)
		{
			if ((double)testSampleNum / SAMPLE_RATE < MIN_CORRECTION_WAVEFORM_TIME)
			{
				continue;
			}
			std::vector<double> currentPhases;
			// calculate phases...
			for (int signalInc = 0; signalInc < freqList.size(); signalInc++)
			{
				// sin{omega*t+phi} = sin{2*PI*frequency*t+phi} = sin{2*PI*frequency*(currentSample / SampleRate) + phi}
				// need to modulate for 2*PI.
				double phase, phaseDif;
				phase = fmod(2 * PI * freqList[signalInc] * ((double)testSampleNum / SAMPLE_RATE) + startPhases[signalInc], 2 * PI);
				if (phase > PI)
				{
					if (phase > 2 * PI)
					{
						// ERROR
						MessageBox(0, "ERROR! Bad location in calculateCorrectionTime.", 0, 0);
					}
					phaseDif = 2 * PI - phase;
				}
				else
				{
					phaseDif = phase;
				}
				currentPhases.push_back(phaseDif);
			}
			matchIsGood = true;
			matchIsOkay = true;
			for (int signalInc = 0; signalInc < freqList.size(); signalInc++)
			{
				if (currentPhases[signalInc] > CORRECTION_WAVEFORM_GOAL)
				{
					matchIsGood = false;
					if (currentPhases[signalInc] > CORRECTION_WAVEFORM_ERROR_THRESHOLD)
					{
						matchIsOkay = false;
					}
				}
			}
			if (matchIsGood)
			{
				bestTime = (double)testSampleNum / SAMPLE_RATE;
				bestSampleNum = testSampleNum;
				break;
			}
			else if (matchIsOkay)
			{
				double testTotalPhaseMismatch = 0;
				for (int signalInc = 0; signalInc < currentPhases.size(); signalInc++)
				{
					testTotalPhaseMismatch += currentPhases[signalInc];
				}
				if ((double)testTotalPhaseMismatch < totalPhaseMismatch)
				{
					foundOkayMatch = true;
					bestTime = testSampleNum / SAMPLE_RATE;
					bestSampleNum = testSampleNum;
					totalPhaseMismatch = testTotalPhaseMismatch;
				}
			}
		}
		if (!matchIsGood)
		{
			if (!foundOkayMatch)
			{
				// throw error
				return -1;
			}
			else
			{
				// set values
				if (order == "before")
				{
					wvData1.time = (bestSampleNum - 2) / SAMPLE_RATE;
					wvData1.sampleNum = bestSampleNum;
					wvData2.time = (bestSampleNum - 2) / SAMPLE_RATE;
					wvData2.sampleNum = bestSampleNum;
				}
				else
				{
					wvData1.time = bestSampleNum / SAMPLE_RATE;
					wvData1.sampleNum = bestSampleNum;
					wvData2.time = bestSampleNum / SAMPLE_RATE;
					wvData2.sampleNum = bestSampleNum;
				}
				// throw warning!
				return totalPhaseMismatch;
			}
		}
		// set values
		if (order == "before")
		{
			wvData1.time = (bestSampleNum - 2) / SAMPLE_RATE;
			wvData1.sampleNum = bestSampleNum;
			wvData2.time = (bestSampleNum - 2) / SAMPLE_RATE;
			wvData2.sampleNum = bestSampleNum;
		}
		else
		{
			wvData1.time = bestSampleNum / SAMPLE_RATE;
			wvData1.sampleNum = bestSampleNum;
			wvData2.time = bestSampleNum / SAMPLE_RATE;
			wvData2.sampleNum = bestSampleNum;
		}
		// if here, a good time must have been found.
		return 0;
	}

};
