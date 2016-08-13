#pragma once

#include <vector>
#include "VariableSystem.h"

class KeyHandler
{
	public:
		std::unordered_map<std::string, std::vector<double>> getKey();
		bool loadVariables(std::vector<variable> variables);
		bool generateKey();
		bool exportKey();

	private:	
		std::vector<variable> variables;
		std::unordered_map<std::string, std::vector<double>> keyValues;
		unsigned int runningKeyValue;
};