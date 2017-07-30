#pragma once
#include <string>
#include <vector>

// for plotting info
class DataSet
{
	public:
		DataSet();
		void initialize(int conditionNumber, int pixelNumber, int pictureNumber);
		void changeLegendText(std::string newLegendText);
		std::string getLegendText();

		void addPicture();
		void removePicture();
		void resetPictureNumber(int pictureNumber);

		void addPixel(int pictureNumber);
		void removePixel();
		void resetPixelNumber(int pixelNumber);

		void setTruthCondition(int picture, int pixel, int trueConditionValue);
		int getTruthCondition(int pixel, int picture);
		void setDataCountsLocation(int maxPixel, int maxPicture, int pixel, int picture);
		
		void addPostSelectionCondition(int pixelNum, int pictureNum);
		void removePostSelectionCondition();
		void setPostSelectionCondition(int conditionNumber, int picture, int pixel, int postSelectionCondition);
		int getPostSelectionCondition(int conditionNumber, int pixel, int picture);
		void resetPostSelectionConditionNumber(int conditionNumber);

		int getDataCountsLocation(int& pixel, int& picture);
		void setPlotThisData(bool plotThisDataInput);
		bool getPlotThisDataValue();
		void clear();
		
		int getFitType();
		void setFitType(int newFitType); 
		
		int getWhenToFit();
		void setWhenToFit(int newWhenToFit);

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