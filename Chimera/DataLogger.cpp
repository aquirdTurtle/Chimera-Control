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
		+ str(currentDataFileNumber) + ".fits";
	int success = DeleteFile(cstr(fitsAddress));
	if (success == false)
	{
		thrower("Failed to delete fits file! Error code: " + str(GetLastError()) + ".\r\n");
	}
	else
	{
		comm->sendStatus("Deleted Fits file located at \"" + fitsAddress + "\"\r\n");
	}
	success = DeleteFile(cstr(dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\key_"
						 + str(currentDataFileNumber) + ".txt"));
	if (success == false)
	{
		thrower("Failed to delete key file! Error code: " + str(GetLastError()) + ".\r\n");
	}
	else
	{
		comm->sendStatus("Deleted Key file located at \"" + dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\key_"
						 + str(currentDataFileNumber) + ".txt\"\r\n");
	}
}


void DataLogger::loadAndMoveKeyFile()
{
	int result = 0;
	std::string keyLoc = dataFilesBaseLocation + "key_" + str(currentDataFileNumber) + ".txt";
	result = CopyFile(cstr(KEY_ORIGINAL_SAVE_LOCATION), cstr(keyLoc), FALSE);
	//result = CopyFile(cstr(KEY_ORIGINAL_SAVE_LOCATION + "key.txt"), cstr(dataFilesBaseLocation + currentSaveFolder
	//				  + "\\Raw Data\\key_" + str(currentDataFileNumber) + ".txt"), FALSE);
	if (result == 0)
	{
		// failed
		thrower("Failed to create copy of key file! Error Code: " + str(GetLastError()) + "\r\n");
	}

	std::ifstream keyFile;
	keyFile.open(keyLoc);
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
	//forceFitsClosed();
	// if the function fails, the fits file will not be open. If it succeeds, this will get set to true.
	fitsIsOpen = false;
	/// First, create the folder for today's fits data.
	// Get the date and use it to set the folder where this data run will be saved.
	// get time now
	
	time_t timeInt = time(0);
	struct tm timeStruct;
	localtime_s(&timeStruct, &timeInt);
	std::string tempStr = str(timeStruct.tm_year + 1900);
	// Create the string of the date.
	std::string finalSaveFolder;
	finalSaveFolder += tempStr + "\\";
	std::string month;
	switch (timeStruct.tm_mon + 1)
	{
		case 1:
			month = "January";
			break;
		case 2:
			month = "February";
			break;
		case 3:
			month = "March";
			break;
		case 4:
			month = "April";
			break;
		case 5:
			month = "May";
			break;
		case 6:
			month = "June";
			break;
		case 7:
			month = "July";
			break;
		case 8:
			month = "August";
			break;
		case 9:
			month = "September";
			break;
		case 10:
			month = "October";
			break;
		case 11:
			month = "November";
			break;
		case 12:
			month = "December";
			break;
	}
	
	finalSaveFolder += month + "\\";
	finalSaveFolder += month + " " + str(timeStruct.tm_mday);
	// right now the save folder IS the date...
	currentSaveFolder = finalSaveFolder;
	// create date's folder.
	int result = CreateDirectory(cstr(dataFilesBaseLocation + currentSaveFolder), 0);
	finalSaveFolder += "\\Raw Data";
	int result2 = CreateDirectory(cstr(dataFilesBaseLocation + currentSaveFolder), 0);
	finalSaveFolder += "\\";
	/// Get a filename appropriate for the data
	std::string finalSaveFileName;

	// find the first data file that hasn't been already written, starting with data_1.fits
	int fileNum = 1;
	// The while condition here check if file exists. No idea how this actually works.
	struct stat statBuffer;
	// figure out the next file number
	while ((stat(cstr(dataFilesBaseLocation + finalSaveFolder + "\\data_" + str(fileNum) + ".fits"),
		   &statBuffer) == 0))
	{
		fileNum++;
	}
	// at this point a valid filename has been found.
	finalSaveFileName = "data_" + str(fileNum) + ".fits";
	// update this, which is used later to move the key file.
	currentDataFileNumber = fileNum;

	//??
	autoAnalysisHandler->updateDataSetNumberEdit(currentDataFileNumber);

	/// save the file
	int fitsStatus = 0;

	fits_create_file(&myFitsFile, cstr(dataFilesBaseLocation + finalSaveFolder + finalSaveFileName), &fitsStatus);
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
		thrower("CFITS ERROR: " + str(fitsErrorMsg.data()) + "\r\n");
	}
}


std::vector<double> DataLogger::getKey()
{
	return keyValues;
}


int DataLogger::getDataFileNumber()
{
	return currentDataFileNumber;
}