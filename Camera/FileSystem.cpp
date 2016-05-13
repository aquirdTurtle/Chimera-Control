#include "stdafx.h"
#include "FileSystem.h"
#include "Windows.h"
#include <fstream>
#include "externals.h"
#include "constants.h"
#include "reorganizeControl.h"
#include "dialogProcedures.h"
#include "Resource.h"
#include "reorganizeWindow.h"
#include "myAndor.h"
#include "appendText.h"
#include "Commctrl.h"

FileSystem::FileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
}
FileSystem::~FileSystem()
{
	// nothing for destructor right now
}

int FileSystem::openConfiguration(std::string configurationNameToOpen)
{
	if (configurationNameToOpen == "" || !FileSystem::fileExists(FILE_SYSTEM_PATH + configurationNameToOpen + ".cConfig"))
	{
		// can't open this.
		return -1;
	}
	configurationName = configurationNameToOpen;
	// open the configuration file
	std::ifstream configurationOpenFile(FILE_SYSTEM_PATH + configurationNameToOpen + ".cConfig");
	/// ...................................................
	/// load all the things.
	/// '''''''''''''''''''''''''''''''''''''''''''''''''''

	/// Exposure Times
	int numberOfExposureTimes;
	configurationOpenFile >> numberOfExposureTimes;
	if (numberOfExposureTimes < 1)
	{
		eExposureTimes.resize(0);
	}
	else
	{
		eExposureTimes.resize(numberOfExposureTimes);
	}
	for (int exposureInc = 0; exposureInc < eExposureTimes.size(); exposureInc++)
	{
		configurationOpenFile >> eExposureTimes[exposureInc];
	}
	// try to set this time.
	if (myAndor::setExposures() < 0)
	{
		appendText("ERROR: failed to set exposure times.", IDC_ERROR_EDIT);
		SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
		return -1;
	}
	// now check actual times.
	if (myAndor::checkAcquisitionTimings() < 0)
	{
		// bad
		appendText("ERROR: Unable to check acquisition timings.\r\n", IDC_ERROR_EDIT);
		SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
		return -1;
	}
	// now output things.
	if (eExposureTimes.size() <= 0)
	{
		// this shouldn't happend
		appendText("ERROR: reached bad location where eExposureTimes was of zero size, but this should have been detected earlier in the code.", IDC_ERROR_EDIT);
	}
	SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
	appendText(std::to_string(eExposureTimes[0]), IDC_EXPOSURE_DISP);
	for (int exposureInc = 1; exposureInc < eExposureTimes.size(); exposureInc++)
	{
		appendText(" -> " + std::to_string(eExposureTimes[exposureInc] * 1000), IDC_EXPOSURE_DISP);
	}
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	///

	configurationOpenFile.get();
	std::getline(configurationOpenFile, eCurrentTriggerMode);
	SendMessage(eTriggerComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)eCurrentTriggerMode.c_str());

	configurationOpenFile >> eLeftImageBorder;
	SendMessage(eImgLeftSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eLeftImageBorder).c_str());
	configurationOpenFile >> eRightImageBorder;
	SendMessage(eImgRightSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eRightImageBorder).c_str());
	configurationOpenFile >> eHorizontalBinning;
	SendMessage(eHorizontalBinningDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eHorizontalBinning).c_str());
	configurationOpenFile >> eTopImageBorder;
	SendMessage(eImageBottomSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eTopImageBorder).c_str());
	configurationOpenFile >> eBottomImageBorder;
	SendMessage(eImageTopSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eBottomImageBorder).c_str());
	configurationOpenFile >> eVerticalBinning;
	SendMessage(eVerticalBinningDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eVerticalBinning).c_str());
	eCurrentlySelectedPixel.first = 0;
	eCurrentlySelectedPixel.second = 0;
	// Calculate the number of actual pixels in each dimension.
	eImageWidth = (eRightImageBorder - eLeftImageBorder + 1) / eHorizontalBinning;
	eImageHeight = (eBottomImageBorder - eTopImageBorder + 1) / eVerticalBinning;

	for (int imageLocation = 0; imageLocation < eImageBackgroundAreas.size(); imageLocation++)
	{
		int imageBoxWidth = eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left + 1;
		int imageBoxHeight = eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top + 1;

		double boxWidth = imageBoxWidth / eImageWidth;
		double boxHeight = imageBoxHeight / eImageHeight;
		if (boxWidth > boxHeight)
		{
			// scale the box width down.
			eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
			eImageDrawAreas[imageLocation].right = (int)eImageBackgroundAreas[imageLocation].left
				+ (eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left) * boxHeight / boxWidth;
			double pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
			// move to center
			eImageDrawAreas[imageLocation].left += (imageBoxWidth - pixelsAreaWidth) / 2;
			eImageDrawAreas[imageLocation].right += (imageBoxWidth - pixelsAreaWidth) / 2;
			eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
			eImageDrawAreas[imageLocation].bottom = eImageBackgroundAreas[imageLocation].bottom;
			double pixelsAreaHeight = imageBoxHeight;
		}
		else
		{
			// cale the box height down.
			eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
			eImageDrawAreas[imageLocation].right = eImageBackgroundAreas[imageLocation].right;
			double pixelsAreaWidth = imageBoxWidth;
			// move to center
			eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
			eImageDrawAreas[imageLocation].bottom = (int)eImageBackgroundAreas[imageLocation].top + (eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top) * boxWidth / boxHeight;
			double pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
			eImageDrawAreas[imageLocation].top += (imageBoxWidth - pixelsAreaHeight) / 2;
			eImageDrawAreas[imageLocation].bottom += (imageBoxWidth - pixelsAreaHeight) / 2;
		}
	}
	// create rectangles for selection circle
	for (int pictureInc = 0; pictureInc < eImageDrawAreas.size(); pictureInc++)
	{
		ePixelRectangles[pictureInc].resize(eImageWidth);
		for (int widthInc = 0; widthInc < eImageWidth; widthInc++)
		{
			ePixelRectangles[pictureInc][widthInc].resize(eImageHeight);
			for (int heightInc = 0; heightInc < eImageHeight; heightInc++)
			{
				// for all 4 pictures...
				ePixelRectangles[pictureInc][widthInc][heightInc].left = (int)(eImageDrawAreas[pictureInc].left
					+ (double)widthInc * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left) / (double)eImageWidth + 2);
				ePixelRectangles[pictureInc][widthInc][heightInc].right = (int)(eImageDrawAreas[pictureInc].left
					+ (double)(widthInc + 1) * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left) / (double)eImageWidth + 2);
				ePixelRectangles[pictureInc][widthInc][heightInc].top = (int)(eImageDrawAreas[pictureInc].top
					+ (double)(heightInc)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top) / (double)eImageHeight);
				ePixelRectangles[pictureInc][widthInc][heightInc].bottom = (int)(eImageDrawAreas[pictureInc].top
					+ (double)(heightInc + 1)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top) / (double)eImageHeight);
			}
		}
	}


	///

	configurationOpenFile >> eEMGainMode;
	configurationOpenFile >> eEMGainLevel;
	if (eEMGainMode == false || eEMGainLevel < 0)
	{
		SendMessage(eEMGainDisplay.hwnd, WM_SETTEXT, 0, (LPARAM)"OFF");
	}
	else
	{
		SendMessage(eEMGainDisplay.hwnd, WM_SETTEXT, 0, (LPARAM)("X" + std::to_string(eEMGainLevel)).c_str());
	}	
	myAndor::setGainMode();
	configurationOpenFile >> ePicturesPerExperiment;
	SendMessage(ePicturesPerExperimentDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerExperiment).c_str());
	configurationOpenFile >> eExperimentsPerStack;
	SendMessage(eExperimentsPerStackDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eExperimentsPerStack).c_str());
	ePicturesPerStack = ePicturesPerExperiment * eExperimentsPerStack;
	configurationOpenFile >> eCurrentAccumulationStackNumber;
	SendMessage(eAccumulationStackNumberDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentAccumulationStackNumber).c_str());
	eTotalNumberOfPicturesInSeries = eCurrentAccumulationStackNumber * ePicturesPerStack;
	// get \n at end of previous line
	configurationOpenFile.get();
	std::getline(configurationOpenFile, eCurrentlySelectedCameraMode);
	SendMessage(eCameraModeComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)eCurrentlySelectedCameraMode.c_str());
	configurationOpenFile >> eKineticCycleTime;
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	configurationOpenFile >> eAccumulationTime;
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	configurationOpenFile >> eCurrentAccumulationModeTotalAccumulationNumber;
	SendMessage(eSetAccumulationNumberDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentAccumulationModeTotalAccumulationNumber * 1000).c_str());
	configurationOpenFile >> eIncSaveFileNameOption;
	if (eIncSaveFileNameOption)
	{
		CheckDlgButton(eCameraWindowHandle, IDC_INCREMENT_FILE_OPTION_BUTTON, MF_CHECKED);
	}
	else
	{
		CheckDlgButton(eCameraWindowHandle, IDC_INCREMENT_FILE_OPTION_BUTTON, MF_UNCHECKED);
	}
	configurationOpenFile >> ePlottingFrequency;
	SendMessage(ePlottingFrequencyDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePlottingFrequency).c_str());
	configurationOpenFile >> eDetectionThreshold;
	SendMessage(eAtomThresholdDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eDetectionThreshold).c_str());
	int temperature;
	configurationOpenFile >> temperature;
	SendMessage(eTempEditHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(temperature).c_str());
	// slider positions
	for (int sliderInc = 0; sliderInc < eCurrentMaximumPictureCount.size(); sliderInc++)
	{
		configurationOpenFile >> eCurrentMaximumPictureCount[sliderInc];
		configurationOpenFile >> eCurrentMinimumPictureCount[sliderInc];
	}
	// update sliders
	SendMessage(eMaximumPictureSlider1.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[0]);
	SendMessage(eMinimumPictureSlider1.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[0]);
	SendMessage(eMaximumPictureSlider2.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[1]);
	SendMessage(eMinimumPictureSlider2.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[1]);
	SendMessage(eMaximumPictureSlider3.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[2]);
	SendMessage(eMinimumPictureSlider3.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[2]);
	SendMessage(eMaximumPictureSlider4.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[3]);
	SendMessage(eMinimumPictureSlider4.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[3]);
	// update edits
	SendMessage(eMaxSliderNumberEdit1.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMaximumPictureCount[0]).c_str());
	SendMessage(eMinSliderNumberEdit1.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMinimumPictureCount[0]).c_str());
	SendMessage(eMaxSliderNumberEdit2.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMaximumPictureCount[1]).c_str());
	SendMessage(eMinSliderNumberEdit2.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMinimumPictureCount[1]).c_str());
	SendMessage(eMaxSliderNumberEdit3.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMaximumPictureCount[2]).c_str());
	SendMessage(eMinSliderNumberEdit3.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMinimumPictureCount[2]).c_str());
	SendMessage(eMaxSliderNumberEdit4.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMaximumPictureCount[3]).c_str());
	SendMessage(eMinSliderNumberEdit4.hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)std::to_string(eCurrentMinimumPictureCount[3]).c_str());
	/// 
	int plotNumber;
	configurationOpenFile >> plotNumber;
	SendMessage(eCurrentPlotsCombo.hwnd, CB_RESETCONTENT, 0, 0);
	// get the terminating \n.
	configurationOpenFile.get();
	for (int plotInc = 0; plotInc < plotNumber; plotInc++)
	{
		std::string plotName;
		std::getline(configurationOpenFile, plotName);
		SendMessage(eCurrentPlotsCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)plotName.c_str());
	}
	reorganizeWindow(eCurrentlySelectedCameraMode, eCameraWindowHandle);
	updateSaveStatus(true);
	return 0;
}
int FileSystem::saveConfiguration(bool isFromSaveAs)
{
	// check if file exists
	struct stat buffer;
	if (configurationName == "" || (!FileSystem::fileExists(FILE_SYSTEM_PATH + configurationName + ".cConfig") && !isFromSaveAs))
	{
		configurationName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_NAME_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::namePromptDialogProcedure, (LPARAM)"This configuration "
					   "has not been named. Please enter a configuration name.");
		if (configurationName == "")
		{
			// user canceled or didn't enter a name.
			return 0;
		}
	}
	// else open it.
	std::ofstream configurationSaveFile(FILE_SYSTEM_PATH + configurationName + ".cConfig");
	if (!configurationSaveFile.is_open())
	{
		MessageBox(0, "Couldn't save configuration file! Check the name for weird characters, or call Mark if everything seems right...", 0, 0);
		return 0;
	}
	/// Start Outputting information
	// # of exposure times
	configurationSaveFile << std::to_string(eExposureTimes.size()) + "\n";
	// each expousure time
	for (int exposureTimesInc = 0; exposureTimesInc < eExposureTimes.size(); exposureTimesInc++)
	{
		configurationSaveFile << std::to_string(eExposureTimes[exposureTimesInc]) + "\n";
	}
	configurationSaveFile << eCurrentTriggerMode + "\n";
	// Image Parameters
	configurationSaveFile << std::to_string(eLeftImageBorder) + "\n";
	configurationSaveFile << std::to_string(eRightImageBorder) + "\n";
	configurationSaveFile << std::to_string(eHorizontalBinning) + "\n";
	configurationSaveFile << std::to_string(eTopImageBorder) + "\n";
	configurationSaveFile << std::to_string(eBottomImageBorder) + "\n";
	configurationSaveFile << std::to_string(eVerticalBinning) + "\n";
	// gain settings
	configurationSaveFile << std::to_string(eEMGainMode) + "\n";
	configurationSaveFile << std::to_string(eEMGainLevel) + "\n";
	// pictures per experiment
	configurationSaveFile << std::to_string(ePicturesPerExperiment) + "\n";
	// experiment # per stack
	configurationSaveFile << std::to_string(eExperimentsPerStack) + "\n";
	// stack #
	configurationSaveFile << std::to_string(eCurrentAccumulationStackNumber) + "\n";
	// camera mode
	configurationSaveFile << eCurrentlySelectedCameraMode + "\n";
	// Kinetic Cycle Time
	configurationSaveFile << std::to_string(eKineticCycleTime) + "\n";
	// accumulation cycle time
	configurationSaveFile << std::to_string(eAccumulationTime) + "\n";
	// accumulation #
	configurationSaveFile << std::to_string(eCurrentAccumulationModeTotalAccumulationNumber) + "\n";
	// data File increment option
	configurationSaveFile << std::to_string(eIncSaveFileNameOption) + "\n";
	// plotting frequency
	configurationSaveFile << std::to_string(ePlottingFrequency) + "\n";
	// atom threshold
	configurationSaveFile << std::to_string(eDetectionThreshold) + "\n";
	// temperature
	configurationSaveFile << std::to_string(eCameraTemperatureSetting) + "\n";
	// slider positions
	for (int sliderInc = 0; sliderInc < eCurrentMaximumPictureCount.size(); sliderInc++)
	{
		configurationSaveFile << std::to_string(eCurrentMaximumPictureCount[sliderInc]) + "\n";
		configurationSaveFile << std::to_string(eCurrentMinimumPictureCount[sliderInc]) + "\n";
	}

	int count = SendMessage(eCurrentPlotsCombo.hwnd, CB_GETCOUNT, 0, 0);
	configurationSaveFile << std::to_string(count) + "\n";
	for (int plotInc = 0; plotInc < count; plotInc++)
	{
		TCHAR text[256];
		SendMessage(eCurrentPlotsCombo.hwnd, CB_GETLBTEXT, plotInc, (LPARAM)text);
		configurationSaveFile << std::string(text) + "\n";
	}
	FileSystem::reloadCombo(configurationName);
	updateSaveStatus(true);
	return 0;
}
int FileSystem::saveConfigurationAs(std::string newConfigurationName)
{
	configurationName = newConfigurationName; 
	if (configurationName == "")
	{
		// canceled
		return 0;
	}
	FileSystem::saveConfiguration(true);
	FileSystem::reloadCombo(configurationName);
	return 0;
}
int FileSystem::renameConfiguration(std::string newConfigurationName)
{
	std::string tempConfigurationName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_NAME_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::namePromptDialogProcedure, (LPARAM)"Please enter a configuration name.");
	if (tempConfigurationName == "")
	{
		// canceled
		return 0;
	}
	// TODO: move the configuration file
	MoveFile((FILE_SYSTEM_PATH + configurationName + ".cConfig").c_str(), (FILE_SYSTEM_PATH + tempConfigurationName + ".cConfig").c_str());
	configurationName = tempConfigurationName;
	FileSystem::reloadCombo(configurationName);
	return 0;
}
int FileSystem::deleteConfiguration()
{
	int answer = MessageBox(0, ("Are you sure that you want to delete the following configuraiton: " + configurationName).c_str(), 0, MB_OKCANCEL);
	if (answer == IDOK)
	{
		int result = DeleteFile((FILE_SYSTEM_PATH + configurationName + ".cConfig").c_str());
		if (!result)
		{
			MessageBox(0, ("ERROR: Couldn't delete configuration File. Error code: " + std::to_string(GetLastError()) + ". Talk to Mark or try to delete it "
						   "yourself from the appropriate folder in explorer.").c_str(), 0, 0);
		}
		configurationName = "";
	}
	FileSystem::reloadCombo("__NONE__");
	// no configuration loaded so don't want save prompt
	updateSaveStatus(true);
	return 0;
}
int FileSystem::checkSave()
{
	if (!configurationSaved)
	{
		// ask the user if they want to save
		return MessageBox(0, "Save Current Camera Configuration First?", 0, MB_YESNOCANCEL);
	}
	else
	{
		return IDNO;
	}
}

int FileSystem::initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous, HWND parentWindow, bool isTriggerModeSensitive)
{
	configLabel.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 100, topLeftPositionKinetic.y + 25 };
	configLabel.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 100, 
									  topLeftPositionAccumulate.y + 25 };
	configLabel.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 100, 
												 topLeftPositionContinuous.y + 25 };
	RECT initPos = configLabel.kineticSeriesModePos;
	configLabel.hwnd = CreateWindowEx(0, "STATIC", "Configuration: ", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	configLabel.fontType = "Normal";

	/// CAMERA MODE
	configCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 100, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, 
		topLeftPositionKinetic.y + 800 };
	configCombo.accumulateModePos = { topLeftPositionAccumulate.x + 100, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480,
		topLeftPositionAccumulate.y + 800 };
	configCombo.continuousSingleScansModePos = { topLeftPositionContinuous.x + 100, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480,
		topLeftPositionContinuous.y + 800 };
	initPos = configCombo.kineticSeriesModePos;
	configCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, (HMENU)IDC_CONFIGURATION_COMBO, eHInst, NULL);
	// add options
	configCombo.fontType = "Normal";
	FileSystem::reloadCombo("__NONE__");
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	updateSaveStatus(true);
	return 0;
}

int FileSystem::reorganizeControls(RECT parentRectangle, std::string mode)
{
	reorganizeControl(configLabel, mode, parentRectangle);
	reorganizeControl(configCombo, mode, parentRectangle);
	return 0;
}

std::vector<std::string> FileSystem::searchForFiles(std::string locationToSearch, std::string extensions)
{
	// Re-add the entries back in and figure out which one is the current one.
	std::vector<std::string> names;
	std::string search_path = locationToSearch + "\\" + extensions;
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	if (extensions == "*")
	{
		hFind = FindFirstFileEx(search_path.c_str(), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0);
	}
	else
	{
		hFind = FindFirstFile(search_path.c_str(), &fd);
	}
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// if looking for folders
			if (extensions == "*")
			{
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					if (std::string(fd.cFileName) != "." && std::string(fd.cFileName) != "..")
					{
						names.push_back(fd.cFileName);
					}
				}
			}
			else
			{
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					names.push_back(fd.cFileName);
				}
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	// Remove suffix from file names and...
	for (int configListInc = 0; configListInc < names.size(); configListInc++)
	{
		if (extensions == "*" || extensions == "*.*" || extensions == "*.hSubConfig" || extensions == "*.vSubConfig" || extensions == "*.seq" 
			|| extensions == "*.cConfig")
		{
			names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - (extensions.size() - 1));
		}
		else
		{
			names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - extensions.size());
		}
	}
	// Make the final vector out of the unique objects left.
	return names;
}
int FileSystem::reloadCombo(std::string nameToLoad)
{
	std::vector<std::string> names;
	// search for folders
	names = FileSystem::searchForFiles(FILE_SYSTEM_PATH, "*.cConfig");

	/// Get current selection
	long long itemIndex = SendMessage(configCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR experimentConfigToOpen[256];
	std::string currentSelection;
	int currentInc = -1;
	if (itemIndex != -1)
	{
		// Send CB_GETLBTEXT message to get the item.
		SendMessage(configCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
		currentSelection = experimentConfigToOpen;
	}
	/// Reset stuffs
	SendMessage(configCombo.hwnd, CB_RESETCONTENT, 0, 0);
	// Send list to object
	for (int comboInc = 0; comboInc < names.size(); comboInc++)
	{
		if (nameToLoad == names[comboInc])
		{
			currentInc = comboInc;
		}
		SendMessage(configCombo.hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(names[comboInc].c_str())));
	}
	// Set initial value
	SendMessage(configCombo.hwnd, CB_SETCURSEL, currentInc, 0);
	return 0;
}

bool FileSystem::fileExists(std::string filePathway)
{
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(filePathway.c_str(), &buffer) == 0);
}
std::string FileSystem::getComboText()
{
	int selectionNum = SendMessage(configCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (selectionNum == -1)
	{
		return "";
	}
	else
	{
		TCHAR text[256];	
		SendMessage(configCombo.hwnd, CB_GETLBTEXT, selectionNum, (LPARAM)text);
		return text;
	}
}

void FileSystem::updateSaveStatus(bool saved)
{
	configurationSaved = saved;
}