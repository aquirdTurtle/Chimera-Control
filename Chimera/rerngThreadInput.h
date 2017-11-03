#pragma once
#include "rerngStructures.h"
#include "rerngContainer.h"
#include "rerngMove.h"
#include <chrono>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "windows.h"

class NiawgController;

// rerng = rearrange
struct rerngThreadInput
{
	rerngThreadInput( UINT gridRows, UINT gridCols ) : moves( gridRows, gridCols ) {}

	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* pictureTimes;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* grabTimes;
	std::mutex* rerngLock;
	std::condition_variable* rerngConditionWatcher;
	std::atomic<bool>* threadActive;
	// an array of images that have been converted to bools for atom in a pixel or not..
	std::vector<std::vector<bool>>* atomsQueue;
	// the static wave?
	waveInfo rerngWave;
	NiawgController* niawg;
	Communicator* comm;
	// stuff from the rearrangement input
	rerngOptions rerngOptions;
	rerngContainer<rerngMove> moves;
	//waveInfo fillerWave;
};

