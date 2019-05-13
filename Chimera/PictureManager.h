// created by Mark O. Brown
#pragma once
#include "PictureControl.h"
#include "atomGrid.h"
#include <array>
#include <fstream>

class PictureManager
{
	public:
		PictureManager ( bool histOption, std::string configurationFileDelim );
		RECT getPicArea ( );
		void updatePlotData ( );
		void handleMouse( CPoint point );
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& configFile, Version ver );
		void initialize( POINT& loc, CWnd* parent, int& id, CBrush* defaultGridBrush, int manWidth, int manHeight,
						 std::array<UINT, 8> minMaxEdits,
						 std::vector<Gdiplus::Pen*> graphPens = std::vector<Gdiplus::Pen*> ( ), CFont* font = NULL,
						 std::vector<Gdiplus::SolidBrush*> graphBrushes = std::vector<Gdiplus::SolidBrush*> ( ) );
		void refreshBackgrounds(CDC* easel);
		void drawGrids(CDC* easel);
		UINT getNumberActive( );
		void setParameters( imageParameters parameters );
		void rearrange( int width, int height, fontMap fonts );
		void handleScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar, CDC* cdc );
		void drawBackgrounds(CDC* easel);
		void setPalletes(std::array<int, 4> palleteIds);
		// draw pictures...
		void drawPicture( CDC* deviceContext, int pictureNumber, std::vector<long> picData, 
						  std::pair<UINT, UINT> minMaxPair );
		void drawBitmap ( CDC* deviceContext, Matrix<long> picData, std::pair<int, int> minMax );
		void drawDongles( CDC* dc, coordinate selectedLocation, std::vector<coordinate> analysisLocs, 
						  std::vector<atomGrid> gridInfo, UINT pictureNumber );
		void createPalettes( CDC* dc );
		void handleEditChange( UINT id );
		void setAlwaysShowGrid(bool showOption, CDC* easel);
		void redrawPictures( CDC* easel, coordinate selectedLocation, std::vector<coordinate> analysisLocs, 
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
		void paint ( CDC* cdc, CRect size, CBrush* bgdBrush );
		const std::string configDelim;
	private:
		std::array<PictureControl, 4> pictures;
		std::array<HPALETTE, 4> palettes;
		CBrush* gridBrush;
		POINT picturesLocation;
		int picturesWidth;
		int picturesHeight;
		bool autoScalePictures = false;
		bool specialGreaterThanMax;
		bool specialLessThanMin;
		bool alwaysShowGrid;
};

