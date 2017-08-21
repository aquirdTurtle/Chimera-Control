#include "stdafx.h"
#include "PlottingInfo.h"
#include <sstream>

// All PlottingInfo() class public functions.

PlottingInfo::PlottingInfo(UINT picNumber)
{
	// initialize things.
	generalPlotType = "Atoms";
	title = "";
	yLabel = "";
	fileName = "";
	dataSets.clear();
	// one line to plot.
	dataSets.resize(1);
	// todo: initialize with real picture number.
	dataSets[0].resetPictureNumber(picNumber);
	dataSets[0].resetPixelNumber(1);
	//
	//analysisGroups.clear();
	// one pixel
	//analysisGroups.resize(1);
	// one analysis set.
	//analysisGroups[0].resize(1);
	//
	currentPixelNumber = 1;
	currentConditionNumber = 1;
	numberOfPictures = picNumber;
	xAxis = "";
}

PlottingInfo::PlottingInfo(std::string fileName)
{
	loadPlottingInfoFromFile(fileName);
}

PlottingInfo::~PlottingInfo()
{
	// no fancy deconstrutor needed
}

void PlottingInfo::changeTitle(std::string newTitle)
{
	title = newTitle;
}

std::string PlottingInfo::getPrcSettingsString()
{
	std::string allConditionsString = "All Result Conditions:\r\n=========================\r\n\r\n";
	for (int dataSetInc = 0; dataSetInc < getDataSetNumber(); dataSetInc++)
	{
		allConditionsString += "Data Set #" + str(dataSetInc + 1) + ":\r\n=====\r\n";
		for (int pictureInc = 0; pictureInc < getPicNumber(); pictureInc++)
		{
			allConditionsString += "Picture #" + str(pictureInc + 1) + ":\r\n";
			for (int pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++)
			{
				allConditionsString += "Pixel #" + str(pixelInc + 1) + ":";
				int currentValue = getResultCondition(dataSetInc, pixelInc, pictureInc);
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
	std::string allConditionsString = "All Current Post-Selection Conditions:\r\n=========================\r\n\r\n";
	for (int conditionInc = 0; conditionInc < getConditionNumber(); conditionInc++)
	{
		allConditionsString += "*****Condition #" + str(conditionInc + 1) + "*****\r\n";
		for (int dataSetInc = 0; dataSetInc < getDataSetNumber(); dataSetInc++)
		{
			allConditionsString += "Data Set #" + str(dataSetInc + 1) + ":\r\n=====\r\n";
			for (int pictureInc = 0; pictureInc < getPicNumber(); pictureInc++)
			{
				allConditionsString += "Picture #" + str(pictureInc + 1) + ":\r\n";
				for (int pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++)
				{
					allConditionsString += "Pixel #" + str(pixelInc + 1) + ":";
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
	for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() + 1);
	}
}


void PlottingInfo::setGroupLocation(int pixel, int analysisSet, int row, int collumn)
{
	if (pixel >= currentPixelNumber)
	{
		thrower("ERROR: something tried to set the position of a pixel that hasn't been allocated yet. Pixel #: " 
				+ str(pixel));
	}
	if (analysisSet >= analysisGroups[pixel].size())
	{
		thrower("ERROR: something tried to set the position of an analysis set that hasn't been allocated yet. Analysis Set #: " 
				    + str(pixel));
	}
	analysisGroups[pixel][analysisSet][0] = row;
	analysisGroups[pixel][analysisSet][1] = collumn;
}

void PlottingInfo::addPixel()
{
	// change all the data set structures.
	currentPixelNumber++;
	analysisGroups.resize(analysisGroups.size() + 1);
	analysisGroups[analysisGroups.size() - 1].resize(analysisGroups[0].size());
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].addPixel(numberOfPictures);
	}
}

void PlottingInfo::removePixel()
{
	// make sure there is a pixel to remove.
	if (currentPixelNumber < 2)
	{
		thrower("ERROR: Something tried to remove the last pixel!");
	}
	// change all the data set structures.
	currentPixelNumber--;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].removePixel();
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
		thrower("ERROR: Something tried to remove the last data set");
	}
	
	dataSets.resize(dataSets.size() - 1);
}

// change all structures that depend on the number of pictures. 
void PlottingInfo::addPicture()
{
	numberOfPictures++;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].addPicture();
	}
}


void PlottingInfo::removePicture()
{
	if (numberOfPictures < 2)
	{
		thrower("ERROR: Something tried to remove the last picture!");
	}
	numberOfPictures--;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].removePicture();
	}
}


void PlottingInfo::setPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture,
											 int postSelectionCondition)
{
	dataSets[dataSetNumber].setPostSelectionCondition(conditionNumber, pixel, picture, postSelectionCondition);
}

void PlottingInfo::setResultCondition(int dataSetNumber, int pixel, int picture, int trueConditionValue)
{
	dataSets[dataSetNumber].setResultCondition(pixel, picture, trueConditionValue);
}

// stores the info in the row and column arguments
void PlottingInfo::getPixelLocation(int pixel, int analysisSet, int& row, int& column)
{
	row = analysisGroups[pixel][analysisSet][0];
	column = analysisGroups[pixel][analysisSet][1];
}

void PlottingInfo::removeAnalysisSet()
{
	// always at least one pixel...

	// make sure tat there is an analysis set to remove.
	for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		if (this->analysisGroups[pixelInc].size() < 2)
		{
			thrower("ERROR: Something tried to remove the last analysis group!");
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


int PlottingInfo::getResultCondition(int dataSetNumber, int pixel, int picture)
{
	return dataSets[dataSetNumber].getTruthCondition(pixel, picture);
}

int PlottingInfo::getPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture)
{
	return dataSets[dataSetNumber].getPostSelectionCondition(conditionNumber, pixel, picture);
}

void PlottingInfo::addPostSelectionCondition()
{
	
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		// the currentPixelNumber is used as a vector index, the number of pictures is used as a resize size.
		dataSets[dataSetInc].addPostSelectionCondition(currentPixelNumber, numberOfPictures);
	}
	currentConditionNumber++;
}


void PlottingInfo::removePostSelectionCondition()
{
	
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].removePostSelectionCondition();
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

void PlottingInfo::setDataCountsLocation(int dataSet, int pixel, int picture)
{
	if (dataSet < dataSets.size() && !(dataSet < 0))
	{
		dataSets[dataSet].setDataCountsLocation(currentPixelNumber, numberOfPictures, pixel, picture);
	}
	else
	{
		thrower("ERROR: tried to set data counts location for data set that hasn't been assigned yet. dataSet = " 
				+ str(dataSet));
	}
}


void PlottingInfo::setPlotData(int dataSet, bool plotData)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower( "ERROR: tried to set \"Plot this data\" value for data set that hasn't been assigned. dataSet = "
			     + str(dataSet));
	}
	dataSets[dataSet].setPlotThisData(plotData);
}


bool PlottingInfo::getPlotThisDataValue(int dataSet)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower("ERROR: tried to get \"Plot this data\" value for data set that hasn't been assigned. dataSet = "
				+ str(dataSet));
	}
	return dataSets[dataSet].getPlotThisDataValue();
}


void PlottingInfo::getDataCountsLocation(int dataSet, int& pixel, int& picture)
{
	dataSets[dataSet].getDataCountsLocation(pixel, picture);
}


void PlottingInfo::changeLegendText(int dataSet, std::string newLegend)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower( "ERROR: attempted to set dataset legend for data set that hadn't been allocated. dataset = " 
			     + str(dataSet));
	}
	dataSets[dataSet].changeLegendText(newLegend);
}


std::string PlottingInfo::getLegendText(int dataSet)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		thrower( "ERROR: attempted to get dataset legend for data set that hadn't been allocated. dataset = " 
			     + str(dataSet));
	}
	return dataSets[dataSet].getLegendText();
}

std::string PlottingInfo::getAllSettingsString()
{
	std::string message = "All Plotting Parameters (" + fileName + ")\r\n=======================================\r\n";
	message += "Plot Title: " + title + "\r\n";
	message += "Plot Type: " + generalPlotType + "\r\n";
	message += "y Label: " + yLabel + "\r\n";
	message += "x axis: " + xAxis + "\r\n";
	message += "file name: " + fileName + "\r\n";
	message += "\r\nAll Current Truth Conditions:\r\n=========================\r\n\r\n";
	message += getPrcSettingsString();
	message += "All Current Post-Selection Conditions:\r\n=========================\r\n\r\n";
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
		thrower("Couldn't open file at + " + PLOT_FILES_SAVE_LOCATION + fileName + ".plot!");
	}
	std::string message;
	message += title + "\n";
	message += generalPlotType + "\n";
	message += yLabel + "\n";
	message += xAxis + "\n";
	message += fileName + "\n";

	message += str(dataSets.size()) + "\n";
	message += str(currentConditionNumber) + "\n";
	message += str(currentPixelNumber) + "\n";
	message += str(numberOfPictures) + "\n";

	message += "TRUTH BEGIN\n";
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += "DATA SET BEGIN\n";
		for (int pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
		{
			message += "PICTURE BEGIN\n";
			for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
			{
				message += str(dataSets[dataSetInc].getTruthCondition(pixelInc, pictureInc)) + "\n";
			}
			message += "PICTURE END\n";
		}
		message += "DATA SET END\n";
	}
	message += "TRUTH END\n";
	message += "POST SELECTION BEGIN\n";
	for (UINT conditionInc = 0; conditionInc < currentConditionNumber; conditionInc++)
	{
		message += "CONDITION BEGIN\n";
		for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
		{
			message += "DATA SET BEGIN\n";
			for (UINT pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
			{
				message += "PICTURE BEGIN\n";
				for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
				{
					message += str(dataSets[dataSetInc].getPostSelectionCondition(conditionInc, pixelInc, pictureInc)) + "\n";
				}
				message += "PICTURE END\n";
			}
			message += "DATA SET END\n";
		}
		message += "CONDITION END\n";
	}
	message += "POST SELECTION END\n";

	// legends
	message += "LEGENDS BEGIN\n";
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += getLegendText(dataSetInc) + "\n";
	}
	message += "LEGENDS END\n";

	// data count locations
	message += "PLOT COUNTS LOCATIONS BEGIN\n";
	bool failedOnce = false;
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		int pixel, picture;
		getDataCountsLocation(dataSetInc, pixel, picture);
	}
	message += "PLOT COUNTS LOCATIONS END\n";
	
	message += "FITTING OPTIONS BEGIN\n";
	for (UINT dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += str(dataSets[dataSetInc].getFitType()) + " " + str(dataSets[dataSetInc].getWhenToFit()) + "\n";
	}
	message += "FITTING OPTIONS END\n";
	saveFile << message;
}


void PlottingInfo::loadPlottingInfoFromFile(std::string fileLocation)
{
	std::fstream loadingFile(fileLocation);
	if (!loadingFile.is_open())
	{
		thrower("ERROR: Couldn't open plot file!");
	}
	// this string will hold headers in this file temporarily and check to make sure they are correct.
	std::string testString;
	//The way it's put into the file:
	getline(loadingFile, title);
	getline(loadingFile, generalPlotType);
	getline(loadingFile, yLabel);
	getline(loadingFile, xAxis);
	getline(loadingFile, fileName);
	// 
	// check filename???
	// 

	/// Data Set Number
	getline(loadingFile, testString);
	try
	{
		int tempDataSetNumber = std::stoi(testString);
		resetDataSetNumber(tempDataSetNumber);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Couldn't read data set number from file. The data set string was " + testString);
	}
	/// Condition Number
	getline(loadingFile, testString);
	try
	{
		currentConditionNumber = std::stoi(testString);
		resetConditionNumber(currentConditionNumber);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Couldn't read post-selection number from file. The post-selection string was " + testString);
	}
	/// Pixel Number
	getline(loadingFile, testString);
	/// Picture Number
	std::string testString2;
	getline(loadingFile, testString2);
	try
	{
		numberOfPictures = std::stoi(testString2);
		resetPictureNumber(numberOfPictures);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Couldn't read Picture number from file. The picture string was " + testString);
	}
	try
	{
		currentPixelNumber = std::stoi(testString);
		resetPixelNumber(currentPixelNumber);
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Couldn't read pixel number from file. The pixel string was " + testString);
	}

	/// Analys pixels
	getline(loadingFile, testString);
	if (testString != "TRUTH BEGIN")
	{
		thrower("File Corrupted or malformatted! Expected \"TRUTH BEGIN\", but saw instead\"" + testString + "\"");
	}
	int dataSetCount = 0;
	// data set loop
	while (true)
	{
		getline(loadingFile, testString);
		if (testString != "DATA SET BEGIN")
		{
			if (testString == "TRUTH END")
			{
				break;
			}
			thrower("File Corrupted or malformatted! Expected \"DATA SET BEGIN\", but saw instead\"" + testString + "\"");
		}
		// picture loop
		int pictureCount = 0;
		while (true)
		{
			getline(loadingFile, testString);
			if (testString != "PICTURE BEGIN")
			{
				if (testString == "DATA SET END")
				{
					break;
				}
				else
				{
					thrower("File Corrupted or malformatted! Expected \"PICTURE BEGIN\", but saw instead\"" + testString + "\"");
				}
			}
			// pixel loop
			int pixelCount = 0;
			while (true)
			{
				getline(loadingFile, testString);
				if (testString == "PICTURE END")
				{
					break;
				}
				int tempTruthCondition;
				try
				{
					tempTruthCondition = std::stoi(testString);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: truth condition failed to evaluate to an integer. The truth condition string was" 
							+ testString);
				}
				if (tempTruthCondition != -1 && tempTruthCondition != 0 && tempTruthCondition != 1)
				{
					thrower("ERROR: truth condition was not one of the valid options: -1, 0, or 1. The truth condition was" 
									+ str(tempTruthCondition));
				}
				setResultCondition(dataSetCount, pixelCount, pictureCount, tempTruthCondition);
				//(int dataSetNumber, int pixel, int picture, int trueConditionValue)
				pixelCount++;
			}
			if (pixelCount != currentPixelNumber)
			{
				thrower(0, "ERROR: number of pixels the truth condition was set for doesn't match the number of pixels"
						" reported earlier in the file.", 0, 0);
			}
			pictureCount++;
		}
		if (pictureCount != numberOfPictures)
		{
			thrower("ERROR: number of pictures the truth condition was set for doesn't match the number of"
					   " pictures reported earlier in the file.");
		}
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size())
	{
		thrower( "ERROR: number of data sets the truth condition was set for doesn't match the number of data sets "
			     "reported earlier in the file.");
	}
	getline(loadingFile, testString);
	if (testString != "POST SELECTION BEGIN")
	{
		thrower("ERROR: Expected text \"POST SELECTION BEGIN\" but instead saw " + testString);
	}
	
	// condition loop
	int conditionCount = 0;
	while (true)
	{
		getline(loadingFile, testString);
		if (testString != "CONDITION BEGIN")
		{
			if (testString == "POST SELECTION END")
			{
				break;
			}
			thrower("File Corrupted or malformatted! Expected \"CONDITION BEGIN\", but saw instead\"" + testString 
				   + "\"");
		}
		// data set loop
		int dataSetCount = 0;
		while (true)
		{
			getline(loadingFile, testString);
			if (testString != "DATA SET BEGIN")
			{
				if (testString == "CONDITION END")
				{
					break;
				}
				thrower( "File Corrupted or malformatted! Expected \"DATA SET BEGIN\", but saw instead\"" + testString 
					     + "\"");
			}
			// picture loop
			int pictureCount = 0;
			while (true)
			{
				getline(loadingFile, testString);
				if (testString != "PICTURE BEGIN")
				{
					if (testString == "DATA SET END")
					{
						break;
					}
					else
					{
						thrower( "File Corrupted or malformatted! Expected \"PICTURE BEGIN\", but saw instead\"" 
							     + testString + "\"");
					}
				}
				// pixel loop
				int pixelCount = 0;
				while (true)
				{
					getline(loadingFile, testString);
					if (testString == "PICTURE END")
					{
						break;
					}
					int tempPostSelectionCondition;

					try
					{
						tempPostSelectionCondition = std::stoi(testString);
					}
					catch (std::invalid_argument&)
					{
						thrower("ERROR: truth condition failed to evaluate to an integer. The truth condition string was" + testString);
					}
					if (tempPostSelectionCondition != -1 && tempPostSelectionCondition != 0 && tempPostSelectionCondition != 1)
					{
						thrower("ERROR: truth condition was not one of the valid options: -1, 0, or 1. The truth condition was"
								+ str(tempPostSelectionCondition));
					}
					setPostSelectionCondition(dataSetCount, conditionCount, pixelCount, pictureCount, tempPostSelectionCondition);
					pixelCount++;
				}
				if (pixelCount != currentPixelNumber)
				{
					thrower( "ERROR: number of pixels the truth condition was set for doesn't match the number of pixels"
						     " reported earlier in the file.");
				}
				pictureCount++;
			}
			if (pictureCount != numberOfPictures)
			{
				thrower( "ERROR: number of pictures the truth condition was set for doesn't match the number of pictures"
					     " reported earlier in the file.");
			}
			dataSetCount++;
		}
		if (dataSetCount != dataSets.size())
		{
			thrower( "ERROR: number of data sets the truth condition was set for doesn't match the number of data sets "
				     "reported earlier in the file.");
		}
		conditionCount++;
	}
	if (conditionCount != currentConditionNumber)
	{
		thrower( "ERROR: number of post selection conditions that were set doesn't match the number of conditions "
			     "reported earlier in the file.");
	}
	
	// get legends
	getline(loadingFile, testString);
	if (testString != "LEGENDS BEGIN")
	{
		thrower("ERROR: Expected text \"LEGENDS BEGIN\" but got text" + testString);
	}
	dataSetCount = 0;
	while (true)
	{
		getline(loadingFile, testString);
		if (testString == "LEGENDS END")
		{
			break;
		}
		changeLegendText(dataSetCount, testString);
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size())
	{
		thrower("ERROR: the number of data sets that legends were read for doesn't match the number of data sets "
			   "reported earlier in the file.");
	}
	// get counts locations
	getline(loadingFile, testString);
	if (testString != "PLOT COUNTS LOCATIONS BEGIN")
	{
		thrower("ERROR: Expected text \"PLOT COUNTS LOCATIONS BEGIN\" but got text" + testString);
	}
	dataSetCount = 0;
	while (true)
	{
		getline(loadingFile, testString);
		if (testString == "PLOT COUNTS LOCATIONS END")
		{
			break;
		}
		std::string pixelStr, pictureStr;
		int pixel, picture;
		std::stringstream tempStream;
		tempStream << testString;
		tempStream >> pixelStr;
		try
		{
			pixel = std::stoi(pixelStr);
		}
		catch (std::invalid_argument&)
		{
			thrower("ERROR: pixel listed in file did not convert to integer correctly. pixel string was" + pixelStr);
		}

		tempStream >> pictureStr;
		try
		{
			picture = std::stoi(pictureStr);
		}
		catch (std::invalid_argument&)
		{
			thrower("ERROR: picture listed in file did not convert to integer correctly. picture string was" + pictureStr);
		}
		this->setDataCountsLocation(dataSetCount, pixel, picture);
		dataSetCount++;
	}

	if (generalPlotType == "Pixel Count Histograms" || generalPlotType == "Pixel Counts")
	{
		if (dataSetCount != dataSets.size())
		{
			thrower("ERROR: the number of data sets that counts locations were read for doesn't match the number of "
				   "data sets reported earlier in the file.");
		}
	}
	else
	{
		if (dataSetCount > 0)
		{
			thrower("ERROR: There were counts plotting locations listed in the file despite the plot type not being"
					" pixel count histograms or pixel counts.");
		}
	}

	getline(loadingFile, testString);
	if (testString != "FITTING OPTIONS BEGIN")
	{
		thrower("ERROR: Expected text \"FITTING OPTIONS BEGIN\" but got text" + testString);
	}
	dataSetCount = 0;
	while (true)
	{
		getline(loadingFile, testString);
		if (testString == "FITTING OPTIONS END")
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
			fitOption = std::stoi(fitOptionStr);
		}
		catch (std::invalid_argument&)
		{
			thrower("ERROR: fit option listed in file did not convert to integer correctly. fit option string was" + fitOptionStr);
		}

		tempStream >> whenToFitStr;
		try
		{
			whenToFit = std::stoi(whenToFitStr);
		}
		catch (std::invalid_argument&)
		{
			thrower("ERROR: When to Fit option listed in file did not convert to integer correctly. when to fit string "
				   "was" + whenToFitStr);
		}
		dataSets[dataSetCount].setFitType(fitOption);
		dataSets[dataSetCount].setWhenToFit(whenToFit);
		dataSetCount++;
	}
	if (dataSetCount > dataSets.size())
	{
		thrower("ERROR: more fit options than data sets!");
	}
}

void PlottingInfo::resetNumberOfAnalysisGroups(int setNumber)
{
	for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		analysisGroups[pixelInc].clear();
		analysisGroups[pixelInc].resize(setNumber);
	}
}

void PlottingInfo::resetPixelNumber(int pixelNumber)
{
	currentPixelNumber = pixelNumber;
	analysisGroups.resize(pixelNumber);
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPixelNumber(pixelNumber);
		dataSets[dataSetInc].resetPictureNumber(numberOfPictures);
	}
}

void PlottingInfo::resetPictureNumber(int pictureNumber)
{
	numberOfPictures = pictureNumber;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPictureNumber(pictureNumber);
	}
}

void PlottingInfo::resetConditionNumber(int conditionNumber)
{
	currentConditionNumber = conditionNumber;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPostSelectionConditionNumber(conditionNumber);
	}
}


void PlottingInfo::resetDataSetNumber(int dataSetNumber)
{
	dataSets.clear();
	dataSets.resize(dataSetNumber);
}


void PlottingInfo::clear()
{
	// arbitrary
	title = "";
	// arbitrary
	yLabel = "";
	// analysisGroups[pixel #][pixel set].first = row
	// analysisGroups[pixel #][pixel set].second = column
	analysisGroups.clear();
	// Contains information for each set of data to be plotted.
	dataSets.clear();
	// arbitrary. Always goes to the same folder.
	fileName = "";
	// grabbed from main code at "OK" Press.
	numberOfPictures = 0;
	currentPixelNumber = 0;
	currentConditionNumber = 0;
	// two options here.
	xAxis = "";
	// three options here.
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

int PlottingInfo::getPixelNumber()
{
	return currentPixelNumber;
}

int PlottingInfo::getPicNumber()
{
	return numberOfPictures;
}

int PlottingInfo::getConditionNumber()
{
	return currentConditionNumber;
}


size_t PlottingInfo::getPixelGroupNumber()
{
	if (analysisGroups.size() == 0)
	{
		// perhaps I should throw here?
		return -1;
	}
	return analysisGroups[0].size();
}


size_t PlottingInfo::getDataSetNumber()
{
	return dataSets.size();
}


void PlottingInfo::setPixelIndex(int pixel, int group, int index)
{
	analysisGroups[pixel][group][2] = index;
}


int PlottingInfo::getPixelIndex(int pixel, int group)
{
	return analysisGroups[pixel][group][2];
}


void PlottingInfo::setFitOption(int dataSet, int fitType)
{
	dataSets[dataSet].setFitType(fitType);
}


void PlottingInfo::setWhenToFit(int dataSet, int whenToFit)
{
	dataSets[dataSet].setWhenToFit(whenToFit);
}


int PlottingInfo::getFitOption(int dataSet)
{
	return dataSets[dataSet].getFitType();
}


int PlottingInfo::whenToFit(int dataSet)
{
	return dataSets[dataSet].getWhenToFit();
}


std::vector<std::pair<int, int>> PlottingInfo::getAllPixelLocations()
{
	std::vector<std::pair<int, int>> allUniqueLocations;
	for (UINT pixelInc = 0; pixelInc < analysisGroups.size(); pixelInc++)
	{
		for (UINT groupInc = 0; groupInc < analysisGroups[pixelInc].size(); groupInc++)
		{
			std::pair<int, int> tempLocation;
			tempLocation.first = analysisGroups[pixelInc][groupInc][0];
			tempLocation.second = analysisGroups[pixelInc][groupInc][1];
			bool alreadyExists = false;
			for (int uniqueLocationInc = 0; uniqueLocationInc < allUniqueLocations.size(); uniqueLocationInc++)
			{
				if (allUniqueLocations[uniqueLocationInc].first == tempLocation.first && allUniqueLocations[uniqueLocationInc].second == tempLocation.second)
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


void PlottingInfo::setGroups(std::vector<std::pair<int, int>> locations)
{
	if (locations.size() % currentPixelNumber != 0)
	{
		thrower( "ERROR: One of your real-time plots was expecting a multiple of " + str(currentPixelNumber) + " pixels to analyze, but you "
			     "selected " + str(locations.size()) + " pixels.");
	}
	int locationInc = 0;
	for (UINT groupInc = 0; groupInc < locations.size() / currentPixelNumber; groupInc++)
	{
		addGroup();
		for (UINT pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
		{
			setGroupLocation(pixelInc, groupInc, locations[locationInc].second+1, locations[locationInc].first+1);
			locationInc++;
		}
	}
}

