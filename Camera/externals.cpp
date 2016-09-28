#include "stdafx.h"
#include "externals.h"
#include "Windows.h"
#include "ATMCD32D.H"
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "constants.h"
#include "gnuplot-iostream.h"
//#include "fitsio.h"
#include "ConfigurationFileSystem.h"
#include "ExperimentTimer.h"
#include "DataFileSystem.h"
#include "SMSTextingControl.h"
#include "AlertSystem.h"
#include "EmbeddedPythonHandler.h"
#include "PictureStats.h"
#include "PictureOptions.h"

/// \\\ THINGS THAT THE USER SETS \\\ ///
ConfigurationFileSystem eCameraFileSystem(CAMERA_CONFIGURATION_FILES_LOCATION);
ExperimentTimer eCameraWindowExperimentTimer;
DataFileSystem eExperimentData(SAVE_BASE_ADDRESS);
SMSTextingControl eTextingHandler;
DataAnalysisHandler eAutoAnalysisHandler;
CameraImage eImageControl;
AlertSystem eAlerts(8000);
EmbeddedPythonHandler Python;
PictureStats ePicStats;
PictureOptions ePictureOptionsControl;

bool eSettingAnalysisLocations = false;
bool eRealTimePictures = false;
bool eAutoanalyzeData = false;
std::array<int, 4> eCurrentMaximumPictureCount{325, 325, 325, 325};
std::array<int, 4> eCurrentMinimumPictureCount{95, 95, 95, 95};
std::array<int, 4> eCurrentPicturePallete{ 0, 0, 1, 2 };
std::array<std::vector<std::vector<RECT> >, 4> ePixelRectangles;
std::pair<int, int> eCurrentlySelectedPixel;

int eSelectedPixelCount;

int eMinimumSliderCount = 0;
int eMaximumSliderCount = 10000;

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
int eCurrentAccumulationModeTotalAccumulationNumber;
double eKineticCycleTime = 0;
int eCurrentTotalVariationNumber = 0;
int ePicturesPerVariation = 1;
int eTotalNumberOfPicturesInSeries = 0;
int eNumberOfRunsToAverage = 0;
//
bool eIncDataFileNamesOption;

int ePicturesPerRepetition = 1;
bool eSystemIsRunning = false;

double eAccumulationTime;
double eAccumulationModeAccumulationNumber;

std::string eCurrentlySelectedCameraMode = "Kinetic Series Mode";
std::string eCurrentlyRunningCameraMode;
std::string eCurrentTriggerMode = "External";

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
int eRepetitionsPerVariation = 1;

/// \\\ GLOBAL VARIABLES THAT THE CODE (ONLY THE CODE) USES \\\ ///

/// Data Analysis
// Initialize this object with the location of the gnuplot executable.
// for my laptop
Gnuplot ePlotter(GNUPLOT_LOCATION);
// main image buffer read from card
std::vector<std::vector<long> > eImagesOfExperiment;
// 
std::vector<std::vector<long> > eImageVecQueue;
// points data required to draw polyline (Is this still needed???)
POINT *ePointsArray = NULL;
/// Other

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

std::string eFinalSaveFolder;
std::string eFinalSaveName;

int eCurrentAccumulationNumber = 1;
volatile int eCurrentThreadAccumulationNumber = 0;

// This gets set when redrawing happens to signal that I don't need to redraw my own plots.
bool eRedrawFlag;
// gets initialized to false and then is never reset.
bool eDataExists = false;

int eCurrentTempColor = ID_RED;

int eCount1 = 0;
int eCount2 = 0;
int eCount3 = 0;

/// Windows API Stuffs
HINSTANCE eHInst;
// Main Handles
HWND eCameraWindowHandle;
// Code-Edited Edit Handles (disp or text)
HwndControl eTriggerTextDisplayHandle, eImgLeftSideTextHandle,
	 eImageBottomSideTextHandle, eImgRightSideTextHandle, eImageTopSideTextHandle, eVerticalBinningTextHandle, eHorizontalBinningTextHandle,
	 eKineticCycleTimeTextHandle, eTotalPictureNumberTextHandle, ePixel1TextDisplay, ePixel2TextDisplay,
	 eImgLeftSideDispHandle, eImgRightSideDispHandle, eHorizontalBinningDispHandle, eImageBottomSideDispHandle, eImageTopSideDispHandle,
	 eVerticalBinningDispHandle, ePixelsXLocationsDispHandle, ePixelsYLocationsDispHandle, ePixelXLocationsTextHandle, ePixelYLocationsTextHandle,
	 eKineticCycleTimeDispHandle, eRepetitionsPerVariationDisp, eCurrentAccumulationNumDispHandle, eMinCountDispHandle,
	 eMaxCountDispHandle, eVariationNumberDisp;
// User-Edited Edit Handles
HwndControl eImgLeftSideEditHandle, eImageTopEditHandle, eImgRightSideEditHandle, eImageBottomEditHandle,
	 eVerticalBinningEditHandle, eHorizontalBinningEditHandle, eKineticCycleTimeEditHandle, eRepetitionsPerVariationEdit, ePixel1XEditHandle,
	 ePixel2XEditHandle, ePixel1YEditHandle, ePixel2YEditHandle, ePixelsXLocationsEditHandle, ePixelsYLocationsEditHandle, 
	 eVariationNumberEdit;
// Temperature Control
HwndControl eTempTextDisplayHandle, eCurrentTempDisplayHandle, eTempDispHandle, eTempEditHandle, eSetTemperatureButtonHandle, eTempOffButton;
// Error Status
HwndControl eErrorEditHandle, eErrorClear, eErrorText;
// Camera Status
HwndControl eStatusEditHandle, eClearStatusButtonHandle, eStatusText;
// Checked Box Handles
HwndControl eIncDataFileOptionBoxHandle;
// Button Handles
HwndControl eSetImageParametersButtonHandle, eSetAnalysisPixelsButtonHandle,
	 eSetKineticSeriesCycleTimeButtonHandle, eSetNumberRepetitionsPerVariationButton, ePlotAverageCountsBoxHandle,
	 eSeVariationNumberButton;
// ComboBox Handles
HwndControl eTriggerComboHandle, eCameraModeComboHandle;
// temporary
HwndControl eSetAccumulationTimeButton, eAccumulationTimeEdit, eAccumulationTimeDisp, eSetAccumulationNumberButton, eAccumulationNumberEdit,
		eSetAccumulationNumberDisp, eSetRunningAverageNumberToAverageButton, eRunningAverageEdit, eRunningAverageDisp, eRunningAverageBox,
		eSetPlottingFrequencyButton, ePlottingFrequencyEdit, ePlottingFrequencyDisp, eAllPlotsCombo, eAllPlotsText, eCurrentPlotsCombo, eCurrentPlotsText,
		eSeriesProgressBar, eMinimumPictureSlider2, eMaximumPictureSlider2, eMinSliderNumberEdit2, eMaxSliderNumberEdit2, eMinSliderText2, eMaxSliderText2,
		eSubSeriesProgressBar;

// contrast controls
HwndControl eMinimumPictureSlider1, eMaximumPictureSlider1, eMinSliderNumberEdit1, eMaxSliderNumberEdit1,
		eMinSliderText1, eMaxSliderText1, eMinimumPictureSlider3, eMaximumPictureSlider3, eMinSliderNumberEdit3,
		eMaxSliderNumberEdit3, eMinSliderText3, eMaxSliderText3, eMinimumPictureSlider4, eMaximumPictureSlider4, eMinSliderNumberEdit4,
		eMaxSliderNumberEdit4, eMinSliderText4, eMaxSliderText4;

// EM Gain
HwndControl eSetEMGain, eEMGainText, eEMGainEdit, eEMGainDisplay, eEMGainForceChangeButton;


//
HDC eDrawPallete;
std::array<HPALETTE, 3> eAppPalette;

HBRUSH eGreyGreenBrush = CreateSolidBrush(RGB(100, 110, 100));
HBRUSH eVeryGreyGreenBrush = CreateSolidBrush(RGB(180, 220, 180));
HBRUSH eDarkGreenBrush = CreateSolidBrush(RGB(0, 30, 0));
HBRUSH eDarkRedBrush = CreateSolidBrush(RGB(100, 0, 0));
HBRUSH eDarkBlueBrush = CreateSolidBrush(RGB(0, 0, 100));
HBRUSH eGreyRedBrush = CreateSolidBrush(RGB(150, 100, 100));
HBRUSH eWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));

HANDLE ePlottingMutex;
HANDLE eImagesMutex;

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

std::string eDataType = RAW_COUNTS;
