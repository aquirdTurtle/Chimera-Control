#pragma once
#include <array>
#include "Control.h"
#include <sstream>
#include <unordered_map>

class MasterWindow;

// this struct keeps variable names.
struct TTL_CommandForm
{
	// the hardware location of this line
	std::pair<unsigned int, unsigned int> line;
	// the time to make the change
	std::pair<std::string, long> time;
	// the value to set it to. 
	bool value;
};

// no variables in this version. It's calculated each variation based on corresponding ComandForm structs.
struct TTL_Command
{
	// the hardware location of this line
	std::pair<unsigned int, unsigned int> line;
	// the time to make the change
	long time;
	// the value to set it to. 
	bool value;
};

// an object constructed for having all info the ttls for a single time
struct TTL_Snapshot
{
	// the time of the snapshot
	long time;
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
]- The TTL_System class is based on the DIO64.bas module in the original VB6 code. It does use the dio64_32.dll system, but we always refer to these 
]- controls as the TTLs, so I call it that.
]- This should be converted to a singleton class at some point.
*/
class TTL_System
{
	public:
		TTL_System(int& startID);
		TTL_System();
		~TTL_System();
		void initialize(POINT& startLocation, HWND windowHandle, HINSTANCE programInstance, std::unordered_map<HWND, std::string>& masterText, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		void handleTTLPress(UINT id);
		void handleHoldPress();
		HBRUSH TTL_System::handleColorMessage(CWnd* window, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rGBs, CDC* cDC);
		void ttlOn(unsigned int row, unsigned int column, std::pair<std::string, long> time);
		void ttlOnDirect( unsigned int row, unsigned int column, long time );
		void ttlOff(unsigned int row, unsigned int column, std::pair<std::string, long> time);
		void ttlOffDirect( unsigned int row, unsigned int column, long time );
		void forceTTL(int row, int number, int state);
		
		void setName(unsigned int row, unsigned int number, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		std::string getName(unsigned int row, unsigned int number);
		std::array<std::array<std::string, 16>, 4> TTL_System::getAllNames();
		// returns -1 if not a name.get
		int TTL_System::getNameIdentifier(std::string name, unsigned int& row, unsigned int& number);
		bool getTTL_Status(int row, int number);
		std::string getErrorMessage(int errorCode);
		void TTL_System::handleTTL_ScriptCommand(std::string command, std::pair<std::string, long> time, std::string name, 
			std::vector<std::pair<unsigned int, unsigned int>>& ttlShadeLocations);
		void interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNum);
		void analyzeCommandList();
		void convertToFinalFormat();
		void writeData();
		void startBoard();
		void stopBoard();
		double getClockStatus();
		void wait(double time);
		void waitTillFinished();

		//int TTL_System::getNameIdentifier(std::string name, unsigned int& row, unsigned int& number);
		void shadeTTLs(std::vector<std::pair<unsigned int, unsigned int>>);
		void unshadeTTLs();
		bool isValidTTLName(std::string name);
		void resetTTLEvents();

	private:
		typedef int(CALLBACK* DIO64_Open)(WORD board, WORD baseio);
		DIO64_Open dioOpen;
		typedef int(CALLBACK* DIO64_Mode)(WORD board, WORD mode);
		DIO64_Mode dioMode;
		typedef int(CALLBACK* DIO64_Load)(WORD board, char *rbfFile, int intputHint, int outputHint );
		DIO64_Load dioLoad;
		typedef int(CALLBACK* DIO64_Close)(WORD board);
		DIO64_Close dioClose;
		typedef int(CALLBACK* DIO64_In_Start)(	WORD board,	DWORD ticks, WORD *mask, WORD maskLength, WORD flags, WORD clkControl, WORD startType, 
												WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl, double *scanRate );
		DIO64_In_Start dioInStart;
		typedef int(CALLBACK* DIO64_In_Status)(WORD board, DWORD *scansAvail, DIO64STAT *status);
		DIO64_In_Status dioInStatus;
		typedef int(CALLBACK* DIO64_In_Read)(WORD board, WORD *buffer, DWORD scansToRead, DIO64STAT *status);
		DIO64_In_Read dioInRead;
		typedef int(CALLBACK* DIO64_In_Stop)(WORD board);
		DIO64_In_Stop dioInStop;
		typedef int(CALLBACK* DIO64_Out_ForceOutput)( WORD board, WORD *buffer, DWORD mask );
		DIO64_Out_ForceOutput dioOutForceOutput;
		typedef int(CALLBACK* DIO64_Out_GetInput)( WORD board, WORD *buffer );
		DIO64_Out_GetInput dioOutGetInput;
		typedef int(CALLBACK* DIO64_Out_Config)(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags, WORD clkControl, WORD startType, 
												WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl, DWORD reps, WORD ntrans, double *scanRate);
		DIO64_Out_Config dioOutConfig;
		typedef int(CALLBACK* DIO64_Out_Start)( WORD board );
		DIO64_Out_Start dioOutStart;
		typedef int(CALLBACK* DIO64_Out_Status)(WORD board,	DWORD *scansAvail, DIO64STAT *status);
		DIO64_Out_Status dioOutStatus;
		typedef int(CALLBACK* DIO64_Out_Write)(WORD board, WORD *buffer, DWORD bufsize, DIO64STAT *status);
		DIO64_Out_Write dioOutWrite;
		typedef int(CALLBACK* DIO64_Out_Stop)(WORD board);
		DIO64_Out_Stop dioOutStop;
		typedef int(CALLBACK* DIO64_SetAttr)(WORD board, DWORD attrID, DWORD value);
		DIO64_SetAttr dioSetAttr;
		typedef int(CALLBACK* DIO64_GetAttr)(WORD board, DWORD *attrID, DWORD *value);
		DIO64_GetAttr dioGetAttr;

		// one control for each TTL
		Control<CStatic> ttlTitle;
		Control<CButton> ttlHold;
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
		std::vector<TTL_Snapshot> fullCommandList;
		std::vector<std::array<WORD, 6>> finalFormattedCommandForDIO;
};

