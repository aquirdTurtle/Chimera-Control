#pragma once
#include <vector>
#include <memory>
#include "memdc.h"

enum plotStyle
{
	// ttl and dac plot use steps.
	TtlPlot,
	DacPlot,
	// uses circs and error bars to represent data and errors
	ErrorPlot,
	// uses bars for histograms
	HistPlot,
	// 
	OscilloscopePlot
};

struct dataPoint
{
	double x;
	double y;
	// yerr, could add xerr in future.
	double err;
};

typedef std::vector<dataPoint> plotDataVec; 
typedef std::shared_ptr<plotDataVec> pPlotDataVec;

class PlotCtrl
{
	public:
		PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::string titleIn="Title!" );
		~PlotCtrl( );
		void rearrange( int width, int height, fontMap fonts );
		void init( POINT topLeftLoc, LONG width, LONG height, CWnd* parent );
		void drawBorder( memDC* d, double width, double height );
		void plotPoints( memDC* d, double width, double height );
		
		void circleMarker( memDC* d, POINT loc, double size );
		void errBars( memDC* d, POINT center, long err, long size );
		void drawBackground( memDC*, double width, double height, CBrush* backgroundBrush );
		void drawGridAndAxes( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX, double width,
							  double height, std::pair<double, double> minMaxRawY,
							  std::pair<double, double> minMaxScaledY );
		void drawLine( CDC* d, double begX, double begY, double endX, double endY );
		void drawLine( CDC* d, POINT beg, POINT end );
		void convertDataToScreenCoords( double width, double height, std::vector<plotDataVec>& dat );
		void shiftTtlData( std::vector<plotDataVec>& rawData );
		void drawLegend( memDC* d, UINT width, UINT height, std::vector<plotDataVec> screenData );
		void getMinMaxY( std::vector<plotDataVec> screenData, std::vector<pPlotDataVec> rawData,
						 std::pair<double, double>& minMaxRaw, std::pair<double, double>& minMaxScaled );
		void drawTitle( memDC* d, long width, long height );
		CRect GetPlotRect( LONG width, LONG height );
		void makeLinePlot( memDC* d, LONG width, LONG height, plotDataVec line );
		void makeStepPlot( memDC* d, LONG width, LONG height, plotDataVec line );

		std::vector<std::mutex> dataMutexes;
	private:
		const plotStyle style;
		// first level deliminates different lines which get different colors. second level deliminates different 
		// points within the line.
		std::string title;
		std::vector<pPlotDataVec> data;
		RECT controlDims;
		RECT plotAreaDims;
		CPen whitePen, greyPen, redPen, solarizedPen;
		Control<CButton> legButton;
		std::vector<CPen*> pens;
		// options for...
		// legend on off
		// yscale dynamic or 0->1
		// ...?
};