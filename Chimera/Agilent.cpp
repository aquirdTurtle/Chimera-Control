#include "stdafx.h"
#include "boost/cast.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>

#include "C:\PROGRAM FILES (X86)\IVI FOUNDATION\VISA\WINNT\INCLUDE\VISA.H"

#include "Agilent.h"
#include "VariableSystem.h"
#include "ScriptStream.h"
#include "ConfigurationFileSystem.h"
#include "DeviceWindow.h"

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
bool ScriptedAgilentWaveform::readIntoSegment(int segNum, ScriptStream& script, Agilent* parent)
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
		thrower( "ERROR: The delimeter is missing in the Intensity script file for Segment #" + str( segNum + 1 )
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
		thrower( "ERROR: Invalid Continuation Option on intensity segment #" + str( segNum + 1 ) + ". The string entered was " + tempContinuationType
				 + ". Please enter \"Repeat #\", \"RepeatUntilTrigger\", \"OnceWaitTrig\", or \"Once\". Code should not be case-sensititve." );
	}
	// Make Everything Permanent
	waveformSegments[segNum].storeInput(workingInput);
	return false;
}

void Segment::convertInputToFinal( key variableKey, UINT variation)
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
	tempSendString = "DATA:ARB seg" + str(segNum + totalSegNum * varNum) + ",";
	// need to handle last one separately so that I can /not/ put a comma after it.
	for (int sendDataInc = 0; sendDataInc < waveformSegments[segNum].returnDataSize() - 1; sendDataInc++)
	{
		tempSendString += str(waveformSegments[segNum].returnDataVal(sendDataInc));
		tempSendString += ", ";
	}
	tempSendString += str(waveformSegments[segNum].returnDataVal(waveformSegments[segNum].returnDataSize() - 1));
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
	tempSegmentInfoString = "seq" + str(sequenceNum) + ",";
	if ( totalSegNum == 0 )
	{
		return;
	}
	for (int segNumInc = 0; segNumInc < totalSegNum - 1; segNumInc++)
	{
		// Format is 
		tempSegmentInfoString += "seg" + str(segNumInc + totalSegNum * sequenceNum) + ",";
		tempSegmentInfoString += str(waveformSegments[segNumInc].getFinalSettings().repeatNum) + ",";
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
		}
		tempSegmentInfoString += "highAtStart,4,";
	}
		
	tempSegmentInfoString += "seg" + str((totalSegNum - 1) + totalSegNum * sequenceNum) + ",";
	tempSegmentInfoString += str(waveformSegments[totalSegNum - 1].getFinalSettings().repeatNum) + ",";
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
	}
	tempSegmentInfoString += "highAtStart,4";
	//
	totalSequence = tempSequenceString + str( (str( tempSegmentInfoString.size() )).size() )
					+ str( tempSegmentInfoString.size() ) + tempSegmentInfoString;
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
void ScriptedAgilentWaveform::replaceVarValues( key variableKey, UINT variation )
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
	}
	else 
	{
		// invalid ramp Type!
		std::string errMsg;
		errMsg = "ERROR: Invalid Ramp Type in intensity sequence! Type entered was: " + finalSettings.rampType + ".";
		thrower(errMsg);
	}
}


/*
 * return boost::numeric_cast<long>(dataArray.size());
 */
UINT Segment::returnDataSize()
{
	return dataArray.size();
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

void Agilent::rearrange(UINT width, UINT height, fontMap fonts)
{
	// GUI ELEMENTS
	header.rearrange("", "", width, height, fonts);
	deviceInfoDisplay.rearrange("", "", width, height, fonts);
	channel1Button.rearrange("", "", width, height, fonts);
	channel2Button.rearrange("", "", width, height, fonts);
	syncedButton.rearrange("", "", width, height, fonts);
	settingCombo.rearrange("", "", width, height, fonts);
	optionsFormat.rearrange("", "", width, height, fonts);
	optionsEdit.rearrange("", "", width, height, fonts);

}

/*
	* This function tells the agilent to put out the DC default waveform.
	*/
void Agilent::agilentDefault( int channel )
{
	visaOpenDefaultRM();
	visaOpen( usbAddress );
	// turn it to the default voltage...
	visaWrite( str( "APPLy:DC DEF, DEF, " ) + AGILENT_DEFAULT_DC );
	// and leave...
	visaClose();
	// update current values
	currentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
	currentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);
}


void Agilent::analyzeAgilentScript( ScriptStream& intensityFile, ScriptedAgilentWaveform* intensityWaveformData, 
								   int& currentSegmentNumber, profileSettings profileInfo )
{
	while (!intensityFile.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = intensityWaveformData->readIntoSegment(currentSegmentNumber, intensityFile, this);

		if (leaveTest < 0)
		{
			thrower( "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #" 
					+ str( currentSegmentNumber ) + "." );
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
	visaWrite( "SOURCE1:FUNC:ARB:SRATE " + str( AGILENT_SAMPLE_RATE ) );
	// Set filtering state
	visaWrite( str( "SOURCE1:FUNC:ARB:FILTER " ) + AGILENT_FILTER_STATE );
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
				visaWrite( "SOURCE1:FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
				// Save the segment
				visaWrite( "MMEM:STORE:DATA \"INT:\\seg" + str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
				// increment for the next.
				visaWrite( "TRIGGER1:SLOPE POSITIVE" );
			}
			// Now handle seqeunce creation / writing.
			intensityWaveformSequence.compileSequenceString(totalSegmentNumber, variation);
			// submit the sequence
			visaWrite( intensityWaveformSequence.returnSequenceString() );
			// Save the sequence
			visaWrite( "SOURCE1:FUNC:ARB seq" + str( variation ) );
			visaWrite( "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
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
			visaWrite( str( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
			// set range of voltages...
			visaWrite( str( "SOURCE1:VOLT:LOW " ) + str( ranges[0].min ) + " V" );
			visaWrite( str( "SOURCE1:VOLT:HIGH " ) + str( ranges[0].max ) + " V" );
			// send to the agilent.
			visaWrite( intensityWaveformSequence.compileAndReturnDataSendString( segNumInc, 0, totalSegmentNumber ) );
			// Select the segment
			visaWrite( "SOURCE1:FUNC:ARB seg" + str( segNumInc ) );
			// Save the segment
			visaWrite( "MMEM:STORE:DATA \"INT:\\seg" + str( segNumInc ) + ".arb\"" );
			// increment for the next.
		}


		// Now handle seqeunce creation / writing.
		intensityWaveformSequence.compileSequenceString(totalSegmentNumber, 0);
		// submit the sequence
		visaWrite( intensityWaveformSequence.returnSequenceString() );
		// Save the sequence
		visaWrite( "SOURCE1:FUNC:ARB seq" + str( 0 ) );
		visaWrite( "MMEM:STORE:DATA \"INT:\\seq" + str( 0 ) + ".seq\"" );
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
		visaWrite("MMEM:LOAD:DATA \"INT:\\seq" + str(varNum) + ".seq\"");
		visaWrite( "SOURCE1:FUNC ARB");
		visaWrite( "SOURCE1:FUNC:ARB \"INT:\\seq" + str(varNum) + ".seq\"");
		// Set output impedance...
		visaWrite( str("OUTPUT1:LOAD ") + AGILENT_LOAD);
		visaWrite( str("SOURCE1:VOLT:LOW ") + str(ranges[varNum].min) + " V");
		visaWrite( str("SOURCE1:VOLT:HIGH ") + str(ranges[varNum].max) + " V");
		visaWrite( "OUTPUT1 ON" );
		// and leave...
		visaClose();
	}
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

void Agilent::initialize( POINT& loc, cToolTips& toolTips, DeviceWindow* master, int& id,
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
	catch (Error&)
	{
		deviceInfo = "Disconnected";
		isConnected = false;
	}
	
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( cstr(headerText), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, master, id++ );
	header.fontType = HeadingFont;

	deviceInfoDisplay.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	deviceInfoDisplay.Create( cstr(deviceInfo), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, deviceInfoDisplay.sPos, 
							 master, id++ );
	deviceInfoDisplay.fontType = SmallFont;

	channel1Button.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 20 };
	channel1Button.Create( "Channel 1", BS_AUTORADIOBUTTON | WS_GROUP | WS_VISIBLE | WS_CHILD, channel1Button.sPos,
						  master, id++ );
	idVerify(channel1Button, IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_AXIAL_UWAVE_CHANNEL1_BUTTON, 
			 IDC_FLASHING_CHANNEL1_BUTTON);
	channel1Button.SetCheck( true );

	channel2Button.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 20 };
	channel2Button.Create( "Channel 2", BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD, channel2Button.sPos, master, id++ );
	idVerify(channel2Button, IDC_TOP_BOTTOM_CHANNEL2_BUTTON, IDC_AXIAL_UWAVE_CHANNEL2_BUTTON,
			 IDC_FLASHING_CHANNEL2_BUTTON);

	syncedButton.sPos = { loc.x, loc.y, loc.x += 160, loc.y += 20 };
	syncedButton.Create( "Synced?", BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, syncedButton.sPos, master, id++ );
	idVerify(syncedButton, IDC_TOP_BOTTOM_SYNC_BUTTON, IDC_AXIAL_UWAVE_SYNC_BUTTON, IDC_FLASHING_SYNC_BUTTON);

	loc.x -= 480;

	settingCombo.sPos = { loc.x, loc.y, loc.x += 240, loc.y + 200 };
	settingCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 
						settingCombo.sPos, master, id++ );
	idVerify(settingCombo, IDC_TOP_BOTTOM_AGILENT_COMBO, IDC_AXIAL_UWAVE_AGILENT_COMBO, IDC_FLASHING_AGILENT_COMBO);
	settingCombo.AddString( "No Control" );
	settingCombo.AddString( "Output Off" );
	settingCombo.AddString( "DC Output" );
	settingCombo.AddString( "Single Frequency Output" );
	settingCombo.AddString( "Square Output" );
	settingCombo.AddString( "Preloaded Arbitrary Waveform" );
	settingCombo.AddString( "Scripted Arbitrary Waveform" );
	settingCombo.SetCurSel( 0 );

	optionsFormat.sPos = { loc.x, loc.y, loc.x += 240, loc.y += 25 };
	optionsFormat.Create( "---", WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsFormat.sPos, master, id++ );
	loc.x -= 480;

	optionsEdit.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 120 };
	optionsEdit.Create( WS_CHILD | WS_VISIBLE | SS_SUNKEN, optionsEdit.sPos, master, id++ );
	
	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
	currentChannel = 1;
}


HBRUSH Agilent::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if ( controlID == deviceInfoDisplay.GetDlgCtrlID() || controlID == channel1Button.GetDlgCtrlID() 
		 || controlID == channel2Button.GetDlgCtrlID() || controlID == syncedButton.GetDlgCtrlID() 
		 || controlID == settingCombo.GetDlgCtrlID() || controlID == optionsFormat.GetDlgCtrlID()
		 || controlID == optionsEdit.GetDlgCtrlID() )
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["White"]);
		return *brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
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
	std::string tempStr;
	switch ( settings.channel[chan].option )
	{
		case -2:
			tempStr = "";
			settingCombo.SetCurSel( 0 );
			break;
		case -1:
			tempStr = "";
			settingCombo.SetCurSel( 1 );
			break;
		case 0:
			// dc
			tempStr = settings.channel[chan].dc.dcLevelInput;
			settingCombo.SetCurSel( 2 );
			break;
		case 1:
			// sine
			tempStr = settings.channel[chan].sine.frequencyInput + " " + settings.channel[chan].sine.amplitudeInput;
			settingCombo.SetCurSel( 3 );
			break;
		case 2:
			// square
			tempStr = settings.channel[chan].square.frequencyInput + " " + settings.channel[chan].square.amplitudeInput
				+ " " + settings.channel[chan].square.offsetInput;
			settingCombo.SetCurSel( 4 );
			break;
		case 3:
			// preprogrammed
			tempStr = settings.channel[chan].preloadedArb.address;
			settingCombo.SetCurSel( 5 );
			break;
		case 4:
			// scripted
			// ???
			settingCombo.SetCurSel( 6 );
			break;
		default:
			thrower( "ERROR: unrecognized agilent setting: " + settings.channel[chan].option);
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
	optionsEdit.SetWindowTextA( cstr(tempStr));
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


void Agilent::convertInputToFinalSettings( key variableKey, UINT variation )
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
					thrower( "Unrecognized Agilent Setting: " + str( settings.channel[chan].option ) );
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
void Agilent::handleSavingConfig(std::ofstream& saveFile)
{
	// make sure data is up to date.
	handleInput();
	// start outputting.
	saveFile << "AGILENT\n";
	saveFile << str(settings.synced) << "\n";
	saveFile << "CHANNEL_1\n";
	saveFile << str(settings.channel[0].option) + "\n";
	saveFile << settings.channel[0].dc.dcLevelInput + "\n";
	saveFile << settings.channel[0].sine.amplitudeInput + "\n";
	saveFile << settings.channel[0].sine.frequencyInput + "\n";
	saveFile << settings.channel[0].square.amplitudeInput + "\n";
	saveFile << settings.channel[0].square.frequencyInput + "\n";
	saveFile << settings.channel[0].square.offsetInput + "\n";
	saveFile << settings.channel[0].preloadedArb.address + " \n";
	saveFile << "CHANNEL_2\n";
	saveFile << str( settings.channel[1].option ) + "\n";
	saveFile << settings.channel[1].dc.dcLevelInput + "\n";
	saveFile << settings.channel[1].sine.amplitudeInput + "\n";
	saveFile << settings.channel[1].sine.frequencyInput + "\n";
	saveFile << settings.channel[1].square.amplitudeInput + "\n";
	saveFile << settings.channel[1].square.frequencyInput + "\n";
	saveFile << settings.channel[1].square.offsetInput + "\n";
	saveFile << settings.channel[1].preloadedArb.address + "\n";
	saveFile << "END_AGILENT\n";
}


void Agilent::zeroSettings()
{
	settings = deviceOutputInfo();
	settings.channel[0].option = -2;
	settings.channel[1].option = -2;
}


void Agilent::readConfigurationFile( std::ifstream& file )
{
	ProfileSystem::checkDelimiterLine(file, "AGILENT");
	file >> settings.synced;
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_1");
	// the extra step in all of the following is to remove the , at the end of each input.
	std::string input;
	file >> input;
	try
	{
		settings.channel[0].option = std::stoi( input );
	}
	catch (std::invalid_argument&)
	{
		thrower( "ERROR: Bad channel 1 option!" );
	}
	std::getline( file, settings.channel[0].dc.dcLevelInput);
	std::getline( file, settings.channel[0].sine.amplitudeInput );
	std::getline( file, settings.channel[0].sine.frequencyInput);
	std::getline( file, settings.channel[0].square.amplitudeInput);
	std::getline( file, settings.channel[0].square.frequencyInput);
	std::getline( file, settings.channel[0].square.offsetInput);
	std::getline( file, settings.channel[0].preloadedArb.address);
	ProfileSystem::checkDelimiterLine(file, "CHANNEL_2"); 
	file >> input;
	try
	{
		settings.channel[1].option = std::stoi(input);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Bad channel 1 option!");
	}
	std::getline( file, settings.channel[1].dc.dcLevelInput);
	std::getline( file, settings.channel[1].sine.amplitudeInput);
	std::getline( file, settings.channel[1].sine.frequencyInput);
	std::getline( file, settings.channel[1].square.amplitudeInput);
	std::getline( file, settings.channel[1].square.frequencyInput);
	std::getline( file, settings.channel[1].square.offsetInput);
	std::getline( file, settings.channel[1].preloadedArb.address);
	ProfileSystem::checkDelimiterLine(file, "END_AGILENT");
	// default to first channel.
	updateEdit( 1 );
}


void Agilent::outputOff( int channel )
{
	channel++;
	visaOpenDefaultRM();
	visaOpen( str( usbAddress ) );
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
		thrower( "ERROR: Attempted to turn off channel " + str( channel ) + " which does not exist! Use channel 1 or 2." );
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
	visaOpen( str( usbAddress ) );
	if (channel == 1)
	{
		visaWrite( "SOURce1:APPLy:DC DEF, DEF, " + str( info.dcLevel ) + " V" );
		chan1Range.min = chan1Range.max = info.dcLevel;
	}
	else if (channel == 2)
	{
		visaWrite( "SOURce2:APPLy:DC DEF, DEF, " + str( info.dcLevel ) + " V" );
		chan2Range.min = chan2Range.max = info.dcLevel;
	}
	else
	{
		thrower( "tried to set DC level for \"channel\" " + str( channel ) + ", which is not supported! "
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
		visaWrite( str( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
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
		visaWrite( str( "OUTPUT2:LOAD " ) + AGILENT_LOAD );
		visaWrite( "OUTPUT2 ON" );

	}
	else
	{
		thrower( "tried to set arbitrary function for \"channel\" " + str( channel ) + ", which is not supported! "
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
		visaWrite( "SOURCE1:APPLY:SQUARE " + str(info.frequency) + " KHZ, " 
				   + str(info.amplitude) + " VPP, " + str(info.offset) + " V" );
	}
	else if (channel == 2)
	{
		visaWrite( "SOURCE2:APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
				   + str( info.amplitude ) + " VPP, " + str( info.offset ) + " V" );
	}
	else
	{
		thrower( "tried to set square function for \"channel\" " + str( channel ) + ", which is not supported! "
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
	ULONG actual;
	if (!AGILENT_SAFEMODE)
	{
		errCheck( viWrite( instrument, (unsigned char*)cstr(message), (ViUInt32)message.size(), &actual ) );
	}
}

void Agilent::visaWrite(ULONG instrumentInput, std::string message)
{
	// not sure what this is for.
	ULONG actual;
	if (!AGILENT_SAFEMODE)
	{
		errCheck(instrumentInput, viWrite(instrumentInput, (unsigned char*)cstr(message), (ViUInt32)message.size(), &actual));
	}
}

void Agilent::errCheck(ULONG instrumentInput, long status)
{
	long errorCode = 0;
	// Check comm status
	if (status < 0)
	{
		// Error detected.
		thrower("ERROR: Communication error with agilent. Error Code: " + str(status) + "\r\n");
	}
	// Query the agilent for errors.
	std::string errMessage;
	visaErrQuery(instrumentInput, errMessage, errorCode);
	if (errorCode != 0)
	{
		// Agilent error
		thrower("ERROR: agilent returned error message: " + str(errorCode) + ":" + errMessage);
	}
}

void Agilent::visaErrQuery(ULONG instrument, std::string& errMsg, long& errCode)
{
	char buf[256] = { 0 };
	if (!AGILENT_SAFEMODE)
	{
		viQueryf(instrument, (ViString)"SYST:ERR?\n", "%ld,%t", &errCode, buf);
	}
	else
	{
		return;
	}
	errMsg = str(buf);
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
		errCheck( viOpen( defaultResourceManager, (char *)cstr(address), VI_NULL, VI_NULL, &instrument ) );
	}
}


void Agilent::visaSetAttribute( ULONG attributeName, ULONG value )
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
		return "Agilent Safemode...\r\n";
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
	errMsg = str( buf );
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
		thrower( "ERROR: Communication error with agilent. Error Code: " + str( status ) + "\r\n" );
	}
	// Query the agilent for errors.
	std::string errMessage;
	visaErrQuery( errMessage, errorCode );
	if (errorCode != 0)
	{
		// Agilent error
		thrower( "ERROR: agilent returned error message: " + str( errorCode ) + ":" + errMessage );
	}
}



void Agilent::visaPrintf( std::string msg )
{
	if (!AGILENT_SAFEMODE)
	{
		errCheck( viPrintf( instrument, (ViString)cstr(msg)) );
	}
}


/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// pilfered from myAgilent.
/// ......................................


/*
* This function tells the agilent to put out the DC default waveform.
*/
void Agilent::agilentDefault()
{
	ULONG viDefaultRM, Instrument, actual;
	if (!AGILENT_SAFEMODE)
	{
		viOpenDefaultRM(&viDefaultRM);
		int val = viOpen(viDefaultRM, (char *)AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
		// turn it to the default voltage...
		visaWrite(Instrument, str("APPLy:DC DEF, DEF, ") + AGILENT_DEFAULT_DC);
		// and leave...
		viClose(Instrument);
		viClose(viDefaultRM);
	}

	// update current values
	eCurrentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
	eCurrentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);
}


void Agilent::analyzeIntensityScript( ScriptStream& intensityFile, myAgilent::IntensityWaveform* intensityWaveformData,
									  int& currentSegmentNumber )
{
	while (!intensityFile.eof())
	{
		// Procedurally read lines into segment informations.
		int leaveTest = intensityWaveformData->readIntoSegment( currentSegmentNumber, intensityFile );
		if (leaveTest < 0)
		{
			// Error
			// should I be throwing here?
			errBox( "ERROR: IntensityWaveform.readIntoSegment threw an error! Error occurred in segment #" 
					+ str( currentSegmentNumber ) + "." );
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
void Agilent::programIntensity( key varKey, bool& intensityVaried, std::vector<std::pair<double, double>>& minsAndMaxes,
								std::vector<std::fstream>& intensityFiles, UINT variations )
{
	// Initialize stuff
	myAgilent::IntensityWaveform intensityWaveformSeq;
	int currentSegmentNumber = 0;
	ULONG viDefaultRM = 0, Instrument = 0;
	ULONG actual;
	if (!AGILENT_SAFEMODE)
	{
		viOpenDefaultRM( &viDefaultRM );
		viOpen( viDefaultRM, (char *)AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument );
		// ???
		agilentErrorCheck( viSetAttribute( Instrument, VI_ATTR_TMO_VALUE, 40000 ), Instrument );
	}
	// Set sample rate
	visaWrite( Instrument, "SOURCE1:FUNC:ARB:SRATE " + str( AGILENT_SAMPLE_RATE ) );
	// Set filtering state
	visaWrite( Instrument, str( "SOURCE1:FUNC:ARB:FILTER " ) + AGILENT_FILTER_STATE );
	// Set Trigger Parameters
	visaWrite( Instrument, "TRIGGER1:SOURCE EXTERNAL" );
	//
	visaWrite( Instrument, "TRIGGER1:SLOPE POSITIVE" );

	for (int sequenceInc = 0; sequenceInc < intensityFiles.size(); sequenceInc++)
	{
		ScriptStream intensityScript;
		intensityScript << intensityFiles[sequenceInc].rdbuf();
		analyzeIntensityScript( intensityScript, &intensityWaveformSeq, currentSegmentNumber );
	}
	int totalSegmentNumber = currentSegmentNumber;
	intensityVaried = intensityWaveformSeq.returnIsVaried();
	// if varied
	if (intensityWaveformSeq.returnIsVaried())
	{
		for (auto variation : range( variations ))
		{
			key;
			for (auto const& variable : varKey)
			{
				// replace variable values where found
				intensityWaveformSeq.replaceVarValues( variable.first, variable.second.first[variation] );
			}
			// Loop through all segments
			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// Use that information to write the data.
				if (intensityWaveformSeq.writeData( segNumInc ) < 0)
				{
					thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
							 + str( totalSegmentNumber ) + "." );
				}
			}
			// loop through again and calc/normalize/write values.
			intensityWaveformSeq.convertPowersToVoltages();
			intensityWaveformSeq.calcMinMax();
			minsAndMaxes.resize( variation + 1 );
			minsAndMaxes[variation].second = intensityWaveformSeq.returnMaxVolt();
			minsAndMaxes[variation].first = intensityWaveformSeq.returnMinVolt();
			intensityWaveformSeq.normalizeVoltages();

			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				visaWrite( Instrument, intensityWaveformSeq.compileAndReturnDataSendString( segNumInc, variation,
																							totalSegmentNumber ) );
																				  // Select the segment
				visaWrite( Instrument, "SOURCE1:FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
				// Save the segment
				visaWrite( Instrument, "MMEM:STORE:DATA \"INT:\\seg"
						   + str( segNumInc + totalSegmentNumber * variation ) + ".arb\"" );
				 // increment for the next.
				visaWrite( Instrument, "TRIGGER1:SLOPE POSITIVE" );
			}
			// Now handle seqeunce creation / writing.
			intensityWaveformSeq.compileSequenceString( totalSegmentNumber, variation );
			// submit the sequence
			visaWrite( Instrument, intensityWaveformSeq.returnSequenceString() );
			// Save the sequence
			visaWrite( Instrument, "SOURCE1:FUNC:ARB seq" + str( variation ) );
			visaWrite( Instrument, "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
			// clear temporary memory.
			visaWrite( Instrument, "SOURCE1:DATA:VOL:CLEAR" );

		}
		// loop through # of variable values
		for (UINT variation : range( variations ))
		{
			for (auto const& variable : varKey)
			{
				intensityWaveformSeq.replaceVarValues( variable.first, variable.second.first[variation] );
			}

			// Loop through all segments
			for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// Use that information to write the data.
				if (intensityWaveformSeq.writeData( segNumInc ) < 0)
				{
					thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
							 + str( totalSegmentNumber ) + "." );
				}
			}
			// loop through again and calc/normalize/write values.
			intensityWaveformSeq.convertPowersToVoltages();
			intensityWaveformSeq.calcMinMax();
			minsAndMaxes.resize( variation + 1 );
			minsAndMaxes[variation].second = intensityWaveformSeq.returnMaxVolt();
			minsAndMaxes[variation].first = intensityWaveformSeq.returnMinVolt();
			intensityWaveformSeq.normalizeVoltages();

			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				visaWrite( Instrument, intensityWaveformSeq.compileAndReturnDataSendString( segNumInc, variation,
																							totalSegmentNumber ) );
																				  // Select the segment
				visaWrite( Instrument, "SOURCE1:FUNC:ARB seg" + str( segNumInc + totalSegmentNumber * variation ) );
				// Save the segment
				visaWrite( Instrument, "MMEM:STORE:DATA \"INT:\\seg" + str( segNumInc + totalSegmentNumber * variation )
						   + ".arb\"" );
				 // increment for the next.
				visaWrite( Instrument, "TRIGGER1:SLOPE POSITIVE" );
			}
			// Now handle seqeunce creation / writing.
			intensityWaveformSeq.compileSequenceString( totalSegmentNumber, variation );
			// submit the sequence
			visaWrite( Instrument, intensityWaveformSeq.returnSequenceString() );
			// Save the sequence
			visaWrite( Instrument, "SOURCE1:FUNC:ARB seq" + str( variation ) );
			visaWrite( Instrument, "MMEM:STORE:DATA \"INT:\\seq" + str( variation ) + ".seq\"" );
			// clear temporary memory.
			visaWrite( Instrument, "SOURCE1:DATA:VOL:CLEAR" );
		}
	}
	else
	{
		// else not varying. Loop through all segments
		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Use that information to write the data.
			if (intensityWaveformSeq.writeData( segNumInc ) < 0)
			{
				thrower( "ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #"
						 + str( totalSegmentNumber ) + "." );
			}
		}
		// no reassignment nessesary, no variables
		intensityWaveformSeq.convertPowersToVoltages();
		intensityWaveformSeq.calcMinMax();
		minsAndMaxes.resize( 1 );
		minsAndMaxes[0].second = intensityWaveformSeq.returnMaxVolt();
		minsAndMaxes[0].first = intensityWaveformSeq.returnMinVolt();
		intensityWaveformSeq.normalizeVoltages();
		for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
		{
			// Set output impedance...
			visaWrite( Instrument, str( "OUTPUT1:LOAD " ) + AGILENT_LOAD );
			// set range of voltages...
			visaWrite( Instrument, str( "SOURCE1:VOLT:LOW " ) + str( minsAndMaxes[0].first ) + " V" );
			visaWrite( Instrument, str( "SOURCE1:VOLT:HIGH " ) + str( minsAndMaxes[0].second ) + " V" );
			// get the send string
			visaWrite( Instrument, intensityWaveformSeq.compileAndReturnDataSendString( segNumInc, 0,
																						totalSegmentNumber ) );
			 // Select the segment
			visaWrite( Instrument, "SOURCE1:FUNC:ARB seg" + str( segNumInc ) );
			// Save the segment
			visaWrite( Instrument, "MMEM:STORE:DATA \"INT:\\seg" + str( segNumInc ) + ".arb\"" );
		}

		// Now handle seqeunce creation / writing.
		intensityWaveformSeq.compileSequenceString( totalSegmentNumber, 0 );
		// submit the sequence
		visaWrite( Instrument, intensityWaveformSeq.returnSequenceString() );
		// Save the sequence
		visaWrite( Instrument, "SOURCE1:FUNC:ARB seq" + str( 0 ) );
		visaWrite( Instrument, "MMEM:STORE:DATA \"INT:\\seq" + str( 0 ) + ".seq\"" );
		// clear temporary memory.
		visaWrite( Instrument, "SOURCE1:DATA:VOL:CLEAR" );
	}
	viClose( Instrument );
	viClose( viDefaultRM );
}


/*
* This function checks if the agilent throws an error or if there is an error communicating with the agilent. it returns -1 if error, 0 otherwise.
*/
void Agilent::agilentErrorCheck(long status, ULONG vi)
{
	long errorCode = 0;
	char buf[256] = { 0 };
	// Check comm status
	if (status < 0)
	{
		// Error detected.
		thrower("ERROR: Communication error with agilent. Error Code: " + str(status));
	}
	if (!ANDOR_SAFEMODE)
	{
		// Query the agilent for errors.
		viQueryf(vi, "SYST:ERR?\n", "%ld,%t", &errorCode, buf);
	}
	if (errorCode != 0)
	{
		// Agilent error
		thrower("ERROR: agilent returned error message: " + str(errorCode) + ":" + buf);
	}
}

/*
* This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
*/
void Agilent::setIntensity(UINT varNum, bool intensityIsVaried, std::vector<std::pair<double, double>> intensityMinMax)
{
	if (intensityIsVaried || varNum == 0)
	{
		ULONG viDefaultRM, Instrument, actual;
		if (!NIAWG_SAFEMODE)
		{
			viOpenDefaultRM(&viDefaultRM);
			viOpen(viDefaultRM, (char *)AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
		}
		std::string SCPIcmd;
		if (!NIAWG_SAFEMODE)
		{
			// Load sequence that was previously loaded.
			SCPIcmd = "MMEM:LOAD:DATA \"INT:\\seq" + str(varNum) + ".seq\"";
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

			SCPIcmd = "SOURCE1:FUNC ARB";
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

			SCPIcmd = "SOURCE1:FUNC:ARB \"INT:\\seq" + str(varNum) + ".seq\"";
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

			// Set output impedance...
			SCPIcmd = str("OUTPUT1:LOAD ") + AGILENT_LOAD;

			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			SCPIcmd = str("SOURCE1:VOLT:LOW ") + str(intensityMinMax[varNum].first) + " V";
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			SCPIcmd = str("SOURCE1:VOLT:HIGH ") + str(intensityMinMax[varNum].second) + " V";
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

			SCPIcmd = "OUTPUT1 ON";
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// and leave...
			viClose(Instrument);
			viClose(viDefaultRM);
		}
	}
}


void Agilent::setAgilent(key varKey, UINT variation)
{
	if (!connected())
	{
		return;
	}
	convertInputToFinalSettings( varKey, variation);
	deviceOutputInfo info = getOutputInfo();
	for (auto chan : range( 2 ))
	{
		switch (info.channel[chan].option)
		{
			case -2:
				// don't do anything.
				break;
			case -1:
				outputOff( chan );
				break;
			case 0:
				setDC( chan, info.channel[chan].dc );
				break;
			case 1:
				setSingleFreq( chan, info.channel[chan].sine );
				break;
			case 2:
				setSquare( chan, info.channel[chan].square );
				break;
			case 3:
				setExistingWaveform( chan, info.channel[chan].preloadedArb );
				break;
			case 4:
				// TODO
			default:
				thrower( "ERROR: unrecognized channel 1 setting: " + str( info.channel[chan].option ) );
		}
	}
}