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
		void initialize(POINT& loc, CWnd* parent, int& id, int width, int height, std::array<UINT, 2> minMaxIds );
		void recalculateGrid( imageParameters newParameters );
		void setPictureArea( POINT loc, int width, int height );
		void setSliderLocations(CWnd* parent);
		void drawPicture(CDC* deviceContext, std::vector<long> picData, 
						 std::tuple<bool, int/*min*/, int/*max*/> autoScaleInfo, bool specialMin, bool specialMax);
		void setSliderPositions(UINT min, UINT max);
		void drawBackground(CDC* easel);
		void drawGrid(CDC* easel, CBrush* brush);
		void drawCircle(CDC* dc, coordinate selectedLocation );
		void drawAnalysisMarkers(CDC* dc, std::vector<coordinate> analysisLocs, atomGrid gridInfo );
		void drawRectangle( CDC* dc, RECT pixelRect );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		void handleScroll( int id, UINT nPos );
		void handleEditChange( int id );
		void updatePalette( HPALETTE pallete );
		void redrawImage(CDC* easel);
		void setActive( bool activeState );
		bool isActive();
		std::pair<UINT, UINT> getSliderLocations();
		coordinate checkClickLocation( CPoint clickLocation );
		void resetStorage();
	private:
		std::tuple<bool, int, int> mostRecentAutoscaleInfo;
		bool mostRecentSpecialMinSetting;
		bool mostRecentSpecialMaxSetting;
		// for replotting.
		std::vector<long> mostRecentImage;
		// stores info as to whether the control is currently being used in plotting camera data or was used 
		// in the most recent run.
		bool active;

		// unofficial; these are just parameters this uses to keep track of grid size on redraws.
		imageParameters unofficialImageParameters;

		// Arguably I should make these static controls instead of keeping track explicitly of these things. 
		RECT unscaledBackgroundArea;
		// scaled for the size of the window
		RECT scaledBackgroundArea;
		// scaled for the dimensions of the picture
		RECT pictureArea;

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
