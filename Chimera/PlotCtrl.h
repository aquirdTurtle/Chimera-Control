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
		PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::vector<Gdiplus::Pen*> pens,
				  CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes,
				  std::string titleIn = "Title!" );
		~PlotCtrl( );
		void setCurrentDims( int width, int height);
		void rearrange( int width, int height, fontMap fonts );
		void init( POINT topLeftLoc, LONG width, LONG height, CWnd* parent );
		void drawBorder( memDC* d );
		void plotPoints( memDC* d );
		
		void circleMarker( memDC* d, POINT loc, double size, Gdiplus::Brush* brush );
		void errBars( memDC* d, POINT center, long err, long capSize, Gdiplus::Pen* pen );
		void drawBackground( memDC*, CBrush* backgroundBrush, CBrush* plotAreaBrush );
		void makeBarPlot( memDC* d, plotDataVec scaledLine, Gdiplus::SolidBrush* brush );
		void drawGridAndAxes( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX, 
							  std::pair<double, double> minMaxRawY, std::pair<double, double> minMaxScaledY );
		void drawLine( CDC* d, double begX, double begY, double endX, double endY, Gdiplus::Pen* p );
		void drawLine( CDC* d, POINT beg, POINT end, Gdiplus::Pen* p );
		void convertDataToScreenCoords( std::vector<plotDataVec>& dat );
		void shiftTtlData( std::vector<plotDataVec>& rawData );
		void drawLegend( memDC* d, std::vector<plotDataVec> screenData );
		void getMinMaxY( std::vector<plotDataVec> screenData, std::vector<pPlotDataVec> rawData,
						 std::pair<double, double>& minMaxRaw, std::pair<double, double>& minMaxScaled );
		void drawTitle( memDC* d );
		CRect GetPlotRect(  );
		void makeLinePlot( memDC* d, plotDataVec line, Gdiplus::Pen* p );
		void makeStepPlot( memDC* d, plotDataVec line, Gdiplus::Pen* p , Gdiplus::Brush* b );

		std::vector<std::mutex> dataMutexes;
	private:
		// in units of the data
		//CBrush* backgroundBrush;
		double boxWidth=1;
		double boxWidthPixels;
		double widthScale2, heightScale2;
		const plotStyle style;
		// first level deliminates different lines which get different colors. second level deliminates different 
		// points within the line.
		std::string title;
		std::vector<pPlotDataVec> data;
		RECT controlDims;
		RECT plotAreaDims;
		CPen whitePen, greyPen, redPen, solarizedPen;
		Gdiplus::SolidBrush* whiteBrush;
		Control<CButton> legButton;
		//std::vector<CPen*> pens;
		std::vector<Gdiplus::SolidBrush*> brushes;
		std::vector<Gdiplus::Pen*> pens;
		Gdiplus::Pen* whiteGdiPen;
		Gdiplus::Pen* greyGdiPen;
		CFont* textFont;
		// options for...
		// legend on off
		// yscale dynamic or 0->1
		// ...?
};
