#include "stdafx.h"
#include "DataSet.h"

/// All DataSet() class public functions.
DataSet::DataSet()
{
	trueConditions.clear();
	// Start with one picture
	trueConditions.resize(1);
	// start pictures loaded into experiment
	trueConditions[0].resize(ePicturesPerRepetition);

	postSelectionConditions.clear();
	// start with one condition
	postSelectionConditions.resize(1);
	// start with one pixel
	postSelectionConditions[0].resize(1);
	// start pictures loaded into experiment
	postSelectionConditions[0][0].resize(ePicturesPerRepetition);
	// start with one pixel
	dataCountsLocation.resize(1);
	// start with one picture
	dataCountsLocation[0].resize(1);
}

DataSet::~DataSet() 
{

}

int DataSet::initialize(int conditionNumber, int pixelNumber, int pictureNumber)
{
	trueConditions.resize(pixelNumber);
	dataCountsLocation.resize(pixelNumber);
	for (int pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
	{
		trueConditions[pixelInc].resize(pictureNumber);
		dataCountsLocation[pixelInc].resize(pictureNumber);
	}
	
	postSelectionConditions.resize(conditionNumber);
	for (int conditionInc = 0; conditionInc < conditionNumber; conditionInc++)
	{
		postSelectionConditions[conditionInc].resize(pixelNumber);
		for (int pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
		{
			postSelectionConditions[conditionInc][pixelInc].resize(pictureNumber);
		}
	}

	return 0;
}

int DataSet::changeLegendText(std::string newLegendText)
{
	legendText = newLegendText;
	return 0;
}

int DataSet::addPicture()
{
	// all data structures have access to the same number of pictures. Currently there is no to have one data structure work on 2 picture increments and one on
	// 3, for example.
	int currentPixelNum = trueConditions.size();
	// this should always be at least one large.
	int currentPictureNum = trueConditions[0].size();
	for (int pixelInc = 0; pixelInc < currentPixelNum; pixelInc++)
	{
		trueConditions[pixelInc].resize(currentPictureNum + 1);
		dataCountsLocation[pixelInc].resize(currentPictureNum + 1);
		for (int postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
		{
			postSelectionConditions[postSelectionConditionInc][pixelInc].resize(currentPictureNum + 1);
		}

	}


	return 0;
}

// picture number is used to initialize the new pixel to the correct size.
int DataSet::addPixel(int pictureNumber)
{
	// get the current pixel number. This should be uniform across thes data structures.
	int currentPixelNum = trueConditions.size();
	// all data sets have access to the same pixels, whether they actually use them for true values or post selection or not.
	trueConditions.resize(currentPixelNum + 1);
	trueConditions[currentPixelNum].resize(pictureNumber);
	dataCountsLocation.resize(currentPixelNum + 1);
	dataCountsLocation[currentPixelNum].resize(pictureNumber);

	for (int postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
	{
		postSelectionConditions[postSelectionConditionInc].resize(currentPixelNum + 1);
		postSelectionConditions[postSelectionConditionInc][currentPixelNum].resize(pictureNumber);
	}

	return 0;
}

int DataSet::setTruthCondition(int pixel, int picture, int trueCondition)
{
	if (trueCondition != -1 && trueCondition != 0 && trueCondition != 1)
	{
		MessageBox(0, ("ERROR: something attempted to set a true condition to value " + std::to_string(trueCondition)).c_str(), 0, 0);
		return -1;
	}
	if (pixel > trueConditions.size() - 1)
	{
		MessageBox(0, ("ERROR: tried to set the true condition of a pixel that isn't loaded into the condition vector. Pixel number: " 
					   + std::to_string(pixel)).c_str(), 0, 0);
		return -1;
	}
	if (picture > trueConditions[pixel].size() - 1)
	{
		MessageBox(0, ("ERROR: tried to set the true condition of a picture that isn't loaded into the condition vector for the given pixel. Picture number: "
			+ std::to_string(picture)).c_str(), 0, 0);
		return -1;
	}
	trueConditions[pixel][picture] = trueCondition;
	return 0;
}

int DataSet::setPostSelectionCondition(int conditionNumber, int pixel, int picture, int postSelectionCondition)
{

	if (conditionNumber > postSelectionConditions.size() - 1)
	{
		MessageBox(0, ("ERROR: tried to set the post selection condition of a condition number that isn't loaded into the condition vector. Condition number: "
			+ std::to_string(conditionNumber)).c_str(), 0, 0);
		return -1;

	}
	if (pixel > postSelectionConditions[conditionNumber].size() - 1)
	{
		MessageBox(0, ("ERROR: tried to set the post selection condition of a pixel that isn't loaded into the condition vector. Pixel number: "
			+ std::to_string(pixel)).c_str(), 0, 0);
		return -1;
	}
	if (picture > postSelectionConditions[conditionNumber][pixel].size() - 1)
	{
		MessageBox(0, ("ERROR: tried to set the post selection condition of a picture that isn't loaded into the condition vector for the given pixel. Picture number: "
			+ std::to_string(picture)).c_str(), 0, 0);
		return -1;
	}
	postSelectionConditions[conditionNumber][pixel][picture] = postSelectionCondition;
	return 0;
}

int DataSet::removePixel()
{
	// make sure there is a pixel to remove.
	int currentPixelNum = trueConditions.size();
	if (currentPixelNum < 2)
	{
		MessageBox(0, "ERROR: Something tried to remove the last pixel!", 0, 0);
		return - 1;
	}
	trueConditions.resize(currentPixelNum - 1);
	for (int postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
	{
		postSelectionConditions[postSelectionConditionInc].resize(currentPixelNum - 1);
	}
	return 0;
}

int DataSet::removePicture()
{
	// make sure there is a picture to remove.
	// all data structures have access to the same number of pictures. Currently there is no to have one data structure work on 2 picture increments and one on
	// 3, for example.
	int currentPixelNum = trueConditions.size();
	// this should always be at least one large.
	int currentPictureNum = trueConditions[0].size();
	
	if (currentPictureNum < 2)
	{
		MessageBox(0, "ERROR: Something tried to remove the last picture!", 0, 0);
		return -1;
	}
	for (int pixelInc = 0; pixelInc < currentPixelNum; pixelInc++)
	{
		trueConditions[pixelInc].resize(currentPictureNum - 1);
		for (int postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
		{
			postSelectionConditions[postSelectionConditionInc][pixelInc].resize(currentPictureNum - 1);
		}
	}
	return 0;
}

int DataSet::getTruthCondition(int pixel, int picture)
{
	if (pixel >= trueConditions.size())
	{
		MessageBox(0, ("ERROR: tried to retrieve true condition from pixel that hasn't been allocated. Pixel: " + std::to_string(pixel)).c_str(), 0, 0);
		return -10;
	}
	if (picture >= trueConditions[pixel].size())
	{
		MessageBox(0, ("ERROR: tried to retrieve true condition from picture that hasn't been allocated. Picture: " + std::to_string(picture)).c_str(), 0, 0);
		return -10;
	}
	return trueConditions[pixel][picture];
}
int DataSet::getPostSelectionCondition(int conditionNumber, int pixel, int picture)
{
	if (conditionNumber > postSelectionConditions.size())
	{
		MessageBox(0, ("ERROR: tried to retrieve post selection condition from condition that hasn't been allocated. condition: " 
			+ std::to_string(conditionNumber)).c_str(), 0, 0);
		return -10;
	}
	if (pixel >= postSelectionConditions[conditionNumber].size())
	{
		MessageBox(0, ("ERROR: tried to retrieve post selection condition from pixel that hasn't been allocated. Pixel: " + std::to_string(pixel)).c_str(), 0, 0);
		return -10;
	}
	if (picture >= postSelectionConditions[conditionNumber][pixel].size())
	{
		MessageBox(0, ("ERROR: tried to retrieve post selection condition from picture that hasn't been allocated. Picture: " + std::to_string(picture)).c_str(), 0, 0);
		return -10;
	}
	return postSelectionConditions[conditionNumber][pixel][picture];
}

int DataSet::addPostSelectionCondition(int pixelNum, int pictureNum)
{
	// add condition, pixels, pictures
	postSelectionConditions.resize(postSelectionConditions.size() + 1);
	int back = postSelectionConditions.size() - 1;
	postSelectionConditions[back].resize(pixelNum);
	for (int pixelInc = 0; pixelInc < pixelNum; pixelInc++)
	{
		postSelectionConditions[back][pixelInc].resize(pictureNum);
	}
	return 0;
}

int DataSet::removePostSelectionCondition()
{
	if (postSelectionConditions.size() == 0)
	{
		MessageBox(0, "ERROR: something tried to remove a post-selection condition when there weren't any!", 0, 0);
		return -1;
	}
	postSelectionConditions.resize(postSelectionConditions.size() - 1);
	return 0;
}

int DataSet::setDataCountsLocation(int maxPixel, int maxPicture, int pixel, int picture)
{
	dataCountsLocation.clear();
	dataCountsLocation.resize(maxPixel);
	for (int pixelCount = 0; pixelCount < maxPixel; pixelCount++)
	{
		dataCountsLocation[pixelCount].resize(maxPicture);
	}
	dataCountsLocation[pixel][picture] = true;

	return 0;
}

int DataSet::setPlotThisData(bool plotThisDataInput)
{
	plotThisData = plotThisDataInput;
	return 0;
}

bool DataSet::getPlotThisDataValue()
{
	return plotThisData;
}

int DataSet::getDataCountsLocation(int& pixel, int& picture)
{
	for (int pixelInc = 0; pixelInc < dataCountsLocation.size(); pixelInc++)
	{
		for (int pictureInc = 0; pictureInc < dataCountsLocation[pixelInc].size(); pictureInc++)
		{
			if (dataCountsLocation[pixelInc][pictureInc] == true)
			{
				pixel = pixelInc;
				picture = pictureInc;
				return 0;
			}
		}
	}
	return -1;
}

std::string DataSet::getLegendText()
{
	return legendText;
}

int DataSet::resetPictureNumber(int pictureNumber)
{
	// trueConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (int pixelInc = 0; pixelInc < trueConditions.size(); pixelInc++)
	{
		trueConditions[pixelInc].clear();
		trueConditions[pixelInc].resize(pictureNumber);
	}
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (int conditionInc = 0; conditionInc < postSelectionConditions.size(); conditionInc++)
	{
		for (int pixelInc = 0; pixelInc < postSelectionConditions[conditionInc].size(); pixelInc++)
		{
			postSelectionConditions[conditionInc][pixelInc].clear();
			postSelectionConditions[conditionInc][pixelInc].resize(pictureNumber);
		}
	}

	// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
	for (int pixelInc = 0; pixelInc < dataCountsLocation.size(); pixelInc++)
	{
		dataCountsLocation[pixelInc].clear();
		dataCountsLocation[pixelInc].resize(pictureNumber);
	}

	return 0;
}

int DataSet::resetPostSelectionConditionNumber(int conditionNumber)
{
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	postSelectionConditions.clear();
	postSelectionConditions.resize(conditionNumber);
	return 0;
}

int DataSet::resetPixelNumber(int pixelNumber)
{
	// trueConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	trueConditions.clear();
	trueConditions.resize(pixelNumber);

	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (int conditionInc = 0; conditionInc < postSelectionConditions.size(); conditionInc++)
	{
		postSelectionConditions[conditionInc].clear();
		postSelectionConditions[conditionInc].resize(pixelNumber);
	}

	// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
	dataCountsLocation.clear();
	dataCountsLocation.resize(pixelNumber);

	return 0;
}

int DataSet::clear()
{
	// trueConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	trueConditions.clear();
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	postSelectionConditions.clear();
	// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
	dataCountsLocation.clear();
	// arbitrary.
	legendText = "";
	//
	plotThisData = 0;
	return 0;
}

int DataSet::getFitType()
{
	return fitType;
}

int DataSet::getWhenToFit()
{
	return whenToFit;
}

int DataSet::setFitType(int newFitType)
{
	fitType = newFitType;
	return 0;
}
int DataSet::setWhenToFit(int newWhenToFit)
{
	whenToFit = newWhenToFit;
	return 0;
}