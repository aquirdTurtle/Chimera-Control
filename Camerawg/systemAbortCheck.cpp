#include "stdafx.h"
#include "systemAbortCheck.h"
#include "externals.h"
#include "postMyString.h"

/*
 * This function checks whether the system abort flag has been set, and if so, sends some messages and returns true. If not aborting, it returns false.
 */
bool systemAbortCheck(Communicator* comm)
{
	// check if aborting
	if (eAbortSystemFlag == true)
	{
		comm->sendStatus("Aborted!\r\n");
		return true;
	}
	else
	{
		return false;
	}
}
