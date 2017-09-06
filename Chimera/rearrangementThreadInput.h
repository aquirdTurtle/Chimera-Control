#pragma once
#include <atomic>
#include <vector>
#include "windows.h"
#include "rearrangementStructures.h"
#include "NiawgController.h"

struct rearrangementThreadInput
{
	std::mutex* rearrangerLock;
	std::atomic<bool>* threadActive;
	// an array of images that have been converted to bools for atom in a pixel or not..
	std::vector<std::vector<bool>>* atomsQueue;
	//std::vector<std::vector<bool>> targetImage;

	//UINT waveNumber;
	//std::vector<double> rearrangeVals;
	waveInfo rearrangementWave;
	NiawgController* niawg;
	Communicator* comm;
};
