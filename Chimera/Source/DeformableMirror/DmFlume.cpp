#include "stdafx.h"
#include "DmFlume.h"
#include "BMCDefs.h"


DmFlume::DmFlume(bool safemode_option) {
	safemode = safemode_option;
	hdm = {};
	if (safemode) {
		ActuatorCount = 137;
	}
	open("25CW012#060");
}


void DmFlume::open(std::string serialNumber) {
	if(!safemode){
		if ((serialNumber.length()) > 12) {
			thrower("Serial Number is incorrect");
		}
		else {
			BMCRC err = NO_ERR;
			const char* SerialChar = serialNumber.c_str();
			err = BMCOpen(&hdm, SerialChar);
			if (err != NO_ERR) {
				thrower("Error opening the drivertype" + hdm.Driver_Type + str(BMCErrorString(err)));
			}
			else {
				ActuatorCount = hdm.ActCount;
			}
		}
	}
}

void DmFlume::close() {
	if (!safemode) {
		BMCRC err = NO_ERR;
		err = BMCClose(&hdm);
		if (err != NO_ERR) {
			thrower("Error closing the drivertype" + hdm.Driver_Type);
			thrower(BMCErrorString(err));
		}
	}
}

std::string DmFlume::getDeviceInfo(std::string serialNumber) {
	std::string msg = "";
	if (!safemode) 	{
		struct DM_DRIVER Mirror = hdm.driver;
		msg += ("Driver: " + str(hdm.Driver_Type));
		msg += "\n";
		msg += ("\t Actuator Count: " + str(hdm.ActCount));
		msg += "\n";
		msg += ("\t Serial Number:" + std::string(Mirror.serial_number));
		msg += "\n";
	}
	else {
		msg = "No Device Found.";
	}
	return msg;
}

void DmFlume::setSingle(unsigned int Actuator, double Value) {
	if (!safemode) {
		BMCRC err = NO_ERR;
		if (Value > 1 || Value < 0) {
			thrower("Actuator Value must be in range [0, 1]");
		}
		else if (Actuator > ActuatorCount - 1) {
			thrower("Actuator specified is higher than the Actuator Count");
		}
		else {
			err = BMCSetSingle(&hdm, Actuator, Value);
			if (err != NO_ERR) {
				thrower(BMCErrorString(err));
			}
		}
	}
}

void DmFlume::getArray(double* vectorPointer, unsigned int length) {
	if(!safemode){
		BMCRC err = NO_ERR;
		if (length < ActuatorCount) {
			thrower("Length of array is not larger enough");
		}
		else {
			err = BMCGetArray(&hdm, vectorPointer, length);
			if (err != NO_ERR) {
				thrower(BMCErrorString(err));
			}
		}
	}
}

void DmFlume::LoadMap(unsigned int* MAP) {
	if (!safemode) {
		BMCRC err = NO_ERR;
		err = BMCLoadMap(&hdm, nullptr, MAP);
		if (err != NO_ERR) {
			thrower(BMCErrorString(err));
		}
	}
}

void DmFlume::setArray(double* vectorPointer, unsigned int* MAP) {
	if (!safemode) {
		BMCRC err = NO_ERR;
		err = BMCSetArray(&hdm, vectorPointer, MAP);
		if (err != NO_ERR) {
			thrower(BMCErrorString(err));
		}
	}

}

void DmFlume::reset() {
	if (!safemode) {
		BMCRC err = NO_ERR;
		err = BMCClearArray(&hdm);
		if (err != NO_ERR) {
			thrower(BMCErrorString(err));
		}
	}
}

DmFlume::~DmFlume() {
	close();
}