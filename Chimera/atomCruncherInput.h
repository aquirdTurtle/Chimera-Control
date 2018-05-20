#pragma once
#include "atomGrid.h"
#include "imageParameters.h"
#include "commonTypes.h"
#include <atomic>
#include <vector>
#include <mutex>
#include "Windows.h"
#include <array>


struct atomCruncherInput
{
	//
	chronoTimes* catchPicTime;
	chronoTimes* finTime;
	std::vector<atomGrid> grids;
	// what the thread watches...
	std::atomic<bool>* cruncherThreadActive;
	// imageQueue[queuePositions][pixelNum(flattened)]
	std::vector<std::vector<long>>* imageQueue;
	// options
	bool plotterActive;
	bool plotterNeedsImages;
	bool rearrangerActive;
	UINT picsPerRep;
	// locks
	std::mutex* imageLock;
	std::mutex* plotLock;
	std::mutex* rearrangerLock;
	std::condition_variable* rearrangerConditionWatcher;
	// what the thread fills.
	// imageQueue[gridNum][queuePositions][pixelNum(flattened)]
	std::vector<std::vector<std::vector<long>>>* plotterImageQueue;
	std::vector<std::vector<std::vector<bool>>>* plotterAtomQueue;
	//
	std::vector<std::vector<bool>>* rearrangerAtomQueue;
	std::array<int, 4> thresholds;
	imageParameters imageDims;
	UINT atomThresholdForSkip = UINT_MAX;
	std::atomic<bool>* skipNext;
};
