// created by Mark O. Brown
#pragma once
#include "PictureControl.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "GeneralObjects/SmartDC.h"
#include "ConfigurationSystems/Version.h"
#include <array>
#include <fstream>
#include "PrimaryWindows/IChimeraQtWindow.h"

class PictureManager
{
	public:
		PictureManager ( bool histOption, std::string configurationFileDelim, bool autoscaleDefault,
			Qt::TransformationMode mode);
		RECT getPicArea ( );
		void updatePlotData ( );
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& configFile);
		void initialize( POINT& loc, CBrush* defaultGridBrush, int manWidth, int manHeight, IChimeraQtWindow* widget, 
						 int scaleFactor=50);
		void drawGrids();
		unsigned getNumberActive( );
		void setParameters( imageParameters parameters );
		void handleScroll( unsigned nSBCode, unsigned nPos, CScrollBar* scrollbar);
		void setPalletes(std::array<int, 4> palleteIds);
		void setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts );
		// draw pictures...
		void drawBitmap (Matrix<long> picData, std::pair<int, int> minMax, unsigned whichPicCtrl,
			std::vector<coordinate> analysisLocs, std::vector<atomGrid> grids, unsigned pictureNumber,
			bool includingAnalysisMarkers);
		void createPalettes( );
		void handleEditChange( unsigned id );
		void setAlwaysShowGrid(bool showOption);
		void redrawPictures(coordinate selectedLocation, std::vector<coordinate> analysisLocs,
							 std::vector<atomGrid> gridInfo, bool forceGrid, unsigned picNumber, QPainter& painter);
		void setNumberPicturesActive( int numberActive );
		coordinate getSelLocation();
		void setSinglePicture( imageParameters imageParams );
		void setMultiplePictures( imageParameters imageParams, unsigned numberActivePics );
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

