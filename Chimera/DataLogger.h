#pragma once

#include <vector>
#include <string>

#include "H5Cpp.h"

#include "DataAnalysisHandler.h"					  
#include "CameraImageDimensions.h"
#include "AndorRunSettings.h"
/*
 * Handles the writing of h5 files. Some parts of this are effectively HDF5 wrappers.
 */
class DataLogger
{
	public:
		DataLogger(std::string systemLocation);

		void initializeDataFiles();
		void writePic( UINT currentPictureNumber, std::vector<long> image, imageParameters dims );
		void logMasterParameters( MasterThreadInput* input);
		void logMiscellaneous();
		void logAndorSettings( AndorRunSettings settings, bool on );
		void logNiawgSettings();
		void logAgilentSettings();
		void logTektronicsSettings();
		UINT getNextFileNumber();
		void closeFile();

		void deleteFile(Communicator* comm);
		int getDataFileNumber();

	private:
	    H5::H5File file;
		H5::DataSet pictureDataset;
		// for the entire set
		H5::DataSpace picureSetDataSpace;
		// just one pic
		H5::DataSpace picDataSpace;
	    bool fileIsOpen;
		std::string mostRecentInitializationDate;
		std::string dataFilesBaseLocation;
		std::string currentSaveFolder;
		int currentDataFileNumber;
		key varKey;
		//std::vector<double> keyValues;
};

