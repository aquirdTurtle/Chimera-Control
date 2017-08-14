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
		void generateKey();
		void exportKey();
		void initialize(POINT loc, CWnd* parent, int& id);
		void rearrange(int width, int height, fontMap fonts);
		
	private:	
		Control<CButton> randomizeVariablesButton;
		Control<CButton> randomizeRepetitionsButton;
		std::vector<variable> variables;
		key keyValues;
		UINT runningKeyValue;
};