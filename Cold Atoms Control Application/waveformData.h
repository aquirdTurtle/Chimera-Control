#ifndef waveformData_h
#define waveformData_h

#include "stdafx.h"

#include "signal.h"

#include <string>
#include <vector>


/**
 * These structures contain all of the needed input data about the waveform in question. Specifically:
 * std::vector<signal> signals;
 * double time;
 * int phaseManagementOption;
 * int signalNum;
 * int initType;
 * long int sampleNum;
 * bool isVaried;
 * int varNum;
 * std::vector<std::string> varNames;
 * std::vector<int> varTypes;
 * std::string delim;
 */
struct waveData
{
	std::vector<signal> signals;
	double time;
	int phaseManagementOption;

	int signalNum;
	int initType;
	long int sampleNum;
	bool isVaried;
	// variables for dealing with varied waveforms. These only get set when a varied waveform is used, and they serve the purpose of carrying relevant info to
	// the end of the program, when these varried waveforms are compiled.
	int varNum;
	std::vector<std::string> varNames;
	std::vector<int> varTypes;
	// This should usually just be a single char, but if it's wrong I can use the whole string to debug.
	std::string delim;

};
#endif