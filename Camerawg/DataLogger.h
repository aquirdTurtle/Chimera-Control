#pragma once

// eventually dream of upgrading to HPF5.xrw
#include <vector>
#include "fitsio.h"
#include <string>

class DataLogger
{
	public:
		DataLogger(std::string systemLocation);
		void initializeDataFiles();
		void writeFits(int currentExperimentPictureNumber, int currentPictureNumber, std::vector<std::vector<long> > images);
		void closeFits();
		std::vector<double> getKey();
		void checkFitsError(int fitsStatusIndicator);
		void forceFitsClosed();
		void loadAndMoveKeyFile();
		void deleteFitsAndKey();
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

