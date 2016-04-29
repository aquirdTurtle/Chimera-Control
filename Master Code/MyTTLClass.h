#pragma once
#include <array>
#include "Control.h"

class MyTTLClass
{
	public:
		MyTTLClass();
		~MyTTLClass();
		int CreateTTLControls(POINT startLocation, HWND windowHandle);
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
	private:
		// one control for each TTL
		Control ttlTitle;
		Control ttlHold;
		std::array<std::array<Control, 16>, 4 > ttlPushControls;
		std::array < Control, 16 > ttlNumberLabels;
		std::array<Control, 4> ttlRowLabels;		
};