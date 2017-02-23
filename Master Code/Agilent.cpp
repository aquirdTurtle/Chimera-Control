#include "stdafx.h"
#include "Agilent.h"
#include "constants.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <fstream>
#include "C:\PROGRAM FILES (X86)\IVI FOUNDATION\VISA\WINNT\INCLUDE\VISA.H"
#include "VariableSystem.h"
#include "ScriptStream.h"
#include "ConfigurationFileSystem.h"


IntensityWaveform::IntensityWaveform()
{
	segmentNum = 0;
	totalSequence = "";
	isVaried = false;
};


/*	* This function reads out a segment of script file and loads it into a segment to be calculated and manipulated.
	* segNum: This tells the function what the next segment # is.
	* script: this is the object to be read from.
	*/
bool IntensityWaveform::readIntoSegment(int segNum, ScriptStream& script, profileSettings profileInfo, Agilent* parent)
{
	segmentInfoInput workingInput;
	std::string intensityCommand;
	if (script.peek() == EOF)
	{
		return true;
	}
	// Grab the command type (e.g. ramp, const). Looks for newline by default.
	intensityCommand = script.getline( '\r' );
	// get rid of case sensitivity.
	if (intensityCommand == "agilent hold")
	{
		waveformSegments.resize( segNum + 1 );
		workingInput.segmentType = 0;
	}
	else if (intensityCommand == "agilent ramp")
	{
		waveformSegments.resize(segNum + 1);
		workingInput.segmentType = 1;
	}
	else if (intensityCommand == "predefined script")
	{
		std::string nestedFileName;
		nestedFileName = script.getline( '\r' );
		std::string path = profileInfo.pathIncludingCategory + nestedFileName + AGILENT_SCRIPT_EXTENSION;
		std::fstream nestedFile(path.c_str(), std::ios::in);
		if (!nestedFile.is_open())
		{
			thrower( "ERROR: tried to open a nested intensity file, but failed! The file was " + profileInfo.pathIncludingCategory
					 + nestedFileName + AGILENT_SCRIPT_EXTENSION );
			return false;
		}
		ScriptStream subStream;
		subStream << nestedFile.rdbuf();
		parent->analyzeIntensityScript( subStream, this, segNum, profileInfo);
	}
	else 
	{
		thrower( "ERROR: Intensity command not recognized. The command was \"" + intensityCommand + "\"" );
		return false;
	}

	std::string delimiter;//, tempContinuationType, tempRampType;	 
	// List of data types for variables for the different arguments:
	// initial intensity = 1
	// final intensity = 2
	// time = 3
	if (workingInput.segmentType == 1)
	{
		// this segment type means ramping.
		script >> workingInput.rampType;
		script >> workingInput.initValue;
		script >> workingInput.finValue;
	}
	else
	{
		workingInput.rampType = "nr";
		script >> workingInput.initValue;
		workingInput.finValue = workingInput.initValue;
	}
	script >> workingInput.time;
	std::string tempContinuationType;
	script >> tempContinuationType;
	if (tempContinuationType == "repeat")
	{
		// There is an extra input in this case.
		script >> workingInput.repeatNum;
	}
	else
	{
		workingInput.repeatNum = "0";
	}
	script >> delimiter;
	if (delimiter != "#")
	{
		// input number mismatch.
		thrower( "ERROR: The delimeter is missing in the Intensity script file for Segment #" + std::to_string( segNum + 1 )
				 + ". The value placed in the delimeter location was " + delimiter + " while it should have been '#'.This"
				 " indicates that either the code is not interpreting the user input incorrectly or that the user has inputted too many parameters for this type"
				 " of Segment. Use of \"Repeat\" without the number of repeats following will also trigger this error." );
		return false;
	}
	if (tempContinuationType == "repeat")
	{
		workingInput.continuationType = 0;
	}
	else if (tempContinuationType == "repeatuntiltrig")
	{
		workingInput.continuationType = 1;
	}
	else if (tempContinuationType == "once")
	{
		workingInput.continuationType = 2;
	}
	else if (tempContinuationType == "repeatforever")
	{
		workingInput.continuationType = 3;
	}
	else if (tempContinuationType == "oncewaittrig")
	{
		workingInput.continuationType = 4;
	}
	else
	{
		// string not recognized
		thrower( "ERROR: Invalid Continuation Option on intensity segment #" + std::to_string( segNum + 1 ) + ". The string entered was " + tempContinuationType
				 + ". Please enter \"Repeat #\", \"RepeatUntilTrigger\", \"OnceWaitTrig\", or \"Once\". Code should not be case-sensititve." );
	}
	// Make Everything Permanent
	waveformSegments[segNum].storeInput(workingInput);
	return false;
}

void Segment::convertInputToFinal( key variableKey, unsigned int variation)
{
	// first transfer things that can't be varied.
	finalSettings.segmentType = input.segmentType;
	finalSettings.continuationType = input.continuationType;
	finalSettings.rampType = input.rampType;
	// initial value
	try
	{
		finalSettings.initValue = std::stod( input.initValue );
	}
	catch (std::invalid_argument&)
	{
		// then check if variable.
		finalSettings.initValue = variableKey[input.initValue].first[variation];
	}
	// final value
	if (finalSettings.rampType == "nr")
	{
		finalSettings.finValue = finalSettings.initValue;
	}
	else
	{
		try
		{
			finalSettings.finValue = std::stod( input.finValue );
		}
		catch (std::invalid_argument&)
		{
			finalSettings.finValue = variableKey[input.finValue].first[variation];
		}
	}
	// time
	try
	{
		finalSettings.time = std::stod( input.time ) / 1000.0;
	}
	catch (std::invalid_argument&)
	{
		// not sure if the key would be normalized or not... don't think so
		finalSettings.time = variableKey[input.time].first[variation] / 1000.0;
	}
	// repeat number
	// (0 here corresponds to "repeat");
	if (finalSettings.continuationType == 0)
	{
		try
		{
			finalSettings.repeatNum = std::stod( input.repeatNum );
		}
		catch (std::invalid_argument&)
		{
			// not sure if the key would be normalized or not... don't think so
			finalSettings.repeatNum = variableKey[input.repeatNum].first[variation];
		}
	}
}


void Segment::storeInput( segmentInfoInput inputToSet)
{
	input = inputToSet;
}


segmentInfoInput Segment::getInput()
{
	return input;
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
		tempSegmentInfoString += std::to_string(waveformSegments[segNumInc].getFinalSettings().repeatNum) + ",";
		switch (waveformSegments[segNumInc].getFinalSettings().continuationType)
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
				thrower("ERROR: entered location in code that shouldn't be entered. Check for logic mistakes in code.");
				return;
				break;
		}
		tempSegmentInfoString += "highAtStart,4,";
	}
		
	tempSegmentInfoString += "seg" + std::to_string((totalSegNum - 1) + totalSegNum * sequenceNum) + ",";
	tempSegmentInfoString += std::to_string(waveformSegments[totalSegNum - 1].getFinalSettings().repeatNum) + ",";
	switch (waveformSegments[totalSegNum - 1].getFinalSettings().continuationType)
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
	//
	totalSequence = tempSequenceString + std::to_string( (std::to_string( tempSegmentInfoString.size() )).size() )
					+ std::to_string( tempSegmentInfoString.size() ) + tempSegmentInfoString;
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
void IntensityWaveform::replaceVarValues( key variableKey, unsigned int variation )
{
	for (int segNumInc = 0; segNumInc < waveformSegments.size(); segNumInc++)
	{
		waveformSegments[segNumInc].convertInputToFinal( variableKey, variation );
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
			double y = waveformSegments[segmentInc].returnDataVal( dataConvertInc ) * 1000.0;

			//double newValue = -a * log(y * b);
			double newValue = a + b * log( y - c );
			waveformSegments[segmentInc].assignDataVal( dataConvertInc, newValue );
		}
	}
	return;
}


/*
	* This wavefunction loops through all the data values and figures out which ones are min and max.
	*/
void IntensityWaveform::calcMinMax()
{
	// NOT DBL_MIN, which is a really small number, not a large negative number. I need a large negative number.
	maxVolt = -DBL_MAX;
	minVolt = DBL_MAX;
	for (int minMaxSegInc = 0; minMaxSegInc < waveformSegments.size(); minMaxSegInc++)
	{
		for (int minMaxDataInc = 0; minMaxDataInc < waveformSegments[minMaxSegInc].returnDataSize(); minMaxDataInc++)
		{
			if (waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc ) < minVolt)
			{
				minVolt = waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc );
			}
			if (waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc ) > maxVolt)
			{
				maxVolt = waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc );
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


segmentInfoFinal Segment::getFinalSettings()
{
	return finalSettings;
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
	input.initValue = -1;
	input.finValue = -1;
	input.time = -1;
	input.continuationType = -1;
	input.segmentType = -1;
	input.rampType = -1;
	input.repeatNum = -1;
};


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
double Segment::rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType )
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
		thrower( "ERROR: ramp type " + rampType + " is unrecognized. If this is a file name, make sure the file exists and is in the project folder.\r\n" );
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
	double numDataPointsf = finalSettings.time * AGILENT_SAMPLE_RATE;
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
	if (finalSettings.rampType == "nr" || finalSettings.rampType == "lin" || finalSettings.rampType == "tanh")
	{
		for (int dataInc = 0; dataInc < numDataPoints; dataInc++)
		{
			// constant waveform. Every data point is the same.
			dataArray.push_back( finalSettings.initValue + rampCalc(numDataPoints, dataInc, finalSettings.initValue, finalSettings.finValue, finalSettings.rampType));
		}
	}
	else if (finalSettings.rampType == "")
	{
		// Error: Ramp Type has not been set!
		thrower("ERROR: Data points tried to be written when the ramp type hadn't been set!");
		return;
	}
	else 
	{
		// invalid ramp Type!
		std::string errMsg;
		errMsg = "ERROR: Invalid Ramp Type in intensity sequence! Type entered was: " + finalSettings.rampType + ".";
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
	* return dataArray[dataNum];
	*/
double Segment::returnDataVal(long dataNum)
{
	return dataArray[dataNum];
}


/*
	* dataArray[dataNum] = val;
	*/
void Segment::assignDataVal(int dataNum, double val)
{
	dataArray[dataNum] = val;
}


/*
	* This function tells the agilent to put out the DC default waveform.
	*/
void Agilent::agilentDefault()
{
	visaOpenDefaultRM();
	visaOpen( usbAddress );
	// turn it to the default voltage...
	visaWrite( std::string( "APPLy:DC DEF, DEF, " ) + AGILENT_DEFAULT_DC );
	// and leave...
	visaClose();
	// update current values
	currentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
	currentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);
}


void Agilent::analyzeIntensityScript( ScriptStream& intensityFile, IntensityWaveform* intensityWaveformData, int& currentSegmentNumber, 
									  profileSettings profileInfo )
{
	while (!intensityFile.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = intensityWaveformData->readIntoSegment(currentSegmentNumber, intensityFile, profileInfo, this);

		if (leaveTest < 0)
		{
			thrower( "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #" + std::to_string( currentSegmentNumber ) + "." );
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
void Agilent::programIntensity( int varNum, key variableKey, bool& intensityVaried, std::vector<minMaxDoublet>& minsAndMaxes,
					   std::vector<std::vector<POINT>>& pointsToDraw, std::vector<ScriptStream>& intensityFiles, profileSettings profileInfo )
{
	// Initialize stuff
	IntensityWaveform intensityWaveformSequence;
	int currentSegmentNumber = 0;
	// connect to the agilent. I refuse to use the stupid typecasts. The way you often see these variables defined is using stupid things like ViRsc, ViUInt32, etc.
	visaOpenDefaultRM();
	visaOpen( usbAddress );
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
		analyzeIntensityScript( intensityFiles[sequenceInc], &intensityWaveformSequence, currentSegmentNumber, profileInfo );
	}
	int totalSegmentNumber = currentSegmentNumber;
	intensityVaried = intensityWaveformSequence.returnIsVaried();
	// if varied
	if (intensityWaveformSequence.returnIsVaried() == true)
	{
		// loop through # of variable values
		for (int variation = 0; variation < variableKey.size(); variation++)
		{
			intensityWaveformSequence.replaceVarValues( variableKey, variation );
			// Loop through all segments
			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// Use that information to write the data.
				intensityWaveformSequence.writeData( segNumInc );
			}
			// loop through again and calc/normalize/write values.
			intensityWaveformSequence.convertPowersToVoltages();
			intensityWaveformSequence.calcMinMax();
			minsAndMaxes.resize(variation + 1);
			minsAndMaxes[variation].max = intensityWaveformSequence.returnMaxVolt();
			minsAndMaxes[variation].min = intensityWaveformSequence.returnMinVolt();
			intensityWaveformSequence.normalizeVoltages();

			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// send to the agilent.
				visaWrite( intensityWaveformSequence.compileAndReturnDataSendString( segNumInc, variation, totalSegmentNumber ) );
				// Select the segment
				visaWrite( "SOURCE1:FUNC:ARB seg" + std::to_string( segNumInc + totalSegmentNumber * variation ) );
				// Save the segment
				visaWrite( "MMEM:STORE:DATA \"INT:\\seg" + std::to_string( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
				// increment for the next.
				visaWrite( "TRIGGER1:SLOPE POSITIVE" );
			}
			// Now handle seqeunce creation / writing.
			intensityWaveformSequence.compileSequenceString(totalSegmentNumber, variation);
			// submit the sequence
			visaWrite( intensityWaveformSequence.returnSequenceString() );
			// Save the sequence
			visaWrite( "SOURCE1:FUNC:ARB seq" + std::to_string( variation ) );
			visaWrite( "MMEM:STORE:DATA \"INT:\\seq" + std::to_string( variation ) + ".seq\"" );
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
	// 
	if (intensityIsVaried || varNum == 0)
	{
		visaOpenDefaultRM();
		visaOpen( usbAddress );
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

// open the default resource manager resource. From ni.com:
// " The viOpenDefaultRM() function must be called before any VISA operations can be invoked.The first call to this function 
// initializes the VISA system, including the Default Resource Manager resource, and also returns a session to that resource. 
// Subsequent calls to this function return unique sessions to the same Default Resource Manager resource.
// When a Resource Manager session is passed to viClose(), not only is that session closed, but also all find lists and device 
// sessions( which that Resource Manager session was used to create ) are closed.
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


void Agilent::visaSetAttribute( unsigned long attributeName, unsigned long value )
{
	if (AGILENT_SAFEMODE)
	{
		errCheck( viSetAttribute( instrument, attributeName, value ) );
	}
}


void Agilent::initialize(std::string address)
{
	usbAddress = address;
}


void Agilent::setDC(std::string level)
{
	try
	{
		double test = std::stod( level );
	}
	catch (std::invalid_argument&)
	{
		thrower( "ERROR: DC level was not a double!" );
		return;
	}
	
	visaOpenDefaultRM();
	visaOpen( std::string( usbAddress ) );
	// turn it to the default voltage...
	visaWrite( std::string( "APPLy:DC DEF, DEF, " ) + level + " V" );
	// and leave...
	visaClose();
	// update current values
	currentAgilentLow = std::stod( level );
	currentAgilentHigh = std::stod( level );
}


void Agilent::setExistingWaveform(std::string address)
{
	visaOpenDefaultRM();
	visaOpen( usbAddress );
	// Load sequence that was previously loaded.
	visaWrite( "MMEM:LOAD:DATA \"INT:\\seq" + std::to_string( varNum ) + ".seq\"" );
	visaWrite( "SOURCE1:FUNC ARB" );
	visaWrite( "SOURCE1:FUNC:ARB \"INT:\\seq" + std::to_string( varNum ) + ".seq\"" );
	// Set output impedance...
	visaWrite( std::string( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
	//visaWrite( std::string( "SOURCE1:VOLT:LOW " ) + std::to_string( intensityMinMax[varNum].min ) + " V" );
	//visaWrite( std::string( "SOURCE1:VOLT:HIGH " ) + std::to_string( intensityMinMax[varNum].max ) + " V" );
	visaWrite( "BURSt::STATe ON" );
	visaWrite( "OUTPUT1 ON" );

}

