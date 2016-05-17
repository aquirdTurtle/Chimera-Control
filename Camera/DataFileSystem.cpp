#include "stdafx.h"
#include "DataFileSystem.h"
#include "appendText.h"

DataFileSystem::DataFileSystem(std::string systemLocation)
{
	// initialize this to false.
	fitsIsOpen = false;
	dataFilesBaseLocation = systemLocation;
}
DataFileSystem::~DataFileSystem()
{
	// nothing right now.
}
bool DataFileSystem::initializeDataFiles(bool incrementFiles, std::string& errMsg)
{
	// check if already open.
	if (fitsIsOpen == true)
	{
		// then close it.
		if (DataFileSystem::closeFits(errMsg))
		{
			return true;
		}
		fitsIsOpen = false;
	}

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
	// create date's folder.
	int result = CreateDirectory((SAVE_BASE_ADDRESS + finalSaveFolder).c_str(), 0);
	finalSaveFolder += "\\";
	/// Get a filename appropriate for the data
	std::string finalSaveFileName;
	if (incrementFiles)
	{
		// find the first data file that hasn't been already written, starting with data_1.fits
		int fileNum = 1;
		// The while condition here check if file exists. No idea how this actually works.
		struct stat statBuffer;
		while ((stat((SAVE_BASE_ADDRESS + finalSaveFolder + "data_" + std::to_string(fileNum) + ".fits").c_str(), &statBuffer) == 0))
		{
			fileNum++;
		}
		// at this point a valid filename has been found.
		finalSaveFileName = "data_" + std::to_string(fileNum) + ".fits";
		// make a copy of the key file giving it an assoicated name in the appropriate folder.
		int result = CopyFile((KEY_FILE_LOCATION + "key.txt").c_str(), 
							  (SAVE_BASE_ADDRESS + finalSaveFolder + "key_" + std::to_string(fileNum) + ".txt").c_str(), FALSE);
		if (result == 0)
		{
			// failed
			MessageBox(0, ("Failed to create copy of key file! Error Code: " + std::to_string(GetLastError())).c_str(), 0, 0);
			// but continue, no breaking...
		}		
	}
	else
	{
		struct stat statBuffer;
		finalSaveFileName = "data.fits";
		// The if statement here check if file exists. No idea how this actually works.
		if (((stat((SAVE_BASE_ADDRESS + finalSaveFolder + "data.fits").c_str(), &statBuffer) == 0)))
		{
			// if it does exist, delete it. The plain "data.fits" file gets overwritten every time. Because of the check at the 
			// beginning of the function, the fits filesystem should never have control of this file at this point.
			int success = DeleteFile((SAVE_BASE_ADDRESS + finalSaveFolder + finalSaveFileName).c_str());
			if (success == false)
			{
				MessageBox(0, "WTF", 0, 0);
			}
		}
		

	}
	/// save the file
	int fitsStatus = 0;
	fits_create_file(&myFitsFile, (SAVE_BASE_ADDRESS + finalSaveFolder + finalSaveFileName).c_str(), &fitsStatus);
	if (DataFileSystem::checkFitsError(fitsStatus, errMsg))
	{
		return true;
	}
	//immediately change this.
	fitsIsOpen = true;

	long axis[] = { eImageWidth, eImageHeight, eTotalNumberOfPicturesInSeries };
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
		return true;
	}
	// it's now closed
	fitsIsOpen = false;
	// no error
	return false;
}
bool DataFileSystem::getKey(std::string& errMsg)
{
	// TODO: get key for x axis in plots.
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