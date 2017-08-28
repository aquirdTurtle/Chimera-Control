#pragma once
#include "PictureControl.h"
#include <array>
#include <fstream>

class PictureManager
{
	public:
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& configFile, double version);
		void initialize( POINT& loc, CWnd* parent, int& id, cToolTips& tooltips, CBrush* defaultGridBrush );
		void refreshBackgrounds(CDC* easel);
		void drawGrids(CDC* easel);
		void setParameters( imageParameters parameters );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		void handleScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void drawBackgrounds(CDC* easel);
		void setPalletes(std::array<int, 4> palleteIds);
		// draw pictures...
		void drawPicture( CDC* deviceContext, int pictureNumber, std::vector<long> picData, 
						 std::pair<UINT, UINT> minMaxPair );
		void drawDongles(CDC* dc, std::pair<UINT, UINT> selectedLocation, std::vector<std::pair<UINT, UINT>> analysisLocs );
		void createPalettes( CDC* dc );
		void handleEditChange( UINT id );
		void setAlwaysShowGrid(bool showOption, CDC* easel);
		void redrawPictures(CDC* easel, std::pair<UINT, UINT> selectedLocation, std::vector<std::pair<UINT, UINT>> analysisLocs);
		void setPictureSliders(CWnd* parent);
		void setNumberPicturesActive( int numberActive );
		std::pair<UINT, UINT> handleRClick(CPoint clickLocation);
		void setSinglePicture( CWnd* parent, std::pair<UINT, UINT> selectedLocation, imageParameters imageParams, 
							   std::vector<std::pair<UINT, UINT>> analysisLocs);
		void setMultiplePictures( CWnd* parent, std::pair<UINT, UINT> selectedLocation, imageParameters imageParams,
								  UINT numberActivePics, std::vector<std::pair<UINT, UINT>> analysisLocs);
		void setAutoScalePicturesOption(bool autoScaleOption);
		void setSpecialLessThanMin(bool option);
		void setSpecialGreaterThanMax(bool option);
		void setSettings( std::array<int, 4> maxCounts, std::array<int, 4>  minCounts, bool autoscale, 
						  bool specialGreater, bool specialLess, bool showGrid );
		void resetPictureStorage();

	private:
		std::array<PictureControl, 4> pictures;
		std::array<HPALETTE, 3> palettes;

		CBrush* gridBrush;
		POINT picturesLocation;
		int picturesWidth;
		int picturesHeight;
		bool autoScalePictures;
		bool specialGreaterThanMax;
		bool specialLessThanMin;
		bool alwaysShowGrid;
};

