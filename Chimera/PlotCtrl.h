#pragma once
#include "myButton.h"
#include "dataPoint.h"
#include <vector>
#include <memory>
#include "memdc.h"
#include "gdiplus.h"
#include <mutex>

enum class plotStyle
{
	// ttl and dac plot use steps.
	TtlPlot,
	DacPlot,
	// uses circs and error bars to represent data and errors
	ErrorPlot,
	// uses bars for histograms
	HistPlot,
	VertHist,
	// 
	OscilloscopePlot
};


typedef std::vector<dataPoint> plotDataVec; 
typedef std::shared_ptr<plotDataVec> pPlotDataVec;


/*
* This is a custom object that I use for plotting. All of the drawing is done manually by standard win32 / MFC
* functionality. Plotting used to be done by gnuplot, an external program which my program would send data to in
* real-time, but this custom plotter, while it took some work (it was fun though) allows me to embed plots in the
* main windows and have a little more direct control over the data being plotted.
*/
class PlotCtrl
{
	public:
		PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::vector<Gdiplus::Pen*> pens,
				  CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes,
				  std::string titleIn = "Title!", bool narrowOpt=false, bool plotHistOption=false);
		~PlotCtrl( );
		void clear( );
		void setCurrentDims( int width, int height);
		void rearrange( int width, int height, fontMap fonts );
		void init( POINT topLeftLoc, LONG width, LONG height, CWnd* parent );
		void drawBorder( memDC* d );
		void plotPoints( memDC* d );
		dataPoint getMainAnalysisResult ( );
		void circleMarker( memDC* d, POINT loc, double size, Gdiplus::Brush* brush );
		void errBars( memDC* d, POINT center, long err, long capSize, Gdiplus::Pen* pen );
		void drawBackground( memDC*, CBrush* backgroundBrush, CBrush* plotAreaBrush );
		void drawPlot( CDC* cdc, CBrush* backgroundBrush, CBrush* plotAreaBrush );
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

		bool wantsSustain( );
		void setControlLocation ( POINT topLeftLoc, LONG width, LONG height );

		std::vector<std::mutex> dataMutexes;
	private:
		const bool narrow;
		// in units of the data
		double boxWidth=10;
		double boxWidthPixels;
		double widthScale2, heightScale2;
		const plotStyle style;
		// first level deliminates different lines which get different colors. second level deliminates different 
		// points within the line.
		std::string title;
		// average data is last element...
		std::vector<pPlotDataVec> data;
		RECT controlDims;
		RECT plotAreaDims;
		CPen whitePen, greyPen, redPen, solarizedPen;
		Gdiplus::SolidBrush* whiteBrush;
		Control<CleanCheck> legButton;
		Control<CleanCheck> sustainButton;
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
