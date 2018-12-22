#include "stdafx.h"
#include "DataLogger.h"
#include "externals.h"
#include "longnam.h"
#include "DataAnalysisControl.h"
#include "CameraImageDimensions.h"
#include "Thrower.h"
#include "MasterManager.h"


DataLogger::DataLogger(std::string systemLocation)
{
	// initialize this to false.
	fileIsOpen = false;
	dataFilesBaseLocation = systemLocation;
}


DataLogger::~DataLogger( )
{
	// try to close before finishing.
	closeFile( );
}


// this file assumes that h5 is the data_#.h5 file. User should check if incDataSet is on before calling. ???
void DataLogger::deleteFile(Communicator* comm)
{
	if (fileIsOpen)
	{
		// I'm not actually sure if this should be a prob with h5.
		thrower ("ERROR: Can't delete current h5 file, the h5 file is open!");
	}
	std::string fileAddress = dataFilesBaseLocation + todayFolder + "\\Raw Data\\data_"
		+ str( currentDataFileNumber ) + ".h5";
	int success = DeleteFile(cstr(fileAddress));
	if (success == false)
	{
		thrower ("Failed to delete h5 file! Error code: " + str(GetLastError()) + ".\r\n");
	}
	else
	{
		comm->sendStatus( "Deleted h5 file located at \"" + fileAddress + "\"\r\n" );
	}
}


void DataLogger::getDataLocation ( std::string base, std::string& todayFolder, std::string& fullPath )
{
	time_t timeInt = time ( 0 );
	struct tm timeStruct;
	localtime_s ( &timeStruct, &timeInt );
	std::string tempStr = str ( timeStruct.tm_year + 1900 );
	// Create the string of the date.
	std::string finalSaveFolder;
	finalSaveFolder += tempStr + "\\";
	std::string month;
	switch ( timeStruct.tm_mon + 1 )
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
	finalSaveFolder += month + "\\" + month + " " + str ( timeStruct.tm_mday );
	todayFolder = finalSaveFolder;
	// create date's folder.
	int result = 1;
	struct stat info;
	int resultStat = stat ( cstr ( base + finalSaveFolder ), &info );
	if ( resultStat != 0 )
	{
		result = CreateDirectory ( cstr ( base + finalSaveFolder ), 0 );
	}
	if ( !result )
	{
		thrower ( "ERROR: Failed to create save location for data at location " + base + finalSaveFolder +
				  ". Make sure you have access to the jilafile or change the save location. Error: " + str ( GetLastError ( ) )
				  + "\r\n" );
	}
	finalSaveFolder += "\\Raw Data";
	resultStat = stat ( cstr ( base + finalSaveFolder ), &info );
	if ( resultStat != 0 )
	{
		result = CreateDirectory ( cstr ( base + finalSaveFolder ), 0 );
	}
	if ( !result )
	{
		thrower ( "ERROR: Failed to create save location for data! Error: " + str ( GetLastError ( ) ) + "\r\n" );
	}
	finalSaveFolder += "\\";
	fullPath = finalSaveFolder;
}


/*
specialName: optional, the name of the data file. This is a unique name, it will not be incremented. This is used
	for things like the mot size measurement or temperature measurements which are automated. The default is to use
	data_ as the name and add an incrementing number at the end of it.
*/
void DataLogger::initializeDataFiles( std::string specialName, bool isCal )
{
	// if the function fails, the h5 file will not be open. If it succeeds, this will get set to true.
	fileIsOpen = false;
	/// First, create the folder for today's h5 data.
	// Get the date and use it to set the folder where this data run will be saved.
	std::string finalSaveFolder;
	getDataLocation ( dataFilesBaseLocation, todayFolder, finalSaveFolder );

	/// check that temperature data is being recorded.
	FILE *temperatureFile;
	auto temperatureDataLocation = dataFilesBaseLocation + finalSaveFolder + "Temperature_Data.csv";
	fopen_s ( &temperatureFile, temperatureDataLocation.c_str(), "r" );
	if ( !temperatureFile )
	{
		thrower ( "ERROR: The Data logger doesn't see the temperature data for today in the data folder, location:"
				  + temperatureDataLocation + ". Please make sure that the temperature logger is working correctly "
				  "before starting an experiment." );
	}
	else
	{
		fclose ( temperatureFile );
	}
	/// check that the mot calibration files have been recorded.
	if ( !isCal )
	{
		for ( std::string fName : {"MOT_NUMBER.h5", "MOT_TEMPERATURE.h5", "RED_PGC_TEMPERATURE.h5",
			  "GREY_MOLASSES_TEMPERATURE.h5"} )
		{
			FILE *calFile;
			auto calDataLoc = dataFilesBaseLocation + finalSaveFolder + fName;
			fopen_s ( &calFile, calDataLoc.c_str ( ), "r" );
			if ( !calFile )
			{
				thrower ( "ERROR: The Data logger doesn't see the MOT calibration data for today in the data folder, "
						  "location:" + calDataLoc + ". Please make sure that the mot is running okay and then "
						  "run F12 before starting an experiment." );
			}
			else
			{
				// all good.
				fclose ( calFile );
			}
		}
	}


	/// Get a filename appropriate for the data
	std::string finalSaveFileName;
	if ( specialName == "" )
	{
		// the default option.
		UINT fileNum = getNextFileIndex ( dataFilesBaseLocation + finalSaveFolder + "data_", ".h5" );
		// at this point a valid filename has been found.
		finalSaveFileName = "data_" + str ( fileNum ) + ".h5";
		// update this, which is used later to move the key file.
		currentDataFileNumber = fileNum;
	}
	else
	{
		finalSaveFileName = specialName + ".h5";
	}

 	try
 	{
		closeFile ( );
 		// create the file. H5F_ACC_TRUNC means it will overwrite files with the same name.
		file = H5::H5File( cstr( dataFilesBaseLocation + finalSaveFolder + finalSaveFileName ), H5F_ACC_TRUNC );
		fileIsOpen = true;
		H5::Group ttlsGroup( file.createGroup( "/TTLs" ) );
		// initial settings
		// list of commands
		H5::Group tektronicsGroup( file.createGroup( "/Tektronics" ) );
		// mode, freq, power
		H5::Group miscellaneousGroup( file.createGroup( "/Miscellaneous" ) );
		time_t t = time( 0 );   // get time now
		struct tm now;
		localtime_s( &now, &t );
		std::string dateString = str( now.tm_year + 1900 ) + "-" + str( now.tm_mon + 1 ) + "-" + str( now.tm_mday );
		writeDataSet( dateString, "Run-Date", miscellaneousGroup );
		std::string timeString = str( now.tm_hour) + ":" + str( now.tm_min) + ":" + str( now.tm_sec) + ":";
		writeDataSet( timeString, "Time-Of-Logging", miscellaneousGroup );
	}
	catch (H5::Exception err)
	{
		logError ( err );
		throwNested ( "ERROR: Failed to initialize HDF5 data file: " + err.getDetailMsg() );
	}
}


void DataLogger::logAgilentSettings( const std::vector<Agilent*>& agilents )
{
	H5::Group agilentsGroup( file.createGroup( "/Agilents" ) );
	for ( auto& agilent : agilents )
	{
		H5::Group singleAgilent( agilentsGroup.createGroup( agilent->configDelim ) );
		// mode
		deviceOutputInfo info = agilent->getOutputInfo( );
		UINT channelCount = 1;
		for ( auto& channel : info.channel )
		{
			H5::Group channelGroup( singleAgilent.createGroup( "Channel-" + str( channelCount ) ) );
			std::string outputModeName;
			switch ( channel.option )
			{
			case -2:
				outputModeName = "No-Control";
				break;
			case -1:
				outputModeName = "Output-Off";
				break;
			case 0:
				outputModeName = "DC";
				break;
			case 1:
				outputModeName = "Sine";
				break;
			case 2:
				outputModeName = "Square";
				break;
			case 3:
				outputModeName = "Preloaded-Arb";
				break;
			case 4:
				outputModeName = "Scripted-Arb";
				break;
			}
			writeDataSet( outputModeName, "Output-Mode", channelGroup );
			H5::Group dcGroup( channelGroup.createGroup( "DC-Settings" ) );
			writeDataSet( channel.dc.dcLevelInput.expressionStr, "DC-Level", dcGroup );
			H5::Group sineGroup( channelGroup.createGroup( "Sine-Settings" ) );
			writeDataSet( channel.sine.frequencyInput.expressionStr, "Frequency", sineGroup );
			writeDataSet( channel.sine.amplitudeInput.expressionStr, "Amplitude", sineGroup );
			H5::Group squareGroup( channelGroup.createGroup( "Square-Settings" ) );
			writeDataSet( channel.square.amplitudeInput.expressionStr, "Amplitude", squareGroup );
			writeDataSet( channel.square.frequencyInput.expressionStr, "Frequency", squareGroup );
			writeDataSet( channel.square.offsetInput.expressionStr, "Offset", squareGroup );
			H5::Group preloadedArbGroup( channelGroup.createGroup( "Preloaded-Arb-Settings" ) );
			writeDataSet( channel.preloadedArb.address, "Address", preloadedArbGroup );
			H5::Group scriptedArbSettings( channelGroup.createGroup( "Scripted-Arb-Settings" ) );
			writeDataSet( channel.scriptedArb.fileAddress, "Script-File-Address", preloadedArbGroup );
			// TODO: load script file itself
			channelCount++;
		}
	}
}


void DataLogger::logFunctions( H5::Group& group )
{
	H5::Group funcGroup( group.createGroup( "Functions" ) );
	try
	{
		// Re-add the entries back in and figure out which one is the current one.
		std::vector<std::string> names;
		std::string search_path = FUNCTIONS_FOLDER_LOCATION + "\\" + "*." + FUNCTION_EXTENSION;
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		hFind = FindFirstFile( cstr( search_path ), &fd );
		if ( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				// if looking for folders
				if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					names.push_back( fd.cFileName );
				}
			} while ( FindNextFile( hFind, &fd ) );
			FindClose( hFind );
		}
		
		for ( auto name : names )
		{
			std::ifstream functionFile( FUNCTIONS_FOLDER_LOCATION + "\\" + name );
			std::stringstream stream;
			stream << functionFile.rdbuf( );
			std::string tempStr( stream.str( ) );
			H5::Group indvFunc( funcGroup.createGroup( name ) );
			writeDataSet( stream.str( ), name, indvFunc);
		}
	}
	catch ( H5::Exception& err )
	{
		throwNested ( "ERROR: Failed to log functions in HDF5 file: " + err.getDetailMsg( ) );
	}
}

void DataLogger::initOptimizationFile ( )
{
	if ( optFile.is_open ( ) )
	{
		finOptimizationFile ( );
	}
	std::string todayFolder, finalFolder;
	DataLogger::getDataLocation ( DATA_SAVE_LOCATION, todayFolder, finalFolder );
	UINT fileNum = getNextFileIndex ( DATA_SAVE_LOCATION + finalFolder + "Optimization_Results_", ".txt" );
	optFile.open( DATA_SAVE_LOCATION + finalFolder + "Optimization_Results_" + str ( fileNum ) + ".txt" );
}


void DataLogger::updateOptimizationFile ( std::string appendTxt )
{
	if ( !optFile.is_open ( ) )
	{
		thrower ( "Tried to update optimization record file, but file was closed!" );
	}
	optFile << appendTxt;
}


void DataLogger::finOptimizationFile ( )
{
	optFile.close ( );
}


void DataLogger::logBaslerSettings ( baslerSettings settings, bool on )
{
	try
	{
		if ( !on )
		{
			H5::Group baslerGroup( file.createGroup ( "/Basler:Off" ) );
			return;
		}
		H5::Group baslerGroup ( file.createGroup ( "/Basler" ) );
		hsize_t rank1[ ] = { 1 };
		// pictures. These are permanent members of the class for speed during the writing process.	
		settings.repCount;
		hsize_t setDims[ ] = { ULONGLONG ( settings.repCount ), settings.dims.width ( ),
			settings.dims.height ( ) };
		hsize_t picDims[ ] = { 1, settings.dims.width ( ), settings.dims.height ( ) };
		BaslerPicureSetDataSpace = H5::DataSpace ( 3, setDims );
		BaslerPicDataSpace = H5::DataSpace ( 3, picDims );
		BaslerPictureDataset = baslerGroup.createDataSet ( "Pictures", H5::PredType::NATIVE_LONG, BaslerPicureSetDataSpace );
		currentBaslerPicNumber = 0;

		writeDataSet ( BaslerAcquisition::toStr ( settings.acquisitionMode ), "Camera-Mode", baslerGroup );
		writeDataSet ( BaslerAutoExposure::toStr(settings.exposureMode), "Exposure-Mode", baslerGroup );
		writeDataSet ( settings.exposureTime, "Exposure-Time", baslerGroup );
		writeDataSet ( BaslerTrigger::toStr( settings.triggerMode ), "Trigger-Mode", baslerGroup );
		// image settings
		H5::Group imageDims = baslerGroup.createGroup ( "Image-Dimensions" );
		writeDataSet ( settings.dims.top, "Top", imageDims );
		writeDataSet ( settings.dims.bottom, "Bottom", imageDims );
		writeDataSet ( settings.dims.left, "Left", imageDims );
		writeDataSet ( settings.dims.right, "Right", imageDims );
		writeDataSet ( settings.dims.horizontalBinning, "Horizontal-Binning", imageDims );
		writeDataSet ( settings.dims.verticalBinning, "Vertical-Binning", imageDims );

		writeDataSet ( settings.frameRate, "Frame-Rate", baslerGroup );
		writeDataSet ( settings.rawGain, "Raw-Gain", baslerGroup );
	}
	catch ( H5::Exception err )
	{
		logError ( err );
		throwNested ( "ERROR: Failed to log basler parameters in HDF5 file: " + err.getDetailMsg ( ) );
	}
}



void DataLogger::writeBaslerPic ( Matrix<long> image, imageParameters dims )
{
	if ( fileIsOpen == false )
	{
		thrower ( "Tried to write to h5 file (for basler pic), but the file is closed!\r\n" );
	}
	// starting coordinates of write area in the h5 file of the array of picture data points.
	hsize_t offset[ ] = { currentBaslerPicNumber++, 0, 0 };
	hsize_t slabdim[ 3 ] = { 1, dims.width ( ), dims.height ( ) };
	try 
	{    
		BaslerPicureSetDataSpace.selectHyperslab ( H5S_SELECT_SET, slabdim, offset );
		BaslerPictureDataset.write ( image.data.data ( ), H5::PredType::NATIVE_LONG, BaslerPicDataSpace,
									 BaslerPicureSetDataSpace );
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "Failed to write data to HDF5 file! Error: " + str ( err.getDetailMsg ( ) ) + "\n" );
	}
}


void DataLogger::logError ( H5::Exception& err )
{
	FILE * pFile;
	fopen_s ( &pFile, "H5ErrLog.txt", "a" );
	if ( pFile != NULL )
	{
		err.printErrorStack ( pFile );
		fclose ( pFile );
	}
}


void DataLogger::logAndorSettings( AndorRunSettings settings, bool on)
{
	try
	{
		if ( !on )
		{
			H5::Group andorGroup( file.createGroup( "/Andor:Off" ) );
			return;
		}
		// in principle there are some other low level settings or things that aren't used very often which I could include 
		// here. I'm gonna leave this for now though.
		H5::Group andorGroup( file.createGroup( "/Andor" ) );
		hsize_t rank1[] = { 1 };
		// pictures. These are permanent members of the class for speed during the writing process.	
		hsize_t setDims[] = { ULONGLONG( settings.totalPicsInExperiment() ), settings.imageSettings.width(),
			settings.imageSettings.height() };
		hsize_t picDims[] = { 1, settings.imageSettings.width(), settings.imageSettings.height() };
		AndorPicureSetDataSpace = H5::DataSpace( 3, setDims );
		AndorPicDataSpace = H5::DataSpace( 3, picDims );
		AndorPictureDataset = andorGroup.createDataSet( "Pictures", H5::PredType::NATIVE_LONG, AndorPicureSetDataSpace );
		currentAndorPicNumber = 0;
		writeDataSet( int(settings.acquisitionMode), "Camera-Mode", andorGroup );
		writeDataSet( settings.exposureTimes, "Exposure-Times", andorGroup );
		writeDataSet( int(settings.triggerMode), "Trigger-Mode", andorGroup );
		writeDataSet( settings.emGainModeIsOn, "EM-Gain-Mode-On", andorGroup );
		if ( settings.emGainModeIsOn )
		{
			writeDataSet( settings.emGainLevel, "EM-Gain-Level", andorGroup );
		}
		else
		{
			writeDataSet( -1, "NA:EM-Gain-Level", andorGroup );
		}
		// image settings
		H5::Group imageDims = andorGroup.createGroup( "Image-Dimensions" );
		writeDataSet( settings.imageSettings.top, "Top", andorGroup );
		writeDataSet( settings.imageSettings.bottom, "Bottom", andorGroup );
		writeDataSet( settings.imageSettings.left, "Left", andorGroup );
		writeDataSet( settings.imageSettings.right, "Right", andorGroup );
		writeDataSet( settings.imageSettings.horizontalBinning, "Horizontal-Binning", andorGroup );
		writeDataSet( settings.imageSettings.verticalBinning, "Vertical-Binning", andorGroup );
		writeDataSet( settings.temperatureSetting, "Temperature-Setting", andorGroup );
		writeDataSet( settings.picsPerRepetition, "Pictures-Per-Repetition", andorGroup );
		writeDataSet( settings.repetitionsPerVariation, "Repetitions-Per-Variation", andorGroup );
		writeDataSet( settings.totalVariations, "Total-Variation-Number", andorGroup );
	}
	catch ( H5::Exception err )
	{
		logError ( err );
		throwNested ( "ERROR: Failed to log andor parameters in HDF5 file: " + err.getDetailMsg( ) );
	}
}


void DataLogger::logMasterParameters( MasterThreadInput* input )
{
	try
	{
		if ( input == NULL )
		{
			H5::Group runParametersGroup( file.createGroup( "/Master-Parameters:NA" ) );
			return;
		}
		H5::Group runParametersGroup( file.createGroup( "/Master-Parameters" ) );
		writeDataSet( input->runMaster, "Run-Master", runParametersGroup );
		if ( input->runMaster )
		{
			std::ifstream masterScript( ProfileSystem::getMasterAddressFromConfig( input->profile ) );
			if ( !masterScript.is_open( ) )
			{
				thrower ( "ERROR: Failed to load master script for data logger!" );
			}
			std::string scriptBuf( str( masterScript.rdbuf( ) ) );
			writeDataSet( scriptBuf, "Master-Script", runParametersGroup);
			writeDataSet( ProfileSystem::getMasterAddressFromConfig(input->profile), "Master-Script-File-Address", 
						  runParametersGroup );
		}
		else
		{
			writeDataSet( "", "NA:Master-Script", runParametersGroup );
			writeDataSet( "", "NA:Master-Script-File-Address", runParametersGroup );
		}
		logFunctions( runParametersGroup );
		writeDataSet( input->repetitionNumber, "Repetitions", runParametersGroup );
		UINT count = 0;
		for ( auto& seqVariables : input->variables )
		{
			logVariables( seqVariables, runParametersGroup, count );
			count++;
		}
		logNiawgSettings( input );
		logAgilentSettings( input->agilents );
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: Failed to log master parameters in HDF5 file: detail:" + err.getDetailMsg( ) );
	}
}


void DataLogger::logVariables( const std::vector<parameterType>& variables, H5::Group& group, UINT seqInc )
{
	H5::Group variableGroup = group.createGroup( "Seq #" + str(seqInc+1) + " Variables" );
	for ( auto& variable : variables )
	{
		H5::DataSet varSet;
		if ( variable.constant )
		{
			varSet = writeDataSet( variable.constantValue, variable.name, variableGroup );
		}
		else
		{
			varSet = writeDataSet( variable.keyValues, variable.name, variableGroup );
		}
		writeAttribute( variable.constant, "Constant", varSet );
	} 
}



void DataLogger::writeAndorPic( std::vector<long> image, imageParameters dims)
{
	if (fileIsOpen == false)
	{
		thrower ("Tried to write to h5 file (for andor pic), but the file is closed!\r\n");
	}
	// MUST initialize status
	// starting coordinates of write area in the h5 file of the array of picture data points.
	hsize_t offset[] = { currentAndorPicNumber++, 0, 0 };
	hsize_t slabdim[3] = { 1, dims.width(), dims.height() };
	try
	{
		AndorPicureSetDataSpace.selectHyperslab( H5S_SELECT_SET, slabdim, offset );
		AndorPictureDataset.write( image.data(), H5::PredType::NATIVE_LONG, AndorPicDataSpace, AndorPicureSetDataSpace );
	}
	catch (H5::Exception& err)
	{
		logError ( err );
		throwNested ("Failed to write data to HDF5 file! Error: " + str(err.getDetailMsg()) + "\n");
	}
}

 
void DataLogger::initializeAioLogging( UINT numSnapshots )
{
	// initial settings
	// list of commands?
	if ( numSnapshots != 0 )
	{
		H5::Group aioGroup ( file.createGroup ( "/AI" ) );
		hsize_t setDims[] = { numSnapshots, NUMBER_AI_CHANNELS };
		hsize_t singleMeasurementDims[] = { 1, NUMBER_AI_CHANNELS };
		voltsSetDataSpace = H5::DataSpace( 2, setDims );
		voltsDataSpace = H5::DataSpace( 2, singleMeasurementDims );
		voltsDataSet = aioGroup.createDataSet( "Voltage-Measurements", H5::PredType::NATIVE_DOUBLE, voltsSetDataSpace );
	}
	else
	{
		H5::Group aioGroup ( file.createGroup ( "/AI:NA" ) );
	}
}
 
 
void DataLogger::writeVolts( UINT currentVoltNumber, std::vector<float64> data )
{
	if ( fileIsOpen == false )
	{
		thrower ( "Tried to write to h5 file, but the file is closed!\r\n" );
	}
	// starting coordinates of write area in the h5 file of the array of picture data points.
	hsize_t offset[2] = { currentVoltNumber, 0 };
	hsize_t slabdim[2] = { 1, data.size() };
	try
	{
		voltsSetDataSpace.selectHyperslab( H5S_SELECT_SET, slabdim, offset );
		voltsDataSet.write( data.data( ), H5::PredType::NATIVE_DOUBLE, voltsDataSpace, voltsSetDataSpace);
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "Failed to write data to HDF5 file! Error: " + str( err.getDetailMsg( ) ) + "\n" );
	}
}
 

void DataLogger::logMiscellaneous()
{
	// times, ...
}
 

void DataLogger::closeFile()
{
	if (!fileIsOpen)
	{
		// wasn't open.
		return;
	}
	AndorPicureSetDataSpace.close();
	AndorPictureDataset.close();	
	//BaslerPicDataSpace.close ( );
	BaslerPictureDataset.close ( );
	BaslerPicureSetDataSpace.close ( );
	voltsDataSpace.close( );
	voltsDataSet.close( );
	file.close();
	fileIsOpen = false;
}

 
UINT DataLogger::getNextFileNumber()
{
	return currentDataFileNumber+1;
}
 

int DataLogger::getDataFileNumber()
{
	return currentDataFileNumber;
} 


void DataLogger::logNiawgSettings(MasterThreadInput* input)
{
	H5::Group niawgGroup( file.createGroup( "/NIAWG" ) );
	writeDataSet( input->runNiawg, "Run-NIAWG", niawgGroup );
	if ( input->runNiawg )
	{
		std::vector<std::fstream> intensityScriptFiles;
		UINT seqInc = 0;
		for ( auto config : input->seq.sequence )
		{
			std::string niawgAddr = ProfileSystem::getNiawgScriptAddrFromConfig( config );
			ScriptStream niawgStream;
			MasterManager::loadNiawgScript ( niawgAddr, niawgStream );
			writeDataSet( niawgStream.str( ), "Seq. " + str(seqInc+1) + " NIAWG-Script", niawgGroup );
			seqInc++;
		}
		writeDataSet( NIAWG_SAMPLE_RATE, "NIAWG-Sample-Rate", niawgGroup );
		writeDataSet( NIAWG_GAIN, "NIAWG-Gain", niawgGroup );
	}
	else
	{
		writeDataSet( "", "NA:NIAWG-Script", niawgGroup );
		writeDataSet( -1, "NA:NIAWG-Sample-Rate", niawgGroup );
		writeDataSet( -1, "NA:NIAWG-Gain", niawgGroup );
	}
}


/// simple wrappers for writing data sets


H5::DataSet DataLogger::writeDataSet( bool data, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_HBOOL, H5::DataSpace( 1, rank1 ) );
		dset.write( &data, H5::PredType::NATIVE_HBOOL );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing bool data set to H5 File. bool was " + str( data )
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( ULONGLONG data, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_ULLONG, H5::DataSpace( 1, rank1 ) );
		dset.write( &data, H5::PredType::NATIVE_ULLONG );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing uint data set to H5 File. uint was " + str( data )
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( UINT data, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_UINT, H5::DataSpace( 1, rank1 ) );
		dset.write( &data, H5::PredType::NATIVE_UINT );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing uint data set to H5 File. uint was " + str( data )
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( int data, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
		dset.write( &data, H5::PredType::NATIVE_INT );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing int data set to H5 File. int was " + str( data )
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( double data, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_DOUBLE, H5::DataSpace( 1, rank1 ) );
		dset.write( &data, H5::PredType::NATIVE_DOUBLE );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing double data set to H5 File. double was " + str(data)
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( std::vector<float> dataVec, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		rank1[0] = dataVec.size( );
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_FLOAT, H5::DataSpace( 1, rank1 ) );
		// get from the key file
		dset.write( dataVec.data( ), H5::PredType::NATIVE_FLOAT );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing double float data set to H5 File. Dataset name was " + name
				 + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( std::vector<double> dataVec, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { 1 };
		rank1[0] = dataVec.size( );
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::NATIVE_DOUBLE, H5::DataSpace( 1, rank1 ) );
		// get from the key file
		dset.write( dataVec.data( ), H5::PredType::NATIVE_DOUBLE );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing double vector data set to H5 File. Dataset name was " + name
				 + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}


H5::DataSet DataLogger::writeDataSet( std::string data, std::string name, H5::Group& group )
{
	try
	{
		hsize_t rank1[] = { data.length( ) };
		H5::DataSet dset = group.createDataSet( cstr( name ), H5::PredType::C_S1, H5::DataSpace( 1, rank1 ) );
		dset.write( cstr( data ), H5::PredType::C_S1 );
		return dset;
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing string data set to H5 File. String was " + data
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}

void DataLogger::writeAttribute( double data, std::string name, H5::DataSet& dset )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::Attribute attr = dset.createAttribute( cstr( name ), H5::PredType::NATIVE_DOUBLE, H5::DataSpace( 1, rank1 ) );
		attr.write( H5::PredType::NATIVE_DOUBLE, &data );
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing bool attribute to H5 File. bool was " + str( data )
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}

void DataLogger::writeAttribute( bool data, std::string name, H5::DataSet& dset )
{
	try
	{
		hsize_t rank1[] = { 1 };
		H5::Attribute attr = dset.createAttribute( cstr(name), H5::PredType::NATIVE_HBOOL, H5::DataSpace( 1, rank1 ) );
		attr.write( H5::PredType::NATIVE_HBOOL, &data);
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: error while writing bool attribute to H5 File. bool was " + str(data)
				 + ". Dataset name was " + name + ". Error was :\r\n" + err.getDetailMsg( ) );
	}
}



