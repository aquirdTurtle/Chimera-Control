#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <vector>

class GPIB
{
	public:
		bool registerGPIB();
		bool unregisterGPIB();
		bool copyIBVars();
		bool ibwrt();
		bool ibwrti();
		bool ibrd();
		bool ibeot();
		bool gpibSend(int address, std::string message);
		bool enableRemote();
		bool ildev();
		bool programRamanFGs(double topFreq, double bottomFreq, double axialFreq);
		std::array<double, 3> GPIB::interpretKeyForRaman(std::array<std::string, 3> raman, std::unordered_map<std::string, std::vector<double>> key,
			unsigned int variableNumber);
	private:
		bool isRegistered;
};