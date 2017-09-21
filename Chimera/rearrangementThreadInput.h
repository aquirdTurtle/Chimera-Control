#pragma once
#include "rearrangementStructures.h"
#include "NiawgController.h"
#include <chrono>
#include <atomic>
#include <vector>
#include "windows.h"

struct rearrangementThreadInput
{
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* pictureTimes;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* grabTimes;
	std::mutex* rearrangerLock;
	std::condition_variable* rearrangerConditionWatcher;
	std::atomic<bool>* threadActive;
	// an array of images that have been converted to bools for atom in a pixel or not..
	std::vector<std::vector<bool>>* atomsQueue;
	waveInfo rearrangementWave;
	NiawgController* niawg;
	Communicator* comm;
	rearrangeParams info;
};
