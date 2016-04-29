#include "stdafx.h"
#include "MyTTLClass.h"
#include "constants.h"

int MyTTLClass::CreateTTLControls(POINT upperLeftCornerLocation, HWND windowHandle)
{
	// title
	RECT location;
	location.left = upperLeftCornerLocation.x;
	location.top = upperLeftCornerLocation.y;
	location.right = location.left + 480;
	location.bottom = location.top + 25;
	ttlTitle.normalPosition = location;
	ttlTitle.hwnd = CreateWindowEx(0, "STATIC", "TTL Boards High / Low Control", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
								   location.left, location.top, location.right - location.left, location.bottom - location.top,
								   windowHandle, (HMENU)-1, GetModuleHandle(NULL), NULL);
	// all number labels
	location.left += 32;
	location.right = location.left + 28;
	location.top += 25;
	location.bottom += 20;
	for (int ttlNumberInc = 0; ttlNumberInc < ttlNumberLabels.size(); ttlNumberInc++)
	{
		ttlNumberLabels[ttlNumberInc].normalPosition = location;
		ttlNumberLabels[ttlNumberInc].hwnd = CreateWindowEx(0, "STATIC", std::to_string(ttlNumberInc + 1).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
			location.left, location.top, location.right - location.left, location.bottom - location.top,
			windowHandle, (HMENU)-1, GetModuleHandle(NULL), NULL);
		location.left += 28;
		location.right += 28; 
	}
	// all row labels
	location.left = upperLeftCornerLocation.x;
	location.top = upperLeftCornerLocation.y + 45;
	location.right = location.left + 32;
	location.bottom = location.top + 28;
	for (int row = 0; row < ttlPushControls.size(); row++)
	{
		ttlRowLabels[row].normalPosition = location;
		std::string rowName;
		switch (row)
		{
			case 0:
				rowName = "A";
				break;
			case 1:
				rowName = "B";
				break;
			case 2:
				rowName = "C";
				break;
			case 3:
				rowName = "D";
				break;
		}
		ttlRowLabels[row].hwnd = CreateWindowEx(0, "STATIC", rowName.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
			location.left, location.top, location.right - location.left, location.bottom - location.top,
			windowHandle, (HMENU)-1, GetModuleHandle(NULL), NULL);
		location.top += 28;
		location.bottom += 28;
	}
	// all push buttons
	location.left = upperLeftCornerLocation.x + 32;
	location.top = upperLeftCornerLocation.y + 45;
	location.right = location.left + 28;
	location.bottom = location.top + 28;
	for (int row = 0; row < ttlPushControls.size(); row++)
	{
		for (int number = 0; number < ttlPushControls[row].size(); number++)
		{
			ttlPushControls[row][number].normalPosition = location;
			ttlPushControls[row][number].hwnd = CreateWindowEx(NULL, "BUTTON", "", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
				location.left, location.top, location.right - location.left, location.bottom - location.top,
				windowHandle, (HMENU)ID_TTL_BUTTON_A1 + row * ttlPushControls.size() + number, GetModuleHandle(NULL), NULL);
			location.left += 28;
			location.right += 28;
		}
		location.left = upperLeftCornerLocation.x + 32;
		location.right = location.left + 28;
		location.top += 28;
		location.bottom += 28;
	}
	location.left = upperLeftCornerLocation.x;
	location.right = location.left + 480;
	location.bottom += 5;
	ttlHold.normalPosition = location;
	ttlHold.hwnd = CreateWindowEx(NULL, "BUTTON", "HOLD", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
								  location.left, location.top, location.right - location.left, location.bottom - location.top,
								  windowHandle, (HMENU)ID_TTL_BUTTON_HOLD, GetModuleHandle(NULL), NULL);
	return location.bottom;
}


int MyTTLClass::getNumberOfTTLRows()
{
	return ttlPushControls.size();
}
int MyTTLClass::getNumberOfTTLsPerRow()
{
	if (ttlPushControls.size() > 0)
	{
		return ttlPushControls[0].size();
	}
	else
	{
		// shouldn't happen. always have ttls. 
		return -1;
	}
}

MyTTLClass::MyTTLClass()
{
	//ttlPushControls[row][number]
}
MyTTLClass::~MyTTLClass(){}