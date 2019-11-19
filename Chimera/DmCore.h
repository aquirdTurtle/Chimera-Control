#pragma once
#include "DmFlume.h"
#include "Expression.h"

class DmCore {
	public:
		DmCore::DmCore(std::string Number, bool safemodeOption);
		std::string getDeviceInfo();
		void setSerial(std::string Number);
		void initialize();
		void setMap();
		void pokePiston(unsigned int piston, double value);
		void zeroPistons();
		void readDMArray(std::vector<double> &testArray);
		void testMirror();
		void loadArray(double *A);
		int getActCount();
		std::vector<double> getActuatorValues();
	private:
		DmFlume DM;
		std::vector<unsigned int> map_lut = std::vector<unsigned int>(MAX_DM_SIZE, 0);
		std::vector<double> valueArray;
		std::string serial;
		bool boolOfSerial;
};
