#pragma once
#include <vector>
#include <string>
#include "DataSet.h"

class PlottingInfo
{
	public:
		PlottingInfo();
		~PlottingInfo();
		int changeTitle(std::string newTitle);
		std::string getTitle();

		int changeYLabel(std::string newYLabel);
		std::string getYLabel();

		int changeFileName(std::string newFileName);
		std::string getFileName();

		int changeGeneralPlotType(std::string newPlotType);
		std::string getPlotType();

		int changeXAxis(std::string newXAxis);
		std::string getXAxis();
		
		int changeLegendText(int dataSet, std::string newLegend);
		std::string getLegendText(int dataSet);

		int addGroup();
		int removeAnalysisSet();
		int resetNumberOfAnalysisGroups(int groupNumber);
		

		int addPixel();
		int removePixel();
		int resetPixelNumber(int pixelNumber);
		int getPixelNumber();
		int setPixelIndex(int pixel, int group, int index);
		int getPixelIndex(int pixel, int group);

		int setGroupLocation(int pixel, int analysisSet, int row, int collumn);
		int getPixelLocation(int pixel, int analysisSet, int& row, int& collumn);
		std::vector<std::pair<int, int>> getAllPixelLocations();

		int addPicture();
		int removePicture();
		int resetPictureNumber(int pictureNumber);
		int getPictureNumber();

		int addDataSet();
		int removeDataSet();
		int resetDataSetNumber(int dataSetNumber);

		int setPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture, int trueConditionValue);
		int getPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture);
		int addPostSelectionCondition();
		int removePostSelectionCondition();
		int resetConditionNumber(int conditionNumber);
		int getConditionNumber();

		int setTruthCondition(int dataSetNumber, int pixel, int picture, int trueConditionValue);
		int getTruthCondition(int dataSetNumber, int pixel, int picture);

		int setDataCountsLocation(int dataSet, int pixel, int picture);
		int getDataCountsLocation(int dataSet, int& pixel, int& picture);

		int setPlotData(int dataSet, bool plotData);
		bool getPlotThisDataValue(int dataSet);

		std::string returnAllInfo();
		int savePlotInfo();

		int loadPlottingInfoFromFile(std::string fileLocation);

		int clear();

		int getPixelGroupNumber();
		int getDataSetNumber();

		int getFitOption(int dataSet);
		int setFitOption(int dataSet, int fitType);

		int getWhenToFit(int dataSet);
		int setWhenToFit(int dataSet, int whenToFit);


	private:
		// arbitrary
		std::string title;
		// arbitrary
		std::string yLabel;
		// analysisGroups[pixel #][pixel set][0] = row
		// analysisGroups[pixel #][pixel set][1] = collumn
		// analysisGroups[pixel #][pixel set][2] = pixel index (set durring plotting)
		std::vector<std::vector<std::array<int, 3>>> analysisGroups;
		// Contains information for each set of data to be plotted.
		std::vector<DataSet> dataSets;
		// arbitrary. Always goes to the same folder.
		std::string fileName;
		// grabbed from main code at "OK" Press.
		int numberOfPictures;
		int currentPixelNumber;
		int currentConditionNumber;
		// two options here.
		std::string xAxis;
		// three options here.
		std::string generalPlotType;
};