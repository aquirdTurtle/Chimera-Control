#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include "Windows.h"
#include <array>
#include "atomGrid.h"
#include "imageParameters.h"


struct atomCruncherInput
{
	//
	chronoTimes* catchPicTime;
	chronoTimes* finTime;
	atomGrid gridInfo;
	// what the thread watches...
	std::atomic<bool>* cruncherThreadActive;
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
	std::vector<std::vector<long>>* plotterImageQueue;
	std::vector<std::vector<bool>>* plotterAtomQueue;
	std::vector<std::vector<bool>>* rearrangerAtomQueue;
	std::array<int, 4> thresholds;
	imageParameters imageDims;
	UINT atomThresholdForSkip = UINT_MAX;
	std::atomic<bool>* skipNext;
};
