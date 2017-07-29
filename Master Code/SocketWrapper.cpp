#include "stdafx.h"
#include "SocketWrapper.h"


void SocketWrapper::sendVars(std::vector<variable> vars, key varKey)
{
	
	for (auto var : varKey)
	{
		if (var.second.second == true)
		{
			std::string varString;
			varString = var.first;
			for (auto val : var.second.first)
			{
				varString += " " + str(val);
			}
			varString += " done!";
			send(varString);
		}
	}
}

void SocketWrapper::initializeWinsock()
{
	WSADATA wsaData;
	int result;

	struct addrinfo *addResult = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[512];
	int recvbuflen = 512;

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		thrower("WSAStartup failed with error: " + str(result));
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	result = getaddrinfo(NULL, "27015", &hints, &addResult);
	if (result != 0)
	{
		WSACleanup();
		thrower("getaddrinfo failed with error: " + str(result));
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(addResult->ai_family, addResult->ai_socktype, addResult->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(addResult);
		WSACleanup();
		thrower("socket failed with error:" + str(WSAGetLastError()));
	}

	result = bind(ListenSocket, addResult->ai_addr, (int)addResult->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		freeaddrinfo(addResult);
		closesocket(ListenSocket);
		WSACleanup();
		thrower("bind failed with error:" + str(WSAGetLastError()));
	}

	freeaddrinfo(addResult);
}

void SocketWrapper::initialize(POINT pos, CWnd* master, int& id)
{
	button.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 25 };
	button.Create("Connect to NIAWG?", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, button.sPos, master, id++);
}

void SocketWrapper::rearrange(UINT width, UINT height, fontMap fonts)
{
	button.rearrange(width, height, fonts);
}

bool SocketWrapper::connectSelected()
{
	return button.GetCheck();
}

void SocketWrapper::connect()
{
	// Setup the TCP listening socket
	int result;
	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) 
	{
		
		closesocket(ListenSocket);
		WSACleanup();
		thrower("listen failed with error:" + str(WSAGetLastError()));
	}

	// Accept a client socket.
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) 
	{
		closesocket(ListenSocket);
		WSACleanup();
		thrower("accept failed with error:" + str(WSAGetLastError()));
	}

	// No longer need server socket
	closesocket(ListenSocket);
}


void SocketWrapper::send(std::string message)
{
	char recvbuf[512];
	int result = 0;
	result = ::send(ClientSocket, message.c_str(), 512, 0);
	if (result == SOCKET_ERROR) 
	{	
		closesocket(ClientSocket);
		WSACleanup();
		thrower("send failed with error:" + str(WSAGetLastError()));
	}
	//printf("Bytes sent: %d\n", iSendResult);
}

std::string SocketWrapper::recieve()
{
	char message[512];
	int result = recv(ClientSocket, message, 512, 0);
	// can check result for bytes recieved.
	return message;
}

void SocketWrapper::close()
{
	// no return value for Close()... for some reason T.T
	closesocket(ClientSocket);
}

