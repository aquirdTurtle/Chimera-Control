// created by Mark O. Brown
#pragma once
#include "PictureControl.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "GeneralObjects/SmartDC.h"
#include "ConfigurationSystems/Version.h"
#include <array>
#include <fstream>

class PictureManager
{
	public:
		PictureManager ( bool histOption, std::string configurationFileDelim, bool autoscaleDefault );
		RECT getPicArea ( );
		void updatePlotData ( );
		void handleMouse( CPoint point );
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& configFile);
		void initialize( POINT& loc, CWnd* parent, int& id, CBrush* defaultGridBrush, int manWidth, int manHeight,
						 std::array<UINT, 8> minMaxEdits,
						 std::vector<Gdiplus::Pen*> graphPens = std::vector<Gdiplus::Pen*> ( ), CFont* font = NULL,
						 std::vector<Gdiplus::SolidBrush*> graphBrushes = std::vector<Gdiplus::SolidBrush*> ( ) );
		void refreshBackgrounds(CDC* easel);
		void drawGrids(CDC* easel);
		UINT getNumberActive( );
		void setParameters( imageParameters parameters );
		void rearrange( int width, int height, fontMap fonts );
		void handleScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar, CDC* sdc );
		void drawBackgrounds(CDC* easel);
		void setPalletes(std::array<int, 4> palleteIds);
		void setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts );
		// draw pictures...
		//void drawPicture(CDC* deviceContext, int pictureNumber, std::vector<long> picData,
		//				  std::pair<UINT, UINT> minMaxPair );
		void drawBitmap (CDC* deviceContext, Matrix<long> picData, std::pair<int, int> minMax, UINT whichPicCtrl);
		void drawDongles(CDC* dc, coordinate selectedLocation, std::vector<coordinate> analysisLocs,
						  std::vector<atomGrid> gridInfo, UINT pictureNumber, bool includingAnalysisMarkers=true );
		void createPalettes(CDC* dc );
		void handleEditChange( UINT id );
		void setAlwaysShowGrid(bool showOption, CDC* easel);
		void redrawPictures(CDC* easel, coordinate selectedLocation, std::vector<coordinate> analysisLocs,
							 std::vector<atomGrid> gridInfo, bool forceGrid, UINT picNumber );
		void setPictureSliders(CWnd* parent);
		void setNumberPicturesActive( int numberActive );
		coordinate getClickLocation(CPoint clickLocation);
		void setSinglePicture( CWnd* parent, imageParameters imageParams );
		void setMultiplePictures( CWnd* parent, imageParameters imageParams, UINT numberActivePics );
		void setAutoScalePicturesOption(bool autoScaleOption);
		void setSpecialLessThanMin(bool option);
		void setSpecialGreaterThanMax(bool option);
		void resetPictureStorage();
		void paint (CDC* sdc, CRect size, CBrush* bgdBrush );
		const std::string configDelim;
	private:
		std::array<PictureControl, 4> pictures;
		std::array<HPALETTE, 4> palettes;
		CBrush* gridBrush;
		POINT picturesLocation;
		int picturesWidth;
		int picturesHeight;
		bool autoScalePictures;
		bool specialGreaterThanMax;
		bool specialLessThanMin;
		bool alwaysShowGrid;
};

