// created by Mark O. Brown
#pragma once
#include "windows.h"
class WinSerialFlume {
	public:
		// THIS CLASS IS NOT COPYABLE.
		WinSerialFlume& operator=(const WinSerialFlume&) = delete;
		WinSerialFlume (const WinSerialFlume&) = delete;

		WinSerialFlume( bool safemode_option, std::string portAddress );
		void open ( std::string fileAddr );
		void close( );
		void write ( std::string msg );
		void resetConnection ();
		std::string read (  );
		std::string query ( std::string msg );

	private:
		const bool safemode;
		HANDLE serialPortHandle;
		std::string portAddress;
};
