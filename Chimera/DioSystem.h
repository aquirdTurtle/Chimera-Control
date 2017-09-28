#pragma once
#include "Control.h"
#include "KeyHandler.h"
#include "miscellaneousCommonFunctions.h"
#include "DioStructures.h"
#include "Expression.h"
#include <array>
#include <sstream>
#include <unordered_map>
/**/
class AuxiliaryWindow;

/*
]- The DioSystem class is based on the DIO64.bas module in the original VB6 code. It does use the dio64_32.dll system, but we always refer to these 
]- controls as the TTLs, so I call it that.
]- This should be converted to a constant class at some point.
*/
class DioSystem
{
	public:
	    DioSystem();
		void handleNewConfig( std::ofstream& saveFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, double version);
		void initialize(POINT& startLocation, cToolTips& toolTips, AuxiliaryWindow* master, int& id);
		double getTotalTime(UINT variation );
		void checkFinalFormatTimes( UINT variation );
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		std::string getTtlSequenceMessage(UINT variation );
		void zeroBoard();
		void handleTTLPress(int id);
		void checkNotTooManyTimes( UINT variation );
		void handleHoldPress();
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		std::string getSystemInfo();
		std::array< std::array<bool, 16>, 4 > getFinalSnapshot();
		void setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status);

		ULONG countDacTriggers(UINT variation );
		ULONG getNumberEvents(UINT variation );

		void rearrange(UINT width, UINT height, fontMap fonts);

		void ttlOn(UINT row, UINT column, timeType time);
		void ttlOnDirect( UINT row, UINT column, double time, UINT variation );
		void ttlOff(UINT row, UINT column, timeType time);
		void ttlOffDirect( UINT row, UINT column, double time, UINT variation );
		void forceTtl(int row, int number, int state);

		std::pair<UINT, UINT> getTtlBoardSize();

		void setName(UINT row, UINT number, std::string name, cToolTips& toolTips, AuxiliaryWindow* master);
		std::string getName(UINT row, UINT number);
		std::array<std::array<std::string, 16>, 4> DioSystem::getAllNames();
		// returns -1 if not a name.get
		int DioSystem::getNameIdentifier(std::string name, UINT& row, UINT& number);
		bool getTtlStatus(int row, int number);
		std::string getErrorMessage(int errorCode);
		void handleTtlScriptCommand( std::string command, timeType time, std::string name,
									 std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
									 std::vector<variableType>& vars );
		void handleTtlScriptCommand( std::string command, timeType time, std::string name,
									 Expression pulseLength, std::vector<std::pair<UINT, UINT>>& ttlShadeLocations,
									 std::vector<variableType>& vars );
		void interpretKey(key variationKey, std::vector<variableType>& vars);
		void organizeTtlCommands(UINT variation );
		void convertToFinalFormat(UINT variation );
		void writeTtlData( UINT variation );
		void startBoard();
		void stopBoard();
		double getClockStatus();
		void wait(double time);
		void waitTillFinished(UINT variation );
		void shadeTTLs(std::vector<std::pair<UINT, UINT>>);
		void unshadeTtls();
		bool isValidTTLName(std::string name);

		void resetTtlEvents();
		void prepareForce();
		void updateDefaultTtl(UINT row, UINT column, bool state);

		bool getDefaultTtl(UINT row, UINT column);

	private:

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
		// tells whether the hold button is down or not.
		bool holdStatus;



		std::vector<DioCommandForm> ttlCommandFormList;
		// Each element of first vector is for each variation.
		std::vector<std::vector<DioCommand>> ttlCommandList;
		// Each element of first vector is for each variation.
		std::vector<std::vector<DioSnapshot>> ttlSnapshots;
		// Each element of first vector is for each variation.
		std::vector<std::vector<std::array<WORD, 6>>> formattedTtlSnapshots;
		// this is just a flattened version of the above snapshots. This is what gets directly sent to the dio64 card.
		std::vector<std::vector<WORD>> finalFormatTtlData;

		std::array<std::array<bool, 16>, 4> defaultTtlState;


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
		void handleInvert();
		/// The following section holds the dio functions that I actually use!
		void dioOpenResource(char* resourceName, WORD board, WORD baseio);
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
		void dioGetAttr( WORD board, DWORD attrID, DWORD& value );

		/// The following functions (all of the ones that start with "raw") ARE NOT MEANT TO BE DIRECTLY USED (at least
		/// in my code. They are the raw functions I'm importing from viewpoints libraries without any bells or whistles.)
		/// In this code, please use my wrapped functions (above) which wraps the functions into slightly shorter calls 
		/// & standardized & built in error handling.

		/// NOT SUGGESTED FOR DIRECT USE! //////////////////////////////////////////////////////////////////////////
		typedef int(__cdecl* DIO64_OpenResource)(char resourceName[], uint16_t board, uint16_t baseio);
		DIO64_OpenResource raw_DIO64_OpenResource;
		// before win7+, used to use dio64_open instead of dio64_openresource
		typedef int(__cdecl* DIO64_Open)(WORD board, WORD baseio);
		DIO64_Open raw_DIO64_Open;
		typedef int(__cdecl* DIO64_Mode)(WORD board, WORD mode);
		DIO64_Mode raw_DIO64_Mode;
		typedef int(__cdecl* DIO64_Load)(WORD board, char *rbfFile, int intputHint, int outputHint );
		DIO64_Load raw_DIO64_Load;
		typedef int(__cdecl* DIO64_Close)(WORD board);
		DIO64_Close raw_DIO64_Close;
		typedef int(__cdecl* DIO64_In_Start)(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags,
											   WORD clkControl, WORD startType, WORD startSource, WORD stopType,
											   WORD stopSource, DWORD AIControl, double *scanRate);
		DIO64_In_Start raw_DIO64_In_Start;
		typedef int(__cdecl* DIO64_In_Status)(WORD board, DWORD *scansAvail, DIO64STAT *status);
		DIO64_In_Status raw_DIO64_In_Status;
		typedef int(__cdecl* DIO64_In_Read)(WORD board, WORD *buffer, DWORD scansToRead, DIO64STAT *status);
		DIO64_In_Read raw_DIO64_In_Read;
		typedef int(__cdecl* DIO64_In_Stop)(WORD board);
		DIO64_In_Stop raw_DIO64_In_Stop;
		typedef int(__cdecl* DIO64_Out_ForceOutput)( WORD board, WORD *buffer, DWORD mask );
		DIO64_Out_ForceOutput raw_DIO64_Out_ForceOutput;
		typedef int(__cdecl* DIO64_Out_GetInput)( WORD board, WORD *buffer );
		DIO64_Out_GetInput raw_DIO64_Out_GetInput;
		typedef int(__cdecl* DIO64_Out_Config)(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags,
												 WORD clkControl, WORD startType, WORD startSource, WORD stopType, 
												 WORD stopSource, DWORD AIControl, DWORD reps, WORD ntrans, double *scanRate);
		DIO64_Out_Config raw_DIO64_Out_Config;
		typedef int(__cdecl* DIO64_Out_Start)( WORD board );
		DIO64_Out_Start raw_DIO64_Out_Start;
		typedef int(__cdecl* DIO64_Out_Status)(WORD board,	DWORD *scansAvail, DIO64STAT *status);
		DIO64_Out_Status raw_DIO64_Out_Status;


		typedef int(__cdecl* DIO64_Out_Write)(WORD board, WORD *buffer, DWORD bufsize, DIO64STAT *status);

		DIO64_Out_Write raw_DIO64_Out_Write;


		typedef int(__cdecl* DIO64_Out_Stop)(WORD board);
		DIO64_Out_Stop raw_DIO64_Out_Stop;
		typedef int(__cdecl* DIO64_SetAttr)(WORD board, DWORD attrID, DWORD value);
		DIO64_SetAttr raw_DIO64_SetAttr;
		typedef int(__cdecl* DIO64_GetAttr)(WORD board, DWORD attrID, DWORD *value);
		DIO64_GetAttr raw_DIO64_GetAttr;

		/// END NOT SUGGESTED FOR DIRECT USE AREA! ////////////////////////////////////////////////////////////////////
};

