#pragma once
#include <string>
#include <vector>

class DataSet
{
	public:
		DataSet();
		~DataSet();
		int initialize(int conditionNumber, int pixelNumber, int pictureNumber);
		int changeLegendText(std::string newLegendText);
		std::string getLegendText();

		int addPicture();
		int removePicture();
		int resetPictureNumber(int pictureNumber);

		int addPixel(int pictureNumber);
		int removePixel();
		int resetPixelNumber(int pixelNumber);

		int setTruthCondition(int picture, int pixel, int trueConditionValue);
		int getTruthCondition(int pixel, int picture);
		int setDataCountsLocation(int maxPixel, int maxPicture, int pixel, int picture);
		
		int addPostSelectionCondition(int pixelNum, int pictureNum);
		int removePostSelectionCondition();
		int setPostSelectionCondition(int conditionNumber, int picture, int pixel, int postSelectionCondition);
		int getPostSelectionCondition(int conditionNumber, int pixel, int picture);
		int resetPostSelectionConditionNumber(int conditionNumber);

		int getDataCountsLocation(int& pixel, int& picture);
		int setPlotThisData(bool plotThisDataInput);
		bool getPlotThisDataValue();
		int clear();
		
		int getFitType();
		int setFitType(int newFitType); 
		
		int getWhenToFit();
		int setWhenToFit(int newWhenToFit);

	private:
		// trueConditions[Pixel#][Picture#] = (1 if atom present selected; -1 if no atom selected, 0 if nothing selected)
		std::vector<std::vector<int> > trueConditions;
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