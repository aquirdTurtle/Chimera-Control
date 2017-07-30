#pragma once
#include "fitsio.h"
#include <string>

class DataFileSystem
{
	public:
		DataFileSystem(std::string systemLocation);
		bool initializeDataFiles(std::string& errMsg);
		bool writeFits(std::string& errMsg, int currentExperimentPictureNumber, int currentPictureNumber, std::vector<std::vector<long> > images);
		bool closeFits(std::string& errMsg);
		std::vector<double> getKey();
		bool checkFitsError(int fitsStatusIndicator, std::string& errMsg);
		bool forceFitsClosed();
		bool loadAndMoveKeyFile(std::string& errMsg);
		bool deleteFitsAndKey(std::string& errMsg);
		std::string getDate();
		int getDataFileNumber();
	private:
		fitsfile* myFitsFile;
		bool fitsIsOpen;
		std::string mostRecentInitializationDate;
		std::string dataFilesBaseLocation;
		std::string currentSaveFolder;
		int currentDataFileNumber;
		std::string currentDate;
		std::vector<double> keyValues;
};

