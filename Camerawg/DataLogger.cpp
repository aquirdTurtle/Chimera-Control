#include "stdafx.h"
#include "DataLogger.h"
#include "appendText.h"
#include "externals.h"
#include "fitsio.h"
#include "longnam.h"
#include "DataAnalysisHandler.h"
#include "CameraImageDimensions.h"


DataLogger::DataLogger(std::string systemLocation)
{
	// initialize this to false.
	fitsIsOpen = false;
	dataFilesBaseLocation = systemLocation;
}


// this file assumes that fits is the fits_#.fits file. User should check if incDataSet is on before calling. ???
void DataLogger::deleteFitsAndKey(Communicator* comm)
{
	if (fitsIsOpen)
	{
		thrower("ERROR: Can't delete current fits file, the fits file is open!");
	}
	std::string fitsAddress = dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\data_"
		+ std::to_string(currentDataFileNumber) + ".fits";
	int success = DeleteFile(fitsAddress.c_str());
	if (success == false)
	{
		thrower("Failed to delete fits file! Error code: " + std::to_string(GetLastError()) + ".\r\n");
	}
	else
	{
		comm->sendStatus("Deleted Fits file located at \"" + fitsAddress + "\"\r\n");
	}
	success = DeleteFile((dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\key_"
						 + std::to_string(currentDataFileNumber) + ".txt").c_str());
	if (success == false)
	{
		thrower("Failed to delete key file! Error code: " + std::to_string(GetLastError()) + ".\r\n");
	}
	else
	{
		comm->sendStatus("Deleted Key file located at \"" + dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\key_"
						 + std::to_string(currentDataFileNumber) + ".txt\"\r\n");
	}
}


void DataLogger::loadAndMoveKeyFile()
{
	int result = 0;
	result = CopyFile((dataFilesBaseLocation + "key.txt").c_str(), (dataFilesBaseLocation + currentSaveFolder
					  + "\\Raw Data\\key_" + std::to_string(currentDataFileNumber) + ".txt").c_str(), FALSE);
	if (result == 0)
	{
		// failed
		thrower("Failed to create copy of key file! Error Code: " + std::to_string(GetLastError()) + "\r\n");
	}

	std::ifstream keyFile;
	keyFile.open(dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\key_" + std::to_string(currentDataFileNumber)
				 + ".txt");
	if (!keyFile.is_open())
	{
		thrower("Couldn't open key file!? Does a key file exist???\r\n");
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
			thrower("Key File contained bad values!\r\n");
		}
		std::getline(keyFile, keyString);
	}
}


void DataLogger::forceFitsClosed()
{
	int fitsStatus = 0;
	if (myFitsFile != NULL)
	{
		fits_close_file(myFitsFile, &fitsStatus);
	}
}


void DataLogger::initializeDataFiles(DataAnalysisControl* autoAnalysisHandler, imageParameters currentImageParameters,
									 int totalPicsInSeries)
{
	forceFitsClosed();
	// if the function fails, the fits file will not be open. If it succeeds, this will get set to true.
	fitsIsOpen = false;
	/// First, create the folder for today's fits data.
	// Get the date and use it to set the folder where this data run will be saved.
	// get time now
	
	time_t timeInt = time(0);
	struct tm timeStruct;
	localtime_s(&timeStruct, &timeInt);
	std::string tempStr = std::to_string(timeStruct.tm_year + 1900);
	// Create the string of the date.
	std::string finalSaveFolder;
	finalSaveFolder = tempStr[2];
	finalSaveFolder += tempStr[3];
	if (timeStruct.tm_mon + 1 < 10)
	{
		finalSaveFolder += "0";
		finalSaveFolder += std::to_string(timeStruct.tm_mon + 1);
	}
	else
	{
		finalSaveFolder += std::to_string(timeStruct.tm_mon + 1);
	}
	if (timeStruct.tm_mday < 10)
	{
		finalSaveFolder += "0";
		finalSaveFolder += std::to_string(timeStruct.tm_mday);
	}
	else
	{
		finalSaveFolder += std::to_string(timeStruct.tm_mday);
	}
	currentDate = finalSaveFolder;
	// right now the save folder IS the date...
	currentSaveFolder = finalSaveFolder;
	// create date's folder.
	int result = CreateDirectory((dataFilesBaseLocation + finalSaveFolder).c_str(), 0);
	finalSaveFolder += "\\Raw Data";
	int result2 = CreateDirectory((dataFilesBaseLocation + finalSaveFolder).c_str(), 0);
	finalSaveFolder += "\\";
	/// Get a filename appropriate for the data
	std::string finalSaveFileName;

	// find the first data file that hasn't been already written, starting with data_1.fits
	int fileNum = 1;
	// The while condition here check if file exists. No idea how this actually works.
	struct stat statBuffer;
	// figure out the next file number
	while ((stat((dataFilesBaseLocation + finalSaveFolder + "\\data_" + std::to_string(fileNum) + ".fits").c_str(),
		   &statBuffer) == 0))
	{
		fileNum++;
	}
	// at this point a valid filename has been found.
	finalSaveFileName = "data_" + std::to_string(fileNum) + ".fits";
	// update this, which is used later to move the key file.
	currentDataFileNumber = fileNum;

	//??
	autoAnalysisHandler->updateDataSetNumberEdit(currentDataFileNumber);

	/// save the file
	int fitsStatus = 0;

	fits_create_file(&myFitsFile, (dataFilesBaseLocation + finalSaveFolder + finalSaveFileName).c_str(), &fitsStatus);
	checkFitsError(fitsStatus);
	//immediately change this.
	fitsIsOpen = true;
	long axis[] = { currentImageParameters.width, currentImageParameters.height, totalPicsInSeries };
	fits_create_img(myFitsFile, LONG_IMG, 3, axis, &fitsStatus);
	checkFitsError(fitsStatus);
}


void DataLogger::writeFits(int currentPictureNumber, std::vector<long> image)
{
	if (fitsIsOpen == false)
	{
		thrower("Tried to write to fits file, but the file is closed!\r\n");
	}
	// MUST initialize status
	int status = 0;
	// starting coordinates of write area in the fits file of the array of picture data points.
	long firstPixel[] = { 1, 1, currentPictureNumber };
	fits_write_pix(myFitsFile, TLONG, firstPixel, image.size(), &image[0], &status);
	checkFitsError(status);
}


void DataLogger::closeFits()
{
	if (!fitsIsOpen)
	{
		// wasn't open.
		return;
	}
	int fitsStatus = 0;
	fits_close_file(myFitsFile, &fitsStatus);
	checkFitsError(fitsStatus);
	// it's now closed
	fitsIsOpen = false;
	// don't think this should really accomplish anything. just gives two signals that it's closed. 
	myFitsFile = NULL;
}


void DataLogger::checkFitsError(int fitsStatusIndicator)
{
	if (fitsStatusIndicator != 0)
	{
		std::vector<char> fitsErrorMsg;
		fitsErrorMsg.resize(80);
		fits_get_errstatus(fitsStatusIndicator, fitsErrorMsg.data());
		thrower("CFITS ERROR: " + std::string(fitsErrorMsg.data()) + "\r\n");
	}
}

std::vector<double> DataLogger::getKey()
{
	return keyValues;
}

std::string DataLogger::getDate()
{
	return currentDate;
}

int DataLogger::getDataFileNumber()
{
	return currentDataFileNumber;
}
