#pragma once
#include "Control.h"
#include "CameraImageDimensions.h"
;
class PictureControl
{
	public:
		void initialize(POINT& loc, CWnd* parent, int& id, int width, int height);
		void updateGridSpecs( imageParameters newParameters );
		void drawBitmap( CDC* deviceContext, std::vector<long> picData );
		void drawBackground(CWnd* parent);
		void drawGrid(CWnd* parent, CBrush* brush);
		void drawRectangles(CWnd* parent, CBrush* brush);
		void drawCircle(CWnd* parent, std::pair<int, int> selectedLocation );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, std::unordered_map<std::string, CFont*> fonts );
		void handleScroll( int id, UINT nPos );
		void handleEditChange( int id );
		void updatePalette( HPALETTE pallete );
		void redrawImage( CWnd* parent );
		void setActive( bool activeState );
		std::pair<int, int> checkClickLocation( CPoint clickLocation );
	private:
		// for replotting.
		std::vector<long> mostRecentImage;
		// stores info as to whether the control is currently being used in plotting camera data or was used 
		// in the most recent run.
		bool active;
		// Arguably I should make these static controls instead of keeping track explicitly of these things. 
		RECT originalBackgroundArea;
		RECT currentBackgroundArea;
		// 
		int maxSliderPosition;
		int minSliderPosition;
		int colorIndicator;
		HPALETTE imagePalette;
		// grid data
		std::vector<std::vector<RECT>> grid;
		// Picture location data
		Control<CSliderCtrl> sliderMax;
		Control<CSliderCtrl> sliderMin;
		//
		Control<CStatic> labelMax;
		Control<CStatic> labelMin;
		//
		Control<CEdit> editMax;
		Control<CEdit> editMin;
};
