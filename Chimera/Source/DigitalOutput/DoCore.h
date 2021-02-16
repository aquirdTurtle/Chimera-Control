#pragma once
#include "GeneralFlumes/ftdiFlume.h"
#include "GeneralFlumes/WinSerialFlume.h"
#include "GeneralFlumes/ftdiStructures.h"
#include "GeneralObjects/ExpWrap.h"
#include <GeneralObjects/Matrix.h>
#include "Plotting/PlotCtrl.h"

#include "DoStructures.h"

/*
	(Stands for DigitalOutput Core)
*/
class DoCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		DoCore& operator=(const DoCore&) = delete;
		DoCore (const DoCore&) = delete;
		DoCore (bool ftSafemode, bool serialSafemode);
		~DoCore ();

		void convertToFinalFtdiFormat (unsigned variation);
		void fillFtdiDataBuffer (std::vector<unsigned char>& dataBuffer, unsigned offset, unsigned count, ftdiPt pt);
		void ftdi_connectasync (const char devSerial[]);
		void ftdi_disconnect ();
		unsigned long ftdi_write (unsigned variation, bool loadSkipf);
		unsigned long ftdi_trigger ();
		std::array< std::array<bool, 16>, 4 > getFinalSnapshot ();
		std::string getDoSystemInfo ();
		void standardNonExperimentStartDoSequence (DoSnapshot initSnap);
		void restructureCommands ();

		void initializeDataObjects (unsigned variationNum);
		void ttlOn (unsigned row, unsigned column, timeType time);
		void ttlOff (unsigned row, unsigned column, timeType time);
		void ttlOnDirect (unsigned row, unsigned column, double timev, unsigned variation);
		void ttlOffDirect (unsigned row, unsigned column, double timev, unsigned variation);
		void sizeDataStructures (unsigned variations);
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker);
		std::vector<std::vector<plotDataVec>> getPlotData (unsigned variation );
		std::string getTtlSequenceMessage (unsigned variation);
		std::vector<double> getFinalTimes ();
		unsigned countTriggers (std::pair<DoRows::which, unsigned> which, unsigned variation);
		unsigned long ftdi_ForceOutput (DoRows::which row, int number, int state, std::array<std::array<bool, 16>, 4> status);
		// returns -1 if not a name.
		bool isValidTTLName (std::string name);
		int getNameIdentifier (std::string name, DoRows::which& row, unsigned& number);
		void organizeTtlCommands (unsigned variation, DoSnapshot initSnap = { 0,0 });
		bool getFtFlumeSafemode ();
		unsigned long getNumberEvents (unsigned variation);
		double getTotalTime (unsigned variation);
		double getFtdiTotalTime (unsigned variation);
		void resetTtlEvents ();
		void FtdiWaitTillFinished (unsigned variation);
		void wait2 (double time);
		void prepareForce ();
		void findLoadSkipSnapshots (double time, std::vector<parameterType>& variables, unsigned variation);
		void convertToFtdiSnaps (unsigned variation);
		ExpWrap<std::vector<DoSnapshot>> getTtlSnapshots ();
		ExpWrap<finBufInfo> getFinalFtdiData ();
		ExpWrap<std::array<ftdiPt, 2048>> getFtdiSnaps ();
		void handleTtlScriptCommand (std::string command, timeType time, std::string name, Expression pulseLength,
			std::vector<parameterType>& vars, std::string scope);
		void handleTtlScriptCommand (std::string command, timeType time, std::string name,
			std::vector<parameterType>& vars, std::string scope);
		void setNames (Matrix<std::string> namesIn);
		Matrix<std::string> getAllNames ();
		void standardExperimentPrep (unsigned variationInc, double currLoadSkipTime, std::vector<parameterType>& expParams);
	private:
		const unsigned DIO_BUFFERSIZESER = 100;
		const unsigned DIO_BUFFERSIZEASYNC = 2048;
		const unsigned DIO_MSGLENGTH = 7;
		const unsigned DIO_WRITESPERDATAPT = 3;

		ftdiConnectionOption connectType;
		ftdiFlume ftFlume;
		WinSerialFlume winSerial;
		/// stuff for felix's do
		const unsigned NUMPOINTS = 2048;
		const unsigned int TIMEOFFS = unsigned int (0x0800);
		const unsigned int BANKAOFFS = unsigned int (0x1000);
		const unsigned int BANKBOFFS = unsigned int (0x1800);
		const unsigned int WBWRITE = (unsigned char)161;

		std::vector<DoCommandForm> ttlCommandFormList;
		ExpWrap<std::vector<DoCommand>> ttlCommandList;
		ExpWrap<std::vector<DoSnapshot>> ttlSnapshots, loadSkipTtlSnapshots;
		ExpWrap<std::array<ftdiPt, 2048>> ftdiSnaps, ftdiSnaps_loadSkip;
		ExpWrap<finBufInfo> finFtdiBuffers, finFtdiBuffers_loadSkip;
		Matrix<std::string> names;
};