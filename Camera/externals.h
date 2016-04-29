#pragma once
#include "Windows.h"
#include "ATMCD32D.H"
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"
#include <array>
#include <deque>
#include "Control.h"
#include "PlottingInfo.h"
#include "fitsio.h"

/// THINGS THAT THE USER SETS !@$#!@#$!@#$!@$!@#$!@#$!@$#!@#$!@#$@!#$!$!@#$!@#$@!$#@!#$!@#$!@$#!#$!@#$!@#$!@#$!@#$!@$#!@$#!@$#!@$#@!#$!@$#!@$#!@#$!@$#!@$#!@$#!#
extern bool eFitsOkay;
extern fitsfile *eFitsFile;

extern std::array<int, 4> eCurrentMaximumPictureCount;
extern std::array<int, 4> eCurrentMinimumPictureCount;
extern std::array<int, 4> eCurrentPicturePallete;
extern int eMinimumSliderCount;
extern int eMaximumSliderCount;
extern std::array<std::vector<std::vector<RECT> >, 4> ePixelRectangles;
extern std::pair<int, int> eCurrentlySelectedPixel;
extern int eSelectedPixelCount;

extern bool eAutoscalePictures;
extern bool eEMGainMode;
extern int eEMGainLevel;

extern unsigned int ePlottingIsSlowMessage, ePlottingCaughtUpMessage;
// Binning Parameters
extern int eVerticalBinning, eHorizontalBinning, eLeftImageBorder, eRightImageBorder, eTopImageBorder, eBottomImageBorder, eImageWidth, eImageHeight;
// Temperature stuff
extern int eCameraTemperatureSetting;
// more parameters.
extern std::vector<float> eExposureTimes;
extern double eKineticCycleTime;
//
extern int ePictureSubSeriesNumber;
extern int ePicturesPerStack;
extern int eTotalNumberOfPicturesInSeries;
extern int eNumberOfRunsToAverage;
// changes data.fits to data_1.fits -> data_2.fits, etc.
extern bool eIncSaveFileNameOption;
// number of counts that a pixel has to contain in order to count as an atom.
extern int eDetectionThreshold;
// // // // // // // // // //
extern std::vector<std::string> eCurrentPlotNames;
/// Plotting Options
extern std::vector<std::string> eCurrentPlotNames;
// Plot Average Counts (first!)
extern volatile bool ePlotAverageCounts;
// Plot Atom Presence
extern volatile bool ePlotAtomLoading;
// plot a histogram of the counts seen on the pixels.
extern volatile bool ePlotCountHistogram;
// 
extern volatile bool ePlotAtomSurvival;
// 
extern volatile bool ePlotTunneling;
// 
extern volatile bool ePlotTwoParticleLoss;
// 
extern volatile bool ePlotRunningAverageSurvival;

extern int ePicturesPerExperiment;
extern bool eSystemIsRunning;

extern std::string eCurrentlySelectedCameraMode;
extern std::string eCurrentlyRunningCameraMode;
extern std::string eCurrentTriggerMode;

extern double eAccumulationTime;
extern double eAccumulationModeAccumulationNumber;

extern int ePreviousPicturesPerSubSeries;
extern int ePlottingFrequency;

extern PlottingInfo eCurrentPlottingInfo;

extern int eCurrentAnalysisSetSelectionNumber;
extern int eCurrentPixelSelectionNumber;
extern int eCurrentDataSetSelectionNumber;
extern int eCurrentPositivePictureNumber;
extern int eCurrentPositivePixelNumber;
extern int eCurrentPostSelectionPictureNumber;
extern int eCurrentPostSelectionPixelNumber;
extern int eCurrentPostSelectionConditionNumber;
extern int eExperimentsPerStack;

/// !@#$!@#$@!#$!@#$!@ GLOBAL VARIABLES THAT THE CODE (ONLY THE CODE) USES !@#$!@#$!@#$!@#$!@#$!@#$!@$!@#$!@#$!@#$!@$#!@#$!@#$!@#$!@#$!@#$!@#$!@#$@!#$@!#$!@#$!@
/// Data Analysis
// This vector contains all of the data points for all pixels in the current picture.
extern std::vector<long> eImageVec;
// This vector contains all of the most recent imageVecs ^ that haven't yet been plotted.
extern std::vector<std::vector<long> > eImageVecQueue;
// I'm using the gnuplot program to plot, and a nice iostream header to feed commands into that program. see the iostream-gnuplot header.
extern Gnuplot ePlotter;

/// Other
// points data required to draw polyline
extern POINT* ePointsArray;
// Contains coordinates of drawing areas and is used during painting.
extern std::array<RECT, 4> eImageBackgroundAreas;
extern std::array<RECT, 4> eImageDrawAreas;
//extern RECT rect;
// an object which will contain information about our camera.
extern AndorCapabilities eCameraCapabilities;
// Indicate if cooler is on
extern BOOL eCooler;
// 
extern BOOL eData;
//
extern BOOL eErrorFlag;
// Cameramodel
extern char eModel[32];   
// Settings for the camera. Currently hard-coded, but I'd like to change that at some point.
extern int eAcquisitionMode;   
extern int eReadMode;
// dims of CCD chip
extern int eXPixels, eYPixels;
// Vertical Speed Index
extern int eVerticalSpeedNumber;
// Horizontal Speed Index
extern int eHorizontalSpeedNumber;
// AD Index
extern int eADNumber;
// File Saving Info
extern std::string eFinalSaveFolder;
extern std::string eFinalSaveName;
extern int eCurrentAccumulationNumber;
extern volatile int eCurrentThreadAccumulationNumber;
// This gets set when redrawing happens to signal that I don't need to redraw my own plots.
extern bool eRedrawFlag;
extern bool eDataExists;
// vector that contains truth values for checks
extern std::vector<bool> eReadyToStart;

extern int eCount1;
extern int eCount2;
extern int eCount3;

/// Windows API Stuffs
extern HINSTANCE eHInst;

// Main Handles
extern HWND eCameraWindowHandle;
// Code-Edited Edit Handles (disp or text)
extern Control eTempTextDisplayHandle, eCurrentTempDisplayHandle, eStatusEditHandle, eTriggerTextDisplayHandle, eImgLeftSideTextHandle,
			eImageBottomSideTextHandle, eImgRightSideTextHandle, eImageTopSideTextHandle, eVerticalBinningTextHandle, eHorizontalBinningTextHandle, 
			eKineticCycleTimeTextHandle, eTotalPictureNumberTextHandle, ePixel1TextDisplay, ePixel2TextDisplay, eTempDispHandle, 
			eImgLeftSideDispHandle, eImgRightSideDispHandle, eHorizontalBinningDispHandle, eImageBottomSideDispHandle, eImageTopSideDispHandle,
			eVerticalBinningDispHandle,	eKineticCycleTimeDispHandle, eExperimentsPerStackDispHandle, eAtomThresholdDispHandle,
			eCurrentAccumulationNumDispHandle, eMinCountDispHandle,	eMaxCountDispHandle, ePictureSubSeriesNumberDispHandle, 
			ePicturesPerExperimentDispHandle, eErrorEditHandle;
// User-Edited Edit Handles
extern Control eTempEditHandle, eImgLeftSideEditHandle, eImageBottomSideEditHandle, eImgRightSideEditHandle, eImageTopSideEditHandle,
			eVerticalBinningEditHandle, eHorizontalBinningEditHandle, eKineticCycleTimeEditHandle, eExperimentsPerStackEditHandle, ePixel1XEditHandle, 
			ePixel2XEditHandle, ePixel1YEditHandle, ePixel2YEditHandle, eAtomThresholdEditHandle, ePictureSubSeriesNumberEditHandle, 
			ePicturesPerExperimentEditHandle;
// Checked Box Handles
extern Control eIncDataFileOptionBoxHandle;
// Button Handles
extern Control eSetTemperatureButtonHandle, eSetImageParametersButtonHandle, eSetAnalysisPixelsButtonHandle,
			eSetKineticSeriesCycleTimeButtonHandle, eSetNumberOfExperimentsPerStackButtonHandle, eSetAtomThresholdButtonHandle, ePlotAverageCountsBoxHandle, 
			eSetAccumulationStackNumberButtonHandle, eClearStatusButtonHandle, eSetPicturesPerExperimentButtonHandle;
// ComboBox Handles
extern Control eTriggerComboHandle, eCameraModeComboHandle;

// temp
extern Control eSetAccumulationTimeButton, eAccumulationTimeEdit, eAccumulationTimeDisp, eSetAccumulationNumberButton, eSetAccumulationNumberEdit, 
			   eSetAccumulationNumberDisp, eSetRunningAverageNumberToAverageButton, eRunningAverageEdit, eRunningAverageDisp, eRunningAverageBox,
			   eSetPlottingFrequencyButton, ePlottingFrequencyEdit, ePlottingFrequencyDisp, eAllPlotsCombo, eAllPlotsText, eCurrentPlotsCombo, 
			   eCurrentPlotsText, eSeriesProgressBar, eSubSeriesProgressBar;
	
// Contrast Control
extern Control eMinimumPictureSlider1, eMaximumPictureSlider1, eMinSliderNumberEdit1, 
			   eMaxSliderNumberEdit1, eMinSliderText1, eMaxSliderText1, eMinimumPictureSlider2, eMaximumPictureSlider2, eMinSliderNumberEdit2, 
			   eMaxSliderNumberEdit2, eMinSliderText2, eMaxSliderText2, eMinimumPictureSlider3, eMaximumPictureSlider3, eMinSliderNumberEdit3, 
			   eMaxSliderNumberEdit3, eMinSliderText3, eMaxSliderText3, eMinimumPictureSlider4, eMaximumPictureSlider4, eMinSliderNumberEdit4, 
			   eMaxSliderNumberEdit4, eMinSliderText4, eMaxSliderText4;
// Displays
extern Control ePic1MaxCountDisp, ePic2MaxCountDisp, ePic3MaxCountDisp, ePic4MaxCountDisp, ePic1MinCountDisp, ePic2MinCountDisp, ePic3MinCountDisp, 
			   ePic4MinCountDisp, ePic1SelectionCountDisp, ePic2SelectionCountDisp, ePic3SelectionCountDisp, ePic4SelectionCountDisp, ePic1Text, ePic2Text, 
			   ePic3Text, ePic4Text, eSelectionText, ePictureText;
// EM Gain Mode
extern Control eSetEMGain, eEMGainText, eEMGainEdit, eEMGainDisplay;

// Ring Exposure
extern Control eExposureTextDisplayHandle, eExposure1EditHandle, eExposure2EditHandle, eExposure3EditHandle, eSetExposureButtonHandle, eExposureDispHandle;

// Other win32 Objects
extern HDC eDrawPallete;
extern std::array<HPALETTE, 3> eAppPalette;

extern HBRUSH eGreyGreenBrush;
extern HBRUSH eDarkGreenBrush;
// This is the mutex used to make sure the data queue doesn't get read and wrote to at the same time. 
extern HANDLE ePlottingMutex;

/// Dialog controls
extern HWND eInitializeDialogBoxHandle;
extern HWND eRichEditMessageBoxRichEditHandle;
extern HWND eRichEditOkCancelMessageBoxRichEditHandle;

/// Threading Stuffs
// Used to tell the camera thread when to exit. volatile is important because this is also handled inside a thread.
extern volatile bool eThreadExitIndicator;
extern volatile bool ePlotThreadExitIndicator;

extern HANDLE eCameraThreadHandle;
extern HANDLE ePlottingThreadHandle;
// The codes that the thread passes back to the main program. 
extern unsigned int eAccMessageID, eFinMessageID, eErrMessageID;

