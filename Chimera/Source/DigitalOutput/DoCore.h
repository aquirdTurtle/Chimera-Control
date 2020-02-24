#pragma once
#include "GeneralFlumes/ftdiFlume.h"
#include "GeneralFlumes/WinSerialFlume.h"
#include "GeneralFlumes/ftdiStructures.h"
#include "GeneralObjects/ExpWrap.h"
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

		void convertToFinalFtdiFormat (UINT variation, UINT seqNum);
		void fillFtdiDataBuffer (std::vector<unsigned char>& dataBuffer, UINT offset, UINT count, ftdiPt pt);
		void ftdi_connectasync (const char devSerial[]);
		void ftdi_disconnect ();
		DWORD ftdi_write (UINT seqNum, UINT variation, bool loadSkipf);
		DWORD ftdi_trigger ();
		std::array< std::array<bool, 16>, 4 > getFinalSnapshot ();
		std::string getDoSystemInfo ();
		void standardNonExperimentStartDoSequence (DoSnapshot initSnap);
		void restructureCommands ();

		void initializeDataObjects (UINT seqNum, UINT variationNum);
		void ttlOn (UINT row, UINT column, timeType time, UINT seqNum);
		void ttlOff (UINT row, UINT column, timeType time, UINT seqNum);
		void ttlOnDirect (UINT row, UINT column, double timev, UINT variation, UINT seqInc);
		void ttlOffDirect (UINT row, UINT column, double timev, UINT variation, UINT seqInc);
		void sizeDataStructures (UINT sequenceLength, UINT variations);
		void interpretKey (std::vector<std::vector<parameterType>>& params);
		void fillPlotData (UINT variation, std::vector<std::vector<pPlotDataVec>> ttlData);
		std::string getTtlSequenceMessage (UINT variation, UINT seqNum);
		std::vector<std::vector<double>> getFinalTimes ();
		UINT countTriggers (std::pair<DoRows::which, UINT> which, UINT variation, UINT seqNum);
		DWORD ftdi_ForceOutput (DoRows::which row, int number, int state, std::array<std::array<bool, 16>, 4> status);
		// returns -1 if not a name.
		bool isValidTTLName (std::string name);
		int getNameIdentifier (std::string name, DoRows::which& row, UINT& number);
		void organizeTtlCommands (UINT variation, UINT seqNum, DoSnapshot initSnap = { 0,0 });
		bool getFtFlumeSafemode ();
		ULONG getNumberEvents (UINT variation, UINT seqNum);
		double getTotalTime (UINT variation, UINT seqNum);
		double getFtdiTotalTime (UINT variation, UINT seqNum);
		void resetTtlEvents ();
		void FtdiWaitTillFinished (UINT variation, UINT seqNum);
		void wait2 (double time);
		void prepareForce ();
		void findLoadSkipSnapshots (double time, std::vector<parameterType>& variables, UINT variation, UINT seqNum);
		void convertToFtdiSnaps (UINT variation, UINT seqNum);
		ExpWrap<std::vector<DoSnapshot>> getTtlSnapshots ();
		ExpWrap<finBufInfo> getFinalFtdiData ();
		ExpWrap<std::array<ftdiPt, 2048>> getFtdiSnaps ();
		void handleTtlScriptCommand (std::string command, timeType time, std::string name, Expression pulseLength,
			std::vector<std::pair<UINT, UINT>>& ttlShadeLocations,
			std::vector<parameterType>& vars, UINT seqNum, std::string scope);
		void handleTtlScriptCommand (std::string command, timeType time, std::string name,
			std::vector<std::pair<UINT, UINT>>& ttlShadeLocations,
			std::vector<parameterType>& vars, UINT seqNum, std::string scope);
		void setNames (Matrix<std::string> namesIn);
		Matrix<std::string> getAllNames ();
	private:
		ftdiConnectionOption connectType;
		ftdiFlume ftFlume;
		WinSerialFlume winSerial;
		/// stuff for felix's do
		const UINT NUMPOINTS = 2048;
		const unsigned int TIMEOFFS = unsigned int (0x0800);
		const unsigned int BANKAOFFS = unsigned int (0x1000);
		const unsigned int BANKBOFFS = unsigned int (0x1800);
		const unsigned int WBWRITE = (unsigned char)161;

		std::vector<std::vector<DoCommandForm>> ttlCommandFormList;
		ExpWrap<std::vector<DoCommand>> ttlCommandList;
		ExpWrap<std::vector<DoSnapshot>> ttlSnapshots, loadSkipTtlSnapshots;
		ExpWrap<std::array<ftdiPt, 2048>> ftdiSnaps, ftdiSnaps_loadSkip;
		ExpWrap<finBufInfo> finFtdiBuffers, finFtdiBuffers_loadSkip;
		Matrix<std::string> names;
};