#pragma once


class WinSerialFlume
{
	public:
		WinSerialFlume( bool safemode_option );
		// probably need an open() function to actually use this...
		void close( );
		unsigned long WinSerialFlume::writeFile( unsigned long index, std::vector<unsigned char> dataBuffer );
	private:
		const bool safemode;
		HANDLE m_hSerialComm;
};
