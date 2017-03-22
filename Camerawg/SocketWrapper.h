#pragma once
#include <string>
#include "afxsock.h"

class SocketWrapper
{
	public:
		// general usage
		void initialize();
		void connect();
		void send(std::string message);
		std::string recieve();
		void close();
		bool isConnected();
		// specific
		int getRepetitions(int scalingFactor);

	private:
		// socket constants
		bool connectedStatus;
		int identifier;
		CSocket server;
		CSocket client;
};