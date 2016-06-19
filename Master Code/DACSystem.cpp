#include "stdafx.h"
#include "DACSystem.h"
#include "constants.h"
DACSystem::DACSystem()
{
}
DACSystem::~DACSystem()
{
}
// this function returns the end location of the set of controls. This can be used for the location for the next control beneath it.
bool DACSystem::initialize(POINT& upperLeftHandCornerPosition, HWND windowHandle)
{
	// title
	RECT location;
	location.left = upperLeftHandCornerPosition.x;
	location.top = upperLeftHandCornerPosition.y;
	location.right = location.left + 480;
	location.bottom = location.top + 25;
	dacTitle.normalPos = location;
	dacTitle.hwnd = CreateWindowEx(0, "STATIC", "DAC Channel Voltage Values", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
								   location.left, location.top, location.right - location.left, location.bottom - location.top,
								   windowHandle, (HMENU)-1, GetModuleHandle(NULL), NULL);
	// all number labels
	// all row labels
	location.left = upperLeftHandCornerPosition.x;
	location.top = upperLeftHandCornerPosition.y + 25;
	location.right = location.left + 480;
	location.bottom = location.top + 25;

	dacSetButton.normalPos = location;
	dacSetButton.hwnd = CreateWindowEx(NULL, "BUTTON", "Set New DAC Values", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
													  location.left, location.top, location.right - location.left, location.bottom - location.top,
													  windowHandle, (HMENU)ID_DAC_SET_BUTTON, GetModuleHandle(NULL), NULL);	
	location.top += 25;
	location.right = location.left + 20;
	location.bottom = location.top + 25;
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		if (dacInc == dacLabels.size() / 3 || dacInc == 2 * dacLabels.size() / 3)
		{
			// go to second collumn
			location.left += 160;
			location.right += 160;
			location.top -= 25 * dacLabels.size() / 3;
			location.bottom -= 25 * dacLabels.size() / 3;
		}
		// create label
		dacLabels[dacInc].normalPos = location;
		dacLabels[dacInc].hwnd = CreateWindowEx(0, "STATIC", std::to_string(dacInc + 1).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
			location.left, location.top, location.right - location.left, location.bottom - location.top,
			windowHandle, (HMENU)-1, GetModuleHandle(NULL), NULL);
		location.left += 20;
		location.right += 140;
		dacLabels[dacInc].normalPos = location;
		dacLabels[dacInc].hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE,
			location.left, location.top, location.right - location.left, location.bottom - location.top,
			windowHandle, (HMENU)ID_DAC_EDIT_1 + dacInc, GetModuleHandle(NULL), NULL);
		location.left -= 20;
		location.right -= 140;
		location.top += 25;
		location.bottom += 25;
		dacLabels[dacInc].ID = ID_DAC_EDIT_1 + dacInc;
	}
	upperLeftHandCornerPosition.y = location.top;
	return true;
}

bool DACSystem::handleButtonPress()
{
	for (int dacInc = 0; dacInc < dacLabels.size(); dacInc++)
	{
		int length = SendMessage(dacLabels[dacInc].hwnd, WM_GETTEXTLENGTH, 0, 0);
		char* dacText = new char[length + 1];
		SendMessage(dacLabels[dacInc].hwnd, WM_GETTEXT, length + 1, (LPARAM)dacText);
		std::string dacString(dacText);
		try
		{

		}
		catch (std::exception& exc)
		{

		}
	}
	return false;
}