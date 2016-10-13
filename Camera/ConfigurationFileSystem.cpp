#include "stdafx.h"
#include "ConfigurationFileSystem.h"
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

ConfigurationFileSystem::ConfigurationFileSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
}
ConfigurationFileSystem::~ConfigurationFileSystem()
{
	// nothing for destructor right now
}

int ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen)
{
	if (configurationNameToOpen == "" || !ConfigurationFileSystem::fileExists(FILE_SYSTEM_PATH + configurationNameToOpen + ".cConfig"))
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
	// first thing may or may not be a version number.
	std::string test;
	configurationOpenFile >> test;
	std::string version;
	int numberOfExposureTimes;
	if (test == "v1.1")
	{
		version = test;
		configurationOpenFile >> numberOfExposureTimes;
	}
	else
	{
		numberOfExposureTimes = std::stoi(test);
	}

	/// Exposure Times
	std::vector<float> times;
	if (numberOfExposureTimes < 1)
	{
		times.resize(0);
	}
	else
	{
		times.resize(numberOfExposureTimes);
	}
	for (int exposureInc = 0; exposureInc < times.size(); exposureInc++)
	{
		configurationOpenFile >> times[exposureInc];
	}
	/// 

	configurationOpenFile.get();
	std::getline(configurationOpenFile, eCurrentTriggerMode);
	SendMessage(eTriggerComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)eCurrentTriggerMode.c_str());
	imageParameters tempParam;
	configurationOpenFile >> tempParam.leftBorder;
	configurationOpenFile >> tempParam.rightBorder;
	configurationOpenFile >> tempParam.horizontalBinning;
	configurationOpenFile >> tempParam.topBorder;
	configurationOpenFile >> tempParam.bottomBorder;
	configurationOpenFile >> tempParam.verticalBinning;
	eImageControl.setImageParametersFromInput(tempParam);

	///
	configurationOpenFile >> eEMGainMode;
	configurationOpenFile >> eEMGainLevel;
	if (eEMGainMode == false || eEMGainLevel < 0)
	{
		SendMessage(eEMGainDisplay.hwnd, WM_SETTEXT, 0, (LPARAM)"OFF");
		SendMessage(eEMGainEdit.hwnd, WM_SETTEXT, 0, (LPARAM)"-1");
	}
	else
	{
		SendMessage(eEMGainDisplay.hwnd, WM_SETTEXT, 0, (LPARAM)("X" + std::to_string(eEMGainLevel)).c_str());
		SendMessage(eEMGainEdit.hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(eEMGainLevel)).c_str());
	}
	myAndor::setGainMode();

	configurationOpenFile >> ePicturesPerRepetition;
	ePictureOptionsControl.setPicturesPerExperiment(ePicturesPerRepetition);
	// try to set this time.
	try
	{
		ePictureOptionsControl.setExposureTimes(times);
	}
	catch (std::runtime_error)
	{
		appendText("ERROR: failed to set exposure times.", IDC_ERROR_EDIT);
		return -1;
	}
	// now check actual times.
	try
	{
		ePictureOptionsControl.confirmAcquisitionTimings();
	}
	catch (std::runtime_error)
	{
		appendText("ERROR: Unable to check acquisition timings.\r\n", IDC_ERROR_EDIT);
		throw;
	}
	// now output things.
	if (ePictureOptionsControl.getUsedExposureTimes().size() <= 0)
	{
		// this shouldn't happend
		appendText("ERROR: reached bad location where eExposureTimes was of zero size, but this should have been detected earlier in the code.", IDC_ERROR_EDIT);
		return -1;
	}
	/// 
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	configurationOpenFile >> eRepetitionsPerVariation;
	ePicturesPerVariation = ePicturesPerRepetition * eRepetitionsPerVariation;
	SendMessage(eRepetitionsPerVariationDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eRepetitionsPerVariation).c_str());
	SendMessage(eRepetitionsPerVariationEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eRepetitionsPerVariation).c_str());
	configurationOpenFile >> eCurrentTotalVariationNumber;
	
	SendMessage(eVariationNumberDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentTotalVariationNumber).c_str());
	SendMessage(eVariationNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentTotalVariationNumber).c_str());
	eTotalNumberOfPicturesInSeries = eCurrentTotalVariationNumber * ePicturesPerVariation;
	// get \n at end of previous line
	configurationOpenFile.get();
	std::getline(configurationOpenFile, eCurrentlySelectedCameraMode);
	//SendMessage(eCameraModeComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)eCurrentlySelectedCameraMode.c_str());
	if (eCurrentlySelectedCameraMode == "Continuous Single Scans Mode")
	{
		eAcquisitionMode = 5;
		if (ePicturesPerVariation != INT_MAX)
		{
			ePreviousPicturesPerSubSeries = ePicturesPerVariation;
		}
		ePicturesPerVariation = INT_MAX;
		SendMessage(eCameraModeComboHandle.hwnd, CB_SETCURSEL, 2, 0);
		//SendMessage(.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerVariation).c_str());
	}
	else if (eCurrentlySelectedCameraMode == "Kinetic Series Mode")
	{
		eAcquisitionMode = 3;
		SendMessage(eCameraModeComboHandle.hwnd, CB_SETCURSEL, 0, 0);
	}
	else if (eCurrentlySelectedCameraMode == "Accumulate Mode")
	{
		eAcquisitionMode = 2;
		if (ePicturesPerVariation != INT_MAX)
		{
			ePreviousPicturesPerSubSeries = ePicturesPerVariation;
		}
		ePicturesPerVariation = INT_MAX;
		SendMessage(eCameraModeComboHandle.hwnd, CB_SETCURSEL, 1, 0);
		//SendMessage(ePicturesPerRepetitionDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerVariation).c_str());
	}
	configurationOpenFile >> eKineticCycleTime;
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	SendMessage(eKineticCycleTimeEditHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	configurationOpenFile >> eAccumulationTime;
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	SendMessage(eAccumulationTimeEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	configurationOpenFile >> eCurrentAccumulationModeTotalAccumulationNumber;
	SendMessage(eSetAccumulationNumberDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentAccumulationModeTotalAccumulationNumber * 1000).c_str());
	SendMessage(eAccumulationNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentAccumulationModeTotalAccumulationNumber * 1000).c_str());
	if (version == "")
	{
		std::string trash;
		configurationOpenFile >> trash;
	}
	configurationOpenFile >> ePlottingFrequency;
	SendMessage(ePlottingFrequencyDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePlottingFrequency).c_str());
	SendMessage(ePlottingFrequencyEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePlottingFrequency).c_str());
	std::array<int, 4> thresholds;
	if (version == "v1.1")
	{
		configurationOpenFile >> thresholds[0];
		configurationOpenFile >> thresholds[1];
		configurationOpenFile >> thresholds[2];
		configurationOpenFile >> thresholds[3];

	}
	else
	{
		// there was only one threshold in the original version.
		int threshold;
		configurationOpenFile >> threshold;
		thresholds[3] = thresholds[2] = thresholds[1] = thresholds[0] = threshold;
	}
	ePictureOptionsControl.setThresholds(thresholds);

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

int ConfigurationFileSystem::saveConfiguration(bool isFromSaveAs)
{
	// check if file exists
	struct stat buffer;
	if (configurationName == "" || (!ConfigurationFileSystem::fileExists(FILE_SYSTEM_PATH + configurationName + ".cConfig") && !isFromSaveAs))
	{
		configurationName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)"This configuration "
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
	// version
	configurationSaveFile << "v1.1\n";
	// # of exposure times
	configurationSaveFile << std::to_string(ePictureOptionsControl.getUsedExposureTimes().size()) + "\n";
	// each expousure time
	for (int exposureTimesInc = 0; exposureTimesInc < ePictureOptionsControl.getUsedExposureTimes().size(); exposureTimesInc++)
	{
		configurationSaveFile << std::to_string(ePictureOptionsControl.getUsedExposureTimes()[exposureTimesInc]) + "\n";
	}
	configurationSaveFile << eCurrentTriggerMode + "\n";
	imageParameters tempParam = eImageControl.getImageParameters();
	// Image Parameters
	configurationSaveFile << std::to_string(tempParam.leftBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.rightBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.horizontalBinning) + "\n";
	configurationSaveFile << std::to_string(tempParam.topBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.bottomBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.verticalBinning) + "\n";
	// gain settings
	configurationSaveFile << std::to_string(eEMGainMode) + "\n";
	configurationSaveFile << std::to_string(eEMGainLevel) + "\n";
	// pictures per experiment
	configurationSaveFile << std::to_string(ePicturesPerRepetition) + "\n";
	// experiment # per stack
	configurationSaveFile << std::to_string(eRepetitionsPerVariation) + "\n";
	// stack #
	configurationSaveFile << std::to_string(eCurrentTotalVariationNumber) + "\n";
	// camera mode
	configurationSaveFile << eCurrentlySelectedCameraMode + "\n";
	// Kinetic Cycle Time
	configurationSaveFile << std::to_string(eKineticCycleTime) + "\n";
	// accumulation cycle time
	configurationSaveFile << std::to_string(eAccumulationTime) + "\n";
	// accumulation #
	configurationSaveFile << std::to_string(eCurrentAccumulationModeTotalAccumulationNumber) + "\n";
	// plotting frequency
	configurationSaveFile << std::to_string(ePlottingFrequency) + "\n";
	// atom threshold
	for (int thresholdInc = 0; thresholdInc < ePictureOptionsControl.getThresholds().size(); thresholdInc++)
	{
		configurationSaveFile << std::to_string(ePictureOptionsControl.getThresholds()[thresholdInc]) + "\n";
	}
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
	ConfigurationFileSystem::reloadCombo(configurationName);
	updateSaveStatus(true);
	return 0;
}
int ConfigurationFileSystem::saveConfigurationAs(std::string newConfigurationName)
{
	configurationName = newConfigurationName; 
	if (configurationName == "")
	{
		// canceled
		return 0;
	}
	ConfigurationFileSystem::saveConfiguration(true);
	ConfigurationFileSystem::reloadCombo(configurationName);
	return 0;
}
int ConfigurationFileSystem::renameConfiguration(std::string newConfigurationName)
{
	std::string tempConfigurationName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)"Please enter a configuration name.");
	if (tempConfigurationName == "")
	{
		// canceled
		return 0;
	}
	// TODO: move the configuration file
	MoveFile((FILE_SYSTEM_PATH + configurationName + ".cConfig").c_str(), (FILE_SYSTEM_PATH + tempConfigurationName + ".cConfig").c_str());
	configurationName = tempConfigurationName;
	ConfigurationFileSystem::reloadCombo(configurationName);
	return 0;
}
int ConfigurationFileSystem::deleteConfiguration()
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
	ConfigurationFileSystem::reloadCombo("__NONE__");
	// no configuration loaded so don't want save prompt
	updateSaveStatus(true);
	return 0;
}
int ConfigurationFileSystem::checkSave()
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

int ConfigurationFileSystem::initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous, HWND parentWindow, bool isTriggerModeSensitive)
{
	configLabel.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 150, topLeftPositionKinetic.y + 25 };
	configLabel.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 150, 
									  topLeftPositionAccumulate.y + 25 };
	configLabel.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 150, 
												 topLeftPositionContinuous.y + 25 };
	RECT initPos = configLabel.kineticSeriesModePos;
	configLabel.hwnd = CreateWindowEx(0, "STATIC", "Configuration: ", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	configLabel.fontType = "Normal";

	/// CAMERA MODE
	configCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 150, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, 
		topLeftPositionKinetic.y + 800 };
	configCombo.accumulateModePos = { topLeftPositionAccumulate.x + 150, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480,
		topLeftPositionAccumulate.y + 800 };
	configCombo.continuousSingleScansModePos = { topLeftPositionContinuous.x + 150, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480,
		topLeftPositionContinuous.y + 800 };
	initPos = configCombo.kineticSeriesModePos;
	configCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, (HMENU)IDC_CONFIGURATION_COMBO, eHInst, NULL);
	// add options
	configCombo.fontType = "Normal";
	ConfigurationFileSystem::reloadCombo("__NONE__");
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	updateSaveStatus(true);
	return 0;
}

int ConfigurationFileSystem::reorganizeControls(RECT parentRectangle, std::string mode)
{
	reorganizeControl(configLabel, mode, parentRectangle);
	reorganizeControl(configCombo, mode, parentRectangle);
	return 0;
}

std::vector<std::string> ConfigurationFileSystem::searchForFiles(std::string locationToSearch, std::string extensions)
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
int ConfigurationFileSystem::reloadCombo(std::string nameToLoad)
{
	std::vector<std::string> names;
	// search for folders
	names = ConfigurationFileSystem::searchForFiles(FILE_SYSTEM_PATH, "*.cConfig");

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

bool ConfigurationFileSystem::fileExists(std::string filePathway)
{
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(filePathway.c_str(), &buffer) == 0);
}
std::string ConfigurationFileSystem::getComboText()
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

void ConfigurationFileSystem::updateSaveStatus(bool saved)
{
	configurationSaved = saved;
}