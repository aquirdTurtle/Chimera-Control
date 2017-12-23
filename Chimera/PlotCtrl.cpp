#include "stdafx.h"
#include "PlotCtrl.h"
#include "limits.h"
#include <algorithm>
#include "memdc.h"

PlotCtrl::PlotCtrl( std::vector<pPlotDataVec> dataHolder ) :
															 whitePen( PS_SOLID, 0, RGB( 255, 255, 255 ) ),
															 greyPen( PS_SOLID, 0, RGB( 100, 100, 100 ) ),
															 redPen( PS_SOLID, 0, RGB( 255, 0, 0 ) ),
															 solarizedPen( PS_SOLID, 0, RGB( 0, 30, 38 ) ),
															 data( dataHolder )
{
	backgroundBrush.CreateSolidBrush( RGB( 0, 30, 38 ) );
	for ( auto elem : GIST_RAINBOW_RGB )
	{
		CPen* pen = new CPen( PS_SOLID, 0, RGB( elem[0], elem[1], elem[2] ) );
		pens.push_back( pen );
	}
}


PlotCtrl::~PlotCtrl( )
{
	for ( auto& pen : pens )
	{
		delete pen;
	}
}


IMPLEMENT_DYNAMIC( PlotCtrl, CDialog )

BEGIN_MESSAGE_MAP( PlotCtrl, CDialog )
	ON_WM_CTLCOLOR( )
	ON_WM_PAINT( )
	ON_WM_SIZE( )
	ON_WM_TIMER()
END_MESSAGE_MAP()


void PlotCtrl::OnTimer( UINT_PTR id )
{
	// in principle could just invalidate rect here.
	OnPaint( );
}

void PlotCtrl::OnSize( UINT s, int cx, int cy)
{
	CDialog::OnSize( s, cx, cy );
	OnPaint( );
}


HBRUSH PlotCtrl::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	return backgroundBrush;
}


BOOL PlotCtrl::OnInitDialog( )
{
	// should fill this dialog.
	UINT_PTR test = SetTimer( 1, 1000, NULL );
	init( { 0, 0 }, 1920, 997);
	return TRUE;
}


void PlotCtrl::OnPaint( )
{
	CRect size;
	GetClientRect( &size );
	memDC dc( GetDC() );
	drawBackground( dc, size.right - size.left, size.bottom - size.top );
	drawBorder( dc, size.right - size.left, size.bottom - size.top );
	plotPoints( &dc, size.right - size.left, size.bottom - size.top );
	CDialog::OnPaint( );
}


void PlotCtrl::drawBackground( memDC* d, double width, double height )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	RECT r = { 0,0, controlDims.right * widthScale, controlDims.bottom*heightScale};
	d->SelectObject( backgroundBrush );
	d->Rectangle( &r );
}


void PlotCtrl::init( POINT topLeftLoc, LONG width, LONG height )
{
	controlDims = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + width, topLeftLoc.y + height };
	RECT d = controlDims;
	long w = d.right - d.left, h = d.bottom - d.top;
	plotAreaDims = { long(d.left + w*0.1), d.top, d.right, long(d.bottom - h*0.1) };
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
			/*
			if ( elem.y - elem.err < miny )
			{
				miny = elem.y - elem.err;
			}
			if ( elem.y + elem.err > maxy )
			{
				maxy = elem.y + elem.err;
			}
			*/
		}
	}
	
	double realWidth = widthScale * (plotAreaDims.right - plotAreaDims.left);
	double realHeight = heightScale * (plotAreaDims.bottom - plotAreaDims.top);
	double rangeX = maxx - minx;
	if ( rangeX == 0 )
	{
		rangeX += 1;
	}
	double rangeY = maxy - miny;
	if ( rangeY == 0 )
	{
		rangeY += 1;
	}
	double dataScaleX = realWidth / (rangeX*1.1);
	//double dataScaleY = realHeight / (rangeY*1.1);
	double dataScaleY = realHeight / 1;
	for ( auto& line : screenData )
	{
		for ( auto& point : line )
		{
			POINT center = { long( plotAreaDims.left * widthScale + (point.x - minx) * dataScaleX + realWidth / line.size( ) ),
							 long( plotAreaDims.bottom * heightScale - (point.y - miny) * dataScaleY ) };
			point.x = plotAreaDims.left * widthScale + (point.x - minx)* dataScaleX + realWidth / line.size( );
			point.y = plotAreaDims.bottom * heightScale - point.y * dataScaleY;
			point.err *= dataScaleY;
		}
	}
}


void PlotCtrl::plotPoints( memDC* d, double width, double height )
{
	std::vector<plotDataVec> screenData;
	for ( auto count : range( data.size() ) )
	{
		screenData.push_back( *data[count] );
	}
	convertDataToScreenCoords( width, height, screenData );
	std::vector<double> xRaw, xScaled;
	for ( auto count : range( screenData[0].size( ) ) )
	{
		xRaw.push_back( data[0]->at( count ).x );
		xScaled.push_back( screenData[0][count].x );
	}
	std::pair<double, double> minMaxScaled, minMaxRaw;
	drawGridAndAxis( d, xRaw, xScaled, width, height, minMaxRaw, minMaxScaled );
	UINT penNum = 0;
	UINT lineNum = 0;
	for ( auto& line : screenData )
	{
		if ( lineNum == screenData.size() - 1 )
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
		drawLegend( d, width, height, lineNum );		
		UINT count = 0;
		for ( auto point : line )
		{
			if ( data[lineNum]->at(count).y <= 1 && data[lineNum]->at( count ).y >= 0 )
			{
				circleMarker( d, { long( point.x ), long( point.y ) }, 10 );
				errBars( d, { long( point.x ), long( point.y ) }, point.err, 10 );
			}
			count++;
		}
		lineNum++;
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


void PlotCtrl::drawGridAndAxis( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX, double width,
								double height, std::pair<double, double> minMaxRawY, 
								std::pair<double, double> minMaxScaledY )
{
	// assumes that data has been converted to screen coords.
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	RECT scaledArea = { plotAreaDims.left * widthScale, plotAreaDims.top * heightScale, 
						plotAreaDims.right * widthScale, plotAreaDims.bottom * heightScale };
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( RGB( 255, 255, 255 ) );
	UINT count = 0;
	for ( auto x : scaledX )
	{
		// draw vertical lines for x points
		drawLine( d, x, scaledArea.bottom + 5, x, scaledArea.top );
		RECT r = { long( x - 10 ), long( scaledArea.bottom + 5 ), long( x + 10 ), long( scaledArea.bottom + 25 ) };
		std::string txt = str(xAxisPts[count] );
		d->DrawTextEx( LPSTR(cstr(txt)), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		count++;
	}
	UINT numLines = 5;
	double vStep = (scaledArea.bottom - scaledArea.top ) / (numLines - 1 );
	double vRawStep = (minMaxRawY.second - minMaxRawY.first) / (numLines - 1);
	// Forces y-axis to be between 0 and 1.
	for ( auto gridline : range( numLines ) )
	{
		drawLine( d, scaledArea.left - 5, scaledArea.top + gridline * vStep, scaledArea.right, 
				  scaledArea.top + gridline * vStep );
		RECT r = { long( scaledArea.left - 55 ), long( scaledArea.top + 10 + gridline * vStep ),
				   long( scaledArea.left - 5 ), long( scaledArea.top - 10 + gridline * vStep ) };
		std::string txt = str( 1 - double(gridline) / numLines, 5 );
		d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	}

	// min to max version
	if ( false )
	{
		double vStep = 0;// (minMaxScaled.top - minMaxScaled.bottom) / (numLines - 1);
		for ( auto gridline : range( numLines ) )
		{
			drawLine( d, scaledArea.left - 5, minMaxScaledY.second - gridline * vStep, scaledArea.right,
					  minMaxScaledY.second - gridline * vStep );
			RECT r = {  long( scaledArea.left - 55 ), long( minMaxScaledY.second + 10 - gridline * vStep ),
						long( scaledArea.left - 5 ), long( minMaxScaledY.second - 10 - gridline * vStep ) };
			std::string txt = str( minMaxRawY.first + gridline * vRawStep, 5 );
			d->DrawTextEx( LPSTR(cstr(txt)), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}
	}

	// axis labels
	RECT r = { scaledArea.left, scaledArea.bottom + 30 , scaledArea.right, scaledArea.bottom + 50 };
	std::string txt = "xlabel";
	d->DrawTextEx( LPSTR(cstr(txt) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	r = { long(controlDims.left * widthScale - 50), scaledArea.top, scaledArea.left, scaledArea.bottom };
	txt = "ylabel";
	d->DrawTextEx( LPSTR( cstr(txt) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
}


void PlotCtrl::drawLegend( memDC* d, UINT width, UINT height, UINT lineNum )
{
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	UINT plotAreaWidth = (plotAreaDims.right - plotAreaDims.left) * widthScale;
	UINT plotAreaHeight = (plotAreaDims.bottom - plotAreaDims.top) * heightScale;
	POINT legendLoc = { plotAreaDims.left * widthScale + 0.9 * plotAreaWidth, 
						plotAreaDims.top * heightScale + 0.025 * plotAreaHeight + 20 * lineNum };
	circleMarker( d, legendLoc, 10 );
	RECT r =  { long( legendLoc.x+10 ), long( legendLoc.y + 10),
				long( plotAreaDims.right * widthScale ), long( legendLoc.y - 10) };
	d->DrawTextEx( const_cast<char *>(cstr( lineNum+1 )), str( lineNum+1 ).size( ), &r,
				   DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
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
	x1 = loc.x - size/2;
	y1 = loc.y - size/2;
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
	drawLine( d, { right, bottom}, { right, top } );
	drawLine( d, { right, top }, { left, top} );

	d->SelectObject( whitePen );
	top = plotAreaDims.top * heightScale, left = plotAreaDims.left * widthScale, 
		right = plotAreaDims.right * widthScale, bottom = plotAreaDims.bottom * heightScale;
	drawLine( d, { left, top }, { left, bottom } );
	drawLine( d, { left, bottom }, { right, bottom } );
	drawLine( d, { right, bottom }, { right, top } );
	drawLine( d, { right, top }, { left, top } );
}