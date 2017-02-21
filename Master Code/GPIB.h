#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <vector>
#include "KeyHandler.h"

class Gpib
{
	public:
		Gpib::Gpib();
		// a lot of these are not used currently... not sure if I will need them yet.
		bool copyIBVars();
		
		bool ibwrti();
		bool ibrd();
		bool ibeot();
		bool enableRemote();
		
		//
		void gpibWrite( int deviceID, std::string msg );
		int gpibIbdev( int pad );
		
		void gpibSend( int address, std::string message );
		
		void programRamanFGs(double topFreq, double bottomFreq, double axialFreq);
		
		std::array<double, 3> interpretKeyForRaman( std::array<std::string, 3> raman,  key variationKey, unsigned int variableNumber );

		std::string getErrMessage( long errCode );
};
