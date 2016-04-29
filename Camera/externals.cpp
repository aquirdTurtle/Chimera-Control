#include "stdafx.h"
#include "externals.h"
#include "Windows.h"
#include "ATMCD32D.H"
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "constants.h"
#include "gnuplot-iostream.h"
#include "fitsio.h"
/// \\\ THINGS THAT THE USER SETS \\\ ///

bool eFitsOkay;

fitsfile *eFitsFile;
std::array<int, 4> eCurrentMaximumPictureCount{325, 325, 325, 325};
std::array<int, 4> eCurrentMinimumPictureCount{95, 95, 95, 95};
std::array<int, 4> eCurrentPicturePallete{ 0, 0, 1, 2 };
std::array<std::vector<std::vector<RECT> >, 4> ePixelRectangles;
std::pair<int, int> eCurrentlySelectedPixel;
int eSelectedPixelCount;

int eMinimumSliderCount = 0;
int eMaximumSliderCount = 1000;

bool eAutoscalePictures = false;
bool eEMGainMode = false;
int eEMGainLevel = 0;

std::vector<std::string> eCurrentPlotNames;
unsigned int ePlottingIsSlowMessage, ePlottingCaughtUpMessage;
// binning things & default values
int eVerticalBinning = 1, eHorizontalBinning = 1, eLeftImageBorder = 1, eRightImageBorder = 512, eTopImageBorder = 1, eBottomImageBorder = 512,
eImageWidth = (eRightImageBorder - eLeftImageBorder + 1) / eHorizontalBinning, eImageHeight = (eBottomImageBorder - eTopImageBorder + 1) / eVerticalBinning;
// temperature things & default value
int eCameraTemperatureSetting = DEFAULT_CAMERA_TEMPERATURE;
//
std::vector<float> eExposureTimes;
double eKineticCycleTime;
int ePictureSubSeriesNumber;
int ePicturesPerStack;
int eTotalNumberOfPicturesInSeries;
int eNumberOfRunsToAverage = 0;
//
bool eIncSaveFileNameOption;
// number of counts that a pixel has to contain in order to count as an atom.
int eDetectionThreshold;
/// Plotting Options
// Plot Average Counts (first!)
volatile bool ePlotAverageCounts;
// Plot Atom Presence
volatile bool ePlotAtomLoading;
// plot a histogram of the counts seen on the pixels.
volatile bool ePlotCountHistogram;

// 
volatile bool ePlotAtomSurvival;
// 
volatile bool ePlotTunneling;
// 
volatile bool ePlotTwoParticleLoss;
// 
volatile bool ePlotRunningAverageSurvival;

int ePicturesPerExperiment = 1;
bool eSystemIsRunning = false;

double eAccumulationTime;
double eAccumulationModeAccumulationNumber;

std::string eCurrentlySelectedCameraMode = "Kinetic Series Mode";
std::string eCurrentlyRunningCameraMode;
std::string eCurrentTriggerMode;

int ePreviousPicturesPerSubSeries;
int ePlottingFrequency = 1;

PlottingInfo eCurrentPlottingInfo;

int eCurrentAnalysisSetSelectionNumber = 0;
int eCurrentPixelSelectionNumber = 0;
int eCurrentDataSetSelectionNumber = 0;
int eCurrentPositivePictureNumber = 0;
int eCurrentPositivePixelNumber = 0;
int eCurrentPostSelectionPictureNumber = 0;
int eCurrentPostSelectionPixelNumber = 0;
int eCurrentPostSelectionConditionNumber = 0;
int eExperimentsPerStack = 0;

/// \\\ GLOBAL VARIABLES THAT THE CODE (ONLY THE CODE) USES \\\ ///

/// Data Analysis
// Initialize this object with the location of the gnuplot executable.
// for andor
//Gnuplot ePlotter("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\"");
// for my laptop
Gnuplot ePlotter("\"C:\\Program Files (x86)\\gnuplot\\bin\\gnuplot.exe\"");
// main image buffer read from card
std::vector<long> eImageVec;
// 
std::vector<std::vector<long> > eImageVecQueue;
// points data required to draw polyline (Is this still needed???)
POINT *ePointsArray = NULL;
/// Other

// vector that contains truth values for checks. Initialize to NOT ready. Currently there are _|v|_ checks that are checked like this.
std::vector<bool> eReadyToStart{ false, false, false, false, false, false, false, false, false, false, false, false, true };

// headmodel
char eModel[32];
std::array<RECT, 4> eImageBackgroundAreas;
std::array<RECT, 4> eImageDrawAreas;
// dims of CCD chip
int	eXPixels;
int	eYPixels;
// Vertical Speed Index
int eVerticalSpeedNumber;
// Horizontal Speed Index
int eHorizontalSpeedNumber;
// AD Index
int eADNumber;
// Set based on current wishes...
int eReadMode = 4;
int eAcquisitionMode = 3;



AndorCapabilities eCameraCapabilities;

BOOL eCooler = FALSE;
BOOL eErrorFlag;
BOOL eData;

std::string eFinalSaveFolder;
std::string eFinalSaveName;

int eCurrentAccumulationNumber;
volatile int eCurrentThreadAccumulationNumber;

// This gets set when redrawing happens to signal that I don't need to redraw my own plots.
bool eRedrawFlag;
// gets initialized to false and then is never reset.
bool eDataExists = false;

int eCount1 = 0;
int eCount2 = 0;
int eCount3 = 0;

/// Windows API Stuffs
HINSTANCE eHInst;
// Main Handles
HWND eCameraWindowHandle;
// Code-Edited Edit Handles (disp or text)
Control eTempTextDisplayHandle, eCurrentTempDisplayHandle, eStatusEditHandle, eTriggerTextDisplayHandle, eImgLeftSideTextHandle,
	 eImageBottomSideTextHandle, eImgRightSideTextHandle, eImageTopSideTextHandle, eVerticalBinningTextHandle, eHorizontalBinningTextHandle,
	 eKineticCycleTimeTextHandle, eTotalPictureNumberTextHandle, ePixel1TextDisplay, ePixel2TextDisplay, eTempDispHandle,
	 eImgLeftSideDispHandle, eImgRightSideDispHandle, eHorizontalBinningDispHandle, eImageBottomSideDispHandle, eImageTopSideDispHandle,
	 eVerticalBinningDispHandle, ePixelsXLocationsDispHandle, ePixelsYLocationsDispHandle, ePixelXLocationsTextHandle, ePixelYLocationsTextHandle,
	 eKineticCycleTimeDispHandle, eExperimentsPerStackDispHandle, eAtomThresholdDispHandle, eCurrentAccumulationNumDispHandle, eMinCountDispHandle,
	 eMaxCountDispHandle, ePictureSubSeriesNumberDispHandle, ePicturesPerExperimentDispHandle;
// User-Edited Edit Handles
Control eTempEditHandle, eImgLeftSideEditHandle, eImageBottomSideEditHandle, eImgRightSideEditHandle, eImageTopSideEditHandle,
	 eVerticalBinningEditHandle, eHorizontalBinningEditHandle, eKineticCycleTimeEditHandle, eExperimentsPerStackEditHandle, ePixel1XEditHandle,
	 ePixel2XEditHandle, ePixel1YEditHandle, ePixel2YEditHandle, ePixelsXLocationsEditHandle, ePixelsYLocationsEditHandle, eAtomThresholdEditHandle,
	 ePictureSubSeriesNumberEditHandle, ePicturesPerExperimentEditHandle, eErrorEditHandle;
// Checked Box Handles
Control eIncDataFileOptionBoxHandle;
// Button Handles
Control eSetTemperatureButtonHandle, eSetImageParametersButtonHandle, eSetAnalysisPixelsButtonHandle,
	 eSetKineticSeriesCycleTimeButtonHandle, eSetNumberOfExperimentsPerStackButtonHandle, eSetAtomThresholdButtonHandle, ePlotAverageCountsBoxHandle,
	 eSetAccumulationStackNumberButtonHandle, eClearStatusButtonHandle, eSetPicturesPerExperimentButtonHandle;
// ComboBox Handles
Control eTriggerComboHandle, eCameraModeComboHandle;
// temp
Control eSetAccumulationTimeButton, eAccumulationTimeEdit, eAccumulationTimeDisp, eSetAccumulationNumberButton, eSetAccumulationNumberEdit,
		eSetAccumulationNumberDisp, eSetRunningAverageNumberToAverageButton, eRunningAverageEdit, eRunningAverageDisp, eRunningAverageBox,
		eSetPlottingFrequencyButton, ePlottingFrequencyEdit, ePlottingFrequencyDisp, eAllPlotsCombo, eAllPlotsText, eCurrentPlotsCombo, eCurrentPlotsText,
		eSeriesProgressBar, eMinimumPictureSlider2, eMaximumPictureSlider2, eMinSliderNumberEdit2, eMaxSliderNumberEdit2, eMinSliderText2, eMaxSliderText2,
		eSubSeriesProgressBar;

// contrast controls
Control eMinimumPictureSlider1, eMaximumPictureSlider1, eMinSliderNumberEdit1, eMaxSliderNumberEdit1,
		eMinSliderText1, eMaxSliderText1, eMinimumPictureSlider3, eMaximumPictureSlider3, eMinSliderNumberEdit3,
		eMaxSliderNumberEdit3, eMinSliderText3, eMaxSliderText3, eMinimumPictureSlider4, eMaximumPictureSlider4, eMinSliderNumberEdit4,
		eMaxSliderNumberEdit4, eMinSliderText4, eMaxSliderText4;

// pixel counts displays
Control ePic1MaxCountDisp, ePic2MaxCountDisp, ePic3MaxCountDisp, ePic4MaxCountDisp, ePic1MinCountDisp, ePic2MinCountDisp, ePic3MinCountDisp,
		ePic4MinCountDisp, ePic1SelectionCountDisp, ePic2SelectionCountDisp, ePic3SelectionCountDisp, ePic4SelectionCountDisp, ePic1Text, ePic2Text,
		ePic3Text, ePic4Text, eSelectionText, ePictureText;
// EM Gain
Control eSetEMGain, eEMGainText, eEMGainEdit, eEMGainDisplay;

// Ring Exposure time controls
Control eExposureTextDisplayHandle, eExposure1EditHandle, eExposure2EditHandle, eExposure3EditHandle, eSetExposureButtonHandle, eExposureDispHandle;

//
HDC eDrawPallete;
std::array<HPALETTE, 3> eAppPalette;

HBRUSH eGreyGreenBrush = CreateSolidBrush(RGB(100, 110, 100));
HBRUSH eDarkGreenBrush = CreateSolidBrush(RGB(0, 30, 0));

HANDLE ePlottingMutex;
HWND eInitializeDialogBoxHandle;

/// Dialog controls
HWND eRichEditMessageBoxRichEditHandle;
HWND eRichEditOkCancelMessageBoxRichEditHandle;

/// Threading Stuffs
// set to false in order to cause the thread to terminate. This is only done on exiting the program.
volatile bool eThreadExitIndicator = true;
volatile bool ePlotThreadExitIndicator;

// Messages for the camera thread.
unsigned int eAccMessageID = 0, eFinMessageID = 0, eErrMessageID = 0;
HANDLE eCameraThreadHandle;
HANDLE ePlottingThreadHandle;
