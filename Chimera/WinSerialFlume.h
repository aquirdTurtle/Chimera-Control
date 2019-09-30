// created by Mark O. Brown
#pragma once

// don't think I've every really used this.
class WinSerialFlume
{
	public:
		WinSerialFlume( bool safemode_option, std::string portAddress );
		void open ( std::string fileAddr );
		void close( );
		void write ( std::string msg );
		std::string read (  );
		std::string query ( std::string msg );
	private:
		const bool safemode;
		HANDLE serialPortHandle;
};
