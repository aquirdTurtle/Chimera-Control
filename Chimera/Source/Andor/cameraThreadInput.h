#pragma once

#include <memory>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>

class AndorCameraCore;

struct cameraThreadInput{
	bool expectingAcquisition;
	std::timed_mutex* runMutex;
	std::condition_variable_any signaler;
	// Andor is set to this in the constructor of the andor camera.
	AndorCameraCore* Andor;
	bool safemode;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>* imageTimes;
};
