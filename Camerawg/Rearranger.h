#pragma once
#include "Windows.h"

struct rearrangementThreadInfo
{
	// might need links to camera, niawg

};

class Rearranger
{
	public:
		void startThread();
		bool threadIsActive();
		void rearrange();
		double getStreamingTime();
	private:
		static unsigned int __stdcall rearrangerThreadFunction(LPVOID input);
		void calculateRearrangingMoves( std::vector<std::vector<bool>> initArrangement );
		// true = active;
		bool threadState;
		std::vector<std::vector<bool>> finalState;
		// could set different thresholds for each location in the camera if expect imbalance.
		// std::vector<std::vector<int>> thresholds;
		int threshold;
};