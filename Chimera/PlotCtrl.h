#pragma once
#include "windows.h"
#include <vector>
#include "memdc.h"

struct dataPoint
{
	double x;
	double y;
	// yerr
	double err;
};

typedef std::vector<dataPoint> plotDataVec;
typedef std::shared_ptr<plotDataVec> pPlotDataVec;

class PlotCtrl : public CDialog
{
	/*
	 * At the moment this is designed only to be put in it's own dialog window. However, it should be possible without
	 * much effort to put this as a single control within a larger window as well, for example the auxiliary window.
	 */
	public:
		DECLARE_DYNAMIC( PlotCtrl );
  		PlotCtrl( std::vector<pPlotDataVec> dataHolder );
		~PlotCtrl( );
		BOOL OnInitDialog( ) override;
		void OnPaint( );// override;
		void init( POINT topLeftLoc, LONG width, LONG height );
		void drawBorder( memDC* d, double width, double height );
		void plotPoints( memDC* d, double width, double height );
		void OnSize( UINT s, int cx, int cy );
		void circleMarker( memDC* d, POINT loc, double size );
		void errBars( memDC* d, POINT center, long err, long size );
		void drawBackground( memDC*, double width, double height );
		void drawGridAndAxis( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX, double width,
							  double height, std::pair<double, double> minMaxRawY, 
							  std::pair<double, double> minMaxScaledY );
		void drawLine( CDC* d, double begX, double begY, double endX, double endY );
		void drawLine( CDC* d, POINT beg, POINT end );
		void convertDataToScreenCoords( double width, double height, std::vector<plotDataVec>& dat );
		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		void drawLegend( memDC* d, UINT width, UINT height, UINT lineNum );
		void getMinMaxY( std::vector<plotDataVec> screenData, std::vector<pPlotDataVec> rawData,
						 std::pair<double, double>& minMaxRaw, std::pair<double, double>& minMaxScaled );
		void OnTimer( UINT_PTR id );
	private:
		DECLARE_MESSAGE_MAP( );
		// first level deliminates different lines which get different colors. second level deliminates different 
		// points within the line.
		std::vector<pPlotDataVec> data;
		CBrush backgroundBrush;
		RECT controlDims;
		RECT plotAreaDims;
		CPen whitePen, greyPen, redPen, solarizedPen;
		std::vector<CPen*> pens;
		// options for...
		// legend on off
		// yscale dynamic or 0->1
		// ...?
};

