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
unsigned __stdcall NiawgWaiter::niawgWaitThread(void* inputParam)
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
			catch (myException&)
			{
				eWaitError = true;
				return -1;
			}
		}
		if (eAbortNiawgFlag == true)
		{
			return -2;
		}
	}
	if (eAbortNiawgFlag == true)
	{
		return -2;
	}
	if (WaitForSingleObjectEx(eWaitingForNIAWGEvent, 0, true) == WAIT_TIMEOUT)
	{
		/// then it's not ready. start the default
		if (input.profile.orientation == HORIZONTAL_ORIENTATION)
		{
			// start generic waveform to maintain power output to AOM.
			try
			{
				input.niawg->configureOutputEnabled(VI_TRUE);
				input.niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript");
			}
			catch (myException&)
			{
				eWaitError = true;
				return -1;
			}
			eCurrentScript = "DefaultHConfigScript";
		}
		else if (input.profile.orientation == VERTICAL_ORIENTATION)
		{
			if (!TWEEZER_COMPUTER_SAFEMODE)
			{
				// start generic waveform to maintain power output to AOM.
				try
				{
					input.niawg->configureOutputEnabled(VI_TRUE);
					input.niawg->setViStringAttribute(NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript");
				}
				catch (myException&)
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
		catch (myException&)
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
	catch (myException&)
	{
		eWaitError = true;
		return -1;
	}
	return 0;
}


void NiawgWaiter::startWait( experimentThreadInput* input )
{
	eWaitError = false;
	// create the waiting thread.
	waitThreadInput waitInput;
	waitInput.niawg = input->niawg;
	waitInput.profile = input->profile;
	unsigned int NIAWGThreadID;
	eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex( 0, 0, this->niawgWaitThread, &waitInput, 0, &NIAWGThreadID );
}

void NiawgWaiter::initialize()
{
	eWaitingForNIAWGEvent = CreateEvent( NULL, FALSE, FALSE, TEXT( "eWaitingForNIAWGEvent" ) );
}


void NiawgWaiter::wait( Communicator* comm, bool& deleteWaveforms )
{
	systemAbortCheck( comm, deleteWaveforms );
	SetEvent( eWaitingForNIAWGEvent );
	WaitForSingleObject( eNIAWGWaitThreadHandle, INFINITE );
	systemAbortCheck( comm, deleteWaveforms );
	// check this flag that can be set by the wait thread.
	if (eWaitError == true)
	{
		eWaitError = false;
		thrower( "ERROR: Error in the wait function!\r\n" );
	}
}

/*
* This function checks whether the system abort flag has been set, and if so, sends some messages and returns true. If not aborting, it returns false.
*/
void NiawgWaiter::systemAbortCheck( Communicator* comm )
{
	bool dummy;
	systemAbortCheck( comm, dummy );
}

void NiawgWaiter::systemAbortCheck( Communicator* comm, bool& aborting )
{
	// check if aborting
	if (eAbortNiawgFlag == true)
	{
		comm->sendStatus( "Aborted!\r\n" );
		aborting = true;
		thrower( "Aborted!" );
	}
}