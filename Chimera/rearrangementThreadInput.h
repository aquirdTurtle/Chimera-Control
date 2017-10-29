#pragma once
//#include "NiawgController.h"
#include "rearrangementStructures.h"
#include "rearrangementMoveContainer.h"
#include <chrono>
#include <atomic>
#include <vector>
#include "windows.h"

class NiawgController;

struct rearrangementThreadInput
{
	rearrangementThreadInput( UINT gridRows, UINT gridCols ) : moves( gridRows, gridCols ) {}

	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* pictureTimes;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* grabTimes;
	std::mutex* rearrangerLock;
	std::condition_variable* rearrangerConditionWatcher;
	std::atomic<bool>* threadActive;
	// an array of images that have been converted to bools for atom in a pixel or not..
	std::vector<std::vector<bool>>* atomsQueue;
	// the static wave?
	waveInfo rearrangementWave;
	NiawgController* niawg;
	Communicator* comm;
	// stuff from the rearrangement input
	rearrangeParams rearrangeOptions;
	rearrangementMoveContainer moves;
};

