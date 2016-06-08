
#include "stdafx.h"
#include "constants.h"
#include "externals.h"
#include "appendText.h"
#include <winsock2.h>
#include <ws2tcpip.h>

/*
 * This function shuts down the socket, closes it, and calls WSACleanup(). it returns true unless the shutdown fails, in which case it returns false.
 */
bool cleanSocket(SOCKET mySocket, bool socketActive)
{
	// if false, the socket was never opened.
	if (eConnectToMaster == true)
	{
		if (socketActive)
		{
			int iResult = 0;
			// shut it down
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				iResult = shutdown(mySocket, SD_SEND);
			}
			// check for error
			if (iResult == SOCKET_ERROR) 
			{
				appendText("ERROR: Socket shutdown failed with error code:" + std::to_string(WSAGetLastError()), IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
				closesocket(mySocket);
				WSACleanup();
				return false;
			}
		}
		// cleanup
		closesocket(mySocket);
		WSACleanup();
	}
	return true;
}
