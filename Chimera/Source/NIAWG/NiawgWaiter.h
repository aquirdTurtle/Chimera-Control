// created by Mark O. Brown
#pragma once
#include <chrono>
#include <thread>

#include "afxwin.h"
#include "niFgen.h"
#include "LowLevel/constants.h"
#include "ConfigurationSystems/profileSettings.h"
#include "NIAWG/NiawgCore.h"

struct niawgIntensityThreadInput;
struct ExperimentThreadInput;
struct waitThreadInput
{
	NiawgCore* niawg;
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
		void startWaitThread( NiawgCore* niawgPtr );
		void wait( Communicator& comm );
		void systemAbortCheck( Communicator& comm );
};
