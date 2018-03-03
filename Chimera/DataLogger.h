#pragma once


#include "DataAnalysisHandler.h"					  
#include "CameraImageDimensions.h"
#include "AndorRunSettings.h"
#include "MasterThreadInput.h"
#include "afxwin.h"
#include "H5Cpp.h"
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
		void setUpVoltsMeasurement( );
		void initializeDataFiles();
		void writePic( UINT currentPictureNumber, std::vector<long> image, imageParameters dims );
		void logMasterParameters( MasterThreadInput* input);
		void logMiscellaneous();
		void logAndorSettings( AndorRunSettings settings, bool on );
		void logNiawgSettings( MasterThreadInput* input );
		void logAgilentSettings( const std::vector<Agilent*>& input );
		void logVariables( const std::vector<variableType>& variables, H5::Group& group, UINT seqInc );
		void logFunctions( H5::Group& group );
		void writeVolts( UINT currentVoltNumber, std::vector<float64> data );
		void logTektronicsSettings();
		UINT getNextFileNumber();
		void closeFile();
		void deleteFile(Communicator* comm);
		int getDataFileNumber( );
		void initializeAioLogging( UINT numSnapshots );
	private:
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
	    H5::H5File file;
		H5::DataSet pictureDataset, voltsDataSet;
		// for the entire set
		H5::DataSpace picureSetDataSpace;
		// just one pic
		H5::DataSpace picDataSpace;
		H5::DataSpace voltsDataSpace, voltsSetDataSpace;
	    bool fileIsOpen;
		std::string mostRecentInitializationDate;
		std::string dataFilesBaseLocation;
		std::string currentSaveFolder;
		int currentDataFileNumber;
		UINT currentPicNumber;
};


template <class type> void writeDataSet( type data, H5::Group group )
{
	H5::DataSet rightSet = imageDims.createDataSet( "Right", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
	rightSet.write( &settings.imageSettings.right, H5::PredType::NATIVE_INT );
}

