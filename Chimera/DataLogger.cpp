// created by Mark O. Brown
#include "stdafx.h"
#include "DataLogger.h"
#include "externals.h"
#include "longnam.h"
#include "DataAnalysisControl.h"
#include "CameraImageDimensions.h"
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
		case 1:  month = "January";		break;
		case 2:  month = "February";	break;
		case 3:  month = "March";		break;
		case 4:  month = "April";		break;
		case 5:  month = "May";			break; 
		case 6:  month = "June";		break;
		case 7:  month = "July";		break;
		case 8:  month = "August";		break; 
		case 9:  month = "September";	break;
		case 10: month = "October";		break; 
		case 11: month = "November";	break;
		case 12: month = "December";	break;
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
void DataLogger::initializeDataFiles( std::string specialName, bool checkForCalibrationFiles )
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
	if ( checkForCalibrationFiles )
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
	}
	catch (H5::Exception err)
	{
		logError ( err );
		throwNested ( "ERROR: Failed to initialize HDF5 data file: " + err.getDetailMsg() );
	}
}


void DataLogger::logPlotData ( std::string name, std::vector<pPlotDataVec> data )
{

}


void DataLogger::logServoInfo ( std::vector<servoInfo> servos )
{
	H5::Group servoGroup( file.createGroup ( "/Servos" ) );
	for ( auto servo : servos )
	{
		H5::Group thisServo ( servoGroup.createGroup ( "/" + servo.servoName ) );
		writeDataSet ( servo.active, "Servo_Active", thisServo );
		writeDataSet ( servo.aiInputChannel, "AI_Input_Channel", thisServo );
		writeDataSet ( servo.aoControlChannel, "AO_Control_Channel", thisServo );
		writeDataSet ( servo.controlValue, "Control_Value", thisServo );
		writeDataSet ( servo.gain, "Servo_Gain", thisServo );
		writeDataSet ( servo.servoed, "Servo_Is_Servoing_Correctly", thisServo );
		writeDataSet ( servo.setPoint, "Set_Point", thisServo );
		writeDataSet ( servo.tolerance, "Servo_Tolerance", thisServo );
		std::string ttlConfigStr;
		for ( auto ttl : servo.ttlConfig )
		{
			ttlConfigStr += DioRows::toStr(ttl.first) + str(ttl.second) + ", ";
		}
		if ( ttlConfigStr.size ( ) > 2 )
		{
			// kill last comma and space.
			ttlConfigStr.substr ( 0, ttlConfigStr.size ( ) - 2 );
		}
		writeDataSet ( ttlConfigStr, "TTL_Configuration_During_Servo", thisServo );
	}
}


void DataLogger::logAoSystemSettings ( AoSystem& aoSys )
{
	auto info = aoSys.getDacInfo ( );
	H5::Group AoSystemGroup ( file.createGroup ( "/Ao_System" ) );
	UINT count = 0;
	for ( auto& output : info )
	{
		H5::Group indvOutput( AoSystemGroup.createGroup ( "/Output_" + str(count++) ) );
		writeDataSet ( output.name, "Output_Name", indvOutput );
		writeDataSet ( output.note, "Note", indvOutput );
		writeDataSet ( output.currVal, "Value_At_Start", indvOutput );
		writeDataSet ( output.defaultVal, "Default_Value", indvOutput );
		writeDataSet ( output.minVal, "Minimum_Value", indvOutput );
		writeDataSet ( output.maxVal, "Maximum_Value", indvOutput );
	}
}

void DataLogger::logDoSystemSettings ( DioSystem& doSys )
{
	auto& doSysOutputs = doSys.getDigitalOutputs ( );
	H5::Group DoSystemGroup ( file.createGroup ( "/Do_System" ) );
	UINT count = 0;
	for ( auto& out : doSysOutputs )
	{
		auto pos = out.getPosition ( );
		H5::Group indvOutput ( DoSystemGroup.createGroup ( "/" + DioRows::toStr( pos.first) + str(pos.second) ) );
		writeDataSet ( out.getName(), "Name", indvOutput );
		writeDataSet ( out.defaultStatus, "Default_Status", indvOutput );
		writeDataSet ( out.getStatus(), "Value_at_start", indvOutput );
		// not sure it makes sense to report this but why not.
		writeDataSet ( out.getShadeStatus(), "Shade_Status", indvOutput );
	}
}



void DataLogger::logTektronicsSettings ( TektronicsAfgControl& tek )
{
	auto info = tek.getTekSettings ( );
	try
	{
		H5::Group tektronicsGroup;
		try
		{
			tektronicsGroup = H5::Group( file.createGroup ( "/Tektronics" ) );
		}
		catch ( H5::Exception err )
		{
			// probably has just already been created.
			tektronicsGroup = H5::Group ( file.openGroup ( "/Tektronics" ) );
		}
		H5::Group thisTek ( tektronicsGroup.createGroup ( tek.configDelim ) );
		writeDataSet ( info.machineAddress, "Machine-Address", thisTek );
		UINT channelCount = 1;
		for ( auto c : { info.channels.first, info.channels.second } )
		{
			H5::Group thisChannel ( thisTek.createGroup ( "Channel_" + str ( channelCount++ ) ) );
			writeDataSet ( c.control, "Controlled_Option", thisChannel );
			writeDataSet ( c.on, "Output_On", thisChannel ); 
			writeDataSet ( c.power.expressionStr, "Power", thisChannel ); 
			writeDataSet ( c.mainFreq.expressionStr, "Main_Frequency", thisChannel );
			writeDataSet ( c.fsk, "FSK_Option", thisChannel );
			writeDataSet ( c.fskFreq.expressionStr, "FSK_Frequency", thisChannel );
		}
	}
	catch ( H5::Exception err)
	{
		logError ( err );
		throwNested ( "Failed to write tektronics settings to the HDF5 data file!" );
	}
}



void DataLogger::logAgilentSettings( const std::vector<Agilent*>& agilents )
{
	H5::Group agilentsGroup( file.createGroup( "/Agilents" ) );
	for ( auto& agilent : agilents )
	{
		H5::Group singleAgilent( agilentsGroup.createGroup( agilent->configDelim ) );
		deviceOutputInfo info = agilent->getOutputInfo( );
		UINT channelCount = 1;
		writeDataSet ( agilent->getStartupCommands(), "Startup-Commands", singleAgilent );
		for ( auto& channel : info.channel )
		{
			H5::Group channelGroup( singleAgilent.createGroup( "Channel-" + str( channelCount ) ) );
			std::string outputModeName = AgilentChannelMode::toStr(channel.option);
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
			writeDataSet( channel.scriptedArb.fileAddress, "Script-File-Address", scriptedArbSettings );
			// TODO: load script file itself
			ScriptStream stream;
			try
			{
				MasterThreadManager::loadAgilentScript ( channel.scriptedArb.fileAddress, stream );
				writeDataSet ( stream.str ( ), "Agilent-Script-Script", scriptedArbSettings );
			}
			catch ( Error& )
			{
				// failed to open, that's probably fine, 
				writeDataSet ( "Script Failed to load.", "Agilent-Script-Script", scriptedArbSettings );
			}
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
		throwNested ( "Failed to write basler pic data to HDF5 file! Error: " + str ( err.getDetailMsg ( ) ) + "\n" );
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
		if (settings.acquisitionMode == AndorRunModes::mode::Kinetic) {
			hsize_t setDims[] = { ULONGLONG (settings.totalPicsInExperiment ()), settings.imageSettings.width (),
				settings.imageSettings.height () };
			hsize_t picDims[] = { 1, settings.imageSettings.width (), settings.imageSettings.height () };
			AndorPicureSetDataSpace = H5::DataSpace (3, setDims);
			AndorPicDataSpace = H5::DataSpace (3, picDims);
			AndorPictureDataset = andorGroup.createDataSet ("Pictures", H5::PredType::NATIVE_LONG, AndorPicureSetDataSpace);
			currentAndorPicNumber = 0;
		}
		else
		{
			/*
			hsize_t setDims[] = { 0, settings.imageSettings.width (), settings.imageSettings.height () };
			hsize_t picDims[] = { 1, settings.imageSettings.width (), settings.imageSettings.height () };
			AndorPicureSetDataSpace = H5::DataSpace (3, setDims);
			AndorPicDataSpace = H5::DataSpace (3, picDims);
			AndorPictureDataset = andorGroup.createDataSet ("Pictures: N/A", H5::PredType::NATIVE_LONG, AndorPicureSetDataSpace);
			*/
		}
		writeDataSet( int(settings.acquisitionMode), "Camera-Mode", andorGroup );
		writeDataSet( settings.exposureTimes, "Exposure-Times", andorGroup );
		writeDataSet( AndorTriggerMode::toStr(settings.triggerMode), "Trigger-Mode", andorGroup );
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

/*
This function is for logging things that are read from the configuration file and otherwise obtained inside the main experiment thread.
*/
void DataLogger::logMasterRuntime ( UINT repNumber,  std::vector<std::vector<parameterType>> allParams)
{
	try
	{
		H5::Group runtimeGroup ( file.createGroup ( "/Master-Runtime" ) );
		writeDataSet ( repNumber, "Repetitions", runtimeGroup );
		UINT count = 0;
		for ( auto& seqParams : allParams )
		{
			logParameters ( seqParams, runtimeGroup, count );
			count++;
		}
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: Failed to log master runtime in HDF5 file: detail:" + err.getDetailMsg ( ) );
	}
}


void DataLogger::logMasterInput( ExperimentThreadInput* input )
{
	try
	{
		if ( input == NULL )
		{
			H5::Group runParametersGroup( file.createGroup( "/Master-Input:NA" ) );
			return;
		}
		H5::Group runParametersGroup( file.createGroup( "/Master-Input" ) );
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
		logNiawgSettings( input );
		logAgilentSettings( input->agilents );
		logTektronicsSettings ( input->topBottomTek );
		logTektronicsSettings ( input->eoAxialTek );
		logAoSystemSettings ( input->aoSys );
		logDoSystemSettings ( input->ttls );
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "ERROR: Failed to log master parameters in HDF5 file: detail:" + err.getDetailMsg( ) );
	}
}


void DataLogger::logParameters( const std::vector<parameterType>& parameters, H5::Group& group, UINT seqInc )
{
	H5::Group paramGroup = group.createGroup( "Seq #" + str(seqInc+1) + " Parameters" );
	for ( auto& param : parameters )
	{
		H5::DataSet varSet;
		if ( param.constant )
		{
			varSet = writeDataSet( param.constantValue, param.name, paramGroup );
		}
		else
		{
			varSet = writeDataSet( param.keyValues, param.name, paramGroup );
		}
		writeAttribute( param.constant, "Constant", varSet );
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
		throwNested ("Failed to write andor pic data to HDF5 file! Error: " + str(err.getDetailMsg()) + "\n");
	}
}

 
void DataLogger::initializeAiLogging( UINT numSnapshots )
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
		throwNested ( "Failed to write Analog voltage data to HDF5 file! Error: " + str( err.getDetailMsg( ) ) + "\n" );
	}
}
 

std::string DataLogger::getMostRecentDateString ( )
{
	return mostRecentDateString;
}


void DataLogger::logMiscellaneousStart()
{
	try
	{
		H5::Group miscellaneousGroup ( file.createGroup ( "/Miscellaneous" ) );
		time_t t = time ( 0 );   // get time now
		struct tm now;
		localtime_s ( &now, &t );
		std::string monStr;
		switch ( now.tm_mon )
		{
			case 0: monStr = "January"; break;
			case 1: monStr = "February"; break;
			case 2: monStr = "March"; break;
			case 3: monStr = "April"; break;
			case 4: monStr = "May"; break;
			case 5: monStr = "June"; break;
			case 6: monStr = "July"; break;
			case 7: monStr = "August"; break;
			case 8: monStr = "September"; break;
			case 9: monStr = "October"; break;
			case 10: monStr = "November"; break;
			case 11: monStr = "December"; break;
		}
		mostRecentDateString = str ( now.tm_mday ) + "," + monStr + "," + str ( now.tm_year + 1900 );
		writeDataSet ( str ( now.tm_year + 1900 ) + "-" + str ( now.tm_mon + 1 ) + "-" + str ( now.tm_mday ),
					   "Start-Date", miscellaneousGroup );
		writeDataSet ( str ( now.tm_hour ) + ":" + str ( now.tm_min ) + ":" + str ( now.tm_sec ) + ":",
					   "Start-Time", miscellaneousGroup );
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		throwNested ( "Failed to write miscellaneous start data to HDF5 file! Error: " + str ( err.getDetailMsg ( ) ) + "\n" );
	}
}
 

void DataLogger::closeFile()
{
	if (!fileIsOpen)
	{
		// wasn't open.
		return;
	}
	try
	{
		// log the close time.
		H5::Group miscellaneousGroup ( file.openGroup ( "/Miscellaneous" ) );
		time_t t = time ( 0 );   // get time now
		struct tm now;
		localtime_s ( &now, &t );
		writeDataSet ( str ( now.tm_year + 1900 ) + "-" + str ( now.tm_mon + 1 ) + "-" + str ( now.tm_mday ),
					   "Stop-Date", miscellaneousGroup );
		writeDataSet ( str ( now.tm_hour ) + ":" + str ( now.tm_min ) + ":" + str ( now.tm_sec ) + ":",
					   "Stop-Time", miscellaneousGroup );
	}
	catch ( H5::Exception& err )
	{
		logError ( err );
		errBox ( "Failed to write closing information (e.g. time of stop) to H5 File! Error was:\n\n" + str( err.getDetailMsg( ) ) + "\n" );
	}
	AndorPicureSetDataSpace.close();
	AndorPictureDataset.close();
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


void DataLogger::logNiawgSettings(ExperimentThreadInput* input)
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
			MasterThreadManager::loadNiawgScript ( niawgAddr, niawgStream );
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


/* As of right now just append all the strings and insert a newline between. There should be a nice way of doing this.*/
H5::DataSet DataLogger::writeDataSet ( std::vector<std::string> dataVec, std::string name, H5::Group& group )
{
	std::string superString = "";
	for ( auto string : dataVec )
	{
		superString += string + "\n";
	}
	return writeDataSet ( superString, name, group );
}

