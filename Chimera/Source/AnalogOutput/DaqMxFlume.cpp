// created by Mark O. Brown
#include "stdafx.h"
#include "DaqMxFlume.h"
#include <AnalogInput/AiSystem.h>

DaqMxFlume::DaqMxFlume( bool safemodeOption ) : safemode ( safemodeOption )
{

}

std::string DaqMxFlume::getErrorMessage( int errorCode )
{
	char errorChars[2048];
	// Get the actual error message. This is much surperior to getErrorString function.
	DAQmxGetExtendedErrorInfo( errorChars, 2048 );
	std::string errorString( errorChars );
	return errorString;
}

long DaqMxFlume::getProductCategory( std::string deviceLocation )
{
	long answer;
	int32 errCode = DAQmxGetDevProductCategory( cstr(deviceLocation), &answer );
	if ( errCode != 0 )
	{
		std::string err = getErrorMessage( errCode );
		thrower ("DAC System: Error! " + err);
	}
	return answer;
}


std::string DaqMxFlume::getDacSystemInfo( std::array<std::string, 3> boardNames )
{
	std::array<long, 3> answers;
	answers[0] = getProductCategory(boardNames[0] );
	answers[1] = getProductCategory(boardNames[0]);
	answers[2] = getProductCategory(boardNames[0]);
	std::string answerStr = "Dac System: Connected... device categories = " + str( answers[0] ) + ", " 
		+ str( answers[1] ) + ", " + str( answers[2] ) + ". Typical value 14647 = AO Series.\n";
	return answerStr;
}


void DaqMxFlume::createTask( const char* taskName, TaskHandle& handle )
{
	if ( !safemode )
	{
		int result = DAQmxCreateTask( taskName, &handle );
		if ( result )
		{
			thrower ( "daqCreateTask Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DaqMxFlume::readAnalogF64( TaskHandle taskHandle, std::vector<float64> &readData, int32& sampsPerChanRead )
{
	if ( !safemode )
	{
		// 3rd argument = timeout of 10s, pretty arbitrary. 10s is prety long actually.
		// *16 in the size because number of 
		int result = DAQmxReadAnalogF64( taskHandle, readData.size() / AiSystem::NUMBER_AI_CHANNELS, 10.0,
										 DAQmx_Val_GroupByChannel, readData.data(), 
										 readData.size(), &sampsPerChanRead, nullptr);
		if ( result )
		{
			thrower ( "daqCreateAiVoltageChan Failed! (" + str( result ) + "): "
					  + getErrorMessage( result ) );
		}
	}
	else
	{
		for (auto& dp : readData)
		{
			dp = double(rand ()) / RAND_MAX;
		}
	}
}


void DaqMxFlume::createAiVoltageChan( TaskHandle taskHandle, const char physicalChannel[],
									  const char nameToAssignToChannel[], int32 terminalConfig, float64 minVal, 
									  float64 maxVal, int32 units, const char customScaleName[] )
{
	if ( !safemode )
	{
		int result = DAQmxCreateAIVoltageChan( taskHandle, physicalChannel, nameToAssignToChannel, terminalConfig, 
											   minVal, maxVal, units, customScaleName );
		if ( result )
		{
			thrower ( "daqCreateAiVoltageChan Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DaqMxFlume::createAoVoltageChan( TaskHandle taskHandle, const char physicalChannel[],
										 const char nameToAssignToChannel[], float64 minVal, float64 maxVal, int32 units,
										 const char customScaleName[] )
{
	if ( !safemode )
	{
		int result = DAQmxCreateAOVoltageChan( taskHandle, physicalChannel, nameToAssignToChannel, minVal, maxVal,
											   units, customScaleName );
		if ( result )
		{
			thrower ( "daqCreateAOVoltageChan Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DaqMxFlume::createDiChan( TaskHandle taskHandle, const char lines[], const char nameToAssignToLines[],
								  int32 lineGrouping )
{
	if ( !safemode )
	{
		int result = DAQmxCreateDIChan( taskHandle, lines, nameToAssignToLines, lineGrouping );
		if ( result )
		{
			thrower ( "daqCreateDIChan Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DaqMxFlume::stopTask( TaskHandle handle )
{
	if ( !safemode )
	{
		int result = DAQmxStopTask( handle );
		// this function is currently meant to be silent.
		if ( result )
		{
			//thrower ( "stopTask Failed! (" + str( result ) + "): "
			//		 + getErrorMessage( result ) );

		}
	}
}


void DaqMxFlume::configSampleClkTiming( TaskHandle taskHandle, const char source[], float64 rate, int32 activeEdge,
										   int32 sampleMode, uInt64 sampsPerChan )
{
	if ( !safemode )
	{
		int result = DAQmxCfgSampClkTiming( taskHandle, source, rate, activeEdge, sampleMode, sampsPerChan );
		if ( result )
		{
			thrower ( "configSampleClkTiming Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DaqMxFlume::writeAnalogF64( TaskHandle handle, int32 numSampsPerChan, bool32 autoStart, float64 timeout,
									bool32 dataLayout, const float64 writeArray[], int32 *sampsPerChanWritten )
{
	if ( !safemode )
	{
		// the last argument must be null as of the writing of this wrapper. may be used in the future for something else.
		int result = DAQmxWriteAnalogF64( handle, numSampsPerChan, autoStart, timeout, dataLayout, writeArray,
										  sampsPerChanWritten, nullptr );
		if ( result )
		{
			thrower ( "writeAnalogF64 Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}


void DaqMxFlume::startTask( TaskHandle handle )
{
	if ( !safemode )
	{
		int result = DAQmxStartTask( handle );
		if ( result )
		{
			thrower ( "startTask Failed! (" + str( result ) + "): "
					 + getErrorMessage( result ) );
		}
	}
}