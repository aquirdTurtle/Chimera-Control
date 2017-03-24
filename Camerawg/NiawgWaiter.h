#pragma once
#include "windows.h"
#include "ExperimentManager.h"
#include "niFgen.h"
#include "externals.h"
#include "time.h"
#include <chrono>
#include <thread>
#include "constants.h"


struct experimentThreadInput;
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
		void startWait( experimentThreadInput* input);
		void wait( Communicator* comm );
		void systemAbortCheck( Communicator* comm );
};
