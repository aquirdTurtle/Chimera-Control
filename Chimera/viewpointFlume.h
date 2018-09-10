#pragma once

#include "afxwin.h"
#include "viewpointStructures.h"

class ViewpointFlume
{
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
	public:
		ViewpointFlume( bool safemode_ );
		std::string getErrorMessage( int errorCode );
		// 
		bool getSafemodeSetting ( );
		/// The following section holds the dio functions that I actually use!
		void dioOpenResource( char* resourceName, WORD board, WORD baseio );
		void dioOpen( WORD board, WORD baseio );
		void dioMode( WORD board, WORD mode );

		void dioLoad( WORD board, char* rbfFile, int inputHint, int outputHint );
		void dioClose( WORD board );
		void dioInStart( WORD board, DWORD ticks, WORD& mask, WORD maskLength, WORD flags, WORD clkControl,
						 WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
						 double& scanRate );
		void dioInStatus( WORD board, DWORD& scansAvail, DIO64STAT& status );
		void dioInRead( WORD board, WORD& buffer, DWORD scansToRead, DIO64STAT& status );
		void dioInStop( WORD board );
		void dioForceOutput( WORD board, WORD* buffer, DWORD mask );
		void dioOutGetInput( WORD board, WORD& buffer );
		void dioOutConfig( WORD board, DWORD ticks, WORD* mask, WORD maskLength, WORD flags, WORD clkControl,
						   WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
						   DWORD reps, WORD ntrans, double& scanRate );
		void dioOutStart( WORD board );
		void dioOutStatus( WORD board, DWORD& scansAvail, DIO64STAT& status );
		void dioOutWrite( WORD board, WORD* buffer, DWORD bufsize, DIO64STAT& status );
		void dioOutStop( WORD board );
		void dioSetAttr( WORD board, DWORD attrID, DWORD value );
		void dioGetAttr( WORD board, DWORD attrID, DWORD& value );
	private:
		const bool safemode;
		/// The following functions (all of the ones that start with "raw") ARE NOT MEANT TO BE DIRECTLY USED (at least
		/// in my code. They are the raw functions I'm importing from viewpoints libraries without any bells or whistles.)
		/// In this code, please use my wrapped functions (above) which wraps the functions into slightly shorter calls 
		/// & standardized & built in error handling.

		/// NOT SUGGESTED FOR DIRECT USE! //////////////////////////////////////////////////////////////////////////
		typedef int( __cdecl* DIO64_OpenResource )(char resourceName[], uint16_t board, uint16_t baseio);
		DIO64_OpenResource raw_DIO64_OpenResource;
		// before win7+, used to use dio64_open instead of dio64_openresource
		typedef int( __cdecl* DIO64_Open )(WORD board, WORD baseio);
		DIO64_Open raw_DIO64_Open;
		typedef int( __cdecl* DIO64_Mode )(WORD board, WORD mode);
		DIO64_Mode raw_DIO64_Mode;
		typedef int( __cdecl* DIO64_Load )(WORD board, char *rbfFile, int intputHint, int outputHint);
		DIO64_Load raw_DIO64_Load;
		typedef int( __cdecl* DIO64_Close )(WORD board);
		DIO64_Close raw_DIO64_Close;
		typedef int( __cdecl* DIO64_In_Start )(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags,
												WORD clkControl, WORD startType, WORD startSource, WORD stopType,
												WORD stopSource, DWORD AIControl, double *scanRate);
		DIO64_In_Start raw_DIO64_In_Start;
		typedef int( __cdecl* DIO64_In_Status )(WORD board, DWORD *scansAvail, DIO64STAT *status);
		DIO64_In_Status raw_DIO64_In_Status;
		typedef int( __cdecl* DIO64_In_Read )(WORD board, WORD *buffer, DWORD scansToRead, DIO64STAT *status);
		DIO64_In_Read raw_DIO64_In_Read;
		typedef int( __cdecl* DIO64_In_Stop )(WORD board);
		DIO64_In_Stop raw_DIO64_In_Stop;
		typedef int( __cdecl* DIO64_Out_ForceOutput )(WORD board, WORD *buffer, DWORD mask);
		DIO64_Out_ForceOutput raw_DIO64_Out_ForceOutput;
		typedef int( __cdecl* DIO64_Out_GetInput )(WORD board, WORD *buffer);
		DIO64_Out_GetInput raw_DIO64_Out_GetInput;
		typedef int( __cdecl* DIO64_Out_Config )(WORD board, DWORD ticks, WORD *mask, WORD maskLength, WORD flags,
												  WORD clkControl, WORD startType, WORD startSource, WORD stopType,
												  WORD stopSource, DWORD AIControl, DWORD reps, WORD ntrans, double *scanRate);
		DIO64_Out_Config raw_DIO64_Out_Config;
		typedef int( __cdecl* DIO64_Out_Start )(WORD board);
		DIO64_Out_Start raw_DIO64_Out_Start;
		typedef int( __cdecl* DIO64_Out_Status )(WORD board, DWORD *scansAvail, DIO64STAT *status);
		DIO64_Out_Status raw_DIO64_Out_Status;


		typedef int( __cdecl* DIO64_Out_Write )(WORD board, WORD *buffer, DWORD bufsize, DIO64STAT *status);

		DIO64_Out_Write raw_DIO64_Out_Write;


		typedef int( __cdecl* DIO64_Out_Stop )(WORD board);
		DIO64_Out_Stop raw_DIO64_Out_Stop;
		typedef int( __cdecl* DIO64_SetAttr )(WORD board, DWORD attrID, DWORD value);
		DIO64_SetAttr raw_DIO64_SetAttr;
		typedef int( __cdecl* DIO64_GetAttr )(WORD board, DWORD attrID, DWORD *value);
		DIO64_GetAttr raw_DIO64_GetAttr;

};