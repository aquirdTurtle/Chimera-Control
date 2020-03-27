// created by Mark O. Brown
#pragma once

#include "RealTimeDataAnalysis/DataAnalysisControl.h"					  
#include "Andor/CameraImageDimensions.h"
#include "Andor/AndorRunSettings.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "Basler/BaslerSettingsControl.h"
#include "AnalogInput/ServoManager.h"
#include "AnalogInput/servoInfo.h"
#include "Tektronix/TektronixStructures.h"
// there's potentially a typedef conflict with a python file which also typedefs ssize_t.
#define ssize_t h5_ssize_t
#include "H5Cpp.h"
#undef ssize_t
#include <vector>
#include <string>
#include <functional>
#include "afxwin.h"


/*
 * Handles the writing of h5 files. Some parts of this are effectively HDF5 wrappers.
 */
class DataLogger
{
	public:
		DataLogger(std::string systemLocation);
		~DataLogger( );
		void logMasterRuntime ( UINT repNumber, std::vector<parameterType> params );
		void logError ( H5::Exception& err );
		void initializeDataFiles( std::string specialName="", bool needsCal=true);
		void writeAndorPic( Matrix<long> image, imageParameters dims );
		void writeBaslerPic ( Matrix<long> image );
		void writeVolts ( UINT currentVoltNumber, std::vector<float64> data );

		void logMasterInput( ExperimentThreadInput* input );
		void logMiscellaneousStart();
		void logAndorSettings( AndorRunSettings settings, bool on );
		void logNiawgSettings( ExperimentThreadInput* input, bool runNiawg);
		void logAgilentSettings (const std::vector<std::reference_wrapper<AgilentCore>>& agilents,
								 const std::vector<deviceOutputInfo>& agOutput);
		void logParameters( const std::vector<parameterType>& variables, H5::Group& group );
		void logFunctions( H5::Group& group );
		void logAoSystemSettings ( AoSystem& aoSys);
		void logDoSystemSettings ( DoCore& doSys );
		void logBaslerSettings ( baslerSettings settings, bool on );
		void DataLogger::logTektronixSettings (tektronixInfo& tekInfo, std::string delim);
		void logPlotData ( std::string name, std::vector<pPlotDataVec> data );
		void logServoInfo ( std::vector<servoInfo> servos );
		void initializeAiLogging ( UINT numSnapshots );

		static void getDataLocation ( std::string base, std::string& todayFolder, std::string& fullPath );
		void assertCalibrationFilesExist();
		void closeFile();
		void deleteFile(Communicator* comm);
		int getDataFileNumber( );
		

		void initOptimizationFile ( );
		void updateOptimizationFile ( std::string appendTxt );
		void finOptimizationFile ( );

		UINT getNextFileNumber ( );
		std::string getMostRecentDateString ( );

	private:
		std::ofstream optFile;

		// a bunch of overloaded wrapper functions for making the main "log" functions above much cleaner.
		H5::DataSet writeDataSet( bool data,							std::string name, H5::Group& group );
		H5::DataSet writeDataSet( UINT data,							std::string name, H5::Group& group );
		H5::DataSet writeDataSet( ULONGLONG data,						std::string name, H5::Group& group );
		H5::DataSet writeDataSet( int data,								std::string name, H5::Group& group );
		H5::DataSet writeDataSet( double data,							std::string name, H5::Group& group );
		H5::DataSet writeDataSet( std::vector<double> data,				std::string name, H5::Group& group );
		H5::DataSet writeDataSet( std::vector<float> data,				std::string name, H5::Group& group );
		H5::DataSet writeDataSet( std::string data,						std::string name, H5::Group& group );
		H5::DataSet writeDataSet ( std::vector<std::string> dataVec,	std::string name, H5::Group& group );
		void writeAttribute( double data,								std::string name, H5::DataSet& dset );
		void writeAttribute( bool data,									std::string name, H5::DataSet& dset );
		// the core file.
	    H5::H5File file;
		H5::DataSet AndorPictureDataset, voltsDataSet, BaslerPictureDataset;
		// for the entire set
		H5::DataSpace AndorPicureSetDataSpace, BaslerPicureSetDataSpace;
		// just one pic
		H5::DataSpace AndorPicDataSpace, BaslerPicDataSpace;

		H5::DataSpace voltsDataSpace, voltsSetDataSpace;
	    bool fileIsOpen;
		std::string mostRecentInitializationDate;
		std::string dataFilesBaseLocation;
		std::string todayFolder;
		int currentDataFileNumber;
		std::string mostRecentDateString;
		UINT currentAndorPicNumber, currentBaslerPicNumber;
};


template <class type> void writeDataSet( type data, H5::Group group )
{
	H5::DataSet rightSet = imageDims.createDataSet( "Right", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
	rightSet.write( &settings.imageSettings.right, H5::PredType::NATIVE_INT );
}

