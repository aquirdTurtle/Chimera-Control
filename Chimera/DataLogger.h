#pragma once


#include "DataAnalysisHandler.h"					  
#include "CameraImageDimensions.h"
#include "AndorRunSettings.h"
#include "MasterThreadInput.h"
#include "afxwin.h"
#include "BaslerSettingsControl.h"
// there's potentially a typedef conflict with a python file which also typedefs ssize_t.
#define ssize_t h5_ssize_t
#include "H5Cpp.h"
#undef ssize_t
#include <vector>
#include <string>

/*
 * Handles the writing of h5 files. Some parts of this are effectively HDF5 wrappers.
 */
class DataLogger
{
	public:
		DataLogger(std::string systemLocation);
		~DataLogger( );
		void logError ( H5::Exception& err );
		void initializeDataFiles( );
		void writeAndorPic( std::vector<long> image, imageParameters dims );
		void writeBaslerPic ( Matrix<long> image, imageParameters dims );
		void logMasterParameters( MasterThreadInput* input);
		void logMiscellaneous();
		void logAndorSettings( AndorRunSettings settings, bool on );
		void logNiawgSettings( MasterThreadInput* input );
		void logAgilentSettings( const std::vector<Agilent*>& input );
		void logVariables( const std::vector<parameterType>& variables, H5::Group& group, UINT seqInc );
		void logFunctions( H5::Group& group );
		void writeVolts( UINT currentVoltNumber, std::vector<float64> data );
		void logBaslerSettings ( baslerSettings settings, bool on );
		void logTektronicsSettings( );
		UINT getNextFileNumber();
		static void getDataLocation ( std::string base, std::string& todayFolder, std::string& fullPath );
		void closeFile();
		void deleteFile(Communicator* comm);
		int getDataFileNumber( );
		void initializeAioLogging( UINT numSnapshots );

		void initOptimizationFile ( );
		void updateOptimizationFile ( std::string appendTxt );
		void finOptimizationFile ( );

	private:
		std::ofstream optFile;

		// a bunch of overloaded wrapper functions for making the main "log" functions above much cleaner.
		H5::DataSet writeDataSet( bool data,				std::string name, H5::Group& group );
		H5::DataSet writeDataSet( UINT data,				std::string name, H5::Group& group );
		H5::DataSet writeDataSet( ULONGLONG data,			std::string name, H5::Group& group );
		H5::DataSet writeDataSet( int data,					std::string name, H5::Group& group );
		H5::DataSet writeDataSet( double data,				std::string name, H5::Group& group );
		H5::DataSet writeDataSet( std::vector<double> data, std::string name, H5::Group& group );
		H5::DataSet writeDataSet( std::vector<float> data,	std::string name, H5::Group& group );
		H5::DataSet writeDataSet( std::string data,			std::string name, H5::Group& group );
		void writeAttribute( double data,					std::string name, H5::DataSet& dset );
		void writeAttribute( bool data,						std::string name, H5::DataSet& dset );
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
		UINT currentAndorPicNumber, currentBaslerPicNumber;
};


template <class type> void writeDataSet( type data, H5::Group group )
{
	H5::DataSet rightSet = imageDims.createDataSet( "Right", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
	rightSet.write( &settings.imageSettings.right, H5::PredType::NATIVE_INT );
}

