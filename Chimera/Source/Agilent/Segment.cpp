// created by Mark O. Brown
#include "stdafx.h"
#include "Segment.h"

void Segment::convertInputToFinal( unsigned variation, std::vector<parameterType>& variables )
{	
	// first transfer things that can't be varied.
	finalSettings.continuationType = input.continuationType;
	// handle more complicated things.
	finalSettings.ramp.isRamp = input.ramp.isRamp;
	finalSettings.pulse.isPulse = input.pulse.isPulse;
	finalSettings.mod.modulationIsOn = input.mod.modulationIsOn;
	if ( input.ramp.isRamp )
	{
		finalSettings.ramp.type = input.ramp.type;
		finalSettings.ramp.start = input.ramp.start.evaluate( variables, variation );
		finalSettings.ramp.end = finalSettings.ramp.type == "nr" ? finalSettings.ramp.start
			: input.ramp.end.evaluate ( variables, variation );
	}
	else if ( input.pulse.isPulse )
	{
		finalSettings.pulse.type = input.pulse.type;
		finalSettings.pulse.vOffset = input.pulse.vOffset.evaluate( variables, variation );
		finalSettings.pulse.amplitude = input.pulse.amplitude.evaluate( variables, variation );
		finalSettings.pulse.tOffset = input.pulse.tOffset.evaluate ( variables, variation ) / 1000.0;
		finalSettings.pulse.width = input.pulse.width.evaluate( variables, variation ) / 1000.0;
	}
	else
	{
		finalSettings.holdVal = input.holdVal.evaluate( variables, variation );
	}

	if ( input.mod.modulationIsOn )
	{
		finalSettings.mod.frequency = input.mod.frequency.evaluate( variables, variation );
		finalSettings.mod.phase = input.mod.phase.evaluate( variables, variation );
	}
	finalSettings.time = input.time.evaluate( variables, variation) / 1000.0;
	if ( finalSettings.time < 1e-9 )
	{
		thrower ( "ERROR: agilent segment set to have zero time! Agilent can't handle zero-length segments." );
	}
	if (finalSettings.continuationType == SegmentEnd::type::repeat)
	{
		// in which case you need a number of times to repeat.);
		finalSettings.repeatNum = unsigned( input.repeatNum.evaluate( variables, variation ));
	}
}


void Segment::storeInput( segmentInfoInput inputToSet )
{
	input = inputToSet;
}


segmentInfoInput Segment::getInput()
{
	return input;
}


segmentInfoFinal Segment::getFinalSettings()
{
	return finalSettings;
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
* @param type is the type of ramp being executed, as specified by the reader.
*/
double Segment::rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType )
{
	if (rampType == "lin")
	{
		return iteration * (finPos - initPos) / size;
	}
	// for no ramp
	else if (rampType == "nr")
	{
		return 0;
	}
	else if (rampType == "tanh")
	{
		return (finPos - initPos) * (tanh( -4 + 8 * (double)iteration / size ) + 1) / 2;
	}
	else
	{
		// error message. I've already checked (outside this function) whether the ramp-type is a filename.
		thrower ( "ERROR: ramp type " + rampType + " is unrecognized.\r\n" );
		return 0;
	}
}


double Segment::modCalc( modData mod, int iteration, long size, double pulseLength )
{
	if ( !mod.modulationIsOn )
	{
		return 1.0;
	}
	double t = double(iteration) / size * pulseLength;
	double result;
	// mult by 1e6 to convert from MHz to Hz.
	result = sin( 2 * PI * mod.frequency * 1e6 * t + mod.phase );
	return result;
}


double Segment::pulseCalc( pulseData pulse, int iteration, long size, double pulseLength, double center )
{
	if ( pulse.type == "__NONE__" )
	{
		return 0;
	}
	else if ( pulse.type == "gaussian" )
	{
		// in this case, the width is the sigma of the gaussian.
		//double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		double result = pulse.amplitude * exp( -(center - x) * (center - x) / (pulse.width * pulse.width) );
		return result;
	}
	else if ( pulse.type == "lorentzian" )
	{
		// in this case, the width is the standard lorentzian full width half max. 
		double FWHM = pulse.width;
		//double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		// see definition: http://mathworld.wolfram.com/LorentzianFunction.html
		return pulse.amplitude * (FWHM / (2.0 * PI)) / ((x - center)*(x - center) + (FWHM / 2) * (FWHM / 2));

	}
	else if ( pulse.type == "sech" )
	{
		// in this case, the width is just the scaling factor for the length
		//double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		// see definition: http://mathworld.wolfram.com/HyperbolicSecant.html
		return pulse.amplitude * 1.0 / cosh( (x - center) / pulse.width );
	}
	else
	{
		thrower ( "ERROR: pulse type " + pulse.type + " is unrecognized.\r\n" );
		return 0;
	}
}


/*
 * This function uses the initial and final points along with the ramp and time of the segment to calculate all of the data points. This should be used so
 * as to, after this function, you have all of the powers that you want (not voltages), and then call the voltage converter afterwards.
 */
void Segment::calcData( unsigned long sampleRate )
{
	// calculate the size of the waveform.
	double numDataPointsf = finalSettings.time * sampleRate;
	// test if good time.
	if (fabs( numDataPointsf - round( numDataPointsf ) ) > 1e-6)
	{
		thrower ( "ERROR: Bad time entered for the time of an intensity sequence segment. The time was "
				 + str(finalSettings.time) + ", and the sample rate is " + str( sampleRate ) + ". The product of these"
				 " is the number of samples in the segment, and this must be an integer." );
	}
	int numDataPoints = (int)round( numDataPointsf );
	// resize to zero. This is a complete reset of the data points in the class.
	dataArray.resize( 0 );
	// writebtn the data points. These are all in powers. These are converted to voltages later.
	for ( int dataInc = 0; dataInc < numDataPoints; dataInc++ )
	{
		double point;
		if ( finalSettings.ramp.isRamp )
		{
			point = finalSettings.ramp.start + rampCalc( numDataPoints, dataInc, finalSettings.ramp.start,
														 finalSettings.ramp.end, finalSettings.ramp.type );
		}
		else if ( finalSettings.pulse.isPulse )
		{
			point = finalSettings.pulse.vOffset + pulseCalc( finalSettings.pulse, dataInc, numDataPoints, 
															 finalSettings.time, finalSettings.pulse.tOffset )
											   * modCalc( finalSettings.mod, dataInc, numDataPoints, finalSettings.time );
		}
		else
		{
			point = finalSettings.holdVal;
		}
		dataArray.push_back( point );
	}
}


unsigned Segment::returnDataSize()
{
	return dataArray.size();
}


double Segment::returnDataVal( long dataNum )
{
	return dataArray[dataNum];
}


void Segment::assignDataVal( int dataNum, double val )
{
	dataArray[dataNum] = val;
}

