// created by Mark O. Brown
#pragma once
#include <string>
#include "nidaqmx2.h"
#include <vector>

/*
 * This is an incomplete wrapper around the DAQmx api. Incomplete in the sense that it doesn't wrap all of DAQmx (which
 * is large). It only wraps what I currently use. Wrappers like this serve to ease working with the API, standardize
 * error reporting between differnet apis (I use the thrower object for error reporting), and to facilitate a safemode.
 */
class DaqMxFlume
{
	public:
		DaqMxFlume( bool safemode );
		/// My wrappers for all of the daqmx functions that I use currently. If I needed to use another function, I'd 
		/// create another wrapper.
		// note that DAQ stands for Data AQuisition (software). It's not a typo!
		void createTask( const char* taskName, TaskHandle& handle );

		void createAoVoltageChan( TaskHandle taskHandle, const char physicalChannel[],
									 const char nameToAssignToChannel[], float64 minVal, float64 maxVal, int32 units,
									 const char customScaleName[] );
		void createAiVoltageChan( TaskHandle taskHandle, const char physicalChannel[], 
								  const char nameToAssignToChannel[], int32 terminalConfig, float64 minVal,
								  float64 maxVal, int32 units, const char customScaleName[] );
		void createDiChan( TaskHandle taskHandle, const char lines[], const char nameToAssignToLines[],
							  int32 lineGrouping );
		void stopTask( TaskHandle handle );
		void configSampleClkTiming( TaskHandle taskHandle, const char source[], float64 rate, int32 activeEdge,
									   int32 sampleMode, uInt64 sampsPerChan );
		void writeAnalogF64( TaskHandle handle, int32 numSampsPerChan, bool32 autoStart, float64 timeout,
								bool32 dataLayout, const float64 writeArray[], int32 *sampsPerChanWritten );
		void startTask( TaskHandle handle );
		std::string getDacSystemInfo(std::array<std::string, 3> boardNames);
		void readAnalogF64( TaskHandle taskHandle, std::vector<float64> &readData, int32& sampsPerChanRead );
		long getProductCategory( std::string deviceLocation );
		const bool safemode;
	private:
		std::string getErrorMessage( int errorCode );
		
};