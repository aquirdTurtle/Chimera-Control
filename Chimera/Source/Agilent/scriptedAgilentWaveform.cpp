// created by Mark O. Brown
#include "stdafx.h"
#include "Agilent/Agilent.h"
#include "scriptedAgilentWaveform.h"
#include "Scripts/ScriptStream.h"
#include <ExperimentThread/ExpThreadWorker.h>

ScriptedAgilentWaveform::ScriptedAgilentWaveform(){
	segmentNum = 0;
	totalSequence = "";
};


void ScriptedAgilentWaveform::resetNumberOfTriggers( ){
	numberOfTriggers = 0;
}


/** This function reads out a segment of script file and loads it into a segment to be calculated and manipulated.
* segNum: This tells the function what the next segment # is.
* script: this is the object to be readbtn from.
*/
bool ScriptedAgilentWaveform::analyzeAgilentScriptCommand( int segNum, ScriptStream& script, 
														   std::vector<parameterType>& params,
															std::string& warnings ){
	std::string scope = AgilentCore::systemScope;
	segmentInfo workingInput;
	std::string word;
	if (script.peek() == EOF)	{
		return true;
	}
	// Grab the command type (e.g. ramp, const). Looks for newline by default.
	script >> word;
	try	{
		if (ExpThreadWorker::handleVariableDeclaration(word, script, params, scope, warnings)){
			return false;
		}
		else if ( word == "hold" ){
			workingInput.ramp.isRamp = false;
			workingInput.pulse.isPulse = false;
			workingInput.mod.modulationIsOn = false;

			script >> workingInput.holdVal;
			workingInput.holdVal.assertValid ( params, scope );
		}
		else if ( word == "ramp" ){
			// this segment type means ramping.
			workingInput.ramp.isRamp = true;
			workingInput.pulse.isPulse = false;
			workingInput.mod.modulationIsOn = false;
			script >> workingInput.ramp.type;
			script >> workingInput.ramp.start;
			workingInput.ramp.start.assertValid ( params, scope );
			script >> workingInput.ramp.end;
			workingInput.ramp.end.assertValid ( params, scope );
		}
		else if ( word == "pulse" ){
			workingInput.ramp.isRamp = false;
			workingInput.pulse.isPulse = true;
			workingInput.mod.modulationIsOn = false;
			script >> workingInput.pulse.type;
			script >> workingInput.pulse.vOffset;
			workingInput.pulse.vOffset.assertValid ( params, scope );
			script >> workingInput.pulse.amplitude;
			workingInput.pulse.amplitude.assertValid ( params, scope );
			script >> workingInput.pulse.width;
			workingInput.pulse.width.assertValid ( params, scope );
			script >> workingInput.pulse.tOffset;
			workingInput.pulse.tOffset.assertValid ( params, scope );
		}
		else if ( word == "modpulse" ){
			workingInput.ramp.isRamp = false;
			workingInput.pulse.isPulse = true;
			workingInput.mod.modulationIsOn = true;
			script >> workingInput.pulse.type;
			script >> workingInput.pulse.vOffset;
			workingInput.pulse.vOffset.assertValid ( params, scope );
			script >> workingInput.pulse.amplitude;
			workingInput.pulse.amplitude.assertValid ( params, scope );
			script >> workingInput.pulse.width;
			workingInput.pulse.width.assertValid ( params, scope );
			script >> workingInput.pulse.tOffset;
			workingInput.pulse.tOffset.assertValid ( params, scope );
			// mod stuff
			script >> workingInput.mod.frequency;
			workingInput.mod.frequency.assertValid ( params, scope );
			script >> workingInput.mod.phase;
			workingInput.mod.phase.assertValid ( params, scope );
		}
		else{
			if ( script.peek ( ) == EOF ){
				return true;
			}
			thrower ( "Agilent Script command not recognized. The command was \"" + word + "\"" );
		}
	}
	catch ( ChimeraError& ){
		throwNested ("Error seen while analyzing scripted agilent waveform." );
	}
	script >> workingInput.time;
	workingInput.time.assertValid( params, scope );

	std::string tempContinuationType;
	script >> tempContinuationType;
	if (tempContinuationType == "repeat"){
		// There is an extra input in this case.
		script >> workingInput.repeatNum;
		workingInput.repeatNum.assertValid( params, scope );
	}
	else{
		workingInput.repeatNum.expressionStr = "0";
	}
	std::string delimiter;
	script >> delimiter;
	if (delimiter != "#"){
		// input number mismatch.
		thrower ( "The delimeter is missing in the Intensity script file for Segment #" + str( segNum + 1 )
				 + ". The value placed in the delimeter location was " + delimiter + " while it should have been '#'. "
				 "This indicates that either the code is not interpreting the user input incorrectly or that the user "
				 "has inputted too many parameters for this type of Segment. Use of \"Repeat\" without the number of "
				 "repeats following will also trigger this error." );
		return false;
	}
	workingInput.continuationType = SegmentEnd::fromStr ( tempContinuationType );
	numberOfTriggers += SegmentEnd::invovlesTrig ( workingInput.continuationType );
	waveformSegments.resize (waveformSegments.size () + 1);
	waveformSegments.back().storeInput( workingInput );
	return false;
}


/*
 * This function takes the data points (that have already been converted and normalized) and puts them into a string
 * for the agilent to readbtn. segNum: this is the segment number that this data is for
 * varNum: This is the variation number for this segment (matters for naming the segments)
 * totalSegNum: This is the number of segments in the waveform (also matters for naming)
 */
std::string ScriptedAgilentWaveform::compileAndReturnDataSendString( int segNum, int varNum, int totalSegNum, unsigned chan ){
	// must get called after data conversion
	std::string tempSendString;
	tempSendString = "SOURce" + str( chan ) + ":DATA:ARB segment" + str( segNum + totalSegNum * varNum ) + ",";
	// need to handle last one separately so that I can /not/ put a comma after it.
	unsigned numData = waveformSegments[segNum].returnDataSize( ) - 1;
	for (unsigned sendDataInc = 0; sendDataInc < numData; sendDataInc++){
		tempSendString += str( waveformSegments[segNum].returnDataVal( sendDataInc ) );
		tempSendString += ", ";
	}
	tempSendString += str( waveformSegments[segNum].returnDataVal( waveformSegments[segNum].returnDataSize() - 1 ) );
	return tempSendString;
}

void ScriptedAgilentWaveform::calSegmentData( int segNum, unsigned long sampleRate, unsigned varNum ){
	waveformSegments[segNum].calcData(sampleRate, varNum);
}

unsigned long ScriptedAgilentWaveform::getSegmentNumber(){
	return waveformSegments.size();
}


/*
* This function compiles the sequence string which tells the agilent what waveforms to output when and with what trigger control. The sequence is stored
* as a part of the class.
*/
void ScriptedAgilentWaveform::compileSequenceString( int totalSegNum, int sequenceNum, unsigned channel, unsigned varNum ){
	std::string tempSequenceString, tempSegmentInfoString;
	// Total format is  #<n><n digits><sequence name>,<arb name1>,<repeat count1>,<play control1>,<marker mode1>,<marker point1>,<arb name2>,<repeat count2>,
	// <play control2>, <marker mode2>, <marker point2>, and so on.
	tempSequenceString = "SOURce" + str( channel) + ":DATA:SEQ #";
	tempSegmentInfoString = "sequence" + str( sequenceNum ) + ",";
	if (totalSegNum == 0){
		thrower ("No segments in agilent waveform???\r\n");
	}
	for (int segNumInc = 0; segNumInc < totalSegNum; segNumInc++){
		tempSegmentInfoString += "segment" + str ( segNumInc + totalSegNum * sequenceNum ) + ",";
		tempSegmentInfoString += str ( waveformSegments[ segNumInc ].getInput ( ).repeatNum.getValue(varNum) ) + ",";
		tempSegmentInfoString += SegmentEnd::toStr ( waveformSegments[ segNumInc ].getInput( ).continuationType ) + ",";
		tempSegmentInfoString += "highAtStart,4,";
	}
	// remove final comma.
	tempSegmentInfoString.pop_back ( );
	totalSequence = tempSequenceString + str( (str( tempSegmentInfoString.size( ) )).size( ) ) 
		+ str( tempSegmentInfoString.size( ) ) + tempSegmentInfoString;
}


/*
 * This function just returns the sequence string. It should already have been compiled using compileSequenceString 
 * when this is called.
 */
std::string ScriptedAgilentWaveform::returnSequenceString( ){
	return totalSequence;
}


/*
 * This function returns the truth of whether this sequence is being varied or not. This gets determined during the 
 * reading process.
 */
bool ScriptedAgilentWaveform::isVaried( ){
	for ( auto& seg : waveformSegments ){
		auto& input = seg.getInput( );
		if ( input.time.varies( ) ){
			return true;
		}
		if ( input.repeatNum.varies( ) ){
			return true;
		}
		if ( input.ramp.isRamp ){
			if ( input.ramp.start.varies( ) || input.ramp.end.varies( ) ){
				return true;
			}
		}
		else if ( input.pulse.isPulse ){
			if ( input.pulse.amplitude.varies( ) || input.pulse.vOffset.varies( ) || input.pulse.width.varies( ) ){
				return true;
			}
			if ( input.mod.modulationIsOn ){
				if ( input.mod.frequency.varies( ) || input.mod.phase.varies( ) ){
					return true;
				}
			}
		}
		else{
			if ( input.holdVal.varies( ) ){
				return true;
			}
		}
	}
	return false;
}

/*
* This waveform loops through all of the segments to find places where a variable value needs to be changed, and changes it.
*/
void ScriptedAgilentWaveform::calculateAllSegmentVariations( unsigned totalNumVariations, std::vector<parameterType>& variables ){
	for (unsigned segNumInc = 0; segNumInc < waveformSegments.size(); segNumInc++){
		waveformSegments[segNumInc].calculateSegVariations(totalNumVariations, variables );
	}
}


unsigned long ScriptedAgilentWaveform::getNumTrigs( ){
	return numberOfTriggers;
}

/*
 * This function takes the powers inputted by the user and converts them (based on calibrations that we have done) to the corresponding voltage values
 * that the agilent needs to output in order to reach those powers. The calibration is currently hard-coded. This needs to be run before compiling the
 * data string.
 */
void ScriptedAgilentWaveform::convertPowersToVoltages(bool useCal, calResult calibration){
	// for each part of the waveform returnDataSize
	for (unsigned segmentInc = 0; segmentInc < waveformSegments.size(); segmentInc++){
		// for each data point in that part
		for (unsigned dataConvertInc = 0; dataConvertInc < waveformSegments[segmentInc].returnDataSize(); dataConvertInc++)	{
			// convert the user power, which is entered in mW, to uW. That's the units this calibration was done in.
			// y is the desired power in microwatts.
			double power = waveformSegments[segmentInc].returnDataVal( dataConvertInc );
			// setPoint = a * power + b
			//double newValue = -a * log(y * b);
			double setPointinVolts = AgilentCore::convertPowerToSetPoint(power, useCal, calibration);
			waveformSegments[segmentInc].assignDataVal( dataConvertInc, setPointinVolts);
		}
	}
}


/*
* This wavefunction loops through all the data values and figures out which ones are min and max.
*/
void ScriptedAgilentWaveform::calcMinMax(){
	// NOT DBL_MIN, which is a really small number, not a large negative number. I need a large negative number.
	maxVolt = -DBL_MAX;
	minVolt = DBL_MAX;
	for (unsigned minMaxSegInc = 0; minMaxSegInc < waveformSegments.size(); minMaxSegInc++)	{
		for (unsigned minMaxDataInc = 0; minMaxDataInc < waveformSegments[minMaxSegInc].returnDataSize(); minMaxDataInc++){
			if (waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc ) < minVolt){
				minVolt = waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc );
			}
			if (waveformSegments[minMaxSegInc].returnDataVal( minMaxDataInc ) > maxVolt){
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
void ScriptedAgilentWaveform::normalizeVoltages(){
	double scaleFactor = 2.0 / (maxVolt - minVolt);
	for (unsigned normSegInc = 0; normSegInc < waveformSegments.size(); normSegInc++){
		for (unsigned normDataInc = 0; normDataInc < waveformSegments[normSegInc].returnDataSize(); normDataInc++){
			double currVal = waveformSegments[normSegInc].returnDataVal( normDataInc );
			double normVal = (currVal - minVolt) * scaleFactor - 1;
			waveformSegments[normSegInc].assignDataVal( normDataInc, normVal );
		}
	}
}


/**
 * Returns the maximum voltage level currently in data structures.
 */
double ScriptedAgilentWaveform::getMaxVolt(){
	return maxVolt;
}

/**
 * Returns the minimum voltage level currently in data structures.
 */
double ScriptedAgilentWaveform::getMinVolt(){
	return minVolt;
}

