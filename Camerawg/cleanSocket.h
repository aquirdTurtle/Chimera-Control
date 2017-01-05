#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afxsock.h>

/*
 * This function shuts down the socket, closes it, and calls WSACleanup(). it returns true unless the shutdown fails, 
 * in which case it returns false.
 */
void cleanSocket(CSocket* mySocket, bool socketActive);

