
#include "stdafx.h"
#include "constants.h"
#include "externals.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "postMyString.h"
#include <afxsock.h>

/*
 * This function shuts down the socket, closes it, and calls WSACleanup(). it returns true unless the shutdown fails, in which case it returns false.
 */
void cleanSocket(CSocket* mySocket, bool socketActive)
{
	// if false, the socket was never opened.

	if (socketActive)
	{
		int result = 0;
		// shut it down
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			result = mySocket->ShutDown(SD_SEND);
			//iResult = shutdown(mySocket, SD_SEND);
		}
		// check for error
		if (result == SOCKET_ERROR) 
		{
			mySocket->Close();
			//closesocket(mySocket);
			WSACleanup();
			thrower("ERROR: Socket shutdown failed!");
		}
	}
	// cleanup
	mySocket->Close();
	//closesocket(mySocket);
	WSACleanup();
}
