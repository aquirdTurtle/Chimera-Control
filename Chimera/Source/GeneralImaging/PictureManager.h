// created by Mark O. Brown
#pragma once
#include "PictureControl.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "GeneralObjects/SmartDC.h"
#include "ConfigurationSystems/Version.h"
#include <array>
#include <fstream>
#include "PrimaryWindows/IChimeraWindowWidget.h"

class PictureManager
{
	public:
		PictureManager ( bool histOption, std::string configurationFileDelim, bool autoscaleDefault );
		RECT getPicArea ( );
		void updatePlotData ( );
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& configFile);
		void initialize( POINT& loc, CBrush* defaultGridBrush, int manWidth, int manHeight, IChimeraWindowWidget* widget);
		void drawGrids();
		UINT getNumberActive( );
		void setParameters( imageParameters parameters );
		void handleScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar);
		void setPalletes(std::array<int, 4> palleteIds);
		void setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts );
		// draw pictures...
		void drawBitmap (Matrix<long> picData, std::pair<int, int> minMax, UINT whichPicCtrl,
			std::vector<coordinate> analysisLocs, std::vector<atomGrid> grids, UINT pictureNumber,
			bool includingAnalysisMarkers);
		void createPalettes( );
		void handleEditChange( UINT id );
		void setAlwaysShowGrid(bool showOption);
		void redrawPictures(coordinate selectedLocation, std::vector<coordinate> analysisLocs,
							 std::vector<atomGrid> gridInfo, bool forceGrid, UINT picNumber, QPainter& painter);
		void setNumberPicturesActive( int numberActive );
		coordinate getSelLocation();
		void setSinglePicture( imageParameters imageParams );
		void setMultiplePictures( imageParameters imageParams, UINT numberActivePics );
		void setAutoScalePicturesOption(bool autoScaleOption);
		void setSpecialLessThanMin(bool option);
		void setSpecialGreaterThanMax(bool option);
		void resetPictureStorage();
		const std::string configDelim;
	private:
		std::array<PictureControl, 4> pictures;
		std::array<QVector<QRgb>,4> palettes;
		QVector<QRgb> inferno, greys;
		CBrush* gridBrush;
		POINT picturesLocation;
		int picturesWidth;
		int picturesHeight;
		bool autoScalePictures;
		bool specialGreaterThanMax;
		bool specialLessThanMin;
		bool alwaysShowGrid;
};

