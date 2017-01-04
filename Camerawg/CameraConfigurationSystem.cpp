#include "stdafx.h"
#include "CameraConfigurationSystem.h"
#include "Windows.h"
#include <fstream>
#include "externals.h"
#include "constants.h"
#include "reorganizeControl.h"
#include "Resource.h"
#include "reorganizeWindow.h"
#include "Commctrl.h"
#include "CameraWindow.h"


CameraConfigurationSystem::CameraConfigurationSystem(std::string fileSystemPath)
{
	FILE_SYSTEM_PATH = fileSystemPath;
}
CameraConfigurationSystem::~CameraConfigurationSystem()
{
	// nothing for destructor right now
}

// the idea here is to input the current settings. any settings that are not part of the configuration therefore are not overwritten.
AndorRunSettings CameraConfigurationSystem::openConfiguration(std::string configurationNameToOpen, AndorRunSettings settings)
{
	if (configurationNameToOpen == "" || !CameraConfigurationSystem::fileExists(FILE_SYSTEM_PATH + configurationNameToOpen + ".cConfig"))
	{
		// can't open this.
		return settings;
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
	std::getline(configurationOpenFile, settings.triggerMode );
	//SendMessage(eTriggerComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)eCurrentTriggerMode.c_str());
	imageParameters tempParam;
	configurationOpenFile >> tempParam.leftBorder;
	configurationOpenFile >> tempParam.rightBorder;
	configurationOpenFile >> tempParam.horizontalBinning;
	configurationOpenFile >> tempParam.topBorder;
	configurationOpenFile >> tempParam.bottomBorder;
	configurationOpenFile >> tempParam.verticalBinning;
	settings.imageSettings = tempParam;
	//eImageControl.setImageParametersFromInput(tempParam);

	///
	configurationOpenFile >> settings.emGainModeIsOn;
	configurationOpenFile >> settings.emGainLevel;
	/*
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
	*/
	configurationOpenFile >> settings.picsPerRepetition;
	//ePictureOptionsControl.setPicturesPerExperiment(ePicturesPerRepetition);
	// try to set this time.
	/*
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
	*/
	/// 
	//SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	//SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	configurationOpenFile >> settings.repetitionsPerVariation;
	settings.totalPicsInVariation = settings.picsPerRepetition * settings.repetitionsPerVariation;
	//SendMessage(eRepetitionsPerVariationDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eRepetitionsPerVariation).c_str());
	//SendMessage(eRepetitionsPerVariationEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eRepetitionsPerVariation).c_str());
	configurationOpenFile >> settings.totalVariations;
	//SendMessage(eVariationNumberDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentTotalVariationNumber).c_str());
	//SendMessage(eVariationNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentTotalVariationNumber).c_str());
	settings.totalPicsInExperiment = settings.totalVariations * settings.totalPicsInVariation;
	// get \n at end of previous line
	configurationOpenFile.get();
	std::getline(configurationOpenFile, settings.cameraMode );
	//SendMessage(eCameraModeComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)eCurrentlySelectedCameraMode.c_str());
	if (settings.cameraMode == "Continuous Single Scans Mode")
	{
		settings.acquisitionMode = 5;
		/*
		if (ePicturesPerVariation != INT_MAX)
		{
			ePreviousPicturesPerSubSeries = ePicturesPerVariation;
		}
		*/
		settings.totalPicsInVariation = INT_MAX;
		//SendMessage(.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerVariation).c_str());
	}
	else if (settings.cameraMode == "Kinetic Series Mode")
	{
		settings.acquisitionMode = 3;
	}
	else if (settings.cameraMode == "Accumulate Mode")
	{
		settings.acquisitionMode = 2;
		/*
		if (ePicturesPerVariation != INT_MAX)
		{
			ePreviousPicturesPerSubSeries = ePicturesPerVariation;
		}
		*/
		settings.totalPicsInVariation = INT_MAX;
		//SendMessage(ePicturesPerRepetitionDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerVariation).c_str());
	}
	configurationOpenFile >> settings.kinetiCycleTime;
	//SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	//SendMessage(eKineticCycleTimeEditHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	configurationOpenFile >> settings.accumulationTime;
	//SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	//SendMessage(eAccumulationTimeEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	configurationOpenFile >> settings.totalAccumulationNumber;
	//SendMessage(eSetAccumulationNumberDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentAccumulationModeTotalAccumulationNumber * 1000).c_str());
	//SendMessage(eAccumulationNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eCurrentAccumulationModeTotalAccumulationNumber * 1000).c_str());
	// I don't remember what this was...
	if (version == "")
	{
		std::string trash;
		configurationOpenFile >> trash;
	}
	//configurationOpenFile >> ePlottingFrequency;
	//SendMessage(ePlottingFrequencyDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePlottingFrequency).c_str());
	//SendMessage(ePlottingFrequencyEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePlottingFrequency).c_str());
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
	//ePictureOptionsControl.setThresholds(thresholds);
	configurationOpenFile >> settings.temperatureSetting;
	//SendMessage(eTempEditHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(temperature).c_str());
	// slider positions
	/*
	for (int sliderInc = 0; sliderInc < eCurrentMaximumPictureCount.size(); sliderInc++)
	{
		configurationOpenFile >> eCurrentMaximumPictureCount[sliderInc];
		configurationOpenFile >> eCurrentMinimumPictureCount[sliderInc];
	}
	*/
	// update sliders
	/*
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
	*/
	/// 
	int plotNumber;
	configurationOpenFile >> plotNumber;
	//SendMessage(eCurrentPlotsCombo.hwnd, CB_RESETCONTENT, 0, 0);
	// get the terminating \n.
	configurationOpenFile.get();
	for (int plotInc = 0; plotInc < plotNumber; plotInc++)
	{
		std::string plotName;
		std::getline(configurationOpenFile, plotName);
		//SendMessage(eCurrentPlotsCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)plotName.c_str());
	}
	//reorganizeWindow(eCurrentlySelectedCameraMode, eCameraWindowHandle);
	updateSaveStatus(true);
	return settings;
}

void CameraConfigurationSystem::saveConfiguration(bool isFromSaveAs, AndorRunSettings settings )
{
	// check if file exists
	struct stat buffer;
	if (configurationName == "" || (!CameraConfigurationSystem::fileExists(FILE_SYSTEM_PATH + configurationName + ".cConfig") && !isFromSaveAs))
	{
		//configurationName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)"This configuration "
		//			   "has not been named. Please enter a configuration name.");
		if (configurationName == "")
		{
			// user canceled or didn't enter a name.
			return;
		}
	}
	// else open it.
	std::ofstream configurationSaveFile(FILE_SYSTEM_PATH + configurationName + ".cConfig");
	if (!configurationSaveFile.is_open())
	{
		errBox("Couldn't save configuration file! Check the name for weird characters, or call Mark if everything seems right...");
		return;
	}
	/// Start Outputting information
	// version
	configurationSaveFile << "v1.1\n";
	// # of exposure times
	configurationSaveFile << std::to_string( settings.exposureTimes.size()) + "\n";
	// each expousure time
	for (int exposureTimesInc = 0; exposureTimesInc < settings.exposureTimes.size(); exposureTimesInc++)
	{
		configurationSaveFile << std::to_string( settings.exposureTimes[exposureTimesInc]) + "\n";
	}
	configurationSaveFile << settings.triggerMode + "\n";
	imageParameters tempParam = settings.imageSettings;
	// Image Parameters
	configurationSaveFile << std::to_string(tempParam.leftBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.rightBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.horizontalBinning) + "\n";
	configurationSaveFile << std::to_string(tempParam.topBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.bottomBorder) + "\n";
	configurationSaveFile << std::to_string(tempParam.verticalBinning) + "\n";
	// gain settings
	configurationSaveFile << std::to_string( settings.emGainModeIsOn) + "\n";
	configurationSaveFile << std::to_string( settings.emGainLevel) + "\n";
	// pictures per experiment
	configurationSaveFile << std::to_string( settings.picsPerRepetition) + "\n";
	// experiment # per stack
	configurationSaveFile << std::to_string( settings.repetitionsPerVariation) + "\n";
	// stack #
	configurationSaveFile << std::to_string( settings.totalVariations) + "\n";
	// camera mode
	configurationSaveFile << settings.cameraMode + "\n";
	// Kinetic Cycle Time
	configurationSaveFile << std::to_string( settings.kinetiCycleTime ) + "\n";
	// accumulation cycle time
	configurationSaveFile << std::to_string( settings.accumulationTime ) + "\n";
	// accumulation #
	configurationSaveFile << std::to_string( settings.totalAccumulationNumber) + "\n";
	// plotting frequency
	///configurationSaveFile << std::to_string(ePlottingFrequency) + "\n";
	// atom threshold
	///for (int thresholdInc = 0; thresholdInc < ePictureOptionsControl.getThresholds().size(); thresholdInc++)
	///{
	///	configurationSaveFile << std::to_string(ePictureOptionsControl.getThresholds()[thresholdInc]) + "\n";
	///}
	// temperature
	configurationSaveFile << std::to_string( settings.temperatureSetting ) + "\n";
	// slider positions
	///for (int sliderInc = 0; sliderInc < eCurrentMaximumPictureCount.size(); sliderInc++)
	///{
	///	configurationSaveFile << std::to_string(eCurrentMaximumPictureCount[sliderInc]) + "\n";
	///	configurationSaveFile << std::to_string(eCurrentMinimumPictureCount[sliderInc]) + "\n";
	///}

	///int count = SendMessage(eCurrentPlotsCombo.hwnd, CB_GETCOUNT, 0, 0);
	///configurationSaveFile << std::to_string(count) + "\n";
	///for (int plotInc = 0; plotInc < count; plotInc++)
	///{
	///	TCHAR text[256];
	///	SendMessage(eCurrentPlotsCombo.hwnd, CB_GETLBTEXT, plotInc, (LPARAM)text);
	///	configurationSaveFile << std::string(text) + "\n";
	///}
	CameraConfigurationSystem::reloadCombo(configurationName);
	updateSaveStatus(true);
	return;
}


void CameraConfigurationSystem::saveConfigurationAs(std::string newConfigurationName, AndorRunSettings settings )
{
	configurationName = newConfigurationName; 
	if (configurationName == "")
	{
		// canceled
		return;
	}
	CameraConfigurationSystem::saveConfiguration(true, settings );
	CameraConfigurationSystem::reloadCombo(configurationName);
	return;
}
int CameraConfigurationSystem::renameConfiguration(std::string newConfigurationName)
{
	std::string tempConfigurationName;
	//std::string tempConfigurationName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)"Please enter a configuration name.");
	//if (tempConfigurationName == "")
	//{
	//	// canceled
	//	return 0;
	//}
	// TODO: move the configuration file
	MoveFile((FILE_SYSTEM_PATH + configurationName + ".cConfig").c_str(), (FILE_SYSTEM_PATH + tempConfigurationName + ".cConfig").c_str());
	configurationName = tempConfigurationName;
	CameraConfigurationSystem::reloadCombo(configurationName);
	return 0;
}
int CameraConfigurationSystem::deleteConfiguration()
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
	CameraConfigurationSystem::reloadCombo("__NONE__");
	// no configuration loaded so don't want save prompt
	updateSaveStatus(true);
	return 0;
}
int CameraConfigurationSystem::checkSave()
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

void CameraConfigurationSystem::initialize( cameraPositions& positions, CWnd* parent, bool isTriggerModeSensitive, int& id)
{
	configLabel.ksmPos = { positions.ksmPos.x, positions.ksmPos.y, positions.ksmPos.x + 150, positions.ksmPos.y + 25 };
	configLabel.amPos = { positions.amPos.x, positions.amPos.y, positions.amPos.x + 150, 
									  positions.amPos.y + 25 };
	configLabel.cssmPos = { positions.cssmPos.x, positions.cssmPos.y, positions.cssmPos.x + 150, 
												 positions.cssmPos.y + 25 };
	configLabel.ID = id++;
	configLabel.Create( "Configuration: ", WS_CHILD | WS_VISIBLE | ES_CENTER, configLabel.ksmPos, parent, configLabel.ID );
	configLabel.fontType = "Normal";

	/// CAMERA MODE
	configCombo.ksmPos = { positions.ksmPos.x + 150, positions.ksmPos.y, positions.ksmPos.x + 480, 
		positions.ksmPos.y + 800 };
	configCombo.amPos = { positions.amPos.x + 150, positions.amPos.y, positions.amPos.x + 480,
		positions.amPos.y + 800 };
	configCombo.cssmPos = { positions.cssmPos.x + 150, positions.cssmPos.y, positions.cssmPos.x + 480,
		positions.cssmPos.y + 800 };
	configCombo.ID = id++;
	configCombo.Create( WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, configCombo.ksmPos, parent, configCombo.ID );
	// add options
	configCombo.fontType = "Normal";
	CameraConfigurationSystem::reloadCombo("__NONE__");
	positions.ksmPos.y += 25;
	positions.amPos.y += 25;
	positions.cssmPos.y += 25;
	updateSaveStatus(true);
	return;
}

void CameraConfigurationSystem::reorganizeControls(RECT parentRectangle, std::string mode)
{
	//configLabel.rearrange();
	//configCombo.rearrange();
	return ;
}

std::vector<std::string> CameraConfigurationSystem::searchForFiles(std::string locationToSearch, std::string extensions)
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


void CameraConfigurationSystem::reloadCombo(std::string nameToLoad)
{
	std::vector<std::string> names;
	// search for folders
	names = CameraConfigurationSystem::searchForFiles(FILE_SYSTEM_PATH, "*.cConfig");

	/// Get current selection
	long long itemIndex = configCombo.GetCurSel();
	CString experimentConfigToOpen;
	std::string currentSelection;
	int currentInc = -1;
	if (itemIndex != -1)
	{
		// Send CB_GETLBTEXT message to get the item.
		configCombo.GetLBText( itemIndex, experimentConfigToOpen );
		currentSelection = experimentConfigToOpen;
	}
	/// Reset stuffs
	configCombo.ResetContent();
	// Send list to object
	for (int comboInc = 0; comboInc < names.size(); comboInc++)
	{
		if (nameToLoad == names[comboInc])
		{
			currentInc = comboInc;
		}
		configCombo.AddString( names[comboInc].c_str() );
	}
	// Set initial value
	configCombo.SetCurSel( currentInc );
	return;
}

bool CameraConfigurationSystem::fileExists(std::string filePathway)
{
	// got this from stack exchange. dunno how it works but it should be fast.
	struct stat buffer;
	return (stat(filePathway.c_str(), &buffer) == 0);
}
std::string CameraConfigurationSystem::getComboText()
{

	int selectionNum = configCombo.GetCurSel();
	if (selectionNum == -1)
	{
		return "";
	}
	else
	{
		CString text;
		configCombo.GetLBText( selectionNum, text );
		return text;
	}
}

void CameraConfigurationSystem::updateSaveStatus(bool saved)
{
	configurationSaved = saved;
}