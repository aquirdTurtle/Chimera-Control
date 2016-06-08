#pragma once

#include "stdafx.h"
#include "NIAWGWaitThread.h"
#include "Windows.h"
#include "niFgen.h"
#include "externals.h"
#include "time.h"
#include <chrono>
#include <thread>
#include "constants.h"

/*
 * This function is called to wait on the NIAWG until it finishes. It will eventually send messages to other threads to indicate when it finishes. 
 * ViSession inputParam: this is the session handle for the session with the NIAWG. 
 * Return: the function returns -1 if error, -2 if abort, 0 if normal.
 */
unsigned __stdcall NIAWGWaitThread(void* inputParam)
{
	ViSession currentSession = *(ViSession*)inputParam;
	ViBoolean isDone;
	if (!TWEEZER_COMPUTER_SAFEMODE)
	{
		 isDone = FALSE;
	}
	else if (TWEEZER_COMPUTER_SAFEMODE)
	{
		isDone = TRUE;
		Sleep(2000);
	}
	while (!isDone)
	{
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			if ((niFgen_IsDone(currentSession, &isDone)) < 0)
			{
				eWaitError = true;
				return -1;
			}
		}
		if (eAbortSystemFlag == true)
		{
			return -2;
		}
	}
	if (eAbortSystemFlag == true)
	{
		return -2;
	}
	if (WaitForSingleObjectEx(eWaitingForNIAWGEvent, 0, true) == WAIT_TIMEOUT)
	{
		/// then it's not ready. start the default
		if (eProfile.getOrientation() == HORIZONTAL_ORIENTATION)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// start generic waveform to maintain power output to AOM.
				if ((niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)) < 0)
				{
					eWaitError = true;
					return -1;
				}
				if ((niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")) < 0)
				{
					eWaitError = true;
					return -1;
				}
			}
			eCurrentScript = "DefaultHConfigScript";

		}
		else if (eProfile.getOrientation() == VERTICAL_ORIENTATION)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				if ((niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)) < 0)
				{
					eWaitError = true;
					return -1;
				}
				if ((niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript")) < 0)
				{
					eWaitError = true;
					return -1;
				}
			}
			eCurrentScript = "DefaultVConfigScript";
		}
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			// Initiate Generation.
			if ((niFgen_InitiateGeneration(eSessionHandle)) < 0)
			{
				eWaitError = true;
				return -1;
			}
		}
		/// now wait until ready.
		WaitForSingleObject(eWaitingForNIAWGEvent, INFINITY);
		/// stop the default.
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
			// then it's not ready. start the default
			if ((niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)) < 0)
			{
				eWaitError = true;
				return -1;
			}
			// Officially stop trying to generate anything.
			if ((niFgen_AbortGeneration(eSessionHandle)) < 0)
			{
				eWaitError = true;
				return -1;
			}
		}
	}
	return 0;
}
