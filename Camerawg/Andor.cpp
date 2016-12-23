#include "stdafx.h"
#include "Andor.h"
//#include "plotThread.h"
#include <process.h>
//#include "plotterProcedure.h"
#include <algorithm>
#include <numeric>
#include "atmcd32d.h"
#include "CameraWindow.h"

AndorCamera::AndorCamera()
{
	this->runSettings.emGainModeIsOn = false;
}
void AndorCamera::confirmAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures, Communicator* comm)
{
	// not sure if this function is necessary...
}

/*
 * Get whatever settings the camera is currently using in it's operation, assuming it's operating.
 */
AndorRunSettings AndorCamera::getSettings()
{
	return this->runSettings;
}

void AndorCamera::setSettings(AndorRunSettings settingsToSet)
{
	this->runSettings = settingsToSet;
	return;
}

/* 
	* Large function which initializes a given camera image run.
	*/
int AndorCamera::setSystem(CameraWindow* camWin, Communicator* comm)
{
	/// Set a bunch of parameters.
	std::string errorMessage;
	// Set to 1 MHz readout rate in both cases
	if (this->runSettings.emGainModeIsOn)
	{
		if (!ANDOR_SAFEMODE)
		{
			errorMessage = this->andorErrorChecker(SetADChannel(1));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetADChannel Error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			errorMessage = this->andorErrorChecker(SetHSSpeed(0, 0));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetHSSpeed Error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}

		}
	}
	else
	{
		if (!ANDOR_SAFEMODE)
		{
			errorMessage = this->andorErrorChecker(SetADChannel(1));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetADChannel Error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			errorMessage = this->andorErrorChecker(SetHSSpeed(1, 0));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetHSSpeed Error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
		}
	}

	if (this->setAcquisitionMode(comm) != 0)
	{
		return -1;
	}
	if (this->setReadMode(comm) != 0)
	{
		return -1;
	}
	if (this->setExposures(comm) != 0)
	{
		return -1;
	}
	if (this->setImageParametersToCamera(comm) != 0)
	{
		return -1;
	}
	// Set Mode-Specific Parameters
	if (runSettings.acquisitionMode == 5)
	{
		if (this->setFrameTransferMode(0) != 0)
		{
			return -1;
		}
	}
	else if (runSettings.acquisitionMode == 3)
	{
		if (this->setKineticCycleTime(comm) != 0)
		{
			return -1;
		}
		if (this->setScanNumber(comm) != 0)
		{
			this->cameraIsRunning = false;
			return -1;
		}
		// set this to 1.
		if (this->setNumberAccumulations(true, comm) != 0)
		{
			return -1;
		}
	}
	else if (runSettings.acquisitionMode == 2)
	{
		if (this->setAccumulationCycleTime(comm) != 0)
		{
			return -1;
		}
		if (this->setNumberAccumulations(false, comm) != 0)
		{
			return -1;
		}
	}
	try
	{
		this->confirmAcquisitionTimings(runSettings.kinetiCycleTime, runSettings.accumulationTime, 
										runSettings.exposureTimes, comm);
	}
	catch (std::runtime_error)
	{
		comm->sendError("ERROR: Unable to check acquisition timings.\r\n", "", "R");
		throw;
	}

	if (this->setGainMode(comm) != 0)
	{
		return -1;
	}
	// Set trigger mode.
	if (this->setTriggerMode(comm) != 0)
	{
		return -1;
	}
	// check plotting parameters
	/// TODO!
	// CAREFUL! I can only modify these guys here because I'm sure that I'm also not writing to them in the plotting thread since the plotting thread hasn't
	// started yet. If moving stuff around, be careful.
	// Initialize the thread accumulation number.
	// this->??? = 1;
	// //////////////////////////////
	
	if (this->getStatus(comm) != 0)
	{
		return -1;
	}

	/// setup fits files
	std::string errMsg;
	if (runSettings.cameraMode != "Continuous Single Scans Mode")
	{
		/// TODO: also, change to HDF5
		/*
		if (eExperimentData.initializeDataFiles(errMsg))
		{
			appendText(errMsg, IDC_ERROR_EDIT);
			return -1;
		}
		*/
	}
	/// Do some plotting stuffs
	//eAlerts.setAlertThreshold();

	// set default colors and linewidths on plots
	this->currentRepetitionNumber = 1;
	// Create the Mutex. This function just opens the mutex if it already exists.
	plottingMutex = CreateMutexEx(0, NULL, FALSE, MUTEX_ALL_ACCESS);
	// prepare for start of thread.
	plotThreadExitIndicator = true;
	/// TODO!
	// start thread.
	/*
	unsigned int * plottingThreadID = NULL;
	std::vector<std::string>* argPlotNames;
	argPlotNames = new std::vector<std::string>;
	for (int plotNameInc = 0; plotNameInc < eCurrentPlotNames.size(); plotNameInc++)
	{
		argPlotNames->push_back(eCurrentPlotNames[plotNameInc]);
	}
	// clear this before starting the thread.
	eImageVecQueue.clear();
	// start the plotting thread.
	ePlottingThreadHandle = (HANDLE)_beginthreadex(0, 0, plotterProcedure, argPlotNames, 0,
													plottingThreadID);
													*/
	// color pictures, clearing last run.
	/// TODO!
	/*
	HDC hDC = camWin->GetDC();

	SelectObject(hDC, GetStockObject(DC_BRUSH));
	SelectObject(hDC, GetStockObject(DC_PEN));
	// dark green brush
	SetDCBrushColor(hDC, RGB(0, 10, 0));
	// Set the Pen to White
	SetDCPenColor(hDC, RGB(255, 255, 255));
	// Drawing a rectangle with the current Device Context
	for (int imageInc = 0; imageInc < eImageBackgroundAreas.size(); imageInc++)
	{
		// slightly larger than the image zone.
		Rectangle(hDC, eImageBackgroundAreas[imageInc].left - 5, eImageBackgroundAreas[imageInc].top - 5, eImageBackgroundAreas[imageInc].right + 5, eImageBackgroundAreas[imageInc].bottom + 5);
	}
	ReleaseDC(eCameraWindowHandle, hDC);
	*/
	// clear the plot grid
	/*
	ePicStats.reset();
	if (this->startAcquisition() != 0)
	{
		return -1;
	}
	// Else started!
	UpdateWindow(eStatusEditHandle.hwnd);
	*/
	return 0;
}

// This function queries the camera for how many pictures are available, retrieves all of them, then paints them to the main window. It returns the success of
// this operation.
BOOL AndorCamera::acquireImageData(Communicator* comm)
{
	long first, last;
	// check the number of images available
	std::string errMsg;

	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(GetNumberNewImages(&first, &last));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
		first = 0;
		last = 0;
	}
	// check success.
	if (errMsg != "DRV_SUCCESS")
	{
		if (errMsg == "DRV_NO_NEW_DATA")
		{
			return TRUE;
		}
		else
		{
			comm->sendError("ERROR: GetNumberNewImages error: " + errMsg + "\r\n", "", "R");
			return FALSE;
		}
	}
	/// ///
	// for only one image... (each image processed from the call from a separate windows message)
	int size;
	// If there is no data the acquisition must have been aborted
	// free all allocated memory
	int experimentPictureNumber;
	if (this->runSettings.showPicsInRealTime)
	{
		experimentPictureNumber = 0;
	}
	else
	{
		experimentPictureNumber = (((this->currentRepetitionNumber - 1) 
						% runSettings.totalPicsInVariation) % runSettings.picsPerRepetition);
	}
	if (experimentPictureNumber == 0)
	{
		WaitForSingleObject(imagesMutex, INFINITE);
		imagesOfExperiment.clear();
		if (this->runSettings.showPicsInRealTime)
		{
			imagesOfExperiment.resize(1);
		}
		else
		{
			imagesOfExperiment.resize(this->runSettings.picsPerRepetition);
		}
		ReleaseMutex(imagesMutex);
	}

	size = runSettings.imageSettings.width * runSettings.imageSettings.height;
	std::vector<long> tempImage;
	tempImage.resize(size);
	WaitForSingleObject(imagesMutex, INFINITE);
	imagesOfExperiment[experimentPictureNumber].resize(size);
	ReleaseMutex(imagesMutex);
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(GetOldestImage(&tempImage[0], tempImage.size()));
		// immediately rotate
		WaitForSingleObject(imagesMutex, INFINITE);
		for (int imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			imagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc 
				% runSettings.imageSettings.width) + 1) * runSettings.imageSettings.height 
				- imageVecInc / runSettings.imageSettings.width - 1];
		}
		ReleaseMutex(imagesMutex);
	}
	else
	{
		errMsg = "DRV_SUCCESS";
		for (int imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			tempImage[imageVecInc] = rand() % 50 + 95;
			if (experimentPictureNumber == 0)
			{
				tempImage[0] = 1000;
			}
			else
			{
				if (rand() % 2 == 0)
				{
					tempImage[0] = 0;
				}
				else
				{
					tempImage[0] = 1000;
				}
			}
		}
		WaitForSingleObject(imagesMutex, INFINITE);
		for (int imageVecInc = 0; imageVecInc < imagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
		{
			imagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc % runSettings.imageSettings.width)
				+ 1) * runSettings.imageSettings.height - imageVecInc / runSettings.imageSettings.width - 1];
		}
		ReleaseMutex(imagesMutex);

	}
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Acquisition Error on Retrieval: " + errMsg + "\r\n", "", "R");
		return FALSE;
	}
	// ???
	// eDataExists = true;
	// Display data and query max data value to be displayed in status box
	BOOL bRetValue = TRUE;
	long maxValue = 1;
	long minValue = 65536;
	if (imagesOfExperiment[experimentPictureNumber].size() != 0)
	{
		// Find max value and scale data to fill rect
		for (int pixelInc = 0; pixelInc < runSettings.imageSettings.width * runSettings.imageSettings.height; pixelInc++)
		{
			if (imagesOfExperiment[experimentPictureNumber][pixelInc] > maxValue)
			{
				maxValue = imagesOfExperiment[experimentPictureNumber][pixelInc];
			}
			if (imagesOfExperiment[experimentPictureNumber][pixelInc] < minValue)
			{
				minValue = imagesOfExperiment[experimentPictureNumber][pixelInc];
			}
		}
		if (maxValue == minValue)
		{
			return FALSE;
		}
		// update the picture
		if (experimentPictureNumber == this->runSettings.picsPerRepetition - 1 
			|| this->runSettings.showPicsInRealTime)
		{
			this->drawDataWindow();
		}
		// Wait until eImageVecQueue is available using the mutex.
		DWORD mutexMsg = WaitForSingleObject(plottingMutex, INFINITE);
		switch (mutexMsg)
		{
			case WAIT_OBJECT_0:
			{
				// Add data to the plotting queue, only if actually plotting something.
				/// TODO
				/*
				if (eCurrentPlotNames.size() != 0)
				{
					eImageVecQueue.push_back(eImagesOfExperiment[experimentPictureNumber]);
				}
				*/
				break;
			}
			case WAIT_ABANDONED:
			{
				// handle error...
				comm->sendError("ERROR: waiting for the plotting mutex failed (Wait Abandoned)!\r\n", "", "R");
				break;
			}
			case WAIT_TIMEOUT:
			{
				// handle error...
				comm->sendError("ERROR: waiting for the plotting mutex failed (timout???)!\r\n", "", "R");
				break;
			}
			case WAIT_FAILED:
			{
				// handle error...
				int a = GetLastError();
				comm->sendError("ERROR: waiting for the plotting mutex failed (Wait Failed: " + std::to_string(a) + ")!\r\n", "", "R");
				break;

			}
			default:
			{
				// handle error...
				comm->sendError("ERROR: unknown response from WaitForSingleObject!\r\n", "", "R");
				break;
			}
		}
		ReleaseMutex(plottingMutex);
		// write the data to the file.
		std::string errMsg;
		int experimentPictureNumber;
		if (this->runSettings.showPicsInRealTime)
		{
			experimentPictureNumber = 0;
		}
		else
		{
			experimentPictureNumber = (((this->currentRepetitionNumber - 1) 
				% this->runSettings.totalPicsInVariation) % runSettings.picsPerRepetition);
		}
		if (this->runSettings.cameraMode != "Continuous Single Scans Mode")
		{
			/// TODO
			/*
			if (eExperimentData.writeFits(errMsg, experimentPictureNumber, this->currentMainThreadRepetitionNumber, eImagesOfExperiment))
			{
				comm->sendError(errMsg, "", "R");
			}
			*/
		}
	}
	else
	{
		bRetValue = FALSE;
		comm->sendError("ERROR: Data range is zero\r\n", "", "R");
		return FALSE;
	}
	/// TODO
	/*
	if (eCooler)
	{
		// start temp timer again when acq is complete
		SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
	}
	*/
	// % 4 at the end because there are only 4 pictures available on the screen.
	int imageLocation = (((this->currentRepetitionNumber - 1) 
		% this->runSettings.totalPicsInVariation) % runSettings.repetitionsPerVariation) % 4;
	return TRUE;
}

void AndorCamera::drawDataWindow(void)
{
	if (imagesOfExperiment.size() != 0)
	{
		for (int experimentImagesInc = 0; experimentImagesInc < imagesOfExperiment.size(); experimentImagesInc++)
		{
			long maxValue = 1;
			long minValue = 65536;
			double avgValue;
			// for all pixels... find the max and min of the picture.
			for (int pixelInc = 0; pixelInc < imagesOfExperiment[experimentImagesInc].size(); pixelInc++)
			{
				try
				{
					if (imagesOfExperiment[experimentImagesInc][pixelInc] > maxValue)
					{
						maxValue = imagesOfExperiment[experimentImagesInc][pixelInc];
					}
					if (imagesOfExperiment[experimentImagesInc][pixelInc] < minValue)
					{
						minValue = imagesOfExperiment[experimentImagesInc][pixelInc];
					}
				}
				catch (std::out_of_range&)
				{
					errBox("ERROR: caught std::out_of_range in this->drawDataWindow! experimentImagesInc = " + std::to_string(experimentImagesInc)
						+ ", pixelInc = " + std::to_string(pixelInc) + ", eImagesOfExperiment.size() = " + std::to_string(imagesOfExperiment.size())
						+ ", eImagesOfExperiment[experimentImagesInc].size() = " + std::to_string(imagesOfExperiment[experimentImagesInc].size())
						+ ". Attempting to continue...");
					return;
				}
			}
			avgValue = std::accumulate(imagesOfExperiment[experimentImagesInc].begin(),
				imagesOfExperiment[experimentImagesInc].end(), 0.0)
										/ imagesOfExperiment[experimentImagesInc].size();
			HDC hDC = 0;
			float xscale, yscale, zscale;
			long modrange = 1;
			double dTemp = 1;
			int imageBoxWidth, imageBoxHeight;
			int pixelsAreaWidth;
			int pixelsAreaHeight;
			int dataWidth, dataHeight;
			int i, j, iTemp;
			HANDLE hloc;
			PBITMAPINFO pbmi;
			WORD argbq[PICTURE_PALLETE_SIZE];
			BYTE *DataArray;
			// % 4 at the end because there are only 4 pictures available on the screen.
				
			int imageLocation;
			if (runSettings.showPicsInRealTime)
			{
				imageLocation = (((currentRepetitionNumber - 1) % runSettings.totalPicsInVariation) 
					% runSettings.picsPerRepetition) % 4;
			}
			else
			{
				imageLocation = experimentImagesInc % 4;
			}
			// Rotated
			/*
			int selectedPixelCount = imagesOfExperiment[experimentImagesInc][eCurrentlySelectedPixel.first 
																				+ eCurrentlySelectedPixel.second * tempParam.width];

			ePicStats.update(selectedPixelCount, maxValue, minValue, avgValue, imageLocation);
			*/
			/*
			hDC = GetDC(eCameraWindowHandle);
			std::array<int, 4> colors = ePictureOptionsControl.getPictureColors();
			SelectPalette(hDC, eAppPalette[colors[imageLocation]], TRUE);
			RealizePalette(hDC);
			pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
			pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
			*/
			/*
			if (eAutoscalePictures)
			{
				modrange = maxValue - minValue;
			}
			else
			{
				modrange = eCurrentMaximumPictureCount[imageLocation] - eCurrentMinimumPictureCount[imageLocation];
			}
			*/
			dataWidth = runSettings.imageSettings.width;
			dataHeight = runSettings.imageSettings.height;
			// imageBoxWidth must be a multiple of 4, otherwise StretchDIBits has problems apparently T.T
			if (pixelsAreaWidth % 4)
			{
				pixelsAreaWidth += (4 - pixelsAreaWidth % 4);
			}

			yscale = (256.0f) / (float)modrange;

			for (i = 0; i < PICTURE_PALLETE_SIZE; i++)
			{
				argbq[i] = (WORD)i;
			}

			hloc = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(WORD)*PICTURE_PALLETE_SIZE));

			pbmi = (PBITMAPINFO)LocalLock(hloc);
			pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pbmi->bmiHeader.biPlanes = 1;
			pbmi->bmiHeader.biBitCount = 8;
			pbmi->bmiHeader.biCompression = BI_RGB;
			pbmi->bmiHeader.biClrUsed = PICTURE_PALLETE_SIZE;

			pbmi->bmiHeader.biHeight = dataHeight;
			memcpy(pbmi->bmiColors, argbq, sizeof(WORD) * PICTURE_PALLETE_SIZE);

			DataArray = (BYTE*)malloc(dataWidth * dataHeight * sizeof(BYTE));
			memset(DataArray, PICTURE_PALLETE_SIZE - 1, dataWidth * dataHeight);
			for (i = 0; i < runSettings.imageSettings.height; i++)
			{
				for (j = 0; j < runSettings.imageSettings.width; j++)
				{
					/*
					if (eAutoscalePictures)
					{
						dTemp = ceil(yscale * (eImagesOfExperiment[experimentImagesInc][j + i * tempParam.width] - minValue));
					}
					else
					{
						dTemp = ceil(yscale * (eImagesOfExperiment[experimentImagesInc][j + i * tempParam.width] - eCurrentMinimumPictureCount[imageLocation]));
					}
					*/
					if (dTemp < 0)
					{
						// raise value to zero which is the floor of values this parameter can take.
						iTemp = 0;
					}
					else if (dTemp > PICTURE_PALLETE_SIZE - 1)
					{
						// round to maximum value.
						iTemp = PICTURE_PALLETE_SIZE - 1;
					}
					else
					{
						// no rounding or flooring to min or max needed.
						iTemp = (int)dTemp;
					}
					// store the value.
					DataArray[j + i * dataWidth] = (BYTE)iTemp;
				}
			}
			SetStretchBltMode(hDC, COLORONCOLOR);
			// eCurrentAccumulationNumber starts at 1.
			BYTE *finalDataArray = NULL;
			switch (runSettings.imageSettings.width % 4)
			{
				case 0:
				{
					pbmi->bmiHeader.biWidth = dataWidth;
					/*
					StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth,
						dataHeight, DataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS,
						SRCCOPY);
					*/
					break;
				}
				case 2:
				{
					// make array that is twice as long.
					finalDataArray = (BYTE*)malloc(dataWidth * dataHeight * 2);
					memset(finalDataArray, 255, dataWidth * dataHeight * 2);

					for (int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++)
					{
						finalDataArray[2 * dataInc] = DataArray[dataInc];
						finalDataArray[2 * dataInc + 1] = DataArray[dataInc];
					}
					pbmi->bmiHeader.biWidth = dataWidth * 2;
					/*
					StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 2, dataHeight,
						finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY);
						*/
					free(finalDataArray);
					break;
				}
				default:
				{
					// make array that is 4X as long.
					finalDataArray = (BYTE*)malloc(dataWidth * dataHeight * 4);
					memset(finalDataArray, 255, dataWidth * dataHeight * 4);
					for (int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++)
					{
						int data = DataArray[dataInc];
						finalDataArray[4 * dataInc] = data;
						finalDataArray[4 * dataInc + 1] = data;
						finalDataArray[4 * dataInc + 2] = data;
						finalDataArray[4 * dataInc + 3] = data;
					}
					pbmi->bmiHeader.biWidth = dataWidth * 4;
					/*
					StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 4, dataHeight,
						finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY);
						*/
					free(finalDataArray);
					break;
				}
			}
			free(DataArray);
			/// other drawings
			/*
			RECT relevantRect = ePixelRectangles[imageLocation][eCurrentlySelectedPixel.first][tempParam.height - 1 - eCurrentlySelectedPixel.second];
			*/
			// make crosses
			/*
			std::vector<std::pair<int, int>> atomLocations = eAutoAnalysisHandler.getAtomLocations();
			for (int analysisPointInc = 0; analysisPointInc < atomLocations.size(); analysisPointInc++)
			{
				RECT crossRect = ePixelRectangles[imageLocation][atomLocations[analysisPointInc].first][tempParam.height - 1 - atomLocations[analysisPointInc].second];
				HDC hdc;
				HPEN crossPen;
				hdc = GetDC(eCameraWindowHandle);
				if (colors[imageLocation] == 0 || colors[imageLocation] == 2)
				{
					crossPen = CreatePen(0, 1, RGB(255, 0, 0));
				}
				else
				{
					crossPen = CreatePen(0, 1, RGB(0, 255, 0));
				}
				SelectObject(hdc, crossPen);
				MoveToEx(hdc, crossRect.left, crossRect.top, 0);
				LineTo(hdc, crossRect.right, crossRect.top);
				LineTo(hdc, crossRect.right, crossRect.bottom);
				LineTo(hdc, crossRect.left, crossRect.bottom);
				LineTo(hdc, crossRect.left, crossRect.top);
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(200, 200, 200));
				int atomNumber = analysisPointInc + 1;
				DrawTextEx(hdc, const_cast<char *>(std::to_string(atomNumber).c_str()), std::to_string(atomNumber).size(),
					&crossRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL);
				ReleaseDC(eCameraWindowHandle, hdc);
				DeleteObject(crossPen);
			}
			// color circle
			RECT halfRect;
			halfRect.left = relevantRect.left + 7.0 * (relevantRect.right - relevantRect.left) / 16.0;
			halfRect.right = relevantRect.left + 9.0 * (relevantRect.right - relevantRect.left) / 16.0;
			halfRect.top = relevantRect.top + 7.0 * (relevantRect.bottom - relevantRect.top) / 16.0;
			halfRect.bottom = relevantRect.top + 9.0 * (relevantRect.bottom - relevantRect.top) / 16.0;
			HGDIOBJ originalBrush = SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
			HGDIOBJ originalPen = SelectObject(hDC, GetStockObject(DC_PEN));
			if (colors[imageLocation] == 0 || colors[imageLocation] == 2)
			{
				SetDCPenColor(hDC, RGB(255, 0, 0));
				Ellipse(hDC, relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom);
				SelectObject(hDC, GetStockObject(DC_BRUSH));
				SetDCBrushColor(hDC, RGB(255, 0, 0));
			}
			else
			{
				SetDCPenColor(hDC, RGB(0, 255, 0));
				Ellipse(hDC, relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom);
				SelectObject(hDC, GetStockObject(DC_BRUSH));
				SetDCBrushColor(hDC, RGB(0, 255, 0));
			}
			Ellipse(hDC, halfRect.left, halfRect.top, halfRect.right, halfRect.bottom);
			SelectObject(hDC, originalBrush);
			SelectObject(hDC, originalPen);
			ReleaseDC(eCameraWindowHandle, hDC);
			LocalUnlock(hloc);
			LocalFree(hloc);
			*/
		}
	}
	return;
}
// The following are a set of simple functions that call the indicated andor SDK function if not in safe mode and check the error message.
int AndorCamera::setTriggerMode(Communicator* comm)
{
	std::string errMsg;
	int trigType;
	if (runSettings.triggerMode == "Internal")
	{
		trigType = 0;
	}
	else if (runSettings.triggerMode == "External")
	{
		trigType = 1;
	}
	else if (runSettings.triggerMode == "Start On Trigger")
	{
		trigType = 6;
	}

	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(SetTriggerMode(trigType));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Set Trigger Mode Error: " + errMsg + "\r\n", "", "R");
		return -1;
	}
	return 0;
}
int AndorCamera::setTemperature(Communicator* comm)
{
	// Get the current temperature
	if (runSettings.temperatureSetting< -60 || runSettings.temperatureSetting > 25)
	{
		int answer = MessageBox(0, "Warning: The selected temperature is outside the normal temperature range of the camera (-60 through 25 C). Proceed "
			"anyways?", 0, MB_OKCANCEL);
		if (answer == IDCANCEL)
		{
			return -1;
		}
	}
	// Proceedure to initiate cooling
	this->changeTemperatureSetting(false, comm);
	return 0;
}
int AndorCamera::setAcquisitionMode(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(SetAcquisitionMode(runSettings.acquisitionMode));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Error while setting Acquisition mode: " + errMsg + "\r\n", "", "R");
		return -1;
	}

	return 0;
}
int AndorCamera::setReadMode(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		// Set read mode to required setting specified in xxxxWndw.c
		errMsg = this->andorErrorChecker(SetReadMode(runSettings.readMode));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Error while setting read msode: " + errMsg + "\r\n", "", "R");
		return -1;
	}
	return 0;
}
	

int AndorCamera::setExposures(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		if (runSettings.exposureTimes.size() > 0 && runSettings.exposureTimes.size() <= 16)
		{
			errMsg = this->andorErrorChecker(SetRingExposureTimes(runSettings.exposureTimes.size(), runSettings.exposureTimes.data()));
		}
		else
		{
			errMsg = "Invalid size for vector of exposure times, value of " + std::to_string(runSettings.exposureTimes.size()) + ".";
		}
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Exposure times error: " + errMsg + "\r\n", "", "R");
		return -1;
	}
	return 0;
}

/*
	*
	*/
int AndorCamera::setImageParametersToCamera(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(SetImage(runSettings.imageSettings.verticalBinning, 
			runSettings.imageSettings.horizontalBinning, runSettings.imageSettings.topBorder, 
			runSettings.imageSettings.bottomBorder, runSettings.imageSettings.leftBorder, 
			runSettings.imageSettings.rightBorder));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}

	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("Set Image Error: " + errMsg + "\r\n", "", "R");
		return -1;
	}
	return 0;
}
int AndorCamera::setKineticCycleTime(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(SetKineticCycleTime(this->runSettings.kinetiCycleTime));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
	//
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: SetKineticCycleTime Error: " + errMsg + "\r\n", "", "R");
		return -1;
	}
	return 0;
}
int AndorCamera::setScanNumber(Communicator* comm)
{
	if (runSettings.totalPicsInExperiment == 0 && runSettings.totalPicsInVariation != 0)
	{
		// all is good. The eCurrentTotalVariationNumber has not been set yet.
	}
	else if (runSettings.totalPicsInVariation == 0)
	{
		comm->sendError("ERROR: Scan Number Was Zero.\r\n", "", "R");
	}
	else
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = this->andorErrorChecker(SetNumberKinetics(runSettings.totalPicsInExperiment));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}

		if (errMsg != "DRV_SUCCESS")
		{
			comm->sendError("ERROR: Set number kinetics error\r\n", "", "R");
			return -1;
		}
	}
	return 0;
}
int AndorCamera::setFrameTransferMode(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(SetFrameTransferMode(runSettings.frameTransferMode));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
		
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: SetFrameTransferMode returned error: " + errMsg + "\r\n", "", "R");
		return -1;
	}
	return 0;
}


/*
	* exposures should be initialized to be the correct size. Nothing else matters for the inputs, they get 
	* over-written.
	* throws exception if fails
	*/
int AndorCamera::checkAcquisitionTimings(float& kinetic, float& accumulation, std::vector<float>& exposures,
	Communicator* comm)
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
	if (!ANDOR_SAFEMODE)
	{
		// get times
		errMsg = this->andorErrorChecker(GetAcquisitionTimings(&tempExposure, &tempAccumTime, &tempKineticTime));
		timesArray = new float[exposures.size()];
		errMsg = this->andorErrorChecker(GetAdjustedRingExposureTimes(exposures.size(), timesArray));
	}
	else 
	{
		timesArray = new float[exposures.size()];
		for (int exposureInc = 0; exposureInc < exposures.size(); exposureInc++)
		{
			timesArray[exposureInc] = exposures[exposureInc];
		}
		errMsg = "DRV_SUCCESS";
	}
	// 
	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: GetAcquisitionTimings returned error: " + errMsg + "\r\n", "", "R");
		throw std::runtime_error("GetAcquisitionTimings returned error: " + errMsg);
		return -1;
	}
	else
	{
		// success. Set times
		if (exposures.size() > 0)
		{
			for (int exposureInc = 0; exposureInc < exposures.size(); exposureInc++)
			{
				exposures[exposureInc] = timesArray[exposureInc];
			}
			delete[] timesArray;
		}
		accumulation = tempAccumTime;
		kinetic = tempKineticTime;
	}
	return 0;
}
	
	
/*
*/
int AndorCamera::getStatus(Communicator* comm)
{
	int status;
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(GetStatus(&status));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
		status = DRV_IDLE;
	}

	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: GetStatus Error " + errMsg + "\r\n", "", "R");
		return -1;
	}

	if (status != DRV_IDLE)
	{
		comm->sendError("ERROR: You tried to start the camera, but the camera was not idle! Camera was in state corresponding to " + std::to_string(status) + "\r\n", "", "R");
		return -1;
	}
	return 0;
}
int AndorCamera::startAcquisition(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(StartAcquisition());
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}

	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Start acquisition error: " + errMsg + "\r\n", "", "R");
		if (!ANDOR_SAFEMODE)
		{
			errMsg = this->andorErrorChecker(AbortAcquisition());
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		if (errMsg != "DRV_SUCCESS")
		{
			comm->sendError("ERROR: Abort Acquisition Error: " + errMsg + "\r\n", "", "R");
			return -1;
		}
	}
	return 0;
}
int AndorCamera::setAccumulationCycleTime(Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(SetAccumulationCycleTime(runSettings.accumulationTime));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}
	return 0;
}
int AndorCamera::setNumberAccumulations(bool isKinetic, Communicator* comm)
{
	std::string errMsg;
	if (!ANDOR_SAFEMODE)
	{
		if (isKinetic)
		{
			// right now, kinetic series mode always has one accumulation. could add this feature later if desired.
			errMsg = this->andorErrorChecker(SetNumberAccumulations(1));
		}
		else
		{
			errMsg = this->andorErrorChecker(SetNumberAccumulations(this->runSettings.totalPicsInExperiment));
		}
	}
	else
	{
		errMsg = "DRV_SUCCESS";
	}

	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: SetNumberAccumulations() returned error: " + errMsg + "\r\n", "", "R");
		return -1;
	}

	return 0;
}
int AndorCamera::setGainMode(Communicator* comm)
{
	if (this->runSettings.emGainModeIsOn == false)
	{
		// Set Gain
		int numGain;
		std::string errorMessage;
		if (!ANDOR_SAFEMODE)
		{
			//
			errorMessage = this->andorErrorChecker(GetNumberPreAmpGains(&numGain));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: GetNumberPreAmpGains() returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			errorMessage = this->andorErrorChecker(SetPreAmpGain(2));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetPreAmpGain() returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			float myGain;
			errorMessage = this->andorErrorChecker(GetPreAmpGain(2, &myGain));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: GetPreAmpGain(2) returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			errorMessage = this->andorErrorChecker(SetOutputAmplifier(1));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetOutputAmplifier(1) (1 for conventional gain mode) returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
		}
		else
		{
			std::string errorMessage;
			errorMessage = "DRV_SUCCESS";
		}
	}
	else
	{
		if (!ANDOR_SAFEMODE)
		{
			std::string errorMessage;
			errorMessage = this->andorErrorChecker(SetOutputAmplifier(0));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetOutputAmplifier(0) (0 for EM gain mode) returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			errorMessage = this->andorErrorChecker(SetPreAmpGain(2));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetPreAmpGain() returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
			if (runSettings.emGainLevel > 300)
			{
				errorMessage = this->andorErrorChecker(SetEMAdvanced(1));
				if (errorMessage != "DRV_SUCCESS")
				{
					comm->sendError("ERROR: SetEMAdvanced(1) returned error: " + errorMessage + "\r\n", "", "R");
					return -1;
				}
			}
			else
			{
				errorMessage = this->andorErrorChecker(SetEMAdvanced(0));
				if (errorMessage != "DRV_SUCCESS")
				{
					comm->sendError("ERROR: SetEMAdvanced(0) returned error: " + errorMessage + "\r\n", "", "R");
					return -1;
				}
			}
			errorMessage = this->andorErrorChecker(SetEMCCDGain(runSettings.emGainLevel));
			if (errorMessage != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: SetEMCCDGain(gainLevel) returned error: " + errorMessage + "\r\n", "", "R");
				return -1;
			}
		}
		else
		{
			std::string errorMessage;
			errorMessage = "DRV_SUCCESS";
		}
	}

	return 0;
}
///

void AndorCamera::changeTemperatureSetting(bool temperatureControlOff, Communicator* comm)
{
	char aBuffer[256];
	int minimumAllowedTemp, maximumAllowedTemp;
	std::string errMsg;
	// clear buffer
	wsprintf(aBuffer, "");

	// check if temp is in valid range
	if (!ANDOR_SAFEMODE)
	{
		errMsg = this->andorErrorChecker(GetTemperatureRange(&minimumAllowedTemp, &maximumAllowedTemp));
	}
	else
	{
		errMsg = "DRV_SUCCESS";
		minimumAllowedTemp = -60;
		maximumAllowedTemp = 25;
	}

	if (errMsg != "DRV_SUCCESS")
	{
		comm->sendError("ERROR: Temperature error: " + errMsg + "\r\n", "", "R");
	}
	else
	{
		if (runSettings.temperatureSetting < minimumAllowedTemp || runSettings.temperatureSetting > maximumAllowedTemp)
		{
			comm->sendError("ERROR: Temperature is out of range\r\n", "", "R");
		}
		else
		{
			// if it is in range, switch on cooler and set temp
			if (!ANDOR_SAFEMODE)
			{
				if (temperatureControlOff == false)
				{
					errMsg = this->andorErrorChecker(CoolerON());
				}
				else
				{
					errMsg = this->andorErrorChecker(CoolerOFF());
					if (errMsg != "DRV_SUCCESS")
					{
						comm->sendError("ERROR: failed to turn temperature control off: " + errMsg + "\r\n", "", "R");
					}
				}
			}
			else
			{
				errMsg = "DRV_SUCCESS";
			}
			// 
			if (errMsg != "DRV_SUCCESS")
			{
				comm->sendError("ERROR: Could not switch cooler on/temperatureControlOff: " + errMsg + "\r\n", "", "R");
			}
			else
			{
				/*
				eCooler = TRUE;
				SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
				*/
				if (temperatureControlOff == false)
				{
					if (!ANDOR_SAFEMODE)
					{
						errMsg = this->andorErrorChecker(SetTemperature(runSettings.temperatureSetting));
					}
					else
					{
						errMsg == "DRV_SUCCESS";
					}

					if (errMsg != "DRV_SUCCESS")
					{
						comm->sendError("ERROR: Could not set temperature: " + errMsg + "\r\n", "", "R");
					}
					else
					{
						comm->sendStatus("Temperature has been set to " + std::to_string(this->runSettings.temperatureSetting) + " (C)\r\n", "", "");
					}
				}
				else
				{
					comm->sendStatus("Temperature Control has been turned off.\r\n", "", "");
				}
			}
		}
	}
}

/*
	*
	*/
std::string AndorCamera::andorErrorChecker(int errorCode)
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
			errorMessage = "UNKNONW ERROR MESSAGE!";
			break;
		}
	}
	return errorMessage;
}
