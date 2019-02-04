#include "stdafx.h"
#include "viewpointFlume.h"
#include "Thrower.h"

ViewpointFlume::ViewpointFlume( bool safemode_ ) : safemode( safemode_ )
{
	if ( safemode )
	{
		return;
	} 
	/// load modules
	// this first module is required for the second module which I actually load functions from.
	HMODULE dio = LoadLibrary( "DIO64_Visa32.dll" );
	if ( !dio )
	{
		int err = GetLastError( );
		errBox( "Failed to load dio64_32.dll! This is the viewpoint dio dll. Windows Error Code: " + str( err ) );
	}
	// initialize function pointers. This only requires the DLLs to be loaded (which requires them to be present on the machine...) 
	// so it's not in a safemode block.
	raw_DIO64_OpenResource = (DIO64_OpenResource)GetProcAddress( dio, "DIO64_OpenResource" );
	raw_DIO64_Open = (DIO64_Open)GetProcAddress( dio, "DIO64_Open" );
	raw_DIO64_Load = (DIO64_Load)GetProcAddress( dio, "DIO64_Load" );
	raw_DIO64_Close = (DIO64_Close)GetProcAddress( dio, "DIO64_Close" );
	raw_DIO64_Mode = (DIO64_Mode)GetProcAddress( dio, "DIO64_Mode" );
	raw_DIO64_GetAttr = (DIO64_GetAttr)GetProcAddress( dio, "DIO64_GetAttr" );
	raw_DIO64_SetAttr = (DIO64_SetAttr)GetProcAddress( dio, "DIO64_SetAttr" );

	raw_DIO64_In_Read = (DIO64_In_Read)GetProcAddress( dio, "DIO64_In_Read" );
	raw_DIO64_In_Start = (DIO64_In_Start)GetProcAddress( dio, "DIO64_In_Start" );
	raw_DIO64_In_Read = (DIO64_In_Read)GetProcAddress( dio, "DIO64_In_Read" );
	raw_DIO64_In_Status = (DIO64_In_Status)GetProcAddress( dio, "DIO64_In_Status" );
	raw_DIO64_In_Stop = (DIO64_In_Stop)GetProcAddress( dio, "DIO64_In_Stop" );

	raw_DIO64_Out_Config = (DIO64_Out_Config)GetProcAddress( dio, "DIO64_Out_Config" );
	raw_DIO64_Out_ForceOutput = (DIO64_Out_ForceOutput)GetProcAddress( dio, "DIO64_Out_ForceOutput" );
	raw_DIO64_Out_GetInput = (DIO64_Out_GetInput)GetProcAddress( dio, "DIO64_Out_GetInput" );
	raw_DIO64_Out_Start = (DIO64_Out_Start)GetProcAddress( dio, "DIO64_Out_Start" );
	raw_DIO64_Out_Status = (DIO64_Out_Status)GetProcAddress( dio, "DIO64_Out_Status" );
	raw_DIO64_Out_Stop = (DIO64_Out_Stop)GetProcAddress( dio, "DIO64_Out_Stop" );

	raw_DIO64_Out_Write = (DIO64_Out_Write)GetProcAddress( dio, "DIO64_Out_Write" );
	// Open and Load DIO64
	try
	{
		char* filename = "C:\\DIO64Visa\\DIO64Visa_Release Beta 2\\DIO64.CAT";
		char* resourceName = "PXI18::11::INSTR";
		WORD temp[4] = { -1, -1, -1, -1 };
		double tempd = 10000000;
		dioOpenResource( resourceName, 0, 0 );
		dioLoad( 0, filename, 0, 4 );
		dioOutConfig( 0, 0, temp, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, tempd );
		// done initializing.
	}
	catch ( Error& exception )
	{
		errBox( exception.trace( ) );
	}
}


bool ViewpointFlume::getSafemodeSetting ( )
{
	return safemode;
}


void ViewpointFlume::dioOpen( WORD board, WORD baseio )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Open( board, baseio );
		if ( result )
		{
			thrower ( "dioOpen failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioMode( WORD board, WORD mode )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Mode( board, mode );
		if ( result )
		{
			thrower ( "dioMode failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioLoad( WORD board, char *rbfFile, int inputHint, int outputHint )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Load( board, rbfFile, inputHint, outputHint );
		if ( result )
		{
			thrower ( "dioLoad failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioClose( WORD board )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Close( board );
		if ( result )
		{
			thrower ( "dioClose failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioInStart( WORD board, DWORD ticks, WORD& mask, WORD maskLength, WORD flags, WORD clkControl,
							WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
							double& scanRate )
{
	if ( !safemode )
	{
		int result = raw_DIO64_In_Start( board, ticks, &mask, maskLength, flags, clkControl, startType, startSource,
										 stopType, stopSource, AIControl, &scanRate );
		if ( result )
		{
			thrower ( "dioInStart failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioInStatus( WORD board, DWORD& scansAvail, DIO64STAT& status )
{
	if ( !safemode )
	{
		int result = raw_DIO64_In_Status( board, &scansAvail, &status );
		if ( result )
		{
			thrower ( "dioInStatus failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioInRead( WORD board, WORD& buffer, DWORD scansToRead, DIO64STAT& status )
{
	if ( !safemode )
	{
		int result = raw_DIO64_In_Read( board, &buffer, scansToRead, &status );
		if ( result )
		{
			thrower ( "dioInRead failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioInStop( WORD board )
{
	if ( !safemode )
	{
		int result = raw_DIO64_In_Stop( board );
		if ( result )
		{
			thrower ( "dioInStop failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioForceOutput( WORD board, WORD* buffer, DWORD mask )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Out_ForceOutput( board, buffer, mask );
		if ( result )
		{
			thrower ( "dioForceOutput failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioOutGetInput( WORD board, WORD& buffer )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Out_GetInput( board, &buffer );
		if ( result )
		{
			thrower ( "dioOutGetInput failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioOutConfig( WORD board, DWORD ticks, WORD* mask, WORD maskLength, WORD flags, WORD clkControl,
							  WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
							  DWORD reps, WORD ntrans, double& scanRate )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Out_Config( board, ticks, mask, maskLength, flags, clkControl,
										   startType, startSource, stopType, stopSource, AIControl,
										   reps, ntrans, &scanRate );
		if ( result )
		{
			thrower ( "dioOutConfig failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioOutStart( WORD board )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Out_Start( board );
		if ( result )
		{
			thrower ( "dioOutStart failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioOutStatus( WORD board, DWORD& scansAvail, DIO64STAT& status )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Out_Status( board, &scansAvail, &status );
		if ( result )
		{
			thrower ( "dioOutStatus failed! : (" + str( result ) + "): " + getErrorMessage( result ) + "\r\n" );
		}
	}
}


void ViewpointFlume::dioOutWrite( WORD board, WORD* buffer, DWORD bufsize, DIO64STAT& status )
{
	/*
	IMPORTANT! the buffer size is the number of snapshots, not the number of words in the buffer! very
	counter-intuitive. Boo.
	*/
	if ( !safemode )
	{
		int result = raw_DIO64_Out_Write( board, buffer, bufsize, &status );
		if ( result )
		{
			thrower ( "dioOutWrite failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioOutStop( WORD board )
{
	if ( !safemode )
	{
		int result = raw_DIO64_Out_Stop( board );
		if ( result )
		{
			thrower ( "dioOutStop failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioSetAttr( WORD board, DWORD attrID, DWORD value )
{
	if ( !safemode )
	{
		int result = raw_DIO64_SetAttr( board, attrID, value );
		if ( result )
		{
			thrower ( "dioSetAttr failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioGetAttr( WORD board, DWORD attrID, DWORD& value )
{
	if ( !safemode )
	{
		int result = raw_DIO64_GetAttr( board, attrID, &value );
		if ( result )
		{
			thrower ( "dioGetAttr failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


void ViewpointFlume::dioOpenResource( char* resourceName, WORD board, WORD baseio )
{
	if ( !safemode )
	{
		int result = raw_DIO64_OpenResource( resourceName, board, baseio );
		if ( result )
		{
			thrower ( "dioOpenResource failed! : (" + str( result ) + "): " + getErrorMessage( result ) );
		}
	}
}


std::string ViewpointFlume::getErrorMessage( int errorCode )
{
	switch ( errorCode )
	{
	case -8:
		return "Illegal board number - the board number must be between 0 and 7.";
	case -9:
		return "The requested board number has not been opened.";
	case -10:
		return "The buffers have over or under run.";
	case -12:
		return "Invalid parameter.";
	case -13:
		return "No Driver Interface.";
	case -14:
		return "Board does not have the OCXO option installed.";
	case -15:
		return "Only available on PXI.";
	case -16:
		return "Stop trigger source is invalid.";
	case -17:
		return "Port number conflicts. Check the hints used in DIO64_Load().";
	case -18:
		return "Missing DIO64.cat file.";
	case -19:
		return "Not enough system resources available.";
	case -20:
		return "Invalid DIO64.cat file.";
	case -21:
		return "Required image not found.";
	case -22:
		return "Error programming the FPGA.";
	case -23:
		return "File not found.";
	case -24:
		return "Board error.";
	case -25:
		return "Function call invalid at this time.";
	case -26:
		return "Not enough transitions specified for operation.";
	default:
		return "Unrecognized DIO64 error code!";
	}
}
