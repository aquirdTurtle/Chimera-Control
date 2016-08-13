#pragma once
#include <string>
#include "afxsock.h"

class SocketWrapper
{
	public:
		int initialize();
		bool connect();
		bool send(std::string message);
		std::string recieve();
		bool close();
	private:
		// socket constants
		int identifier;
		CAsyncSocket server;
		CAsyncSocket client;
};