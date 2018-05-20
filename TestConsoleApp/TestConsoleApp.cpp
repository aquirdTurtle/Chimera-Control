#include "stdafx.h" 
#include <iostream>
#include <chrono>
#include <vector>
#include "../Chimera/range.h"
#include "Windows.h"
#include "../Chimera/Thrower.h"
#include <fstream>
#include <string>
#include "../Chimera/my_str.h"

#define NIAWG_SAMPLE_RATE 320000000
#define PI 3.14159265
#define TOTAL_POWER 1.5

template <typename T> void errBox( T msg )
{
	MessageBox( NULL, cstr( msg ), "ERROR!", MB_ICONERROR | MB_SYSTEMMODAL );
}

boost::integer_range<ULONG> range( ULONG stop );
boost::integer_range<UINT> range( UINT stop );
boost::integer_range<int> range( int stop );


boost::integer_range<ULONG> range( ULONG stop )
{
	return boost::irange<ULONG>( ULONG( 0 ), stop );
}

boost::integer_range<UINT> range( UINT stop )
{
	return boost::irange<UINT>( UINT( 0 ), stop );
}

boost::integer_range<int> range( int stop )
{
	return boost::irange<int>( 0, stop );
}



struct waveSignal
{
	double freqInit;
	double freqFin;
	std::string freqRampType;

	double initPower;
	double finPower;
	std::string powerRampType;

	double initPhase;
	// Asssigned only after a waveform is calculated or read.
	double finPhase;
};;

struct channelWave
{
	std::vector<waveSignal> signals;
	// should be 0 until this option is re-implemented!
	int phaseOption = 0;
	int initType = 0;
	// This should usually just be a single char, but if it's wrong I can use the whole string to debug.
	std::string delim;
	// the actual waveform data.
	std::vector<double> wave;
};;



void calcWaveData( channelWave& inputData, std::vector<double>& readData, long int sampleNum, double waveTime,
				   bool powerCap, bool constPower );

double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType );

double rampCalc( int size, int iteration, double initPos, double finPos, std::string rampType )
{
	// for linear ramps
	if ( rampType == "lin" )
	{
		return iteration * (finPos - initPos) / size;
	}
	// for no ramp
	else if ( rampType == "nr" )
	{
		return 0;
	}
	// for hyperbolic tangent ramps
	else if ( rampType == "tanh" )
	{
		return (finPos - initPos) * (tanh( -4 + 8 * (double)iteration / size ) + 1) / 2;
	}
	// error message. I've already checked (outside this function) whether the ramp-type is a filename.
	else
	{
		std::string errMsg = "ERROR: ramp type " + rampType + " is unrecognized. If this is a file name, make sure the file exists and is in the project folder.\r\n";
		errBox( errMsg );
		return 0;
	}
}

void calcWaveData( channelWave& inputData, std::vector<double>& readData, long int sampleNum, double waveTime, 
				   bool powerCap, bool constPower )
{
	// Declarations
	std::vector<double> powerPos, freqRampPos, phasePos( inputData.signals.size( ) );
	std::vector<double*> powerRampFileData, freqRampFileData;
	std::fstream powerRampFile, freqRampFile;
	std::string tempStr;

	/// deal with ramp calibration files. check all signals for files and read if yes.
	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		if ( inputData.signals[signal].initPhase < 0 )
		{
			thrower( "ERROR: initial phase of waveform was negative! This shouldn't happen. At this point, if using -1,"
					 "phase from prev waveform should have been grabbed already." );
		}
		// create spots for the ramp positions.
		powerPos.push_back( 0 );
		freqRampPos.push_back( 0 );
		// If the ramp type isn't a standard command...
		if ( inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr"
			 && inputData.signals[signal].powerRampType != "tanh" )
		{
			powerRampFileData.push_back( new double[sampleNum] );
			// try to open it as a file
			powerRampFile.open( inputData.signals[signal].powerRampType );
			// if successful....
			if ( powerRampFile.is_open( ) )
			{
				int powerValNumber = 0;
				// load the data in
				while ( !powerRampFile.eof( ) )
				{
					powerRampFile >> powerRampFileData[signal][powerValNumber];
					powerValNumber++;
				}
				// error message for bad size (powerRampFile.eof() reached too early or too late).
				if ( powerValNumber != sampleNum + 1 )
				{
					thrower( "ERROR: niawg ramp file not the correct size?\nSize of upload is " + str( powerValNumber )
							 + "; size of file is " + str( sampleNum ) );
				}
				// close the file.
				powerRampFile.close( );
			}
			else
			{
				thrower( "ERROR: niawg ramp type " + str( inputData.signals[signal].powerRampType ) + " is unrecognized. If "
						 "this is a file name, make sure the file exists and is in the project folder. " );
			}
		}
		// If the ramp type isn't a standard command...
		if ( inputData.signals[signal].freqRampType != "lin" && inputData.signals[signal].freqRampType != "nr"
			 && inputData.signals[signal].freqRampType != "tanh" && inputData.signals[signal].freqRampType != "fast" )
		{
			// try to open it
			freqRampFileData.push_back( new double[sampleNum] );
			freqRampFile.open( inputData.signals[signal].freqRampType, std::ios::in );
			// if open
			if ( freqRampFile.is_open( ) )
			{
				int freqRampValNum = 0;
				while ( !freqRampFile.eof( ) )
				{
					freqRampFile >> freqRampFileData[signal][freqRampValNum];
					freqRampValNum++;
				}
				// error message for bad size (powerRampFile.eof() reached too early or too late).
				if ( freqRampValNum != sampleNum + 1 )
				{
					thrower( "ERROR: niawg freq ramp file not the correct size?\nSize of upload is " + str( freqRampValNum )
							 + "; size of file is " + str( sampleNum ) );
				}
				// close file
				freqRampFile.close( );
			}
			else
			{
				thrower( "ERROR: niawg freq ramp type " + inputData.signals[signal].freqRampType + " is unrecognized. "
						 "If this is a file name, make sure the file exists and is in the project folder." );
			}
		}
	}
	/// Pre-calculate a bunch of parameters for the ramps. 
	// These are not all used, but it's simple one-time calcs so I just od them anyways.
	auto& t_r = waveTime;
	auto t_r2 = t_r / 2;
	std::vector<double> deltaOmega;
	std::vector<double> deltaNu;
	std::vector<double> accel_w0;
	std::vector<double> accel_w1;
	std::vector<double> jerk;
	std::vector<double> freq_1;
	std::vector<double> phi_halfway;
	double deltaTanh = std::tanh( 4 ) - std::tanh( -4 );
	for ( UINT signal = 0; signal < inputData.signals.size( ); signal++ )
	{
		// I try to keep the "auto" aliases here consistent with what's used later in the calculation.
		auto f_0 = inputData.signals[signal].freqInit;
		auto dNu = (inputData.signals[signal].freqFin - f_0);
		deltaNu.push_back( dNu );
		auto dOmega = 2 * PI *  dNu;
		deltaOmega.push_back( dOmega );
		// initial phase acceleration, can modify this to give tweezer an initial velocity. 0 = start from static
		auto a_w0 = 0;//2 * PI * dNu / t_r;
		accel_w0.push_back( a_w0 );
		auto a_w1 = 4 * PI * dNu / t_r - a_w0;
		accel_w1.push_back( a_w1 );
		jerk.push_back( 8 * PI * dNu / (t_r*t_r) - 4 * a_w0 / t_r );
		freq_1.push_back( f_0 + dNu / 2 );
		auto phi_0 = inputData.signals[signal].initPhase;
		phi_halfway.push_back( 0.5 * a_w0 * (t_r2*t_r2) + (t_r2 / 6.0) * (2 * PI * dNu - a_w0 * t_r)
							   + 2 * PI * f_0 * t_r2 + phi_0 );
	}
	///		Get Data Points.		///
	int sample = 0;
	/// increment through all samples
	for ( ; sample < sampleNum; sample++ )
	{
		// calculate the time that this j number refers to
		double t = (double)sample / NIAWG_SAMPLE_RATE;
		/// Calculate Phase and Power Positions. For Every signal...
		for ( auto signal : range( inputData.signals.size( ) ) )
		{
			// these "auto" aliases should match what was used above to calculate constants.
			auto dOmega = deltaOmega[signal];
			auto phi_0 = inputData.signals[signal].initPhase;
			auto f_0 = inputData.signals[signal].freqInit;
			/// Handle Frequency Ramps
			// Frequency ramps are actually a little complex. we have dPhi/dt = omega(t) and we need phi to calculate data points. So in order to get 
			// the phase you need to integrate the omega(t) you want and modify the integration constant to get your initial phase.
			if ( inputData.signals[signal].freqRampType == "lin" )
			{
				// W{t} = Wi + (DeltaW * t) / (Tfin)
				// Phi{t}   = Wi * t + (DeltaW * t ^ 2) / 2 + phi_i
				phasePos[signal] = 2 * PI * f_0 * t + dOmega * pow( t, 2 ) / (2 * t_r) + phi_0;
			}
			else if ( inputData.signals[signal].freqRampType == "tanh" )
			{
				// We want
				// W{t} = Wi + (DeltaW * (Tanh{-4 + 8 * t' / Tf} - Tanh{-4}) / (Tanh{4} - Tanh{-4})
				// This gives the correct initial value, final value, and symmetry of the ramp for the frequency.
				// -4 was picked somewhat arbitrarily. The integral of this is
				// dw/(2*tanh{4}) * T_f/8 * ln{cosh{-4+8t/T_f}} + (w_i - (dw * tanh{-4})/2*tanh{4}) t + C
				// Evaluating C to give the correct phase gives
				// phi{t} = (w_i+dw/2)t+(dw)/dtanh * T_f/8 * (ln{cosh{-4+8t/T_f}}-ln{cosh{-4}}) + phi_0
				// See onenote for more math.
				phasePos[signal] = (2 * PI * f_0 + dOmega / 2.0) * t + (dOmega / deltaTanh) * (t_r / 8.0)
					* (std::log( std::cosh( 4 - (8 / t_r) * t ) ) - std::log( std::cosh( 4 ) )) + phi_0;
			}
			else if ( inputData.signals[signal].freqRampType == "nr" )
			{
				// omega{t} = omega
				// phi = omega*t
				phasePos[signal] = 2 * PI * f_0 * t + phi_0;
			}
			else if ( inputData.signals[signal].freqRampType == "fast" )
			{
				// these "auto" aliases should match what was used above to calculate constants.
				auto a_w0 = accel_w0[signal];
				auto a_w1 = accel_w1[signal];
				auto dNu = deltaNu[signal];
				auto J = jerk[signal];
				auto f_1 = freq_1[signal];
				auto phi_1 = phi_halfway[signal];
				// constant phase-jerk ramp, except (optionally) an initial phase acceleration. I have a jupyter 
				// notebook about this.
				if ( t < t_r2 )
				{
					phasePos[signal] = (1.0 / 6.0) * J * (t * t * t) + 0.5 * a_w0 * (t * t) + 2.0 * PI * f_0 * t + phi_0;
				}
				else
				{
					auto tp = t - t_r2;
					phasePos[signal] = -(1.0 / 6.0) * J * (tp*tp*tp) + 0.5 * a_w1 * (tp*tp) + 2.0 * PI * f_1 * tp + phi_1;
				}
			}
			else
			{
				// special ramp case. I'm not sure if this is actually useful. 
				// The frequency file would have to be designed very carefully.
				freqRampPos[signal] = freqRampFileData[signal][sample] * (deltaNu[signal]);
				phasePos[signal] = (double)sample * 2 * PI * (f_0 + freqRampPos[signal]) / NIAWG_SAMPLE_RATE
					+ phi_0;
			}

			/// handle amplitude ramps, which are much simpler.
			if ( inputData.signals[signal].powerRampType != "lin" && inputData.signals[signal].powerRampType != "nr"
				 && inputData.signals[signal].powerRampType
				 != "tanh" )
			{
				// use data from file
				powerPos[signal] = powerRampFileData[signal][sample] * (inputData.signals[signal].finPower
																		 - inputData.signals[signal].initPower);
			}
			else
			{
				// use the ramp calc function to find the current power.
				powerPos[signal] = rampCalc( sampleNum, sample, inputData.signals[signal].initPower,
															  inputData.signals[signal].finPower,
															  inputData.signals[signal].powerRampType );
			}
		}

		/// If option is marked, then normalize the power.
		if ( constPower )
		{
			double currentPower = 0;
			// calculate the total current amplitude.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				currentPower += fabs( inputData.signals[signal].initPower + powerPos[signal] );
				/// modify here for frequency-dependent calibrations!
				/// need current frequency and calibration file.
			}

			// normalize each signal.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				// After this, a "currentPower" calculated the same above will always give TOTAL_POWER. 
				powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal])
					* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
			}
		}
		else if ( powerCap )
		{
			double currentPower = 0;
			// calculate the total current amplitude.
			for ( auto signal : range( inputData.signals.size( ) ) )
			{
				currentPower += fabs( inputData.signals[signal].initPower + powerPos[signal] );
				/// modify here for frequency-dependent calibrations!
				/// need current frequency and calibration file.
			}
			// normalize each signal only if power is above TOTAL_POWER. hence the word "cap".
			if ( currentPower > TOTAL_POWER )
			{
				for ( auto signal : range( inputData.signals.size( ) ) )
				{
					// After this, a "currentPower" calculated the same above will always give TOTAL_POWER. 
					powerPos[signal] = (inputData.signals[signal].initPower + powerPos[signal])
						* (TOTAL_POWER / currentPower) - inputData.signals[signal].initPower;
				}
			}
		}

		///  finally, Calculate voltage data point.
		readData[sample] = 0;
		for ( auto signal : range( inputData.signals.size( ) ) )
		{
			// get data point. V = Sqrt(Power) * Sin(Phase)
			readData[sample] += sqrt( inputData.signals[signal].initPower + powerPos[signal] ) * sin( phasePos[signal] );
		}
	}

	/// Calculate one last time for the final phases. I want the final phase to be the phase of the NEXT data point. 
	/// Then, following waveforms can START at this phase.
	double curTime = (double)sample / NIAWG_SAMPLE_RATE;
	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		// Calculate Phase Position. See above for description.
		if ( inputData.signals[signal].freqRampType == "lin" )
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime
				+ deltaOmega[signal] * pow( curTime, 2 ) * 1 / (2 * waveTime)
				+ inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "tanh" )
		{
			phasePos[signal] = (2 * PI * inputData.signals[signal].freqInit + deltaOmega[signal] / 2.0) * curTime
				+ (deltaOmega[signal] / deltaTanh) * (waveTime / 8.0) * std::log( std::cosh( 4 - (8 / waveTime) * curTime ) )
				- (deltaOmega[signal] / deltaTanh) * (waveTime / 8.0) * std::log( std::cosh( 4 ) )
				+ inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "nr" )
		{
			phasePos[signal] = 2 * PI * inputData.signals[signal].freqInit * curTime + inputData.signals[signal].initPhase;
		}
		else if ( inputData.signals[signal].freqRampType == "fast" )
		{
			// these "auto" aliases should match what was used above to calculate constants.
			auto t = curTime;
			auto f_0 = inputData.signals[signal].freqInit;
			auto phi_0 = inputData.signals[signal].initPhase;
			auto a_w0 = accel_w0[signal];
			auto a_w1 = accel_w1[signal];
			auto dNu = deltaNu[signal];
			auto J = jerk[signal];
			auto f_1 = freq_1[signal];
			auto phi_1 = phi_halfway[signal];
			// constant phase-jerk ramp, except (optionally) an initial phase acceleration. I have a jupyter 
			// notebook about this.
			if ( t < t_r2 )
			{
				phasePos[signal] = (1.0 / 6.0) * J * (t*t*t) + 0.5 * a_w0 * (t * 2) + 2 * PI * f_0 * t + phi_0;
			}
			else
			{
				auto tp = t - t_r2;
				phasePos[signal] = -(1.0 / 6.0) * J * (tp*tp*tp) + 0.5 * a_w1 * (tp*tp) + 2.0 * PI * f_1 * tp + phi_1;
			}
		}
		else
		{
			freqRampPos[signal] = freqRampFileData[signal][sample] * (inputData.signals[signal].freqFin
																	   - inputData.signals[signal].freqInit);
			phasePos[signal] = (double)sample * 2 * PI * (inputData.signals[signal].freqInit
															 + freqRampPos[signal]) / (NIAWG_SAMPLE_RATE)
				+inputData.signals[signal].initPhase;
		}
		// Don't need amplitude info.
	}

	for ( auto signal : range( inputData.signals.size( ) ) )
	{
		// get the final phase of this waveform. Note that this is the phase of the /next/ data point (the last time 
		// signalInc gets incremented, the for loop doesn't run) so that if the next waveform starts at this data 
		// point, it will avoid repeating the same data point. This is used for the option where the user uses this 
		// phase as the starting phase of the next waveform.
		inputData.signals[signal].finPhase = fmod( phasePos[signal], 2 * PI );
		// catch the case in which the final phase is virtually identical to 2*PI, which isn't caught in the above 
		// line because of bad floating point arithmetic.
		if ( fabs( inputData.signals[signal].finPhase - 2 * PI ) < 0.00000005 )
		{
			inputData.signals[signal].finPhase = 0;
		}
		// put the final phase in the last data point.
		readData[sample + signal] = inputData.signals[signal].finPhase;
	}
}


using namespace std;
using namespace chrono;


int main( )
{
	size_t numReps = 5;
	typedef steady_clock::time_point time;
	cout << "Beginning loop..." << endl;
	time begin = steady_clock::now( );	
	time end = steady_clock::now( );
	cout << "Average (" << numReps << " runs) Execution Time (ms): " 
		<< duration_cast<nanoseconds>(end - begin).count( ) / (1e6 * numReps) << endl;
	cin.get( );
	return 0;
}

