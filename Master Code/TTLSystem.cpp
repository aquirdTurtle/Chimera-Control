#include "stdafx.h"
#include "TTLSystem.h"
#include "constants.h"
#include "MasterWindow.h"

bool TTLSystem::initialize(POINT& upperLeftCornerLocation, HWND windowHandle)
{
	// title
	RECT location;
	location.left = upperLeftCornerLocation.x;
	location.top = upperLeftCornerLocation.y;
	location.right = location.left + 480;
	location.bottom = location.top + 25;
	ttlTitle.normalPos = location;
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
		ttlNumberLabels[ttlNumberInc].normalPos = location;
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
		ttlRowLabels[row].normalPos = location;
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
			ttlPushControls[row][number].normalPos = location;
			ttlPushControls[row][number].ID = ID_TTL_BUTTON_A1 + row * ttlPushControls[0].size() + number;
			ttlPushControls[row][number].hwnd = CreateWindowEx(NULL, "BUTTON", "", WS_CHILD | WS_VISIBLE | BS_RIGHT | BS_3STATE,
				location.left, location.top, location.right - location.left, location.bottom - location.top,
				windowHandle, (HMENU)ttlPushControls[row][number].ID, GetModuleHandle(NULL), NULL);
			
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
	ttlHold.normalPos = location;
	ttlHold.hwnd = CreateWindowEx(NULL, "BUTTON", "HOLD", WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX | WS_CHILD | BS_PUSHLIKE,
								  location.left, location.top, location.right - location.left, location.bottom - location.top,
								  windowHandle, (HMENU)ID_TTL_BUTTON_HOLD, GetModuleHandle(NULL), NULL);
	upperLeftCornerLocation.y = location.bottom;
	return true;
}


int TTLSystem::getNumberOfTTLRows()
{
	return ttlPushControls.size();
}
int TTLSystem::getNumberOfTTLsPerRow()
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

TTLSystem::TTLSystem(){}
TTLSystem::~TTLSystem(){}

bool TTLSystem::handleTTLPress(WPARAM parentWParam, LPARAM parentLParam)
{
	if (LOWORD(parentWParam) >= ID_TTL_BUTTON_A1 && LOWORD(parentWParam) < ID_TTL_BUTTON_A1 + ttlPushControls.size() * ttlPushControls[0].size())
	{
		// figure out row #
		int row = (LOWORD(parentWParam) - ID_TTL_BUTTON_A1) / ttlPushControls[0].size();
		// figure out collumn #
		int number = (LOWORD(parentWParam) - ID_TTL_BUTTON_A1) % ttlPushControls[0].size();
		if (holdStatus == false)
		{
			if (this->ttlStatus[row][number])
			{
				SendMessage(ttlPushControls[row][number].hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
				ttlStatus[row][number] = false;
			}
			else
			{
				SendMessage(ttlPushControls[row][number].hwnd, BM_SETCHECK, BST_CHECKED, 0);
				ttlStatus[row][number] = true;
			}
		}
		else
		{
			if (this->ttlHoldStatus[row][number])
			{
				ttlHoldStatus[row][number] = false;
					ttlPushControls[row][number].colorState = -1;
				RedrawWindow(ttlPushControls[row][number].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else
			{
				ttlHoldStatus[row][number] = true;
				ttlPushControls[row][number].colorState = 1;
				RedrawWindow(ttlPushControls[row][number].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		return false;
	}
	else
	{
		return true;
	}

}

bool TTLSystem::handleHoldPress(WPARAM parentWParam, LPARAM parentLParam)
{
	if (LOWORD(parentWParam) != ID_TTL_BUTTON_HOLD)
	{
		return true;
	}
	if (this->holdStatus == true)
	{
		this->holdStatus = false;
		// make changes
		for (int rowInc = 0; rowInc < ttlHoldStatus.size(); rowInc++)
		{
			for (int numberInc = 0; numberInc < ttlHoldStatus[0].size(); numberInc++)
			{
				if (ttlHoldStatus[rowInc][numberInc])
				{
					SendMessage(ttlPushControls[rowInc][numberInc].hwnd, BM_SETCHECK, BST_CHECKED, 0);
					ttlStatus[rowInc][numberInc] = true;
				}
				else
				{
					SendMessage(ttlPushControls[rowInc][numberInc].hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
					ttlStatus[rowInc][numberInc] = false;
				}
				ttlPushControls[rowInc][numberInc].colorState = 0;
				RedrawWindow(ttlPushControls[rowInc][numberInc].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
	}
	else
	{
		this->holdStatus = true;
		ttlHoldStatus = ttlStatus;
	}
	return false;
}

INT_PTR TTLSystem::colorTTLs(HWND window, UINT message, WPARAM wParam, LPARAM lParam, MasterWindow* Master)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	int loword = LOWORD(wParam);
	int raw = loword - ID_TTL_BUTTON_A1;
	if (controlID >= ID_TTL_BUTTON_A1 && controlID < ID_TTL_BUTTON_A1 + ttlPushControls.size() * ttlPushControls[0].size())
	{
		// figure out row #
		int row = (controlID - ID_TTL_BUTTON_A1) / ttlPushControls[0].size();
		// figure out collumn #
		int number = (controlID - ID_TTL_BUTTON_A1) % ttlPushControls[0].size();
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		if (this->ttlPushControls[row][number].colorState == -1)
		{
			SetBkColor(hdcStatic, RGB(200, 50, 50));
			return (LRESULT)Master->redBrush;
		}
		else if (this->ttlPushControls[row][number].colorState == 1)
		{
			SetBkColor(hdcStatic, RGB(50, 200, 50));
			return (LRESULT)Master->greenBrush;
		}
		else
		{
			return DefWindowProc(window, message, wParam, lParam);
		}
	}
	else
	{
		return NULL;
	}
}

