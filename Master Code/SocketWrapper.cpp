#include "stdafx.h"
#include "SocketWrapper.h"

int SocketWrapper::initialize()
{
	if (this->server.Create(10010, SOCK_STREAM) == 0)
	{
		errBox("Socket initialization error!");
		return false;
	}
	return true;
}

bool SocketWrapper::connect()
{
	if (this->server.Listen() == 0)
	{
		errBox("Socket listen error!");
		return false;
	}
	if (this->server.Accept(client) == 0)
	{
		int a = GetLastError();
		errBox("Socket Accept error! Code: " + std::to_string(a));
		return false;
	}
	return true;
}

bool SocketWrapper::send(std::string message)
{
	if (this->client.Send(message.c_str(), message.size()) == SOCKET_ERROR)
	{
		errBox("ERROR: socket send error");
		return false;
	}
	return true;
}

std::string SocketWrapper::recieve()
{
	//std::string message;
	char message[256];
	if (this->client.Receive(message, 256) == SOCKET_ERROR)
	{
		errBox("ERROR: Socket recieve error");
		return "";
	}
	return message;
}

bool SocketWrapper::close()
{
	// no return value for Close().
	this->client.Close();
	return true;
}
