// created by Mark O. Brown
#include "stdafx.h"
#include "PlottingInfo.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

PlottingInfo::PlottingInfo(UINT picNumber)
{
	// initialize things.
	generalPlotType = "Atoms";
	title = "";
	yLabel = "";
	fileName = "";
	dataSets.clear();
	dataSets.resize(1);
	dataSets[0].resetPixelNumber( 1 ); 
	dataSets[0].resetPictureNumber(picNumber);	
	currentPixelNumber = 1;
	currentConditionNumber = 1;
	numberOfPictures = picNumber;
	xAxis = "";
}


PlottingInfo::PlottingInfo(std::string fileName)
{
	loadPlottingInfoFromFile(fileName);
}


analysisGroupLocation& PlottingInfo::groupInfo( UINT pixelNumber, UINT pixelSet )
{
	if ( pixelNumber > analysisGroups.size( ) )
	{
		thrower ( "ERROR: pixel number out of range in analysis group access!" );
	}
	if ( pixelSet > analysisGroups[pixelNumber].size( ) )
	{
		thrower ( "ERROR: pixel set out of range in analysis group access!" );
	}
	return analysisGroups[pixelNumber][pixelSet];
}

analysisGroupLocation  PlottingInfo::groupInfo( UINT pixelNumber, UINT pixelSet ) const
{
	if ( pixelNumber > analysisGroups.size( ) )
	{
		thrower ( "ERROR: pixel number out of range in analysis group access!" );
	}
	if ( pixelSet > analysisGroups[pixelNumber].size( ) )
	{
		thrower ( "ERROR: pixel set out of range in analysis group access!" );
	}
	return analysisGroups[pixelNumber][pixelSet];
}


void PlottingInfo::changeTitle(std::string newTitle)
{
	title = newTitle;
}

std::string PlottingInfo::getPrcSettingsString()
{
	std::string allConditionsString = "\r\nAll Result Conditions:\r\n=========================\r\n";
	allConditionsString += "Data Set #, picture #, pixel #:\r\n";
	for (UINT dataSetInc = 0; dataSetInc < getDataSetNumber(); dataSetInc++)
	{
		for (UINT pictureInc = 0; pictureInc < getPicNumber(); pictureInc++)
		{
			for (UINT pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++)
			{
				int currentValue = getResultCondition(dataSetInc, pixelInc, pictureInc);
				allConditionsString += str( dataSetInc + 1 ) + ", " + str( pictureInc + 1 ) + ", " + str( pixelInc + 1 )
					+ " = ";
				if (currentValue == 1)
				{
					allConditionsString += " Atom Present";
				}
				else if (currentValue == -1)
				{
					allConditionsString += " Atom Not Present";
				}
				else
				{
					allConditionsString += " No Condition";
				}
				allConditionsString += "\r\n";
			}
		}
		allConditionsString += "=====\r\n";
	}
	return allConditionsString;
}


std::string PlottingInfo::getPscSettingsString()
{
	std::string allConditionsString = "\r\nAll Current Post-Selection Conditions:\r\n=========================\r\n";
	allConditionsString += "condition #, Data Set #, picture #, pixel #:\r\n";
	for (UINT conditionInc = 0; conditionInc < getConditionNumber(); conditionInc++)
	{
		for (UINT dataSetInc = 0; dataSetInc < getDataSetNumber(); dataSetInc++)
		{
			for (UINT pictureInc = 0; pictureInc < getPicNumber(); pictureInc++)
			{
				for (UINT pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++)
				{
					allConditionsString += str( conditionInc + 1 ) + ", " + str( dataSetInc + 1 ) + ", "
						+ str( pictureInc + 1 ) + ", " + str( pixelInc + 1 ) + " = ";
					int currentValue = getPostSelectionCondition(dataSetInc, conditionInc, pixelInc, pictureInc);
					if (currentValue == 1)
					{
						allConditionsString += " Atom Present";
					}
					else if (currentValue == -1)
					{
						allConditionsString += " Atom Not Present";
					}
					else
					{
						allConditionsString += " No Condition";
					}
					allConditionsString += "\r\n";
				}
			}
			allConditionsString += "=====\r\n";
		}
	}
	return allConditionsString;
}


void PlottingInfo::changeYLabel(std::string newYLabel)
{
	yLabel = newYLabel;
}


void PlottingInfo::changeFileName(std::string newFileName) 
{
	fileName = newFileName;
}


void PlottingInfo::changeGeneralPlotType(std::string newPlotType)
{
	generalPlotType = newPlotType;
}


void PlottingInfo::changeXAxis(std::string newXAxis)
{
	xAxis = newXAxis;
}


void PlottingInfo::addGroup()
{
	for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() + 1);
	}
}


void PlottingInfo::setGroupLocation( UINT pixel, UINT analysisSet, UINT row, UINT column)
{
	groupInfo( pixel, analysisSet ).row = row;
	groupInfo( pixel, analysisSet ).col = column;
}


void PlottingInfo::addPixel()
{
	// change all the data set structures.
	currentPixelNumber++;
	analysisGroups.resize(analysisGroups.size() + 1);
	analysisGroups[analysisGroups.size() - 1].resize(analysisGroups[0].size());
	for (auto& dset : dataSets)
	{
		dset.addPixel(numberOfPictures);
	}
}

void PlottingInfo::removePixel()
{
	// make sure there is a pixel to remove.
	if (currentPixelNumber < 2)
	{
		thrower ("ERROR: Something tried to remove the last pixel!");
	}
	// change all the data set structures.
	currentPixelNumber--;
	for (auto& dset : dataSets)
	{
		dset.removePixel();
	}
}



// Change all structures that depend on the number of data sets.
void PlottingInfo::addDataSet()
{	
	// initialize new data set with 1 condition and the right number of pixels and pictures.
	dataSets.resize(dataSets.size() + 1);
	dataSets[dataSets.size() - 1].initialize(currentConditionNumber, currentPixelNumber, numberOfPictures);
}


void PlottingInfo::removeDataSet()
{
	// make sure there is a data set to remove.
	if (dataSets.size() < 2)
	{
		thrower ("ERROR: Something tried to remove the last data set");
	}	
	dataSets.resize(dataSets.size() - 1);
}

// change all structures that depend on the number of pictures. 
void PlottingInfo::addPicture()
{
	numberOfPictures++;
	for (auto& dset : dataSets)
	{
		dset.addPicture();
	}
}


void PlottingInfo::removePicture()
{
	if (numberOfPictures < 2)
	{
		thrower ("ERROR: Something tried to remove the last picture!");
	}
	numberOfPictures--;
	for (auto& dset : dataSets)
	{
		dset.removePicture();
	}
}


void PlottingInfo::setPostSelCondition( UINT dataSetNumber, UINT conditionNumber, UINT pixel, UINT picture,
											  UINT postSelectionCondition)
{
	dataSets[dataSetNumber].setPostSelectionCondition(conditionNumber, pixel, picture, postSelectionCondition);
}

void PlottingInfo::setResultCondition( UINT dataSetNumber, UINT pixel, UINT picture, UINT positiveResultCondition)
{
	dataSets[dataSetNumber].setResultCondition(pixel, picture, positiveResultCondition);
}

// stores the info in the row and column arguments
void PlottingInfo::getPixelLocation( UINT pixel, UINT analysisSet, UINT& row, UINT& column)
{
	row = groupInfo(pixel, analysisSet).row;
	column = groupInfo( pixel, analysisSet ).col;
}

void PlottingInfo::removeAnalysisSet()
{
	// always at least one pixel...

	// make sure tat there is an analysis set to remove.
	for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		if (analysisGroups[pixelInc].size() < 2)
		{
			thrower ("ERROR: Something tried to remove the last analysis group!");
		}
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() - 1);
	}
}

UINT PlottingInfo::getPicNumberFromFile(std::string fileAddress)
{
	// load the file completely then read the picture number.
	PlottingInfo temp(fileAddress);
	return temp.getPicNumber();
}


UINT PlottingInfo::getResultCondition( UINT dataSetNumber, UINT pixel, UINT picture)
{
	return dataSets[dataSetNumber].getPositiveResultCondition(pixel, picture);
}

UINT PlottingInfo::getPostSelectionCondition( UINT dataSetNumber, UINT conditionNumber, UINT pixel, UINT picture)
{
	return dataSets[dataSetNumber].getPostSelectionCondition(conditionNumber, pixel, picture);
}

void PlottingInfo::addPostSelectionCondition()
{
	for (auto& dset : dataSets )
	{
		// the currentPixelNumber is used as a vector index, the number of pictures is used as a resize size.
		dset.addPostSelectionCondition(currentPixelNumber, numberOfPictures);
	}
	currentConditionNumber++;
}


void PlottingInfo::removePostSelectionCondition()
{
	for ( auto& dset : dataSets )
	{
		dset.removePostSelectionCondition();
	}
	currentConditionNumber--;
}


std::string PlottingInfo::getPlotType()
{
	return generalPlotType;
}

std::string PlottingInfo::getXAxis()
{
	return xAxis;
}

void PlottingInfo::setDataCountsLocation( UINT dataSet, UINT pixel, UINT picture)
{
	if (dataSet < dataSets.size() && !(dataSet < 0))
	{
		dataSets[dataSet].setDataCountsLocation(currentPixelNumber, numberOfPictures, pixel, picture);
	}
	else
	{
		thrower ("ERROR: tried to set data counts location for data set that hasn't been assigned yet. dataSet = " 
				+ str(dataSet));
	}
}


void PlottingInfo::setPlotData( UINT dataSet, bool plotData)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower ( "ERROR: tried to set \"Plot this data\" value for data set that hasn't been assigned. dataSet = "
			     + str(dataSet));
	}
	dataSets[dataSet].setPlotThisData(plotData);
}


void PlottingInfo::setDataSetHistBinWidth( UINT dataSet, UINT width )
{
	if ( dataSet >= dataSets.size( ) || dataSet < 0 )
	{
		thrower ( "ERROR: tried to set Histogram Bin Width value for data set that hasn't been assigned. dataSet = "
				 + str( dataSet ) );
	}
	dataSets[dataSet].setHistBinWidth( width );
}


UINT PlottingInfo::getDataSetHistBinWidth( UINT dataSet )
{
	if ( dataSet >= dataSets.size( ) || dataSet < 0 )
	{
		thrower ( "ERROR: tried to get histogram bin width value for data set that hasn't been assigned. dataSet = "
				 + str( dataSet ) );
	}
	return dataSets[dataSet].getHistBinWidth( );
}

bool PlottingInfo::getPlotThisDataValue( UINT dataSet)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower ("ERROR: tried to get \"Plot this data\" value for data set that hasn't been assigned. dataSet = "
				+ str(dataSet));
	}
	return dataSets[dataSet].getPlotThisDataValue();
}


void PlottingInfo::getDataCountsLocation( UINT dataSet, UINT& pixel, UINT& picture)
{
	dataSets[dataSet].getDataCountsLocation(pixel, picture);
}


void PlottingInfo::changeLegendText( UINT dataSet, std::string newLegend)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower ( "ERROR: attempted to set dataset legend for data set that hadn't been allocated. dataset = " 
			     + str(dataSet));
	}
	dataSets[dataSet].changeLegendText(newLegend);
}


std::string PlottingInfo::getLegendText( UINT dataSet)
{
	if (dataSet >= dataSets.size())
	{
		thrower ( "ERROR: attempted to get dataset legend for data set that hadn't been allocated. dataset = " 
			     + str(dataSet));
	}
	return dataSets[dataSet].getLegendText();
}

std::string PlottingInfo::getAllSettingsString()
{
	std::string message = "All Plotting Parameters (" + fileName + ")\r\n=======================================\r\n\r\n";
	message += "Plot Title: " + title + "\r\n";
	message += "Plot Type: " + generalPlotType + "\r\n";
	message += "Y Label: " + yLabel + "\r\n";
	message += "X Axis Type: " + xAxis + "\r\n";
	message += "File Name: " + fileName + "\r\n";
	message += getPrcSettingsString();
	message += getPscSettingsString();
	return message;
}


std::string PlottingInfo::getAllSettingsStringFromFile(std::string fileAddress)
{
	PlottingInfo temp(fileAddress);
	return temp.getAllSettingsString();
}

void PlottingInfo::savePlotInfo()
{
	std::string completeAddress = PLOT_FILES_SAVE_LOCATION + "\\" + fileName + ".plot";
	std::fstream saveFile(completeAddress, std::fstream::out);
	if (!saveFile.is_open())
	{
		thrower ("Couldn't open file at + " + PLOT_FILES_SAVE_LOCATION + "\\" + fileName + ".plot!");
	}
	std::string message;
	message += "Version: " + str( versionMajor ) + "." + str( versionMinor ) + "\n";
	message += title + "\n";
	message += generalPlotType + "\n";
	message += yLabel + "\n";
	message += xAxis + "\n";
	message += fileName + "\n";

	message += str(dataSets.size()) + "\n";
	message += str(currentConditionNumber) + "\n";
	message += str(currentPixelNumber) + "\n";
	message += str(numberOfPictures) + "\n";

	message += "POSITIVE_RESULT_BEGIN\n";
	for (auto& dset : dataSets)
	{
		message += "DATA_SET_BEGIN\n";
		for (UINT pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
		{
			message += "PICTURE_BEGIN\n";
			for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
			{
				message += str(dset.getPositiveResultCondition(pixelInc, pictureInc)) + "\n";
			}
			message += "PICTURE_END\n";
		}
		message += "DATA_SET_END\n";
	}
	message += "POSITIVE_RESULT_END\n";
	message += "POST_SELECTION_BEGIN\n";
	for (UINT conditionInc = 0; conditionInc < currentConditionNumber; conditionInc++)
	{
		message += "CONDITION_BEGIN\n";
		for ( auto& dset : dataSets )
		{
			message += "DATA_SET_BEGIN\n";
			for (UINT pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
			{
				message += "PICTURE_BEGIN\n";
				for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
				{
					message += str(dset.getPostSelectionCondition(conditionInc, pixelInc, pictureInc)) + "\n";
				}
				message += "PICTURE_END\n";
			}
			message += "DATA_SET_END\n";
		}
		message += "CONDITION_END\n";
	}
	message += "POST_SELECTION_END\n";

	// legends
	message += "LEGENDS_BEGIN\n";
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += getLegendText(dataSetInc) + "\n";
	}
	message += "LEGENDS_END\n";

	// data count locations
	message += "PLOT_COUNTS_LOCATIONS_BEGIN\n";
	if ( getPlotType( ) != "Atoms" )
	{
		for ( UINT dataSetInc = 0; dataSetInc < dataSets.size( ); dataSetInc++ )
		{
			UINT pixel, picture;
			getDataCountsLocation( dataSetInc, pixel, picture );
			message += str( pixel ) + " " + str( picture ) + "\n";
		}
	}
	message += "PLOT_COUNTS_LOCATIONS_END\n";
	
	message += "FITTING_OPTIONS_BEGIN\n";
	for ( auto& dset : dataSets )
	{
		message += str(dset.getFitType()) + " " + str(dset.getWhenToFit()) + "\n";
	}
	message += "FITTING_OPTIONS_END\n";
	
	message += "HIST_OPTIONS_BEGIN\n";
	message += str( dataSets.size()) + "\n";
	for ( auto& dset : dataSets )
	{
		message += str( dset.getHistBinWidth() )+ "\n";
	}
	message += "HIST_OPTIONS_END\n";

	saveFile << message;
}


void PlottingInfo::loadPlottingInfoFromFile(std::string fileLocation)
{
	std::ifstream loadFile(fileLocation);
	if (!loadFile.is_open())
	{
		thrower ("ERROR: Couldn't open plot file!");
	}
	ScriptStream plotStream (loadFile);
	// this string will hold headers in this file temporarily and check to make sure they are correct.
	std::string versionStr;
	plotStream >> versionStr >> versionStr;
	double version;
	try
	{
		version = boost::lexical_cast<double>( versionStr );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: Version string failed to convert to double while opening configuration!" );
	}
	int versionMajor = int( version );
	int versionMinor;
	double tempDouble = std::round( (version - versionMajor) * 1e6 );
	while ( true )
	{
		double res = fabs( double( double( tempDouble / 10 ) - int( tempDouble / 10 ) ) );
		if ( res < 1e-6 )
		{
			break;
		}
		tempDouble /= 10;
	}
	versionMinor = int(std::round( tempDouble ));
	plotStream.get( );
	getline(plotStream, title);
	getline(plotStream, generalPlotType);
	getline(plotStream, yLabel);
	getline(plotStream, xAxis);
	getline(plotStream, fileName);

	// Data Set Number
	std::string testString;
	getline(plotStream, testString);
	try
	{
		int tempDataSetNumber = boost::lexical_cast<int>(testString);
		resetDataSetNumber(tempDataSetNumber);
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ("ERROR: Couldn't read data set number from file. The data set string was " + testString);
	}
	// Condition Number
	getline(plotStream, testString);
	try
	{
		currentConditionNumber = boost::lexical_cast<int>(testString);
		resetConditionNumber(currentConditionNumber);
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ("ERROR: Couldn't read post-selection number from file. The post-selection string was " + testString);
	}
	// Pixel Number
	getline(plotStream, testString);
	// Picture Number
	std::string testString2;
	getline(plotStream, testString2);
	try
	{
		numberOfPictures = boost::lexical_cast<int>(testString2);
		resetPictureNumber(numberOfPictures);
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ("ERROR: Couldn't read Picture number from file. The picture string was " + testString);
	}
	try
	{
		currentPixelNumber = boost::lexical_cast<int>(testString);
		resetPixelNumber(currentPixelNumber);
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ("ERROR: Couldn't read pixel number from file. The pixel string was " + testString);
	}
	/// Analys pixels
	ProfileSystem::checkDelimiterLine(plotStream, "POSITIVE_RESULT_BEGIN" );
	UINT dataSetCount = 0;
	// data set loop
	while (!ProfileSystem::checkDelimiterLine(plotStream, "DATA_SET_BEGIN", "POSITIVE_RESULT_END" ) )
	{
		// picture loop
		int pictureCount = 0;
		while ( !ProfileSystem::checkDelimiterLine(plotStream, "PICTURE_BEGIN", "DATA_SET_END" ) )
		{			
			// pixel loop
			int pixelCount = 0;
			while (true)
			{
				plotStream >> testString;
				if (testString == "PICTURE_END")
				{
					break;
				}
				int tempTruthCondition;
				try
				{
					tempTruthCondition = boost::lexical_cast<int>(testString);
				}
				catch ( boost::bad_lexical_cast&)
				{
					throwNested ("ERROR: truth condition failed to evaluate to an integer. The truth condition string was"
							+ testString);
				}
				if (tempTruthCondition != -1 && tempTruthCondition != 0 && tempTruthCondition != 1)
				{
					thrower ("ERROR: truth condition was not one of the valid options: -1, 0, or 1. The truth condition was" 
									+ str(tempTruthCondition));
				}
				setResultCondition(dataSetCount, pixelCount, pictureCount, tempTruthCondition);
				//(int dataSetNumber, int pixel, int picture, int trueConditionValue)
				pixelCount++;
			}
			if (pixelCount != currentPixelNumber)
			{
				thrower ("ERROR: number of pixels the truth condition was set for doesn't match the number of pixels"
						" reported earlier in the file.");
			}
			pictureCount++;
		}
		if (pictureCount != numberOfPictures)
		{
			thrower ("ERROR: number of pictures the truth condition was set for doesn't match the number of"
					   " pictures reported earlier in the file.");
		}
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size())
	{
		thrower ( "ERROR: number of data sets the truth condition was set for doesn't match the number of data sets "
			     "reported earlier in the file.");
	}

	ProfileSystem::checkDelimiterLine(plotStream, "POST_SELECTION_BEGIN" );
	// condition loop
	int conditionCount = 0;
	while (!ProfileSystem::checkDelimiterLine(plotStream, "CONDITION_BEGIN", "POST_SELECTION_END" ) )
	{
		// data set loop
		int dataSetCount = 0;
		while ( !ProfileSystem::checkDelimiterLine(plotStream, "DATA_SET_BEGIN", "CONDITION_END" ) )
		{
			// picture loop
			int pictureCount = 0;
			while ( !ProfileSystem::checkDelimiterLine(plotStream, "PICTURE_BEGIN", "DATA_SET_END" ) )
			{
				// pixel loop
				int pixelCount = 0;
				while (true)
				{
					plotStream >> testString;
					if (testString == "PICTURE_END")
					{
						break;
					}
					int tempPostSelectionCondition;

					try
					{
						tempPostSelectionCondition = boost::lexical_cast<int>(testString);
					}
					catch ( boost::bad_lexical_cast&)
					{
						throwNested ("ERROR: truth condition failed to evaluate to an integer. The truth condition string was" + testString);
					}
					if (tempPostSelectionCondition != -1 && tempPostSelectionCondition != 0 && tempPostSelectionCondition != 1)
					{
						thrower ("ERROR: truth condition was not one of the valid options: -1, 0, or 1. The truth condition was"
								+ str(tempPostSelectionCondition));
					}
					setPostSelCondition(dataSetCount, conditionCount, pixelCount, pictureCount, tempPostSelectionCondition);
					pixelCount++;
				}
				if (pixelCount != currentPixelNumber)
				{
					thrower ( "ERROR: number of pixels the truth condition was set for doesn't match the number of pixels"
						     " reported earlier in the file.");
				}
				pictureCount++;
			}
			if (pictureCount != numberOfPictures)
			{
				thrower ( "ERROR: number of pictures the truth condition was set for doesn't match the number of pictures"
					     " reported earlier in the file.");
			}
			dataSetCount++;
		}
		if (dataSetCount != dataSets.size())
		{
			thrower ( "ERROR: number of data sets the truth condition was set for doesn't match the number of data sets "
				     "reported earlier in the file.");
		}
		conditionCount++;
	}
	if (conditionCount != currentConditionNumber)
	{
		thrower ( "ERROR: number of post selection conditions that were set doesn't match the number of conditions "
			     "reported earlier in the file.");
	}
	
	// get legends
	ProfileSystem::checkDelimiterLine(plotStream, "LEGENDS_BEGIN" );
	plotStream.get( );
	dataSetCount = 0;
	while (true)
	{
		getline(plotStream, testString);
		if (testString == "LEGENDS_END")
		{
			break;
		}
		changeLegendText(dataSetCount, testString);
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size())
	{
		thrower ("ERROR: the number of data sets that legends were read for doesn't match the number of data sets "
			   "reported earlier in the file.");
	}
	// get counts locations
	ProfileSystem::checkDelimiterLine(plotStream, "PLOT_COUNTS_LOCATIONS_BEGIN" );
	dataSetCount = 0;
	while (true)
	{
		std::string pixelStr, pictureStr;
		int pixel, picture;
		plotStream >> pixelStr;
		if ( pixelStr == "PLOT_COUNTS_LOCATIONS_END" )
		{
			break;
		}
		plotStream >> pictureStr;
		try
		{
			pixel = boost::lexical_cast<int>(pixelStr);
		}
		catch ( boost::bad_lexical_cast&)
		{
			throwNested ("ERROR: pixel listed in file did not convert to integer correctly. pixel string was" + pixelStr);
		}
		try
		{
			picture = boost::lexical_cast<int>(pictureStr);
		}
		catch ( boost::bad_lexical_cast&)
		{
			throwNested ("ERROR: picture listed in file did not convert to integer correctly. picture string was" + pictureStr);
		}
		setDataCountsLocation(dataSetCount, pixel, picture);
		dataSetCount++;
	}

	if (generalPlotType == "Pixel Count Histograms" || generalPlotType == "Pixel Counts")
	{
		if (dataSetCount != dataSets.size())
		{
			thrower ("ERROR: the number of data sets that counts locations were read for doesn't match the number of "
				   "data sets reported earlier in the file.");
		}
	}
	else
	{
		if (dataSetCount > 0)
		{
			thrower ("ERROR: There were counts plotting locations listed in the file despite the plot type not being"
					" pixel count histograms or pixel counts.");
		}
	}
	ProfileSystem::checkDelimiterLine(plotStream, "FITTING_OPTIONS_BEGIN" );
	plotStream.get( );
	dataSetCount = 0;
	while (true)
	{
		getline(plotStream, testString);
		if (testString == "FITTING_OPTIONS_END")
		{
			break;
		}
		std::string fitOptionStr, whenToFitStr;
		int fitOption, whenToFit;
		std::stringstream tempStream;
		tempStream << testString;
		tempStream >> fitOptionStr;
		try
		{
			fitOption = boost::lexical_cast<int>(fitOptionStr);
		}
		catch ( boost::bad_lexical_cast&)
		{
			throwNested ("ERROR: fit option listed in file did not convert to integer correctly. fit option string was" + fitOptionStr);
		}
		tempStream >> whenToFitStr;
		try
		{
			whenToFit = boost::lexical_cast<int>(whenToFitStr);
		}
		catch ( boost::bad_lexical_cast&)
		{
			throwNested ("ERROR: When to Fit option listed in file did not convert to integer correctly. when to fit string "
				   "was" + whenToFitStr);
		}
		dataSets[dataSetCount].setFitType(fitOption);
		dataSets[dataSetCount].setWhenToFit(whenToFit);
		dataSetCount++;
	}
	if (dataSetCount > dataSets.size())
	{
		thrower ("ERROR: more fit options than data sets!");
	}

	ProfileSystem::checkDelimiterLine(plotStream, "HIST_OPTIONS_BEGIN" );
	plotStream.get( );
	std::string tmpStr;
	plotStream >> tmpStr;
	UINT dsetNum = 0;
	try
	{
		dsetNum = boost::lexical_cast<unsigned long>( tmpStr );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "ERROR: expected number of data sets in plot file while loading hist options, instead found: " + tmpStr );
	}
	if ( dsetNum != dataSets.size( ) )
	{
		thrower ( "ERROR: data set number in file while opening hist settings doesn't match number from earlier in file." );
	}
	for ( auto& dset : dataSets )
	{
		plotStream >> tmpStr;
		UINT width;
		try
		{
			width = boost::lexical_cast<unsigned long>( tmpStr );
		}
		catch ( boost::bad_lexical_cast& )
		{
			throwNested ( "ERROR: failed to convert histogram bin width to an unsigned int! string was: " + tmpStr );
		}
		dset.setHistBinWidth( width );
	}
	ProfileSystem::checkDelimiterLine(plotStream, "HIST_OPTIONS_END" );
}


void PlottingInfo::resetNumberOfAnalysisGroups( UINT setNumber)
{
	for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		analysisGroups[pixelInc].clear();
		analysisGroups[pixelInc].resize(setNumber);
	}
}


void PlottingInfo::resetPixelNumber( UINT pixelNumber)
{
	currentPixelNumber = pixelNumber;
	analysisGroups.resize(pixelNumber);
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPixelNumber(pixelNumber);
		dataSets[dataSetInc].resetPictureNumber(numberOfPictures);
	}
}

void PlottingInfo::resetPictureNumber( UINT pictureNumber)
{
	numberOfPictures = pictureNumber;
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPictureNumber(pictureNumber);
	}
}

void PlottingInfo::resetConditionNumber( UINT conditionNumber)
{
	currentConditionNumber = conditionNumber;
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPostSelectionConditionNumber(conditionNumber);
	}
}


void PlottingInfo::resetDataSetNumber( UINT dataSetNumber)
{
	dataSets.clear();
	dataSets.resize(dataSetNumber);
}


void PlottingInfo::clear()
{
	title = "";
	yLabel = "";
	analysisGroups.clear();
	dataSets.clear();
	fileName = "";
	numberOfPictures = 0;
	currentPixelNumber = 0;
	currentConditionNumber = 0;
	// these are invalid values... should probably default to some default valid values?
	xAxis = "";
	generalPlotType = "";
}

std::string PlottingInfo::getTitle()
{
	return title;
}

std::string PlottingInfo::getYLabel()
{
	return yLabel;
}

std::string PlottingInfo::getFileName()
{
	return fileName;
}

UINT PlottingInfo::getPixelNumber()
{
	return currentPixelNumber;
}

UINT PlottingInfo::getPicNumber()
{
	return numberOfPictures;
}

UINT PlottingInfo::getConditionNumber()
{
	return currentConditionNumber;
}


UINT PlottingInfo::getPixelGroupNumber()
{
	if (analysisGroups.size() == 0)
	{
		thrower ( "ERROR: tried to get group number when no data sets!" );
	}
	return analysisGroups[0].size();
}


UINT PlottingInfo::getDataSetNumber()
{
	return dataSets.size();
}


void PlottingInfo::setPixelIndex( UINT pixel, UINT group, UINT index)
{
	groupInfo( pixel, group ).pixelIndex = index;
}


UINT PlottingInfo::getPixelIndex( UINT pixel, UINT group)
{
	return groupInfo( pixel, group ).pixelIndex;
}


void PlottingInfo::setFitOption( UINT dataSet, UINT fitType)
{
	dataSets[dataSet].setFitType(fitType);
}


void PlottingInfo::setWhenToFit( UINT dataSet, UINT whenToFit)
{
	dataSets[dataSet].setWhenToFit(whenToFit);
}


UINT PlottingInfo::getFitOption( UINT dataSet)
{
	return dataSets[dataSet].getFitType();
}


UINT PlottingInfo::whenToFit( UINT dataSet)
{
	return dataSets[dataSet].getWhenToFit();
}


std::vector<std::pair<UINT, UINT>> PlottingInfo::getAllPixelLocations()
{
	std::vector<std::pair<UINT, UINT>> allUniqueLocations;
	for (UINT pixelInc = 0; pixelInc < analysisGroups.size(); pixelInc++)
	{
		for (UINT groupInc = 0; groupInc < analysisGroups[pixelInc].size(); groupInc++)
		{
			std::pair<UINT, UINT> tempLocation;
			tempLocation.first = groupInfo( pixelInc, groupInc ).row;
			tempLocation.second = groupInfo( pixelInc, groupInc ).col;
			bool alreadyExists = false;
			for (UINT uniqueLocationInc = 0; uniqueLocationInc < allUniqueLocations.size(); uniqueLocationInc++)
			{
				if (allUniqueLocations[uniqueLocationInc].first == tempLocation.first 
					 && allUniqueLocations[uniqueLocationInc].second == tempLocation.second)
				{
					alreadyExists = true;
				}
			}
			if (!alreadyExists)
			{
				allUniqueLocations.push_back(tempLocation);
			}
		}
	}
	return allUniqueLocations;
}


void PlottingInfo::setGroups(std::vector<coordinate> locations)
{
	if (locations.size() % currentPixelNumber != 0)
	{
		thrower ( "ERROR: One of your real-time plots was expecting a multiple of " + str(currentPixelNumber) + " pixels to analyze, but you "
			     "selected " + str(locations.size()) + " pixels.");
	}
	UINT locationInc = 0;
	for (UINT groupInc = 0; groupInc < locations.size() / currentPixelNumber; groupInc++)
	{
		addGroup();
		for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
		{
			setGroupLocation(pixelInc, groupInc, locations[locationInc].column+1, locations[locationInc].row+1);
			locationInc++;
		}
	}
}

