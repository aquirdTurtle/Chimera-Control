#include "stdafx.h"
#include "myAndor.h"
#include "appendText.h"
#include "plotThread.h"
#include <process.h>
#include "arbitraryPlottingThreadProcedure.h"

namespace myAndor
{
	/* 
	 * Large function which initializes a given camera image run.
	 */
	int setSystem()
	{
		char aBuffer2[256];
		/// Set a bunch of parameters.
		std::string errorMessage;
		// Set to 1 MHz readout rate in both cases
		if (eEMGainMode == true)
		{
			if (!ANDOR_SAFEMODE)
			{
				errorMessage = myAndor::andorErrorChecker(SetADChannel(1));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetADChannel Error: " + errorMessage + "\r\n", IDC_STATUS_EDIT);
					return -1;
				}
				errorMessage = myAndor::andorErrorChecker(SetHSSpeed(0, 0));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetHSSpeed Error: " + errorMessage + "\r\n", IDC_STATUS_EDIT);
					return -1;
				}

			}
		}
		else
		{
			if (!ANDOR_SAFEMODE)
			{
				errorMessage = myAndor::andorErrorChecker(SetADChannel(1));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetADChannel Error: " + errorMessage + "\r\n", IDC_STATUS_EDIT);
					return -1;
				}
				errorMessage = myAndor::andorErrorChecker(SetHSSpeed(1, 0));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetHSSpeed Error: " + errorMessage + "\r\n", IDC_STATUS_EDIT);
					return -1;
				}
			}
		}

		if (myAndor::setAcquisitionMode() != 0)
		{
			return -1;
		}
		if (myAndor::setReadMode() != 0)
		{
			return -1;
		}
		if (myAndor::setExposures() != 0)
		{
			return -1;
		}
		imageParameters currentImageParameters = eImageParameters.getImageParameters();
		if (myAndor::setImageParametersToCamera(currentImageParameters) != 0)
		{
			return -1;
		}
		// Set Mode-Specific Parameters
		if (eAcquisitionMode == 5)
		{
			if (myAndor::setFrameTransferMode(0) != 0)
			{
				return -1;
			}
		}
		else if (eAcquisitionMode == 3)
		{
			if (myAndor::setKineticCycleTime() != 0)
			{
				return -1;
			}
			if (myAndor::setScanNumber() != 0)
			{
				eSystemIsRunning = false;
				return -1;
			}
			// set this to 1.
			if (myAndor::setNumberAccumulations(true) != 0)
			{
				return -1;
			}
		}
		else if (eAcquisitionMode == 2)
		{
			if (myAndor::setAccumulationCycleTime() != 0)
			{
				return -1;
			}
			if (myAndor::setNumberAccumulations(false) != 0)
			{
				return -1;
			}
		}
		if (myAndor::checkAcquisitionTimings() != 0)
		{
			return -1;
		}
		if (myAndor::setGainMode() != 0)
		{
			return -1;
		}
		// Set trigger mode.
		if (myAndor::setTriggerMode() != 0)
		{
			return -1;
		}
		

		// CAREFUL! I can only modify these guys here because I'm sure that I'm also not writing to them in the plotting thread since the plotting thread hasn't
		// started yet. If moving stuff around, be careful.
		// Initialize the thread accumulation number.
		eCurrentThreadAccumulationNumber = 1;
		// //////////////////////////////
		eCount1 = 0;
		eCount2 = 0;
		eCount3 = 0;

		if (myAndor::getStatus() != 0)
		{
			return -1;
		}

		/// setup fits files
		std::string errMsg;
		if (eCurrentlyRunningCameraMode != "Continuous Single Scans Mode")
		{
			if (eExperimentData.initializeDataFiles(eIncDataFileNamesOption, errMsg))
			{
				appendText(errMsg, IDC_ERROR_EDIT);
				return -1;
			}
		}
		/// Do some plotting stuffs
		eAlerts.setAlertThreshold();

		// set default colors and linewidths on plots
		eCurrentAccumulationNumber = 1;
		// Create the Mutex. This function just opens the mutex if it already exists.
		ePlottingMutex = CreateMutexEx(0, NULL, FALSE, MUTEX_ALL_ACCESS);
		// prepare for start of thread.
		ePlotThreadExitIndicator = true;
		// start thread.
		unsigned int * plottingThreadID = NULL;
		std::vector<std::string>* argPlotNames;
		argPlotNames = new std::vector<std::string>;
		for (int plotNameInc = 0; plotNameInc < eCurrentPlotNames.size(); plotNameInc++)
		{
			argPlotNames->push_back(eCurrentPlotNames[plotNameInc]);
		}
		ePlottingThreadHandle = (HANDLE)_beginthreadex(0, 0, arbitraryPlottingThreadProcedure, argPlotNames, 0, plottingThreadID);
		// color pictures, clearing last run.
		HDC hDC = GetDC(eCameraWindowHandle);
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
		// clear the plot grid
		SendMessage(ePic1MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic1MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic1SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic2MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic2MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic2SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic3MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic3MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic3SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic4MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic4MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		SendMessage(ePic4SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
		

		if (myAndor::startAcquisition() != 0)
		{
			return -1;
		}
		// Else started!
		UpdateWindow(eStatusEditHandle.hwnd);
		return 0;
	}

	// This function queries the camera for how many pictures are available, retrieves all of them, then paints them to the main window. It returns the success of
	// this operation.
	BOOL acquireImageData()
	{
		long first, last;
		// check the number of images available
		std::string errMsg;

		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(GetNumberNewImages(&first, &last));
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
				appendText("ERROR: GetNumberNewImages error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
				return FALSE;
			}
		}
		/// ///
		// for only one image... (each image processed from the call from a separate windows message)
		int size;
		// If there is no data the acquisition must have been aborted
		// free all allocated memory
		int experimentPictureNumber;
		if (eRealTimePictures)
		{
			experimentPictureNumber = 0;
		}
		else
		{
			experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition);
		}
		if (experimentPictureNumber == 0)
		{
			eImagesOfExperiment.clear();
			if (eRealTimePictures)
			{
				eImagesOfExperiment.resize(1);
			}
			else
			{
				eImagesOfExperiment.resize(ePicturesPerRepetition);
			}
		}
		imageParameters tempParam = eImageParameters.getImageParameters();
		size = tempParam.width * tempParam.height;
		std::vector<long> tempImage;
		tempImage.resize(size);
		eImagesOfExperiment[experimentPictureNumber].resize(size);
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(GetOldestImage(&tempImage[0], tempImage.size()));
			// immediately rotate
			for (int imageVecInc = 0; imageVecInc < eImagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
			{
				eImagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc % tempParam.width) + 1) * tempParam.height - imageVecInc / tempParam.width - 1];
			}
		}
		else
		{
			errMsg = "DRV_SUCCESS";
			for (int imageVecInc = 0; imageVecInc < eImagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
			{
				tempImage[imageVecInc] = rand() % 50 + 95;
				tempImage[0] = 1000;
			}
			for (int imageVecInc = 0; imageVecInc < eImagesOfExperiment[experimentPictureNumber].size(); imageVecInc++)
			{
				eImagesOfExperiment[experimentPictureNumber][imageVecInc] = tempImage[((imageVecInc % tempParam.width) + 1) * tempParam.height - imageVecInc / tempParam.width - 1];
			}
		}
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Acquisition Error on Retrieval: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return FALSE;
		}
		eDataExists = true;
		// Display data and query max data value to be displayed in status box
		BOOL bRetValue = TRUE;
		long maxValue = 1;
		long minValue = 65536;
		if (eImagesOfExperiment[experimentPictureNumber].size() != 0)
		{
			//HDC hdc = GetDC(eCameraWindowHandle);
			// Find max value and scale data to fill rect
			for (int i = 0; i<(tempParam.width*tempParam.height); i++)
			{
				if (eImagesOfExperiment[experimentPictureNumber][i] > maxValue)
				{
					maxValue = eImagesOfExperiment[experimentPictureNumber][i];
				}
				if (eImagesOfExperiment[experimentPictureNumber][i] < minValue)
				{
					minValue = eImagesOfExperiment[experimentPictureNumber][i];
				}
			}
			if (maxValue == minValue)
			{
				return FALSE;
			}
			// update the picture
			if (experimentPictureNumber == ePicturesPerRepetition - 1 || eRealTimePictures)
			{
				myAndor::drawDataWindow();
			}
			// Wait until eImageVecQueue is available using the mutex.
			DWORD mutexMsg = WaitForSingleObject(ePlottingMutex, INFINITE);
			switch (mutexMsg)
			{
				case WAIT_OBJECT_0:
				{
					// Add data to the plotting queue, only if actually plotting something.
					if (eCurrentPlotNames.size() != 0)
					{
						eImageVecQueue.push_back(eImagesOfExperiment[experimentPictureNumber]);
					}
					eCount1++;
					break;
				}
				case WAIT_ABANDONED:
				{
					// handle error...
					appendText("ERROR: waiting for the plotting mutex failed (Wait Abandoned)!\r\n", IDC_ERROR_EDIT);
					break;
				}
				case WAIT_TIMEOUT:
				{
					// handle error...
					appendText("ERROR: waiting for the plotting mutex failed (timout???)!\r\n", IDC_ERROR_EDIT);
					break;
				}
				case WAIT_FAILED:
				{
					// handle error...
					int a = GetLastError();
					appendText("ERROR: waiting for the plotting mutex failed (Wait Failed: " + std::to_string(a) + ")!\r\n", IDC_ERROR_EDIT);
					break;

				}
				default:
				{
					// handle error...
					appendText("ERROR: unknown response from WaitForSingleObject!\r\n", IDC_ERROR_EDIT);
					break;
				}
			}
			ReleaseMutex(ePlottingMutex);
			// write the data to the file.
			std::string errMsg;
			int experimentPictureNumber;
			if (eRealTimePictures)
			{
				experimentPictureNumber = 0;
			}
			else
			{
				experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition);
			}
			if (eCurrentlyRunningCameraMode != "Continuous Single Scans Mode")
			{
				if (eExperimentData.writeFits(errMsg, experimentPictureNumber, eCurrentAccumulationNumber, eImagesOfExperiment))
				{
					appendText(errMsg, IDC_ERROR_EDIT);
				}
			}
		}
		else
		{
			bRetValue = FALSE;
			appendText("ERROR: Data range is zero\r\n", IDC_ERROR_EDIT);
			return FALSE;
		}
		if (eCooler)
		{
			// start temp timer again when acq is complete
			SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);
		}
		// % 4 at the end because there are only 4 pictures available on the screen.
		int imageLocation = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition) % 4;
		return TRUE;
	}

	void drawDataWindow(void)
	{
		if (eImagesOfExperiment.size() != 0)
		{
			imageParameters tempParam = eImageParameters.getImageParameters();
			for (int experimentImagesInc = 0; experimentImagesInc < eImagesOfExperiment.size(); experimentImagesInc++)
			{
				long maxValue = 1;
				long minValue = 65536;
				// for all pixels...
				for (int pixelInc = 0; pixelInc < (tempParam.width * tempParam.height); pixelInc++)
				{
					if (eImagesOfExperiment[experimentImagesInc][pixelInc] > maxValue)
					{
						maxValue = eImagesOfExperiment[experimentImagesInc][pixelInc];
					}
					if (eImagesOfExperiment[experimentImagesInc][pixelInc] < minValue)
					{
						minValue = eImagesOfExperiment[experimentImagesInc][pixelInc];
					}
				}

				HDC hDC;
				float xscale, yscale, zscale;
				long modrange;
				double dTemp;
				int imageBoxWidth, imageBoxHeight;
				int pixelsAreaWidth;
				int pixelsAreaHeight;
				int dataWidth, dataHeight;
				int i, j, iTemp;
				HANDLE hloc;
				PBITMAPINFO pbmi;
				WORD argbq[NUM_PALETTE_COLORS];
				BYTE *DataArray;
				// % 4 at the end because there are only 4 pictures available on the screen.
				
				int imageLocation;
				if (eRealTimePictures)
				{
					imageLocation = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition) % 4;
				}
				else
				{
					imageLocation = experimentImagesInc % 4;
				}
				// Rotated
				int selectedPixelCount = eImagesOfExperiment[experimentImagesInc][eCurrentlySelectedPixel.first + eCurrentlySelectedPixel.second * tempParam.width];
				if (imageLocation == 0)
				{
					SendMessage(ePic1MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(maxValue)).c_str());
					SendMessage(ePic1MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(minValue)).c_str());
					SendMessage(ePic1SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(selectedPixelCount)).c_str());
				}
				else if (imageLocation == 1)
				{
					SendMessage(ePic2MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(maxValue)).c_str());
					SendMessage(ePic2MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(minValue)).c_str());
					SendMessage(ePic2SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(selectedPixelCount)).c_str());
				}
				else if (imageLocation == 2)
				{
					SendMessage(ePic3MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(maxValue)).c_str());
					SendMessage(ePic3MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(minValue)).c_str());
					SendMessage(ePic3SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(selectedPixelCount)).c_str());
				}
				else if (imageLocation == 3)
				{
					SendMessage(ePic4MaxCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(maxValue)).c_str());
					SendMessage(ePic4MinCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(minValue)).c_str());
					SendMessage(ePic4SelectionCountDisp.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(selectedPixelCount)).c_str());
				}
				hDC = GetDC(eCameraWindowHandle);
				SelectPalette(hDC, eAppPalette[eCurrentPicturePallete[imageLocation]], TRUE);
				RealizePalette(hDC);
				pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
				pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;

				if (eAutoscalePictures)
				{
					modrange = maxValue - minValue;
				}
				else
				{
					modrange = eCurrentMaximumPictureCount[imageLocation] - eCurrentMinimumPictureCount[imageLocation];
				}

				dataWidth = tempParam.width;
				dataHeight = tempParam.height;
				// imageBoxWidth must be a multiple of 4, otherwise StretchDIBits has problems apparently T.T
				if (pixelsAreaWidth % 4)
				{
					pixelsAreaWidth += (4 - pixelsAreaWidth % 4);
				}

				yscale = (256.0f) / (float)modrange;

				for (i = 0; i < NUM_PALETTE_COLORS; i++)
				{
					argbq[i] = (WORD)i;
				}

				hloc = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(WORD)*NUM_PALETTE_COLORS));

				pbmi = (PBITMAPINFO)LocalLock(hloc);
				pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				pbmi->bmiHeader.biPlanes = 1;
				pbmi->bmiHeader.biBitCount = 8;
				pbmi->bmiHeader.biCompression = BI_RGB;
				pbmi->bmiHeader.biClrUsed = NUM_PALETTE_COLORS;

				pbmi->bmiHeader.biHeight = dataHeight;
				memcpy(pbmi->bmiColors, argbq, sizeof(WORD) * NUM_PALETTE_COLORS);

				DataArray = (BYTE*)malloc(dataWidth * dataHeight * sizeof(BYTE));
				memset(DataArray, 255, dataWidth * dataHeight);
				for (i = 0; i < tempParam.height; i++)
				{
					for (j = 0; j < tempParam.width; j++)
					{
						if (eAutoscalePictures)
						{
							dTemp = ceil(yscale * (eImagesOfExperiment[experimentImagesInc][j + i * tempParam.width] - minValue));
						}
						else
						{
							dTemp = ceil(yscale * (eImagesOfExperiment[experimentImagesInc][j + i * tempParam.width] - eCurrentMinimumPictureCount[imageLocation]));
						}
						if (dTemp < 0)
						{
							// raise value to zero which is the floor of values this parameter can take.
							iTemp = 0;
						}
						else if (dTemp > NUM_PALETTE_COLORS - 1)
						{
							// round to maximum value.
							iTemp = NUM_PALETTE_COLORS - 1;
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
				switch (tempParam.width % 4)
				{
					case 0:
					{
						pbmi->bmiHeader.biWidth = dataWidth;
						StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth,
							dataHeight, DataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS,
							SRCCOPY);
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
						StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 2, dataHeight,
							finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY);
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
						StretchDIBits(hDC, eImageDrawAreas[imageLocation].left, eImageDrawAreas[imageLocation].top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 4, dataHeight,
							finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY);
						free(finalDataArray);
						break;
					}
				}
				free(DataArray);
				/// other drawings
				RECT relevantRect = ePixelRectangles[imageLocation][eCurrentlySelectedPixel.first][tempParam.height - 1 - eCurrentlySelectedPixel.second];
				// make crosses
				std::vector<std::pair<int, int>> atomLocations = eAutoAnalysisHandler.getAtomLocations();
				for (int analysisPointInc = 0; analysisPointInc < atomLocations.size(); analysisPointInc++)
				{
					RECT crossRect = ePixelRectangles[imageLocation][atomLocations[analysisPointInc].first][tempParam.height - 1 - atomLocations[analysisPointInc].second];
					HDC hdc;
					HPEN crossPen;
					hdc = GetDC(eCameraWindowHandle);
					if (eCurrentPicturePallete[imageLocation] == 0 || eCurrentPicturePallete[imageLocation] == 2)
					{
						crossPen = CreatePen(0, 1, RGB(255, 0, 0));
					}
					else
					{
						crossPen = CreatePen(0, 1, RGB(0, 255, 0));
					}
					SelectObject(hdc, crossPen);
					long boxWidth = crossRect.right - crossRect.left;
					long boxHeight = crossRect.top - crossRect.bottom;
					MoveToEx(hdc, crossRect.left + boxWidth / 4, crossRect.top - boxHeight / 4, 0);
					LineTo(hdc, crossRect.right - boxWidth / 4, crossRect.bottom + boxHeight / 4);
					MoveToEx(hdc, crossRect.right - boxWidth / 4, crossRect.top - boxHeight / 4, 0);
					LineTo(hdc, crossRect.left + boxWidth / 4, crossRect.bottom + boxHeight / 4);
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
				if (eCurrentPicturePallete[imageLocation] == 0 || eCurrentPicturePallete[imageLocation] == 2)
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
			}
		}
		return;
	}
	// The following are a set of simple functions that call the indicated andor SDK function if not in safe mode and check the error message.
	int setTriggerMode(void)
	{
		std::string errMsg;
		int trigType;
		if (eCurrentTriggerMode == "Internal")
		{
			trigType = 0;
		}
		else if (eCurrentTriggerMode == "External")
		{
			trigType = 1;
		}
		else if (eCurrentTriggerMode == "Start On Trigger")
		{
			trigType = 6;
		}

		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(SetTriggerMode(trigType));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Set Trigger Mode Error: " + errMsg + "\r\n", IDC_STATUS_EDIT);
			return -1;
		}

		return 0;
	}
	int setTemperature(void)
	{
		// Get the current temperature
		std::string tempTemperature;
		GetWindowText(eTempEditHandle.hwnd, (LPSTR)tempTemperature.c_str(), 4);
		eCameraTemperatureSetting = std::stoi(tempTemperature);
		if (eCameraTemperatureSetting < -60 || eCameraTemperatureSetting > 25)
		{
			int answer = MessageBox(0, "Warning: The selected temperature is outside the normal temperature range of the camera (-60 through 25 C). Proceed "
				"anyways?", 0, MB_OKCANCEL);
			if (answer == IDCANCEL)
			{
				return -1;
			}
		}
		SendMessage(eTempDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempTemperature.c_str());
		// Proceedure to initiate cooling
		myAndor::changeTemperatureSetting(false);
		return 0;
	}
	int setAcquisitionMode(void)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(SetAcquisitionMode(eAcquisitionMode));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Error while setting Acquisition mode: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}

		return 0;
	}
	int setReadMode(void)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			// Set read mode to required setting specified in xxxxWndw.c
			errMsg = myAndor::andorErrorChecker(SetReadMode(eReadMode));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Error while setting read msode: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}
		return 0;
	}
	int setExposures(void)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			if (eExposureTimes.size() > 0 && eExposureTimes.size() <= 16)
			{
				errMsg = myAndor::andorErrorChecker(SetRingExposureTimes(eExposureTimes.size(), eExposureTimes.data()));
			}
			else
			{
				errMsg = "Invalid eExposureTimes.size() value of " + std::to_string(eExposureTimes.size()) + ".";
			}
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Exposure times error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}
		return 0;
	}

	/*
	 *
	 */
	int setImageParametersToCamera(imageParameters tempImageParam)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(SetImage(tempImageParam.verticalBinning, tempImageParam.horizontalBinning, tempImageParam.topBorder, tempImageParam.bottomBorder, tempImageParam.leftBorder, tempImageParam.rightBorder));
			//errMsg = myAndor::andorErrorChecker(SetImage(eHorizontalBinning, eVerticalBinning, eLeftImageBorder, eRightImageBorder, eTopImageBorder, eBottomImageBorder));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}

		if (errMsg != "DRV_SUCCESS")
		{
			appendText("Set Image Error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}
		return 0;
	}
	int setKineticCycleTime(void)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(SetKineticCycleTime(eKineticCycleTime));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		//
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: SetKineticCycleTime Error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}
		return 0;
	}
	int setScanNumber(void)
	{
		if (eTotalNumberOfPicturesInSeries == 0 && ePicturesPerVariation != 0)
		{
			// all is good. The eCurrentTotalVariationNumber has not been set yet.
		}
		else if (ePicturesPerVariation == 0)
		{
			appendText("ERROR: Scan Number Was Zero.\r\n", IDC_STATUS_EDIT);
		}
		else
		{
			std::string errMsg;
			if (!ANDOR_SAFEMODE)
			{
				errMsg = myAndor::andorErrorChecker(SetNumberKinetics(eTotalNumberOfPicturesInSeries));
			}
			else
			{
				errMsg = "DRV_SUCCESS";
			}

			if (errMsg != "DRV_SUCCESS")
			{
				appendText("ERROR: Set number kinetics error\r\n", IDC_STATUS_EDIT);
				return -1;
			}
		}
		return 0;
	}
	int setFrameTransferMode(int mode)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(SetFrameTransferMode(mode));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: SetFrameTransferMode returned error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}
		return 0;
	}
	int checkAcquisitionTimings()
	{
		float tempExposure, tempAccumTime, tempKineticTime;
		float * timesArray = NULL;
		std::string errMsg;
		if (ANDOR_SAFEMODE)
		{
			// if in safemode initialize this stuff to the values to be outputted.
			if (eExposureTimes.size() > 0)
			{
				tempExposure = eExposureTimes[0];
			}
			else
			{
				tempExposure = 0;
			}
			tempAccumTime = eAccumulationTime;
			tempKineticTime = eKineticCycleTime;
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
			errMsg = myAndor::andorErrorChecker(GetAcquisitionTimings(&tempExposure, &tempAccumTime, &tempKineticTime));
			if (eExposureTimes.size() > 0)
			{
				timesArray = new float[eExposureTimes.size()];
				errMsg = myAndor::andorErrorChecker(GetAdjustedRingExposureTimes(eExposureTimes.size(), timesArray));
			}
		}
		else 
		{
			timesArray = new float[eExposureTimes.size()];
			for (int exposureInc = 0; exposureInc < eExposureTimes.size(); exposureInc++)
			{
				timesArray[exposureInc] = eExposureTimes[exposureInc];
			}
			errMsg = "DRV_SUCCESS";
		}
		// 
		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: GetAcquisitionTimings return error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}
		else
		{
			// success. Set times
			if (eExposureTimes.size() > 0)
			{
				for (int exposureInc = 0; exposureInc < eExposureTimes.size(); exposureInc++)
				{
					eExposureTimes[exposureInc] = timesArray[exposureInc];
				}
				delete[] timesArray;
			}
			eAccumulationTime = tempAccumTime;
			eKineticCycleTime = tempKineticTime;
		}
		return 0;
	}
	int getStatus()
	{
		int status;
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(GetStatus(&status));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
			status = DRV_IDLE;
		}

		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: GetStatus Error " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}

		if (status != DRV_IDLE)
		{
			appendText("ERROR: You tried to start the camera, but the camera was not idle! Camera was in state corresponding to " + std::to_string(status) + "\r\n", IDC_ERROR_EDIT);;
			return -1;
		}
		return 0;
	}
	int startAcquisition()
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(StartAcquisition());
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}

		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Start acquisition error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			if (!ANDOR_SAFEMODE)
			{
				errMsg = myAndor::andorErrorChecker(AbortAcquisition());
			}
			else
			{
				errMsg = "DRV_SUCCESS";
			}
			if (errMsg != "DRV_SUCCESS")
			{
				appendText("ERROR: Abort Acquisition Error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
				return -1;
			}
		}
		return 0;
	}
	int setAccumulationCycleTime()
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(SetAccumulationCycleTime(eAccumulationTime));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}
		return 0;
	}
	int setNumberAccumulations(int kinetic)
	{
		std::string errMsg;
		if (!ANDOR_SAFEMODE)
		{
			if (kinetic)
			{
				// right now, kinetic series mode always has one accumulation. could add this feature later if desired.
				errMsg = myAndor::andorErrorChecker(SetNumberAccumulations(1));
			}
			else
			{
				errMsg = myAndor::andorErrorChecker(SetNumberAccumulations(eCurrentAccumulationModeTotalAccumulationNumber));
			}
		}
		else
		{
			errMsg = "DRV_SUCCESS";
		}

		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: SetNumberAccumulations() returned error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
			return -1;
		}

		return 0;
	}
	int setGainMode()
	{
		if (eEMGainMode == false)
		{
			// Set Gain
			int numGain;
			std::string errorMessage;
			if (!ANDOR_SAFEMODE)
			{
				//
				errorMessage = myAndor::andorErrorChecker(GetNumberPreAmpGains(&numGain));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: GetNumberPreAmpGains() returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
					return -1;
				}
				errorMessage = myAndor::andorErrorChecker(SetPreAmpGain(2));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetPreAmpGain() returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
					return -1;
				}
				float myGain;
				errorMessage = myAndor::andorErrorChecker(GetPreAmpGain(2, &myGain));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: GetPreAmpGain(2) returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
					return -1;
				}
				errorMessage = myAndor::andorErrorChecker(SetOutputAmplifier(1));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetOutputAmplifier(1) (1 for conventional gain mode) returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
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
				errorMessage = myAndor::andorErrorChecker(SetOutputAmplifier(0));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetOutputAmplifier(0) (0 for EM gain mode) returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
					return -1;
				}
				errorMessage = myAndor::andorErrorChecker(SetPreAmpGain(2));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetPreAmpGain() returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
					return -1;
				}
				if (eEMGainLevel > 300)
				{
					errorMessage = myAndor::andorErrorChecker(SetEMAdvanced(1));
					if (errorMessage != "DRV_SUCCESS")
					{
						appendText("ERROR: SetEMAdvanced(1) returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
						return -1;
					}
				}
				else
				{
					errorMessage = myAndor::andorErrorChecker(SetEMAdvanced(0));
					if (errorMessage != "DRV_SUCCESS")
					{
						appendText("ERROR: SetEMAdvanced(0) returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
						return -1;
					}
				}
				errorMessage = myAndor::andorErrorChecker(SetEMCCDGain(eEMGainLevel));
				if (errorMessage != "DRV_SUCCESS")
				{
					appendText("ERROR: SetEMCCDGain(gainLevel) returned error: " + errorMessage + "\r\n", IDC_ERROR_EDIT);
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

	void changeTemperatureSetting(bool temperatureControlOff)
	{
		char aBuffer[256];
		int minimumAllowedTemp, maximumAllowedTemp;
		std::string errMsg;
		// clear buffer
		wsprintf(aBuffer, "");

		// check if temp is in valid range
		if (!ANDOR_SAFEMODE)
		{
			errMsg = myAndor::andorErrorChecker(GetTemperatureRange(&minimumAllowedTemp, &maximumAllowedTemp));
		}
		else
		{
			errMsg = "DRV_SUCCESS";
			minimumAllowedTemp = -60;
			maximumAllowedTemp = 25;
		}

		if (errMsg != "DRV_SUCCESS")
		{
			appendText("ERROR: Temperature error: " + errMsg + "\r\n", IDC_ERROR_EDIT);
		}
		else
		{
			if (eCameraTemperatureSetting<minimumAllowedTemp || eCameraTemperatureSetting>maximumAllowedTemp)
			{
				appendText("ERROR: Temperature is out of range\r\n", IDC_ERROR_EDIT);
			}
			else
			{
				// if it is in range, switch on cooler and set temp
				if (!ANDOR_SAFEMODE)
				{
					if (temperatureControlOff == false)
					{
						errMsg = myAndor::andorErrorChecker(CoolerON());
					}
					else
					{
						errMsg = myAndor::andorErrorChecker(CoolerOFF());
						if (errMsg != "DRV_SUCCESS")
						{
							appendText("ERROR: failed to turn temperature control off: " + errMsg + "\r\n", IDC_ERROR_EDIT);
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
					appendText("ERROR: Could not switch cooler on/temperatureControlOff: " + errMsg + "\r\n", IDC_ERROR_EDIT);
				}
				else
				{
					eCooler = TRUE;
					SetTimer(eCameraWindowHandle, ID_TEMPERATURE_TIMER, 1000, NULL);

					if (temperatureControlOff == false)
					{
						if (!ANDOR_SAFEMODE)
						{
							errMsg = myAndor::andorErrorChecker(SetTemperature(eCameraTemperatureSetting));
						}
						else
						{
							errMsg == "DRV_SUCCESS";
						}

						if (errMsg != "DRV_SUCCESS")
						{
							appendText("ERROR: Could not set temperature: " + errMsg + "\r\n", IDC_ERROR_EDIT);
						}
						else
						{
							appendText("Temperature has been set to " + std::to_string(eCameraTemperatureSetting) + " (C)\r\n", IDC_STATUS_EDIT);
						}
					}
					else
					{
						appendText("Temperature Control has been turned off.\r\n", IDC_STATUS_EDIT);
					}
				}
			}
		}
	}

	std::string andorErrorChecker(int errorCode)
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
}
