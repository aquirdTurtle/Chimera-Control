#include "stdafx.h"
#include "PlotDataSet.h"


/// All PlotDataSet() class public functions.
PlotDataSet::PlotDataSet()
{
	resultConditions.clear();
	resultConditions.resize(1);
	postSelectionConditions.clear();
	postSelectionConditions.resize(1);
	postSelectionConditions[0].resize(1);
	dataCountsLocation.resize(1);
	dataCountsLocation[0].resize(1);
}


void PlotDataSet::initialize(UINT conditionNumber, UINT pixelNumber, UINT pictureNumber)
{
	resultConditions.resize(pixelNumber);
	dataCountsLocation.resize(pixelNumber);
	for (UINT pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
	{
		resultConditions[pixelInc].resize(pictureNumber);
		dataCountsLocation[pixelInc].resize(pictureNumber);
	}
	
	postSelectionConditions.resize(conditionNumber);
	for (UINT conditionInc = 0; conditionInc < conditionNumber; conditionInc++)
	{
		postSelectionConditions[conditionInc].resize(pixelNumber);
		for (UINT pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
		{
			postSelectionConditions[conditionInc][pixelInc].resize(pictureNumber);
		}
	}
}



void PlotDataSet::setHistBinWidth( UINT width )
{
	if ( width < 1 || width > 1e6 )
	{
		thrower( "ERROR: Bad value (" + str( width ) + ") for plotting data set histogram bin width!" );
	}
	histBinWidth = width;
}

UINT PlotDataSet::getHistBinWidth( )
{
	return histBinWidth;
}

void PlotDataSet::changeLegendText(std::string newLegendText)
{
	legendText = newLegendText;
}


void PlotDataSet::addPicture()
{
	// all data structures have access to the same number of pictures. Currently there is no to have one data structure work on 2 picture increments and one on
	// 3, for example.
	UINT currentPixelNum = resultConditions.size();
	// this should always be at least one large.
	UINT currentPictureNum = resultConditions[0].size();
	for (UINT pixelInc = 0; pixelInc < currentPixelNum; pixelInc++)
	{
		resultConditions[pixelInc].resize(currentPictureNum + 1);
		dataCountsLocation[pixelInc].resize(currentPictureNum + 1);
		for (UINT postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
		{
			postSelectionConditions[postSelectionConditionInc][pixelInc].resize(currentPictureNum + 1);
		}

	}
}

// picture number is used to initialize the new pixel to the correct size.
void PlotDataSet::addPixel( UINT pictureNumber)
{
	// get the current pixel number. This should be uniform across thes data structures.
	int currentPixelNum = resultConditions.size();
	// all data sets have access to the same pixels, whether they actually use them for result values or post selection or not.
	resultConditions.resize(currentPixelNum + 1);
	resultConditions[currentPixelNum].resize(pictureNumber);
	dataCountsLocation.resize(currentPixelNum + 1);
	dataCountsLocation[currentPixelNum].resize(pictureNumber);

	for (UINT postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
	{
		postSelectionConditions[postSelectionConditionInc].resize(currentPixelNum + 1);
		postSelectionConditions[postSelectionConditionInc][currentPixelNum].resize(pictureNumber); ///
	}
}


void PlotDataSet::setResultCondition(UINT pixel, UINT picture, UINT resultCondition)
{
	if (resultCondition != -1 && resultCondition != 0 && resultCondition != 1)
	{
		thrower("ERROR: something attempted to set a result condition to bad value: " + str(resultCondition+1));
	}
	if (pixel >= resultConditions.size())
	{
		thrower("ERROR: tried to set the result condition of a pixel that isn't loaded into the condition vector. "
				"Pixel number: " + str(pixel+1));
	}
	if (picture >= resultConditions[pixel].size())
	{
		thrower("ERROR: tried to set the result condition of a picture that isn't loaded into the condition vector for"
				" the given pixel. Picture number: " + str(picture+1));
	}
	resultConditions[pixel][picture] = resultCondition;
}

void PlotDataSet::setPostSelectionCondition(UINT conditionNumber, UINT pixel, UINT picture, UINT postSelectionCondition)
{

	if (conditionNumber >= postSelectionConditions.size())
	{
		thrower("ERROR: tried to set the post selection condition of a condition number that isn't loaded into the condition vector. "
				 "Condition number: " + str(conditionNumber+1));

	}
	if (pixel >= postSelectionConditions[conditionNumber].size())
	{
		thrower("ERROR: tried to set the post selection condition of a pixel that isn't loaded into the condition vector. Pixel "
						"number: " + str(pixel+1));
	}
	if (picture >= postSelectionConditions[conditionNumber][pixel].size())
	{
		thrower("ERROR: tried to set the post selection condition of a picture that isn't loaded into the condition "
				"vector for the given pixel. Picture number: " + str(picture+1));
	}
	postSelectionConditions[conditionNumber][pixel][picture] = postSelectionCondition;
}


void PlotDataSet::removePixel()
{
	// make sure there is a pixel to remove.
	size_t currentPixelNum = resultConditions.size();
	if (currentPixelNum < 2)
	{
		thrower("ERROR: Something tried to remove the last pixel!");
	}
	resultConditions.resize(currentPixelNum - 1);
	for (UINT postSelectionConditionInc = 0; postSelectionConditionInc < postSelectionConditions.size(); postSelectionConditionInc++)
	{
		postSelectionConditions[postSelectionConditionInc].resize(currentPixelNum - 1);
	}
}


void PlotDataSet::removePicture()
{
	// make sure there is a picture to remove.
	// all data structures have access to the same number of pictures. Currently there is no to have one data structure work on 2 picture increments and one on
	// 3, for example.
	size_t currentPixelNum = resultConditions.size();
	// this should always be at least one large.
	size_t currentPictureNum = resultConditions[0].size();	
	if (currentPictureNum < 2)
	{
		thrower("ERROR: Something tried to remove the last picture!");
	}
	for (UINT pixelInc : range(currentPixelNum))
	{
		resultConditions[pixelInc].resize(currentPictureNum - 1);
		for (auto& condition : postSelectionConditions)
		{
			condition[pixelInc].resize(currentPictureNum - 1);
		}
	}
}


int PlotDataSet::getTruthCondition(UINT pixel, UINT picture)
{
	if (pixel >= resultConditions.size())
	{
		thrower("ERROR: tried to retrieve result condition from pixel that hasn't been allocated. Pixel: "
				 + str(pixel+1));
	}
	if (picture >= resultConditions[pixel].size())
	{
		thrower("ERROR: tried to retrieve result condition from picture that hasn't been allocated. Picture: " 
				+ str(picture+1));
	}
	return resultConditions[pixel][picture];
}


int PlotDataSet::getPostSelectionCondition(UINT conditionNumber, UINT pixel, UINT picture)
{
	if (conditionNumber >= postSelectionConditions.size())
	{
		thrower("ERROR: tried to retrieve post selection condition from condition that hasn't been allocated. condition: " 
			+ str(conditionNumber));
	}
	if (pixel >= postSelectionConditions[conditionNumber].size())
	{
		thrower("ERROR: tried to retrieve post selection condition from a pixel that hasn't been allocated. Pixel: " 
				 + str(pixel));
	}
	if (picture >= postSelectionConditions[conditionNumber][pixel].size())
	{
		thrower("ERROR: tried to retrieve post selection condition from picture that hasn't been allocated. Picture: " 
				 + str(picture));
	}
	return postSelectionConditions[conditionNumber][pixel][picture];
}


void PlotDataSet::addPostSelectionCondition( UINT pixelNum, UINT pictureNum )
{
	// add condition, pixels, pictures
	postSelectionConditions.resize( postSelectionConditions.size( ) + 1 );
	size_t back = postSelectionConditions.size( ) - 1;
	postSelectionConditions[back].resize( pixelNum );
	for ( UINT pixelInc : range( pixelNum ) )
	{
		postSelectionConditions[back][pixelInc].resize( pictureNum );
	}
}


void PlotDataSet::removePostSelectionCondition()
{
	if (postSelectionConditions.size() == 0)
	{
		thrower("ERROR: something tried to remove a post-selection condition when there weren't any!");
	}
	postSelectionConditions.resize(postSelectionConditions.size() - 1);
}


void PlotDataSet::setDataCountsLocation( UINT maxPixel, UINT maxPicture, UINT pixel, UINT picture)
{
	dataCountsLocation.clear();
	dataCountsLocation.resize(maxPixel);
	for (UINT pixelCount = 0; pixelCount < maxPixel; pixelCount++)
	{
		dataCountsLocation[pixelCount].resize(maxPicture);
	}
	dataCountsLocation[pixel][picture] = true;
}


void PlotDataSet::setPlotThisData(bool plotThisDataInput)
{
	plotThisData = plotThisDataInput;
}


bool PlotDataSet::getPlotThisDataValue()
{
	return plotThisData;
}


int PlotDataSet::getDataCountsLocation( UINT& pixel, UINT& picture)
{
	for (UINT pixelInc = 0; pixelInc < dataCountsLocation.size(); pixelInc++)
	{
		for (UINT pictureInc = 0; pictureInc < dataCountsLocation[pixelInc].size(); pictureInc++)
		{
			if (dataCountsLocation[pixelInc][pictureInc] )
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


std::string PlotDataSet::getLegendText()
{
	return legendText;
}


void PlotDataSet::resetPictureNumber( UINT pictureNumber)
{
	// resultConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (UINT pixelInc = 0; pixelInc < resultConditions.size(); pixelInc++)
	{
		resultConditions[pixelInc].clear();
		resultConditions[pixelInc].resize(pictureNumber);
	}
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (UINT conditionInc = 0; conditionInc < postSelectionConditions.size(); conditionInc++)
	{
		for (UINT pixelInc = 0; pixelInc < postSelectionConditions[conditionInc].size(); pixelInc++)
		{
			postSelectionConditions[conditionInc][pixelInc].clear();
			postSelectionConditions[conditionInc][pixelInc].resize(pictureNumber);
		}
	}

	// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
	for (UINT pixelInc = 0; pixelInc < dataCountsLocation.size(); pixelInc++)
	{
		dataCountsLocation[pixelInc].clear();
		dataCountsLocation[pixelInc].resize(pictureNumber);
	}
}

void PlotDataSet::resetPostSelectionConditionNumber( UINT conditionNumber)
{
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	postSelectionConditions.clear();
	postSelectionConditions.resize(conditionNumber);
}

void PlotDataSet::resetPixelNumber( UINT pixelNumber)
{
	// resultConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	resultConditions.clear();
	resultConditions.resize(pixelNumber);
	// resize the result conditions.
	for (UINT pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
	{
		resultConditions[pixelInc].resize(pixelNumber);
	}
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (UINT conditionInc = 0; conditionInc < postSelectionConditions.size(); conditionInc++)
	{
		postSelectionConditions[conditionInc].clear();
		postSelectionConditions[conditionInc].resize(pixelNumber);
	}

	// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
	dataCountsLocation.clear();
	dataCountsLocation.resize(pixelNumber);
}


void PlotDataSet::clear()
{
	// resultConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	resultConditions.clear();
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	postSelectionConditions.clear();
	dataCountsLocation.clear();
	// arbitrary.
	legendText = "";
	plotThisData = 0;
}


int PlotDataSet::getFitType()
{
	return fitType;
}


int PlotDataSet::getWhenToFit()
{
	return whenToFit;
}

void PlotDataSet::setFitType( UINT newFitType)
{
	fitType = newFitType;
}
void PlotDataSet::setWhenToFit( UINT newWhenToFit)
{
	whenToFit = newWhenToFit;
}

