#include "stdafx.h"
#include "PlotCtrl.h"


PlotCtrl::PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::string titleIn ) :
	whitePen( PS_SOLID, 0, RGB( 255, 255, 255 ) ),
	greyPen( PS_SOLID, 0, RGB( 100, 100, 100 ) ),
	redPen( PS_SOLID, 0, RGB( 255, 0, 0 ) ),
	solarizedPen( PS_SOLID, 0, RGB( 0, 30, 38 ) ),
	data( dataHolder ), style( inStyle )
{
	for ( auto elem : GIST_RAINBOW_RGB )
	{
		CPen* pen = new CPen( PS_SOLID, 0, RGB( elem[0], elem[1], elem[2] ) );
		pens.push_back( pen );
	}
	title = titleIn;
}


PlotCtrl::~PlotCtrl( )
{
	for ( auto& pen : pens )
	{
		delete pen;
	}
}


void PlotCtrl::drawBackground( memDC* d, double width, double height, CBrush* backgroundBrush )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	RECT r = { controlDims.left * widthScale, controlDims.top*heightScale, controlDims.right * widthScale, 
		controlDims.bottom*heightScale };
	d->SelectObject( *backgroundBrush );
	d->Rectangle( &r );
}


void PlotCtrl::drawTitle( memDC* d, long width, long height )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	RECT scaledArea = { plotAreaDims.left * widthScale, plotAreaDims.top * heightScale,
						plotAreaDims.right * widthScale, plotAreaDims.bottom * heightScale };
	RECT r = { scaledArea.left, scaledArea.top - 30, scaledArea.right, scaledArea.top};
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( RGB( 255, 255, 255 ) );
	d->DrawTextEx( LPSTR( cstr( title ) ), title.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
}


CRect PlotCtrl::GetPlotRect( LONG width, LONG height )
{
	CRect realArea;
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	realArea.left = controlDims.left * widthScale;
	realArea.right = controlDims.right * widthScale;
	realArea.top = controlDims.top * heightScale;
	realArea.bottom = controlDims.bottom * heightScale;
	return realArea;
}


void PlotCtrl::init( POINT topLeftLoc, LONG width, LONG height, CWnd* parent )
{
	controlDims = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + width, topLeftLoc.y + height };
	RECT d = controlDims;
	long w = d.right - d.left, h = d.bottom - d.top;
	plotAreaDims = { long( d.left + w*0.1 ), long(d.top + h*0.08), d.right, long( d.bottom - h*0.1 ) };
	legButton.sPos = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + 22, topLeftLoc.y + 22 };
	legButton.Create( "", NORM_CHECK_OPTIONS, legButton.sPos, parent, 0 );
}


void PlotCtrl::rearrange( int width, int height, fontMap fonts )
{
	legButton.rearrange( width, height, fonts );
}


void PlotCtrl::convertDataToScreenCoords( double width, double height, std::vector<plotDataVec>& screenData )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
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
	if ( style == OscilloscopePlot )
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

	double plotWidthPixels = widthScale * (plotAreaDims.right - plotAreaDims.left);
	double plotHeightPixels = heightScale * (plotAreaDims.bottom - plotAreaDims.top);
	double rangeX = maxx - minx;
	if ( rangeX == 0 )
	{
		rangeX += 1;
	}
	double dataScaleX = plotWidthPixels / (rangeX*1.1);
	double dataHeight = 1;
	double dataMin = 0;
	if ( style == ErrorPlot )
	{
		// currently assuming 0-1 instead of auto-sizing.
		dataHeight = 1;
		dataMin = 0;
	}
	else if ( style == TtlPlot )
	{
		dataHeight = 2;
		// because of offset in plot
		dataMin = -0.5;
	}
	else if ( style == DacPlot )
	{
		dataHeight = 21;
		dataMin = -10;
	}
	else if ( style == OscilloscopePlot )
	{
		dataHeight = maxy-miny;
		dataMin = miny;
	}
	else
	{
		thrower( "ERROR: bad value for plot style???" );
	}
	double dataScaleY = plotHeightPixels / dataHeight;
	 
	for ( auto& line : screenData )
	{
		for ( auto& point : line )
		{
			POINT center = { long( plotAreaDims.left * widthScale + (point.x - minx) * dataScaleX + plotWidthPixels / line.size( ) ),
				long( plotAreaDims.bottom * heightScale - (point.y - miny) * dataScaleY ) };
			point.x = plotAreaDims.left * widthScale + (point.x - minx)* dataScaleX + 10;
			point.y = plotAreaDims.bottom * heightScale - (point.y - dataMin) * dataScaleY;
			point.err *= dataScaleY;
		}
	}
}


void PlotCtrl::plotPoints( memDC* d, double width, double height )
{
	if ( data.size( ) == 0 )
	{
		return;
	}
	std::vector<plotDataVec> screenData;
	std::vector<plotDataVec> shiftedData;
	for ( auto count : range( data.size( ) ) )
	{
		screenData.push_back( *data[count] );
	}
	shiftedData = screenData;
	if ( style == TtlPlot )
	{
		shiftTtlData( shiftedData );
	}
	convertDataToScreenCoords( width, height, shiftedData );
	std::vector<double> xRaw, xScaled;
	for ( auto count : range( shiftedData[0].size( ) ) )
	{
		xRaw.push_back( data[0]->at( count ).x );
		xScaled.push_back( shiftedData[0][count].x );
	}
	
	std::pair<double, double> minMaxScaled, minMaxRaw;
	if ( style == OscilloscopePlot )
	{
		getMinMaxY( screenData, data, minMaxRaw, minMaxScaled );
	}
	drawGridAndAxes( d, xRaw, xScaled, width, height, minMaxRaw, minMaxScaled );
	UINT penNum = 0;
	UINT lineNum = 0;
	for ( auto& line : shiftedData )
	{
		if ( lineNum == shiftedData.size( ) - 1 )
		{
			// average data set is white.
			d->SelectObject( whitePen );
		}
		else
		{
			// TODO: handle alpha here...
			d->SelectObject( pens[penNum] );
			penNum += int( 256 / shiftedData.size( ) );
		}
		if ( style == ErrorPlot )
		{
			UINT count = 0;
			for ( auto point : line )
			{
				if ( data[lineNum]->at( count ).y <= 1 && data[lineNum]->at( count ).y >= 0 )
				{
					circleMarker( d, { long( point.x ), long( point.y ) }, 10 );
					errBars( d, { long( point.x ), long( point.y ) }, point.err, 10 );
				}
				count++;
			}
		}
		else if ( style == TtlPlot || style == DacPlot )
		{
			makeStepPlot( d, width, height, line );
		}
		else if ( style == OscilloscopePlot )
		{
			makeLinePlot( d, width, height, line );
		}

		lineNum++;
	}
	if ( legButton && legButton.GetCheck( ) )
	{
		drawLegend( d, width, height, shiftedData );
	}
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
	for ( auto line : rawData )
	{
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

void PlotCtrl::makeLinePlot( memDC* d, LONG width, LONG height, plotDataVec scaledLine )
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
		drawLine( d, { long( prevPoint.x ), long( prevPoint.y ) }, { long( point.x ), long( point.y ) } );
		prevPoint = point;
	}
}


void PlotCtrl::makeStepPlot( memDC* d, LONG width, LONG height, plotDataVec scaledLine )
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
		drawLine( d, { long( prevPoint.x ), long( prevPoint.y ) }, { long( point.x ), long( prevPoint.y ) } );
		// using the same x makes this a vertical line.
		drawLine( d, { long( point.x ), long( prevPoint.y ) }, { long( point.x ), long( point.y ) } );
		if ( point.y != prevPoint.y )
		{
			circleMarker( d, { long( point.x ), long( point.y ) }, 10 );
		}
		prevPoint = point;
	}
}


void PlotCtrl::drawGridAndAxes( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX, double width,
								double height, std::pair<double, double> minMaxRawY,
								std::pair<double, double> minMaxScaledY )
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
	// assumes that data has been converted to screen coords.
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	RECT scaledArea = { plotAreaDims.left * widthScale, plotAreaDims.top * heightScale,
		plotAreaDims.right * widthScale, plotAreaDims.bottom * heightScale };
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( RGB( 255, 255, 255 ) );
	UINT count = 0;
	bool labelEachPoint = (scaledX.size( ) < 15);
	for ( auto x : scaledX )
	{
		// draw vertical lines for x points
		RECT r = { long( scaledArea.left - 10 ), long( scaledArea.bottom + 5 ), long( scaledArea.left + 10 ), long( scaledArea.bottom + 25) };
		std::string txt = str( xAxisPts[count] );
		if ( labelEachPoint )
		{
			drawLine( d, x, scaledArea.bottom + 5, x, scaledArea.top );
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
			RECT r = { long( scaledArea.left + count * scaledWidth/10.0 - 40), long( scaledArea.bottom + 5 ),
					   long( scaledArea.left + 60 + count * scaledWidth / 10.0), long( scaledArea.bottom + 25 ) };
			std::string txt = str( xMin + count * dataRange / 10.0 );
			drawLine( d, scaledArea.left + count * scaledWidth / 10.0 + 10, scaledArea.bottom + 5,
					  scaledArea.left + count * scaledWidth / 10.0 + 10, scaledArea.top );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}
	}
	UINT numLines = 6;
	double vStep = (scaledArea.bottom - scaledArea.top) / (numLines - 1);
	double vRawStep = (minMaxRawY.second - minMaxRawY.first) / (numLines - 1);
	// min to max version
	if ( style == TtlPlot )
	{
		// no left to right gridlines.
	}
	else 
	{
		double minY, maxY;
		if ( style == DacPlot )
		{
			minY = -10;
			maxY = 10;
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
					  scaledArea.top + gridline * vStep );
			RECT r = { long( scaledArea.left - 55 ), long( scaledArea.top + 10 + gridline * vStep ),
					   long( scaledArea.left - 5 ), long( scaledArea.top - 10 + gridline * vStep ) };
			std::string txt = str( maxY - (maxY - minY) * double( gridline ) / (numLines - 1), 5 );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}

		if ( false )
		{
			double vStep = 0;// (minMaxScaled.top - minMaxScaled.bottom) / (numLines - 1);
			for ( auto gridline : range( numLines ) )
			{
				drawLine( d, scaledArea.left - 5, minMaxScaledY.second - gridline * vStep, scaledArea.right,
						  minMaxScaledY.second - gridline * vStep );
				RECT r = { long( scaledArea.left - 55 ), long( minMaxScaledY.second + 10 - gridline * vStep ),
					long( scaledArea.left - 5 ), long( minMaxScaledY.second - 10 - gridline * vStep ) };
				std::string txt = str( minMaxRawY.first + gridline * vRawStep, 5 );
				d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
			}
		}

	}

	// axis labels
	RECT r = { scaledArea.left, scaledArea.bottom + 30 , scaledArea.right, scaledArea.bottom + 50 };
	std::string txt = "xlabel";
	d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	r = { long( controlDims.left * widthScale - 50 ), scaledArea.top, scaledArea.left, scaledArea.bottom };
	if ( style == TtlPlot )
	{
		txt = "Ttl State";
	}
	d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
}


void PlotCtrl::drawLegend( memDC* d, UINT width, UINT height, std::vector<plotDataVec> screenData )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	UINT plotAreaWidth = (plotAreaDims.right - plotAreaDims.left) * widthScale;
	UINT plotAreaHeight = (plotAreaDims.bottom - plotAreaDims.top) * heightScale;
	UINT itemNum = screenData.size( );
	UINT itemsPerColumn = 21;
	UINT totalColNum = int(screenData.size() / itemsPerColumn );
	UINT penNum = 0;
	for ( auto lineNum : range( screenData.size() ) )
	{
		if ( lineNum == screenData.size( ) - 1 )
		{
			// average data set is white.
			d->SelectObject( whitePen );
		}
		else
		{
			// TODO: handle alpha here...
			d->SelectObject( pens[penNum] );
			penNum += int( 256 / screenData.size( ) );
		}
		UINT rowNum = lineNum % itemsPerColumn;
		UINT colNum = lineNum / itemsPerColumn;
		POINT legendLoc = { plotAreaDims.left * widthScale + (1 - (colNum + 1) * 0.075) * plotAreaWidth,
			plotAreaDims.top * heightScale + 0.025 * plotAreaHeight + 20 * rowNum };
		circleMarker( d, legendLoc, 10 );
		RECT r = { long( legendLoc.x + 10 ), long( legendLoc.y + 10 ),
			long( plotAreaDims.right * widthScale - colNum * 0.075 * plotAreaWidth ), long( legendLoc.y - 10 ) };
		d->DrawTextEx( const_cast<char *>(cstr( lineNum)), str( lineNum ).size( ), &r,
					   DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	}
}


void PlotCtrl::drawLine( CDC* d, double begX, double begY, double endX, double endY )
{

	d->MoveTo( { long( begX ), long( begY ) } );
	d->LineTo( { long( endX ), long( endY ) } );
}


void PlotCtrl::drawLine( CDC* d, POINT beg, POINT end )
{
	d->MoveTo( { beg.x, beg.y } );
	d->LineTo( { end.x, end.y } );
}


void PlotCtrl::circleMarker( memDC* d, POINT loc, double size )
{
	int x1, x2, y1, y2;
	x1 = loc.x - size / 2;
	y1 = loc.y - size / 2;
	x2 = x1 + size;
	y2 = y1 + size;
	d->Ellipse( x1, y1, x2, y2 );
}


void PlotCtrl::errBars( memDC* d, POINT center, long err, long capSize )
{
	// middle line
	drawLine( d, { center.x, center.y - err }, { center.x, center.y + err } );
	// caps
	drawLine( d, { center.x - capSize, center.y + err }, { center.x + capSize, center.y + err } );
	drawLine( d, { center.x - capSize, center.y - err }, { center.x + capSize, center.y - err } );
}


void PlotCtrl::drawBorder( memDC* d, double width, double height )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	d->SelectObject( redPen );
	long top = controlDims.top * heightScale, left = controlDims.left * widthScale, right = controlDims.right * widthScale,
		bottom = controlDims.bottom * heightScale;
	drawLine( d, { left, top }, { left, bottom } );
	drawLine( d, { left, bottom }, { right, bottom } );
	drawLine( d, { right, bottom }, { right, top } );
	drawLine( d, { right, top }, { left, top } );

	d->SelectObject( whitePen );
	top = plotAreaDims.top * heightScale, left = plotAreaDims.left * widthScale,
		right = plotAreaDims.right * widthScale, bottom = plotAreaDims.bottom * heightScale;
	drawLine( d, { left, top }, { left, bottom } );
	drawLine( d, { left, bottom }, { right, bottom } );
	drawLine( d, { right, bottom }, { right, top } );
	drawLine( d, { right, top }, { left, top } );
}


