#pragma once
#include "Control.h"

class MyVariableClass
{
	public:
		MyVariableClass();
		~MyVariableClass();
		int initializeVariableControls(POINT upperLeftHandLocation, HWND parentWindow);
		int addVariable(std::string variableName, double initialVal, double finalVal, std::string units);
		int deleteVariable();
	private:
		Control variableTitle;
		Control variableListControl;
		Control cNewVariableButton;
		Control cDeleteButton;
};