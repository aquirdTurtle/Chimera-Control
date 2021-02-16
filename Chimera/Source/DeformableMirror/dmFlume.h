#pragma once
#include "BmcApi.h"
#include "ParameterSystem/Expression.h"

class DmFlume {
	public:
		DmFlume(bool safemode_option);
		~DmFlume();
		void open(std::string serialNumber);
		void close();
		void setSingle(unsigned int Actuator, double Value);
		std::string getDeviceInfo(std::string serialNumber);
		void getArray(double* vectorPointer, unsigned int length);
		void reset();
		void setArray(double* vectorPointer, unsigned int* MAP);
		void LoadMap(unsigned int* MAP);
		unsigned int getActuatorCount() { return ActuatorCount; }

	private:
		bool safemode;
		DM hdm;
		unsigned int ActuatorCount;
};