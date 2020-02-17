#pragma once

#include "GeneralFlumes/WinSerialFlume.h"
#include <string>
#include <vector>

/*
For the older piezo drivers which only include a serial port and no dll for the serial communication, I need to 
implement all the serial communication myself. 
*/
class SerialPiezoFlume
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		SerialPiezoFlume& operator=(const SerialPiezoFlume&) = delete;
		SerialPiezoFlume (const SerialPiezoFlume&) = delete;

		SerialPiezoFlume ( bool sMode, std::string sn );
		std::string getDeviceInfo ( );
		void open ( );
		void close ( );
		void setXAxisVoltage ( double val );
		double getXAxisVoltage ( );
		void setYAxisVoltage ( double val );
		double getYAxisVoltage ( );
		void setZAxisVoltage ( double val );
		double getZAxisVoltage ( );
	private:
		WinSerialFlume flumeCore;
		const bool safemode;
		
};