#include "stdafx.h"
#include "Agilent.h"
#include "constants.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <fstream>
#include "C:\PROGRAM FILES (X86)\IVI FOUNDATION\VISA\WINNT\INCLUDE\VISA.H"
#include "VariableSystem.h"
#include "ScriptStream.h"

/*
* This Namespace includes all of my function handling for interacting withe agilent waveform generator. It includes:
* The Segment Class
* The IntensityWaveform Class
* The agilentDefault function
* The errCheck function
* The selectIntensityProfile function
*/
IntensityWaveform::IntensityWaveform()
{
	segmentNum = 0;
	totalSequence = "";
	isVaried = false;
};


/*
	* This function reads out a segment of script file and loads it into a segment to be calculated and manipulated.
	* segNum: This tells the function what the next segment # is.
	* scriptName: this is the file object to be read from.
	*/
bool IntensityWaveform::readIntoSegment(int segNum, ScriptStream script, std::vector<variable> singletons, profileSettings profileInfo)
{
	std::string intensityCommand;
	std::vector<std::string> tempVarNames;
	std::vector<int> tempVarLocations;
	int tempVarNum = 0;
	if (script.peek == EOF)
	{
		return true;
	}
	// Grab the command type (e.g. ramp, const). Looks for newline by default.
	intensityCommand = script.getline( '\r' );
	// get rid of case sensitivity.
	if (intensityCommand == "agilent hold")
	{
		waveformSegments.resize( segNum + 1 );
		waveformSegments[segNum].assignSegType( 0 );
	}
	else if (intensityCommand == "agilent ramp")
	{
		waveformSegments.resize(segNum + 1);
		waveformSegments[segNum].assignSegType(1);
	}
	else if (intensityCommand == "predefined script")
	{
		std::string nestedFileName;
		// remove \n at the end of last line.
		script.get();
		nestedFileName = script.getline( '\r' );
		std::string path = profileInfo.pathIncludingCategory + nestedFileName + AGILENT_SCRIPT_EXTENSION;
		std::fstream nestedFile(path.c_str(), std::ios::in);
		if (!nestedFile.is_open())
		{
			thrower("ERROR: tried to open a nested intensity file, but failed! The file was " + profileInfo.pathIncludingCategory
				+ nestedFileName + AGILENT_SCRIPT_EXTENSION);
			return false;
		}

		analyzeIntensityScript( nestedFile, this, segNum, singletons, profileInfo );
	}
	else 
	{
		thrower( "ERROR: Intensity command not recognized. The command was \"" + intensityCommand + "\"" );
		return false;
	}

	double tempTimeInMilliSeconds, tempIntensityInit, tempIntensityFin;
	unsigned int tempRepeatNum = 0;
	std::string delimiter, tempContinuationType, tempRampType;	 
	// List of data types for variables for the different arguments:
	// initial intensity = 1
	// final intensity = 2
	// time = 3
	if (waveformSegments[segNum].returnSegmentType() == 1)
	{
		// this segment type means ramping.
		script >> tempRampType;
		NiawgController::getParamCheckVar(tempIntensityInit, scriptName, tempVarNum, tempVarNames, tempVarLocations, 1, singletons);
		NiawgController::getParamCheckVar(tempIntensityFin, scriptName, tempVarNum, tempVarNames, tempVarLocations, 2, singletons);
	}
	else
	{
		tempRampType = "nr";
		NiawgController::getParamCheckVarConst(tempIntensityInit, tempIntensityFin, scriptName, tempVarNum, tempVarNames, tempVarLocations, 1, 2, singletons);
	}
	NiawgController::getParamCheckVar(tempTimeInMilliSeconds, scriptName, tempVarNum, tempVarNames, tempVarLocations, 3, singletons);

	rmWhite(scriptName);
	scriptName >> tempContinuationType;
	std::transform(tempContinuationType.begin(), tempContinuationType.end(), tempContinuationType.begin(), ::tolower);
	if (tempContinuationType == "repeat")
	{
		// There is an extra input in this case.
		rmWhite(scriptName);
		scriptName >> tempRepeatNum;
	}
	else
	{
		tempRepeatNum = 0;
	}
	rmWhite(scriptName);
	scriptName >> delimiter;
	if (delimiter != "#")
	{
		// input number mismatch.
		std::string errorMsg;
		errorMsg = "ERROR: The delimeter is missing in the Intensity script file for Segment #" + std::to_string(segNum + 1)
					+ ". The value placed in the delimeter location was " + delimiter + " while it should have been '#'.This"
					+ " indicates that either the code is not interpreting the user input incorrectly or that the user has inputted too many parameters for this type"
					+ " of Segment. Use of \"Repeat\" without the number of repeats following will also trigger this error.";
		thrower( errorMsg );
		return;
	}

	if (tempTimeInMilliSeconds <= 0)
	{
		// check if being varied.
		bool varCheck = false;
		for (int varCheckInc = 0; varCheckInc < tempVarLocations.size(); varCheckInc++)
		{
			if (tempVarLocations[varCheckInc] == 3)
			{
				varCheck = true;
				break;
			}
		}
		if (varCheck == false)
		{
			// Invalid time
			std::string errMsg;
			thrower( "ERROR: Invalid time entered in the intensity script file for Segment #" + std::to_string( segNum ) + ". the value entered was "
					 + std::to_string( tempTimeInMilliSeconds ) + "." );
			return;
		}
	}

	if (tempIntensityInit < 0)
	{
		// check if being varied.
		bool varCheck = false;
		for (int varCheckInc = 0; varCheckInc < tempVarLocations.size(); varCheckInc++)
		{
			if (tempVarLocations[varCheckInc] == 1)
			{
				varCheck = true;
			}
		}
		if (varCheck == false)
		{
			// Invalid intensity
			std::string errMsg;
			errMsg = "ERROR: Invalid Initial intensity entered in the intensity script file for Segment #" + std::to_string(segNum) 
						+ ". the value entered was " + std::to_string(tempIntensityInit) + ".";
			thrower(errMsg);
			return;
		}
	}

	if (tempIntensityFin < 0)
	{
		// check if being varied.
		bool varCheck = false;
		for (int varCheckInc = 0; varCheckInc < tempVarLocations.size(); varCheckInc++)
		{
			if (tempVarLocations[varCheckInc] == 2)
			{
				varCheck = true;
			}
		}
		if (varCheck == false)
		{
			std::string errMsg;
			errMsg = "ERROR: Invalid Initial intensity entered in the intensity script file for Segment #" + std::to_string(segNum)	
						+ ". the value entered was " + std::to_string(tempIntensityInit) + ".";
			thrower(errMsg);
			return;
		}
	}
	if (tempContinuationType == "repeat")
	{
		waveformSegments[segNum].assignContinuationType(0);
		if (tempRepeatNum < 0)
		{
			std::string errMsg;
			errMsg = "ERROR: Invalid intensity entered in the intensity script file for Segment #" + std::to_string(segNum) + ". the value entered was " 
						+ std::to_string(tempIntensityInit) + ".";
			thrower(errMsg);
			return;
		}
	}
	else if (tempContinuationType == "repeatuntiltrig")
	{
		waveformSegments[segNum].assignContinuationType(1);
	}
	else if (tempContinuationType == "once")
	{
		waveformSegments[segNum].assignContinuationType(2);
	}
	else if (tempContinuationType == "repeatforever")
	{
		waveformSegments[segNum].assignContinuationType(3);
	}
	else if (tempContinuationType == "oncewaittrig")
	{
		waveformSegments[segNum].assignContinuationType(4);
	}
	else
	{
		// string not recognized
		std::string errMsg;
		errMsg = "ERROR: Invalid Continuation Option on intensity segment #" + std::to_string(segNum + 1) + ". The string entered was " + tempContinuationType
					+ ". Please enter \"Repeat #\", \"RepeatUntilTrigger\", \"OnceWaitTrig\", or \"Once\". Code should not be case-sensititve.";
		thrower( errMsg );
	}
	// Make Everything Permanent
	waveformSegments[segNum].assignRepeatNum(tempRepeatNum);
	waveformSegments[segNum].assignInitValue(tempIntensityInit);
	waveformSegments[segNum].assignFinValue(tempIntensityFin);
	// the actual time that gets assigned is in seconds
	waveformSegments[segNum].assignTime(tempTimeInMilliSeconds / 1000.0);
	waveformSegments[segNum].assignRampType(tempRampType);
	waveformSegments[segNum].assignVarNum(tempVarNum);
	waveformSegments[segNum].assignSegVarNames(tempVarNames);
	waveformSegments[segNum].assignVarLocations(tempVarLocations);

	if (tempVarNum > 0)
	{
		isVaried = true;
	}

	return;
}
/*
	* This function takes the data points (that have already been converted and normalized) and puts them into a string for the agilent to read.
	* segNum: this is the segment number that this data is for
	* varNum: This is the variation number for this segment (matters for naming the segments)
	* totalSegNum: This is the number of segments in the waveform (also matters for naming)
	*/
std::string IntensityWaveform::compileAndReturnDataSendString(int segNum, int varNum, int totalSegNum)
{
	// must get called after data conversion
	std::string tempSendString;
	tempSendString = "DATA:ARB seg" + std::to_string(segNum + totalSegNum * varNum) + ",";
	// need to handle last one separately so that I can /not/ put a comma after it.
	for (int sendDataInc = 0; sendDataInc < waveformSegments[segNum].returnDataSize() - 1; sendDataInc++)
	{
		tempSendString += std::to_string(waveformSegments[segNum].returnDataVal(sendDataInc));
		tempSendString += ", ";
	}
	tempSendString += std::to_string(waveformSegments[segNum].returnDataVal(waveformSegments[segNum].returnDataSize() - 1));
	return tempSendString;
}
/*
	* This function tells a given segment number to calculate all of it's data points. It doesn't do much itself.
	* {//x
	*	return waveformSegments[segNum].calcData();
	* }//x
	*/
void IntensityWaveform::writeData(int segNum)
{
	waveformSegments[segNum].calcData();
}
/*
	* This function compiles the sequence string which tells the agilent what waveforms to output when and with what trigger control. The sequence is stored
	* as a part of the class.
	*/
void IntensityWaveform::compileSequenceString(int totalSegNum, int sequenceNum)
{
	std::string tempSequenceString, tempSegmentInfoString;
	// Total format is  #<n><n digits><sequence name>,<arb name1>,<repeat count1>,<play control1>,<marker mode1>,<marker point1>,<arb name2>,<repeat count2>,
	// <play control2>, <marker mode2>, <marker point2>, and so on.
	tempSequenceString = "DATA:SEQ #";
	tempSegmentInfoString = "seq" + std::to_string(sequenceNum) + ",";
	if ( totalSegNum == 0 )
	{
		return;
	}
	for (int segNumInc = 0; segNumInc < totalSegNum - 1; segNumInc++)
	{
		// Format is 
		tempSegmentInfoString += "seg" + std::to_string(segNumInc + totalSegNum * sequenceNum) + ",";
		tempSegmentInfoString += std::to_string(waveformSegments[segNumInc].returnRepeatNum()) + ",";
		switch (waveformSegments[segNumInc].returnContinuationType())
		{
			case 0:
				tempSegmentInfoString += "repeat,";
				break;
			case 1:
				tempSegmentInfoString += "repeatTilTrig,";
				break;
			case 2:
				tempSegmentInfoString += "once,";
				break;
			case 3:
				tempSegmentInfoString += "repeatInf,";
				break;
			case 4:
				tempSegmentInfoString += "onceWaitTrig,";
				break;
			default:
				// ERROR!
				errBox("ERROR: entered location in code that shouldn't be entered. Check for logic mistakes in code.");
				return;
				break;
		}
		tempSegmentInfoString += "highAtStart,4,";
	}
		
	tempSegmentInfoString += "seg" + std::to_string((totalSegNum - 1) + totalSegNum * sequenceNum) + ",";
	tempSegmentInfoString += std::to_string(waveformSegments[totalSegNum - 1].returnRepeatNum()) + ",";
	switch (waveformSegments[totalSegNum - 1].returnContinuationType())
	{
		case 0:
			tempSegmentInfoString += "repeat,";
			break;
		case 1:
			tempSegmentInfoString += "repeatTilTrig,";
			break;
		case 2:
			tempSegmentInfoString += "once,";
			break;
		case 3:
			tempSegmentInfoString += "repeatInf,";
			break;
		case 4:
			tempSegmentInfoString += "onceWaitTrig,";
			break;
		default:
			// ERROR!
			MessageBox(0, "ERROR: entered location in code that shouldn't be entered. Check for logic mistakes in code.", 0, MB_OK);
			return;
			break;
	}
	tempSegmentInfoString += "highAtStart,4";

	tempSequenceString = tempSequenceString + std::to_string((std::to_string(tempSegmentInfoString.size())).size())
		+ std::to_string(tempSegmentInfoString.size()) + tempSegmentInfoString;

	//
	totalSequence = tempSequenceString;
	return;
}
/*
	* This function just returns the sequence string. It should already have been compiled using compileSequenceString when this is called.
	*/
std::string IntensityWaveform::returnSequenceString()
{
	return totalSequence;
}
/*
	* This function returns the truth of whether this sequence is being varied or not. This gets determined during the reading process.
	*/
bool IntensityWaveform::returnIsVaried()
{
	return isVaried;
}
/*
	* This waveform loops through all of the segments to find places where a variable value needs to be changed, and changes it.
	*/
void IntensityWaveform::replaceVarValues(std::string varName, double varValue)
{
	// loop through number of segments, looking for variable name.
	for (int segNumInc = 0; segNumInc < waveformSegments.size(); segNumInc++)
	{
		// loop through variables stored in this waveform
		for (int segVarInc = 0; segVarInc < waveformSegments[segNumInc].returnSegVarNamesSize(); segVarInc++)
		{
			// check if match
			if (varName == waveformSegments[segNumInc].returnVarName(segVarInc))
			{
				// if match then change
				switch (waveformSegments[segNumInc].returnVarLocation(segVarInc))
				{
						/*
						List of data types for variables for the different arguments:
						initial intensity = 1
						final intensity = 2
						time = 3
						*/
					case 1:
					{
						waveformSegments[segNumInc].assignInitValue(varValue);
						break;
					}
					case 2:
					{
						waveformSegments[segNumInc].assignFinValue(varValue);
						break;
					}
					case 3:
					{
						waveformSegments[segNumInc].assignTime(varValue);
						break;
					}
					default:
					{
						// BAD VALUE!
						thrower("ERROR: Bad location reached in code, this location shouldn't have been reached. Look for a logic error in the code.");
						return;
						break;
					}
				}
			}
		}
	}
}


/*
	* This function takes the powers inputted by the user and converts them (based on calibrations that we have done) to the corresponding voltage values
	* that the agilent needs to output in order to reach those powers. The calibration is currently hard-coded. This needs to be run before compiling the 
	* data string. 
	*/
void IntensityWaveform::convertPowersToVoltages()
{
	/// NOTES
	// TODO:make a structure and a front panel option. 
	// need to implement using calibrations, which haven't been done yet.
	// HARD CODED right now.
	// Expecting a calibration in terms of /MICROWATTS/!
	/// PAST CALIBRATIONs
	/// double newValue = -a * log(y * b);
	// (February 1st, 2016 calibrations)
	// double a = 0.245453772102427, b = 1910.3567515711145;
	// (February 2st, 2016 calibrations)
	// double a = 0.2454742248, b = 1684.849955;
	// double a = 0.2454742248, b = 1684.849955;
	// (April 14th, 2016 calibrations);
	// double a = 0.24182, b = 1943.25;
	// (June 16th, 2016 calibrations (NE10 filter in front of log pd)
	//double a = 0.247895, b = 218.559;
	// June 18th, 2016 calibrations (NE20 filter in front of log pd)
	// double a = 0.262771, b = 11.2122;
		
	// June 22nd AM, 2016 calibrations (No filter in front of log pd)
	//double a = 0.246853, b = 1330.08;
	// June 22nd PM, 2016 calibrations (NE10 filter in front of log pd, after tweaking servo parameters)
	//double a = 0.246862, b = 227.363;

	/// CLIBRATIOND WITH DIGITAL LOCK BOX
	/// newValue = a +  b * log(y - c); // here c is treated as a background light level, and the voltage output should be positive

	// July 14 2016, NE10 filter in front of log pd
	double a = 0.479262, b = 0.215003, c = 0.018189;
	// for each part of the waveform returnDataSize
	for (int segmentInc = 0; segmentInc < waveformSegments.size(); segmentInc++)
	{
		// for each data point in that part
		for (int dataConvertInc = 0; dataConvertInc < waveformSegments[segmentInc].returnDataSize(); dataConvertInc++)
		{
			// convert the user power, which is entered in mW, to uW. That's the units this calibration was done in.
			// y is the desired power in microwatts.
			double y = waveformSegments[segmentInc].returnDataVal(dataConvertInc) * 1000.0;
				
			//double newValue = -a * log(y * b);
			double newValue = a + b * log(y - c);
			waveformSegments[segmentInc].assignDataVal(dataConvertInc, newValue);
		}
	}
	return;
}


/*
	* This wavefunction loops through all the data values and figures out which ones are min and max.
	*/
void IntensityWaveform::calcMinMax()
{
	// NOT DBL_MIN!!!! this is a really small number, not a large negative number. I need a large negative number.
	maxVolt = -DBL_MAX;
	minVolt = DBL_MAX;
	for (int minMaxSegInc = 0; minMaxSegInc < waveformSegments.size(); minMaxSegInc++)
	{
		for (int minMaxDataInc = 0; minMaxDataInc < waveformSegments[minMaxSegInc].returnDataSize(); minMaxDataInc++)
		{
			if (waveformSegments[minMaxSegInc].returnDataVal(minMaxDataInc) < minVolt)
			{
				minVolt = waveformSegments[minMaxSegInc].returnDataVal(minMaxDataInc);
			}
			if (waveformSegments[minMaxSegInc].returnDataVal(minMaxDataInc) > maxVolt)
			{
				maxVolt = waveformSegments[minMaxSegInc].returnDataVal(minMaxDataInc);
			}
		}
	}
	// TODO: Test if these are really necessary.
	maxVolt += 1e-6;
	minVolt -= 1e-6;
	return;
}


/*
	* This function normalizes all of the data points to lie within the -1 to 1 range that I need to send to the agilent. The actual values outputted 
	* by the agilent are determined jointly by these values and the output range. you therefore need to use calcMinMax before this function.
	*/
void IntensityWaveform::normalizeVoltages()
{
	double scaleFactor = 2.0 / (maxVolt - minVolt);
	for (int normSegInc = 0; normSegInc < waveformSegments.size(); normSegInc++)
	{
		for (int normDataInc = 0; normDataInc < waveformSegments[normSegInc].returnDataSize(); normDataInc++)
		{
			double currVal = waveformSegments[normSegInc].returnDataVal(normDataInc);
			double normVal = (currVal - minVolt) * scaleFactor - 1;
			waveformSegments[normSegInc].assignDataVal(normDataInc, normVal);
		}
	}
	return;
}


/*
	* Returns the maximum voltage level currently in data structures.
	*/
double IntensityWaveform::returnMaxVolt()
{
	return maxVolt;
}


/*
	* Returns the minimum voltage level currently in data structures.
	*/
double IntensityWaveform::returnMinVolt()
{
	return minVolt;
}

/*
	* The constructor doesn't take arguments and initializes some values here to bad (but not garbage) values.
	*/
Segment::Segment()
{
	// initialize everything to invalid values.
	segmentType = -1;
	rampType = "";
	initValue = -1;
	finValue = -1;
	time = -1;
	continuationType = -1;
	// The actual initial value.
	varNum = 0;
};


/*
	* Nothing special right now.
	*/
Segment::~Segment()
{
	//
};


/*
	* segmentType = segTypeIn;
	*/
void Segment::assignSegType(int segTypeIn)
{
	segmentType = segTypeIn;
}


/*
	* initValue = initValIn;
	*/
void Segment::assignInitValue(double initValIn)
{
	initValue = initValIn;
}


/*
	* finValue = finValIn;
	*/
void Segment::assignFinValue(double finValIn)
{
	finValue = finValIn;
}


/*
	* rampType = rampTypeIn;
	*/
void Segment::assignRampType(std::string rampTypeIn)
{
	rampType = rampTypeIn;
}


/*
	* time = timeIn;
	*/
void Segment::assignTime(double timeIn)
{
	time = timeIn;
}


/*
	* continuationType = contTypeIn;
	*/
void Segment::assignContinuationType(int contTypeIn)
{
	continuationType = contTypeIn;
}


/*
	* repeatNum = repeatNumIn;
	*/
void Segment::assignRepeatNum(int repeatNumIn)
{
	repeatNum = repeatNumIn;
}


/*
	* return segmentType;
	*/
int Segment::returnSegmentType()
{
	return segmentType;
}


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
double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType )
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
		return (finPos - initPos) * (tanh( -4 + 8 * (double)iteration / size ) + 1) / 2;
	}
	// error message. I've already checked (outside this function) whether the ramp-type is a filename.
	else
	{
		std::string errMsg = "ERROR: ramp type " + rampType + " is unrecognized. If this is a file name, make sure the file exists and is in the project folder.\r\n";
		MessageBox( NULL, errMsg.c_str(), NULL, MB_OK );
		return 0;
	}
}


/*
	* This function uses the initial and final points along with the ramp and time of the segment to calculate all of the data points. This should be used so 
	* as to, after this function, you have all of the powers that you want (not voltages), and then call the voltage converter afterwards.
	*/
void Segment::calcData()
{
	// calculate the size of the waveform.
	double numDataPointsf = time * AGILENT_SAMPLE_RATE;
	// test if good time.
	if (fabs(numDataPointsf - round(numDataPointsf)) > 1e-6)
	{
		// Bad Time Warning
		thrower("ERROR: Bad time entered for the time of an intensity sequence segment. This resulted in a non-integer number of samples. Time cannot be"
						"defined with precision below the microsecond level for normal sample rates.");
		return;
	}
	// Convert to integer
	int numDataPoints = (int)round(numDataPointsf);
	// resize to zero. This is a complete reset of the data points in the class.
	dataArray.resize(0);
	// write the data points. These are all in powers. These are converted to voltages later.
	if (rampType == "nr" || rampType == "lin" || rampType == "tanh")
	{
		for (int dataInc = 0; dataInc < numDataPoints; dataInc++)
		{
			// constant waveform. Every data point is the same.
			dataArray.push_back(initValue + rampCalc(numDataPoints, dataInc, initValue, finValue, rampType));
		}
	}
	else if (rampType == "")
	{
		// Error: Ramp Type has not been set!
		thrower("ERROR: Data points tried to be written when the ramp type hadn't been set!");
		return;
	}
	else 
	{
		// invalid ramp Type!
		std::string errMsg;
		errMsg = "ERROR: Invalid Ramp Type in intensity sequence! Type entered was: " + rampType + ".";
		thrower(errMsg);
		return;
	}
	return;
}
/*
	* return boost::numeric_cast<long>(dataArray.size());
	*/
long Segment::returnDataSize()
{
	return boost::numeric_cast<long>(dataArray.size());
}
/*
	* return repeatNum;
	*/
int Segment::returnRepeatNum()
{
	return repeatNum;
}
/*
	* return dataArray[dataNum];
	*/
double Segment::returnDataVal(long dataNum)
{
	return dataArray[dataNum];
}
/*
	* return continuationType;
	*/
int Segment::returnContinuationType()
{
	return continuationType;
}
/*
	* varNum = vNumIn;
	*/
void Segment::assignVarNum(int vNumIn)
{
	varNum = vNumIn;
}
/*
	* segVarNames = segVarNamesIn;
	*/
void Segment::assignSegVarNames(std::vector<std::string> segVarNamesIn)
{
	segVarNames = segVarNamesIn;
}

/*
	* varLocations = varLocationsIn;
	*/
void Segment::assignVarLocations(std::vector<int> varLocationsIn)
{
	varLocations = varLocationsIn;
}

/*
	* return boost::numeric_cast<int>(segVarNames.size());
	*/
int Segment::returnSegVarNamesSize()
{
	return boost::numeric_cast<int>(segVarNames.size());
}
/*
	* return segVarNames[varNameIdentifier];
	*/
std::string Segment::returnVarName(int varNameIdentifier)
{
	return segVarNames[varNameIdentifier];
}
/*
	* return varLocations[varNameIdentifier];
	*/
int Segment::returnVarLocation(int varNameIdentifier)
{
	return varLocations[varNameIdentifier];
}
/*
	* dataArray[dataNum] = val;
	*/
void Segment::assignDataVal(int dataNum, double val)
{
	dataArray[dataNum] = val;
}
/*
	* return time;
	*/
double Segment::returnTime()
{
	return time;
}

/*
	* This function tells the agilent to put out the DC default waveform.
	*/
void Agilent::agilentDefault()
{
	visaOpenDefaultRM();
	visaOpen( AGILENT_ADDRESS );
	// turn it to the default voltage...
	visaWrite( std::string( "APPLy:DC DEF, DEF, " ) + AGILENT_DEFAULT_DC );
	// and leave...
	visaClose();
	// update current values
	currentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
	currentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);
}


void Agilent::analyzeIntensityScript(ScriptStream intensityFile, IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, std::vector<variable> singletons, profileSettings profileInfo)
{
	while (!intensityFile.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = intensityWaveformData->readIntoSegment(currentSegmentNumber, intensityFile, singletons, profileInfo);
		if (leaveTest < 0)
		{
			// Error
			std::string errMsg;
			errMsg = "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #" + std::to_string(currentSegmentNumber) + ".";
			thrower(errMsg);
			return;
		}
		if (leaveTest == 1)
		{
			// read function is telling this function to stop reading the file because it's at its end.
			break;
		}
		currentSegmentNumber++;
	}
}

/*
	* programIntensity opens the intensity file, reads the contents, loads them into an appropriate data structure, then from this data structure writes
	* segment and sequence information to the function generator.
	*/
void Agilent::programIntensity(int varNum, std::vector<variable> variables, std::vector<std::vector<double> > varValues, bool& intensityVaried,
						std::vector<minMaxDoublet>& minsAndMaxes, std::vector<std::vector<POINT>>& pointsToDraw, 
						std::vector<std::fstream>& intensityFiles, std::vector<variable> singletons, profileSettings profileInfo)
{
	// Initialize stuff
	IntensityWaveform intensityWaveformSequence;
	int currentSegmentNumber = 0;
	// connect to the agilent. I refuse to use the stupid typecasts. The way you often see these variables defined is using stupid things like ViRsc, ViUInt32, etc.
	visaOpenDefaultRM();
	visaOpen( AGILENT_ADDRESS );
	// looks like I'm setting the timeout value here... to be 40 s?
	visaSetAttribute( VI_ATTR_TMO_VALUE, 40000 );
	// Set sample rate
	visaWrite( "SOURCE1:FUNC:ARB:SRATE " + std::to_string( AGILENT_SAMPLE_RATE ) );
	// Set filtering state
	visaWrite( std::string( "SOURCE1:FUNC:ARB:FILTER " ) + AGILENT_FILTER_STATE );
	// Set Trigger Parameters
	visaWrite( "TRIGGER1:SOURCE EXTERNAL" );
	//
	visaWrite( "TRIGGER1:SLOPE POSITIVE" );
	for (int sequenceInc = 0; sequenceInc < intensityFiles.size(); sequenceInc++)
	{
		analyzeIntensityScript( intensityFiles[sequenceInc], &intensityWaveformSequence, currentSegmentNumber, singletons, profileInfo );
	}
	int totalSegmentNumber = currentSegmentNumber;
	intensityVaried = intensityWaveformSequence.returnIsVaried();
	// if varied
	if (intensityWaveformSequence.returnIsVaried() == true)
	{
		// loop through # of variable values
		for (int varValueCount = 0; varValueCount < varValues[0].size(); varValueCount++)
		{
			// Loop through variable names
			for (int varNameCount = 0; varNameCount < variables.size(); varNameCount++)
			{
				// replace variable values where found
				intensityWaveformSequence.replaceVarValues(variables[varNameCount].name, varValues[varNameCount][varValueCount]);
			}
			// Loop through all segments
			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// Use that information to write the data.
				intensityWaveformSequence.writeData( segNumInc );
			}
			// loop through again and calc/normalize/write values.
			intensityWaveformSequence.convertPowersToVoltages();
			intensityWaveformSequence.calcMinMax();
			minsAndMaxes.resize(varValueCount + 1);
			minsAndMaxes[varValueCount].max = intensityWaveformSequence.returnMaxVolt();
			minsAndMaxes[varValueCount].min = intensityWaveformSequence.returnMinVolt();
			intensityWaveformSequence.normalizeVoltages();

			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// send to the agilent.
				visaWrite( intensityWaveformSequence.compileAndReturnDataSendString( segNumInc, varValueCount, totalSegmentNumber ) );
				// Select the segment
				visaWrite( "SOURCE1:FUNC:ARB seg" + std::to_string( segNumInc + totalSegmentNumber * varValueCount ) );
				// Save the segment
				visaWrite( "MMEM:STORE:DATA \"INT:\\seg" + std::to_string( segNumInc + totalSegmentNumber * varValueCount ) + ".arb\"" );
				// increment for the next.
				visaWrite( "TRIGGER1:SLOPE POSITIVE" );
			}
			// Now handle seqeunce creation / writing.
			intensityWaveformSequence.compileSequenceString(totalSegmentNumber, varValueCount);
			// submit the sequence
			visaWrite( intensityWaveformSequence.returnSequenceString() );
			// Save the sequence
			visaWrite( "SOURCE1:FUNC:ARB seq" + std::to_string( varValueCount ) );
			visaWrite( "MMEM:STORE:DATA \"INT:\\seq" + std::to_string( varValueCount ) + ".seq\"" );
			// clear temporary memory.
			visaWrite( "SOURCE1:DATA:VOL:CLEAR" );
		}
	}
	// else not varying
	else
	{
		// Loop through all segments
		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Use that information to write the data.
			intensityWaveformSequence.writeData( segNumInc );
		}
		// no reassignment nessesary, no variables
		intensityWaveformSequence.convertPowersToVoltages();
		intensityWaveformSequence.calcMinMax();
		minsAndMaxes.resize(1);
		minsAndMaxes[0].max = intensityWaveformSequence.returnMaxVolt();
		minsAndMaxes[0].min = intensityWaveformSequence.returnMinVolt();
		intensityWaveformSequence.normalizeVoltages();

		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Set output impedance...
			visaWrite( std::string( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
			// set range of voltages...
			visaWrite( std::string( "SOURCE1:VOLT:LOW " ) + std::to_string( minsAndMaxes[0].min ) + " V" );
			visaWrite( std::string( "SOURCE1:VOLT:HIGH " ) + std::to_string( minsAndMaxes[0].max ) + " V" );
			// send to the agilent.
			visaWrite( intensityWaveformSequence.compileAndReturnDataSendString( segNumInc, 0, totalSegmentNumber ) );
			// Select the segment
			visaWrite( "SOURCE1:FUNC:ARB seg" + std::to_string( segNumInc ) );
			// Save the segment
			visaWrite( "MMEM:STORE:DATA \"INT:\\seg" + std::to_string( segNumInc ) + ".arb\"" );
			// increment for the next.
		}


		// Now handle seqeunce creation / writing.
		intensityWaveformSequence.compileSequenceString(totalSegmentNumber, 0);
		// submit the sequence
		visaWrite( intensityWaveformSequence.returnSequenceString() );
		// Save the sequence
		visaWrite( "SOURCE1:FUNC:ARB seq" + std::to_string( 0 ) );
		visaWrite( "MMEM:STORE:DATA \"INT:\\seq" + std::to_string( 0 ) + ".seq\"" );
		// clear temporary memory.
		visaWrite( "SOURCE1:DATA:VOL:CLEAR" );
	}
	visaClose();
}

/*
	* This function checks if the agilent throws an error or if there is an error communicating with the agilent.
	*/
void Agilent::errCheck(long status)
{
	long errorCode = 0;
	char buf[256] = { 0 };
	// Check comm status
	if (status < 0)	
	{
		// Error detected.
		thrower( "ERROR: Communcation error with agilent. Error Code: " + std::to_string( status ));
		return;
	}
	// Query the agilent for errors.
	viQueryf(instrument, "SYST:ERR?\n", "%ld,%t", &errorCode, buf);
	if (errorCode != 0)
	{
		// Agilent error
		thrower( "ERROR: agilent returned error message: " + std::to_string( errorCode ) + ":" + buf );
		return;
	}
	return;
}

/*
	* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
	*/
void Agilent::selectIntensityProfile(int varNum, bool intensityIsVaried, std::vector<minMaxDoublet> intensityMinMax)
{
	if (intensityIsVaried || varNum == 0)
	{
		unsigned long viDefaultRM, Instrument;
		unsigned long actual;
		if (!AGILENT_SAFEMODE)
		{
			visaOpenDefaultRM();
			visaOpen( AGILENT_ADDRESS );
		}
		std::string SCPIcmd;
		if (!AGILENT_SAFEMODE)
		{
			// Load sequence that was previously loaded.
			visaWrite("MMEM:LOAD:DATA \"INT:\\seq" + std::to_string(varNum) + ".seq\"");
			visaWrite( "SOURCE1:FUNC ARB");
			visaWrite( "SOURCE1:FUNC:ARB \"INT:\\seq" + std::to_string(varNum) + ".seq\"");
			// Set output impedance...
			visaWrite( std::string("OUTPUT1:LOAD ") + AGILENT_LOAD);
			visaWrite( std::string("SOURCE1:VOLT:LOW ") + std::to_string(intensityMinMax[varNum].min) + " V");
			visaWrite( std::string("SOURCE1:VOLT:HIGH ") + std::to_string(intensityMinMax[varNum].max) + " V");
			visaWrite( "OUTPUT1 ON" );
			// and leave...
			visaClose();
		}
	}
	return;
}

/// 

/// 

void Agilent::visaWrite( std::string message )
{
	// not sure what this is for.
	unsigned long actual;
	if (AGILENT_SAFEMODE)
	{
		errCheck( viWrite( instrument, (unsigned char*)message.c_str(), (ViUInt32)message.size(), &actual ) );
	}
}


void Agilent::visaClose()
{
	if (AGILENT_SAFEMODE)
	{
		errCheck( viClose( defaultResourceManager ) );
	}
}


void Agilent::visaOpenDefaultRM()
{
	if (AGILENT_SAFEMODE)
	{
		errCheck( viOpenDefaultRM( &defaultResourceManager ) );
	}
}


void Agilent::visaOpen( std::string address )
{
	if (AGILENT_SAFEMODE)
	{
		errCheck( viOpen( defaultResourceManager, (char *)address.c_str(), VI_NULL, VI_NULL, &instrument ) );
	}
}

void Agilent::visaSetAttribute(ViAttr attributeName, ViAttrState value)
{
	if (AGILENT_SAFEMODE)
	{
		errCheck( viSetAttribute( instrument, attributeName, value ) );
	}
}



/**
* Overload for double input.
* Test if the input is a variable. If it was, store what parameter is getting varied (varParamTypes), store which variable name is associated
* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
* function using this function to check the arrays that get returned for the specified number of variables.
*
* @param dataToAssign is the value of a waveData variable that is being read in. If a variable is found it isn't actually assigned.
* @param file is the instructions file being read for this input.
* @param vCount is the number of variables that have been assigned so far.
* @param vNames is a vector that holds the names of the variables that are found in the script.
* @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
* @param dataType is the number to assign to vParamTypes if a variable is being used.
*/
void getParamCheckVar( double& dataToAssign, std::fstream& fName, int& vCount, std::vector<std::string>& vNames, std::vector<int>& vParamTypes,
									   int dataType, std::vector<variable> singletons )
{
	std::string tempInput;
	int stringPos;
	rmWhite( fName );
	fName >> tempInput;
	// pull input to lower case to prevent stupid user input errors.
	std::transform( tempInput.begin(), tempInput.end(), tempInput.begin(), tolower );
	if (tempInput[0] == '\'')
	{
		thrower("ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n");
		return;
	}
	if (tempInput[0] == '#')
	{
		thrower("ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
					"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
		return;
	}
	if (tempInput[0] == '%')
	{
		thrower("ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
		return;
	}
	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '-' || tempInput[0] == '.')
	{
		stringPos = 1;
		if (tempInput[1] == '-' || tempInput[1] == '.')
		{
			thrower("ERROR: The first two characters of some input are both either '-' or '.'. This might be because you tried to input a negative"
						"decimal, which you aren't allowed to do.");
			return;
		}
	}
	else
	{
		stringPos = 0;
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			dataToAssign = singletons[singletonInc].value;
			return;
		}
	}
	if (!isdigit( tempInput[stringPos] ))
	{
		// load variable name into structure.
		vNames.push_back( tempInput );
		vParamTypes.push_back( dataType );
		vCount++;
		return;
	}
	else
	{
		// this should happen most of the time.
		dataToAssign = (std::stod( tempInput ));
		return;
	}
}


/**
* overload for integer input.
* Test if the input is a variable. If it was, store what parameter is getting varied (varParamTypes), store which variable name is associated
* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
* function using this function to check the arrays that get returned for the specified number of variables.
*
* @param dataToAssign is the value of a waveData variable that is being read in. If a variable is found it isn't actually assigned.
* @param file is the instructions file being read for this input.
* @param vCount is the number of variables that have been assigned so far.
* @param vNames is a vector that holds the names of the variables that are found in the script.
* @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
* @param dataType is the number to assign to vParamTypes if a variable is being used.
*/
int getParamCheckVar( int& dataToAssign, std::fstream& scriptName, int& vCount, std::vector<std::string>& vNames, std::vector<int>& vParamTypes,
									   int dataType, std::vector<variable> singletons )
{
	std::string tempInput;
	int stringPos;
	rmWhite( scriptName );
	scriptName >> tempInput;
	// pull input to lower case to prevent stupid user input errors.
	std::transform( tempInput.begin(), tempInput.end(), tempInput.begin(), ::tolower );

	if (tempInput[0] == '\'')
	{
		thrower( "ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n" );
		return;
	}
	if (tempInput[0] == '#')
	{
		thrower("ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
					"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
		return;
	}
	if (tempInput[0] == '%')
	{
		thrower( "ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors." );
		return;
	}
	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '-' || tempInput[0] == '.')
	{
		stringPos = 1;
		if (tempInput[1] == '-' || tempInput[1] == '.')
		{
			thrower( "ERROR: The first two characters of some input are both either '-' or '.'. This might be because you tried to input a negative"
					 "decimal, which you aren't allowed to do." );
			return;
		}
	}
	else
	{
		stringPos = 0;
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			dataToAssign = singletons[singletonInc].value;
			return;
		}
	}
	if (!isdigit( tempInput[stringPos] ))
	{
		// load variable name into structure.
		vNames.push_back( tempInput );
		vParamTypes.push_back( dataType );
		vCount++;
		return;
	}
	else
	{
		// this should happen most of the time.
		dataToAssign = (std::stoi( tempInput ));
		return;
	}
}

/**
* Overload for double input.
* Test if the input is a variable. If it was, store what parameter is getting varied (vParamTypes), store which variable name is associated
* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
* function using this function to check the arrays that get returned for the specified number of variables. This function deals with constant waveforms,
* where either the frequency or the ramp is not being ramped, and so the inputted data needs to be assigned to both the initial and final values of the
* parameter type.
*
* data1ToAssign is the value of the first waveData variable that is being read in. If a variable is found it isn't actually assigned.
* data2ToAssign is the value of the second waveData variable that is being read in. If a variable is found it isn't actually assigned.
* file is the instructions file being read for this input.
* vCount is the number of variables that have been assigned so far.
* vNames is a vector that holds the names of the variables that are found in the script.
* vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
* dataType1 is the number to assign to vParamTypes for the first parameter if a variable is being used.
* dataType2 is the number to assign to vParamTypes for the second parameter if a variable is being used.
* singletons is the list of singletons that the user set for this configuration. If a variable name is found to match a singleton name, the value is
*		immediately set to the singleton's value.
*/
int NiawgController::getParamCheckVarConst( double& data1ToAssign, double& data2ToAssign, std::fstream& file, int& vCount, std::vector<std::string>& vNames,
											std::vector<int>& vParamTypes, int dataType1, int dataType2, std::vector<variable> singletons )
{
	std::string tempInput;
	rmWhite( file );
	file >> tempInput;
	// pull input to lower case to prevent stupid user input errors.
	std::transform( tempInput.begin(), tempInput.end(), tempInput.begin(), ::tolower );

	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '\'')
	{
		thrower("ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n");
		return;
	}
	if (tempInput[0] == '#')
	{
		thrower( "ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
					"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
		return;
	}
	if (tempInput[0] == '%')
	{
		thrower("ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
		return;
	}
	if (tempInput[0] == '-')
	{
		thrower( "ERROR: it appears that you entered a negative frequency or amplitude. You can't do that.");
		return;
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			data1ToAssign = singletons[singletonInc].value;
			data2ToAssign = data1ToAssign;
			return;
		}
	}
	// I don't need to check for -1 input because this function should never be used on the phase or the time, only frequency or amplitude for non-ramping waveforms. 
	if (!isdigit( tempInput[0] ))
	{
		// add variable name
		vNames.push_back( tempInput );
		vParamTypes.push_back( dataType1 );
		vCount++;
		// Do the same for the second data that needs to be assigned.
		vNames.push_back( tempInput );
		vParamTypes.push_back( dataType2 );
		vCount++;
		return;
	}
	else
	{
		// this should happen most of the time.
		data1ToAssign = std::stod( tempInput );
		data2ToAssign = data1ToAssign;
		return;
	}
}

/**
* Overload for integer input.
* Test if the input is a variable. If it was, store what parameter is getting varied (vParamTypes), store which variable name is associated
* with this variable, increment the number of variables in this waveform, and increment the varPresent marker, which gets returned and tells the
* function using this function to check the arrays that get returned for the specified number of variables. This function deals with constant waveforms,
* where either the frequency or the ramp is not being ramped, and so the inputted data needs to be assigned to both the initial and final values of the
* parameter type.
*
* @param data1ToAssign is the value of the first waveData variable that is being read in. If a variable is found it isn't actually assigned.
* @param data2ToAssign is the value of the second waveData variable that is being read in. If a variable is found it isn't actually assigned.
* @param file is the instructions file being read for this input.
* @param vCount is the number of variables that have been assigned so far.
* @param vNames is a vector that holds the names of the variables that are found in the script.
* @param vParamTypes is a vector that holds the information as to what type of parameter is being varried for a given variable name.
* @param dataType1 is the number to assign to vParamTypes for the first parameter if a variable is being used.
* @param dataType2 is the number to assign to vParamTypes for the second parameter if a variable is being used.
*/
int getParamCheckVarConst( int& data1ToAssign, double& data2ToAssign, std::fstream& scriptName, int& vCount, std::vector<std::string>& vNames,
											std::vector<int>& vParamTypes, int dataType1, int dataType2, std::vector<variable> singletons )
{
	std::string tempInput;
	rmWhite( scriptName );
	scriptName >> tempInput;
	// pull input to lower case to prevent stupid user input errors.
	std::transform( tempInput.begin(), tempInput.end(), tempInput.begin(), ::tolower );

	// the following aren't digits, but don't indicate variables.
	if (tempInput[0] == '\'')
	{
		thrower( "ERROR: Don't use \' as a variable in your instructions file, this character is reserved by the program.\n");
		return;
	}
	if (tempInput[0] == '#')
	{
		thrower( "ERROR: the delimeter '#' was detected in a waveform before it was supposed to be. This indicates either that there are too few "
					"inputs for this waveform type or that the program is not reading the input correctly, e.g. because of extraneous semicolons.");
		return;
	}
	if (tempInput[0] == '%')
	{
		thrower( "ERROR: the character % was detected in the input. This shouldn't be possible. Look for logic errors.");
		return;
	}
	if (tempInput[0] == '-')
	{
		thrower( "ERROR: it appears that you entered a negative frequency or amplitude. You can't do that.");
		return;
	}
	// check if this is a singleton variable. If so, immediately assign relevant data point to singleton value.
	for (int singletonInc = 0; singletonInc < singletons.size(); singletonInc++)
	{
		if (tempInput == singletons[singletonInc].name)
		{
			data1ToAssign = singletons[singletonInc].value;
			data2ToAssign = data1ToAssign;
			return;
		}
	}
	// I don't need to check for -1 input because this function should never be used on the phase or the time, only frequency or amplitude for non-ramping waveforms. 
	if (!isdigit( tempInput[0] ))
	{
		// add variable name
		vNames.push_back( tempInput );
		vParamTypes.push_back( dataType1 );
		vCount++;
		// Do the same for the second data that needs to be assigned.
		vNames.push_back( tempInput );
		vParamTypes.push_back( dataType2 );
		vCount++;
		return;
	}
	else
	{
		// this should happen most of the time.
		data1ToAssign = std::stoi( tempInput );
		data2ToAssign = data1ToAssign;
		return;
	}
}