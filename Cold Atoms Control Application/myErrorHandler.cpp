#include "stdafx.h"

#include "myErrorHandler.h"

#include "appendText.h"
#include "cleanSocket.h"

#include "constants.h"
#include "externals.h"

#include "myAgilent.h"
#include "niFgen.h"
//#include "agilentDefault.h"

#include <string>
#include <iostream>

/*
 * This is a function for handling errors that MY functions return. Returns true if error is detected, false otherwise.
 */
bool myErrorHandler(int errorCode, std::string errMsg, SOCKET& socketToClose, std::vector<std::fstream>& verticalFiles, std::vector<std::fstream>& horizontalFiles, bool aborting, ViStatus error, 
					ViSession& mySession, bool scriptIsWritten, char scriptNameToDelete[260], bool sockActive, bool deleteScriptOpt, bool connected, bool isThreaded)
{
	if (errorCode != 0)
	{
		if (eDontActuallyGenerate == false)
		{
			// a check later checks the value and handles it specially in this case.
			eCurrentScript = "continue";
		}
		if (aborting == false)
		{
			// if this is being called in a threaded function, can't set window text directly.
			if (!isThreaded)
			{
				// Append error message to the system error handle.
				appendText(errMsg, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
				SetWindowText(eColoredStatusEdit, "EXITED WITH ERROR! Passively Outputting Default Waveform");
				// Set the status color.
				eGenStatusColor = "R";
				// Redraw the windows.
				// T.T
				//RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		else if (aborting == true)
		{
			if (!isThreaded) 
			{
				appendText("Aborted Generation!\r\n", IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
			}
		}
		// Call Clean Socket.
		cleanSocket(socketToClose, sockActive, connected);
		// turn the agilent to the default setting.
		myAgilent::agilentDefault();
		// close files.
		for (int sequenceInc = 0; sequenceInc < verticalFiles.size(); sequenceInc++)
		{
			if (verticalFiles[sequenceInc].is_open())
			{
				verticalFiles[sequenceInc].close();
			}
		}
		for (int sequenceInc = 0; sequenceInc < horizontalFiles.size(); sequenceInc++)
		{
			if (horizontalFiles[sequenceInc].is_open())
			{
				horizontalFiles[sequenceInc].close();
			}
		}
		// Leave.
		return true;
	}
	return false;
}
