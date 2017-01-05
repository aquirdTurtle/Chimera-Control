#pragma once
#include <afxsock.h>
// Returns true if error is detected, false otherwise.
bool myErrorHandler(int errorCode, std::string errMsg, CSocket* socketToClose, std::vector<std::fstream>& verticalFiles, 
					std::vector<std::fstream>& horizontalFiles, bool aborting, bool scriptIsWritten,
					char scriptNameToDelete[260], bool sockActive, bool deleteScriptOpt, bool connected, Communicator* comm, bool isThreaded = false);
