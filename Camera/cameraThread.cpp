#include "stdafx.h"
#include "cameraThread.h"
#include "externals.h"
#include "constants.h"
#include "appendText.h"
#include "Windows.h"
#include <string>
#include <chrono>

DWORD WINAPI cameraThread(LPVOID voidPtr)
{
	//Windows messages which will be sent to the main application when an event occurs
	eAccMessageID = RegisterWindowMessage("ACQ_TAKEN");
	eFinMessageID = RegisterWindowMessage("ACQ_COMPLETE");
	eErrMessageID = RegisterWindowMessage("ACQ_ERROR");
	
	DWORD successWait;
	int safeModeCount = 0;
	int errorvalue, status;
	long accumulationDummy, seriesNumber;
	bool finSentStatus = false;
	while (eThreadExitIndicator) 
	{
		// alternative to directly using events.
		if (!ANDOR_SAFEMODE)
		{
			successWait = WaitForAcquisition();
			GetStatus(&status);
			if (status == DRV_IDLE)
			{
				// Signal main application
				PostMessage(eCameraWindowHandle, eFinMessageID, 0, 0);
			}
			else
			{
				//Check the number of accumulations and series that have taken place
				errorvalue = GetAcquisitionProgress(&accumulationDummy, &seriesNumber);
				if (errorvalue != DRV_SUCCESS)
				{
					// Signal main application
					PostMessage(eCameraWindowHandle, eErrMessageID, 0, 0);
				}

				// Signal main application
				PostMessage(eCameraWindowHandle, eAccMessageID, 0, seriesNumber);
			}
		}
		else
		{
			// prevents this from flooding the window with messages if camera is off.
			safeModeCount++;
			Sleep(eKineticCycleTime * 1000);
			if (eSystemIsRunning && safeModeCount < eTotalNumberOfPicturesInSeries)
			{
				finSentStatus = false;
				PostMessage(eCameraWindowHandle, eAccMessageID, 0, (LPARAM)safeModeCount);
			}
			else
			{
				if (!eSystemIsRunning)
				{
					safeModeCount = 0;
				}
				else if (finSentStatus == false)
				{
					PostMessage(eCameraWindowHandle, eFinMessageID, 0, 0);
					finSentStatus = true;
				}
			}
		}
	}
	return 0;
}
