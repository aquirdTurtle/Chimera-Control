#pragma once
#include "DmFlume.h"
#include "Expression.h"
#include "Version.h"
#include "DmOutputForm.h"


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
		void handleNewConfig(std::ofstream& newFile);
		void handleSaveConfig(std::ofstream& newFile, DMOutputForm out);
		static DMOutputForm handleGetConfig(std::ifstream& configFile, Version ver);
		static void interpretKey(std::vector<std::vector<parameterType>>& variables, DmCore& DM);
		void ProgramNow(UINT variation);
		void handleOpenConfig(std::ifstream& openFile, Version Ver);
		const std::string delimeter = "DM";
		DMOutputForm getInfo();


	private:
		DmFlume DM;
		std::vector<unsigned int> map_lut = std::vector<unsigned int>(MAX_DM_SIZE, 0);
		std::vector<double> valueArray;
		std::string serial;
		bool boolOfSerial;
		//std::vector<DM_Info> currentDMNums; //vector of outputs
		DMOutputForm currentInfo;
};
