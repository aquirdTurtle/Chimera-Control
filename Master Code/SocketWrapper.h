#pragma once
#include <string>
#include "afxsock.h"

class SocketWrapper
{
	public:
		void initialize();
		void connect();
		void send(std::string message);
		std::string recieve();
		void close();
	private:
		// socket constants
		int identifier;
		CAsyncSocket server;
		CAsyncSocket client;
};