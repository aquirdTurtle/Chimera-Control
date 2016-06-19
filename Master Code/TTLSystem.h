#pragma once
#include <array>
#include "Control.h"

class MasterWindow;

class TTLSystem
{
	public:
		TTLSystem();
		~TTLSystem();
		bool initialize(POINT& startLocation, HWND windowHandle);
		int getNumberOfTTLRows();
		int getNumberOfTTLsPerRow();
		bool handleTTLPress(WPARAM parentWParam, LPARAM parentLParam);
		bool handleHoldPress(WPARAM parentWParam, LPARAM parentLParam);
		INT_PTR colorTTLs(HWND window, UINT message, WPARAM wParam, LPARAM lParam, MasterWindow* Master);
	private:
		// one control for each TTL
		Control ttlTitle;
		Control ttlHold;
		std::array< std::array< Control, 16 >, 4 > ttlPushControls;
		std::array< Control, 16 > ttlNumberLabels;
		std::array< Control, 4 > ttlRowLabels;		
		std::array< std::array<bool, 16>, 4 > ttlStatus;
		std::array< std::array<bool, 16>, 4 > ttlHoldStatus;
		bool holdStatus;
};