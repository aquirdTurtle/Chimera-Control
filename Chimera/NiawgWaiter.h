#pragma once
#include <chrono>
#include <thread>

#include "windows.h"
#include "niFgen.h"
#include "externals.h"
#include "time.h"
#include "constants.h"


struct niawgIntensityThreadInput;
struct MasterThreadInput;
struct waitThreadInput
{
	NiawgController* niawg;
	profileSettings profile;
};

/*
* This function is called to wait on the NIAWG until it finishes. It will eventually send messages to other threads to indicate when it finishes.
* ViSession inputParam: this is the session handle for the session with the NIAWG.
* Return: the function returns -1 if error, -2 if abort, 0 if normal.
*/
class NiawgWaiter
{
	public:
		void initialize();
		static unsigned __stdcall niawgWaitThread( void* inputParam );
		void startWaitThread( NiawgController* niawgPtr, profileSettings profile );
		void wait( Communicator* comm );
		void systemAbortCheck( Communicator* comm );
};
