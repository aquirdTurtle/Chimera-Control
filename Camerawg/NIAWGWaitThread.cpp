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
	waitThreadInput input = *(waitThreadInput*)inputParam;
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
			try
			{
				isDone = input.niawg->isDone();
			}
			catch (my_exception&)
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
		if (input.profileInfo.orientation == HORIZONTAL_ORIENTATION)
		{
			// start generic waveform to maintain power output to AOM.
			try
			{
				input.niawg->configureOutputEnabled(VI_TRUE);
				input.niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript");
			}
			catch (my_exception&)
			{
				eWaitError = true;
				return -1;
			}
			eCurrentScript = "DefaultHConfigScript";
		}
		else if (input.profileInfo.orientation == VERTICAL_ORIENTATION)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// start generic waveform to maintain power output to AOM.
				try
				{
					input.niawg->configureOutputEnabled(VI_TRUE);
					input.niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript");
				}
				catch (my_exception&)
				{
					eWaitError = true;
					return -1;
				}
			}
			eCurrentScript = "DefaultVConfigScript";
		}
		try
		{
			input.niawg->initiateGeneration();
		}
		catch (my_exception&)
		{
			eWaitError = true;
			return -1;
		}
	}
	/// now wait until ready.
	WaitForSingleObject(eWaitingForNIAWGEvent, INFINITY);
	/// stop the default.
	// now it's ready. stop the default.
	try
	{
		input.niawg->configureOutputEnabled(VI_FALSE);
		input.niawg->abortGeneration();
	}
	catch (my_exception&)
	{
		eWaitError = true;
		return -1;
	}
	return 0;
}
