#include "stdafx.h"
#include "PlotCtrl.h"
#include "Thrower.h"


PlotCtrl::PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::vector<Gdiplus::Pen*> pensIn,
					CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes,	std::string titleIn, bool narrowOpt,
					bool plotHistOption ) :
	whitePen( PS_SOLID, 0, RGB( 255, 255, 255 ) ),
	greyPen( PS_SOLID, 0, RGB( 100, 100, 100 ) ),
	redPen( PS_SOLID, 0, RGB( 255, 0, 0 ) ),
	solarizedPen( PS_SOLID, 0, RGB( 0, 30, 38 ) ),
	data( dataHolder ), style( inStyle ), dataMutexes( dataHolder.size( ) ), textFont( font ),
	brushes( plotBrushes ), pens( pensIn ), narrow(narrowOpt )
{
	Gdiplus::Color whiteColor( 255, 255, 255, 255 );
	whiteBrush = new Gdiplus::SolidBrush( whiteColor );
	whiteGdiPen = new Gdiplus::Pen( whiteBrush );
	Gdiplus::Color greyColor( 100, 100, 100, 100);
	whiteBrush = new Gdiplus::SolidBrush( greyColor );
	greyGdiPen = new Gdiplus::Pen( greyColor );
	title = titleIn;
	if ( narrow )
	{
		boxWidth = 1;
	}
}


PlotCtrl::~PlotCtrl ( )
{
	for ( auto& pen : pens )
	{
		delete pen;
	}
}


dataPoint PlotCtrl::getMainAnalysisResult ( )
{
	// get the average data. If not only a single data point, as this is currently ment to be used, then I'm not 
	// positive what value this is grabbing... maybe the last point of the average?
	return data.back ( )->back();
}


void PlotCtrl::drawBackground( memDC* d, CBrush* backgroundBrush, CBrush* plotAreaBrush )
{
	RECT r = { controlDims.left * widthScale2, controlDims.top*heightScale2, controlDims.right * widthScale2, 
		controlDims.bottom*heightScale2 };
	//d->
	CPen pen( 0, 0, RGB( 0, 0, 0 ) );
	d->SelectObject( pen );
	d->SelectObject( *backgroundBrush );
	d->Rectangle( &r );
	r.left = plotAreaDims.left * widthScale2;
	r.right = plotAreaDims.right * widthScale2;
	r.top = plotAreaDims.top * heightScale2;
	r.bottom = plotAreaDims.bottom * heightScale2;
	d->SelectObject( *plotAreaBrush );
	d->SelectObject( pen );
	d->Rectangle( &r );
}


void PlotCtrl::drawTitle( memDC* d )
{
	RECT scaledArea = { plotAreaDims.left * widthScale2, plotAreaDims.top * heightScale2,
						plotAreaDims.right * widthScale2, plotAreaDims.bottom * heightScale2 };
	RECT r = { scaledArea.left, scaledArea.top - 30, scaledArea.right, scaledArea.top};
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( RGB( 255, 255, 255 ) );
	d->SelectObject( textFont );
	d->DrawTextEx( LPSTR( cstr( title ) ), title.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
}


CRect PlotCtrl::GetPlotRect( )
{
	CRect realArea;
	realArea.left = controlDims.left * widthScale2;
	realArea.right = controlDims.right * widthScale2;
	realArea.top = controlDims.top * heightScale2;
	realArea.bottom = controlDims.bottom * heightScale2;
	return realArea;
}


void PlotCtrl::setControlLocation ( POINT topLeftLoc, LONG width, LONG height )
{
	controlDims = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + width, topLeftLoc.y + height };
	controlDims = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + width, topLeftLoc.y + height };
	RECT d = controlDims;
	long w = d.right - d.left, h = d.bottom - d.top;
	if ( !narrow )
	{
		plotAreaDims = { long ( d.left + w*0.1 ), long ( d.top + h*0.08 ), long ( d.right*0.98 ), long ( d.bottom - h*0.1 ) };
	}
	else
	{
		plotAreaDims = controlDims;
	}
}


void PlotCtrl::init( POINT topLeftLoc, LONG width, LONG height, CWnd* parent )
{
	setControlLocation ( topLeftLoc, width, height );

	legButton.sPos = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + 22, topLeftLoc.y += 22 };
	legButton.Create( "", NORM_CHECK_OPTIONS, legButton.sPos, parent, 0 );
	sustainButton.sPos = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + 22, topLeftLoc.y += 22 };
	sustainButton.Create( "", NORM_CHECK_OPTIONS, sustainButton.sPos, parent, 0 );
}


bool PlotCtrl::wantsSustain( )
{
	return sustainButton.GetCheck( );
}


void PlotCtrl::rearrange( int width, int height, fontMap fonts )
{
	legButton.rearrange( width, height, fonts );
	sustainButton.rearrange( width, height, fonts );
}


void PlotCtrl::convertDataToScreenCoords( std::vector<plotDataVec>& screenData )
{
	double minx = DBL_MAX, maxx = -DBL_MAX, miny = DBL_MAX, maxy = -DBL_MAX;
	for ( auto line : screenData )
	{
		for ( auto elem : line )
		{
			if ( elem.x < minx )
			{
				minx = elem.x;
			}
			if ( elem.x > maxx )
			{
				maxx = elem.x;
			}
		}
	}
	if ( style == plotStyle::OscilloscopePlot || style == plotStyle::HistPlot || style == plotStyle::DacPlot 
		 || style == plotStyle::VertHist )
	{
		for ( auto line : screenData )
		{
			for ( auto elem : line )
			{
				if ( elem.y < miny )
				{
					miny = elem.y;
				}
				if ( elem.y > maxy )
				{
					maxy = elem.y;
				}
			}
		}
	}
	double plotWidthPixels = widthScale2 * (plotAreaDims.right - plotAreaDims.left);
	double plotHeightPixels = heightScale2 * (plotAreaDims.bottom - plotAreaDims.top);
	double rangeX = maxx - minx;
	double rangeY = maxy - miny;

	if ( rangeX == 0 )
	{
		rangeX += 1;
	}
	if ( rangeY == 0 )
	{
		rangeY += 1;
	}
	double dataScaleX = ( plotWidthPixels - 20 ) / (rangeX);
	if ( style == plotStyle::HistPlot )
	{
		// resize things to take into acount the widths.
		maxx += boxWidth;
		minx -= boxWidth;
		rangeX = maxx - minx;
		boxWidthPixels = ceil(boxWidth * dataScaleX * 0.99);
	}
	double dataHeight = 1;
	double dataMin = 0;
	if ( style == plotStyle::ErrorPlot )
	{
		// currently assuming 0-1 instead of auto-sizing.
		dataHeight = 1;
		dataMin = 0;
	}
	else if ( style == plotStyle::TtlPlot )
	{
		dataHeight = 2;
		// because of offset in plot
		dataMin = -0.5;
	}
	else if ( style == plotStyle::DacPlot )
	{
		// currently doing autoscaling here.
		dataHeight = maxy - miny;
		dataMin = miny;
		//dataHeight = 21;
		//dataMin = -10;
	}
	else if ( style == plotStyle::OscilloscopePlot )
	{
		dataHeight = (maxy - miny) * 1.1;
		dataMin = miny - (maxy - miny)*0.05;
	}
	else if ( style == plotStyle::HistPlot || style == plotStyle::VertHist )
	{
		dataHeight = maxy - miny;
		dataMin = 0;
	}
	else
	{
		thrower ( "ERROR: bad value for plot style???  (A low level bug, this shouldn't happen)" );
	}
	if ( style == plotStyle::VertHist )
	{
		// flipped
		double dataScaleX = plotWidthPixels / dataHeight;
		double dataScaleY = plotHeightPixels / rangeX;
		boxWidthPixels = ceil ( boxWidth * dataScaleY * 0.99 );
		for ( auto& line : screenData )
		{
			for ( auto& point : line )
			{
				auto px = point.x;
				auto py = point.y;
				point.x = plotAreaDims.left * widthScale2 + ( py - dataMin ) * dataScaleX + 10;
				point.y = plotAreaDims.bottom * heightScale2 - ( px - minx ) * dataScaleY;
				point.err *= dataScaleY;
			}
		}
	}
	else
	{
		double dataScaleY = plotHeightPixels / dataHeight;

		for ( auto& line : screenData )
		{
			for ( auto& point : line )
			{
				point.x = plotAreaDims.left * widthScale2 + ( point.x - minx ) * dataScaleX + 10;
				point.y = plotAreaDims.bottom * heightScale2 - ( point.y - dataMin ) * dataScaleY;
				point.err *= dataScaleY;
			}
		}
	}
}

void PlotCtrl::clear( )
{
	pens.clear( );
	brushes.clear( );
}


void PlotCtrl::plotPoints( memDC* d )
{
	if ( data.size( ) == 0 )
	{
		return;
	}
	std::vector<plotDataVec> screenData;
	std::vector<plotDataVec> shiftedData;
	{
		for ( auto lineCount : range( data.size( ) ) )
		{
			std::lock_guard<std::mutex> lock( dataMutexes[lineCount] );
			screenData.push_back( *data[lineCount] );
		}
	}
	shiftedData = screenData;
	if ( style == plotStyle::TtlPlot )
	{
		shiftTtlData( shiftedData );
	}
	convertDataToScreenCoords( shiftedData );
	std::vector<double> xRaw, xScaled;
	{
		std::lock_guard<std::mutex> lock( dataMutexes[0] );
		for ( auto pointCount : range( shiftedData[0].size( ) ) )
		{
			xRaw.push_back( data[0]->at( pointCount ).x );
			xScaled.push_back( shiftedData[0][pointCount].x );
		}
	}
	std::pair<double, double> minMaxScaled, minMaxRaw;
	if ( style == plotStyle::OscilloscopePlot || style == plotStyle::HistPlot || style == plotStyle::VertHist )
	{
		getMinMaxY( screenData, data, minMaxRaw, minMaxScaled );
	}
	drawGridAndAxes( d, xRaw, xScaled, minMaxRaw, minMaxScaled );
	UINT penNum = 0;
	UINT lineNum = 0;
	for ( auto& line : shiftedData )
	{
		Gdiplus::SolidBrush* brush;
		Gdiplus::Pen* pen;
		if ( lineNum == shiftedData.size( ) - 1 && style == plotStyle::ErrorPlot )
		{
			// average data set is white.
			pen = whiteGdiPen;
			brush = whiteBrush;
		}
		else
		{
			pen = pens[penNum];
			brush = brushes[penNum];
			penNum += int( 256 / shiftedData.size( ) );
		}
		if ( style == plotStyle::ErrorPlot )
		{
			UINT pointCount = 0;
			std::lock_guard<std::mutex> lock( dataMutexes[lineNum] );
			for ( auto point : line )
			{
				if ( data[lineNum]->at( pointCount ).y <= 1 && data[lineNum]->at( pointCount ).y >= 0 )
				{
					UINT circSize;
					if ( point.err < 3 )
					{
						circSize = 2*point.err;
					}
					else
					{
						circSize = 6;
					}
					circleMarker( d, { long( point.x ), long( point.y ) }, circSize, brush );
					errBars( d, { long( point.x ), long( point.y ) }, point.err, 6, pen );
				}
				pointCount++;
			}
		}
		else if ( style == plotStyle::TtlPlot || style == plotStyle::DacPlot )
		{
			makeStepPlot( d, line, pen, brush );
		}
		else if ( style == plotStyle::OscilloscopePlot )
		{
			makeLinePlot( d, line, pen );
		}
		else if ( style == plotStyle::HistPlot || style == plotStyle::VertHist )
		{
			makeBarPlot( d, line, brush );
		}
		lineNum++;
	}
	if ( legButton && legButton.GetCheck( ) )
	{
		drawLegend( d, shiftedData );
	}
	if ( legButton )
	{
		legButton.Invalidate( );
		//legButton.RedrawWindow( );
	}
	if ( sustainButton )
	{
		sustainButton.Invalidate( );
		//sustainButton.RedrawWindow( );
	}
}


void PlotCtrl::makeBarPlot( memDC* d, plotDataVec scaledLine, Gdiplus::SolidBrush* brush )
{
	for ( auto& point : scaledLine )
	{
		Gdiplus::Rect r;
		if ( style == plotStyle::HistPlot )
		{
			r = Gdiplus::Rect ( floor(point.x - boxWidthPixels / 2), point.y, boxWidthPixels,
								plotAreaDims.bottom * heightScale2 - point.y );
		}
		else if ( style == plotStyle::VertHist )
		{
			r = Gdiplus::Rect ( plotAreaDims.left * widthScale2, floor(point.y - boxWidthPixels / 2),
								point.x - plotAreaDims.left * widthScale2, ceil(boxWidthPixels) );
		}
		else
		{
			thrower ( "Bad plot style for making a bar plot?!?!?" );
		}
		Gdiplus::Graphics g( d->GetSafeHdc( ) );
		g.FillRectangle( brush, r );
		//circleMarker( d, { LONG( point.x), LONG( point.y + 1 ) }, 5, brush );
	}
}


void PlotCtrl::setCurrentDims( int width, int height )
{
	widthScale2 = width / 1920.0;
	heightScale2 = height / 997.0;
}


void PlotCtrl::getMinMaxY( std::vector<plotDataVec> screenData, std::vector<pPlotDataVec> rawData,
						   std::pair<double, double>& minMaxRaw, std::pair<double, double>& minMaxScaled )
{
	minMaxRaw.first = DBL_MAX;
	minMaxRaw.second = -DBL_MAX;
	minMaxScaled.first = DBL_MAX;
	minMaxScaled.second = -DBL_MAX;
	for ( auto line : screenData )
	{
		for ( auto p : line )
		{
			if ( p.y < minMaxScaled.first )
			{
				minMaxScaled.first = p.y;
			}
			if ( p.y > minMaxScaled.second )
			{
				minMaxScaled.second = p.y;
			}
		}
	}
	UINT lineCount = 0;
	for ( auto line : rawData )
	{
		std::lock_guard<std::mutex> lock( dataMutexes[lineCount] );
		for ( auto p : *line )
		{
			if ( p.y < minMaxRaw.first )
			{
				minMaxRaw.first = p.y;
			}
			if ( p.y > minMaxRaw.second )
			{
				minMaxRaw.second = p.y;
			}
		}
		lineCount++;
	}
}


void PlotCtrl::shiftTtlData( std::vector<plotDataVec>& rawData )
{
	/*
		shift all the data points away from 0 or 1 just so that the points are visible.
	*/
	UINT count = 1;
	for ( auto& line : rawData )
	{
		for ( auto& elem : line )
		{
			elem.y += (count - 8.0) * 1.0 / 20.0;
		}
		count++;
	}
}

void PlotCtrl::makeLinePlot( memDC* d, plotDataVec scaledLine, Gdiplus::Pen* p )
{
	// want to draw a vertical line at each point of line, and then draw horizontal lines to connect the ends of the 
	// vertical lines.
	dataPoint prevPoint;
	UINT count = 0;
	for ( auto& point : scaledLine )
	{
		count++;
		// draw line from prev point to current point
		if ( count == 1 )
		{
			// update prev point
			prevPoint = point;
			continue;
		}
		drawLine( d, { long( prevPoint.x ), long( prevPoint.y ) }, { long( point.x ), long( point.y ) }, p );
		prevPoint = point;
	}
}


void PlotCtrl::makeStepPlot( memDC* d, plotDataVec scaledLine, Gdiplus::Pen* p, Gdiplus::Brush* b )
{
	// want to draw a vertical line at each point of line, and then draw horizontal lines to connect the ends of the 
	// vertical lines.
	dataPoint prevPoint;
	UINT count = 0;
	for ( auto& point : scaledLine )
	{
		count++;
		// draw line from prev point to current point
		if ( count == 1 )
		{
			// update prev point
			prevPoint = point;
			continue;
		}
		// make a horizontal line from the previous x to the current x at the previous height.
		drawLine( d, { long( prevPoint.x ), long( prevPoint.y ) }, { long( point.x ), long( prevPoint.y ) }, p );
		// using the same x makes this a vertical line.
		drawLine( d, { long( point.x ), long( prevPoint.y ) }, { long( point.x ), long( point.y ) }, p );
		if ( point.y != prevPoint.y )
		{
			circleMarker( d, { long( point.x ), long( point.y ) }, 5, b );
		}
		prevPoint = point;
	}
}


void PlotCtrl::drawGridAndAxes( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX, 
								std::pair<double, double> minMaxRawY, std::pair<double, double> minMaxScaledY )
{
	double xMin = DBL_MAX, xMax = -DBL_MAX;
	for ( auto x : xAxisPts )
	{
		if ( x > xMax )
		{
			xMax = x;
		}
		if ( x < xMin )
		{
			xMin = x;
		}
	}
	if ( style == plotStyle::HistPlot )
	{
		// resize things to take into acount the widths.
		xMax += boxWidth;
		xMin -= boxWidth;
	}
	// assumes that data has been converted to screen coords.
	RECT scaledArea = { plotAreaDims.left * widthScale2, plotAreaDims.top * heightScale2,
		plotAreaDims.right * widthScale2, plotAreaDims.bottom * heightScale2 };
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( RGB( 255, 255, 255 ) );
	UINT count = 0;
	bool labelEachPoint = (scaledX.size( ) < 15);
	for ( auto x : scaledX )
	{
		// draw vertical lines for x points
		RECT r = { long( x - 40 ), long( scaledArea.bottom ),
			long( x + 60 ), long( controlDims.bottom * heightScale2 ) };
		std::string txt = str( xAxisPts[count], 3 );
		if ( labelEachPoint )
		{
			drawLine( d, x, scaledArea.bottom + 5, x, scaledArea.top, greyGdiPen );
			d->SelectObject( textFont );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}
		count++;
	}
	if ( !labelEachPoint )
	{
		double scaledWidth = scaledArea.right - scaledArea.left;
		double dataRange = xMax - xMin;
		
		for ( auto count : range( 11 ) )
		{
			RECT r = { long( scaledArea.left + count * (scaledWidth - 20) / 10.0 - 40), long( scaledArea.bottom ),
					   long( scaledArea.left + count * (scaledWidth - 20) / 10.0 + 60 ),
					   long( controlDims.bottom * heightScale2 ) };
			std::string txt = str( xMin + count * dataRange / 10.0 );
			drawLine( d, scaledArea.left + count * (scaledWidth-20) / 10.0 + 10, scaledArea.bottom + 5,
					  scaledArea.left + count * (scaledWidth - 20) / 10.0 + 10, scaledArea.top, greyGdiPen );
			d->SelectObject( textFont );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}
	}
	UINT numLines = 6;
	double vStep = (scaledArea.bottom - scaledArea.top) / (numLines - 1);
	double vRawStep = (minMaxRawY.second - minMaxRawY.first) / (numLines - 1);
	// min to max version
	if ( style == plotStyle::TtlPlot )
	{
		// no left to right gridlines.
	}
	else 
	{
		double minY, maxY;
		if ( style == plotStyle::DacPlot )
		{
			minY = minMaxRawY.first;
			maxY = minMaxRawY.second;
		}
		else if ( style == plotStyle::ErrorPlot )
		{
			minY = 0;
			maxY = 1;
		}
		else
		{
			minY = minMaxRawY.first;
			maxY = minMaxRawY.second;
		}
		// Forces y-axis to be between 0 and 1.
		for ( auto gridline : range( numLines ) )
		{
			drawLine( d, scaledArea.left - 5, scaledArea.top + gridline * vStep, scaledArea.right,
					  scaledArea.top + gridline * vStep, greyGdiPen );
			RECT r = { long( controlDims.left * widthScale2 ), long( scaledArea.top + 10 + gridline * vStep ),
					   long( scaledArea.left ), long( scaledArea.top - 10 + gridline * vStep ) };
			std::string txt = str( maxY - (maxY - minY) * double( gridline ) / (numLines - 1), 5 );
			d->SelectObject( textFont );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}

		if ( false )
		{
			double vStep = 0;// (minMaxScaled.top - minMaxScaled.bottom) / (numLines - 1);
			for ( auto gridline : range( numLines ) )
			{
				drawLine( d, scaledArea.left - 5, minMaxScaledY.second - gridline * vStep, scaledArea.right,
						  minMaxScaledY.second - gridline * vStep, greyGdiPen );
				RECT r = { long( controlDims.left * widthScale2 ), long( minMaxScaledY.second + 10 - gridline * vStep ),
					long( scaledArea.left), long( minMaxScaledY.second - 10 - gridline * vStep ) };
				std::string txt = str( minMaxRawY.first + gridline * vRawStep, 5 );
				d->SelectObject( textFont );
				d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
			}
		}

	}

	// axis labels
	RECT r = { scaledArea.left, scaledArea.bottom, scaledArea.right, controlDims.bottom * heightScale2 };
	std::string txt = "xlabel";
	d->SelectObject( textFont );
	d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	r = { long( controlDims.left * widthScale2), scaledArea.top, scaledArea.left, scaledArea.bottom };
	if ( style == plotStyle::TtlPlot )
	{
		txt = "Ttl State";
	}
	else if ( style == plotStyle::DacPlot )
	{
		txt = "Dac Voltage";
	}
	else if ( style == plotStyle::ErrorPlot )
	{
		txt = "%";
	}
	else if ( style == plotStyle::HistPlot )
	{
		txt = "Occurances";
	}
	d->SelectObject( textFont );
	d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
}


void PlotCtrl::drawLegend( memDC* d, std::vector<plotDataVec> screenData )
{
	UINT plotAreaWidth = (plotAreaDims.right - plotAreaDims.left) * widthScale2;
	UINT plotAreaHeight = (plotAreaDims.bottom - plotAreaDims.top) * heightScale2;
	UINT itemNum = screenData.size( );
	UINT itemsPerColumn = 21;
	UINT totalColNum = int(screenData.size() / itemsPerColumn );
	UINT penNum = 0;
	for ( auto lineNum : range( screenData.size() ) )
	{
		Gdiplus::SolidBrush* brush;
		Gdiplus::Pen* pen;
		if ( lineNum == screenData.size( ) - 1 )
		{
			// average data set is white.
			d->SelectObject( whitePen );
			brush = whiteBrush;
		}
		else
		{
			// TODO: handle alpha here...
			//d->SelectObject( pens[penNum] );
			brush = brushes[penNum];
			pen = pens[penNum];
			penNum += int( 256 / screenData.size( ) );
		}
		UINT rowNum = lineNum % itemsPerColumn;
		UINT colNum = lineNum / itemsPerColumn;
		POINT legendLoc = { plotAreaDims.left * widthScale2 + (1 - (colNum + 1) * 0.075) * plotAreaWidth,
			plotAreaDims.top * heightScale2 + 0.025 * plotAreaHeight + 20 * rowNum };
		circleMarker( d, legendLoc, 10, brush );
		RECT r = { long( legendLoc.x + 10 ), long( legendLoc.y + 10 ),
			long( plotAreaDims.right * widthScale2 - colNum * 0.075 * plotAreaWidth ), long( legendLoc.y - 10 ) };
		d->DrawTextEx( const_cast<char *>(cstr( lineNum)), str( lineNum ).size( ), &r,
					   DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	}
}


void PlotCtrl::drawLine( CDC* d, double begX, double begY, double endX, double endY, Gdiplus::Pen* p )
{
	Gdiplus::Graphics g( d->GetSafeHdc( ) );
	Gdiplus::Point p1( { int(begX), int(begY) } ), p2( { int(endX), int(endY) } );
	g.DrawLine( p, p1, p2 );
}


void PlotCtrl::drawLine( CDC* d, POINT beg, POINT end, Gdiplus::Pen* p )
{
	Gdiplus::Graphics g( d->GetSafeHdc( ) );
	Gdiplus::Point p1( { beg.x, beg.y } ), p2( { end.x, end.y } );
	g.DrawLine( p, p1, p2 );
}


void PlotCtrl::circleMarker( memDC* d, POINT loc, double size, Gdiplus::Brush* brush )
{
	Gdiplus::Graphics g( d->GetSafeHdc( ) );
	int x1, x2, y1, y2;
	x1 = loc.x - size / 2;
	y1 = loc.y - size / 2;
	x2 = x1 + size;
	y2 = y1 + size;
	Gdiplus::Rect r(x1, y1, x2-x1, y2-y1 );
	g.FillEllipse( brush, r );
}


void PlotCtrl::errBars( memDC* d, POINT center, long err, long capSize, Gdiplus::Pen* pen )
{
	// middle line
	drawLine( d, { center.x, center.y - err }, { center.x, center.y + err }, pen );
	// caps
	drawLine( d, { center.x - capSize, center.y + err }, { center.x + capSize, center.y + err }, pen );
	drawLine( d, { center.x - capSize, center.y - err }, { center.x + capSize, center.y - err }, pen );
}


void PlotCtrl::drawBorder( memDC* d )
{
	long top = plotAreaDims.top * heightScale2, left = plotAreaDims.left * widthScale2,
		right = plotAreaDims.right * widthScale2, bottom = plotAreaDims.bottom * heightScale2;
	drawLine( d, { left, top }, { left, bottom }, greyGdiPen );
	drawLine( d, { left, bottom }, { right, bottom }, greyGdiPen );
	drawLine( d, { right, bottom }, { right, top }, greyGdiPen );
	drawLine( d, { right, top }, { left, top }, greyGdiPen );
}


