#pragma once

// eventually dream of upgrading to HPF5.xrw
#include <vector>
#include "fitsio.h"
#include <string>
#include "DataAnalysisHandler.h"
#include "CameraImageDimensions.h"

/*
 * Handles the writing of fits files. I dream of eventually upgrading this to HPF5.
 */
class DataLogger
{
	public:
		DataLogger(std::string systemLocation);
		void initializeDataFiles( DataAnalysisControl* autoAnalysisHandler, imageParameters currentImageParameters,
								  int totalPicsInSeries );
		void writeFits(int currentPictureNumber, std::vector<long> image);
		void closeFits();
		std::vector<double> getKey();
		void checkFitsError(int fitsStatusIndicator);
		void forceFitsClosed();
		void loadAndMoveKeyFile();
		void deleteFitsAndKey(Communicator* comm);
		int getDataFileNumber();
	private:
		fitsfile* myFitsFile;
		bool fitsIsOpen;
		std::string mostRecentInitializationDate;
		std::string dataFilesBaseLocation;
		std::string currentSaveFolder;
		int currentDataFileNumber;
		std::vector<double> keyValues;
};

