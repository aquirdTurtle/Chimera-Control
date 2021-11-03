
#include "stdafx.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "DdsCore.h"

std::string DdsCore::systemScope = "dds";

DdsCore::DdsCore ( bool safemode ) : ftFlume ( safemode ){
	connectasync ( );
	lockPLLs ( );
}

DdsCore::~DdsCore ( ){
	disconnect ( );
}

std::pair<DoRows::which, unsigned> DdsCore::getResetLine() {
	return resetTriggerLine;
}

std::pair<DoRows::which, unsigned> DdsCore::getStepLine() {
	return stepTriggerLine;
}

void DdsCore::assertDdsValuesValid ( std::vector<parameterType>& params ){
	unsigned variations = ( ( params.size ( ) ) == 0 ) ? 1 : params.front ( ).keyValues.size ( );
	for (auto& ramp : expRampList) {
		ramp.rampTime.assertValid (params, systemScope);
		ramp.freq1.assertValid (params, systemScope);
		ramp.freq2.assertValid (params, systemScope);
		ramp.amp1.assertValid (params, systemScope);
		ramp.amp2.assertValid (params, systemScope);
	}
}

void DdsCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	evaluateDdsInfo (params);
	unsigned variations = ((params.size ()) == 0) ? 1 : params.front ().keyValues.size ();
	generateFullExpInfo (variations);
}

// this probably needs an overload with a default value for the empty parameters case...
void DdsCore::evaluateDdsInfo ( std::vector<parameterType> params ){
	assertDdsValuesValid(params);
	unsigned variations = ( ( params.size ( ) ) == 0 ) ? 1 : params.front ( ).keyValues.size ( );
	for ( auto& ramp : expRampList)	{
		ramp.rampTime.internalEvaluate (params, variations );
		ramp.freq1.internalEvaluate (params, variations );
		ramp.freq2.internalEvaluate (params, variations );
		ramp.amp1.internalEvaluate (params, variations );
		ramp.amp2.internalEvaluate (params, variations );
		ramp.rampTime.internalEvaluate (params, variations );
	}
}

void DdsCore::programVariation ( unsigned variationNum, std::vector<parameterType>& params, 
								 ExpThreadWorker* threadworker){
	clearDdsRampMemory ( );
	auto& thisExpFullRampList = fullExpInfo ( variationNum );
	if ( thisExpFullRampList.size ( ) == 0 ){
		return;
	}
	lockPLLs ( );
	writeAmpMultiplier ( 0 );
	writeAmpMultiplier ( 1 );
	for ( auto boardNum : range ( thisExpFullRampList.front ( ).rampParams.numBoards ( ) ) ){
		for ( auto channelNum : range ( thisExpFullRampList.front ( ).rampParams.numChannels ( ) ) ){
			writeResetFreq ( boardNum, channelNum, thisExpFullRampList.front ( ).rampParams ( boardNum, channelNum ).freq1 );
			writeResetAmp ( boardNum, channelNum, thisExpFullRampList.front ( ).rampParams ( boardNum, channelNum ).amp1 );
		}
	}
	for ( auto rampIndex : range ( thisExpFullRampList.size ( ) ) ){
		writeOneRamp ( thisExpFullRampList[ rampIndex ], rampIndex );
	}
	longUpdate ( );
}


void DdsCore::writeOneRamp ( ddsRampFinFullInfo boxRamp, UINT8 rampIndex ){
	unsigned short reps = getRepsFromTime ( boxRamp.rampTime );
	if (reps == 0) {
		thrower("Zero time dds ramp detected?!");
	}
	writeRampReps ( rampIndex, reps );
	for ( auto boardNum : range ( boxRamp.rampParams.numBoards ( ) ) ){
		for ( auto channelNum : range ( boxRamp.rampParams.numChannels ( ) ) ){
			auto& channel = boxRamp.rampParams ( boardNum, channelNum );
			writeRampDeltaFreq ( boardNum, channelNum, rampIndex, ( channel.freq2 - channel.freq1 ) / reps );
			writeRampDeltaAmp ( boardNum, channelNum, rampIndex, ( channel.amp2 - channel.amp1 ) / reps );
		}
	}
}

void DdsCore::generateFullExpInfo (unsigned numVariations){
	fullExpInfo.resizeVariations (numVariations);
	for (auto varInc : range (numVariations)){
		fullExpInfo (varInc) = analyzeRampList (expRampList, varInc);
	}
}

/*
converts the list of individual set ramps, as set by the user, to the full ramp list which contains the state
of each dds at each point in the ramp.
*/
std::vector<ddsRampFinFullInfo> DdsCore::analyzeRampList ( std::vector<ddsIndvRampListInfo> rampList, unsigned variation ){
	// always rewrite the full vector
	unsigned maxIndex = 0;
	for ( auto& rampL : rampList ){
		maxIndex = ( ( maxIndex > rampL.index ) ? maxIndex : rampL.index );
	}
	std::vector<ddsRampFinFullInfo> fullRampInfo ( maxIndex + 1 );

	for ( auto& rampL : rampList ){
		auto& rampF = fullRampInfo[ rampL.index ].rampParams ( rampL.channel / 4, rampL.channel % 4 );
		if ( rampF.explicitlySet == true ){
			if ( fullRampInfo[ rampL.index ].rampTime != rampL.rampTime.getValue ( variation ) ){
				thrower ( "The ramp times of different channels on the same ramp index must match!" );
			}
		}
		else{
			fullRampInfo[ rampL.index ].rampTime = rampL.rampTime.getValue ( variation );
			rampF.explicitlySet = true;
		}
		rampF.freq1 = rampL.freq1.getValue ( variation );
		rampF.freq2 = rampL.freq2.getValue ( variation );
		rampF.amp1 = rampL.amp1.getValue ( variation );
		rampF.amp2 = rampL.amp2.getValue ( variation );
	}
	return fullRampInfo;
}


void DdsCore::forceRampsConsistent ( ){
	for ( auto varInc : range ( fullExpInfo.getNumVariations ( ) ) ){
		if ( fullExpInfo ( varInc ).size ( ) == 0 ) continue;
		if ( fullExpInfo ( varInc ).size ( ) == 1 ) continue; // no consistency to check if only 1. 
		auto& thisExpRampList = fullExpInfo ( varInc );
		for ( auto boardInc : range ( thisExpRampList.front ( ).rampParams.numBoards ( ) ) ){
			for ( auto channelInc : range ( thisExpRampList.front ( ).rampParams.numChannels ( ) ) ){
				// find the initial values
				double currentDefaultFreq, currentDefaultAmp;
				bool used = false;
				for ( auto rampInc : range ( thisExpRampList.size ( ) ) ){
					if ( thisExpRampList[ rampInc ].rampParams ( boardInc, channelInc ).explicitlySet ){
						currentDefaultFreq = thisExpRampList[ rampInc ].rampParams ( boardInc, channelInc ).freq1;
						currentDefaultAmp = thisExpRampList[ rampInc ].rampParams ( boardInc, channelInc ).amp1;
						used = true;
						break;
					}
				}
				if ( !used ){
					currentDefaultFreq = defaultFreq;
					currentDefaultAmp = defaultAmp;
				}
				for ( auto rampInc : range ( thisExpRampList.size ( ) - 1 ) ){
					auto& thisRamp = thisExpRampList[ rampInc ].rampParams ( boardInc, channelInc );
					auto& nextRamp = thisExpRampList[ rampInc + 1 ].rampParams ( boardInc, channelInc );
					bool bothExplicit = ( thisRamp.explicitlySet && nextRamp.explicitlySet );

					// this first if clause is basically just to handle the special case of the first ramp.
					if ( thisRamp.explicitlySet == false ){
						thisRamp.freq1 = thisRamp.freq2 = currentDefaultFreq;
						thisRamp.amp1 = thisRamp.amp2 = currentDefaultAmp;
						thisRamp.explicitlySet = true;
					}
					currentDefaultFreq = thisRamp.freq2;
					currentDefaultAmp = thisRamp.amp2;

					if ( nextRamp.explicitlySet == false ){
						thisRamp.freq1 = thisRamp.freq2 = currentDefaultFreq;
						thisRamp.amp1 = thisRamp.amp2 = currentDefaultAmp;
						nextRamp.explicitlySet = true;
					}
					// then a new ramp that the user set explicitly has been reached, and I need to check that
					// the parameters match up.
					if ( thisRamp.freq2 != nextRamp.freq1 || thisRamp.amp2 != nextRamp.amp1 ){
						thrower ( "Incompatible explicitly set ramps!" );
					}
				}
			}
		}
	}
}

// a wrapper around the ftFlume open
void DdsCore::connectasync ( ){
	unsigned long numDevs = ftFlume.getNumDevices ( );
	if ( numDevs > 0 ){
		// hard coded for now but you can get the connected serial numbers via the getDeviceInfoList function.
		ftFlume.open ( "FT1FJ8PEB" );
		ftFlume.setUsbParams ( );
		connType = ddsConnectionType::type::Async;
	}
	else{
		thrower ( "No devices found." );
	}
}

void DdsCore::disconnect ( ){
	ftFlume.close ( );
}

std::string DdsCore::getSystemInfo ( ){
	unsigned numDev;
	std::string msg = "";
	try{
		numDev = ftFlume.getNumDevices ( );
		msg += "Number ft devices: " + str ( numDev ) + "\n";
	}
	catch ( ChimeraError& err ){
		msg += "Failed to Get number ft Devices! Error was: " + err.trace ( );
	}
	msg += ftFlume.getDeviceInfoList ( );
	return msg;
}

/* Get Frequency Tuning Word - convert a frequency in double to  */
int DdsCore::getFTW ( double freq ){
	// Negative ints, Nyquist resetFreq, works out.
	if ( freq > INTERNAL_CLOCK / 2 ){
		thrower ( "DDS frequency out of range. Must be < 250MHz." );
		return 0;
	}
	return (int) round ( ( freq * pow ( 2, 32 ) ) / ( INTERNAL_CLOCK ) );;
}

unsigned DdsCore::getATW ( double amp ){
	// input is a percentage (/100) of the maximum amplitude
	if ( amp > 100 ){
		thrower ( "DDS amplitude out of range, should be < 100 %" );
	}
	return (unsigned) round ( amp * ( pow ( 2, 10 ) - 1 ) / 100.0 );
}

int DdsCore::get32bitATW ( double amp ){
	// why do we need this and the getATW function?
	//SIGNED
	if ( abs ( amp ) > 100 ){
		thrower ( "ERROR: DDS amplitude out of range, should be < 100%." );
	}
	return (int) round ( amp * ( pow ( 2, 32 ) - pow ( 2, 22 ) ) / 100.0 );
}

void DdsCore::longUpdate ( ){
	writeDDS ( 0, 0x1d, 0, 0, 0, 1 );
	writeDDS ( 1, 0x1d, 0, 0, 0, 1 );
}

void DdsCore::lockPLLs ( ){
	writeDDS ( 0, 1, 0, 0b10101000, 0, 0 );
	writeDDS ( 1, 1, 0, 0b10101000, 0, 0 );
	longUpdate ( );
	Sleep ( 100 ); //This delay is critical, need to give the PLL time to lock.
}

void DdsCore::channelSelect ( UINT8 device, UINT8 channel ){
	// ??? this is hard-coded...
	UINT8 CW = 0b11100000;
	//CW |= 1 << (channel + 4);
	writeDDS ( device, 0, 0, 0, 0, CW );
}


void DdsCore::writeAmpMultiplier ( UINT8 device ){
	// Necessary to turn on amplitude multiplier.
	UINT8 byte1 = 1 << 4;
	writeDDS ( device, 6, 0, 0, 0, 0 );
	writeDDS ( device, 6, 0, 0, byte1, 0 );
}

void DdsCore::writeResetFreq ( UINT8 device, UINT8 channel, double freq ){
	UINT16 address = RESET_FREQ_ADDR_OFFSET + 4 * device + 3 - channel;
	writeDDS ( WBWRITE_ARRAY, address, intTo4Bytes ( getFTW ( freq ) ) );
}

void DdsCore::writeResetAmp ( UINT8 device, UINT8 channel, double amp ){
	UINT16 address = RESET_AMP_ADDR_OFFSET + 4 * device + 3 - channel;
	writeDDS ( WBWRITE_ARRAY, address, intTo4Bytes ( getATW ( amp ) << 22 ) );
}

void DdsCore::writeRampReps ( UINT8 index, UINT16 reps ){
	UINT16 address = REPS_ADDRESS_OFFSET + index;
	UINT8 byte4 = reps & 0x000000ffUL;
	UINT8 byte3 = ( reps & 0x0000ff00UL ) >> 8;
	writeDDS ( WBWRITE_ARRAY, address, 0, 0, byte3, byte4 );
}

UINT16 DdsCore::getRepsFromTime ( double time ){
	// 125 kHz update rate
	// units of time is milliseconds
	double deltaTime = 8e-3; // 8 usec
	double maxTime = ( ( std::numeric_limits<UINT16>::max )( ) - 0.5 ) * deltaTime;
	unsigned __int64 repNum = ( time / deltaTime ) + 0.5;
	unsigned __int64 maxTimeLong = ( std::numeric_limits<UINT16>::max )( );
	// I'm moderately confused as to why I have to also exclude the max time itself here, this should be all 1's in 
	// binary.
	if ( repNum >= maxTimeLong ){
		thrower ( "time too long for ramp! Max time is " + str ( maxTime ) + " ms" );
	}
	return repNum;
}


void DdsCore::clearDdsRampMemory ( ){
	for ( auto rampI : range ( 255 ) ){
		writeRampReps ( rampI, 0 );
		for ( auto board : range ( 2 ) ){
			for ( auto chan : range ( 4 ) ){
				writeRampDeltaFreq ( board, chan, rampI, 0 );
				writeRampDeltaAmp ( board, chan, rampI, 0 );
			}
		}
	}
}


void DdsCore::writeRampDeltaFreq ( UINT8 device, UINT8 channel, UINT8 index, double deltafreq ){
	UINT16 address = RAMP_FREQ_ADDR_OFFSET + RAMP_CHANNEL_ADDR_SPACING * ( 4 * device + 3 - channel ) + index;
	writeDDS ( WBWRITE_ARRAY, address, intTo4Bytes ( getFTW ( deltafreq ) ) );
}

void DdsCore::writeRampDeltaAmp ( UINT8 device, UINT8 channel, UINT8 index, double deltaamp ){
	UINT16 address = RAMP_AMP_ADDR_OFFSET + RAMP_CHANNEL_ADDR_SPACING * ( 4 * device + 3 - channel ) + index;
	writeDDS ( WBWRITE_ARRAY, address, intTo4Bytes ( get32bitATW ( deltaamp ) ) );
}

std::array<UINT8, 4> DdsCore::intTo4Bytes ( int i_ ){
	// order here is hiword to loword, i.e. if you wrote out the int in 
	// This looks strange. FTW is an insigned int but it's being bitwise compared to an unsigned long. (the UL suffix)
	std::array<UINT8, 4> res;
	res[ 3 ] = ( i_ & 0x000000ffUL );
	res[ 2 ] = ( ( i_ & 0x0000ff00UL ) >> 8 );
	res[ 1 ] = ( ( i_ & 0x00ff0000UL ) >> 16 );
	res[ 0 ] = ( ( i_ & 0xff000000UL ) >> 24 );
	return res;
}

// a low level writebtn wrapper around the ftFlume writebtn
void DdsCore::writeDDS ( UINT8 DEVICE, UINT16 ADDRESS, std::array<UINT8, 4> data ){
	writeDDS ( DEVICE, ADDRESS, data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}


void DdsCore::writeDDS ( UINT8 DEVICE, UINT16 ADDRESS, UINT8 dat1, UINT8 dat2, UINT8 dat3, UINT8 dat4 ){
	if ( connType == ddsConnectionType::type::Async ){
		// None of these should be possible based on the data types of these args. 
		if ( DEVICE > 255 || ADDRESS > 65535 || dat1 > 255 || dat2 > 255 || dat3 > 255 || dat4 > 255 ){
			thrower ( "Error: DDS write out of range." );
		}
		UINT8 ADDRESS_LO = ADDRESS & 0x00ffUL;
		UINT8 ADDRESS_HI = ( ADDRESS & 0xff00UL ) >> 8;
		std::vector<unsigned char> input = { unsigned char ( WBWRITE + DEVICE ), ADDRESS_HI, ADDRESS_LO, dat1, dat2, dat3, dat4 };
		ftFlume.write ( input, MSGLENGTH );
	}
	else{
		// could probably take out other options entierly...
		thrower ( "Incorrect connection type, should be ASYNC" );
	}
}


ddsExpSettings DdsCore::getSettingsFromConfig ( ConfigStream& file ){
	ddsExpSettings settings;
	unsigned numRamps = 0;
	if (file.ver < Version ("5.1")) {
		settings.control = true;
	}
	else {
		file >> settings.control;
	}

	file >> numRamps;
	settings.ramplist = std::vector<ddsIndvRampListInfo> ( numRamps );

	for ( auto& ramp : settings.ramplist){
		file >> ramp.index >> ramp.channel >> ramp.freq1.expressionStr >> ramp.amp1.expressionStr
			 >> ramp.freq2.expressionStr >> ramp.amp2.expressionStr >> ramp.rampTime.expressionStr;
		file.get ( );
	}	
	return settings;
}


void DdsCore::writeRampListToConfig ( std::vector<ddsIndvRampListInfo> list, ConfigStream& file ){
	file << "/*Ramp List Size:*/ " << list.size ( );
	unsigned count = 0;
	for ( auto& ramp : list )
	{
		file << "\n/*Ramp List Element #" + str (++count) + "*/"
			 << "\n/*Index:*/\t\t" << ramp.index
			 << "\n/*Channel:*/\t" << ramp.channel
			 << "\n/*Freq1:*/\t\t" << ramp.freq1
			 << "\n/*Amp1:*/\t\t" << ramp.amp1
			 << "\n/*Freq2:*/\t\t" << ramp.freq2
			 << "\n/*Amp2:*/\t\t" << ramp.amp2
			 << "\n/*Ramp-Time:*/\t" << ramp.rampTime;
	}
}

void DdsCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
}

void DdsCore::manualLoadExpRampList (std::vector< ddsIndvRampListInfo> ramplist) {
	expRampList = ramplist;
}

void DdsCore::loadExpSettings (ConfigStream& stream){
	ddsExpSettings settings;
	ConfigSystem::stdGetFromConfig (stream, *this, settings);
	expRampList = settings.ramplist;
	experimentActive = settings.control;
}
