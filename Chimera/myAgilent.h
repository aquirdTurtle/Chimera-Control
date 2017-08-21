#pragma once
#include <vector>
#include "Windows.h"
#include "constants.h"
#include "externals.h"
#include "NiawgController.h"
#include "boost/cast.hpp"
#include <algorithm>
#include "ScriptStream.h"

struct profileSettings;

/*
 * This Namespace includes all of my function handling for interacting withe agilent waveform generator. It includes:
 * The Segment Class
 * The IntensityWaveform Class
 * The agilentDefault function
 * The agilentErrorCheck function
 * The selectIntensityProfile function
 */
namespace myAgilent
{

	/*
	 * The agilentDefalut function restores the status of the Agilent to be outputting the default DC level (full depth traps). It returns an error code.
	 */
	void agilentDefault();
	
	/*
	]---- This function is used to analyze a given intensity file. It's used to analyze all of the basic intensity files listed in the sequence of 
	]- configurations, but also recursively to analyze nested intensity scripts.
	*/
	bool analyzeIntensityScript( ScriptStream& intensityFile, IntensityWaveform* intensityWaveformData, 
								 int& currentSegmentNumber, profileSettings profile );
	/*
	 * The programIntensity function reads in the intensity script file, interprets it, creates the segments and sequences, and outputs them to the andor to be
	 * ready for usage. 
	 * 
	 */
	void programIntensity( UINT varNum, std::vector<variable> varNames, std::vector<std::vector<double> > varValues,
						   bool& intensityVaried, std::vector<std::pair<double, double>>& minsAndMaxes, 
						   std::vector<std::fstream>& intensityFiles, std::vector<variable> singletons, 
						   profileSettings profile );

	int agilentErrorCheck(long status, ULONG vi);
	
	void setIntensity(UINT varNum, bool intensityIsVaried, std::vector<std::pair<double, double>> intensityMinMax);

}
