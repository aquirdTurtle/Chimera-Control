#pragma once
#include "Control.h"
#include "CameraImageDimensions.h"


/*
 * This class manages a single picture displayed on the camera window and the controls associated with that single 
 * picture. Unlike many classes in my program, this is /not/ built to be a singleton. Instead, there should be one 
 * such control for every picture that needs to be displayed on the screen at a given time. 
 */
;
class PictureControl
{
	public:
		void initialize(POINT& loc, CWnd* parent, int& id, int width, int height);
		void recalculateGrid( imageParameters newParameters );
		void setPictureArea( POINT loc, int width, int height );
		void setSliderLocations(CWnd* parent);
		void drawPicture( CDC* deviceContext, std::vector<long> picData, 
						 std::tuple<bool, int/*min*/, int/*max*/> autoScaleInfo);
		void drawBackground(CWnd* parent);
		void drawGrid(CWnd* parent, CBrush* brush);
		void drawCircle(CWnd* parent, std::pair<int, int> selectedLocation );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		void handleScroll( int id, UINT nPos );
		void handleEditChange( int id );
		void updatePalette( HPALETTE pallete );
		void redrawImage(CWnd* parent);
		void setActive( bool activeState );
		bool isActive();
		std::pair<int, int> checkClickLocation( CPoint clickLocation );
	private:
		std::tuple<bool, int, int> mostRecentAutoscaleInfo;
		// for replotting.
		std::vector<long> mostRecentImage;
		// stores info as to whether the control is currently being used in plotting camera data or was used 
		// in the most recent run.
		bool active;
		// Arguably I should make these static controls instead of keeping track explicitly of these things. 
		RECT unscaledBackgroundArea;
		RECT scaledBackgroundArea;
		// 
		int maxSliderPosition;
		int minSliderPosition;
		int colorIndicator;
		HPALETTE imagePalette;
		// grid data that outlines each pixel.
		std::vector<std::vector<RECT>> grid;

		Control<CSliderCtrl> sliderMax;
		Control<CSliderCtrl> sliderMin;
		//
		Control<CStatic> labelMax;
		Control<CStatic> labelMin;
		//
		Control<CEdit> editMax;
		Control<CEdit> editMin;
};
