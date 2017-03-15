#include "stdafx.h"
#include "DataSet.h"

/// All DataSet() class public functions.
DataSet::DataSet()
{
	trueConditions.clear();
	// Start with one picture
	trueConditions.resize(1);
	// start pictures loaded into experiment
	//trueConditions[0].resize(ePicturesPerRepetition);

	postSelectionConditions.clear();
	// start with one condition
	postSelectionConditions.resize(1);
	// start with one pixel
	postSelectionConditions[0].resize(1);
	// start pictures loaded into experiment
	//postSelectionConditions[0][0].resize(ePicturesPerRepetition);
	// start with one pixel
	dataCountsLocation.resize(1);
	// start with one picture
	dataCountsLocation[0].resize(1);
}


void DataSet::initialize(int conditionNumber, int pixelNumber, int pictureNumber)
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
}


void DataSet::changeLegendText(std::string newLegendText)
{
	legendText = newLegendText;
}


void DataSet::addPicture()
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
}

// picture number is used to initialize the new pixel to the correct size.
void DataSet::addPixel(int pictureNumber)
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
		postSelectionConditions[postSelectionConditionInc][currentPixelNum].resize(pictureNumber); ///
	}
}


void DataSet::setTruthCondition(int pixel, int picture, int trueCondition)
{
	if (trueCondition != -1 && trueCondition != 0 && trueCondition != 1)
	{
		thrower("ERROR: something attempted to set a true condition to value " + std::to_string(trueCondition));
	}
	if (pixel > trueConditions.size() - 1)
	{
		thrower("ERROR: tried to set the true condition of a pixel that isn't loaded into the condition vector. Pixel number: " 
					   + std::to_string(pixel));
	}
	if (picture > trueConditions[pixel].size() - 1)
	{
		thrower("ERROR: tried to set the true condition of a picture that isn't loaded into the condition vector for the given pixel. "
				 "Picture number: "	+ std::to_string(picture));
	}
	trueConditions[pixel][picture] = trueCondition;
}

void DataSet::setPostSelectionCondition(int conditionNumber, int pixel, int picture, int postSelectionCondition)
{

	if (conditionNumber > postSelectionConditions.size() - 1)
	{
		thrower("ERROR: tried to set the post selection condition of a condition number that isn't loaded into the condition vector. "
				 "Condition number: " + std::to_string(conditionNumber));

	}
	if (pixel > postSelectionConditions[conditionNumber].size() - 1)
	{
		thrower("ERROR: tried to set the post selection condition of a pixel that isn't loaded into the condition vector. Pixel "
						"number: " + std::to_string(pixel));
	}
	if (picture > postSelectionConditions[conditionNumber][pixel].size() - 1)
	{
		thrower("ERROR: tried to set the post selection condition of a picture that isn't loaded into the condition vector for the given pixel. Picture number: "
			+ std::to_string(picture));
	}
	postSelectionConditions[conditionNumber][pixel][picture] = postSelectionCondition;
}

void DataSet::removePixel()
{
	// make sure there is a pixel to remove.
	int currentPixelNum = trueConditions.size();
	if (currentPixelNum < 2)
	{
		thrower("ERROR: Something tried to remove the last pixel!");
	}
	trueConditions.resize(currentPixelNum - 1);
	for (int postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
	{
		postSelectionConditions[postSelectionConditionInc].resize(currentPixelNum - 1);
	}
}


void DataSet::removePicture()
{
	// make sure there is a picture to remove.
	// all data structures have access to the same number of pictures. Currently there is no to have one data structure work on 2 picture increments and one on
	// 3, for example.
	int currentPixelNum = trueConditions.size();
	// this should always be at least one large.
	int currentPictureNum = trueConditions[0].size();
	
	if (currentPictureNum < 2)
	{
		thrower("ERROR: Something tried to remove the last picture!");
	}
	for (int pixelInc = 0; pixelInc < currentPixelNum; pixelInc++)
	{
		trueConditions[pixelInc].resize(currentPictureNum - 1);
		for (int postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
		{
			postSelectionConditions[postSelectionConditionInc][pixelInc].resize(currentPictureNum - 1);
		}
	}
}


int DataSet::getTruthCondition(int pixel, int picture)
{
	if (pixel >= trueConditions.size())
	{
		thrower("ERROR: tried to retrieve true condition from pixel that hasn't been allocated. Pixel: " + std::to_string(pixel));
	}
	if (picture >= trueConditions[pixel].size())
	{
		thrower("ERROR: tried to retrieve true condition from picture that hasn't been allocated. Picture: " + std::to_string(picture));
	}
	return trueConditions[pixel][picture];
}


int DataSet::getPostSelectionCondition(int conditionNumber, int pixel, int picture)
{
	if (conditionNumber > postSelectionConditions.size())
	{
		thrower("ERROR: tried to retrieve post selection condition from condition that hasn't been allocated. condition: " 
			+ std::to_string(conditionNumber));
	}
	if (pixel >= postSelectionConditions[conditionNumber].size())
	{
		thrower("ERROR: tried to retrieve post selection condition from pixel that hasn't been allocated. Pixel: " 
				 + std::to_string(pixel));
	}
	if (picture >= postSelectionConditions[conditionNumber][pixel].size())
	{
		thrower("ERROR: tried to retrieve post selection condition from picture that hasn't been allocated. Picture: " 
				 + std::to_string(picture));
	}
	return postSelectionConditions[conditionNumber][pixel][picture];
}


void DataSet::addPostSelectionCondition(int pixelNum, int pictureNum)
{
	// add condition, pixels, pictures
	postSelectionConditions.resize(postSelectionConditions.size() + 1);
	int back = postSelectionConditions.size() - 1;
	postSelectionConditions[back].resize(pixelNum);
	for (int pixelInc = 0; pixelInc < pixelNum; pixelInc++)
	{
		postSelectionConditions[back][pixelInc].resize(pictureNum);
	}
}

void DataSet::removePostSelectionCondition()
{
	if (postSelectionConditions.size() == 0)
	{
		thrower("ERROR: something tried to remove a post-selection condition when there weren't any!");
	}
	postSelectionConditions.resize(postSelectionConditions.size() - 1);
}

void DataSet::setDataCountsLocation(int maxPixel, int maxPicture, int pixel, int picture)
{
	dataCountsLocation.clear();
	dataCountsLocation.resize(maxPixel);
	for (int pixelCount = 0; pixelCount < maxPixel; pixelCount++)
	{
		dataCountsLocation[pixelCount].resize(maxPicture);
	}
	dataCountsLocation[pixel][picture] = true;
}

void DataSet::setPlotThisData(bool plotThisDataInput)
{
	plotThisData = plotThisDataInput;
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
	// avoid memory issues
	pixel = picture = 0;
	return -1;
}

std::string DataSet::getLegendText()
{
	return legendText;
}

void DataSet::resetPictureNumber(int pictureNumber)
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
}

void DataSet::resetPostSelectionConditionNumber(int conditionNumber)
{
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	postSelectionConditions.clear();
	postSelectionConditions.resize(conditionNumber);
}

void DataSet::resetPixelNumber(int pixelNumber)
{
	// trueConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	trueConditions.clear();
	trueConditions.resize(pixelNumber);
	// resize the true conditions.
	for (int pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
	{
		trueConditions[pixelInc].resize(pixelNumber);
	}
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (int conditionInc = 0; conditionInc < postSelectionConditions.size(); conditionInc++)
	{
		
		postSelectionConditions[conditionInc].clear();
		postSelectionConditions[conditionInc].resize(pixelNumber);
	}

	// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
	dataCountsLocation.clear();
	dataCountsLocation.resize(pixelNumber);
}


void DataSet::clear()
{
	// trueConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	trueConditions.clear();
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	postSelectionConditions.clear();
	dataCountsLocation.clear();
	// arbitrary.
	legendText = "";
	plotThisData = 0;
}


int DataSet::getFitType()
{
	return fitType;
}


int DataSet::getWhenToFit()
{
	return whenToFit;
}

void DataSet::setFitType(int newFitType)
{
	fitType = newFitType;
}
void DataSet::setWhenToFit(int newWhenToFit)
{
	whenToFit = newWhenToFit;
}

