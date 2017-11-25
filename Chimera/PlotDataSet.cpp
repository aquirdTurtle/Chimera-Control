#include "stdafx.h"
#include "PlotDataSet.h"


PlotDataSet::PlotDataSet()
{
	resultConditions.resize(1);
	postSelectionConditions.resize(1);
	postSelectionConditions[0].resize(1);
	dataCountsLocation.resize(1);
	dataCountsLocation[0].resize(1);
}

int & PlotDataSet::prc( UINT pixel, UINT picture )
{
	if ( pixel > resultConditions.size( ) )
	{
		thrower( "pixel out of range for positive result condition access!" );
	}
	if ( picture > resultConditions[pixel].size( ) )
	{
		thrower( "picture out of range for positive result condition access!" );
	}
	return resultConditions[pixel][picture];
}
int PlotDataSet::prc( UINT pixel, UINT picture ) const
{
	if ( pixel > resultConditions.size( ) )
	{
		thrower( "pixel out of range for positive result condition access!" );
	}
	if ( picture > resultConditions[pixel].size( ) )
	{
		thrower( "picture out of range for positive result condition access!" );
	}
	return resultConditions[pixel][picture];
}


int PlotDataSet::psc( UINT condition, UINT pixel, UINT picture ) const
{
	if ( condition > postSelectionConditions.size() )
	{
		thrower( "condition out of range for post-selection condition access!" );
	}
	if ( pixel > postSelectionConditions[condition].size( ) )
	{
		thrower( "pixel out of range for post-selection condition access!" );
	}
	if ( picture > postSelectionConditions[condition][pixel].size( ) )
	{
		thrower( "picture out of range for post-selection condition access!" );
	}
	return postSelectionConditions[condition][pixel][picture];
}
int & PlotDataSet::psc( UINT condition, UINT pixel, UINT picture )
{
	if ( condition > postSelectionConditions.size( ) )
	{
		thrower( "condition out of range for post-selection condition access!" );
	}
	if ( pixel > postSelectionConditions[condition].size( ) )
	{
		thrower( "pixel out of range for post-selection condition access!" );
	}
	if ( picture > postSelectionConditions[condition][pixel].size( ) )
	{
		thrower( "picture out of range for post-selection condition access!" );
	}
	return postSelectionConditions[condition][pixel][picture];
}


UINT PlotDataSet::getPixelNumber( )
{
	return resultConditions.size( );
}

UINT PlotDataSet::getPictureNumber( )
{
	if ( resultConditions.size( ) == 0 )
	{
		thrower( "ERROR: result conditions is empty in get picture number!" );
	}
	return resultConditions[0].size( );
}

UINT PlotDataSet::getConditionNumber( )
{
	return postSelectionConditions.size( );
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
	UINT currentPixelNum = getPixelNumber( );
	// this should always be at least one large.
	UINT currentPictureNum = getPictureNumber( );
	for (UINT pixelInc = 0; pixelInc < currentPixelNum; pixelInc++)
	{
		resultConditions[pixelInc].resize(currentPictureNum + 1);
		dataCountsLocation[pixelInc].resize(currentPictureNum + 1);
		for ( auto& condition : postSelectionConditions )
		{
			condition[pixelInc].resize( currentPictureNum + 1 );
		}
	}
}

// picture number is used to initialize the new pixel to the correct size.
void PlotDataSet::addPixel( UINT pictureNumber)
{
	// get the current pixel number. This should be uniform across thes data structures.
	int currentPixelNum = getPixelNumber( );
	// all data sets have access to the same pixels, whether they actually use them for result values or post selection or not.
	resultConditions.resize(currentPixelNum + 1);
	resultConditions[currentPixelNum].resize(pictureNumber);
	dataCountsLocation.resize(currentPixelNum + 1);
	dataCountsLocation[currentPixelNum].resize(pictureNumber);
	for ( auto& condition : postSelectionConditions )
	{
		condition.resize(currentPixelNum + 1);
		// should I be resizing all pixels with a separate loop?
		condition[currentPixelNum].resize(pictureNumber); 
	}
}


void PlotDataSet::setResultCondition(UINT pixel, UINT picture, UINT resultCondition)
{
	if (resultCondition != -1 && resultCondition != 0 && resultCondition != 1)
	{
		thrower("ERROR: something attempted to set a result condition to bad value: " + str(resultCondition+1));
	}
	prc( pixel, picture ) = resultCondition;
}


void PlotDataSet::setPostSelectionCondition(UINT conditionNumber, UINT pixel, UINT picture, UINT postSelectionCondition)
{
	psc( conditionNumber, pixel, picture ) = postSelectionCondition;
}


void PlotDataSet::removePixel()
{
	// make sure there is a pixel to remove.
	UINT currentPixelNum = getPixelNumber( );
	if (currentPixelNum < 2)
	{
		thrower("ERROR: Something tried to remove the last pixel!");
	}
	resultConditions.resize(currentPixelNum - 1);
	for ( auto& condition : postSelectionConditions )
	{
		condition.resize( currentPixelNum - 1 );
	}
}


void PlotDataSet::removePicture()
{
	// make sure there is a picture to remove.
	// all data structures have access to the same number of pictures. Currently there is no to have one data structure work on 2 picture increments and one on
	// 3, for example.
	UINT currentPixelNum = getPixelNumber( );
	// this should always be at least one large.
	UINT currentPictureNum = getPictureNumber( );
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


int PlotDataSet::getPositiveResultCondition(UINT pixel, UINT picture)
{
	return prc(pixel, picture);
}


int PlotDataSet::getPostSelectionCondition(UINT conditionNumber, UINT pixel, UINT picture)
{
	return psc( conditionNumber, pixel, picture );
}


void PlotDataSet::addPostSelectionCondition( UINT pixelNum, UINT pictureNum )
{
	// add condition, pixels, pictures
	postSelectionConditions.resize( postSelectionConditions.size( ) + 1 );
	UINT back = postSelectionConditions.size( ) - 1;
	postSelectionConditions[back].resize( pixelNum );
	for (auto& pixel : postSelectionConditions[back] )
	{
		pixel.resize( pictureNum );
	}
}


void PlotDataSet::removePostSelectionCondition()
{
	if ( getConditionNumber( ) == 0)
	{
		thrower("ERROR: something tried to remove a post-selection condition when there weren't any!");
	}
	postSelectionConditions.resize(getConditionNumber() - 1);
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
	for (UINT pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++)
	{
		resultConditions[pixelInc].clear();
		resultConditions[pixelInc].resize(pictureNumber);
	}
	// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (auto& condition : postSelectionConditions )
	{
		for ( auto& pixel : condition )
		{
			pixel.clear( );
			pixel.resize( pictureNumber );
		}
	}
	for ( auto& pixel : dataCountsLocation )
	{
		pixel.clear( );
		pixel.resize( pictureNumber );
	}
}

void PlotDataSet::resetPostSelectionConditionNumber( UINT conditionNumber)
{
	postSelectionConditions.clear();
	postSelectionConditions.resize(conditionNumber);
}

void PlotDataSet::resetPixelNumber( UINT pixelNumber)
{
	UINT picNum = getPictureNumber( );
	resultConditions.clear();
	resultConditions.resize(pixelNumber);
	for (auto& pixel : resultConditions)
	{
		pixel.resize( picNum );
	}
	for ( auto& condition : postSelectionConditions )
	{
		condition.clear( );
		condition.resize( pixelNumber );
	}
	dataCountsLocation.clear();
	dataCountsLocation.resize(pixelNumber);
}


void PlotDataSet::clear()
{
	resultConditions.clear();
	postSelectionConditions.clear();
	dataCountsLocation.clear();
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

