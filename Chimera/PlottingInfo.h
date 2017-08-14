#pragma once
#include <vector>
#include <string>
//#include "DataAnalysisHandler.h"
#include "DataSet.h"

class PlottingInfo
{
	public:
		PlottingInfo(UINT picNumber);
		PlottingInfo(std::string fileName);
		~PlottingInfo();
		void changeTitle(std::string newTitle);
		std::string getTitle();

		void changeYLabel(std::string newYLabel);
		std::string getYLabel();

		void changeFileName(std::string newFileName);
		std::string getFileName();

		void changeGeneralPlotType(std::string newPlotType);
		std::string getPlotType();

		void changeXAxis(std::string newXAxis);
		std::string getXAxis();
		
		void changeLegendText(int dataSet, std::string newLegend);
		std::string getLegendText(int dataSet);

		void addGroup();
		void removeAnalysisSet();
		void resetNumberOfAnalysisGroups(int groupNumber);
		void setGroups(std::vector<std::pair<int, int>> locations);
		
		void addPixel();
		void removePixel();
		void resetPixelNumber(int pixelNumber);
		int getPixelNumber();
		void setPixelIndex(int pixel, int group, int index);
		int getPixelIndex(int pixel, int group);

		void setGroupLocation(int pixel, int analysisSet, int row, int collumn);
		void getPixelLocation(int pixel, int analysisSet, int& row, int& collumn);
		std::vector<std::pair<int, int>> getAllPixelLocations();

		void addPicture();
		void removePicture();
		void resetPictureNumber(int pictureNumber);
		int getPicNumber();

		void addDataSet();
		void removeDataSet();
		void resetDataSetNumber(int dataSetNumber);

		void setPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture, int trueConditionValue);
		int getPostSelectionCondition(int dataSetNumber, int conditionNumber, int pixel, int picture);
		void addPostSelectionCondition();
		void removePostSelectionCondition();
		void resetConditionNumber(int conditionNumber);
		int getConditionNumber();

		void setResultCondition(int dataSetNumber, int pixel, int picture, int trueConditionValue);
		int getResultCondition(int dataSetNumber, int pixel, int picture);

		void setDataCountsLocation(int dataSet, int pixel, int picture);
		void getDataCountsLocation(int dataSet, int& pixel, int& picture);

		void setPlotData(int dataSet, bool plotData);
		bool getPlotThisDataValue(int dataSet);

		std::string getAllSettingsString();
		void savePlotInfo();
		
		void loadPlottingInfoFromFile(std::string fileLocation);
		
		

		void clear();

		UINT getPixelGroupNumber();
		UINT getDataSetNumber();

		int getFitOption(int dataSet);
		void setFitOption(int dataSet, int fitType);

		int whenToFit(int dataSet);
		void setWhenToFit(int dataSet, int whenToFit);

		std::string getPrcSettingsString();
		std::string getPscSettingsString();

		static UINT getPicNumberFromFile(std::string fileAddress);
		static std::string getAllSettingsStringFromFile(std::string fileAddress);


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
		//int pixelNumber;
		// two options here.
		std::string xAxis;
		// three options here.
		std::string generalPlotType;
};