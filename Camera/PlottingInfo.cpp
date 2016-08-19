#include "stdafx.h"
#include "PlottingInfo.h"
#include <sstream>

// All PlottingInfo() class public functions.

PlottingInfo::PlottingInfo()
{
	// initialize things.
	title = "";
	yLabel = "";
	fileName = "";
	dataSets.clear();
	// one line to plot.
	dataSets.resize(1);
	//
	analysisGroups.clear();
	// one pixel
	analysisGroups.resize(1);
	// one analysis set.
	analysisGroups[0].resize(1);
	//
	currentPixelNumber = 1;
	currentConditionNumber = 1;
	// this should get set again after the user presses "okay"
	numberOfPictures = ePicturesPerRepetition;
	xAxis = "";
	generalPlotType = "";
}

PlottingInfo::~PlottingInfo()
{
	// no fancy deconstrutor needed
}

int PlottingInfo::changeTitle(std::string newTitle)
{
	title = newTitle;
	return 0;
}
int PlottingInfo::changeYLabel(std::string newYLabel)
{
	yLabel = newYLabel;
	return 0;
}

int PlottingInfo::changeFileName(std::string newFileName) 
{
	fileName = newFileName;
	return 0;
}

int PlottingInfo::changeGeneralPlotType(std::string newPlotType)
{
	generalPlotType = newPlotType;
	return 0;
}

int PlottingInfo::changeXAxis(std::string newXAxis)
{
	xAxis = newXAxis;
	return 0;
}

int PlottingInfo::addGroup()
{
	for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() + 1);
	}
	return 0;
}

int PlottingInfo::setGroupLocation(int pixel, int analysisSet, int row, int collumn)
{
	if (pixel >= currentPixelNumber)
	{
		MessageBox(0, ("ERROR: something tried to set the position of a pixel that hasn't been allocated yet. Pixel #: " + std::to_string(pixel)).c_str(), 0, 
				   0);
		return -1;
	}
	if (analysisSet >= analysisGroups[pixel].size())
	{
		MessageBox(0, ("ERROR: something tried to set the position of an analysis set that hasn't been allocated yet. Analysis Set #: " 
				   + std::to_string(pixel)).c_str(), 0, 0);
		return -1;
	}
	analysisGroups[pixel][analysisSet][0] = row;
	analysisGroups[pixel][analysisSet][1] = collumn;
	return 0;
}

int PlottingInfo::addPixel()
{
	// change all the data set structures.
	currentPixelNumber++;
	analysisGroups.resize(analysisGroups.size() + 1);
	analysisGroups[analysisGroups.size() - 1].resize(analysisGroups[0].size());
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].addPixel(numberOfPictures);
	}
	return 0;
}

int PlottingInfo::removePixel()
{
	// make sure there is a pixel to remove.
	if (currentPixelNumber < 2)
	{
		MessageBox(0, "ERROR: Something tried to remove the last pixel!", 0, 0);
		return -1;
	}
	// change all the data set structures.
	currentPixelNumber--;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].removePixel();
	}
	return 0;
}



// Change all structures that depend on the number of data sets.
int PlottingInfo::addDataSet()
{
	
	// initialize new data set with 1 condition and the right number of pixels and pictures.
	dataSets.resize(dataSets.size() + 1);
	dataSets[dataSets.size() - 1].initialize(currentConditionNumber, currentPixelNumber, numberOfPictures);
	return 0;
}

int PlottingInfo::removeDataSet()
{
	// make sure there is a data set to remove.
	if (dataSets.size() < 2)
	{
		MessageBox(0, "ERROR: Something tried to remove the last data set", 0, 0);
		return -1;
	}
	
	dataSets.resize(dataSets.size() - 1);
	return 0;
}

// change all structures that depend on the number of pictures. 
int PlottingInfo::addPicture()
{
	numberOfPictures++;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].addPicture();
	}
	return 0;
}

int PlottingInfo::removePicture()
{
	if (numberOfPictures < 2)
	{
		MessageBox(0, "ERROR: Something tried to remove the last picture!", 0, 0);
		return -1;
	}
	numberOfPictures--;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].removePicture();
	}
	return 0;
}

int PlottingInfo::setPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture, int postSelectionCondition)
{
	dataSets[dataSetNumber].setPostSelectionCondition(conditionNumber, pixel, picture, postSelectionCondition);
	return 0;
}

int PlottingInfo::setTruthCondition(int dataSetNumber, int pixel, int picture, int trueConditionValue)
{
	dataSets[dataSetNumber].setTruthCondition(pixel, picture, trueConditionValue);
	return 0;
}

int PlottingInfo::getPixelLocation(int pixel, int analysisSet, int& row, int& collumn)
{
	row = analysisGroups[pixel][analysisSet][0];
	collumn = analysisGroups[pixel][analysisSet][1];
	return 0;
}

int PlottingInfo::removeAnalysisSet()
{
	// always at least one pixel...

	// make sure tat there is an analysis set to remove.
	for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		if (this->analysisGroups[pixelInc].size() < 2)
		{
			MessageBox(0, "ERROR: Something tried to remove the last analysis group!", 0, 0);
			return -1;
		}
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() - 1);
	}
	return 0;
}

int PlottingInfo::getTruthCondition(int dataSetNumber, int pixel, int picture)
{
	return dataSets[dataSetNumber].getTruthCondition(pixel, picture);
}

int PlottingInfo::getPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture)
{
	return dataSets[dataSetNumber].getPostSelectionCondition(conditionNumber, pixel, picture);
}

int PlottingInfo::addPostSelectionCondition()
{
	
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		// the currentPixelNumber is used as a vector index, the number of pictures is used as a resize size.
		dataSets[dataSetInc].addPostSelectionCondition(currentPixelNumber, numberOfPictures);
	}
	currentConditionNumber++;
	return 0;
}

int PlottingInfo::removePostSelectionCondition()
{
	
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].removePostSelectionCondition();
	}
	currentConditionNumber--;
	return 0;
}

std::string PlottingInfo::getPlotType()
{
	return generalPlotType;
}

std::string PlottingInfo::getXAxis()
{
	return xAxis;
}

int PlottingInfo::setDataCountsLocation(int dataSet, int pixel, int picture)
{
	if (dataSet < dataSets.size() && !(dataSet < 0))
	{
		dataSets[dataSet].setDataCountsLocation(currentPixelNumber, numberOfPictures, pixel, picture);
	}
	else
	{
		MessageBox(0, ("ERROR: tried to set data counts location for data set that hasn't been assigned yet. dataSet = " + std::to_string(dataSet)).c_str(), 0, 0);
		return -1;
	}
	return 0;
}
int PlottingInfo::setPlotData(int dataSet, bool plotData)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		MessageBox(0, ("ERROR: tried to set \"Plot this data\" value for data set that hasn't been assigned. dataSet = " + std::to_string(dataSet)).c_str(), 0, 
				   0);
		return -1;
	}
	dataSets[dataSet].setPlotThisData(plotData);
	return 0;
}

bool PlottingInfo::getPlotThisDataValue(int dataSet)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		MessageBox(0, ("ERROR: tried to get \"Plot this data\" value for data set that hasn't been assigned. dataSet = " + std::to_string(dataSet)).c_str(), 0,
			0);
		return -1;
	}
	
	return dataSets[dataSet].getPlotThisDataValue();
}

int PlottingInfo::getDataCountsLocation(int dataSet, int& pixel, int& picture)
{
	if (dataSets[dataSet].getDataCountsLocation(pixel, picture) == -1)
	{
		// didn't find it.
		return -1;
	}
	return 0;
}

int PlottingInfo::changeLegendText(int dataSet, std::string newLegend)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		MessageBox(0, ("ERROR: attempted to set dataset legend for data set that hadn't been allocated. dataset = " + std::to_string(dataSet)).c_str(), 0, 0);
		return -1;
	}
	dataSets[dataSet].changeLegendText(newLegend);
	return 0;
}

std::string PlottingInfo::getLegendText(int dataSet)
{
	if (dataSet >= dataSets.size() || dataSet < 0)
	{
		MessageBox(0, ("ERROR: attempted to get dataset legend for data set that hadn't been allocated. dataset = " + std::to_string(dataSet)).c_str(), 0, 0);
		return "";
	}
	return dataSets[dataSet].getLegendText();
}

std::string PlottingInfo::returnAllInfo()
{
	std::string message = "All Plotting Parameters\r\n=======================================\r\n";
	message += "Plot Title: " + title + "\r\n";
	message += "Plot Type: " + generalPlotType + "\r\n";
	message += "y Label: " + yLabel + "\r\n";
	message += "x axis: " + xAxis + "\r\n";
	message += "file name: " + fileName + "\r\n";
	
	message += "\r\nAll Currently Set Analysis Pixels (rowXcollumn):\r\n=========================\r\n\r\n";
	if (analysisGroups.size() != 0)
	{
		for (int pixelGroupInc = 0; pixelGroupInc < analysisGroups[0].size(); pixelGroupInc++)
		{
			message += "Group #" + std::to_string(pixelGroupInc + 1) + ":\r\n";
			for (int pixelInc = 0; pixelInc < analysisGroups.size(); pixelInc++)
			{
				int row, collumn;
				row = analysisGroups[pixelInc][pixelGroupInc][0];
				collumn = analysisGroups[pixelInc][pixelGroupInc][1];
				message += "\tPixel #" + std::to_string(pixelInc + 1) + ": ("
					+ std::to_string(row) + "X"
					+ std::to_string(collumn) + ")\r\n";
			}
		}
	}
	else
	{
		message += "No analysis pixels???\r\n";
	}

	message += "\r\nAll Current Truth Conditions:\r\n=========================\r\n\r\n";
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += "Data Set #" + std::to_string(dataSetInc + 1) + ":\r\n=====\r\n";
		for (int pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
		{
			message += "Picture #" + std::to_string(pictureInc + 1) + ":\r\n";
			for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
			{
				message += "Pixel #" + std::to_string(pixelInc + 1) + ":";
				int currentValue = dataSets[dataSetInc].getTruthCondition(pixelInc, pictureInc);
				if (currentValue == 1)
				{
					message += " Atom Present";
				}
				else if (currentValue == -1)
				{
					message += " Atom Not Present";
				}
				else
				{
					message += " No Condition";
				}
				message += "\r\n";
			}
		}
		message += "=====\r\n";
	}

	message += "All Current Post-Selection Conditions:\r\n=========================\r\n\r\n";
	for (int conditionInc = 0; conditionInc < currentConditionNumber; conditionInc++)
	{
		message += "*****Condition #" + std::to_string(conditionInc + 1) + "*****\r\n";
		for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
		{
			message += "Data Set #" + std::to_string(dataSetInc + 1) + ":\r\n=====\r\n";
			for (int pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
			{
				message += "Picture #" + std::to_string(pictureInc + 1) + ":\r\n";
				for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
				{
					message += "Pixel #" + std::to_string(pixelInc + 1) + ":";
					int currentValue = dataSets[dataSetInc].getPostSelectionCondition(conditionInc, pixelInc, pictureInc);;
					if (currentValue == 1)
					{
						message += " Atom Present";
					}
					else if (currentValue == -1)
					{
						message += " Atom Not Present";
					}
					else
					{
						message += " No Condition";
					}
					message += "\r\n";
				}
			}
			message += "=====\r\n";
		}
	}
	return message;
}

int PlottingInfo::savePlotInfo()
{
	std::string completeAddress = PLOT_FILES_SAVE_LOCATION + fileName + ".plot";
	std::fstream saveFile(completeAddress.c_str(), std::fstream::out);
	if (!saveFile.is_open())
	{
		MessageBox(0, ("Couldn't open file at + " + PLOT_FILES_SAVE_LOCATION + fileName + ".plot!").c_str(), 0, 0);
		return -1;
	}
	std::string message;
	message += title + "\n";
	message += generalPlotType + "\n";
	message += yLabel + "\n";
	message += xAxis + "\n";
	message += fileName + "\n";

	message += std::to_string(dataSets.size()) + "\n";
	message += std::to_string(currentConditionNumber) + "\n";
	message += std::to_string(currentPixelNumber) + "\n";

	if (currentPixelNumber > 0)
	{
		message += std::to_string(analysisGroups[0].size()) + "\n";
	}
	else
	{
		message += "0\n";
	}
	message += std::to_string(numberOfPictures) + "\n";

	message += "ANALYSIS PIXELS START\n";
	
	for (int pixelSetInc = 0; pixelSetInc < analysisGroups[0].size(); pixelSetInc++)
	{
		message += "GROUP BEGIN\n";
		for (int pixelInc = 0; pixelInc < analysisGroups.size(); pixelInc++)
		{
			int row, collumn;
			row = analysisGroups[pixelInc][pixelSetInc][0];
			collumn = analysisGroups[pixelInc][pixelSetInc][1];
			message += std::to_string(row) + " " + std::to_string(collumn) + "\n";
		}
		message += "GROUP END\n";
	}
	message += "ANALYSIS PIXELS END\n";
	message += "TRUTH BEGIN\n";
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += "DATA SET BEGIN\n";
		for (int pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
		{
			message += "PICTURE BEGIN\n";
			for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
			{
				message += std::to_string(dataSets[dataSetInc].getTruthCondition(pixelInc, pictureInc)) + "\n";
			}
			message += "PICTURE END\n";
		}
		message += "DATA SET END\n";
	}
	message += "TRUTH END\n";
	message += "POST SELECTION BEGIN\n";
	for (int conditionInc = 0; conditionInc < currentConditionNumber; conditionInc++)
	{
		message += "CONDITION BEGIN\n";
		for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
		{
			message += "DATA SET BEGIN\n";
			for (int pictureInc = 0; pictureInc < numberOfPictures; pictureInc++)
			{
				message += "PICTURE BEGIN\n";
				for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
				{
					message += std::to_string(dataSets[dataSetInc].getPostSelectionCondition(conditionInc, pixelInc, pictureInc)) + "\n";
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
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += getLegendText(dataSetInc) + "\n";
	}
	message += "LEGENDS END\n";

	// data count locations
	message += "PLOT COUNTS LOCATIONS BEGIN\n";
	bool failedOnce = false;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		int pixel, picture;
		
		if (getDataCountsLocation(dataSetInc, pixel, picture) == 0)
		{
			// if things are going well, shouldn't have failed. 
			if (failedOnce)
			{
				MessageBox(0, "ERROR: The getDataCountsLocation function isn't consistently finding locations!", 0, 0);
				return -1;
			}
			message += std::to_string(pixel) + " " + std::to_string(picture) + "\n";
		}
		else
		{
			failedOnce = true;
		}
	}
	message += "PLOT COUNTS LOCATIONS END\n";
	
	message += "FITTING OPTIONS BEGIN\n";
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		message += std::to_string(dataSets[dataSetInc].getFitType()) + " " + std::to_string(dataSets[dataSetInc].getWhenToFit()) + "\n";
	}
	message += "FITTING OPTIONS END\n";
	saveFile << message;
	return 0;
}

int PlottingInfo::loadPlottingInfoFromFile(std::string fileLocation)
{
	std::fstream loadingFile(fileLocation.c_str());
	if (!loadingFile.is_open())
	{
		MessageBox(0, "ERROR: Couldn't open plot file!", 0, 0);
		return -1;
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
		MessageBox(0, ("ERROR: Couldn't read data set number from file. The data set string was " + testString).c_str(), 0, 0);
		return -1;
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
		MessageBox(0, ("ERROR: Couldn't read post-selection number from file. The post-selection string was " + testString).c_str(), 0, 0);
		return -1;
	}
	/// Pixel Number
	getline(loadingFile, testString);
	try
	{
		currentPixelNumber = std::stoi(testString);
		resetPixelNumber(currentPixelNumber);
	}
	catch (std::invalid_argument&)
	{
		MessageBox(0, ("ERROR: Couldn't read pixel number from file. The pixel string was " + testString).c_str(), 0, 0);
		return -1;
	}
	/// Analysis Group Number
	getline(loadingFile, testString);
	int tempAnalysisGroupNumber;
	try
	{
		tempAnalysisGroupNumber = std::stoi(testString);
		resetNumberOfAnalysisGroups(tempAnalysisGroupNumber);
	}
	catch (std::invalid_argument&)
	{
		MessageBox(0, ("ERROR: Couldn't read analysis group number from file. The analysis group string was " + testString).c_str(), 0, 0);
		return -1;
	}
	/// Picture Number
	getline(loadingFile, testString);
	try
	{
		numberOfPictures = std::stoi(testString);
		resetPictureNumber(numberOfPictures);
	}
	catch (std::invalid_argument&)
	{
		MessageBox(0, ("ERROR: Couldn't read Picture number from file. The picture string was " + testString).c_str(), 0, 0);
		return -1;
	}

	/// Analys pixels
	getline(loadingFile, testString);
	if (testString != "ANALYSIS PIXELS START")
	{
		MessageBox(0, ("File Corrupted or malformatted! Expected \"ANALYSIS PIXELS START\", but saw instead" + testString).c_str(), 0, 0);
		return -1;
	}

	// all loops in this function are broken out with break;
	// loop for pixel locations
	int analysisGroupCount = 0;
	while (true)
	{
		// loop for an individual set
		getline(loadingFile, testString);

		if (testString != "GROUP BEGIN")
		{
			if (testString == "ANALYSIS PIXELS END")
			{
				break;
			}
			else
			{
				MessageBox(0, ("File Corrupted or malformatted! Expected \"GROUP BEGIN\", but saw instead" + testString).c_str(), 0, 0);
				return -1;
			}
		}
		if (analysisGroupCount >= tempAnalysisGroupNumber)
		{
			MessageBox(0, "File Corrupted or malformatted! There are more analysis groups recorded than the number listed at the beginning of the file.", 0, 0);
			return -1;
		}
		int pixelCount = 0;
		while (true)
		{
			std::stringstream tempStream;
			std::string rowStr, collumnStr;
			int row, collumn;

			getline(loadingFile, testString);
			if (testString == "GROUP END")
			{
				break;
			}
			tempStream << testString;
			tempStream >> rowStr;
			try
			{
				row = std::stoi(rowStr);
			}
			catch (std::invalid_argument&)
			{
				MessageBox(0, ("ERROR: Row listed in file did not convert to integer correctly. Row string was" + rowStr).c_str(), 0, 0);
				return -1;
			}

			tempStream >> collumnStr;
			try
			{
				collumn = std::stoi(collumnStr);
			}
			catch (std::invalid_argument&)
			{
				MessageBox(0, ("ERROR: collumn listed in file did not convert to integer correctly. collumn string was" + collumnStr).c_str(), 0, 0);
				return -1;
			}
			setGroupLocation(pixelCount, analysisGroupCount, row, collumn);
			pixelCount++;
		}
		if (pixelCount != currentPixelNumber)
		{
			MessageBox(0, ("ERROR: pixel number recorded in file doesn't match actual number of pixels for analysis group #" 
							+ std::to_string(analysisGroupCount)).c_str(), 0, 0);
			return -1;
		}
		analysisGroupCount++;
	}
	if (analysisGroupCount != analysisGroups[0].size())
	{
		MessageBox(0, "ERROR: analysis groups number recorded in the file doesn't match information recorded.", 0, 0);
		return -1;
	}

	getline(loadingFile, testString);
	if (testString != "TRUTH BEGIN")
	{
		MessageBox(0, ("File Corrupted or malformatted! Expected \"TRUTH BEGIN\", but saw instead\"" + testString + "\"").c_str(), 0, 0);
		return -1;
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
			MessageBox(0, ("File Corrupted or malformatted! Expected \"DATA SET BEGIN\", but saw instead\"" + testString + "\"").c_str(), 0, 0);
			return -1;
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
					MessageBox(0, ("File Corrupted or malformatted! Expected \"PICTURE BEGIN\", but saw instead\"" + testString + "\"").c_str(), 0, 0);
					return -1;
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
					MessageBox(0, ("ERROR: truth condition failed to evaluate to an integer. The truth condition string was" + testString).c_str(), 0, 0);
					return -1;
				}
				if (tempTruthCondition != -1 && tempTruthCondition != 0 && tempTruthCondition != 1)
				{
					MessageBox(0, ("ERROR: truth condition was not one of the valid options: -1, 0, or 1. The truth condition was" 
									+ std::to_string(tempTruthCondition)).c_str(), 0, 0);
					return -1;
				}
				setTruthCondition(dataSetCount, pixelCount, pictureCount, tempTruthCondition);
				//(int dataSetNumber, int pixel, int picture, int trueConditionValue)
				pixelCount++;
			}
			if (pixelCount != currentPixelNumber)
			{
				MessageBox(0, "ERROR: number of pixels the truth condition was set for doesn't match the number of pixels reported earlier in the file.", 0, 0);
				return -1;
			}
			pictureCount++;
		}
		if (pictureCount != numberOfPictures)
		{
			MessageBox(0, "ERROR: number of pictures the truth condition was set for doesn't match the number of pictures reported earlier in the file.", 0, 0);
			return -1;
		}
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size())
	{
		MessageBox(0, "ERROR: number of data sets the truth condition was set for doesn't match the number of data sets reported earlier in the file.", 0, 0);
		return -1;
	}
	getline(loadingFile, testString);
	if (testString != "POST SELECTION BEGIN")
	{
		MessageBox(0, ("ERROR: Expected text \"POST SELECTION BEGIN\" but instead saw " + testString).c_str(), 0, 0);
		return -1;
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
			MessageBox(0, ("File Corrupted or malformatted! Expected \"CONDITION BEGIN\", but saw instead\"" + testString + "\"").c_str(), 0, 0);
			return -1;
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
				MessageBox(0, ("File Corrupted or malformatted! Expected \"DATA SET BEGIN\", but saw instead\"" + testString + "\"").c_str(), 0, 0);
				return -1;
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
						MessageBox(0, ("File Corrupted or malformatted! Expected \"PICTURE BEGIN\", but saw instead\"" + testString + "\"").c_str(), 0, 0);
						return -1;
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
						MessageBox(0, ("ERROR: truth condition failed to evaluate to an integer. The truth condition string was" + testString).c_str(), 0, 0);
						return -1;
					}
					if (tempPostSelectionCondition != -1 && tempPostSelectionCondition != 0 && tempPostSelectionCondition != 1)
					{
						MessageBox(0, ("ERROR: truth condition was not one of the valid options: -1, 0, or 1. The truth condition was"
							+ std::to_string(tempPostSelectionCondition)).c_str(), 0, 0);
						return -1;
					}
					setPostSelectionCondition(dataSetCount, conditionCount, pixelCount, pictureCount, tempPostSelectionCondition);
					pixelCount++;
				}
				if (pixelCount != currentPixelNumber)
				{
					MessageBox(0, "ERROR: number of pixels the truth condition was set for doesn't match the number of pixels reported earlier in the file.", 0, 0);
					return -1;
				}
				pictureCount++;
			}
			if (pictureCount != numberOfPictures)
			{
				MessageBox(0, "ERROR: number of pictures the truth condition was set for doesn't match the number of pictures reported earlier in the file.", 0, 0);
				return -1;
			}
			dataSetCount++;
		}
		if (dataSetCount != dataSets.size())
		{
			MessageBox(0, "ERROR: number of data sets the truth condition was set for doesn't match the number of data sets reported earlier in the file.", 0, 0);
			return -1;
		}
		conditionCount++;
	}
	if (conditionCount != currentConditionNumber)
	{
		MessageBox(0, "ERROR: number of post selection conditions that were set doesn't match the number of conditions reported earlier in the file.", 0, 0);
		return -1;
	}
	
	// get legends
	getline(loadingFile, testString);
	if (testString != "LEGENDS BEGIN")
	{
		MessageBox(0, ("ERROR: Expected text \"LEGENDS BEGIN\" but got text" + testString).c_str(), 0, 0);
		return -1;
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
		MessageBox(0, "ERROR: the number of data sets that legends were read for doesn't match the number of data sets reported earlier in the file.", 0, 0);
		return -1;
	}
	// get counts locations
	getline(loadingFile, testString);
	if (testString != "PLOT COUNTS LOCATIONS BEGIN")
	{
		MessageBox(0, ("ERROR: Expected text \"PLOT COUNTS LOCATIONS BEGIN\" but got text" + testString).c_str(), 0, 0);
		return -1;
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
			MessageBox(0, ("ERROR: pixel listed in file did not convert to integer correctly. pixel string was" + pixelStr).c_str(), 0, 0);
			return -1;
		}

		tempStream >> pictureStr;
		try
		{
			picture = std::stoi(pictureStr);
		}
		catch (std::invalid_argument&)
		{
			MessageBox(0, ("ERROR: picture listed in file did not convert to integer correctly. picture string was" + pictureStr).c_str(), 0, 0);
			return -1;
		}
		this->setDataCountsLocation(dataSetCount, pixel, picture);
		dataSetCount++;
	}

	if (generalPlotType == "Pixel Count Histograms" || generalPlotType == "Pixel Counts")
	{
		if (dataSetCount != dataSets.size())
		{
			MessageBox(0, "ERROR: the number of data sets that counts locations were read for doesn't match the number of data sets reported earlier in the file.", 0, 0);
			return -1;
		}
	}
	else
	{
		if (dataSetCount > 0)
		{
			MessageBox(0, "ERROR: There were counts plotting locations listed in the file despite the plot type not being pixel count histograms or pixel counts.", 0, 0);
			return -1;
		}
	}

	getline(loadingFile, testString);
	if (testString != "FITTING OPTIONS BEGIN")
	{
		MessageBox(0, ("ERROR: Expected text \"FITTING OPTIONS BEGIN\" but got text" + testString).c_str(), 0, 0);
		return -1;
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
			MessageBox(0, ("ERROR: fit option listed in file did not convert to integer correctly. fit option string was" + fitOptionStr).c_str(), 0, 0);
			return -1;
		}

		tempStream >> whenToFitStr;
		try
		{
			whenToFit = std::stoi(whenToFitStr);
		}
		catch (std::invalid_argument&)
		{
			MessageBox(0, ("ERROR: When to Fit option listed in file did not convert to integer correctly. when to fit string was" + whenToFitStr).c_str(), 0, 0);
			return -1;
		}
		dataSets[dataSetCount].setFitType(fitOption);
		dataSets[dataSetCount].setWhenToFit(whenToFit);
		dataSetCount++;
	}
	if (dataSetCount > dataSets.size())
	{
		MessageBox(0, "ERROR: more fit options than data sets!", 0, 0);
		return -1;
	}

	return 0;
}

int PlottingInfo::resetNumberOfAnalysisGroups(int setNumber)
{
	for (int pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++)
	{
		analysisGroups[pixelInc].clear();
		analysisGroups[pixelInc].resize(setNumber);
	}
	return 0;
}

int PlottingInfo::resetPixelNumber(int pixelNumber)
{
	currentPixelNumber = pixelNumber;
	analysisGroups.resize(pixelNumber);
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPixelNumber(pixelNumber);
	}
	return 0;
}

int PlottingInfo::resetPictureNumber(int pictureNumber)
{
	numberOfPictures = pictureNumber;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPictureNumber(pictureNumber);
	}
	return 0;
}

int PlottingInfo::resetConditionNumber(int conditionNumber)
{
	currentConditionNumber = conditionNumber;
	for (int dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++)
	{
		dataSets[dataSetInc].resetPostSelectionConditionNumber(conditionNumber);
	}
	return 0;
}

int PlottingInfo::resetDataSetNumber(int dataSetNumber)
{
	dataSets.clear();
	dataSets.resize(dataSetNumber);
	return 0;
}

int PlottingInfo::clear()
{
	// arbitrary
	title = "";
	// arbitrary
	yLabel = "";
	// analysisGroups[pixel #][pixel set].first = row
	// analysisGroups[pixel #][pixel set].second = collumn
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
	return 0;
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

int PlottingInfo::getPictureNumber()
{
	return numberOfPictures;
}

int PlottingInfo::getConditionNumber()
{
	return currentConditionNumber;
}

int PlottingInfo::getPixelGroupNumber()
{
	if (analysisGroups.size() == 0)
	{
		return -1;
	}
	return analysisGroups[0].size();
}

int PlottingInfo::getDataSetNumber()
{
	return dataSets.size();
}

int PlottingInfo::setPixelIndex(int pixel, int group, int index)
{
	analysisGroups[pixel][group][2] = index;
	return 0;
}

int PlottingInfo::getPixelIndex(int pixel, int group)
{
	return analysisGroups[pixel][group][2];
}

int PlottingInfo::setFitOption(int dataSet, int fitType)
{
	dataSets[dataSet].setFitType(fitType);
	return 0;
}

int PlottingInfo::setWhenToFit(int dataSet, int whenToFit)
{
	dataSets[dataSet].setWhenToFit(whenToFit);
	return 0;
}

int PlottingInfo::getFitOption(int dataSet)
{
	return dataSets[dataSet].getFitType();
}

int PlottingInfo::getWhenToFit(int dataSet)
{
	return dataSets[dataSet].getWhenToFit();
}

std::vector<std::pair<int, int>> PlottingInfo::getAllPixelLocations()
{
	std::vector<std::pair<int, int>> allUniqueLocations;
	for (int pixelInc = 0; pixelInc < this->analysisGroups.size(); pixelInc++)
	{
		for (int groupInc = 0; groupInc < this->analysisGroups[pixelInc].size(); groupInc++)
		{
			std::pair<int, int> tempLocation;
			tempLocation.first = this->analysisGroups[pixelInc][groupInc][0];
			tempLocation.second = this->analysisGroups[pixelInc][groupInc][1];
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