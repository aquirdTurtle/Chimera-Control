#pragma once
#include <vector>
#include <string>
#include "PlotDataSet.h"
#include "coordinate.h"

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
		
		void changeLegendText( UINT dataSet, std::string newLegend);
		std::string getLegendText( UINT dataSet);

		void addGroup();
		void removeAnalysisSet();
		void resetNumberOfAnalysisGroups( UINT groupNumber);
		void setGroups(std::vector<coordinate> locations);
		
		void addPixel();
		void removePixel();
		void resetPixelNumber( UINT pixelNumber);
		UINT getPixelNumber();
		void setPixelIndex( UINT pixel, UINT group, UINT index);
		UINT getPixelIndex( UINT pixel, UINT group);

		void setGroupLocation( UINT pixel, UINT analysisSet, UINT row, UINT collumn);
		void getPixelLocation( UINT pixel, UINT analysisSet, UINT& row, UINT& collumn);
		std::vector<std::pair<UINT, UINT>> getAllPixelLocations();

		void addPicture();
		void removePicture();
		void resetPictureNumber( UINT pictureNumber);
		UINT getPicNumber();

		void addDataSet();
		void removeDataSet();
		void resetDataSetNumber( UINT dataSetNumber);

		void setPostSelectionCondition( UINT dataSetNumber, UINT conditionNumber, UINT pixel, UINT picture, UINT trueConditionValue);
		UINT getPostSelectionCondition( UINT dataSetNumber, UINT conditionNumber, UINT pixel, UINT picture);
		void addPostSelectionCondition();
		void removePostSelectionCondition();
		void resetConditionNumber( UINT conditionNumber);
		UINT getConditionNumber();

		void setResultCondition( UINT dataSetNumber, UINT pixel, UINT picture, UINT trueConditionValue);
		UINT getResultCondition( UINT dataSetNumber, UINT pixel, UINT picture);

		void setDataCountsLocation( UINT dataSet, UINT pixel, UINT picture);
		void getDataCountsLocation( UINT dataSet, UINT& pixel, UINT& picture);

		void setPlotData( UINT dataSet, bool plotData);
		bool getPlotThisDataValue( UINT dataSet);

		std::string getAllSettingsString();
		void savePlotInfo();
		
		void loadPlottingInfoFromFile(std::string fileLocation);
		
		

		void clear();

		UINT getPixelGroupNumber();
		UINT getDataSetNumber();

		UINT getFitOption( UINT dataSet);
		void setFitOption( UINT dataSet, UINT fitType);

		UINT whenToFit( UINT dataSet);
		void setWhenToFit( UINT dataSet, UINT whenToFit);

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
		std::vector<PlotDataSet> dataSets;
		// arbitrary. Always goes to the same folder.
		std::string fileName;
		// grabbed from main code at "OK" Press.
		UINT numberOfPictures;
		UINT currentPixelNumber;
		UINT currentConditionNumber;
		// two options here.
		std::string xAxis;
		// three options here.
		std::string generalPlotType;
};