#pragma once
#include "PictureControl.h"
#include <array>

class PictureManager
{
	public:
		void initialize( POINT& loc, CWnd* parent, int& id, std::unordered_map<std::string, CFont*> fonts,
						 std::vector<CToolTipCtrl*>& tooltips, CBrush* defaultGridBrush );
		void refreshBackgrounds( CWnd* parent );
		void drawGrids( CWnd* parent );
		void setParameters( imageParameters parameters );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height,
						std::unordered_map<std::string, CFont*> fonts );
		void handleScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		// draw pictures...
		void drawPicture( CDC* deviceContext, int pictureNumber, std::vector<long> picData );
		void drawDongles( CWnd* parent, std::pair<int, int> selectedLocation );
		void createPalettes( CDC* dc );
		void handleEditChange( UINT id );
		void redrawPictures( CWnd* parent, std::pair<int, int> selectedLocation );
		void setNumberPicturesActive( int numberActive );
		std::pair<int, int> handleRClick(CPoint clickLocation);
	private:
		std::array<PictureControl, 4> pictures;
		std::array<HPALETTE, 3> palettes;
		CBrush* gridBrush;
};

