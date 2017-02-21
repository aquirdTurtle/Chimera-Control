#include "stdafx.h"
#include "SocketWrapper.h"

void SocketWrapper::initialize()
{
	if (this->server.Create(10010, SOCK_STREAM) == 0)
	{
		thrower("Socket initialization error!");
	}
}

void SocketWrapper::connect()
{
	if (this->server.Listen() == 0)
	{
		thrower("Socket listen error!");
	}
	if (this->server.Accept(client) == 0)
	{
		int a = GetLastError();
		thrower("Socket Accept error! Code: " + std::to_string(a));
	}
}


void SocketWrapper::send(std::string message)
{
	if (this->client.Send(message.c_str(), message.size()) == SOCKET_ERROR)
	{
		thrower("ERROR: socket send error");
	}
}


std::string SocketWrapper::recieve()
{
	//std::string message;
	char message[256];
	if (this->client.Receive(message, 256) == SOCKET_ERROR)
	{
		thrower("ERROR: Socket recieve error!");
		return "";
	}
	return message;
}


void SocketWrapper::close()
{
	// no return value for Close()... for some reason T.T
	this->client.Close();
	return;
}

