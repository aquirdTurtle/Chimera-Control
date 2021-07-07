#pragma once
#include "ATMCD32D.h"
#include <GeneralObjects/Matrix.h>

class AndorFlume{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AndorFlume& operator=(const AndorFlume&) = delete;
		AndorFlume (const AndorFlume&) = delete;

		AndorFlume ( bool safemode_option );
		void initialize ( );
		void setVSSpeed (int index);
		unsigned getNumberVSSpeeds ();
		float getVSSpeed (int index);

		void setHSSpeed (int type, int index);
		unsigned getNumberHSSpeeds ();
		float getHSSpeed (int channel, int type, int index);

		void setVSAmplitude(int index);
		void setBaselineClamp ( int clamp );
		void setBaselineOffset ( int offset );
		void setDMAParameters ( int maxImagesPerDMA, float secondsPerDMA );
		void waitForAcquisition ( );
		int getTemperature ( int& temp );
		std::string getErrorMsg (int errCode);
		void getAdjustedRingExposureTimes ( int size, float* timesArray );
		void setNumberKinetics ( int number );
		void getTemperatureRange ( int& min, int& max );
		void temperatureControlOn ( );
		void temperatureControlOff ( );
		void setTemperature ( int temp );
		void setADChannel ( int channel );
		unsigned checkForNewImages ( );
		void getOldestImage ( Matrix<long>& dataMatrix );
		void setTriggerMode ( int mode );
		void setAcquisitionMode ( int mode );
		void setReadMode ( int mode );
		void setRingExposureTimes ( int sizeOfTimesArray, float* arrayOfTimes );
		void setImage ( int hBin, int vBin, int lBorder, int rBorder, int tBorder, int bBorder );
		void setKineticCycleTime ( float cycleTime );
		void setFrameTransferMode ( int mode );
		void getAcquisitionTimes ( float& exposure, float& accumulation, float& kinetic );
		int queryStatus ( );
		void startAcquisition ( );
		void abortAcquisition ( );
		void setAccumulationCycleTime ( float time );
		void setAccumulationNumber ( int number );
		void getNumberOfPreAmpGains ( int& number );
		void setPreAmpGain ( int index );
		void getPreAmpGain ( int index, float& gain );
		void setOutputAmplifier ( int type );
		void setEmGainSettingsAdvanced ( int state );
		void setEmCcdGain ( int gain );
		void getAcquisitionProgress ( long& seriesNumber );
		void getAcquisitionProgress ( long& accumulationNumber, long& seriesNumber );
		void getCapabilities ( AndorCapabilities& caps );
		void getSerialNumber ( int& num );
		std::string getHeadModel ( );
		void andorErrorChecker ( int errorCode );
	private:
		const bool safemode;
};
