// created by Mark O. Brown
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

int & PlotDataSet::prc( unsigned pixel, unsigned picture )
{
	if ( pixel > resultConditions.size( ) )
	{
		thrower ( "pixel out of range for positive result condition access!" );
	}
	if ( picture > resultConditions[pixel].size( ) )
	{
		thrower ( "picture out of range for positive result condition access!" );
	}
	return resultConditions[pixel][picture];
}
int PlotDataSet::prc( unsigned pixel, unsigned picture ) const
{
	if ( pixel > resultConditions.size( ) )
	{
		thrower ( "pixel out of range for positive result condition access!" );
	}
	if ( picture > resultConditions[pixel].size( ) )
	{
		thrower ( "picture out of range for positive result condition access!" );
	}
	return resultConditions[pixel][picture];
}


int PlotDataSet::psc( unsigned condition, unsigned pixel, unsigned picture ) const
{
	if ( condition > postSelectionConditions.size() )
	{
		thrower ( "condition out of range for post-selection condition access!" );
	}
	if ( pixel > postSelectionConditions[condition].size( ) )
	{
		thrower ( "pixel out of range for post-selection condition access!" );
	}
	if ( picture > postSelectionConditions[condition][pixel].size( ) )
	{
		thrower ( "picture out of range for post-selection condition access!" );
	}
	return postSelectionConditions[condition][pixel][picture];
}
int & PlotDataSet::psc( unsigned condition, unsigned pixel, unsigned picture )
{
	if ( condition > postSelectionConditions.size( ) )
	{
		thrower ( "condition out of range for post-selection condition access!" );
	}
	if ( pixel > postSelectionConditions[condition].size( ) )
	{
		thrower ( "pixel out of range for post-selection condition access!" );
	}
	if ( picture > postSelectionConditions[condition][pixel].size( ) )
	{
		thrower ( "picture out of range for post-selection condition access!" );
	}
	return postSelectionConditions[condition][pixel][picture];
}


unsigned PlotDataSet::getPixelNumber( )
{
	return resultConditions.size( );
}

unsigned PlotDataSet::getPictureNumber( )
{
	if ( resultConditions.size( ) == 0 )
	{
		thrower ( "ERROR: result conditions is empty in get picture number!" );
	}
	return resultConditions[0].size( );
}

unsigned PlotDataSet::getConditionNumber( )
{
	return postSelectionConditions.size( );
}

void PlotDataSet::initialize(unsigned conditionNumber, unsigned pixelNumber, unsigned pictureNumber)
{
	resultConditions.resize(pixelNumber);
	dataCountsLocation.resize(pixelNumber);
	for (unsigned pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
	{
		resultConditions[pixelInc].resize(pictureNumber);
		dataCountsLocation[pixelInc].resize(pictureNumber);
	}
	postSelectionConditions.resize(conditionNumber);
	for (unsigned conditionInc = 0; conditionInc < conditionNumber; conditionInc++)
	{
		postSelectionConditions[conditionInc].resize(pixelNumber);
		for (unsigned pixelInc = 0; pixelInc < pixelNumber; pixelInc++)
		{
			postSelectionConditions[conditionInc][pixelInc].resize(pictureNumber);
		}
	}
}



void PlotDataSet::setHistBinWidth( unsigned width )
{
	if ( width < 1 || width > 1e6 )
	{
		thrower ( "ERROR: Bad value (" + str( width ) + ") for plotting data set histogram bin width!" );
	}
	histBinWidth = width;
}

unsigned PlotDataSet::getHistBinWidth( )
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
	unsigned currentPixelNum = getPixelNumber( );
	// this should always be at least one large.
	unsigned currentPictureNum = getPictureNumber( );
	for (unsigned pixelInc = 0; pixelInc < currentPixelNum; pixelInc++)
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
void PlotDataSet::addPixel( unsigned pictureNumber)
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


void PlotDataSet::setResultCondition(unsigned pixel, unsigned picture, unsigned resultCondition)
{
	if (resultCondition != -1 && resultCondition != 0 && resultCondition != 1)
	{
		thrower ("ERROR: something attempted to set a result condition to bad value: " + str(resultCondition+1));
	}
	prc( pixel, picture ) = resultCondition;
}


void PlotDataSet::setPostSelectionCondition(unsigned conditionNumber, unsigned pixel, unsigned picture, unsigned postSelectionCondition)
{
	psc( conditionNumber, pixel, picture ) = postSelectionCondition;
}


void PlotDataSet::removePixel()
{
	// make sure there is a pixel to remove.
	unsigned currentPixelNum = getPixelNumber( );
	if (currentPixelNum < 2)
	{
		thrower ("ERROR: Something tried to remove the last pixel!");
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
	unsigned currentPixelNum = getPixelNumber( );
	// this should always be at least one large.
	unsigned currentPictureNum = getPictureNumber( );
	if (currentPictureNum < 2)
	{
		thrower ("ERROR: Something tried to remove the last picture!");
	}
	for (unsigned pixelInc : range(currentPixelNum))
	{
		resultConditions[pixelInc].resize(currentPictureNum - 1);
		for (auto& condition : postSelectionConditions)
		{
			condition[pixelInc].resize(currentPictureNum - 1);
		}
	}
}


int PlotDataSet::getPositiveResultCondition(unsigned pixel, unsigned picture)
{
	return prc(pixel, picture);
}


int PlotDataSet::getPostSelectionCondition(unsigned conditionNumber, unsigned pixel, unsigned picture)
{
	return psc( conditionNumber, pixel, picture );
}


void PlotDataSet::addPostSelectionCondition( unsigned pixelNum, unsigned pictureNum )
{
	// add condition, pixels, pictures
	postSelectionConditions.resize( postSelectionConditions.size( ) + 1 );
	unsigned back = postSelectionConditions.size( ) - 1;
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
		thrower ("ERROR: something tried to remove a post-selection condition when there weren't any!");
	}
	postSelectionConditions.resize(getConditionNumber() - 1);
}


void PlotDataSet::setDataCountsLocation( unsigned maxPixel, unsigned maxPicture, unsigned pixel, unsigned picture)
{
	dataCountsLocation.clear();
	dataCountsLocation.resize(maxPixel);
	for (unsigned pixelCount = 0; pixelCount < maxPixel; pixelCount++)
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


int PlotDataSet::getDataCountsLocation( unsigned& pixel, unsigned& picture)
{
	for (unsigned pixelInc = 0; pixelInc < dataCountsLocation.size(); pixelInc++)
	{
		for (unsigned pictureInc = 0; pictureInc < dataCountsLocation[pixelInc].size(); pictureInc++)
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


void PlotDataSet::resetPictureNumber( unsigned pictureNumber)
{
	// resultConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
	for (unsigned pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++)
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

void PlotDataSet::resetPostSelectionConditionNumber( unsigned conditionNumber)
{
	postSelectionConditions.clear();
	postSelectionConditions.resize(conditionNumber);
}

void PlotDataSet::resetPixelNumber( unsigned pixelNumber)
{
	unsigned picNum = getPictureNumber( );
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

void PlotDataSet::setFitType( unsigned newFitType)
{
	fitType = newFitType;
}
void PlotDataSet::setWhenToFit( unsigned newWhenToFit)
{
	whenToFit = newWhenToFit;
}

