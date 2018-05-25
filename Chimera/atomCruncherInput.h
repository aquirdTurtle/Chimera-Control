#pragma once
#include "atomGrid.h"
#include "imageParameters.h"
#include "commonTypes.h"
#include <atomic>
#include <vector>
#include <mutex>
#include "Queues.h"
#include "afxwin.h"
#include <array>


struct atomCruncherInput
{
	//
	chronoTimes* catchPicTime;
	chronoTimes* finTime;
	std::vector<atomGrid> grids;
	// what the thread watches...
	std::atomic<bool>* cruncherThreadActive;
	// imQueue[queuePositions][pixelNum(flattened)]
	imageQueue* imQueue;
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
	// imQueue[gridNum][queuePositions][pixelNum(flattened)]
	multiGridImageQueue* plotterImageQueue;
	multiGridAtomQueue* plotterAtomQueue;
	atomQueue* rearrangerAtomQueue;

	std::array<int, 4> thresholds;
	imageParameters imageDims;
	UINT atomThresholdForSkip = UINT_MAX;
	std::atomic<bool>* skipNext;
};
