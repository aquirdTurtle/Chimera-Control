#pragma once
#include <string>
#include "nidaqmx2.h"

class DaqMxFlume
{
	public:
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
		std::string getDacSystemInfo( );
		void readAnalogF64( TaskHandle taskHandle, std::vector<float64> readData, int32& sampsPerChanRead );
	private:
		std::string getErrorMessage( int errorCode );
};