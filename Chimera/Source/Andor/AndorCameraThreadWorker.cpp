#include "stdafx.h"
#include "Andor/AndorCameraThreadWorker.h"
#include <Andor/AndorCameraCore.h>

AndorCameraThreadWorker::AndorCameraThreadWorker (cameraThreadInput* input_){
	input = input_;
}

AndorCameraThreadWorker::~AndorCameraThreadWorker () {
}

void AndorCameraThreadWorker::process (){
	//... I'm not sure what this lock is doing here... why not inside while loop?
	int safeModeCount = 0;
	long pictureNumber = 0;
	bool armed = false;
	std::unique_lock<std::timed_mutex> lock (*input->runMutex, std::chrono::milliseconds (1000));
	if (!lock.owns_lock ()) {
		errBox ("ERROR: ANDOR IMAGING THREAD FAILED TO LOCK THE RUN MUTEX! IMAGING THREAD CLOSING!");
	}
	while (!input->Andor->cameraThreadExitIndicator){
		/*
		 * wait until unlocked. this happens when data is started.
		 * the first argument is the lock.  The when the lock is locked, this function just sits and doesn't use cpu,
		 * unlike a while(gGlobalCheck){} loop that waits for gGlobalCheck to be set. The second argument here is a
		 * lambda, more or less a quick inline function that doesn't in this case have a name. This handles something
		 * called spurious wakeups, which are weird and appear to relate to some optimization things from the quick
		 * search I did. Regardless, I don't fully understand why spurious wakeups occur, but this protects against
		 * them.
		 */
		 // Also, anytime this gets locked, the count should be reset.
		 //input->signaler.wait( lock, [input]() { return input->expectingAcquisition; } );
		while (!input->expectingAcquisition) {
			input->signaler.wait (lock);
		}

		if (!input->safemode){
			try	{
				int status = input->Andor->flume.queryStatus ();
				if (status == DRV_IDLE && armed) {
					// get the last picture. acquisition is over so getAcquisitionProgress returns 0.
					if (input->Andor->isCalibrating ()) {
						//input->comm->sendCameraCalProgress (-1);
						// signal the end to the main thread.
						//input->comm->sendCameraCalFin ();
						armed = false;
					}
					else {
						//emit pictureTaken (-1);
						emit acquisitionFinished ();
						// make sure the thread waits when it hits the condition variable.
						//input->expectingAcquisition = false;
						armed = false;
					}
				}
				else{
					input->Andor->flume.waitForAcquisition ();
					if (pictureNumber % 2 == 0) {
						(*input->imageTimes).push_back (std::chrono::high_resolution_clock::now ());
					}
					armed = true;
					try {
						input->Andor->flume.getAcquisitionProgress (pictureNumber);
					}
					catch (ChimeraError& exception) {
						//input->comm->sendError (exception.trace ());
					}
					if (input->Andor->isCalibrating ()) {
						//input->comm->sendCameraCalProgress (pictureNumber);
					}
					else {
						emit pictureTaken (pictureNumber);
						//input->comm->sendCameraProgress (pictureNumber);
					}
				}
			}
			catch (ChimeraError&) {
				//...? When does this happen? not sure why this is here...
			}
		}
		else { // safemode
			// simulate an actual wait.
			Sleep (500);
			if (pictureNumber % 2 == 0) {
				(*input->imageTimes).push_back (std::chrono::high_resolution_clock::now ());
			}
			if (input->Andor->cameraIsRunning && safeModeCount < input->Andor->runSettings.totalPicsInExperiment ()) {
				if (input->Andor->runSettings.acquisitionMode == AndorRunModes::mode::Kinetic) {
					safeModeCount++;
					if (input->Andor->isCalibrating ()) {
						//input->comm->sendCameraCalProgress (safeModeCount);
					}
					else {
						emit pictureTaken (safeModeCount);
						//input->comm->sendCameraProgress (safeModeCount);
					}
				}
				else {
					if (input->Andor->isCalibrating ()) {
						//input->comm->sendCameraCalProgress (1);
					}
					else {
						emit pictureTaken (1);
						//input->comm->sendCameraProgress (1);
					}
				}
			}
			else{
				input->Andor->cameraIsRunning = false;
				safeModeCount = 0;
				if (input->Andor->isCalibrating ()) {
					//input->comm->sendCameraCalFin ();
				}
				else {
					emit acquisitionFinished ();
					//input->comm->sendCameraFin ();
				}
				input->expectingAcquisition = false;
			}
		}
	}
}

