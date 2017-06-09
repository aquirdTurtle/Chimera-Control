#pragma once
#include "PictureControl.h"
#include <array>

class PictureManager
{
	public:
		void initialize( POINT& loc, CWnd* parent, int& id, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips, 
						 CBrush* defaultGridBrush );
		void refreshBackgrounds( CWnd* parent );
		void drawGrids( CWnd* parent );
		void setParameters( imageParameters parameters );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		void handleScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void drawBackgrounds(CWnd* parent);
		void setPalletes(std::array<int, 4> palleteIds);
		// draw pictures...
		void drawPicture( CDC* deviceContext, int pictureNumber, std::vector<long> picData, 
						 std::pair<int, int> minMaxPair );
		void drawDongles( CWnd* parent, std::pair<int, int> selectedLocation );
		void createPalettes( CDC* dc );
		void handleEditChange( UINT id );
		void redrawPictures( CWnd* parent, std::pair<int, int> selectedLocation );
		void setPictureSliders(CWnd* parent);
		void setNumberPicturesActive( int numberActive );
		std::pair<int, int> handleRClick(CPoint clickLocation);
		void setSinglePicture( CWnd* parent, std::pair<int, int> selectedLocation, imageParameters imageParams );
		void setMultiplePictures( CWnd* parent, std::pair<int, int> selectedLocation, imageParameters imageParams, 
								  int numberActivePics);
		void setAutoScalePicturesOption(bool autoScaleOption);

	private:
		std::array<PictureControl, 4> pictures;
		std::array<HPALETTE, 3> palettes;
		CBrush* gridBrush;
		POINT picturesLocation;
		int picturesWidth;
		int picturesHeight;
		bool autoScalePictures;
};

