// created by Mark O. Brown
#pragma once
#include "GeneralImaging/imageParameters.h"
#include "RealTimeDataAnalysis/atomGrid.h"
#include "Plotting/PlotCtrl.h"
#include "CustomQtControls/LongQSlider.h"
#include "softwareAccumulationOption.h"
#include <GeneralObjects/Matrix.h>
#include "QPixmap.h"
#include <qlabel.h>
#include "ImageLabel.h"
#include <GeneralObjects/Matrix.h>
#include <Plotting/QCustomPlotCtrl.h>

/*
 * This class manages a single picture displayed on the camera window and the controls associated with that single 
 * picture. Unlike many classes in my program, this is /not/ built to be a singleton. Instead, there should be one 
 * such control for every picture that needs to be displayed on the screen at a given time. 
 */ 
class PictureManager;
class PictureControl : public QWidget{
	Q_OBJECT
	public:
		PictureControl ( bool histogramOption, Qt::TransformationMode mode );
		void initialize( QPoint loc, int width, int height, IChimeraQtWindow* widget, PictureManager* managerParent,
			int picScaleFactorIn=50);
		void handleContextMenu (const QPoint& pos, PictureManager* managerParent);
		void handleMouse( QMouseEvent* event );
		void drawPicNum(unsigned picNum, QPainter& painter);
		void recalculateGrid( imageParameters newParameters );
		void setPictureArea( QPoint loc, int width, int height );
		void setPicScaleFactor (int scaleFactorIn);
		void setSliderControlLocs(QPoint pos, int height);
		void drawBitmap (const Matrix<long>& picData, bool autoScale, int autoMin, int autoMax,
						 bool specialMin, bool specialMax, std::vector<atomGrid> grids, unsigned pictureNumber, 
						 bool includingAnalysisMarkers);
		void setSliderPositions(unsigned min, unsigned max);
		void drawGrid(QPainter& painter);
		void drawCircle(coordinate selectedLocation, QPainter& painter);
		void setSoftwareAccumulationOption ( softwareAccumulationOption opt );
		softwareAccumulationOption getSoftwareAccumulationOption ();

		void drawAnalysisMarkers( std::vector<atomGrid> gridInfo, QPainter& painter);
		void updatePalette(QVector<QRgb> pallete );
		void redrawImage();
		void setActive( bool activeState );
		bool isActive();
		std::pair<unsigned, unsigned> getSliderLocations();
		void resetStorage();
		void setHoverValue( );
		void updatePlotData ( );
		void drawDongles (QPainter& painter, std::vector<atomGrid> grids, unsigned pictureNumber, 
			bool includingAnalysisMarkers=true);
		coordinate selectedLocation;
		void setTransformationMode (Qt::TransformationMode);
		Matrix<double> getAccumPicData();
		void updatePlotVisibility();
	private:

		Qt::TransformationMode transformationMode;
		int picScaleFactor;
		softwareAccumulationOption saOption;
		Matrix<double> accumPicData;
		int graphSmallSize = 100;
		unsigned accumNum=0;
		bool histOption=false;
		QCustomPlotCtrl* horGraph = nullptr;
		QCustomPlotCtrl* vertGraph = nullptr;
		bool mostRecentAutoScale;
		int mostRecentAutoMin;
		int mostRecentAutoMax;
		bool mostRecentSpecialMinSetting;
		bool mostRecentSpecialMaxSetting;
		// for replotting.
		Matrix<long> mostRecentImage_m;
		int mostRecentPicNum;
		std::vector<coordinate> mostRecentAnalysisLocs;
		std::vector<atomGrid> mostRecentGrids;
		// stores info as to whether the control is currently being used in plotting camera data or was used 
		// in the most recent run.
		unsigned maxWidth, maxHeight;
		bool active;
		
		// unofficial; these are just parameters this uses to keep track of grid size on redraws.
		imageParameters unofficialImageParameters;
		// Arguably I should make these static controls instead of keeping track explicitly of these things. 
		QRect unscaledBackgroundArea;
		// scaled for the size of the window
		QRect scaledBackgroundArea;
		// scaled for the dimensions of the picture
		QRect pictureArea;
		int colorIndicator;
		QVector<QRgb> imagePalette;
		// grid data that outlines each pixel. Used for drawing the grid, text over pixels, etc.
		std::vector<std::vector<QRect>> grid;
		ImageLabel* pictureObject = nullptr;
		QPixmap* pixmap = nullptr;

		LongQSlider sliderMax;
		LongQSlider sliderMin;

		//Control<CPushButton> myButton;
		QLabel* coordinatesText = nullptr;
		QLabel* coordinatesDisp = nullptr;
		QLabel* valueText = nullptr;
		QLabel* valueDisp = nullptr;
};
