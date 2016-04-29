#pragma once
#include "windows.h"

/*
* This function is called to wait on the NIAWG until it finishes. It will eventually send messages to other threads to indicate when it finishes.
* ViSession inputParam: this is the session handle for the session with the NIAWG.
* Return: the function returns -1 if error, -2 if abort, 0 if normal.
*/
unsigned __stdcall NIAWGWaitThread(void* inputParam);
