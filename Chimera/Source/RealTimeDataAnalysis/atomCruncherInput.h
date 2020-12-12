// created by Mark O. Brown
#pragma once
#include "atomGrid.h"
#include "GeneralImaging/imageParameters.h"
#include "GeneralObjects/commonTypes.h"
#include <atomic>
#include <vector>
#include <mutex>
#include "GeneralObjects/Queues.h"
#include <array>


struct atomCruncherInput
{
	// timing info is stored in these.
	chronoTimes* catchPicTime;
	chronoTimes* finTime;
	// instructions.
	std::vector<atomGrid> grids;
	// the thread watches this to know when to quit.
	std::atomic<bool>* cruncherThreadActive;
	// imQueue[queuePositions][pixelNum(flattened)]
	//imageQueue* imQueue;
	// options
	bool plotterActive;
	bool plotterNeedsImages;
	bool rearrangerActive;
	unsigned picsPerRep;
	unsigned atomThresholdForSkip = UINT_MAX;
	// outer vector here is for each location in the first grid.
	std::array<std::vector<int>, 4> thresholds;
	imageParameters imageDims;
	// locks
	//std::mutex* imageQueueLock;
	//std::mutex* plotLock;
	//std::mutex* rearrangerLock;
	std::condition_variable* rearrangerConditionWatcher;
	// what the thread fills.
	//multiGridImageQueue* plotterImageQueue;
	//multiGridAtomQueue* plotterAtomQueue;
	//atomQueue* rearrangerAtomQueue;
	std::atomic<bool>* skipNext;
};
