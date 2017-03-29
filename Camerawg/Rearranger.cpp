#include "stdafx.h"
#include "Rearranger.h"

void Rearranger::startThread()
{
	threadState = true;
}


bool Rearranger::threadIsActive()
{
	return threadState;
}

// calculate the maximum possible time that the rearranging algorithm could take, and set that as the streaming time.
double Rearranger::getStreamingTime()
{
	// dummy
	return -10000;
}


void Rearranger::rearrange()
{
	// calc pattern.
	// convert pattern to waveform data. 
	// stream data.
	// send software trigger.
}


void Rearranger::calculateRearrangingMoves(std::vector<std::vector<bool>> initArrangement)
{
	//... Kai's work
}


unsigned int __stdcall Rearranger::rearrangerThreadFunction( LPVOID input )
{
	// wait for data

	// get initial state from picture.
	std::vector<std::vector<bool>> initialState;

	// rearrange

	// repeat until experiment over.

	return 0;
}
