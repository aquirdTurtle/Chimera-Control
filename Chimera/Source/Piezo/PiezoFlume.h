#pragma once

#include <string>

class PiezoFlume
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		PiezoFlume& operator=(const PiezoFlume&) = delete;
		PiezoFlume (const PiezoFlume&) = delete;

		PiezoFlume ( bool sMode, std::string sn);
		std::string list (  );
		std::string getDeviceInfo ( );
		void open ( );
		bool isOpen ( );
		void close ( );
		std::string getID ( );
		std::string getSerialNumber ( );
		double getLimitVoltage ( );
		void setXAxisVoltage ( double val );
		double getXAxisVoltage (  );
		void setYAxisVoltage ( double val );
		double getYAxisVoltage ( );
		void setZAxisVoltage ( double val );
		double getZAxisVoltage ( );

	private:
		const bool safemode;
		std::vector<unsigned char> comPortNumber;
		int deviceHandle=0;
		const unsigned bufferSize = 256;
};