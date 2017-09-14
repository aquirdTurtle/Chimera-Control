#pragma once

#include "Control.h"
#include <vector>
#include "VariableSystem.h"

class KeyHandler
{
	public:
		key getKey();
		void loadVariables(std::vector<variableType> variables);
		void generateKey( bool randomizeVariablesOption );
		std::vector<double> getKeyValueArray();
	private:
		std::vector<variableType> variables;
		key keyValues;
		UINT runningKeyValue;
};