// created by Mark O. Brown
#pragma once
#include <string>
#include <vector>

// for plotting info
class PlotDataSet
{
	public:
		PlotDataSet();
		void initialize(unsigned conditionNumber, unsigned pixelNumber, unsigned pictureNumber);
		void changeLegendText(std::string newLegendText);
		std::string getLegendText();

		void addPicture();
		void removePicture();
		void resetPictureNumber( unsigned pictureNumber);

		void addPixel( unsigned pictureNumber);
		void removePixel();
		void resetPixelNumber(unsigned pixelNumber);

		void setResultCondition(unsigned picture, unsigned pixel, unsigned trueConditionValue);
		int getPositiveResultCondition(unsigned pixel, unsigned picture);
		void setDataCountsLocation(unsigned maxPixel, unsigned maxPicture, unsigned pixel, unsigned picture);
		
		void addPostSelectionCondition(unsigned pixelNum, unsigned pictureNum);
		void removePostSelectionCondition();
		void setPostSelectionCondition(unsigned conditionNumber, unsigned picture, unsigned pixel, unsigned postSelectionCondition);
		int getPostSelectionCondition(unsigned conditionNumber, unsigned pixel, unsigned picture);
		void resetPostSelectionConditionNumber(unsigned conditionNumber);

		int getDataCountsLocation(unsigned& pixel, unsigned& picture);
		void setPlotThisData(bool plotThisDataInput);
		bool getPlotThisDataValue();

		void setHistBinWidth( unsigned width );
		unsigned getHistBinWidth( );

		void clear();
		
		int getFitType();
		void setFitType(unsigned newFitType); 
		
		int getWhenToFit();
		void setWhenToFit(unsigned newWhenToFit);

	private:
		int prc( unsigned pixel, unsigned picture ) const;
		int & prc( unsigned pixel, unsigned picture );
		int psc( unsigned condition, unsigned pixel, unsigned picture ) const;
		int & psc( unsigned condition, unsigned pixel, unsigned picture );
		unsigned getPixelNumber( );
		unsigned getPictureNumber( );
		unsigned getConditionNumber( );

		// resultConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
		std::vector<std::vector<int> > resultConditions;
		// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
		std::vector<std::vector<std::vector<int> > > postSelectionConditions;
		// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
		std::vector<std::vector<bool> > dataCountsLocation;
		std::string legendText;
		bool plotThisData;
		int fitType;
		int whenToFit;
		unsigned histBinWidth=10;
};

