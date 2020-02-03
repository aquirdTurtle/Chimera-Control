// created by Mark O. Brown
#pragma once
#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include "DigitalOutput/DioStructures.h"
#include "ParameterSystem/Expression.h"
#include "Plotting/PlotCtrl.h"
#include "GeneralFlumes/ftdiStructures.h"
#include "GeneralFlumes/ftdiFlume.h"
#include "GeneralFlumes/WinSerialFlume.h"
#include "ConfigurationSystems/Version.h"
#include "viewpointFlume.h"
#include "DigitalOutput/DigitalOutput.h"
#include "GeneralObjects/ExpWrap.h"
#include <array>
#include <sstream>
#include <unordered_map>


/**/
class AuxiliaryWindow;


/*
 * The DioSystem class is based on the DIO64.bas module in the original VB6 code. It does use the dio64_32.dll system, 
 * but we always refer to these controls as the TTLs, so I call it that.
 * This should be converted to a constant class at some point.
 */
class DioSystem
{
	public:
		DioSystem ( bool ftSafemode, bool serialSafemode, bool viewpointSafemode );
		~DioSystem();
		/// Felix's Dio handling. Much originally in a class called "RC028".
		void ftdi_disconnect( );
		void ftdi_connectasync( const char devSerial[] );
		void fillFtdiDataBuffer( std::vector<unsigned char>& dataBuffer, UINT offset, UINT count, ftdiPt pt );
		DWORD ftdi_write( UINT seqNum, UINT variation, bool loadSkipf );
		DWORD ftdi_trigger( );
		void standardNonExperimentStartDioSequence( );
		// an "alias template". effectively a local "using std::vector;" declaration. makes these declarations much more
		// readable. I very rarely use things like this.
		//template<class T> using vec = std::vector<T>;

		/// config handling
		void initializeDataObjects( UINT seqNum, UINT cmdNum );
		void handleNewConfig( std::ofstream& saveFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, Version ver );
		void initialize(POINT& startLocation, cToolTips& toolTips, AuxiliaryWindow* master, int& id );
		double getTotalTime(UINT variation, UINT seqNum );
		void checkFinalFormatTimes( UINT variation, UINT seqNum );
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		std::string getTtlSequenceMessage(UINT variation, UINT seqNum );
		void zeroBoard();
		void ftdiZeroBoard ( );
		void handleTTLPress(int id);
		void checkNotTooManyTimes( UINT variation, UINT seqNum );
		void handleHoldPress();
		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		std::string getSystemInfo();
		std::string getDioSystemInfo();
		std::array< std::array<bool, 16>, 4 > getFinalSnapshot();
		void setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status);

		ULONG getNumberEvents(UINT variation, UINT seqNum );

		void rearrange(UINT width, UINT height, fontMap fonts);

		void ttlOn(UINT row, UINT column, timeType time, UINT seqNum );
		void ttlOnDirect( UINT row, UINT column, double time, UINT variation, UINT seqNum, UINT totalVariations );
		void ttlOff(UINT row, UINT column, timeType time, UINT seqNum );
		void ttlOffDirect( UINT row, UINT column, double time, UINT variation, UINT seqNum, UINT totalVariations );
		void forceTtl( DioRows::which row, int number, bool state);

		std::pair<UINT, UINT> getTtlBoardSize();

		void setName( DioRows::which row, UINT number, std::string name, cToolTips& toolTips, AuxiliaryWindow* master);
		std::string getName ( DioRows::which row, UINT number );
		std::array<std::array<std::string, 16>, 4> DioSystem::getAllNames();
		// returns -1 if not a name.
		int getNameIdentifier(std::string name, DioRows::which& row, UINT& number);
		bool getTtlStatus ( DioRows::which row, int number );
		void handleTtlScriptCommand( std::string command, timeType time, std::string name,
									 std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
									 std::vector<parameterType>& vars, UINT seqNum, std::string scope );
		void handleTtlScriptCommand( std::string command, timeType time, std::string name,
									 Expression pulseLength, std::vector<std::pair<UINT, UINT>>& ttlShadeLocations,
									 std::vector<parameterType>& vars, UINT seqNum, std::string scope );
		void organizeTtlCommands(UINT variation, UINT seqNum );
		void convertToFinalViewpointFormat(UINT variation, UINT seqNum );
		void convertToFtdiSnaps( UINT variation, UINT seqNum );
		void convertToFinalFtdiFormat( UINT variation, UINT seqNum );
		DWORD ftdi_ForceOutput( DioRows::which row, int number, int state );
		void sizeDataStructures( UINT sequenceLength, UINT variations );
		void writeTtlData( UINT variation, UINT seqNum, bool loadSkip );
		void startBoard();
		void stopBoard();
		double getClockStatus();
		void wait(double time);
		void waitTillFinished( UINT variation, UINT seqNum, bool skipOption );
		void shadeTTLs(std::vector<std::pair<UINT, UINT>>);
		void unshadeTtls();
		bool isValidTTLName(std::string name);
		void resetTtlEvents();
		void prepareForce();
		void updateDefaultTtl( DioRows::which row, UINT column, bool state);
		UINT countTriggers( std::pair<DioRows::which, UINT> which, UINT variation, UINT seqNum );
		bool getDefaultTtl( DioRows::which row, UINT column);
		void findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, UINT variation, UINT seqNum );
		void fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> ttlData );
		std::pair<USHORT, USHORT> calcDoubleShortTime( double time );
		std::vector<std::vector<double>> getFinalTimes( );
		std::array< std::array<bool, 16>, 4 > getCurrentStatus( );
		void updatePush( DioRows::which row, UINT col );
		double getFtdiTotalTime( UINT variation, UINT seqNum );
		bool getViewpointSafemode ( );
		bool getFtFlumeSafemode();
		std::string testTTL();
		allDigitalOutputs& getDigitalOutputs();
		void interpretKey ( std::vector<std::vector<parameterType>>& params );
		void wait2(double time);
		void FtdiWaitTillFinished( UINT variation, UINT seqNum );
		ExpWrap<std::vector<DioSnapshot>> getTtlSnapshots ( );
		ExpWrap<std::vector<WORD>> getFinalViewpointData ( );
		ExpWrap<std::array<ftdiPt, 2048>> getFtdiSnaps ( );
		ExpWrap<finBufInfo> getFinalFtdiData ( );
		void restructureCommands ( );
	private:
		ViewpointFlume vp_flume;
		/// stuff for felix's dio
		ftdiConnectionOption connectType;
		const UINT NUMPOINTS = 2048;
		const unsigned int TIMEOFFS = unsigned int(0x0800);
		const unsigned int BANKAOFFS = unsigned int(0x1000);
		const unsigned int BANKBOFFS = unsigned int(0x1800);
		const unsigned int WBWRITE = (unsigned char)161;
		ftdiFlume ftFlume;
		// note: it doesn't look like felix's / Adam's programming actually facilitates the serial mode programming
		// because this handle never gets initialized anywhere in the code. Probably not hard to set up, although I 
		// think that the ftdi stuff is a superset of the normal serial communications so probably no reason to do 
		// this? I don't know, there might be speed considerations.
		WinSerialFlume winSerial;
		/// other.
		void handleInvert( );
		// one control for each TTL
		Control<CStatic> ttlTitle;
		Control<CleanPush> ttlHold;
		Control<CleanPush> zeroTtls;
		std::array< Control<CStatic>, 16 > ttlNumberLabels;
		std::array< Control<CStatic>, 4 > ttlRowLabels;
		allDigitalOutputs outputs;
		// tells whether the hold button is down or not.
		bool holdStatus; 
		std::vector<std::vector<DioCommandForm>> ttlCommandFormList;
		ExpWrap<std::vector<DioCommand>> ttlCommandList;
		ExpWrap<std::vector<DioSnapshot>> ttlSnapshots, loadSkipTtlSnapshots;
		ExpWrap<std::vector<std::array<WORD, 6>>> formattedTtlSnapshots, loadSkipFormattedTtlSnapshots;
		// this is just a flattened version of the above snapshots. This is what gets directly sent to the dio64 card.
		ExpWrap<std::vector<WORD>> finalFormatViewpointData, loadSkipFinalFormatViewpointData;
		// ftdi equivalents...
		ExpWrap<std::array<ftdiPt, 2048>> ftdiSnaps;
		ExpWrap<finBufInfo> finFtdiBuffers;
		ExpWrap<std::array<ftdiPt, 2048>> ftdiSnaps_loadSkip;
		ExpWrap<finBufInfo> finFtdiBuffers_loadSkip;
};
