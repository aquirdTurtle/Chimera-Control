#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "GPIB.h"
class RhodeSchwarz
{
	public:
		// RhodeSchwarz();
		bool initialize();
		bool programRSG(GPIB* gpibHandler);
		bool addFrequency(std::string frequency);
		bool clearFrequencies();
		std::vector<std::string> getFrequencyForms();
		bool interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNum);
	private:
		std::vector<std::string> frequencyForms;
		std::vector<double> frequencies;
};