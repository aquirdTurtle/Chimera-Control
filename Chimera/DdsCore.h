#pragma once 


#include "DdsSystemStructures.h"
#include "ExpWrap.h"
#include "ftdiFlume.h"
#include <vector>
#include <array>


/*
This class handles the programming of the DDS and not any of the gui elements associated with the gui system.
This is a part of the DdsSystem, and is meant to be subclassed as such. It is used by the gui system itself when the 
user programms the dds immediately without running an experiment. It is also the only part of the DdsSystem which
needs to be passed to the main experiment thread. This is part of a new attempt to better divide the gui functionality 
from the core functionality and have a more minimal object passed into the main experiment thread. 
*/
class DdsCore
{
	public:
		DdsCore ( bool safemode );
		~DdsCore ( );
		void getRampListFromConfig ( );
		void writeExperiment ( UINT sequenceNum, UINT variationNum );
		void connectasync ( );
		void disconnect ( );
		void writeOneRamp ( ddsRampFinFullInfo boxRamp, UINT8 rampIndex );
		std::vector<ddsRampFinFullInfo> analyzeRampList ( std::vector<ddsIndvRampListInfo> rampList );
		void generateFullExpInfo ( );
		void evaluateDdsInfo ( std::vector<parameterType> params = std::vector<parameterType> ( ),
							   UINT variationNumber = 0, UINT sequenceNumber = 0 );
		void forceRampsConsistent ( );
		void updateRampLists ( ExpWrap<std::vector<ddsIndvRampListInfo>> rampList );
		std::string getSystemInfo ( );
		void clearDdsRampMemory ( );
	private:
		ExpWrap<std::vector<ddsIndvRampListInfo>> rampLists;
		ExpWrap<std::vector<ddsRampFinFullInfo>> fullExpInfo;
		ddsConnectionType::type connType;
		const UINT MSGLENGTH = 7;
		const unsigned char WBWRITE = (unsigned char) 161;
		const unsigned char WBWRITE_ARRAY = (unsigned char) 2; //Add 2 to WBWRITE
		const double INTERNAL_CLOCK = ( double ) 500.0; //Internal clock in MHz

		const UINT16 RESET_FREQ_ADDR_OFFSET = 0x0;
		const UINT16 RESET_AMP_ADDR_OFFSET = 0x100;
		const UINT16 REPS_ADDRESS_OFFSET = 0x200;
		const UINT16 RAMP_CHANNEL_ADDR_SPACING = 0x200;
		const UINT16 RAMP_FREQ_ADDR_OFFSET = 0x400;
		const UINT16 RAMP_AMP_ADDR_OFFSET = 0x1400;

		ftdiFlume ftFlume;
		const double defaultFreq = 10;
		const double defaultAmp = 100;

		void longUpdate ( );
		void lockPLLs ( );
		// get (frequency/amplitude) ? word
		std::array<UINT8, 4> intTo4Bytes ( int i_ );
		void writeDDS ( UINT8 DEVICE, UINT16 ADDRESS, std::array<UINT8, 4> data );
		INT getFTW ( double freq );
		UINT getATW ( double amp );
		UINT16 getRepsFromTime ( double time );
		INT get32bitATW ( double amp );
		void channelSelect ( UINT8 device, UINT8 channel );

		void writeAmpMultiplier ( UINT8 device );
		void writeResetFreq ( UINT8 device, UINT8 channel, double freq );
		void writeResetAmp ( UINT8 device, UINT8 channel, double amp );
		void writeRampReps ( UINT8 index, UINT16 reps );
		void writeRampDeltaFreq ( UINT8 device, UINT8 channel, UINT8 index, double deltafreq );
		void writeRampDeltaAmp ( UINT8 device, UINT8 channel, UINT8 index, double deltaamp );
		bool log = true;
		std::string writeLogFileName = "DDS_Write_Log.txt";
		std::ofstream writeLog;
		// readDDS ( UINT device, UINT16 address );
		void writeDDS ( UINT8 device, UINT16 address, UINT8 dat1, UINT8 dat2, UINT8 dat3, UINT8 dat4 );
};