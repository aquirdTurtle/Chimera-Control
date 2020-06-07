// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "GeneralImaging/imageParameters.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "Plotting/PlotCtrl.h"
#include "CustomMfcControlWrappers/LongCSlider.h"
#include "softwareAccumulationOption.h"
#include "QPixmap.h"
#include <qlabel.h>
/*
 * This class manages a single picture displayed on the camera window and the controls associated with that single 
 * picture. Unlike many classes in my program, this is /not/ built to be a singleton. Instead, there should be one 
 * such control for every picture that needs to be displayed on the screen at a given time. 
 */
;
class PictureControl
{
	public:
		PictureControl ( bool histogramOption );
		void initialize( POINT loc, int width, int height, IChimeraWindowWidget* widget);
		void handleMouse( CPoint p );
		void drawPicNum(UINT picNum );
		void recalculateGrid( imageParameters newParameters );
		void setPictureArea( POINT loc, int width, int height );
		void setSliderControlLocs(POINT pos, int height);
		void drawBitmap (const Matrix<long>& picData, std::tuple<bool, int, int> autoscaleInfo, 
						 bool specialMin, bool specialMax);
		void setSliderPositions(UINT min, UINT max);
		void drawBackground();
		void drawGrid(CBrush* brush);
		void drawCircle(coordinate selectedLocation );
		void setSoftwareAccumulationOption ( softwareAccumulationOption opt );
		void drawAnalysisMarkers( std::vector<coordinate> analysisLocs, std::vector<atomGrid> gridInfo );
		void setCursorValueLocations( CWnd* parent );
		void drawRectangle( RECT pixelRect );
		void handleScroll( int id, UINT nPos );
		void handleEditChange( int id );
		void updatePalette( HPALETTE pallete );
		void redrawImage( bool bkgd=true );
		void setActive( bool activeState );
		bool isActive();
		std::pair<UINT, UINT> getSliderLocations();
		coordinate checkClickLocation( CPoint clickLocation );
		void resetStorage();
		void setHoverValue( );
		void updatePlotData ( );
		void paint ( CRect size, CBrush* bgdBrush );
	private:
		softwareAccumulationOption saOption;
		std::vector<double> accumPicData;
		UINT accumNum;
		int plotIDs = 1000;

		const bool histOption;
		std::vector<pPlotDataVec> horData, vertData;
		PlotCtrl* horGraph;
		PlotCtrl* vertGraph;
		std::tuple<bool, int, int> mostRecentAutoscaleInfo;
		bool mostRecentSpecialMinSetting;
		bool mostRecentSpecialMaxSetting;
		POINT mouseCoordinates;
		// for replotting.
		Matrix<long> mostRecentImage_m;
		// stores info as to whether the control is currently being used in plotting camera data or was used 
		// in the most recent run.
		UINT maxWidth, maxHeight;
		bool active;

		// unofficial; these are just parameters this uses to keep track of grid size on redraws.
		imageParameters unofficialImageParameters;
		// Arguably I should make these static controls instead of keeping track explicitly of these things. 
		RECT unscaledBackgroundArea;
		// scaled for the size of the window
		RECT scaledBackgroundArea;
		// scaled for the dimensions of the picture
		RECT pictureArea;
		int colorIndicator;
		HPALETTE imagePalette;
		// grid data that outlines each pixel. Used for drawing the grid, text over pixels, etc.
		std::vector<std::vector<RECT>> grid;
		QLabel* pictureObject;
		QPixmap* pixmap;

		LongCSlider sliderMax;
		LongCSlider sliderMin;

		//Control<CPushButton> myButton;
		QLabel* coordinatesText;
		QLabel* coordinatesDisp;
		QLabel* valueText;
		QLabel* valueDisp;
};
