#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <vector>

// this fairly simplistic class could easily just be a namespace instead.
class Gpib
{
	public:
		Gpib::Gpib(short deviceID, bool safemode);
		std::string query( std::string query );
		std::string queryIdentity();
		int ibdev( int pad );
		void send( std::string message );
		std::string receive();
		std::string getErrMessage( long errCode );
	private:
		short deviceID;
		bool deviceSafemode;
};
