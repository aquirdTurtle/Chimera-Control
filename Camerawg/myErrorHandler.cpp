#include "stdafx.h"

#include "myErrorHandler.h"

#include "constants.h"
#include "externals.h"

#include "myAgilent.h"
#include "niFgen.h"

#include <string>
#include <iostream>
#include "postMyString.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <afxsock.h>

/*
 * This is a function for handling errors that MY functions return. Returns true if error is detected, false otherwise.
 */
bool myErrorHandler(int errorCode, CSocket* socketToClose, niawgPair<std::vector<std::fstream>>& scriptFiles, 
					 bool sockActive, bool connected, Communicator* comm)
{
	if (errorCode != 0)
	{
		if (eDontActuallyGenerate == false)
		{
			// a check later checks the value and handles it specially in this case.
			eCurrentScript = "continue";
		}
		try
		{
			// if false, the socket was never opened.
			if (sockActive)
			{
				int result = 0;
				// shut it down
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					result = socketToClose->ShutDown( SD_SEND );
					//iResult = shutdown(mySocket, SD_SEND);
				}
				// check for error
				if (result == SOCKET_ERROR)
				{
					socketToClose->Close();
					WSACleanup();
					thrower( "ERROR: Socket shutdown failed!" );
				}
			}
			// cleanup
			socketToClose->Close();
			WSACleanup();
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
