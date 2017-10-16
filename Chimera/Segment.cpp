#include "stdafx.h"
#include "Segment.h"


void Segment::convertInputToFinal( UINT variation, std::vector<variableType>& variables )
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
		if ( finalSettings.ramp.type == "nr" )
		{
			finalSettings.ramp.end = finalSettings.ramp.start;
		}
		else
		{
			finalSettings.ramp.end = input.ramp.end.evaluate( variables, variation );
		}
	}
	else if ( input.pulse.isPulse )
	{
		finalSettings.pulse.type = input.pulse.type;
		finalSettings.pulse.offset = input.pulse.offset.evaluate( variables, variation );
		finalSettings.pulse.amplitude = input.pulse.amplitude.evaluate( variables, variation );
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

	// time
	finalSettings.time = input.time.evaluate( variables, variation) / 1000.0;
	// repeat number
	// (0 here corresponds to "repeat", in which case you need a number of times to repeat.);
	if (finalSettings.continuationType == 0)
	{
		finalSettings.repeatNum = UINT( input.repeatNum.evaluate( variables, variation ));
	}
}


void Segment::convertInputToFinal()
{	// first transfer things that can't be varied.
	finalSettings.continuationType = input.continuationType;
	// handle more complicated things.
	finalSettings.ramp.isRamp = input.ramp.isRamp;
	finalSettings.pulse.type = input.pulse.type;
	finalSettings.pulse.isPulse = input.pulse.isPulse;
	finalSettings.mod.modulationIsOn = input.mod.modulationIsOn;

	if ( input.ramp.isRamp )
	{
		finalSettings.ramp.type = input.ramp.type;
		finalSettings.ramp.start = input.ramp.start.evaluate( );
		if ( finalSettings.ramp.type == "nr" )
		{
			finalSettings.ramp.end = finalSettings.ramp.start;
		}
		else
		{
			finalSettings.ramp.end = input.ramp.end.evaluate( );
		}
	}
	else if ( input.pulse.isPulse )
	{
		finalSettings.pulse.offset = input.pulse.offset.evaluate( );
		finalSettings.pulse.amplitude = input.pulse.amplitude.evaluate( );
		finalSettings.pulse.width = input.pulse.width.evaluate( ) / 1000.0;
	}
	else
	{
		finalSettings.holdVal = input.holdVal.evaluate( );
	}

	if ( input.mod.modulationIsOn )
	{
		finalSettings.mod.frequency = input.mod.frequency.evaluate( );
		finalSettings.mod.phase = input.mod.phase.evaluate( );
	}

	// time
	finalSettings.time = input.time.evaluate( ) / 1000.0;
	// repeat number
	// (0 here corresponds to "repeat", in which case you need a number of times to repeat.);
	if ( finalSettings.continuationType == 0 )
	{
		finalSettings.repeatNum = UINT( input.repeatNum.evaluate( ) );
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
	else
	{
		// error message. I've already checked (outside this function) whether the ramp-type is a filename.
		thrower( "ERROR: ramp type " + rampType + " is unrecognized.\r\n" );
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


double Segment::pulseCalc( pulseData pulse, int iteration, long size, double pulseLength )
{
	if ( pulse.type == "__NONE__" )
	{
		return 0;
	}
	// for no ramp
	else if ( pulse.type == "gaussian" )
	{
		// in this case, the width is the sigma of the gaussian.
		double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		double result = pulse.amplitude * exp( -(center - x) * (center - x) / (pulse.width * pulse.width) );
		return result;
	}
	else if ( pulse.type == "lorentzian" )
	{
		// in this case, the width is the standard lorentzian full width half max. 
		double FWHM = pulse.width;
		double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		// see definition: http://mathworld.wolfram.com/LorentzianFunction.html
		return pulse.amplitude * (FWHM / (2.0 * PI)) / ((x - center)*(x - center) + (FWHM / 2) * (FWHM / 2));

	}
	else if ( pulse.type == "sech" )
	{
		// in this case, the width is just the scaling factor for the length
		double center = pulseLength / 2.0;
		double x = pulseLength * iteration / size;
		// see definition: http://mathworld.wolfram.com/HyperbolicSecant.html
		return pulse.amplitude * 1.0 / cosh( (x - center) / pulse.width );
	}
	else
	{
		thrower( "ERROR: pulse type " + pulse.type + " is unrecognized.\r\n" );
		return 0;
	}
}


/*
* This function uses the initial and final points along with the ramp and time of the segment to calculate all of the data points. This should be used so
* as to, after this function, you have all of the powers that you want (not voltages), and then call the voltage converter afterwards.
*/
void Segment::calcData( ULONG sampleRate )
{
	// calculate the size of the waveform.
	double numDataPointsf = finalSettings.time * sampleRate;
	// test if good time.
	if (fabs( numDataPointsf - round( numDataPointsf ) ) > 1e-6)
	{
		// Bad Time Warning
		thrower( "ERROR: Bad time entered for the time of an intensity sequence segment. This resulted in a non-integer number of samples. Time cannot be"
				 "defined with precision below the microsecond level for normal sample rates." );
	}
	// Convert to integer
	int numDataPoints = (int)round( numDataPointsf );
	// resize to zero. This is a complete reset of the data points in the class.
	dataArray.resize( 0 );
	// write the data points. These are all in powers. These are converted to voltages later.
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
			point = finalSettings.pulse.offset + pulseCalc( finalSettings.pulse, dataInc, numDataPoints, finalSettings.time )
												* modCalc( finalSettings.mod, dataInc, numDataPoints, finalSettings.time );
		}
		else
		{
			point = finalSettings.holdVal;
		}
		dataArray.push_back( point );
	}
}


UINT Segment::returnDataSize()
{
	return dataArray.size();
}


/*
* return dataArray[dataNum];
*/
double Segment::returnDataVal( long dataNum )
{
	return dataArray[dataNum];
}


/*
* dataArray[dataNum] = val;
*/
void Segment::assignDataVal( int dataNum, double val )
{
	dataArray[dataNum] = val;
}

