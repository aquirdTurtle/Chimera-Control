#pragma once

#include "Control.h"
#include <vector>
#include "VariableSystem.h"

// the first value of the pair in the map is the array of values that the variable name takes on.
// the second value here stores whether it varies or not. This is mainly used to determine if the value is displayed on the status or not.
typedef std::unordered_map<std::string, std::pair<std::vector<double>, bool>> key;

class KeyHandler
{
	public:
		key getKey();
		void loadVariables(std::vector<variable> variables);
		void generateKey( bool randomizeVariablesOption );
		std::vector<double> getKeyValueArray();
	private:
		std::vector<variable> variables;
		key keyValues;
		UINT runningKeyValue;
};