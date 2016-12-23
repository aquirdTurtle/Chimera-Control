#pragma once
#include "stdafx.h"
#include <vector>
#include "waveformData.h"
/*
 * This namespace includes functions that I've programmed that essentially just do mathematical calculations. Most of these are highly specialized and revolve 
 * around NIAWG algorithms, but I'm keeping the namespace general because not all of them are.
 *
 * struct minMaxDoublet;
 *
 * This namespace includes the functions:
 *		double calculateCorrectionTime(waveData& wvData1, waveData& wvData2, std::vector<double> startPhases, std::string order);
 *		double rampCalc(int size, int iteration, double initPos, double finPos, std::string rampType);
 */
namespace myMath
{
	/**
	 *
	 */
	struct minMaxDoublet
	{
		double min;
		double max;
	};
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
	double calculateCorrectionTime(waveData& wvData1, waveData& wvData2, std::vector<double> startPhases, std::string order);
	double rampCalc(int size, int iteration, double initPos, double finPos, std::string rampType);
};
