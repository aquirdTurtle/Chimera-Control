#pragma once
#include <array>
#include <string>
#include "Control.h"
#include "Windows.h"

class MyDACClass
{
	public:
		MyDACClass();
		~MyDACClass();
		int InitializeDACControls(POINT upperLeftHandCornerPosition, HWND windowHandle);
	private:
		Control dacTitle;
		Control dacSetButton;
		std::array<Control, 24> dacLabels;
		std::array<Control, 24> breakoutBoardEdits;
};