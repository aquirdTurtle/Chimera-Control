
#include "stdafx.h"
#include "constants.h"
#include "externals.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "postMyString.h"
/*
 * This function shuts down the socket, closes it, and calls WSACleanup(). it returns true unless the shutdown fails, in which case it returns false.
 */
void cleanSocket(SOCKET mySocket, bool socketActive)
{
	// if false, the socket was never opened.

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
			closesocket(mySocket);
			WSACleanup();
			thrower("ERROR: Socket shutdown failed!");
			return;
		}
	}
	// cleanup
	closesocket(mySocket);
	WSACleanup();
	return;
}
