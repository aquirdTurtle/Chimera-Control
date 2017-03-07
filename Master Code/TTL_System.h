#pragma once
#include <array>
#include "Control.h"
#include <sstream>
#include <unordered_map>
#include "KeyHandler.h"

/*
 * Parts of changing ttls:
 *  Ways to change TTLs:
 * In Script:
 * - Set array, then chnge
 * In main Window:
 * - Force TTL
 *		- 
 *
 *
 */
class MasterWindow;

// this struct keeps variable names.
struct TTL_CommandForm
{
	// the hardware location of this line
	std::pair<unsigned int, unsigned int> line;
	// the time to make the change
	timeType time;
	// the value to set it to. 
	bool value;
};

// no variables in this version. It's calculated each variation based on corresponding ComandForm structs.
struct TTL_Command
{
	// the hardware location of this line
	std::pair<unsigned int, unsigned int> line;
	// the time to make the change
	double time;
	// the value to set it to. 
	bool value;
};

// an object constructed for having all info the ttls for a single time
struct TTL_Snapshot
{
	// the time of the snapshot
	double time;
	// all values at this time.
	std::array< std::array<bool, 16>, 4 > ttlStatus;
};


typedef struct _DIO64STAT 
{
	USHORT pktsize;
	USHORT portCount;
	USHORT writePtr;
	USHORT readPtr;
	USHORT time[2];
	ULONG fifoSize;
	USHORT fifo0;
	ULONG  ticks;
	USHORT flags;
	USHORT clkControl;
	USHORT startControl;
	USHORT stopControl;
	ULONG AIControl;
	USHORT AICurrent;
	USHORT startTime[2];
	USHORT stopTime[2];
	USHORT user[4];
} DIO64STAT;

/*
]- The TtlSystem class is based on the DIO64.bas module in the original VB6 code. It does use the dio64_32.dll system, but we always refer to these 
]- controls as the TTLs, so I call it that.
]- This should be converted to a singleton class at some point.
*/
class TtlSystem
{
	public:
		TtlSystem();
		~TtlSystem();
		void initialize(POINT& startLocation, std::unordered_map<HWND, std::string>& masterText, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id);
		double getTotalTime();
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		std::string getTtlSequenceMessage();
		void zeroBoard();
		void handleTTLPress(UINT id);
		void handleHoldPress();
		HBRUSH TtlSystem::handleColorMessage(CWnd* window, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rGBs, CDC* cDC);
		std::string getSystemInfo();

		void ttlOn(unsigned int row, unsigned int column, timeType time);
		void ttlOnDirect( unsigned int row, unsigned int column, double time );
		void ttlOff(unsigned int row, unsigned int column, timeType time);
		void ttlOffDirect( unsigned int row, unsigned int column, double time );
		void forceTtl(int row, int number, int state);

		void abort();
		void setName(unsigned int row, unsigned int number, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		std::string getName(unsigned int row, unsigned int number);
		std::array<std::array<std::string, 16>, 4> TtlSystem::getAllNames();
		// returns -1 if not a name.get
		int TtlSystem::getNameIdentifier(std::string name, unsigned int& row, unsigned int& number);
		bool getTTL_Status(int row, int number);
		std::string getErrorMessage(int errorCode);
		void handleTTL_ScriptCommand( std::string command, timeType time, std::string name,
									  std::vector<std::pair<unsigned int, unsigned int>>& ttlShadeLocations );
		void handleTTL_ScriptCommand( std::string command, timeType time, std::string name,
									  std::string pulseLength,
									  std::vector<std::pair<unsigned int, unsigned int>>& ttlShadeLocations );
		void interpretKey(key variationKey, unsigned int variationNum);
		void analyzeCommandList();
		void convertToFinalFormat();
		void writeData();
		void startBoard();
		void stopBoard();
		double getClockStatus();
		void wait(double time);
		void waitTillFinished();

		//int TtlSystem::getNameIdentifier(std::string name, unsigned int& row, unsigned int& number);
		void shadeTTLs(std::vector<std::pair<unsigned int, unsigned int>>);
		void unshadeTTLs();
		bool isValidTTLName(std::string name);
		void resetTTLEvents();

	private:

		/* ***********************************************************************************************************
		 * All of the functions (and a bit of redundancy here) might be a bit confusing. In short, there are 3 
		 * "versions" of all of the functions directly below. There's the (1) function in the DLL (start with DIO64_...),
		 * which I import into (2) functions with the raw_ prefix which I COULD use directly, but which I then wrap 
		 * into (3) functions that I actually use (start with "dio..."). Note that for many other libraries in my code
		 * I use a header & statically import the dll which allows me to just use functions pretty direclty. For 
		 * whatever reason I was having trouble getting this implemented in this case (and apparently Adam/Debbie did
		 * too in the visual basic code?) and so I manually import all of the functions in the constructor for this 
		 * ttl class.
		 * ***********************************************************************************************************
		 */
		
		/// The following section holds the dio functions that I actually use!
		void dioOpen( WORD board, WORD baseio );
		void dioMode( WORD board, WORD mode );

		void dioLoad(WORD board, char* rbfFile, int inputHint, int outputHint);
		void dioClose(WORD board);
		void dioInStart( WORD board, DWORD ticks, WORD& mask, WORD maskLength, WORD flags, WORD clkControl, 
						 WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl, 
						 double& scanRate );
		void dioInStatus( WORD board, DWORD& scansAvail, DIO64STAT& status );
		void dioInRead( WORD board, WORD& buffer, DWORD scansToRead, DIO64STAT& status );
		void dioInStop( WORD board );
		void dioForceOutput( WORD board, WORD* buffer, DWORD mask );
		void dioOutGetInput( WORD board, WORD& buffer );
		void dioOutConfig(WORD board, DWORD ticks, WORD* mask, WORD maskLength, WORD flags, WORD clkControl, 
						   WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl, 
						   DWORD reps, WORD ntrans, double& scanRate);
		void dioOutStart( WORD board );
		void dioOutStatus( WORD board, DWORD& scansAvail, DIO64STAT& status );
		void dioOutWrite(WORD board, WORD* buffer, DWORD bufsize, DIO64STAT& status);
		void dioOutStop( WORD board );
		void dioSetAttr( WORD board, DWORD attrID, DWORD value );
		void dioGetAttr( WORD board, DWORD& attrID, DWORD& value );

		// one control for each TTL
		Control<CStatic> ttlTitle;
		Control<CButton> ttlHold;
		Control<CButton> zeroTtls;
		std::array< std::array< Control<CButton>, 16 >, 4 > ttlPushControls;
		std::array< Control<CStatic>, 16 > ttlNumberLabels;
		std::array< Control<CStatic>, 4 > ttlRowLabels;
		std::array< std::array<bool, 16>, 4 > ttlStatus;
		std::array< std::array<bool, 16>, 4 > ttlShadeStatus;
		std::array< std::array<bool, 16>, 4 > ttlHoldStatus;
		std::array< std::array<std::string, 16 >, 4> ttlNames;
		bool holdStatus;
		std::vector<TTL_CommandForm> ttlCommandFormList;
		std::vector<TTL_Command> individualTTL_CommandList;
		std::vector<TTL_Snapshot> ttlSnapshots;
		std::vector<std::array<WORD, 6>> finalFormattedCommandForDIO;

		/// The following functions (all of the ones that start with "raw") ARE NOT MEANT TO BE DIRECTLY USED (at least
		/// in my code. They are the raw functions I'm importing from viewpoints libraries without any bells or whistles.)
		/// In this code, please use my wrapped functions (above) which wraps the functions into slightly shorter calls 
		/// & standardized & built in error handling.

		/// NOT SUGGESTED FOR DIRECT USE! //////////////////////////////////////////////////////////////////////////
		typedef int(CALLBACK* DIO64_Open)(WORD board, WORD baseio);
		DIO64_Open raw_DIO64_Open;
		typedef int(CALLBACK* DIO64_Mode)(WORD board, WORD mode);
		DIO64_Mode raw_DIO64_Mode;
		typedef int(CALLBACK* DIO64_Load)(WORD board, char *rbfFile, int intputHint, int outputHint );
		DIO64_Load raw_DIO64_Load;
		typedef int(CALLBACK* DIO64_Close)(WORD board);
		DIO64_Close raw_DIO64_Close;
		typedef int(CALLBACK* DIO64_In_Start)(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags,
											   WORD clkControl, WORD startType, WORD startSource, WORD stopType,
											   WORD stopSource, DWORD AIControl, double *scanRate);
		DIO64_In_Start raw_DIO64_In_Start;
		typedef int(CALLBACK* DIO64_In_Status)(WORD board, DWORD *scansAvail, DIO64STAT *status);
		DIO64_In_Status raw_DIO64_In_Status;
		typedef int(CALLBACK* DIO64_In_Read)(WORD board, WORD *buffer, DWORD scansToRead, DIO64STAT *status);
		DIO64_In_Read raw_DIO64_In_Read;
		typedef int(CALLBACK* DIO64_In_Stop)(WORD board);
		DIO64_In_Stop raw_DIO64_In_Stop;
		typedef int(CALLBACK* DIO64_Out_ForceOutput)( WORD board, WORD *buffer, DWORD mask );
		DIO64_Out_ForceOutput raw_DIO64_Out_ForceOutput;
		typedef int(CALLBACK* DIO64_Out_GetInput)( WORD board, WORD *buffer );
		DIO64_Out_GetInput raw_DIO64_Out_GetInput;
		typedef int(CALLBACK* DIO64_Out_Config)(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags, 
												 WORD clkControl, WORD startType, WORD startSource, WORD stopType, 
												 WORD stopSource, DWORD AIControl, DWORD reps, WORD ntrans, double *scanRate);
		DIO64_Out_Config raw_DIO64_Out_Config;
		typedef int(CALLBACK* DIO64_Out_Start)( WORD board );
		DIO64_Out_Start raw_DIO64_Out_Start;
		typedef int(CALLBACK* DIO64_Out_Status)(WORD board,	DWORD *scansAvail, DIO64STAT *status);
		DIO64_Out_Status raw_DIO64_Out_Status;
		typedef int(CALLBACK* DIO64_Out_Write)(WORD board, WORD *buffer, DWORD bufsize, DIO64STAT *status);
		DIO64_Out_Write raw_DIO64_Out_Write;
		typedef int(CALLBACK* DIO64_Out_Stop)(WORD board);
		DIO64_Out_Stop raw_DIO64_Out_Stop;
		typedef int(CALLBACK* DIO64_SetAttr)(WORD board, DWORD attrID, DWORD value);
		DIO64_SetAttr raw_DIO64_SetAttr;
		typedef int(CALLBACK* DIO64_GetAttr)(WORD board, DWORD *attrID, DWORD *value);
		DIO64_GetAttr raw_DIO64_GetAttr;
		/// END NOT SUGGESTED FOR DIRECT USE AREA! ////////////////////////////////////////////////////////////////////


};

