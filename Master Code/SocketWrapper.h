#pragma once
#include <string>
#include "afxsock.h"

class SocketWrapper
{
	public:
		void initialize(POINT pos, CWnd* master, int& id);
		void connect();
		void send(std::string message);
		std::string recieve();
		void close();
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		// socket constants
		int identifier;
		CAsyncSocket server;
		CAsyncSocket client;
		Control<CButton> button;
};