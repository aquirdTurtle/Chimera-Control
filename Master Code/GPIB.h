#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <vector>
#include "KeyHandler.h"
#include "TektronicsControl.h"

class Gpib
{
	public:
		Gpib::Gpib();
		void gpibWrite( int deviceID, std::string msg );
		std::string gpibRead( int deviceID );
		std::string gpibQuery( int deviceID, std::string query );
		std::string queryIdentity( int deviceAddress );

		int gpibIbdev( int pad );
		void gpibSend( int address, std::string message );
		
		// as of now the raman stuff isn't complicated enough to warrant it's own control. might change in the future.
		void programTektronixs(tektronicsInfo info);
		void interpretKeyForTektronics( tektronicsInfo& raman,  key variationKey, UINT variableNumber, 
									    std::vector<variable>& vars);

		std::string getErrMessage( long errCode );
};
