#include "stdafx.h"
#include "SocketWrapper.h"

void SocketWrapper::initialize(POINT pos, CWnd* master, int& id)
{
	if (server.Create(10010, SOCK_STREAM) == 0)
	{
		thrower("Socket initialization error!");
	}
	button.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 25 };
	button.ID = id++;
	button.Create("Connect to NIAWG?", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, button.sPos, master, button.ID);
}

void SocketWrapper::rearrange(UINT width, UINT height, fontMap fonts)
{
	button.rearrange(width, height, fonts);
}

void SocketWrapper::connect()
{
	if (button.GetCheck())
	{
		errBox("Listening to socket...");
		if (server.Listen() == 0)
		{
			int a = GetLastError();
			thrower("Socket listen error! Code: " + str(a));
		}
		if (server.Accept(client) == 0)
		{
			int a = GetLastError();
			thrower("Socket Accept error! Code: " + std::to_string(a));
		}
		errBox("Connected!");
	}
}


void SocketWrapper::send(std::string message)
{
	if (client.Send(message.c_str(), message.size()) == SOCKET_ERROR)
	{
		thrower("ERROR: socket send error");
	}
}


std::string SocketWrapper::recieve()
{
	//std::string message;
	char message[256];
	if (client.Receive(message, 256) == SOCKET_ERROR)
	{
		thrower("ERROR: Socket recieve error!");
		return "";
	}
	return message;
}


void SocketWrapper::close()
{
	// no return value for Close()... for some reason T.T
	client.Close();
}

