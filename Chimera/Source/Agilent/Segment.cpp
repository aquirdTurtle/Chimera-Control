// created by Mark O. Brown
#include "stdafx.h"
#include "Segment.h"
#include <boost/lexical_cast.hpp>


void Segment::calculateSegVariations( unsigned totalNumVariations, std::vector<parameterType>& variables ){	
	// handle more complicated things.
	if ( input.ramp.isRamp ){
		input.ramp.start.internalEvaluate (variables, totalNumVariations);
		input.ramp.end.internalEvaluate (variables, totalNumVariations);
		input.ramp.start.internalEvaluate( variables, totalNumVariations );
		input.ramp.end.internalEvaluate (variables, totalNumVariations);
	}
	else if ( input.pulse.isPulse )	{
		input.pulse.vOffset.internalEvaluate ( variables, totalNumVariations );
		input.pulse.amplitude.internalEvaluate ( variables, totalNumVariations );
		input.pulse.tOffset.internalEvaluate ( variables, totalNumVariations );
		input.pulse.width.internalEvaluate ( variables, totalNumVariations );
	}
	else{
		input.holdVal.internalEvaluate ( variables, totalNumVariations );
	}

	if ( input.mod.modulationIsOn )	{
		input.mod.frequency.internalEvaluate ( variables, totalNumVariations );
		input.mod.phase.internalEvaluate ( variables, totalNumVariations );
	}
	input.time.internalEvaluate( variables, totalNumVariations);
	if ( input.time.getValue(0)/1e3 < 1e-9 ){
		thrower ( "ERROR: agilent segment set to have zero time! Agilent can't handle zero-length segments." );
	}
	input.repeatNum.internalEvaluate (variables, totalNumVariations);
	//if (input.continuationType == SegmentEnd::type::repeat)	{
	//	// in which case you need a number of times to repeat.);
	//	input.repeatNum.internalEvaluate( variables, totalNumVariations );
	//}
}

void Segment::storeInput( segmentInfo inputToSet ){
	input = inputToSet;
}

segmentInfo Segment::getInput(){
	return input;
}

/**
* This function takes ramp-related information as an input and returns the "position" in the ramp 
* (i.e. the amount to add to the initial value due to ramping) that the waveform should be at.
*/
double Segment::rampCalc( int totalSamples, int sample, rampInfo ramp, unsigned varnum){
	auto rampSize = ramp.end.getValue (varnum) - ramp.start.getValue (varnum);
	if (ramp.type == "lin"){
		return sample * rampSize / totalSamples;
	}
	else if (ramp.type == "nr"){
		return 0;
	}
	else if (ramp.type == "tanh"){
		return rampSize * (tanh( -4 + 8 * (double)sample / totalSamples ) + 1) / 2;
	}
	else{
		if (ramp.isFileRamp) {
			if (ramp.rampFileVals.size () != totalSamples) {
				thrower ("ramp file vals not same size as total samples in agilent waveform! ramp size was "
					+ str (ramp.rampFileVals.size ()) + ", total sample number was " + str(totalSamples));
			}
			return ramp.rampFileVals[sample] * rampSize;
		}
		return 0;
	}
}

double Segment::modCalc( segModInfo mod, int iteration, long size, double pulseLength, unsigned varNum ){
	if ( !mod.modulationIsOn ){
		return 1.0;
	}
	double t = double(iteration) / size * pulseLength;
	double result;
	// mult by 1e6 to convert from MHz to Hz.
	result = sin( 2 * PI * mod.frequency.getValue(varNum) * 1e6 * t + mod.phase.getValue (varNum));
	return result;
}


double Segment::pulseCalc( segPulseInfo pulse, int iteration, long size, double pulseLength, double center,
	unsigned varNum){
	auto widthV = pulse.width.getValue (varNum) / 1000.0;
	auto ampV = pulse.amplitude.getValue (varNum);
	if ( pulse.type == "__NONE__" ){
		return 0;
	}
	else if ( pulse.type == "gaussian" ){
		// in this case, the width is the sigma of the gaussian. Note that this definition the width = tau from
		// adam's thesis because this will give the intensity of one of the beams, and the rabi rate~ sqrt(I1*I2) for
		// the two beams.

		double x = pulseLength * iteration / size;
		double result = ampV * exp( -(center - x) * (center - x) / (widthV * widthV) );
		return result;
	}
	else if ( pulse.type == "lorentzian" ){
		// in this case, the width is the standard lorentzian full width half max. 
		double FWHM = widthV;
		//double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		// see definition: http://mathworld.wolfram.com/LorentzianFunction.html
		return ampV * (FWHM / (2.0 * PI)) / ((x - center)*(x - center) + (FWHM / 2) * (FWHM / 2));

	}
	else if ( pulse.type == "sech" ){
		// in this case, the width is just the scaling factor for the length
		//double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		// see definition: http://mathworld.wolfram.com/HyperbolicSecant.html
		return ampV * 1.0 / cosh( (x - center) / widthV);
	}
	else{
		thrower ( "ERROR: pulse type " + pulse.type + " is unrecognized.\r\n" );
		return 0;
	}
}


/*
 * This function uses the initial and final points along with the ramp and time of the segment to calculate all of the data points. This should be used so
 * as to, after this function, you have all of the powers that you want (not voltages), and then call the voltage converter afterwards.
 */
void Segment::calcData( unsigned long sampleRate, unsigned varNum){
	// calculate the size of the waveform.
	double numDataPointsf = input.time.getValue(varNum)/1e3 * sampleRate;
	int numDataPoints = (int)round (numDataPointsf);
	// test if good time.
	if (fabs( numDataPointsf - round( numDataPointsf ) ) > 1e-6){
		thrower ( "ERROR: Bad time entered for the time of an intensity sequence segment. The time was "
				 + str(input.time.getValue (varNum) / 1e3) + ", and the sample rate is " + str( sampleRate ) + ". The product of these"
				 " is the number of samples in the segment, and this must be an integer." );
	}
	if (input.ramp.isRamp && (input.ramp.type != "lin" && input.ramp.type != "tanh" && input.ramp.type != "nr")) {
		Segment::analyzeRampFile (input.ramp, numDataPoints);
	}
	// resize to zero. This is a complete reset of the data points in the class.
	dataArray.resize( 0 );
	// writebtn the data points. These are all in powers. These are converted to voltages later.
	for ( int dataInc = 0; dataInc < numDataPoints; dataInc++ ){
		double point;
		if ( input.ramp.isRamp ){
			point = input.ramp.start.getValue (varNum) + rampCalc( numDataPoints, dataInc, input.ramp, varNum );
		}
		else if (input.pulse.isPulse ){
			point = input.pulse.vOffset.getValue (varNum) + pulseCalc(input.pulse, dataInc, numDataPoints,
				input.time.getValue (varNum) / 1e3, input.pulse.tOffset.getValue (varNum)/1000.0, varNum)
				* modCalc(input.mod, dataInc, numDataPoints, input.time.getValue (varNum) / 1e3, varNum);
		}
		else{
			point = input.holdVal.getValue (varNum);
		}
		dataArray.push_back( point );
	}
}

unsigned Segment::returnDataSize(){
	return dataArray.size();
}

double Segment::returnDataVal( long dataNum ){
	return dataArray[dataNum];
}

void Segment::assignDataVal( int dataNum, double val ){
	dataArray[dataNum] = val;
}


void Segment::analyzeRampFile (rampInfo& ramp, long totalSamples) {
	std::ifstream rampFile (RAMP_LOCATION + ramp.type + ".txt");
	if (!rampFile.is_open ()) {
		thrower ("ERROR: ramp type " + ramp.type + " is unrecognized.\r\n");
	}
	ramp.rampFileName = ramp.type;
	ramp.rampFileVals.clear ();
	std::string word;
	while (rampFile >> word) {
		try {
			ramp.rampFileVals.push_back (boost::lexical_cast<double>(word));
		}
		catch (boost::bad_lexical_cast &) {
			thrower ("Failed to convert ramp file to doubles!");
		}
	}
	if (ramp.rampFileVals.size () != totalSamples) {
		thrower ("ramp file vals not same size as total samples! ramp size was "
			+ str (ramp.rampFileVals.size ()) + ", total sample number was " + str (totalSamples));
	}
	ramp.isFileRamp = true;
}
