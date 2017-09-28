#pragma once
#include "stdafx.h"
#include "NiawgWaiter.h"

/*
 * This function is called to wait on the NIAWG until it finishes. It will eventually send messages to other threads 
 * to indicate when it finishes. If the niawg finishes before programming does, it starts outputting the default
 * waveform in order to maintain a constant amount of power to the deflection AOMs.
 * Return: the function returns -1 if error, -2 if abort, 0 if normal.
 // opps it returns an unsigned. Watch out for that, -1 goes to huge number.
 */
unsigned __stdcall NiawgWaiter::niawgWaitThread(void* inputParam)
{
	waitThreadInput* input = (waitThreadInput*)inputParam;
	ViBoolean isDone = FALSE;
	if (NIAWG_SAFEMODE)
	{
		// basically just pass through with a little wait, as if the niawg ran for 2 seconds then finished.
		isDone = TRUE;
		Sleep(2000);
	}

	while (!isDone)
	{
		if (eAbortNiawgFlag)
		{
			// aborting
			return -2;
		}
		else
		{
			try
			{
				isDone = input->niawg->fgenConduit.isDone();
			}
			catch (Error&)
			{
				eWaitError = true;
				delete input;
				return -1;
			}
		}
	}
	if (eAbortNiawgFlag)
	{
		return -2;
	}

	if (WaitForSingleObjectEx(eWaitingForNIAWGEvent, 0, true) == WAIT_TIMEOUT)
	{
		/// then it's not ready. start the default generic waveform to maintain power output to AOM.
		try
		{
			input->niawg->setDefaultWaveformScript( );
			input->niawg->turnOn();
		}
		catch (Error&)
		{
			eWaitError = true;
			delete input;
			return -1;
		}
	}
	/// now wait until programming is finished and main thread is ready.
	WaitForSingleObject(eWaitingForNIAWGEvent, ULONG(INFINITY));
	/// stop the default.
	// now it's ready. stop the default.
	try
	{
		input->niawg->turnOff();
	}
	catch (Error&)
	{
		eWaitError = true;
		delete input;
		return -1;
	}
	delete input;
	return 0;
}


void NiawgWaiter::startWaitThread( NiawgController* niawgPtr, profileSettings profile )
{
	eWaitError = false;
	// create the waiting thread.
	waitThreadInput* waitInput = new waitThreadInput;
	waitInput->niawg = niawgPtr;
	waitInput->profile = profile;
	UINT NIAWGThreadID;
	eNIAWGWaitThreadHandle = (HANDLE)_beginthreadex( 0, 0, &NiawgWaiter::niawgWaitThread, waitInput, 0, &NIAWGThreadID );
}


void NiawgWaiter::initialize()
{
	eWaitingForNIAWGEvent = CreateEvent( NULL, FALSE, FALSE, TEXT( "eWaitingForNIAWGEvent" ) );
}


// waits for waiting thread to finish it's execution. If the niawg finished before the programming, this will return 
// immediately. I think that I can get rid of the abort checks in here, this only ever gets called at the very end
// of the experiment and there's no reason for the checks to be this far inside the niawg handling.
void NiawgWaiter::wait( Communicator* comm )
{
	systemAbortCheck( comm );
	SetEvent( eWaitingForNIAWGEvent );
	WaitForSingleObject( eNIAWGWaitThreadHandle, INFINITE );
	systemAbortCheck( comm );
	// check this flag that can be set by the wait thread.
	if (eWaitError)
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
	// check if aborting
	if ( eAbortNiawgFlag )
	{
		comm->sendStatus( "Aborted!\r\n" );
		thrower( "Aborted!\r\n" );
	}
}