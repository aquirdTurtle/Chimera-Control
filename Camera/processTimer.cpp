#include "stdafx.h"
#include "processTimer.h"
#include "atmcd32d.h"
#include "myAndor.h"
#include "constants.h"
#include "externals.h"
#include "appendText.h"
#include <string>
void processTimer(WPARAM wparam)
{
	int temperature, errorValue = wparam;
	switch (wparam) 
	{
		case ID_TEMPERATURE_TIMER: 
		{
			// This case displays the current temperature in the main window. When the temp stabilizes at the desired 
			// level the appropriate message is displayed.
			std::string errMsg;
			if (!ANDOR_SAFEMODE)
			{
				errMsg = myAndor::andorErrorChecker(GetTemperature(&temperature));
			}
			else
			{
				errMsg = "SAFEMODE";
				temperature = 25;
			}

			if (errMsg == "DRV_TEMPERATURE_STABILIZED")
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Temperature has stabilized at " + std::to_string(temperature) + " (C)\r\n").c_str());
			}
			else if (errMsg == "DRV_TEMPERATURE_NOT_REACHED")
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Current temperature is " + std::to_string(temperature) + " (C)\r\n").c_str());
			}
			else if (errMsg == "DRV_TEMP_NOT_STABILIZED")
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Temperature of " + std::to_string(temperature) + " (C) reached but not stable.").c_str());
			}
			else if (errMsg == "DRV_TEMP_DRIFT")
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"Temperature had stabilized but has since drifted.");
			}
			else if (errMsg == "DRV_TEMP_OFF")
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"Temperature control is off.");
			}
			else if (errMsg == "SAFEMODE")
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Application is running in ANDOR_SAFEMODE."));
			}
			else
			{
				SendMessage(eCurrentTempDisplayHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Unexpected Temperature Code: " + errMsg).c_str());
			}
		}
	}
}
