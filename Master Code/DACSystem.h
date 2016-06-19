#pragma once
#include <array>
#include <string>
#include "Control.h"
//#include "Windows.h"

class DACSystem
{
	public:
		DACSystem();
		~DACSystem();
		bool initialize(POINT& upperLeftHandCornerPosition, HWND windowHandle);
		bool handleButtonPress();
	private:
		Control dacTitle;
		Control dacSetButton;
		std::array<Control, 24> dacLabels;
		std::array<Control, 24> breakoutBoardEdits;
		std::array<Control, 24> dacValues;
};