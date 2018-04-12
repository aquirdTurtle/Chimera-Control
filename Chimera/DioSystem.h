#pragma once
#include "Control.h"
#include "myButton.h"
#include "miscellaneousCommonFunctions.h"
#include "DioStructures.h"
#include "Expression.h"
#include "PlotCtrl.h"
#include "ftdiStructures.h"
#include "ftdiFlume.h"
#include "WinSerialFlume.h"
#include "viewpointFlume.h"
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
	    DioSystem( bool ftSafemode, bool serialSafemode );
		/// Felixes Dio handling. Much originally in a class called "RC028".
		void ftdi_disconnect( );
		void ftdi_connectasync( const char devSerial[] );
		void fillFtdiDataBuffer( std::vector<unsigned char>& dataBuffer, UINT offset, UINT count, ftdiPt pt );
		DWORD ftdi_write( UINT seqNum, UINT variation, bool loadSkipf );
		DWORD ftdi_trigger( );
		
		// an "alias template". effectively a local "using std::vector;" declaration. makes these declarations much more
		// readable. I very rarely use things like this.
		template<class T> using vec = std::vector<T>;

		/// config handling
		void handleNewConfig( std::ofstream& saveFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor );
		void initialize(POINT& startLocation, cToolTips& toolTips, AuxiliaryWindow* master, int& id, rgbMap rgbs );
		double getTotalTime(UINT variation, UINT seqNum );
		void checkFinalFormatTimes( UINT variation, UINT seqNum );
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		std::string getTtlSequenceMessage(UINT variation, UINT seqNum );
		void zeroBoard();
		void handleTTLPress(int id);
		void checkNotTooManyTimes( UINT variation, UINT seqNum );
		void handleHoldPress();
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		std::string getSystemInfo();
		std::array< std::array<bool, 16>, 4 > getFinalSnapshot();
		void setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status);

		ULONG countDacTriggers(UINT variation, UINT seqNum );
		ULONG getNumberEvents(UINT variation, UINT seqNum );

		void rearrange(UINT width, UINT height, fontMap fonts);

		void ttlOn(UINT row, UINT column, timeType time, UINT seqNum );
		void ttlOnDirect( UINT row, UINT column, double time, UINT variation, UINT seqNum );
		void ttlOff(UINT row, UINT column, timeType time, UINT seqNum );
		void ttlOffDirect( UINT row, UINT column, double time, UINT variation, UINT seqNum );
		void forceTtl(int row, int number, int state);

		std::pair<UINT, UINT> getTtlBoardSize();

		void setName(UINT row, UINT number, std::string name, cToolTips& toolTips, AuxiliaryWindow* master);
		std::string getName(UINT row, UINT number);
		std::array<std::array<std::string, 16>, 4> DioSystem::getAllNames();
		// returns -1 if not a name.
		int getNameIdentifier(std::string name, UINT& row, UINT& number);
		bool getTtlStatus(int row, int number);
		void handleTtlScriptCommand( std::string command, timeType time, std::string name,
									 std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
									 std::vector<variableType>& vars, UINT seqNum );
		void handleTtlScriptCommand( std::string command, timeType time, std::string name,
									 Expression pulseLength, std::vector<std::pair<UINT, UINT>>& ttlShadeLocations,
									 std::vector<variableType>& vars, UINT seqNum );
		void interpretKey( std::vector<std::vector<variableType>>& variables );
		void organizeTtlCommands(UINT variation, UINT seqNum );
		void convertToFinalViewpointFormat(UINT variation, UINT seqNum );
		void convertToFtdiSnaps( UINT variation, UINT seqNum );
		void convertToFinalFtdiFormat( UINT variation, UINT seqNum );
		DWORD ftdi_ForceOutput( int row, int number, int state );
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
		void initTtlObjs( UINT totalSequenceNumber );
		void prepareForce();
		void updateDefaultTtl(UINT row, UINT column, bool state);
		UINT countTriggers( UINT row, UINT number, UINT variation, UINT seqNum );
		bool getDefaultTtl(UINT row, UINT column);
		void findLoadSkipSnapshots( double time, std::vector<variableType>& variables, UINT variation, UINT seqNum );
		void fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> ttlData );
		std::pair<USHORT, USHORT> calcDoubleShortTime( double time );
		std::vector<std::vector<double>> getFinalTimes( );
		std::array< std::array<bool, 16>, 4 > getCurrentStatus( );
		void updatePush( UINT row, UINT col );
		vec<vec<vec<DioSnapshot>>> getSnapshots( );
		vec<vec<std::array<ftdiPt, 2048>>> getFtdiSnaps( );
		vec<vec<vec<WORD>>> getFinalViewpointData( );
		vec<vec<finBufInfo>> getFinalFtdiData( );
		double getFtdiTotalTime( UINT variation, UINT seqNum );
	private:
		ViewpointFlume vp_flume;
		/// stuff for felix's dio
		ftdiConnectionOption connectType;
		const UINT NUMPOINTS = 2048;
		const unsigned char TIMEOFFS = 0x0800;
		const unsigned char BANKAOFFS = 0x1000;
		const unsigned char BANKBOFFS = 0x1800;
		const unsigned char WBWRITE = 161;
		ftdiFlume ftFlume;
		// note: it doesn't look like felix's / Adam's programming actually facilitates the serial mode programming
		// because this handle never gets initialized anywhere int he code. Probably not hard to set up, although I 
		// think that the ftdi stuff is a superset of the normal serial communications so probably no reason to do 
		// this? I don't know, there might be speed considerations.
		WinSerialFlume winSerial;
		/// other.
		void handleInvert( );
		// one control for each TTL
		Control<CStatic> ttlTitle;
		Control<CleanButton> ttlHold;
		Control<CleanButton> zeroTtls;
		std::array< std::array< Control<CButton>, 16 >, 4 > ttlPushControls;
		std::array< Control<CStatic>, 16 > ttlNumberLabels;
		std::array< Control<CStatic>, 4 > ttlRowLabels;
		std::array< std::array<bool, 16>, 4 > ttlStatus;
		std::array< std::array<bool, 16>, 4 > ttlShadeStatus;
		std::array< std::array<bool, 16>, 4 > ttlHoldStatus;
		std::array< std::array<std::string, 16 >, 4> ttlNames;
		// tells whether the hold button is down or not.
		bool holdStatus;
		// Each element of first vector is for each variation.
		vec<vec<DioCommandForm>> ttlCommandFormList;
		vec<vec<vec<DioCommand>>> ttlCommandList;
		vec<vec<vec<DioSnapshot>>> ttlSnapshots, loadSkipTtlSnapshots;
		vec<vec<vec<std::array<WORD, 6>>>> formattedTtlSnapshots, loadSkipFormattedTtlSnapshots;
		// this is just a flattened version of the above snapshots. This is what gets directly sent to the dio64 card.
		vec<vec<vec<WORD>>> finalFormatViewpointData, loadSkipFinalFormatViewpointData;
		// ftdiSnaps[seqNum][variationNum][snapshotNum]
		vec<vec<std::array<ftdiPt, 2048>>> ftdiSnaps;
		vec<vec<finBufInfo>> finFtdiBuffers;
		vec<vec<std::array<ftdiPt, 2048>>> ftdiSnaps_loadSkip;
		vec<vec<finBufInfo>> finFtdiBuffers_loadSkip;

		std::array<std::array<bool, 16>, 4> defaultTtlState;
};

