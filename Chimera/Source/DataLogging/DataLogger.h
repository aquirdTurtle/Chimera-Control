// created by Mark O. Brown
#pragma once

#include "RealTimeDataAnalysis/DataAnalysisControl.h"					  
#include "Andor/CameraImageDimensions.h"
#include "Andor/AndorRunSettings.h"
#include "Basler/BaslerSettingsControl.h"
#include "AnalogInput/servoInfo.h"
#include "Tektronix/TektronixStructures.h"
#include <Piezo/piezoChan.h>
#include <GeneralObjects/Matrix.h>
// there's potentially a typedef conflict with a python file which also typedefs ssize_t.
#define ssize_t h5_ssize_t
#include "H5Cpp.h"
#undef ssize_t
#include <vector>
#include <string>
#include <functional>

class DoCore;
class AoSystem;
struct ExperimentThreadInput;
/*
 * Handles the writing of h5 files. Some parts of this are effectively HDF5 wrappers.
 */
class DataLogger : public IChimeraSystem {
	public:
		DataLogger(std::string dataLoc, std::string remoteDataLoc, IChimeraQtWindow* parent);
		~DataLogger( );
		void logMasterRuntime ( unsigned repNumber, std::vector<parameterType> params );
		void logError ( H5::Exception& err );
		void initializeDataFiles( std::string specialName="", bool needsCal=true);
		void writeAndorPic( Matrix<long> image, imageParameters dims );
		void writeBaslerPic ( Matrix<long> image );
		void writeVolts ( unsigned currentVoltNumber, std::vector<double> data );
		void assertCalibrationFilesExist ();
		void logServoInfo (std::vector<servoInfo> servos);
		void logMasterInput( ExperimentThreadInput* input );
		void logMiscellaneousStart();
		void logParameters( const std::vector<parameterType>& variables, H5::Group& group );
		void logFunctions( H5::Group& group );
		void logAoSystemSettings ( AoSystem& aoSys);
		void logDoSystemSettings ( DoCore& doSys );
		void logPlotData ( std::string name );
		void logAndorPiezos (piezoChan<double> cameraPiezoVals);
		void initializeAiLogging ( unsigned numSnapshots );
		int getCalibrationFileIndex ();
		static void getDataLocation ( std::string base, std::string& todayFolder, std::string& fullPath );
		void normalCloseFile();
		void copyDataFile(std::string specialName="");
		void deleteFile();
		int getDataFileNumber( );
		void assertClosed ();
		std::string getFullError (H5::Exception& err);
		void initOptimizationFile ( );
		void updateOptimizationFile ( std::string appendTxt );
		void finOptimizationFile ( );
		bool andorDataSetShouldBeValid = false;
		unsigned getNextFileNumber ( );
		std::string getMostRecentDateString ( );
		// the core file.
		H5::H5File file;
		// a bunch of overloaded wrapper functions for making the main "log" functions above much cleaner.
		H5::DataSet writeDataSet (bool data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (unsigned data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (unsigned __int64 data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (int data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (double data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (std::vector<double> data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (std::vector<float> data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (std::string data, std::string name, H5::Group& group);
		H5::DataSet writeDataSet (std::vector<std::string> dataVec, std::string name, H5::Group& group);
		void writeAttribute (double data, std::string name, H5::DataSet& dset);
		void writeAttribute (bool data, std::string name, H5::DataSet& dset);
		H5::DataSet AndorPictureDataset, voltsDataSet, BaslerPictureDataset;
		// for the entire set
		H5::DataSpace AndorPicureSetDataSpace, BaslerPicureSetDataSpace;
		// just one pic
		H5::DataSpace AndorPicDataSpace, BaslerPicDataSpace;

		H5::DataSpace voltsDataSpace, voltsSetDataSpace;
		unsigned currentAndorPicNumber, currentBaslerPicNumber;
		std::string mr_dayStr, mr_monthStr, mr_yearStr;

	private:
		std::ofstream optFile;
	    bool fileIsOpen;
		std::string mostRecentInitializationDate;
		std::string dataFilesLocalLocation;
		std::string dataFilesRemoteLocation;
		std::string todayFolder;
		int currentDataFileNumber;
		std::string mostRecentDateString;
};


template <class type> void writeDataSet( type data, H5::Group group )
{
	H5::DataSet rightSet = imageDims.createDataSet( "Right", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
	rightSet.write( &settings.imageSettings.right, H5::PredType::NATIVE_INT );
}

