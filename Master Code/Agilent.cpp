#include "stdafx.h"
#include "Agilent.h"
#include "constants.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
#include "C:\PROGRAM FILES (X86)\IVI FOUNDATION\VISA\WINNT\INCLUDE\VISA.H"
#include "VariableSystem.h"
#include "ScriptStream.h"
#include "ConfigurationFileSystem.h"
#include "MasterWindow.h"


ScriptedAgilentWaveform::ScriptedAgilentWaveform()
{
	segmentNum = 0;
	totalSequence = "";
	isVaried = false;
};


/*	* This function reads out a segment of script file and loads it into a segment to be calculated and manipulated.
	* segNum: This tells the function what the next segment # is.
	* script: this is the object to be read from.
	*/
bool ScriptedAgilentWaveform::readIntoSegment(int segNum, ScriptStream& script, profileSettings profileInfo, Agilent* parent)
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
		parent->analyzeAgilentScript( subStream, this, segNum, profileInfo);
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
std::string ScriptedAgilentWaveform::compileAndReturnDataSendString(int segNum, int varNum, int totalSegNum)
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
void ScriptedAgilentWaveform::writeData(int segNum)
{
	waveformSegments[segNum].calcData();
}


/*
	* This function compiles the sequence string which tells the agilent what waveforms to output when and with what trigger control. The sequence is stored
	* as a part of the class.
	*/
void ScriptedAgilentWaveform::compileSequenceString(int totalSegNum, int sequenceNum)
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
			thrower("ERROR: entered location in code that shouldn't be entered. Check for logic mistakes in code.");
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
std::string ScriptedAgilentWaveform::returnSequenceString()
{
	return totalSequence;
}


/*
	* This function returns the truth of whether this sequence is being varied or not. This gets determined during the reading process.
	*/
bool ScriptedAgilentWaveform::returnIsVaried()
{
	return isVaried;
}


/*
	* This waveform loops through all of the segments to find places where a variable value needs to be changed, and changes it.
	*/
void ScriptedAgilentWaveform::replaceVarValues( key variableKey, unsigned int variation )
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
void ScriptedAgilentWaveform::convertPowersToVoltages()
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
	/// newValue = a +  b * log(y - info); // here info is treated as a background light level, and the voltage output should be positive
	
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
void ScriptedAgilentWaveform::calcMinMax()
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
void ScriptedAgilentWaveform::normalizeVoltages()
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
double ScriptedAgilentWaveform::returnMaxVolt()
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
double ScriptedAgilentWaveform::returnMinVolt()
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
void Agilent::agilentDefault( int channel )
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


void Agilent::analyzeAgilentScript( ScriptStream& intensityFile, ScriptedAgilentWaveform* intensityWaveformData, int& currentSegmentNumber, 
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
	* programScript opens the intensity file, reads the contents, loads them into an appropriate data structure, then from this data structure writes
	* segment and sequence information to the function generator.
	*/
void Agilent::programScript( int varNum, key variableKey, std::vector<ScriptStream>& intensityFiles, profileSettings profileInfo )
{
	// Initialize stuff
	ScriptedAgilentWaveform intensityWaveformSequence;
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
		analyzeAgilentScript( intensityFiles[sequenceInc], &intensityWaveformSequence, currentSegmentNumber, profileInfo );
	}
	int totalSegmentNumber = currentSegmentNumber;
	isVaried = intensityWaveformSequence.returnIsVaried();
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
			ranges.resize( variation + 1 );
			ranges[variation].max = intensityWaveformSequence.returnMaxVolt();
			ranges[variation].min = intensityWaveformSequence.returnMinVolt();
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
		ranges.resize( 1 );
		ranges[0].max = intensityWaveformSequence.returnMaxVolt();
		ranges[0].min = intensityWaveformSequence.returnMinVolt();
		intensityWaveformSequence.normalizeVoltages();

		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Set output impedance...
			visaWrite( std::string( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
			// set range of voltages...
			visaWrite( std::string( "SOURCE1:VOLT:LOW " ) + std::to_string( ranges[0].min ) + " V" );
			visaWrite( std::string( "SOURCE1:VOLT:HIGH " ) + std::to_string( ranges[0].max ) + " V" );
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
	* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
	*/
void Agilent::selectIntensityProfile(int varNum)
{
	// TODO: add checks for differnet types of programmings.
	// 
	if (isVaried || varNum == 0)
	{
		visaOpenDefaultRM();
		visaOpen( usbAddress );
		// Load sequence that was previously loaded.
		visaWrite("MMEM:LOAD:DATA \"INT:\\seq" + std::to_string(varNum) + ".seq\"");
		visaWrite( "SOURCE1:FUNC ARB");
		visaWrite( "SOURCE1:FUNC:ARB \"INT:\\seq" + std::to_string(varNum) + ".seq\"");
		// Set output impedance...
		visaWrite( std::string("OUTPUT1:LOAD ") + AGILENT_LOAD);
		visaWrite( std::string("SOURCE1:VOLT:LOW ") + std::to_string(ranges[varNum].min) + " V");
		visaWrite( std::string("SOURCE1:VOLT:HIGH ") + std::to_string(ranges[varNum].max) + " V");
		visaWrite( "OUTPUT1 ON" );
		// and leave...
		visaClose();
	}
	return;
}


std::string Agilent::getDeviceIdentity()
{
	std::string msg = visaIdentityQuery();
	if ( msg == "" )
	{
		msg = "Disconnected...\n";
	}
	return msg;
}

void Agilent::initialize( POINT& loc, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id,
						  std::string address, std::string headerText )
{
	usbAddress = address;
	try
	{
		visaOpenDefaultRM();
		visaOpen( usbAddress );
		int errCode = 0;
		deviceInfo = visaIdentityQuery();
		isConnected = true;
	}
	catch (myException& except)
	{
		deviceInfo = "Disconnected";
		isConnected = false;
	}
	
	header.position = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.ID = id++;
	header.Create( headerText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.position, master, header.ID );
	header.SetFont( CFont::FromHandle(sHeadingFont) );

	deviceInfoDisplay.position = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	deviceInfoDisplay.ID = id++;
	deviceInfoDisplay.Create(deviceInfo.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, deviceInfoDisplay.position, master, deviceInfoDisplay.ID );
	deviceInfoDisplay.SetFont( CFont::FromHandle( sSmallFont ) );

	channel1Button.position = { loc.x, loc.y, loc.x += 160, loc.y + 20 };
	channel1Button.ID = id++;
	if (channel1Button.ID != IDC_TOP_BOTTOM_CHANNEL1_BUTTON && channel1Button.ID != IDC_AXIAL_UWAVE_CHANNEL1_BUTTON && channel1Button.ID != IDC_FLASHING_CHANNEL1_BUTTON)
	{
		throw;
	}
	channel1Button.Create( "Channel 1", BS_AUTORADIOBUTTON | WS_GROUP | WS_VISIBLE | WS_CHILD, channel1Button.position, master, channel1Button.ID );
	channel1Button.SetCheck( true );

	channel2Button.position = { loc.x, loc.y, loc.x += 160, loc.y + 20 };
	channel2Button.ID = id++;
	if (channel2Button.ID != IDC_TOP_BOTTOM_CHANNEL2_BUTTON && channel2Button.ID != IDC_AXIAL_UWAVE_CHANNEL2_BUTTON && channel2Button.ID != IDC_FLASHING_CHANNEL2_BUTTON)
	{
		throw;
	}
	channel2Button.Create( "Channel 2", BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD, channel2Button.position, master, channel2Button.ID );

	syncedButton.position = { loc.x, loc.y, loc.x += 160, loc.y += 20 };
	syncedButton.ID = id++;
	if (syncedButton.ID != IDC_TOP_BOTTOM_SYNC_BUTTON && syncedButton.ID != IDC_AXIAL_UWAVE_SYNC_BUTTON && syncedButton.ID != IDC_FLASHING_SYNC_BUTTON)
	{
		throw;
	}
	syncedButton.Create( "Synced?", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, syncedButton.position, master, syncedButton.ID );
	
	loc.x -= 480;

	settingCombo.position = { loc.x, loc.y, loc.x += 240, loc.y + 200 };
	settingCombo.ID = id++;	
	if (settingCombo.ID != IDC_TOP_BOTTOM_AGILENT_COMBO && settingCombo.ID != IDC_AXIAL_UWAVE_AGILENT_COMBO
      		 && settingCombo.ID != IDC_FLASHING_AGILENT_COMBO)
	{
		throw;
	}
	settingCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, settingCombo.position, master, settingCombo.ID );
	settingCombo.AddString( "No Control" );
	settingCombo.AddString( "Output Off" );
	settingCombo.AddString( "DC Output" );
	settingCombo.AddString( "Single Frequency Output" );
	settingCombo.AddString( "Square Output" );
	settingCombo.AddString( "Preloaded Arbitrary Waveform" );
	settingCombo.AddString( "Scripted Arbitrary Waveform" );
	settingCombo.SetCurSel( 0 );

	optionsFormat.position = { loc.x, loc.y, loc.x += 240, loc.y += 25 };
	optionsFormat.ID = id++;
	optionsFormat.Create( "---", WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsFormat.position, 
						  master, optionsFormat.ID );
	loc.x -= 480;

	optionsEdit.position = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	optionsEdit.ID = id++;
	optionsEdit.Create( WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsEdit.position, master, optionsEdit.ID );
	
	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
	currentChannel = 1;
}


void Agilent::handleInput(int chan)
{
	chan -= 1;
	CString text;
	optionsEdit.GetWindowTextA(text);
	std::string textStr( text );
	ScriptStream stream;
	stream << textStr;
	stream.seekg( 0 );		
	switch (settings.channel[chan].option)
	{
		case -2:
			// no control.
			break;
		case -1:
			// output off.
			break;
		case 0:
			// DC.
			stream >> settings.channel[chan].dc.dcLevelInput;
			break;
		case 1:
			// sine wave
			stream >> settings.channel[chan].sine.frequencyInput;
			stream >> settings.channel[chan].sine.amplitudeInput;
			break;
		case 2:
			stream >> settings.channel[chan].square.frequencyInput;
			stream >> settings.channel[chan].square.amplitudeInput;
			stream >> settings.channel[chan].square.offsetInput;
			break;
		case 3:
			stream >> settings.channel[chan].preloadedArb.address;
			break;
		case 4:
			thrower( "ERROR: attempted to use a scripted agilent sequence. This feature has"
						" yet to be implemented on this code." );
			break;
		default:
			thrower( "ERROR: unknown agilent option" );
	}
}


void Agilent::handleInput()
{
	handleInput( (!channel1Button.GetCheck()) + 1);
}



void Agilent::updateEdit(int chan)
{
	chan -= 1;
	std::string str;
	switch ( settings.channel[chan].option )
	{
		case -2:
			str = "";
			settingCombo.SetCurSel( 0 );
			break;
		case -1:
			str = "";
			settingCombo.SetCurSel( 1 );
			break;
		case 0:
			// dc
			str = settings.channel[chan].dc.dcLevelInput;
			settingCombo.SetCurSel( 2 );
			break;
		case 1:
			// sine
			str = settings.channel[chan].sine.frequencyInput + " " + settings.channel[chan].sine.amplitudeInput;
			settingCombo.SetCurSel( 3 );
			break;
		case 2:
			// square
			str = settings.channel[chan].square.frequencyInput + " " + settings.channel[chan].square.amplitudeInput
				+ " " + settings.channel[chan].square.offsetInput;
			settingCombo.SetCurSel( 4 );
			break;
		case 3:
			// preprogrammed
			str = settings.channel[chan].preloadedArb.address;
			settingCombo.SetCurSel( 5 );
			break;
		case 4:
			// scripted
			// ???
			settingCombo.SetCurSel( 6 );
			break;
		default:
			thrower( "ERROR: unrecognized agilent setting: " + std::to_string( settings.channel[chan].option ) );
	}
	if ( chan == 0 )
	{
		channel1Button.SetCheck( true );
		channel2Button.SetCheck( false );
	}
	else
	{
		channel1Button.SetCheck( false );
		channel2Button.SetCheck( true );
	}
	optionsEdit.SetWindowTextA( str.c_str() );

}


void Agilent::handleChannelPress( int chan )
{
	// convert from channel 1/2 to 0/1 to access the right array entr
	handleInput(currentChannel);
	updateEdit(chan);
	if (channel1Button.GetCheck())
	{
		currentChannel = 1;
	}
	else
	{
		currentChannel = 2;
	}
}


void Agilent::handleCombo()
{
	int selection = settingCombo.GetCurSel();
	int selectedChannel = int( !channel1Button.GetCheck() );
	switch (selection)
	{
		case 0:
			// do nothing
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = -2;
			break;
		case 1:
			// do nothing
			optionsFormat.SetWindowTextA( "---" );
			settings.channel[selectedChannel].option = -1;
			break;
		case 2:
			optionsFormat.SetWindowTextA( "[DC Level]" );
			settings.channel[selectedChannel].option = 0;
			break;
		case 3:
			optionsFormat.SetWindowTextA( "[Frequency] [Amplitude]" );
			settings.channel[selectedChannel].option = 1;
			break;
		case 4:
			optionsFormat.SetWindowTextA( "[Frequency] [Amplitude] [Offset]" );
			settings.channel[selectedChannel].option = 2;
			break;
		case 5:
			optionsFormat.SetWindowTextA( "[Address]" );
			settings.channel[selectedChannel].option = 3;
			break;
		case 6:
			optionsFormat.SetWindowTextA( "[Not yet implemented]" );
			settings.channel[selectedChannel].option = 4;
			break;
	}
}


deviceOutputInfo Agilent::getOutputInfo()
{
	return settings;
}


void Agilent::convertInputToFinalSettings( key variableKey, unsigned int variation )
{
	// iterate between 0 and 1...
	try
	{
		for (auto chan : range( 2 ))
		{
			switch (settings.channel[chan].option)
			{
				case -2:
					// no control
					break;
				case -1:
					// no ouput
					break;
				case 0:
					// DC output
					try
					{
						settings.channel[chan].dc.dcLevel = std::stod( settings.channel[chan].dc.dcLevelInput );
					}
					catch (std::invalid_argument&)
					{
						settings.channel[chan].dc.dcLevel = variableKey[settings.channel[chan].dc.dcLevelInput]
							.first[variation];
					}
					break;
				case 1:
					// single frequency output
					// frequency
					try
					{
						settings.channel[chan].sine.frequency = std::stod( settings.channel[chan].sine.frequencyInput );
					}
					catch (std::invalid_argument&)
					{
						settings.channel[chan].sine.frequency = variableKey[settings.channel[chan].sine.frequencyInput].first[variation];
					}
					// amplitude
					try
					{
						settings.channel[chan].sine.amplitude = std::stod( settings.channel[chan].sine.amplitudeInput );
					}
					catch (std::invalid_argument&)
					{
						settings.channel[chan].sine.amplitude = variableKey[settings.channel[chan].sine.amplitudeInput].first[variation];
					}
					break;
				case 2:
					// Square Output
					// frequency
					try
					{
						settings.channel[chan].square.frequency = std::stod( settings.channel[chan].square.frequencyInput );
					}
					catch (std::invalid_argument&)
					{
						settings.channel[chan].square.frequency = variableKey[settings.channel[chan].square.frequencyInput].first[variation];
					}
					// amplitude
					try
					{
						settings.channel[chan].square.amplitude = std::stod( settings.channel[chan].square.amplitudeInput );
					}
					catch (std::invalid_argument&)
					{
						settings.channel[chan].square.amplitude = variableKey[settings.channel[chan].square.amplitudeInput].first[variation];
					}

					break;
				case 3:
					// Preloaded Arb Output
					break;
				case 4:
					// Scripted Arb Output
					break;
				default:
					thrower( "Unrecognized Agilent Setting: " + std::to_string( settings.channel[chan].option ) );
			}
		}
	}
	catch (std::out_of_range&)
	{
		thrower( "ERROR: unrecognized variable!" );
	}
}


/*
This function outputs a string that contains all of the information that is set by the user for a given configuration. 
*/
std::string Agilent::getConfigurationString()
{
	// make sure data is up to date.
	handleInput();
	// start outputting.
	std::stringstream info;
	info << "AGILENTVERSION 1.0\n";
	info << std::to_string(settings.synced); 
	info << "\nCHANNEL 1\n";
	info << std::to_string(settings.channel[0].option) + ", ";
	info << settings.channel[0].dc.dcLevelInput + ", ";
	info << settings.channel[0].sine.amplitudeInput + ", ";
	info << settings.channel[0].sine.frequencyInput + ", ";
	info << settings.channel[0].square.amplitudeInput + ", ";
	info << settings.channel[0].square.frequencyInput + ", ";
	info << settings.channel[0].square.offsetInput + ", ";
	info << settings.channel[0].preloadedArb.address + ", ";
	info << "\nCHANNEL 2\n";
	info << std::to_string( settings.channel[1].option ) + ", ";
	info << settings.channel[1].dc.dcLevelInput + ", ";
	info << settings.channel[1].sine.amplitudeInput + ", ";
	info << settings.channel[1].sine.frequencyInput + ", ";
	info << settings.channel[1].square.amplitudeInput + ", ";
	info << settings.channel[1].square.frequencyInput + ", ";
	info << settings.channel[1].square.offsetInput + ", ";
	info << settings.channel[1].preloadedArb.address + ", \n";
	return info.str();
}


void Agilent::zeroSettings()
{
	settings = deviceOutputInfo();
	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
}


void Agilent::readConfigurationFile( std::ifstream& file )
{
	std::string input;
	int pos = file.tellg();
	file >> input;
	if ( input != "AGILENTVERSION" )
	{
		// assume old version of file which did not include agilent info.
		// return to the original position and return silently.
		file.seekg( pos );
		zeroSettings();
		updateEdit( 1 );
		return;
	}
	
	file >> input;

	if ( input != "1.0" )
	{
		thrower( "ERROR: Unexpected version for agilent info in config file!" );
	}
	file >> settings.synced;
	file.get();
	std::getline( file, input );
	if ( input != "CHANNEL 1" )
	{
		thrower( "ERROR: Expected \"CHANNEL 1\" but instead found " + input + " inside configuration file." );
	}
	// the extra step in all of the following is to remove the , at the end of each input.
	file >> input;
	try
	{
		settings.channel[0].option = std::stoi( input.substr( 0, input.size() - 1 ) );
	}
	catch (std::invalid_argument&)
	{
		thrower( "ERROR: Bad channel 1 option!" );
	}
	file >> input;
	settings.channel[0].dc.dcLevelInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[0].sine.amplitudeInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[0].sine.frequencyInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[0].square.amplitudeInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[0].square.frequencyInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[0].square.offsetInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[0].preloadedArb.address = input.substr( 0, input.size() - 1 );
	file.get(); 
	file.get();
	std::getline( file, input );
	if ( input != "CHANNEL 2" )
	{
		thrower( "ERROR: Expected \"CHANNEL 2\" but instead found " + input + " inside configuration file." );
	}
	file >> input;
	settings.channel[1].option = std::stoi( input.substr( 0, input.size() - 1 ) );
	file >> input;
	settings.channel[1].dc.dcLevelInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[1].sine.amplitudeInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[1].sine.frequencyInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[1].square.amplitudeInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[1].square.frequencyInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[1].square.offsetInput = input.substr( 0, input.size() - 1 );
	file >> input;
	settings.channel[1].preloadedArb.address = input.substr( 0, input.size() - 1 );
	// default to first channel.
	updateEdit( 1 );
	return;
}


void Agilent::outputOff( int channel )
{
	channel++;
	visaOpenDefaultRM();
	visaOpen( std::string( usbAddress ) );
	if (channel == 1)
	{
		visaWrite( "OUTPUT1 OFF" );
	}
	else if (channel == 2)
	{
		visaWrite( "OUTPUT2 OFF" );
	}
	else
	{
		thrower( "ERROR: Attempted to turn off channel " + std::to_string( channel ) + " which does not exist! Use channel 1 or 2." );
	}
	visaClose();
}


bool Agilent::connected()
{
	return isConnected;
}


void Agilent::setDC( int channel, dcInfo info )
{
	channel++;
	visaOpenDefaultRM();
	visaOpen( std::string( usbAddress ) );
	if (channel == 1)
	{
		visaWrite( "SOURce1:APPLy:DC DEF, DEF, " + std::to_string( info.dcLevel ) + " V" );
		chan1Range.min = chan1Range.max = info.dcLevel;
	}
	else if (channel == 2)
	{
		visaWrite( "SOURce2:APPLy:DC DEF, DEF, " + std::to_string( info.dcLevel ) + " V" );
		chan2Range.min = chan2Range.max = info.dcLevel;
	}
	else
	{
		thrower( "tried to set DC level for \"channel\" " + std::to_string( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	// and leave...
	visaClose();
}


void Agilent::setExistingWaveform( int channel, preloadedArbInfo info )
{
	channel++;
	visaOpenDefaultRM();
	visaOpen( usbAddress );
	if (channel == 1)
	{
		// Load sequence that was previously loaded.
		visaWrite( "MMEM:LOAD:DATA \"" + info.address + "\"" );
		// tell it that it's outputting something arbitrary (not sure if necessary)
		visaWrite( "SOURCE1:FUNC ARB" );
		// tell it what arb it's outputting.
		visaWrite( "SOURCE1:FUNC:ARB \"" + info.address + "\"" );
		// Set output impedance...
		visaWrite( std::string( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
		// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
		visaWrite( "SOURCE1:BURST::MODE TRIGGERED" );
		visaWrite( "SOURCE1:BURST::NCYCLES 1" );
		visaWrite( "SOURCE1:BURST::PHASE 0" );
		// 
		visaWrite( "SOURCE1:BURST::STATE ON" );
		visaWrite( "OUTPUT1 ON" );
	}
	else if (channel == 2)
	{
		// Load sequence that was previously loaded onto the agilent's non-volitile memory.
		visaWrite( "MMEM:LOAD:DATA \"" + info.address + "\"" );
		// tell it that it's outputting something arbitrary (not sure if necessary)
		visaWrite( "SOURCE2:FUNC ARB" );
		// tell it what arb it's outputting.
		visaWrite( "SOURCE2:FUNC:ARB \"" + info.address + "\"" );
		// not really bursting... but this allows us to reapeat on triggers. Probably another way to do this.
		visaWrite( "SOURCE2:BURST::MODE TRIGGERED" );
		visaWrite( "SOURCE2:BURST::NCYCLES 1" );
		visaWrite( "SOURCE2:BURST::PHASE 0" );
		visaWrite( "SOURCE2:BURST::STATE ON" );
		// Set output impedance...
		visaWrite( std::string( "OUTPUT2:LOAD " ) + AGILENT_LOAD );
		visaWrite( "OUTPUT2 ON" );

	}
	else
	{
		thrower( "tried to set arbitrary function for \"channel\" " + std::to_string( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	visaClose();
}

void Agilent::setSquare( int channel, squareInfo info )
{
	channel++;
	visaOpenDefaultRM();
	visaOpen( usbAddress );
	if (channel == 1)
	{
		visaWrite( "SOURCE1:APPLY:SQUARE " + std::to_string(info.frequency) + " KHZ, " 
				   + std::to_string(info.amplitude) + " VPP, " + std::to_string(info.offset) + " V" );
	}
	else if (channel == 2)
	{
		visaWrite( "SOURCE2:APPLY:SQUARE " + std::to_string( info.frequency ) + " KHZ, "
				   + std::to_string( info.amplitude ) + " VPP, " + std::to_string( info.offset ) + " V" );
	}
	else
	{
		thrower( "tried to set square function for \"channel\" " + std::to_string( channel ) + ", which is not supported! "
				 "Channel should be either 1 or 2" );
	}
	visaClose();
}

void Agilent::setSingleFreq( int channel, sineInfo info )
{

}

/// 

/// 

void Agilent::visaWrite( std::string message )
{
	// not sure what this is for.
	unsigned long actual;
	if (!AGILENT_SAFEMODE)
	{
		errCheck( viWrite( instrument, (unsigned char*)message.c_str(), (ViUInt32)message.size(), &actual ) );
	}
}

void Agilent::visaClose()
{
	if (!AGILENT_SAFEMODE)
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
	if (!AGILENT_SAFEMODE)
	{
		errCheck( viOpenDefaultRM( &defaultResourceManager ) );
	}
}


void Agilent::visaOpen( std::string address )
{
	if (!AGILENT_SAFEMODE)
	{
		errCheck( viOpen( defaultResourceManager, (char *)address.c_str(), VI_NULL, VI_NULL, &instrument ) );
	}
}


void Agilent::visaSetAttribute( unsigned long attributeName, unsigned long value )
{
	if (!AGILENT_SAFEMODE)
	{
		errCheck( viSetAttribute( instrument, attributeName, value ) );
	}
}


std::string Agilent::visaIdentityQuery()
{
	char buf[256] = { 0 };
	if (!AGILENT_SAFEMODE)
	{
		viQueryf( instrument, (ViString)"*IDN?\n", "%t", buf );
	}
	else
	{
		return "Agilent Safemode...";
	}
	return buf;
}


void Agilent::visaErrQuery( std::string& errMsg, long& errCode )
{
	char buf[256] = { 0 };
	if (!AGILENT_SAFEMODE)
	{
		viQueryf( instrument, (ViString)"SYST:ERR?\n", "%ld,%t", &errCode, buf );
	}
	else
	{
		return;
	}
	errMsg = std::string( buf );
}

/*
* This function checks if the agilent throws an error or if there is an error communicating with the agilent.
*/
void Agilent::errCheck( long status )
{
	long errorCode = 0;
	// Check comm status
	if (status < 0)
	{
		// Error detected.
		thrower( "ERROR: Communication error with agilent. Error Code: " + std::to_string( status ) + "\r\n" );
		return;
	}
	// Query the agilent for errors.
	std::string errMessage;
	visaErrQuery( errMessage, errorCode );
	//viQueryf( instrument, "SYST:ERR?\n", "%ld,%t", &errorCode, buf );
	if (errorCode != 0)
	{
		// Agilent error
		thrower( "ERROR: agilent returned error message: " + std::to_string( errorCode ) + ":" + errMessage );
		return;
	}
	return;
}


void Agilent::visaPrintf( std::string msg )
{

	if (!AGILENT_SAFEMODE)
	{
		errCheck( viPrintf( instrument, (ViString)msg.c_str() ) );
	}
}

