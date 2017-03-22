#pragma once
#include "stdafx.h"
#include <vector>

struct channelWave;

/*
 * This namespace includes functions that I've programmed that essentially just do mathematical calculations. Most of these are highly specialized and revolve 
 * around NIAWG algorithms, but I'm keeping the namespace general because not all of them are.
 */
namespace myMath
{
	struct minMaxDoublet
	{
		double min;
		double max;
	};
	double calculateCorrectionTime( channelWave& wvData1, channelWave& wvData2, std::vector<double> startPhases, 
									std::string order, double time, long int sampleNum );
	double rampCalc(int size, int iteration, double initPos, double finPos, std::string rampType);
};
