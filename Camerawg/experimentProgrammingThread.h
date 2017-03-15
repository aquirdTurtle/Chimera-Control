#pragma once
#include "Windows.h"
/*
* This runs the experiment. It calls analyzeNiawgScripts and then procedurally goes through all variable values. It also communicates with the other computer
* throughout the process.
* inputParam is the list of all of the relevant parameters to be used during this run of the experiment.
*/
unsigned __stdcall experimentProgrammingThread(LPVOID inputParam);

