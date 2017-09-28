#pragma once
#include <string>
#include <vector>

// for plotting info
class PlotDataSet
{
	public:
		PlotDataSet();
		void initialize(UINT conditionNumber, UINT pixelNumber, UINT pictureNumber);
		void changeLegendText(std::string newLegendText);
		std::string getLegendText();

		void addPicture();
		void removePicture();
		void resetPictureNumber( UINT pictureNumber);

		void addPixel( UINT pictureNumber);
		void removePixel();
		void resetPixelNumber(UINT pixelNumber);

		void setResultCondition(UINT picture, UINT pixel, UINT trueConditionValue);
		int getTruthCondition(UINT pixel, UINT picture);
		void setDataCountsLocation(UINT maxPixel, UINT maxPicture, UINT pixel, UINT picture);
		
		void addPostSelectionCondition(UINT pixelNum, UINT pictureNum);
		void removePostSelectionCondition();
		void setPostSelectionCondition(UINT conditionNumber, UINT picture, UINT pixel, UINT postSelectionCondition);
		int getPostSelectionCondition(UINT conditionNumber, UINT pixel, UINT picture);
		void resetPostSelectionConditionNumber(UINT conditionNumber);

		int getDataCountsLocation(UINT& pixel, UINT& picture);
		void setPlotThisData(bool plotThisDataInput);
		bool getPlotThisDataValue();
		void clear();
		
		int getFitType();
		void setFitType(UINT newFitType); 
		
		int getWhenToFit();
		void setWhenToFit(UINT newWhenToFit);

	private:
		// resultConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
		std::vector<std::vector<int> > resultConditions;
		// postSelectionConditions[Condition#][Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
		std::vector<std::vector<std::vector<int> > > postSelectionConditions;
		// dataCountsLocation[pixel#][picture#] = this pixel is used or not.
		std::vector<std::vector<bool> > dataCountsLocation;
		// arbitrary.
		std::string legendText;
		// 
		bool plotThisData;
		// 
		int fitType;
		// 
		int whenToFit;
		// 
};