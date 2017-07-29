#pragma once
#include <string>
#include "afxsock.h"
#include "VariableSystem.h"


/*
 * I tried and failed to get MFC sockets to work, so this class is built around C-style sockets. 
 * right now this is blocking and the code will stop on here until something tries to connect.
 * There's no way around this right now. You could build something better by calling select() on the socket,
 * but I haven't figured out how to do this yet, and I'm not going to since I only implemented sockets as a temp.
 * solution in my code.
 */
class SocketWrapper
{
	public:
		void initialize(POINT pos, CWnd* master, int& id);
		void connect();
		void send(std::string message);
		std::string recieve();
		void close();
		void rearrange(UINT width, UINT height, fontMap fonts);
		bool connectSelected();
		void initializeWinsock();
		void sendVars(std::vector<variable> vars, key varKey);
	private:
		// socket constants
		int identifier;
		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;
		// CSocket server;
		// CSocket acceptdSock;
		// CAsyncSocket client;
		Control<CButton> button;
};