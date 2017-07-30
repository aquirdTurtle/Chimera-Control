#pragma once
#include <afxsock.h>
// Returns true if error is detected, false otherwise.
bool myErrorHandler(int errorCode, CSocket* socketToClose, niawgPair<std::vector<std::fstream>>& scriptFiles, 
					bool sockActive, bool connected, Communicator* comm);
