#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <vector>

// this fairly simplistic class could easily just be a namespace instead.
class GpibFlume
{
	public:
		GpibFlume::GpibFlume(short deviceID, bool safemode);
		std::string query( std::string query );
		std::string queryIdentity();
		int ibdev( int pad );
		void send( std::string message, bool checkError=true );
		std::string receive();
		std::string getErrMessage( long errCode );
		void queryError( );
	private:
		short deviceID;
		bool deviceSafemode;
};
