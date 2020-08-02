// created by Mark O. Brown
#pragma once
#include <vector>
#include <string>
#include "PlotDataSet.h"
#include "GeneralObjects/coordinate.h"


struct analysisGroupLocation
{
	int row;
	int col;
	int pixelIndex;
};


class PlottingInfo
{
	public:
		PlottingInfo(unsigned picNumber=1);
		PlottingInfo(std::string fileName);
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
		
		void changeLegendText( unsigned dataSet, std::string newLegend);
		std::string getLegendText( unsigned dataSet);

		void addGroup();
		void removeAnalysisSet();
		void resetNumberOfAnalysisGroups( unsigned groupNumber);
		void setGroups(std::vector<coordinate> locations);
		
		void addPixel();
		void removePixel();
		void resetPixelNumber( unsigned pixelNumber);
		unsigned getPixelNumber();
		void setPixelIndex( unsigned pixel, unsigned group, unsigned index);
		unsigned getPixelIndex( unsigned pixel, unsigned group);

		void setGroupLocation( unsigned pixel, unsigned analysisSet, unsigned row, unsigned collumn);
		void getPixelLocation( unsigned pixel, unsigned analysisSet, unsigned& row, unsigned& collumn);
		std::vector<std::pair<unsigned, unsigned>> getAllPixelLocations();

		void addPicture();
		void removePicture();
		void resetPictureNumber( unsigned pictureNumber);
		unsigned getPicNumber();

		void addDataSet();
		void removeDataSet();
		void resetDataSetNumber( unsigned dataSetNumber);

		void setPostSelCondition( unsigned dataSetNumber, unsigned conditionNumber, unsigned pixel, unsigned picture, unsigned trueConditionValue);
		unsigned getPostSelectionCondition( unsigned dataSetNumber, unsigned conditionNumber, unsigned pixel, unsigned picture);
		void addPostSelectionCondition();
		void removePostSelectionCondition();
		void resetConditionNumber( unsigned conditionNumber);
		unsigned getConditionNumber();

		void setResultCondition( unsigned dataSetNumber, unsigned pixel, unsigned picture, unsigned trueConditionValue);
		unsigned getResultCondition( unsigned dataSetNumber, unsigned pixel, unsigned picture);

		void setDataCountsLocation( unsigned dataSet, unsigned pixel, unsigned picture);
		void getDataCountsLocation( unsigned dataSet, unsigned& pixel, unsigned& picture);

		void setDataSetHistBinWidth( unsigned dataSet, unsigned width );
		unsigned getDataSetHistBinWidth( unsigned dataSet );

		void setPlotData( unsigned dataSet, bool plotData);
		bool getPlotThisDataValue( unsigned dataSet);

		std::string getAllSettingsString();
		void savePlotInfo();
		void loadPlottingInfoFromFile(std::string fileLocation);
		
		void clear();

		unsigned getPixelGroupNumber();
		unsigned getDataSetNumber();

		unsigned getFitOption( unsigned dataSet);
		void setFitOption( unsigned dataSet, unsigned fitType);

		unsigned whenToFit( unsigned dataSet);
		void setWhenToFit( unsigned dataSet, unsigned whenToFit);

		std::string getPrcSettingsString();
		std::string getPscSettingsString();

		static unsigned getPicNumberFromFile(std::string fileAddress);
		static std::string getAllSettingsStringFromFile(std::string fileAddress);

		analysisGroupLocation& groupInfo( unsigned pixelNumber, unsigned pixelSet );
		analysisGroupLocation  groupInfo( unsigned pixelNumber, unsigned pixelSet ) const;

	private:
		// Versions of the plot save file system. 
		// Version 1.x refers to files that were outputted with no version number
		const unsigned versionMajor = 2;
		const unsigned versionMinor = 1;
		std::string title;
		std::string yLabel;
		// analysisGroups[pixel #][pixel set]
		std::vector<std::vector<analysisGroupLocation>> analysisGroups;
		// Contains information for each set of data to be plotted.
		std::vector<PlotDataSet> dataSets;
		// arbitrary. Always goes to the same folder.
		std::string fileName;
		// grabbed from main code at "OK" Press.
		unsigned numberOfPictures;
		unsigned currentPixelNumber;
		unsigned currentConditionNumber;
		// two options here.
		std::string xAxis;
		// three options here.
		std::string generalPlotType;
};

Q_DECLARE_METATYPE (PlottingInfo)
