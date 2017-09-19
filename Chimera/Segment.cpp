#include "stdafx.h"
#include "Segment.h"


void Segment::convertInputToFinal( key variableKey, UINT variation, std::vector<variableType>& vars )
{	
	// first transfer things that can't be varied.
	finalSettings.segmentType = input.segmentType;
	finalSettings.continuationType = input.continuationType;
	finalSettings.rampType = input.rampType;
	// handle more complicated things.
	finalSettings.initValue = input.initValue.evaluate( variableKey, variation, vars );
	if (finalSettings.rampType == "nr")
	{
		finalSettings.finValue = finalSettings.initValue;
	}
	else
	{
		finalSettings.finValue = input.finValue.evaluate( variableKey, variation, vars );
	}
	// time
	finalSettings.time = input.time.evaluate( variableKey, variation, vars ) / 1000.0;
	// repeat number
	// (0 here corresponds to "repeat", in which case you need a number of times to repeat.);
	if (finalSettings.continuationType == 0)
	{
		finalSettings.repeatNum = UINT( input.repeatNum.evaluate( variableKey, variation, vars ));
	}
}


void Segment::convertInputToFinal()
{
	// first transfer things that can't be varied.
	finalSettings.segmentType = input.segmentType;
	finalSettings.continuationType = input.continuationType;
	finalSettings.rampType = input.rampType;
	// handle more complicated things.
	finalSettings.initValue = input.initValue.evaluate(  );
	if (finalSettings.rampType == "nr")
	{
		finalSettings.finValue = finalSettings.initValue;
	}
	else
	{
		finalSettings.finValue = input.finValue.evaluate( );
	}
	// time
	finalSettings.time = input.time.evaluate() / 1000.0;
	// repeat number
	// (0 here corresponds to "repeat");
	if (finalSettings.continuationType == 0)
	{
		finalSettings.repeatNum = UINT( input.repeatNum.evaluate( ));
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



/*
* The constructor doesn't take arguments and initializes some values here to bad (but not garbage) values.
*/
Segment::Segment()
{
	// initialize everything to invalid values.
	input.initValue.expressionStr = "";
	input.finValue.expressionStr = "";
	input.time.expressionStr = "";
	input.continuationType = -1;
	input.segmentType = -1;
	input.rampType = "";
	input.repeatNum.expressionStr = "";
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
	if (finalSettings.rampType == "nr" || finalSettings.rampType == "lin" || finalSettings.rampType == "tanh")
	{
		for (int dataInc = 0; dataInc < numDataPoints; dataInc++)
		{
			// constant waveform. Every data point is the same.
			dataArray.push_back( finalSettings.initValue + rampCalc( numDataPoints, dataInc, finalSettings.initValue, finalSettings.finValue, finalSettings.rampType ) );
		}
	}
	else if (finalSettings.rampType == "")
	{
		// Error: Ramp Type has not been set!
		thrower( "ERROR: Data points tried to be written when the ramp type hadn't been set!" );
	}
	else
	{
		// invalid ramp Type!
		std::string errMsg;
		errMsg = "ERROR: Invalid Ramp Type in intensity sequence! Type entered was: " + finalSettings.rampType + ".";
		thrower( errMsg );
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

