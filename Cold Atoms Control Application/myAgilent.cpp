#include "stdafx.h"
#include "myAgilent.h"
#include "rmWhite.h"
//#include "scriptingFuncs.h"
#include "constants.h"
//#include "rampCalc.h"
#include "externals.h"
#include "myMath.h"
#include "myNIAWG.h"
#include "boost/cast.hpp"

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
	IntensityWaveform::IntensityWaveform()
	{
		segmentNum = 0;
		totalSequence = "";
		isVaried = false;
	};
	IntensityWaveform::~IntensityWaveform()
	{

	};
	/*
	 * This function reads out a segment of script file and loads it into a segment to be calculated and manipulated.
	 * segNum: This tells the function what the next segment # is.
	 * fileName: this is the file object to be read from.
	 */
	int IntensityWaveform::readIntoSegment(int segNum, std::fstream& fileName)
	{
		rmWhite(fileName);
		std::string intensityCommand;
		std::vector<std::string> tempVarNames;
		std::vector<int> tempVarLocations;
		int tempVarNum = 0;
		if (fileName.eof() == true)
		{
			// reached end of file, return with message.
			return 1;
		}
		// Grab the command type (e.g. ramp, const)
		getline(fileName, intensityCommand, '\r');
		if (intensityCommand == "Intensity Hold")
		{
			waveformSegments.resize(segNum + 1);
			waveformSegments[segNum].assignSegType(0);
		}
		else if (intensityCommand == "Intensity Ramp")
		{
			waveformSegments.resize(segNum + 1);
			waveformSegments[segNum].assignSegType(1);
		}
		else
		{
			std::string errMsg = "ERROR: Intensity command not recognized. The command was \"" + intensityCommand + "\"";
			MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
			return -1;
		}

		double tempTime, tempIntensityInit, tempIntensityFin;
		unsigned int tempRepeatNum = 0;
		std::string delimiter, tempContinuationType, tempRampType;
		// List of data types for variables for the different arguments:
		// initial intensity = 1
		// final intensity = 2
		// time = 3
		if (waveformSegments[segNum].returnSegType() == 1)
		{
			rmWhite(fileName);
			fileName >> tempRampType;
			myNIAWG::script::getParamCheckVar(tempIntensityInit, fileName, tempVarNum, tempVarNames, tempVarLocations, 1);
			myNIAWG::script::getParamCheckVar(tempIntensityFin, fileName, tempVarNum, tempVarNames, tempVarLocations, 2);
		}
		else
		{
			tempRampType = "nr";
			myNIAWG::script::getParamCheckVarConst(tempIntensityInit, tempIntensityFin, fileName, tempVarNum, tempVarNames, tempVarLocations, 1, 2);
		}
		myNIAWG::script::getParamCheckVar(tempTime, fileName, tempVarNum, tempVarNames, tempVarLocations, 3);

		rmWhite(fileName);
		fileName >> tempContinuationType;
		if (tempContinuationType == "Repeat")
		{
			// There is an extra input in this case.
			rmWhite(fileName);
			fileName >> tempRepeatNum;
		}
		else
		{
			tempRepeatNum = 0;
		}
		rmWhite(fileName);
		fileName >> delimiter;
		if (delimiter != "#")
		{
			// input number mismatch.
			std::string errorMsg;
			errorMsg = "ERROR: The delimeter is missing in the Intensity script file for Segment #" + std::to_string(segNum + 1)
						+ ". The value placed in the delimeter location was " + delimiter + " while it should have been '#'.This"
						+ " indicates that either the code is not interpreting the user input incorrectly or that the user has inputted too many parameters for this type"
						+ " of Segment. Use of \"Repeat\" without the number of repeats following will also trigger this error.";
			MessageBox(NULL, errorMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);
			return -1;
		}

		if (tempTime <= 0)
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
				errMsg = "ERROR: Invalid time entered in the intensity script file for Segment #" + std::to_string(segNum) + ". the value entered was " 
						 + std::to_string(tempTime) + ".";
				MessageBox(NULL, errMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);
				return -1;
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
				MessageBox(NULL, errMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);
				return -1;
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
				MessageBox(NULL, errMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);
				return -1;
			}
		}
		if (tempContinuationType == "Repeat")
		{
			waveformSegments[segNum].assignContinuationType(0);
			if (tempRepeatNum < 0)
			{
				std::string errMsg;
				errMsg = "ERROR: Invalid intensity entered in the intensity script file for Segment #" + std::to_string(segNum) + ". the value entered was " 
						 + std::to_string(tempIntensityInit) + ".";
				MessageBox(NULL, errMsg.c_str(), "ERROR", MB_OK | MB_ICONERROR);
				return -1;
			}
		}
		else if (tempContinuationType == "RepeatUntilTrigger")
		{
			waveformSegments[segNum].assignContinuationType(1);
		}
		else if (tempContinuationType == "Once")
		{
			waveformSegments[segNum].assignContinuationType(2);
		}
		else if (tempContinuationType == "RepeatForever")
		{
			waveformSegments[segNum].assignContinuationType(3);
		}
		else if (tempContinuationType == "OnceWaitTrig")
		{
			waveformSegments[segNum].assignContinuationType(4);
		}
		else
		{
			// string not recognized
			std::string errMsg;
			errMsg = "ERROR: Invalid Continuation Option on intensity segment #" + std::to_string(segNum) + ". The string entered was " + tempContinuationType
					 + ". Please enter \"Repeat #\", \"RepeatUntilTrigger\", \"OnceWaitTrig\", or \"Once\".";
			MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
		}
		// Make Everything Permanent
		waveformSegments[segNum].assignRepeatNum(tempRepeatNum);
		waveformSegments[segNum].assignInitValue(tempIntensityInit);
		waveformSegments[segNum].assignFinValue(tempIntensityFin);
		waveformSegments[segNum].assignTime(tempTime);
		waveformSegments[segNum].assignRampType(tempRampType);

		waveformSegments[segNum].assignVarNum(tempVarNum);
		waveformSegments[segNum].assignSegVarNames(tempVarNames);
		waveformSegments[segNum].assignVarLocations(tempVarLocations);

		if (tempVarNum > 0)
		{
			isVaried = true;
		}

		return 0;
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
	int IntensityWaveform::writeData(int segNum)
	{
		return waveformSegments[segNum].calcData();
	}
	/*
	 * This function compiles the sequence string which tells the agilent what waveforms to output when and with what trigger control. The sequence is stored
	 * as a part of the class.
	 */
	int IntensityWaveform::compileSequenceString(int totalSegNum, int sequenceNum)
	{
		std::string tempSequenceString, tempSegmentInfoString;
		// Total format is  #<n><n digits><sequence name>,<arb name1>,<repeat count1>,<play control1>,<marker mode1>,<marker point1>,<arb name2>,<repeat count2>,
		// <play control2>, <marker mode2>, <marker point2>, and so on.
		tempSequenceString = "DATA:SEQ #";
		tempSegmentInfoString = "seq" + std::to_string(sequenceNum) + ",";
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
					MessageBox(0, "ERROR: entered location in code that shouldn't be entered. Check for logic mistakes in code.", 0, MB_OK);
					return -1;
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
				return -1;
				break;
		}
		tempSegmentInfoString += "highAtStart,4";

		tempSequenceString = tempSequenceString + std::to_string((std::to_string(tempSegmentInfoString.size())).size())
			+ std::to_string(tempSegmentInfoString.size()) + tempSegmentInfoString;

		//
		totalSequence = tempSequenceString;
		return 0;
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
	int IntensityWaveform::replaceVarValues(std::string varName, double varValue)
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
							MessageBox(0, "ERROR: Bad location reached in code, this location shouldn't have been reached. Look for a logic error in the code.", 0, MB_OK);
							return -1;
							break;
						}
					}
				}
			}
		}
		return 0;
	}
	/*
	 * This function takes the powers inputted by the user and converts them (based on calibrations that we have done) to the corresponding voltage values
	 * that the agilent needs to output in order to reach those powers. The calibration is currently hard-coded. This needs to be run before compiling the 
	 * data string. 
	 */
	int IntensityWaveform::convertPowersToVoltages()
	{
		/// NOTES
		// TODO:make a structure and a front panel option. 
		// need to implement using calibrations, which haven't been done yet.
		// HARD CODED right now.
		// Expecting a calibration in terms of /MICROWATTS/!
		/// PAST CALIBRATIONs
		// (February 1st, 2016 calibrations)
		// double a = 0.245453772102427, b = 1910.3567515711145;
		// (February 2st, 2016 calibrations)
		// double a = 0.2454742248, b = 1684.849955;
		// double a = 0.2454742248, b = 1684.849955;
		// (April 14th, 2016 calibrations);
		double a = 0.24182, b = 1943.25;
		// for each part of the waveform returnDataSize
		for (int segmentInc = 0; segmentInc < waveformSegments.size(); segmentInc++)
		{
			// for each data point in that part
			for (int dataConvertInc = 0; dataConvertInc < waveformSegments[segmentInc].returnDataSize(); dataConvertInc++)
			{
				// convert the user power, which is entered in mW, to uW. That's the units this calibration was done in.
				double y = waveformSegments[segmentInc].returnDataVal(dataConvertInc) * 1000.0;
				double newValue = -a * log(y * b);
				waveformSegments[segmentInc].assignDataVal(dataConvertInc, newValue);
			}
		}
		return 0;
	}
	/*
	 * This wavefunction loops through all the data values and figures out which ones are min and max.
	 */
	int IntensityWaveform::calcMinMax()
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
		return 0;
	}
	/*
	 * This function normalizes all of the data points to lie within the -1 to 1 range that I need to send to the agilent. The actual values outputted 
	 * by the agilent are determined jointly by these values and the output range. you therefore need to use calcMinMax before this function.
	 */
	int IntensityWaveform::normalizeVoltages()
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
		return 0;
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
	int Segment::assignSegType(int segTypeIn)
	{
		segmentType = segTypeIn;
		return 0;
	}
	/*
	 * initValue = initValIn;
	 */
	int Segment::assignInitValue(double initValIn)
	{
		initValue = initValIn;
		return 0;
	}
	/*
	 * finValue = finValIn;
	 */
	int Segment::assignFinValue(double finValIn)
	{
		finValue = finValIn;
		return 0;
	}
	/*
	 * rampType = rampTypeIn;
	 */
	int Segment::assignRampType(std::string rampTypeIn)
	{
		rampType = rampTypeIn;
		return 0;
	}
	/*
	 * time = timeIn;
	 */
	int Segment::assignTime(double timeIn)
	{
		time = timeIn;
		return 0;
	}
	/*
	 * continuationType = contTypeIn;
	 */
	int Segment::assignContinuationType(int contTypeIn)
	{
		continuationType = contTypeIn;
		return 0;
	}
	/*
	 * repeatNum = repeatNumIn;
	 */
	int	Segment::assignRepeatNum(int repeatNumIn)
	{
		repeatNum = repeatNumIn;
		return 0;
	}
	/*
	 * return segmentType;
	 */
	int Segment::returnSegType()
	{
		return segmentType;
	}
	/*
	 * This function uses the initial and final points along with the ramp and time of the segment to calculate all of the data points. This should be used so 
	 * as to, after this function, you have all of the powers that you want (not voltages), and then call the voltage converter afterwards.
	 */
	int Segment::calcData()
	{
		// calculate the size of the waveform.
		double numDataPointsf = time * AGILENT_SAMPLE_RATE;
		// test if good time.
		if (fabs(numDataPointsf - round(numDataPointsf)) > 1e-6)
		{
			// Bad Time Warning
			MessageBox(0, "ERROR: Bad time entered for the time of an intensity sequence segment. This resulted in a non-integer number of samples. Time cannot be"
						  "defined with precision below the microsecond level for normal sample rates.", 0, MB_OK);
			return -1;
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
				dataArray.push_back(initValue + myMath::rampCalc(numDataPoints, dataInc, initValue, finValue, rampType));
			}
		}
		else if (rampType == "")
		{
			// Error: Ramp Type has not been set!
			MessageBox(0, "ERROR: Data points tried to be written when the ramp type hadn't been set!", 0, MB_OK);
			return -1;
		}
		else 
		{
			// invalid ramp Type!
			std::string errMsg;
			errMsg = "ERROR: Invalid Ramp Type in intensity sequence! Type entered was: " + rampType + ".";
			MessageBox(0, errMsg.c_str(), 0, MB_OK);
			return -1;
		}

		return 0;
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
	int Segment::assignVarNum(int vNumIn)
	{
		varNum = vNumIn;
		return 0;
	}
	/*
	 * segVarNames = segVarNamesIn;
	 */
	int Segment::assignSegVarNames(std::vector<std::string> segVarNamesIn)
	{
		segVarNames = segVarNamesIn;
		return 0;
	}
	/*
	 * varLocations = varLocationsIn;
	 */
	int Segment::assignVarLocations(std::vector<int> varLocationsIn)
	{
		varLocations = varLocationsIn;
		return 0;
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
	int Segment::assignDataVal(int dataNum, double val)
	{
		dataArray[dataNum] = val;
		return 0;
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
	int agilentDefault()
	{
		unsigned long viDefaultRM, Instrument;
		unsigned long actual;
		std::string SCPIcmd;
		if (!SAFEMODE)
		{
			viOpenDefaultRM(&viDefaultRM);
			viOpen(viDefaultRM, (char *)AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
			// turn it to the default voltage...
			SCPIcmd = std::string("APPLy:DC DEF, DEF, ") + AGILENT_DEFAULT_DC;
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)(SCPIcmd).c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// and leave...
			viClose(Instrument);
			viClose(viDefaultRM);
		}

		// update current values
		eCurrentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
		eCurrentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);

		return 0;
	}

	/*
	 * programIntensity opens the intensity file, reads the contents, loads them into an appropriate data structure, then from this data structure writes
	 * segment and sequence information to the function generator.
	 */
	int programIntensity(int varNum, std::vector<std::string> varNames, std::vector<std::vector<double> > varValues, bool& intensityVaried, 
						 std::vector<myMath::minMaxDoublet>& minsAndMaxes, std::vector<std::vector<POINT>>& pointsToDraw, std::vector<std::fstream>& intensityFiles)
	{
		// Initialize stuff
		myAgilent::IntensityWaveform intensityWaveformSequence;

		int currentSegmentNumber = 0;

		// connect to the agilent. I refuse to use the stupid typecasts. The way you often see these variables defined is using stupid things like ViRsc, ViUInt32, etc.
		unsigned long viDefaultRM = 0, Instrument = 0;
		unsigned long actual;
		std::string SCPIcmd;
		if (!SAFEMODE)
		{
			viOpenDefaultRM(&viDefaultRM);
			viOpen(viDefaultRM, (char *)AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
			// ???
			agilentErrorCheck(viSetAttribute(Instrument, VI_ATTR_TMO_VALUE, 40000), Instrument);
			// Set sample rate
			SCPIcmd = "SOURCE1:FUNC:ARB:SRATE " + std::to_string(AGILENT_SAMPLE_RATE);
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// Set filtering state
			SCPIcmd = std::string("SOURCE1:FUNC:ARB:FILTER ") + AGILENT_FILTER_STATE;
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// Set Trigger Parameters
			SCPIcmd = std::string("TRIGGER1:SOURCE EXTERNAL");
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			//
			SCPIcmd = std::string("TRIGGER1:SLOPE POSITIVE");
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
		}
		for (int sequenceInc = 0; sequenceInc < intensityFiles.size(); sequenceInc++)
		{

			while (!intensityFiles[sequenceInc].eof())
			{
				// Procedurally read lines into segment informations.
				int leaveTest = intensityWaveformSequence.readIntoSegment(currentSegmentNumber, intensityFiles[sequenceInc]);
				if (leaveTest < 0)
				{
					// Error
					std::string errMsg;
					errMsg = "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #" + std::to_string(currentSegmentNumber) + ".";
					MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
					return -1;
				}
				if (leaveTest == 1)
				{
					break;
				}
				currentSegmentNumber++;
			}
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
				for (int varNameCount = 0; varNameCount < varNames.size(); varNameCount++)
				{
					// replace variable values where found
					intensityWaveformSequence.replaceVarValues(varNames[varNameCount], varValues[varNameCount][varValueCount]);
				}
				// Loop through all segments
				for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
				{
					// Use that information to write the data.
					if (intensityWaveformSequence.writeData(segNumInc) < 0)
					{
						// Error
						std::string errMsg;
						errMsg = "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #" + std::to_string(totalSegmentNumber) + ".";
						MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
						return -1;
					}
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
					if (!SAFEMODE)
					{
						SCPIcmd = intensityWaveformSequence.compileAndReturnDataSendString(segNumInc, varValueCount, totalSegmentNumber);
						// send to the agilent.
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
						// Select the segment
						SCPIcmd = "SOURCE1:FUNC:ARB seg" + std::to_string(segNumInc + totalSegmentNumber * varValueCount);
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
						// Save the segment
						SCPIcmd = "MMEM:STORE:DATA \"INT:\\seg" + std::to_string(segNumInc + totalSegmentNumber * varValueCount) + ".arb\"";
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
						// increment for the next.
						SCPIcmd = std::string("TRIGGER1:SLOPE POSITIVE");
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					}
				}

				// Now handle seqeunce creation / writing.
				intensityWaveformSequence.compileSequenceString(totalSegmentNumber, varValueCount);
				if (!SAFEMODE)
				{
					// submit the sequence
					SCPIcmd = intensityWaveformSequence.returnSequenceString();
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// Save the sequence
					SCPIcmd = "SOURCE1:FUNC:ARB seq" + std::to_string(varValueCount);
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

					SCPIcmd = "MMEM:STORE:DATA \"INT:\\seq" + std::to_string(varValueCount) + ".seq\"";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// clear temporary memory.
					SCPIcmd = "SOURCE1:DATA:VOL:CLEAR";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				}
			}
		}
		// else not varying
		else
		{
			// Loop through all segments
			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// Use that information to write the data.
				if (intensityWaveformSequence.writeData(segNumInc) < 0)
				{
					// Error
					std::string errMsg;
					errMsg = "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #" + std::to_string(totalSegmentNumber) + ".";
					MessageBox(NULL, errMsg.c_str(), NULL, MB_OK);
					return -1;
				}
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
				if (!SAFEMODE)
				{
					// Set output impedance...
					SCPIcmd = std::string("OUTPUT1:LOAD ") + AGILENT_LOAD;
					// set range of voltages...
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					SCPIcmd = std::string("SOURCE1:VOLT:LOW ") + std::to_string(minsAndMaxes[0].min) + " V";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					SCPIcmd = std::string("SOURCE1:VOLT:HIGH ") + std::to_string(minsAndMaxes[0].max) + " V";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// get the send string
					SCPIcmd = intensityWaveformSequence.compileAndReturnDataSendString(segNumInc, 0, totalSegmentNumber);
					// send to the agilent.
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

					// Select the segment
					SCPIcmd = "SOURCE1:FUNC:ARB seg" + std::to_string(segNumInc);
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// Save the segment
					SCPIcmd = "MMEM:STORE:DATA \"INT:\\seg" + std::to_string(segNumInc) + ".arb\"";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// increment for the next.
				}
			}


			// Now handle seqeunce creation / writing.
			intensityWaveformSequence.compileSequenceString(totalSegmentNumber, 0);
			if (!SAFEMODE)
			{
				// submit the sequence
				SCPIcmd = intensityWaveformSequence.returnSequenceString();
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				// Save the sequence
				SCPIcmd = "SOURCE1:FUNC:ARB seq" + std::to_string(0);
				viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual);
				//		agilentErrorCheck(, Instrument);
				SCPIcmd = "MMEM:STORE:DATA \"INT:\\seq" + std::to_string(0) + ".seq\"";
				viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual);
				//		agilentErrorCheck(, Instrument);
				// clear temporary memory.
				SCPIcmd = "SOURCE1:DATA:VOL:CLEAR";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			}
		}
		viClose(Instrument);
		viClose(viDefaultRM);
		return 0;
	}

	/*
	 * This function checks if the agilent throws an error or if there is an error communicating with the agilent. it returns -1 if error, 0 otherwise.
	 */
	int agilentErrorCheck(long status, unsigned long vi)
	{
		long errorCode = 0;
		char buf[256] = { 0 };
		// Check comm status
		if (status < 0)	
		{
			// Error detected.
			std::string commErrMsg;
			commErrMsg = "ERROR: Communcation error with agilent. Error Code: " + std::to_string(status);
			MessageBox(0, commErrMsg.c_str(), 0, MB_OK);
			return -1;
		}
		if (!SAFEMODE)
		{
			// Query the agilent for errors.
			viQueryf(vi, "SYST:ERR?\n", "%ld,%t", &errorCode, buf);
		}
		if (errorCode != 0)
		{
			// Agilent error
			std::string agErrMsg;
			agErrMsg = "ERROR: agilent returned error message: " + std::to_string(errorCode) + ":" + buf;
			MessageBox(0, agErrMsg.c_str(), 0, MB_OK);
			return -1;
		}
		return 0;
	}

	/*
	 * This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
	 */
	int selectIntensityProfile(int varNum, bool intensityIsVaried, std::vector<myMath::minMaxDoublet> intensityMinMax)
	{
		if (intensityIsVaried || varNum == 0)
		{
			unsigned long viDefaultRM, Instrument;
			unsigned long actual;
			if (!SAFEMODE)
			{
				viOpenDefaultRM(&viDefaultRM);
				viOpen(viDefaultRM, (char *)AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
			}
			std::string SCPIcmd;
			if (!SAFEMODE)
			{
				// Load sequence that was previously loaded.
				SCPIcmd = "MMEM:LOAD:DATA \"INT:\\seq" + std::to_string(varNum) + ".seq\"";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				SCPIcmd = "SOURCE1:FUNC ARB";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				SCPIcmd = "SOURCE1:FUNC:ARB \"INT:\\seq" + std::to_string(varNum) + ".seq\"";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				// Set output impedance...
				SCPIcmd = std::string("OUTPUT1:LOAD ") + AGILENT_LOAD;

				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				SCPIcmd = std::string("SOURCE1:VOLT:LOW ") + std::to_string(intensityMinMax[varNum].min) + " V";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				SCPIcmd = std::string("SOURCE1:VOLT:HIGH ") + std::to_string(intensityMinMax[varNum].max) + " V";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				SCPIcmd = "OUTPUT1 ON";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)SCPIcmd.c_str(), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				// and leave...
				viClose(Instrument);
				viClose(viDefaultRM);
			}
		}
		return 0;
	}
}
