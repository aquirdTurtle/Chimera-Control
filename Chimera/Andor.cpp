#include "stdafx.h"
#include "ATMCD32D.h"
#include "Andor.h"
#include "CameraWindow.h"
#include <chrono>
#include <process.h>
#include <algorithm>
#include <numeric>

std::string AndorCamera::getSystemInfo()
{
	std::string info;
	// can potentially get more info from this.
	//AndorCapabilities capabilities;
	//getCapabilities( capabilities );
	info += "Camera Model: " + getHeadModel() + "\n";
	int num; 
	getSerialNumber(num);
	info += "Camera Serial Number: " + str(num) + "\n";
	return info;
}


AndorCamera::AndorCamera()
{
	runSettings.emGainModeIsOn = false;
	std::string errorMessage;
	if (ANDOR_SAFEMODE)
	{
		errorMessage = "Andor Camera is in SAFEMODE: Initialization Not attempted.";
	}
	// Initialize driver in current directory
	try
	{
		initialize();
		setBaselineClamp(1);
		setBaselineOffset(0);
		setDMAParameters(1, 0.0001f);
	}
	catch (Error& err)
	{
		errBox(err.what());
	}
	/*
	//appendColoredText("Initializing......................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Get camera eCameraCapabilities
		//errorMessage = myAndor::andorErrorChecker(GetCapabilities(&eCameraCapabilities));
	}
	//eCameraCapabilities.
	//appendColoredText("Get Andor Capabilities information... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Get Head Model
		//errorMessage = myAndor::andorErrorChecker(GetHeadModel(eModel));
	}
	//appendColoredText("Get Head Model information........... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Get detector information
		//errorMessage = myAndor::andorErrorChecker(GetDetector(&eXPixels, &eYPixels));
	}
	else
	{
		//eXPixels = 512;
		//eYPixels = 512;
	}
	//appendColoredText("Get Detector information............. ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Set Vertical speed to recommended
		//GetFastestRecommendedVSSpeed(&eVerticalSpeedNumber, &speed);
		//errorMessage = myAndor::andorErrorChecker(SetVSSpeed(eVerticalSpeedNumber));
	}
	//appendColoredText("Set Vertical Speed................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	// Set Horizontal Speed to max
	STemp = 0;
	eHorizontalSpeedNumber = 0;
	eADNumber = 0;
	if (!ANDOR_SAFEMODE)
	{
		errorMessage = myAndor::andorErrorChecker(GetNumberADChannels(&nAD));
	}
	//appendColoredText("Get number AD Channel................ ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	if (!ANDOR_SAFEMODE)
	{
		errorMessage = myAndor::andorErrorChecker(SetBaselineClamp(1));
	}
	//appendColoredText("Set Baseline Clamp................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	if (!ANDOR_SAFEMODE)
	{
		errorMessage = myAndor::andorErrorChecker(SetBaselineOffset(0));
	}
	//appendColoredText("Set Baseline Offset Value............ ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Setting this makes the camera ALWAYS send an image as soon as it receives it instead of waiting a couple images before notifying the computer.
		// It does this wait if you don't set this and the images are taken very fast, e.g. < 15 ms.
		errorMessage = myAndor::andorErrorChecker(SetDMAParameters(1, 0.0001));
	}
	/*
	//appendColoredText("Set DMA Parameters................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	
	if (!(*input->plotter).is_open())
	{
		errorMessage = "GNUPLOT didn't open correctly.";
	}
	else
	{
		errorMessage = "GOOD";
	}
	//appendColoredText("Opening GNUPLOT...................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
	//				  eInitializeDialogBoxHandle);
	//initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	*/


}

void AndorCamera::initializeClass(Communicator* comm, chronoTimes* imageTimes)
{
	threadInput.comm = comm;
	threadInput.imageTimes = imageTimes;
	threadInput.Andor = this;
	threadInput.spuriousWakeupHandler = false;
	// begin the camera wait thread.
	_beginthreadex(NULL, 0, &AndorCamera::cameraThread, &threadInput, 0, &cameraThreadID);
}

void AndorCamera::updatePictureNumber( ULONGLONG newNumber )
{
	currentPictureNumber = newNumber;
}

/* 
 * pause the camera thread which watches the camera for pictures
 */
void AndorCamera::pauseThread()
{
	// andor should not be taking images anymore at this point.
	threadInput.spuriousWakeupHandler = false;
}


/*
 * this should get called when the camera finishes running. right now this is very simple.
 */
void AndorCamera::onFinish()
{
	threadInput.signaler.notify_all();
	cameraIsRunning = false;
}


/*
 * this thread watches the camera for pictuers and when it sees a picture lets the main thread know via a message. 
 * it gets initialized at the start of the program and is basically always running.
 */
unsigned __stdcall AndorCamera::cameraThread( void* voidPtr )
{
	cameraThreadInput* input = (cameraThreadInput*) voidPtr;
	//... I'm not sure what this lock is doing here... why not inside while loop?
	std::unique_lock<std::mutex> lock( input->runMutex );
	int safeModeCount = 0;
	long pictureNumber = 0;
	bool armed = false;
	while ( !input->Andor->cameraThreadExitIndicator )
	{
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
		input->signaler.wait( lock, [input, &safeModeCount ]() { return input->spuriousWakeupHandler; } );
		if ( !ANDOR_SAFEMODE )
		{
			try
			{
				int status;
				input->Andor->queryStatus(status);
				if (status == DRV_IDLE && armed)
				{
					// signal the end to the main thread.
					input->comm->sendCameraFin();
					armed = false;
				}
				else
				{
					input->Andor->waitForAcquisition();
					if ( pictureNumber % 2 == 0 )
					{
						(*input->imageTimes).push_back( std::chrono::high_resolution_clock::now( ) );
					}
					armed = true;
					try
					{
						input->Andor->getAcquisitionProgress(pictureNumber);
					}
					catch (Error& exception)
					{
						input->comm->sendError(exception.what());
					}

					input->comm->sendCameraProgress(pictureNumber);
				}
			}
			catch (Error&)
			{
				//...? When does this happen? not sure why this is here...
			}
		}
		else
		{
			// simulate an actual wait.
			//Sleep( ULONG(input->Andor->runSettings.kineticCycleTime * 1000) );
			Sleep( 100 );
			if ( pictureNumber % 2 == 0 )
			{
				(*input->imageTimes).push_back( std::chrono::high_resolution_clock::now( ) );
			}
			if ( input->Andor->cameraIsRunning && safeModeCount < input->Andor->runSettings.totalPicsInExperiment)
			{
				if ( input->Andor->runSettings.cameraMode == "Kinetic Series Mode" 
					 || input->Andor->runSettings.cameraMode == "Accumulation Mode" )
				{
					safeModeCount++;
					input->comm->sendCameraProgress( safeModeCount );
				}
				else
				{
					input->comm->sendCameraProgress( 1 );
				}
			}
			else
			{
				input->Andor->cameraIsRunning = false;
				safeModeCount = 0;
				input->comm->sendCameraFin();
				input->spuriousWakeupHandler = false;
			}
		}
	}
	return 0;
}


/*
 * Get whatever settings the camera is currently using in it's operation, assuming it's operating.
 */
AndorRunSettings AndorCamera::getSettings()
{
	return runSettings;
}

void AndorCamera::setSettings(AndorRunSettings settingsToSet)
{
	runSettings = settingsToSet;
}

void AndorCamera::setAcquisitionMode()
{
	setAcquisitionMode(runSettings.acquisitionMode);
}

/* 
	* Large function which initializes a given camera image run.
	*/
void AndorCamera::armCamera(CameraWindow* camWin, double& minKineticCycleTime)
{
	/// Set a bunch of parameters.
	// Set to 1 MHz readout rate in both cases
	setADChannel(1);
	if (runSettings.emGainModeIsOn)
	{
		setHSSpeed(0, 0);
	}
	else
	{
		setHSSpeed(1, 0);
	}
	setAcquisitionMode();
	setReadMode();
	setExposures();
	setImageParametersToCamera();
	// Set Mode-Specific Parameters
	if (runSettings.acquisitionMode == 5)
	{
		setFrameTransferMode();
	}
	else if (runSettings.acquisitionMode == 3)
	{
		setKineticCycleTime();
		setScanNumber();
		// set this to 1.
		setNumberAccumulations(true);
	}	
	else if (runSettings.acquisitionMode == 2)
	{
		setAccumulationCycleTime();
		setNumberAccumulations(false);
	}
	setGainMode();
	setCameraTriggerMode();
	// Set trigger mode.
	// check plotting parameters
	/// TODO!
	// CAREFUL! I can only modify these guys here because I'm sure that I'm also not writing to them in the plotting 
	// thread since the plotting thread hasn't
	// started yet. If moving stuff around, be careful.
	// Initialize the thread accumulation number.
	// this->??? = 1;
	// //////////////////////////////
	queryStatus();

	/// Do some plotting stuffs
	//eAlerts.setAlertThreshold();
	//ePicStats.reset();

	// the lock is released when the lock object function goes out of scope, which happens immediately after
	// the start acquisition call
	//std::lock_guard<std::mutex> lock( threadInput.runMutex );
	
	// get the min time after setting everything else.
	minKineticCycleTime = getMinKineticCycleTime( );

	cameraIsRunning = true;
	// remove the spurious wakeup check.
	threadInput.spuriousWakeupHandler = true;
	// notify the thread that the experiment has started..
	threadInput.signaler.notify_all();
	
	startAcquisition();
}


/* 
 * This function checks for new pictures, if they exist it gets them, and shapes them into the array which holds all of
 * the pictures for a given repetition.
 */
std::vector<std::vector<long>> AndorCamera::acquireImageData()
{
	try
	{
		checkForNewImages();
	}
	catch (Error& exception)
	{
		if (exception.whatBare() == "DRV_NO_NEW_DATA")
		{
			// just return this anyways.
			return imagesOfExperiment;
		}
		else
		{
			// it's an actual error, pass it up.
			throw;
		}
	}
	/// ///
	// for only one image... (each image processed from the call from a separate windows message)
	int size;
	// If there is no data the acquisition must have been aborted
	// free all allocated memory
	int experimentPictureNumber;
	if (runSettings.showPicsInRealTime)
	{
		experimentPictureNumber = 0;
	}
	else
	{
		experimentPictureNumber = (((currentPictureNumber - 1) % runSettings.totalPicsInVariation) 
								   % runSettings.picsPerRepetition);
	}

	if (experimentPictureNumber == 0)
	{
		WaitForSingleObject(imagesMutex, INFINITE);
		imagesOfExperiment.clear();
		if (runSettings.showPicsInRealTime)
		{
			imagesOfExperiment.resize(1);
		}
		else
		{
			imagesOfExperiment.resize(runSettings.picsPerRepetition);
		}
		ReleaseMutex(imagesMutex);
	}

	size = runSettings.imageSettings.width * runSettings.imageSettings.height;
	std::vector<long> tempImage;
	tempImage.resize(size);
	WaitForSingleObject(imagesMutex, INFINITE);
	imagesOfExperiment[experimentPictureNumber].resize(size);
 	if (!ANDOR_SAFEMODE)
	{
		getOldestImage(tempImage);
		// immediately rotate
		for (UINT imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			imagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc 
				% runSettings.imageSettings.width) + 1) * runSettings.imageSettings.height 
				- imageVecInc / runSettings.imageSettings.width - 1];
		}
	}
	else
	{
		// generate a fake image.
		std::vector<bool> atomSpots = { 0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
										0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,	1,	0,
										0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0
									  };
		atomSpots = { 0,0,0,
					  0,1,0,
					  0,0,0,
					  0,0,0,
					  0,0,0};
					
		for (UINT imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			tempImage[imageVecInc] = rand() % 30 + 95;
			if (!(imageVecInc >= atomSpots.size()))
			{
				if (imageVecInc == 0)
				{
					tempImage[imageVecInc] = 3;
					continue;
				}
				if (atomSpots[imageVecInc])
				{
					// can have an atom here.
					if (UINT(rand()) % 300 > imageVecInc + 50)
					{
						tempImage[imageVecInc] += 400;
					}
				}
			}
		}
		WaitForSingleObject(imagesMutex, INFINITE);
		for (UINT imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			imagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc % runSettings.imageSettings.width)
				+ 1) * runSettings.imageSettings.height - imageVecInc / runSettings.imageSettings.width - 1];
		}
		ReleaseMutex(imagesMutex);
	}
	ReleaseMutex( imagesMutex );
	return imagesOfExperiment;
}


// sets this based on internal settings object.
void AndorCamera::setCameraTriggerMode()
{
	std::string errMsg;
	int trigType;
	if (runSettings.triggerMode == "Internal Trigger")
	{
		trigType = 0;
	}
	else if (runSettings.triggerMode == "External Trigger")
	{
		trigType = 1;
	}
	else if (runSettings.triggerMode == "Start On Trigger")
	{
		trigType = 6;
	}
	setTriggerMode(trigType);
}


void AndorCamera::setTemperature()
{
	// Get the current temperature
	if (runSettings.temperatureSetting < -60 || runSettings.temperatureSetting > 25)
	{
		int answer = promptBox( "Warning: The selected temperature is outside the normal temperature range of the "
								"camera (-60 through 25 C). Proceed anyways?", MB_OKCANCEL );
		if (answer == IDCANCEL)
		{
			return;
		}
	}
	// Proceedure to initiate cooling
	changeTemperatureSetting( false );
}


void AndorCamera::setReadMode()
{
	setReadMode(runSettings.readMode);
}


void AndorCamera::setExposures()
{
	if (runSettings.exposureTimes.size() > 0 && runSettings.exposureTimes.size() <= 16)
	{
		setRingExposureTimes(runSettings.exposureTimes.size(), runSettings.exposureTimes.data());
	}
	else
	{
		thrower("ERROR: Invalid size for vector of exposure times, value of " + str(runSettings.exposureTimes.size()) + ".");
	}
}


void AndorCamera::setImageParametersToCamera()
{
	setImage(runSettings.imageSettings.verticalBinning, runSettings.imageSettings.horizontalBinning, 
			 runSettings.imageSettings.bottom, runSettings.imageSettings.top, 
			 runSettings.imageSettings.left, runSettings.imageSettings.right);
}


void AndorCamera::setKineticCycleTime()
{
	setKineticCycleTime(runSettings.kineticCycleTime);
}


void AndorCamera::setScanNumber()
{
	if (runSettings.totalPicsInExperiment == 0 && runSettings.totalPicsInVariation != 0)
	{
		// all is good. The first variable has not been set yet.
	}
	else if (runSettings.totalPicsInVariation == 0)
	{
		thrower("ERROR: Scan Number Was Zero.\r\n");
	}
	else
	{
		setNumberKinetics(int(runSettings.totalPicsInExperiment));
	}
}


void AndorCamera::setFrameTransferMode()
{
	setFrameTransferMode(runSettings.frameTransferMode);
}


/*
 * exposures should be initialized to be the correct size. Nothing else matters for the inputs, they get 
 * over-written.
 * throws exception if fails
 */
void AndorCamera::checkAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures)
{
	float tempExposure, tempAccumTime, tempKineticTime;
	float * timesArray = NULL;
	std::string errMsg;
	if (ANDOR_SAFEMODE)
	{
		// if in safemode initialize this stuff to the values to be outputted.
		if (exposures.size() > 0)
		{
			tempExposure = exposures[0];
		}
		else
		{
			tempExposure = 0;
		}
		tempAccumTime = accumulation;
		tempKineticTime = kinetic;
	}
	else
	{
		tempExposure = 0;
		tempAccumTime = 0;
		tempKineticTime = 0;
	}
	// It is necessary to get the actual times as the system will calculate the
	// nearest possible time. eg if you set exposure time to be 0, the system
	// will use the closest value (around 0.01s)
	timesArray = new float[exposures.size()];
	if (ANDOR_SAFEMODE)
	{
		getAcquisitionTimes(tempExposure, tempAccumTime, tempKineticTime);
		getAdjustedRingExposureTimes(exposures.size(), timesArray);
	}
	else 
	{
		for (UINT exposureInc = 0; exposureInc < exposures.size(); exposureInc++)
		{
			timesArray[exposureInc] = exposures[exposureInc];
		}
	}
	// Set times
	if (exposures.size() > 0)
	{
		for (UINT exposureInc = 0; exposureInc < exposures.size(); exposureInc++)
		{
			exposures[exposureInc] = timesArray[exposureInc];
		}
		delete[] timesArray;
	}
	accumulation = tempAccumTime;
	kinetic = tempKineticTime;
}
	

/*
 (
 */
void AndorCamera::setAccumulationCycleTime()
{
	setAccumulationCycleTime(runSettings.accumulationTime);
}


void AndorCamera::setNumberAccumulations(bool isKinetic)
{
	std::string errMsg;
	if (isKinetic)
	{
		// right now, kinetic series mode always has one accumulation. could add this feature later if desired.
		//setNumberAccumulations(true); // ???
		setAccumulationNumber(1);
	}
	else
	{
		// ???
		// setNumberAccumulations(false); // ???
		setAccumulationNumber(runSettings.accumulationNumber);
	}
}


void AndorCamera::setGainMode()
{
	if (!runSettings.emGainModeIsOn)
	{
		// Set Gain
		int numGain;
		getNumberOfPreAmpGains(numGain);
		setPreAmpGain(2);
		float myGain;
		getPreAmpGain(2, myGain);
		// 1 is for conventional gain mode.
		setOutputAmplifier(1);
	}
	else
	{
		// 0 is for em gain mode.
		setOutputAmplifier(0);
		setPreAmpGain(2);
		if (runSettings.emGainLevel > 300)
		{
			setEmGainSettingsAdvanced(1);
		}
		else
		{
			setEmGainSettingsAdvanced(0);
		}
		setEmCcdGain(runSettings.emGainLevel);
	}
}


void AndorCamera::changeTemperatureSetting(bool turnTemperatureControlOff)
{
	char aBuffer[256];
	int minimumAllowedTemp, maximumAllowedTemp;
	// the default, in case the program is in safemode.
	minimumAllowedTemp = -60;
	maximumAllowedTemp = 25;
	// clear buffer
	wsprintf(aBuffer, "");
	// check if temp is in valid range
	getTemperatureRange(minimumAllowedTemp, maximumAllowedTemp);
	if (runSettings.temperatureSetting < minimumAllowedTemp || runSettings.temperatureSetting > maximumAllowedTemp)
	{
		thrower("ERROR: Temperature is out of range\r\n");
	}
	else
	{
		// if it is in range, switch on cooler and set temp
		if (turnTemperatureControlOff == false)
		{
			temperatureControlOn();
		}
		else
		{
			temperatureControlOff();
		}
	}

	// ???
	/*
	eCooler = TRUE;
	SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
	*/
	if (turnTemperatureControlOff == false)
	{
		setTemperature(runSettings.temperatureSetting);
	}
	else
	{
		thrower("Temperature Control has been turned off.\r\n");
	}
}

/*
 *
 */
void AndorCamera::andorErrorChecker(int errorCode)
{
	std::string errorMessage = "uninitialized";
	switch (errorCode)
	{
		case 20001:
		{
			errorMessage = "DRV_ERROR_CODES";
			break;
		}
		case 20002:
		{
			errorMessage = "DRV_SUCCESS";
			break;
		}
		case 20003:
		{
			errorMessage = "DRV_VXDNOTINSTALLED";
			break;
		}
		case 20004:
		{
			errorMessage = "DRV_ERROR_SCAN";
			break;
		}
		case 20005:
		{
			errorMessage = "DRV_ERROR_CHECK_SUM";
			break;
		}
		case 20006:
		{
			errorMessage = "DRV_ERROR_FILELOAD";
			break;
		}
		case 20007:
		{
			errorMessage = "DRV_UNKNOWN_FUNCTION";
			break;
		}
		case 20008:
		{
			errorMessage = "DRV_ERROR_VXD_INIT";
			break;
		}
		case 20009:
		{
			errorMessage = "DRV_ERROR_ADDRESS";
			break;
		}
		case 20010:
		{
			errorMessage = "DRV_ERROR_PAGELOCK";
			break;
		}
		case 20011:
		{
			errorMessage = "DRV_ERROR_PAGE_UNLOCK";
			break;
		}
		case 20012:
		{
			errorMessage = "DRV_ERROR_BOARDTEST";
			break;
		}
		case 20013:
		{
			errorMessage = "DRV_ERROR_ACK";
			break;
		}
		case 20014:
		{
			errorMessage = "DRV_ERROR_UP_FIFO";
			break;
		}
		case 20015:
		{
			errorMessage = "DRV_ERROR_PATTERN";
			break;
		}
		case 20017:
		{
			errorMessage = "DRV_ACQUISITION_ERRORS";
			break;
		}
		case 20018:
		{
			errorMessage = "DRV_ACQ_BUFFER";
			break;
		}
		case 20019:
		{
			errorMessage = "DRV_ACQ_DOWNFIFO_FULL";
			break;
		}
		case 20020:
		{
			errorMessage = "DRV_PROC_UNKNOWN_INSTRUCTION";
			break;
		}
		case 20021:
		{
			errorMessage = "DRV_ILLEGAL_OP_CODE";
			break;
		}
		case 20022:
		{
			errorMessage = "DRV_KINETIC_TIME_NOT_MET";
			break;
		}
		case 20023:
		{
			errorMessage = "DRV_KINETIC_TIME_NOT_MET";
			break;
		}
		case 20024:
		{
			errorMessage = "DRV_NO_NEW_DATA";
			break;
		}
		case 20026:
		{
			errorMessage = "DRV_SPOOLERROR";
			break;
		}
		case 20033:
		{
			errorMessage = "DRV_TEMPERATURE_CODES";
			break;
		}
		case 20034:
		{
			errorMessage = "DRV_TEMPERATURE_OFF";
			break;
		}
		case 20035:
		{
			errorMessage = "DRV_TEMPERATURE_NOT_STABILIZED";
			break;
		}
		case 20036:
		{
			errorMessage = "DRV_TEMPERATURE_STABILIZED";
			break;
		}
		case 20037:
		{
			errorMessage = "DRV_TEMPERATURE_NOT_REACHED";
			break;
		}
		case 20038:
		{
			errorMessage = "DRV_TEMPERATURE_OUT_RANGE";
			break;
		}
		case 20039:
		{
			errorMessage = "DRV_TEMPERATURE_NOT_SUPPORTED";
			break;
		}
		case 20040:
		{
			errorMessage = "DRV_TEMPERATURE_DRIFT";
			break;
		}
		case 20049:
		{
			errorMessage = "DRV_GENERAL_ERRORS";
			break;
		}
		case 20050:
		{
			errorMessage = "DRV_INVALID_AUX";
			break;
		}
		case 20051:
		{
			errorMessage = "DRV_COF_NOTLOADED";
			break;
		}
		case 20052:
		{
			errorMessage = "DRV_FPGAPROG";
			break;
		}
		case 20053:
		{
			errorMessage = "DRV_FLEXERROR";
			break;
		}
		case 20054:
		{
			errorMessage = "DRV_GPIBERROR";
			break;
		}
		case 20064:
		{
			errorMessage = "DRV_DATATYPE";
			break;
		}
		case 20065:
		{
			errorMessage = "DRV_DRIVER_ERRORS";
			break;
		}
		case 20066:
		{
			errorMessage = "DRV_P1INVALID";
			break;
		}
		case 20067:
		{
			errorMessage = "DRV_P2INVALID";
			break;
		}
		case 20068:
		{
			errorMessage = "DRV_P3INVALID";
			break;
		}
		case 20069:
		{
			errorMessage = "DRV_P4INVALID";
			break;
		}
		case 20070:
		{
			errorMessage = "DRV_INIERROR";
			break;
		}
		case 20071:
		{
			errorMessage = "DRV_COFERROR";
			break;
		}
		case 20072:
		{
			errorMessage = "DRV_ACQUIRING";
			break;
		}
		case 20073:
		{
			errorMessage = "DRV_IDLE";
			break;
		}
		case 20074:
		{
			errorMessage = "DRV_TEMPCYCLE";
			break;
		}
		case 20075:
		{
			errorMessage = "DRV_NOT_INITIALIZED";
			break;
		}
		case 20076:
		{
			errorMessage = "DRV_P5INVALID";
			break;
		}
		case 20077:
		{
			errorMessage = "DRV_P6INVALID";
			break;
		}
		case 20078:
		{
			errorMessage = "DRV_INVALID_MODE";
			break;
		}
		case 20079:
		{
			errorMessage = "DRV_INVALID_FILTER";
			break;
		}
		case 20080:
		{
			errorMessage = "DRV_I2CERRORS";
			break;
		}
		case 20081:
		{
			errorMessage = "DRV_DRV_ICDEVNOTFOUND";
			break;
		}
		case 20082:
		{
			errorMessage = "DRV_I2CTIMEOUT";
			break;
		}
		case 20083:
		{
			errorMessage = "DRV_P7INVALID";
			break;
		}
		case 20089:
		{
			errorMessage = "DRV_USBERROR";
			break;
		}
		case 20090:
		{
			errorMessage = "DRV_IOCERROR";
			break;
		}
		case 20091:
		{
			errorMessage = "DRV_NOT_SUPPORTED";
			break;
		}
		case 20093:
		{
			errorMessage = "DRV_USB_INTERRUPT_ENDPOINT_ERROR";
			break;
		}
		case 20094:
		{
			errorMessage = "DRV_RANDOM_TRACK_ERROR";
			break;
		}
		case 20095:
		{
			errorMessage = "DRV_INVALID_tRIGGER_MODE";
			break;
		}
		case 20096:
		{
			errorMessage = "DRV_LOAD_FIRMWARE_ERROR";
			break;
		}
		case 20097:
		{
			errorMessage = "DRV_DIVIDE_BY_ZERO_ERROR";
			break;
		}
		case 20098:
		{
			errorMessage = "DRV_INVALID_RINGEXPOSURES";
			break;
		}
		case 20099:
		{
			errorMessage = "DRV_BINNING_ERROR";
			break;
		}
		case 20100:
		{
			errorMessage = "DRV_INVALID_AMPLIFIER";
			break;
		}
		case 20115:
		{
			errorMessage = "DRV_ERROR_MAP";
			break;
		}
		case 20116:
		{
			errorMessage = "DRV_ERROR_UNMAP";
			break;
		}
		case 20117:
		{
			errorMessage = "DRV_ERROR_MDL";
			break;
		}
		case 20118:
		{
			errorMessage = "DRV_ERROR_UNMDL";
			break;
		}
		case 20119:
		{
			errorMessage = "DRV_ERROR_BUFSIZE";
			break;
		}
		case 20121:
		{
			errorMessage = "DRV_ERROR_NOHANDLE";
			break;
		}
		case 20130:
		{
			errorMessage = "DRV_GATING_NOT_AVAILABLE";
			break;
		}
		case 20131:
		{
			errorMessage = "DRV_FPGA_VOLTAGE_ERROR";
			break;
		}
		case 20990:
		{
			errorMessage = "DRV_ERROR_NOCAMERA";
			break;
		}
		case 20991:
		{
			errorMessage = "DRV_NOT_SUPPORTED";
			break;
		}
		case 20992:
		{
			errorMessage = "DRV_NOT_AVAILABLE";
			break;
		}
		default:
		{
			errorMessage = "UNKNOWN ERROR MESSAGE RETURNED FROM CAMERA FUNCTION!";
			break;
		}
	}
	/// So no throw is considered success.
	if (errorMessage != "DRV_SUCCESS")
	{
		thrower( errorMessage );
	}
}

/// ANDOR SDK WRAPPERS
// the following functions are wrapped to throw errors if error are returned by the raw functions, as well as to only 
// excecute the raw functions if the camera is not in safemode.

void AndorCamera::initialize()
{
	char aBuffer[256];
	// Look in current working directory for driver files
	GetCurrentDirectory(256, aBuffer);
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(Initialize(aBuffer));
	}
}

void AndorCamera::setBaselineClamp(int clamp)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetBaselineClamp(clamp));
	}
}

void AndorCamera::setBaselineOffset(int offset)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetBaselineOffset(offset));
	}
}

void AndorCamera::setDMAParameters(int maxImagesPerDMA, float secondsPerDMA)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetDMAParameters(maxImagesPerDMA, secondsPerDMA));
	}
}


void AndorCamera::waitForAcquisition()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(WaitForAcquisition());
	}
}


void AndorCamera::getTemperature(int& temp)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetTemperature(&temp));
	}
}

//
void AndorCamera::getAdjustedRingExposureTimes(int size, float* timesArray)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetAdjustedRingExposureTimes(size, timesArray));
	}
}


void AndorCamera::setNumberKinetics(int number)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetNumberKinetics(number));
	}

}


// Andor Wrappers
void AndorCamera::getTemperatureRange(int& min, int& max)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetTemperatureRange(&min, &max));
	}
}


void AndorCamera::temperatureControlOn()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(CoolerON());
	}
}


void AndorCamera::temperatureControlOff()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(CoolerOFF());
	}
}


void AndorCamera::setTemperature(int temp)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetTemperature(temp));
	}
}


void AndorCamera::setADChannel(int channel)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetADChannel(channel));
	}
}


void AndorCamera::setHSSpeed(int type, int index)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetHSSpeed(type, index));
	}
}

// note that the function used here could be used to get actual information about the number of images, I just only use
// it to check whether there are any new images or not. Not sure if this is the smartest way to do this.
void AndorCamera::checkForNewImages()
{
	long first, last;
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetNumberNewImages(&first, &last));
	}
	// don't do anything with the info.
}


void AndorCamera::getOldestImage(std::vector<long>& dataArray)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetOldestImage(dataArray.data(), dataArray.size()));
	}
}


void AndorCamera::setTriggerMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetTriggerMode(mode));
	}
}


void AndorCamera::setAcquisitionMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetAcquisitionMode(mode));
	}
}


void AndorCamera::setReadMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetReadMode(mode));
	}
}


void AndorCamera::setRingExposureTimes(int sizeOfTimesArray, float* arrayOfTimes)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetRingExposureTimes(sizeOfTimesArray, arrayOfTimes));
	}
}


void AndorCamera::setImage(int hBin, int vBin, int lBorder, int rBorder, int tBorder, int bBorder)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetImage(hBin, vBin, lBorder, rBorder, tBorder, bBorder));
	}
}


void AndorCamera::setKineticCycleTime(float cycleTime)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetKineticCycleTime(cycleTime));
	}
}


void AndorCamera::setFrameTransferMode(int mode)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetFrameTransferMode(mode));
	}
}

double AndorCamera::getMinKineticCycleTime( )
{
	// get the currently set kinetic cycle time.
	float minKineticCycleTime, dummy1, dummy2;	
	setKineticCycleTime( 0 );
	getAcquisitionTimes( dummy1, dummy2, minKineticCycleTime );
	// re-set whatever's currently in the settings.
	setKineticCycleTime( );
	return minKineticCycleTime;
}


void AndorCamera::getAcquisitionTimes(float& exposure, float& accumulation, float& kinetic)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetAcquisitionTimings(&exposure, &accumulation, &kinetic));
	}
}

/*
*/
void AndorCamera::queryStatus()
{
	int status;
	queryStatus(status);
	if (ANDOR_SAFEMODE)
	{
		status = DRV_IDLE;
	}
	if (status != DRV_IDLE)
	{
		thrower("ERROR: You tried to start the camera, but the camera was not idle! Camera was in state corresponding to "
				+ str(status) + "\r\n");
	}
}

void AndorCamera::setIsRunningState(bool state)
{
	cameraIsRunning = state;
}


void AndorCamera::queryStatus(int& status)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetStatus(&status));
	}
}


void AndorCamera::startAcquisition()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(StartAcquisition());
	}
}


void AndorCamera::abortAcquisition()
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(AbortAcquisition());
	}
}


void AndorCamera::setAccumulationCycleTime(float time)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetAccumulationCycleTime(time));
	}
}


void AndorCamera::setAccumulationNumber(int number)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetNumberAccumulations(number));
	}
}


void AndorCamera::getNumberOfPreAmpGains(int& number)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetNumberPreAmpGains(&number));
	}
}


void AndorCamera::setPreAmpGain(int index)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetPreAmpGain(index));
	}
}


void AndorCamera::getPreAmpGain(int index, float& gain)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetPreAmpGain(index, &gain));
	}
}


void AndorCamera::setOutputAmplifier(int type)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetOutputAmplifier(type));
	}
}


void AndorCamera::setEmGainSettingsAdvanced(int state)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetEMAdvanced(state));
	}
}


void AndorCamera::setEmCcdGain(int gain)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(SetEMCCDGain(gain));
	}
}

///
bool AndorCamera::isRunning()
{
	return cameraIsRunning;
}


/*
* the input here will store how many whole pictures (not accumulations) have been taken.
*/
void AndorCamera::getAcquisitionProgress(long& seriesNumber)
{
	if (!ANDOR_SAFEMODE)
	{
		long dummyAccumulationNumber;
		andorErrorChecker(GetAcquisitionProgress(&dummyAccumulationNumber, &seriesNumber));
	}
}

/*
* overload to get both the acccumulation progress and the whole picture progress.
*/
void AndorCamera::getAcquisitionProgress(long& accumulationNumber, long& seriesNumber)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetAcquisitionProgress(&accumulationNumber, &seriesNumber));
	}
}


void AndorCamera::getCapabilities(AndorCapabilities& caps)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker( GetCapabilities( &caps ) );
	}
}

void AndorCamera::getSerialNumber(int& num)
{
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker( GetCameraSerialNumber( &num ) );
	}
}

std::string AndorCamera::getHeadModel()
{
	char nameChars[1024];
	if (!ANDOR_SAFEMODE)
	{
		andorErrorChecker(GetHeadModel( nameChars ));
	}
	else
	{
		return "safemode";
	}
	return str(nameChars);
}


