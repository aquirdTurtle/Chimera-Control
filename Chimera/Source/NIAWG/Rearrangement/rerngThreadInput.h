// created by Mark O. Brown
#pragma once
#include "rerngOptionStructures.h"
#include "rerngContainer.h"
#include "rerngMoveStructures.h"
#include <chrono>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "GeneralObjects/Queues.h"

class NiawgCore;

// rerng is short for rearrange
struct rerngThreadInput{
	rerngThreadInput( unsigned gridRows, unsigned gridCols ) : flashMoves( gridRows, gridCols ), 
		noFlashMoves(gridRows, gridCols) {}
	unsigned sourceRows = 0;
	unsigned sourceCols = 0;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* pictureTimes;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* grabTimes;
	std::mutex* rerngLock;
	std::condition_variable* rerngConditionWatcher;
	std::atomic<bool>* threadActive;
	// an array of images that have been converted to bools for atom in a pixel or not..
	atomQueue* atomsQueue;
	// contains info from Niawg script about rearrangement, including the static wave. This is a pointer to avoid
	// making an extra copy of the static waveform, which is in general pretty large.
	waveInfoForm* rerngWave;
	NiawgCore* niawg;
	// stuff from the rearrangement input
	rerngGuiOptions guiOptions;
	rerngContainer<rerngMove> flashMoves;
	rerngContainer<rerngMove> noFlashMoves;
	Matrix < std::vector<double> > indvDimMoves;
};

