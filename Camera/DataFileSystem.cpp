#include "stdafx.h"
#include "DataFileSystem.h"
#include "appendText.h"
#include "externals.h"


DataFileSystem::DataFileSystem(std::string systemLocation)
{
	// initialize this to false.
	fitsIsOpen = false;
	dataFilesBaseLocation = systemLocation;
}

// this file assumes that fits is the fits_#.fits file. User should check if incDataSet is on before calling. ???
bool DataFileSystem::deleteFitsAndKey(std::string& errMsg)
{
	errMsg = "";
	if (fitsIsOpen)
	{
		errMsg = "ERROR: Can't delete current fits file, the fits file is open!";
		return true;
	}
	std::string fitsAddress = SAVE_BASE_ADDRESS + currentSaveFolder + "\\Raw Data\\data_" + std::to_string(currentDataFileNumber) + ".fits";
	int success = DeleteFile(fitsAddress.c_str());
	bool returnVal = false;
	if (success == false)
	{
		errMsg = "Failed to delete fits file! Error code: " + std::to_string(GetLastError()) + ".\r\n";
		returnVal = true;
	}
	else
	{
		appendText("Deleted Fits file located at \"" + fitsAddress + "\"\r\n", IDC_STATUS_EDIT);
	}
	success = DeleteFile((SAVE_BASE_ADDRESS + currentSaveFolder + "\\Raw Data\\key_" + std::to_string(currentDataFileNumber) + ".txt").c_str());
	if (success == false)
	{
		errMsg += "Failed to delete key file! Error code: " + std::to_string(GetLastError()) + ".\r\n";
		returnVal = true;
	}
	else
	{
		appendText("Deleted Key file located at \"" + SAVE_BASE_ADDRESS + currentSaveFolder + "\\Raw Data\\key_" + std::to_string(currentDataFileNumber) 
				   + ".txt\"\r\n", IDC_STATUS_EDIT);
	}
	return returnVal;
}


bool DataFileSystem::loadAndMoveKeyFile(std::string& errMsg)
{
	int result = 0;
	result = CopyFile((KEY_FILE_LOCATION + "key.txt").c_str(), (SAVE_BASE_ADDRESS + currentSaveFolder 
		+ "\\Raw Data\\key_" + std::to_string(currentDataFileNumber) + ".txt").c_str(), FALSE);
	if (result == 0)
	{
		// failed
		errMsg = "Failed to create copy of key file! Error Code: " + std::to_string(GetLastError()) + "\r\n";
		// but continue, no breaking...
		return true;
	}

	std::ifstream keyFile;
	keyFile.open(SAVE_BASE_ADDRESS + currentSaveFolder + "\\Raw Data\\key_" + std::to_string(currentDataFileNumber) + ".txt");
	if (!keyFile.is_open())
	{
		errMsg = "Couldn't open key file!? Does a key file exist???\r\n";
		return true;
	}
	keyValues.clear();
	std::string keyString;
	std::getline(keyFile, keyString);
	while (keyFile)
	{
		double keyItem;
		try
		{
			keyItem = std::stod(keyString);
			keyValues.push_back(keyItem);
		}
		catch (std::invalid_argument& exception)
		{
			errMsg = "Key File contained bad values!\r\n";
			return true;
		}
		std::getline(keyFile, keyString);
	}
	return false;
}


bool DataFileSystem::forceFitsClosed()
{

	int fitsStatus = 0;
	if (myFitsFile != NULL)
	{
		fits_close_file(myFitsFile, &fitsStatus);
	}
	return false;
}


bool DataFileSystem::initializeDataFiles(std::string& errMsg)
{
	DataFileSystem::forceFitsClosed();
	// if the function fails, the fits file will not be open. If it succeeds, this will get set to true.
	fitsIsOpen = false;
	/// First, create the folder for today's fits data.
	// Get the date and use it to set the folder where this data run will be saved.
	// get time now
	time_t t = time(0);
	struct tm now;
	localtime_s(&now, &t);
	std::string tempStr = std::to_string(now.tm_year + 1900);
	// Create the string of the date.
	std::string finalSaveFolder;
	finalSaveFolder = tempStr[2];
	finalSaveFolder += tempStr[3];
	if (now.tm_mon + 1 < 10)
	{
		finalSaveFolder += "0";
		finalSaveFolder += std::to_string(now.tm_mon + 1);
	}
	else
	{
		finalSaveFolder += std::to_string(now.tm_mon + 1);
	}
	if (now.tm_mday < 10)
	{
		finalSaveFolder += "0";
		finalSaveFolder += std::to_string(now.tm_mday);
	}
	else
	{
		finalSaveFolder += std::to_string(now.tm_mday);
	}
	currentDate = finalSaveFolder;
	// right now the save folder IS the date...
	currentSaveFolder = finalSaveFolder;
	// create date's folder.
	int result = CreateDirectory((SAVE_BASE_ADDRESS + finalSaveFolder).c_str(), 0);
	finalSaveFolder += "\\Raw Data";
	int result2 = CreateDirectory((SAVE_BASE_ADDRESS + finalSaveFolder).c_str(), 0);
	finalSaveFolder += "\\";
	/// Get a filename appropriate for the data
	std::string finalSaveFileName;

	// find the first data file that hasn't been already written, starting with data_1.fits
	int fileNum = 1;
	// The while condition here check if file exists. No idea how this actually works.
	struct stat statBuffer;
	while ((stat((SAVE_BASE_ADDRESS + finalSaveFolder + "\\data_" + std::to_string(fileNum) + ".fits").c_str(), &statBuffer) == 0))
	{
		fileNum++;
	}
	// at this point a valid filename has been found.
	finalSaveFileName = "data_" + std::to_string(fileNum) + ".fits";
	// update this, which is used later to move the key file.
	currentDataFileNumber = fileNum;
	eAutoAnalysisHandler.updateDataSetNumberEdit(currentDataFileNumber);
	/// save the file
	int fitsStatus = 0;
	
	fits_create_file(&myFitsFile, (SAVE_BASE_ADDRESS + finalSaveFolder + finalSaveFileName).c_str(), &fitsStatus);
	if (DataFileSystem::checkFitsError(fitsStatus, errMsg))
	{
		errBox(errMsg);
		return true;
	}

	if (DataFileSystem::checkFitsError(fitsStatus, errMsg))
	{
		errBox(errMsg);
		//return true;
	}
	//immediately change this.
	fitsIsOpen = true;
	imageParameters currentImageParameters = eImageControl.getImageParameters();
	long axis[] = { currentImageParameters.width, currentImageParameters.height, eTotalNumberOfPicturesInSeries };
	fits_create_img(myFitsFile, LONG_IMG, 3, axis, &fitsStatus);
	if (DataFileSystem::checkFitsError(fitsStatus, errMsg))
	{
		return true;
	}
	// no errors.
	return false;
}


bool DataFileSystem::writeFits(std::string& errMsg, int currentExperimentPictureNumber, int currentPictureNumber, std::vector<std::vector<long> > images)
{
	if (fitsIsOpen == false)
	{
		errMsg = "Tried to write to fits file, but the file is closed!\r\n";
		return true;
	}
	// MUST initialize status
	int status = 0;
	// starting coordinates of write area in the fits file of the array of picture data points.
	long fpixel[] = { 1, 1, currentPictureNumber };
	fits_write_pix(myFitsFile, TLONG, fpixel, images[currentExperimentPictureNumber].size(), &images[currentExperimentPictureNumber][0], &status);
	if (DataFileSystem::checkFitsError(status, errMsg))
	{
		errBox("FITS ERROR: " + errMsg);
		return true;
	}
	// no errors
	return false;
}


bool DataFileSystem::closeFits(std::string& errMsg)
{
	if (!fitsIsOpen)
	{
		// wasn't open.
		return false;
	}
	int fitsStatus = 0;
	fits_close_file(myFitsFile, &fitsStatus);
	if (DataFileSystem::checkFitsError(fitsStatus, errMsg))
	{
		// not necessarily game-ending.
		errBox("FITS ERROR: " + errMsg);
		return true;
	}
	// it's now closed
	fitsIsOpen = false;
	// don't think this should really accomplish anything. just gives two signals that it's closed. 
	myFitsFile = NULL;
	// no error
	return false;
}

bool DataFileSystem::checkFitsError(int fitsStatusIndicator, std::string& errMsg)
{
	if (fitsStatusIndicator != 0)
	{
		std::vector<char> fitsErrorMsg;
		fitsErrorMsg.resize(80);
		fits_get_errstatus(fitsStatusIndicator, fitsErrorMsg.data());
		errMsg = "CFITS ERROR: " + std::string(fitsErrorMsg.data()) + "\r\n";
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<double> DataFileSystem::getKey()
{
	return keyValues;
}
std::string DataFileSystem::getDate()
{
	return currentDate;
}
int DataFileSystem::getDataFileNumber()
{
	return currentDataFileNumber;
}