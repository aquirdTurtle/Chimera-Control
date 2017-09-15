#include "stdafx.h"
#include "DataLogger.h"
#include "externals.h"
#include "longnam.h"
#include "DataAnalysisHandler.h"
#include "CameraImageDimensions.h"


DataLogger::DataLogger(std::string systemLocation)
{
	// initialize this to false.
	fileIsOpen = false;
	dataFilesBaseLocation = systemLocation;
}


// this file assumes that h5 is the data_#.h5 file. User should check if incDataSet is on before calling. ???
void DataLogger::deleteFile(Communicator* comm)
{
	if (fileIsOpen)
	{
		// I'm not actually sure if this should be a prob with h5.
		thrower("ERROR: Can't delete current h5 file, the h5 file is open!");
	}
	std::string fileAddress = dataFilesBaseLocation + currentSaveFolder + "\\Raw Data\\data_"
		+ str( currentDataFileNumber ) + ".h5";
	int success = DeleteFile(cstr(fileAddress));
	if (success == false)
	{
		thrower("Failed to delete h5 file! Error code: " + str(GetLastError()) + ".\r\n");
	}
	else
	{
		comm->sendStatus( "Deleted h5 file located at \"" + fileAddress + "\"\r\n" );
	}
}


void DataLogger::initializeDataFiles()
{
	// if the function fails, the h5 file will not be open. If it succeeds, this will get set to true.
	fileIsOpen = false;
	/// First, create the folder for today's h5 data.
	// Get the date and use it to set the folder where this data run will be saved.
	// get time now
	
	time_t timeInt = time(0);
	struct tm timeStruct;
	localtime_s(&timeStruct, &timeInt);
	std::string tempStr = str(timeStruct.tm_year + 1900);
	// Create the string of the date.
	std::string finalSaveFolder;
	finalSaveFolder += tempStr + "\\";
	std::string month;
	switch (timeStruct.tm_mon + 1)
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
	
	finalSaveFolder += month + "\\";
	finalSaveFolder += month + " " + str(timeStruct.tm_mday);
	// right now the save folder IS the date...
	currentSaveFolder = finalSaveFolder;
	// create date's folder.
	int result = 1;
	struct stat info;
	int resultStat = stat( cstr( dataFilesBaseLocation + finalSaveFolder ), &info );
	if (resultStat != 0)
	{
		result = CreateDirectory( cstr( dataFilesBaseLocation + finalSaveFolder ), 0 );
	}
	if (!result)
	{
		thrower( "ERROR: Failed to create save location for data at location " + dataFilesBaseLocation + finalSaveFolder +
				 ". Make sure you have access to the jilafile or change the save location. Error: " + str(GetLastError()) 
				 + "\r\n" );
	}
	finalSaveFolder += "\\Raw Data";
	resultStat = stat( cstr( dataFilesBaseLocation + finalSaveFolder ), &info );
	if (resultStat != 0)
	{
		result = CreateDirectory( cstr( dataFilesBaseLocation + finalSaveFolder ), 0 );
	}
	if (!result)
	{
		thrower( "ERROR: Failed to create save location for data! Error: " + str( GetLastError() ) + "\r\n" );
	}
	finalSaveFolder += "\\";
	/// Get a filename appropriate for the data
	std::string finalSaveFileName;

	// find the first data file that hasn't been already written, starting with data_1.h5
	int fileNum = 1;
	// The while condition here check if file exists. No idea how this actually works.
	struct stat statBuffer;
	// figure out the next file number
	while ((stat(cstr(dataFilesBaseLocation + finalSaveFolder + "data_" + str(fileNum) + ".h5"),
		   &statBuffer) == 0))
	{
		fileNum++;
	}
	// at this point a valid filename has been found.
	finalSaveFileName = "data_" + str(fileNum) + ".h5";
	// update this, which is used later to move the key file.
	currentDataFileNumber = fileNum;
	
	try
	{
		// create the file. H5F_ACC_TRUNC means it will overwrite files with the same name.
		file = H5::H5File( cstr( dataFilesBaseLocation + finalSaveFolder + finalSaveFileName ), H5F_ACC_TRUNC );
		H5::Group ttlsGroup( file.createGroup( "/TTLs" ) );
		// initial settings
		// list of commands
		H5::Group dacsGroup( file.createGroup( "/DACs" ) );
		// initial settings
		// list of commands
		H5::Group tektronicsGroup( file.createGroup( "/Tektronics" ) );
		// mode, freq, power

		H5::Group miscellaneousGroup( file.createGroup( "/Miscellaneous" ) );

		time_t t = time( 0 );   // get time now
		struct tm now;
		localtime_s( &now, &t );
		std::string dateString = str( now.tm_year + 1900 ) + "-" + str( now.tm_mon + 1 ) + "-" + str( now.tm_mday );
		hsize_t rank1[] = { 1 };
		rank1[0] = dateString.size( );
		H5::DataSet dateSet = miscellaneousGroup.createDataSet( "Run-Date", H5::PredType::C_S1, 
																H5::DataSpace( 1, rank1 ) );
		dateSet.write( cstr(dateString), H5::PredType::C_S1 );

		std::string timeString = str( now.tm_hour) + ":" + str( now.tm_min) + ":" + str( now.tm_sec) + ":";
		rank1[0] = timeString.size( );
		H5::DataSet timeSet = miscellaneousGroup.createDataSet( "Time-Of-Logging", H5::PredType::C_S1,
																H5::DataSpace( 1, rank1 ) );
		timeSet.write( cstr( timeString ), H5::PredType::C_S1 );
		fileIsOpen = true;
	}
	catch (H5::Exception err)
	{
		thrower( "ERROR: Failed to initialize HDF5 data file: " + err.getDetailMsg() );
	}
}


void DataLogger::logAndorSettings( AndorRunSettings settings, bool on)
{
	try
	{
		if ( !on )
		{
			H5::Group andorGroup( file.createGroup( "/Andor:NA" ) );
			return;
		}
		// in principle there are some other low level settings or things that aren't used very often which I could include 
		// here. I'm gonna leave this for now though.

		H5::Group andorGroup( file.createGroup( "/Andor" ) );
		hsize_t rank1[] = { 1 };
		// pictures. These are permanent members of the class for speed during the writing process.	
		hsize_t setDims[] = { ULONGLONG( settings.totalPicsInExperiment ), settings.imageSettings.width, settings.imageSettings.height };
		hsize_t picDims[] = { 1, settings.imageSettings.width, settings.imageSettings.height };
		picureSetDataSpace = H5::DataSpace( 3, setDims );
		picDataSpace = H5::DataSpace( 3, picDims );
		pictureDataset = andorGroup.createDataSet( "Pictures", H5::PredType::NATIVE_LONG, picureSetDataSpace );
		// camera mode
		rank1[0] = settings.cameraMode.size( );
		H5::DataSet modeSet = andorGroup.createDataSet( "Camera-Mode", H5::PredType::C_S1, H5::DataSpace( 1, rank1 ) );
		modeSet.write( cstr( settings.cameraMode ), H5::PredType::C_S1 );
		// exposure times
		rank1[0] = settings.exposureTimes.size( );
		H5::DataSet exposureSet = andorGroup.createDataSet( "Exposure-Times", H5::PredType::NATIVE_FLOAT,
															H5::DataSpace( 1, rank1 ) );
		exposureSet.write( settings.exposureTimes.data( ), H5::PredType::NATIVE_FLOAT );
		// Trigger Mode
		rank1[0] = settings.triggerMode.size( );
		H5::DataSet trigSet = andorGroup.createDataSet( "Trigger-Mode", H5::PredType::C_S1, H5::DataSpace( 1, rank1 ) );
		trigSet.write( cstr( settings.triggerMode ), H5::PredType::C_S1 );
		// EM-Gain-Mode
		rank1[0] = 1;
		H5::DataSet emGainSet = andorGroup.createDataSet( "EM-Gain-Mode-On", H5::PredType::NATIVE_HBOOL,
														  H5::DataSpace( 1, rank1 ) );
		emGainSet.write( &settings.emGainModeIsOn, H5::PredType::NATIVE_HBOOL );
		// EM-Gain-Level
		if ( settings.emGainModeIsOn )
		{
			H5::DataSet emGainLevelSet = andorGroup.createDataSet( "EM-Gain-Level", H5::PredType::NATIVE_INT,
																   H5::DataSpace( 1, rank1 ) );
			emGainLevelSet.write( &settings.emGainLevel, H5::PredType::NATIVE_INT );
		}
		// image settings
		H5::Group imageDims = andorGroup.createGroup( "Image-Dimensions" );
		H5::DataSet bottomSet = imageDims.createDataSet( "Bottom", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
		bottomSet.write( &settings.imageSettings.top, H5::PredType::NATIVE_INT );
		H5::DataSet topSet = imageDims.createDataSet( "Top", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
		topSet.write( &settings.imageSettings.bottom, H5::PredType::NATIVE_INT );
		H5::DataSet leftSet = imageDims.createDataSet( "Left", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
		leftSet.write( &settings.imageSettings.left, H5::PredType::NATIVE_INT );
		H5::DataSet rightSet = imageDims.createDataSet( "Right", H5::PredType::NATIVE_INT, H5::DataSpace( 1, rank1 ) );
		rightSet.write( &settings.imageSettings.right, H5::PredType::NATIVE_INT );
		H5::DataSet hBinSet = imageDims.createDataSet( "Horizontal-Binning", H5::PredType::NATIVE_INT,
													   H5::DataSpace( 1, rank1 ) );
		hBinSet.write( &settings.imageSettings.horizontalBinning, H5::PredType::NATIVE_INT );
		H5::DataSet vBinSet = imageDims.createDataSet( "Vertical-Binning", H5::PredType::NATIVE_INT,
													   H5::DataSpace( 1, rank1 ) );
		vBinSet.write( &settings.imageSettings.verticalBinning, H5::PredType::NATIVE_INT );
		// Temp
		H5::DataSet tempSet = andorGroup.createDataSet( "Temperature-Setting", H5::PredType::NATIVE_INT,
														H5::DataSpace( 1, rank1 ) );
		tempSet.write( &settings.temperatureSetting, H5::PredType::NATIVE_INT );
		// picsPerRep
		rank1[0] = 1;
		H5::DataSet picsPerRepSet = andorGroup.createDataSet( "Pictures-Per-Repetition", H5::PredType::NATIVE_ULONG,
															  H5::DataSpace( 1, rank1 ) );
		picsPerRepSet.write( &settings.picsPerRepetition, H5::PredType::NATIVE_ULONG );
		// reps 
		H5::DataSet repsSet = andorGroup.createDataSet( "Repetitions-Per-Variation", H5::PredType::NATIVE_ULONG,
														H5::DataSpace( 1, rank1 ) );
		picsPerRepSet.write( &settings.repetitionsPerVariation, H5::PredType::NATIVE_ULONG );
		// variations
		H5::DataSet variationSet = andorGroup.createDataSet( "Variation-Number", H5::PredType::NATIVE_ULONG,
															 H5::DataSpace( 1, rank1 ) );
		variationSet.write( &settings.totalVariations, H5::PredType::NATIVE_ULONG );
	}
	catch ( H5::Exception err )
	{
		thrower( "ERROR: Failed to log andor parameters in HDF5 file: " + err.getDetailMsg( ) );
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
		// to include
		hsize_t rank1[] = { 1 };
		// - program master option
		H5::DataSet programMasterOption( runParametersGroup.createDataSet( "Run-Master", H5::PredType::NATIVE_HBOOL,
																		   H5::DataSpace( 1, rank1 ) ) );
		programMasterOption.write( &input->runMaster, H5::PredType::NATIVE_HBOOL );

		if ( input->runMaster )
		{
			// - master script
			std::ifstream masterScript( input->masterScriptAddress );
			if ( !masterScript.is_open( ) )
			{
				thrower( "ERROR: Failed to load master script!" );
			}
			std::string scriptBuf( str( masterScript.rdbuf( ) ) );
			rank1[0] = scriptBuf.size( );
			H5::DataSet scriptDataset = runParametersGroup.createDataSet( "Master-Script", H5::PredType::C_S1,
																		  H5::DataSpace( 1, rank1 ) );
			scriptDataset.write( cstr( scriptBuf ), H5::PredType::C_S1 );
			// file address of script
			rank1[0] = input->masterScriptAddress.size( );
			H5::Attribute scriptNameAttr( scriptDataset.createAttribute( "File-Address", H5::PredType::C_S1,
																		 H5::DataSpace( 1, rank1 ) ) );
			scriptNameAttr.write( H5::PredType::C_S1, cstr( input->masterScriptAddress ) );
		}
		else
		{
			H5::DataSet scriptDataset = runParametersGroup.createDataSet( "Master-Script:NA", H5::PredType::C_S1,
																		  H5::DataSpace( 1, rank1 ) );
			H5::Attribute scriptNameAttr( scriptDataset.createAttribute( "Master-Script-Address:NA", H5::PredType::C_S1,
																		 H5::DataSpace( 1, rank1 ) ) );
		}
		// - repetitions
		rank1[0] = 1;
		H5::DataSet repSet = runParametersGroup.createDataSet( "Repetitions", H5::PredType::NATIVE_LONG,
															   H5::DataSpace( 1, rank1 ) );
		repSet.write( &input->repetitionNumber, H5::PredType::NATIVE_LONG );
		// - debuging options

		/// Variables
		// - variables / key info
		H5::Group variableGroup = runParametersGroup.createGroup( "Variables" );
		for ( auto& variable : input->variables )
		{
			H5::DataSet varSet;
			if ( variable.constant )
			{
				rank1[0] = 1;
				varSet = variableGroup.createDataSet( cstr( variable.name ), H5::PredType::NATIVE_DOUBLE,
													  H5::DataSpace( 1, rank1 ) );
				// just grab from variable value
				varSet.write( &variable.ranges.front( ).initialValue, H5::PredType::NATIVE_DOUBLE );
			}
			else
			{
				rank1[0] = input->key->getKey( )[variable.name].first.size( );
				varSet = variableGroup.createDataSet( cstr( variable.name ), H5::PredType::NATIVE_DOUBLE,
													  H5::DataSpace( 1, rank1 ) );
				// get from the key file
				varSet.write( input->key->getKey( )[variable.name].first.data( ), H5::PredType::NATIVE_DOUBLE );
			}
			rank1[0] = 1;
			H5::Attribute attr = varSet.createAttribute( "Constant", H5::PredType::NATIVE_HBOOL, H5::DataSpace( 1, rank1 ) );
			attr.write( H5::PredType::NATIVE_HBOOL, &variable.constant );
		}

		/// NIAWG
		H5::Group niawgGroup( file.createGroup( "/NIAWG" ) );

		H5::DataSet runNiawgSet = niawgGroup.createDataSet( "Run-NIAWG", H5::PredType::NATIVE_HBOOL,
															H5::DataSpace( 1, rank1 ) );
		runNiawgSet.write( &input->runNiawg, H5::PredType::NATIVE_HBOOL );

		if ( input->runNiawg )
		{
			niawgPair<std::vector<std::fstream>> niawgFiles;
			std::vector<std::fstream> intensityScriptFiles;
			ProfileSystem::openNiawgFiles( niawgFiles, input->profile, input->runNiawg );
			// Hor Script
			std::stringstream stream;
			stream << niawgFiles[Horizontal][0].rdbuf( );
			rank1[0] = stream.str( ).size( );
			H5::DataSet horScriptSet = niawgGroup.createDataSet( "Horizontal-NIAWG-Script", H5::PredType::C_S1,
																 H5::DataSpace( 1, rank1 ) );
			horScriptSet.write( cstr( stream.str( ) ), H5::PredType::C_S1 );
			// vert script;
			stream = std::stringstream( );
			stream << niawgFiles[Vertical][0].rdbuf( );
			rank1[0] = stream.str( ).size( );
			H5::DataSet vertScriptSet = niawgGroup.createDataSet( "Vertical-NIAWG-Script", H5::PredType::C_S1,
																  H5::DataSpace( 1, rank1 ) );
			vertScriptSet.write( cstr( stream.str( ) ), H5::PredType::C_S1 );
			// sample rate
			rank1[0] = 1;
			H5::DataSet sampleSet = niawgGroup.createDataSet( "NIAWG-Sample-Rate", H5::PredType::NATIVE_DOUBLE,
															  H5::DataSpace( 1, rank1 ) );
			sampleSet.write( &NIAWG_SAMPLE_RATE, H5::PredType::NATIVE_UINT );

			H5::DataSet gainSet = niawgGroup.createDataSet( "NIAWG-GAIN", H5::PredType::NATIVE_DOUBLE,
															H5::DataSpace( 1, rank1 ) );
			int gain = NIAWG_GAIN;
			gainSet.write( &gain, H5::PredType::NATIVE_DOUBLE );
		}
		else
		{
			niawgGroup.createDataSet( "Horizontal-NIAWG-Script:NA", H5::PredType::C_S1, H5::DataSpace( 1, rank1 ) );
			niawgGroup.createDataSet( "Vertical-NIAWG-Script:NA", H5::PredType::C_S1, H5::DataSpace( 1, rank1 ) );
			niawgGroup.createDataSet( "NIAWG-Sample-Rate:NA", H5::PredType::NATIVE_DOUBLE, H5::DataSpace( 1, rank1 ) );
			niawgGroup.createDataSet( "NIAWG-GAIN", H5::PredType::NATIVE_DOUBLE, H5::DataSpace( 1, rank1 ) );
		}

		/// agilents
		H5::Group agilentsGroup( file.createGroup( "/Agilents" ) );
		for ( auto& agilent : input->agilents )
		{
			H5::Group singleAgilent( agilentsGroup.createGroup( agilent->getName( ) ) );
			// mode
			deviceOutputInfo info = agilent->getOutputInfo( );
			UINT channelCount = 1;
			for ( auto& channel : info.channel )
			{
				H5::Group channelGroup( singleAgilent.createGroup( "Channel-" + str( channelCount ) ) );

				H5::DataSet modeSet = channelGroup.createDataSet( "Output-Mode", H5::PredType::C_S1,
																H5::DataSpace( 1, rank1 ) );
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
				rank1[0] = outputModeName.size( );
				modeSet.write( cstr( outputModeName ), H5::PredType::C_S1 );
				// dc
				H5::Group dcSettings( channelGroup.createGroup( "DC-Settings" ) );
				rank1[0] = channel.dc.dcLevelInput.expressionStr.size( );
				H5::DataSet dcLevelSet = dcSettings.createDataSet( "DC-Level", H5::PredType::C_S1,
																   H5::DataSpace( 1, rank1 ) );
				dcLevelSet.write( cstr( channel.dc.dcLevelInput.expressionStr ), H5::PredType::C_S1 );
				// sine 
				H5::Group sineSettingsGroup( channelGroup.createGroup( "Sine-Settings" ) );
				rank1[0] = channel.sine.frequencyInput.expressionStr.size( );
				H5::DataSet sineFreqSet = sineSettingsGroup.createDataSet( "Frequency", H5::PredType::C_S1,
																		   H5::DataSpace( 1, rank1 ) );
				sineFreqSet.write( cstr( channel.sine.frequencyInput.expressionStr ), H5::PredType::C_S1 );

				rank1[0] = channel.sine.amplitudeInput.expressionStr.size( );
				H5::DataSet sineAmpSet = sineSettingsGroup.createDataSet( "Ampiltude", H5::PredType::C_S1,
																		  H5::DataSpace( 1, rank1 ) );
				sineAmpSet.write( cstr( channel.sine.amplitudeInput.expressionStr ), H5::PredType::C_S1 );

				//
				H5::Group squareSettings( channelGroup.createGroup( "Square-Settings" ) );
				rank1[0] = channel.square.amplitudeInput.expressionStr.size( );
				H5::DataSet squareAmpSet = squareSettings.createDataSet( "Ampiltude", H5::PredType::C_S1,
																		 H5::DataSpace( 1, rank1 ) );
				squareAmpSet.write( cstr( channel.square.amplitudeInput.expressionStr ), H5::PredType::C_S1 );

				rank1[0] = channel.square.frequencyInput.expressionStr.size( );
				H5::DataSet squareFreqSet = squareSettings.createDataSet( "Frequency", H5::PredType::C_S1,
																		  H5::DataSpace( 1, rank1 ) );
				squareFreqSet.write( cstr( channel.square.frequencyInput.expressionStr ), H5::PredType::C_S1 );

				rank1[0] = channel.square.offsetInput.expressionStr.size( );
				H5::DataSet squareoffsetSet = squareSettings.createDataSet( "Offset", H5::PredType::C_S1,
																			H5::DataSpace( 1, rank1 ) );
				squareoffsetSet.write( cstr( channel.square.offsetInput.expressionStr ), H5::PredType::C_S1 );
				//
				H5::Group preloadedArbSettings( channelGroup.createGroup( "Preloaded-Arb-Settings" ) );
				rank1[0] = channel.preloadedArb.address.size( );
				H5::DataSet preloadedAddress = preloadedArbSettings.createDataSet( "Address", H5::PredType::C_S1,
																				   H5::DataSpace( 1, rank1 ) );
				preloadedAddress.write( cstr( channel.preloadedArb.address ), H5::PredType::C_S1 );

				H5::Group scriptedArbSettings( channelGroup.createGroup( "Scripted-Arb-Settings" ) );
				rank1[0] = channel.scriptedArb.fileAddress.size( );
				H5::DataSet scriptAddress = preloadedArbSettings.createDataSet( "File-Address", H5::PredType::C_S1,
																				H5::DataSpace( 1, rank1 ) );
				scriptAddress.write( cstr( channel.scriptedArb.fileAddress ), H5::PredType::C_S1 );
				// TODO: load file itself
				channelCount++;
			}
		}
	}
	catch ( H5::Exception& err )
	{
		thrower( "ERROR: Failed to log master parameters in HDF5 file: detail:" + err.getDetailMsg( ) );
				 //+ ", major:" + err.getMajorString(err.) + ", minor:" + err.getMinorString(err));
	}
}


void DataLogger::writePic(UINT currentPictureNumber, std::vector<long> image, imageParameters dims)
{
	if (fileIsOpen == false)
	{
		thrower("Tried to write to h5 file, but the file is closed!\r\n");
	}
	// MUST initialize status
	// starting coordinates of write area in the h5 file of the array of picture data points.
	hsize_t offset[] = { currentPictureNumber-1, 0, 0 };
	hsize_t slabdim[3] = { 1, dims.width, dims.height };
	std::vector<long> im( image.size(), 0 );
	try
	{
		picureSetDataSpace.selectHyperslab( H5S_SELECT_SET, slabdim, offset );
		pictureDataset.write( image.data(), H5::PredType::NATIVE_LONG, picDataSpace, picureSetDataSpace );
	}
	catch (H5::Exception& err)
	{
		thrower("Failed to write data to HDF5 file! Error: " + str(err.getDetailMsg()) + "\n");
	}
}


void DataLogger::logMiscellaneous()
{
	// time
}


void DataLogger::closeFile()
{
	if (!fileIsOpen)
	{
		// wasn't open.
		return;
	}
	picureSetDataSpace.close();
	pictureDataset.close();	
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


void DataLogger::logNiawgSettings()
{
}