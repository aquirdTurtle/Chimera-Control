#include "stdafx.h"

#include "myErrorHandler.h"

#include "cleanSocket.h"

#include "constants.h"
#include "externals.h"

#include "myAgilent.h"
#include "niFgen.h"

#include <string>
#include <iostream>
#include "postMyString.h"

/*
 * This is a function for handling errors that MY functions return. Returns true if error is detected, false otherwise.
 */
bool myErrorHandler(int errorCode, std::string errMsg, CSocket* socketToClose, niawgPair<std::vector<std::fstream>>& scriptFiles, 
					 bool aborting, bool scriptIsWritten, char scriptNameToDelete[260], bool sockActive, bool deleteScriptOpt, 
					 bool connected, Communicator* comm, bool isThreaded)
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
				colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
				comm->sendError(errMsg);
			}
		}
		else if (aborting == true)
		{
			if (!isThreaded) 
			{
				comm->sendStatus("Aborted Generation!\r\n");
			}
		}
		// Call Clean Socket.
		try
		{
			if (connected)
			{
				cleanSocket(socketToClose, sockActive);
			}
		}
		catch (std::runtime_error& err)
		{
			comm->sendFatalError(err.what());
		}
		// turn the agilent to the default setting.
		myAgilent::agilentDefault();
		// close files.
		for (auto axis : AXES)
		{
			for (auto& file : scriptFiles[axis])
			{
				if (file.is_open())
				{
					file.close();
				}
			}
		}
		return true;
	}
	return false;
}
