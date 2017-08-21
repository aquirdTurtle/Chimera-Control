#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include "Windows.h"
#include <array>

struct atomCruncherInput
{
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
	// what the thread fills.
	std::vector<std::vector<long>>* plotterImageQueue;
	std::vector<std::vector<bool>>* plotterAtomQueue;
	std::vector<std::vector<bool>>* rearrangerAtomQueue;
	std::array<int, 4> thresholds;
};
