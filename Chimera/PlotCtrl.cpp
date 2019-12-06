// created by Mark O. Brown
#include "stdafx.h"
#include "PlotCtrl.h"
#include "PlotDialog.h"
#include <numeric>

PlotCtrl::PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::vector<Gdiplus::Pen*> pensIn,
					CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::vector<int> thresholds_in,
					std::string titleIn, bool narrowOpt, bool plotHistOption ) :
	whitePen( PS_SOLID, 0, RGB( 255, 255, 255 ) ),
	greyPen( PS_SOLID, 0, RGB( 100, 100, 100 ) ),
	data( dataHolder ), style( inStyle ), dataMutexes( dataHolder.size( ) ), textFont( font ),
	brushes( plotBrushes ), pens( pensIn ), narrow(narrowOpt )
{
	thresholds = thresholds_in;
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
	errBox ( "Deleteing Plot Ctrl!" );
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


void PlotCtrl::drawPlot ( CDC* cdc, CBrush* backgroundBrush, CBrush* plotAreaBrush )
{
	memDC dc ( cdc, &GetPlotRect() );
	drawBackground ( dc, backgroundBrush, plotAreaBrush );
	drawTitle ( dc );
	drawBorder ( dc );
	plotPoints ( &dc );
}

std::vector<pPlotDataVec> PlotCtrl::getCurrentData ( )
{
	return data;
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
	RECT r = { scaledArea.left, scaledArea.top - 40, scaledArea.right, scaledArea.top};
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( _myRGBs[ "Text" ] );
	d->SelectObject( textFont );
	d->DrawTextEx( LPSTR( cstr( title ) ), title.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_TOP, NULL );
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

void PlotCtrl::setData (std::vector<pPlotDataVec> newData)
{
	for (auto& dvec : data)
	{
		dvec.reset ();
	}
	data.clear ();
	data = newData;
}

void PlotCtrl::setTitle (std::string newTitle)
{
	title = newTitle;
}

void PlotCtrl::setThresholds (std::vector<int> newThresholds)
{
	thresholds = newThresholds;
}

void PlotCtrl::setStyle (plotStyle newStyle)
{
	style = newStyle;
}

void PlotCtrl::init( POINT& topLeftLoc, LONG width, LONG height, CWnd* parent, int popId )
{ 
	setControlLocation ( topLeftLoc, width, height );
 	legButton.sPos = { topLeftLoc.x, topLeftLoc.y, topLeftLoc.x + 22, topLeftLoc.y + 22 };
	legButton.Create( "", NORM_CHECK_OPTIONS, legButton.sPos, parent, 0 );
 	sustainButton.sPos = { topLeftLoc.x, topLeftLoc.y+22, topLeftLoc.x + 22, topLeftLoc.y + 44 };
	sustainButton.Create( "", NORM_CHECK_OPTIONS, sustainButton.sPos, parent, 0 );
	popDlgBttn.sPos = { topLeftLoc.x, topLeftLoc.y+44, topLeftLoc.x + 22, topLeftLoc.y + 66 };
	popDlgBttn.Create ("Pop", NORM_PUSH_OPTIONS, popDlgBttn.sPos, parent, popId);
	popDlgBttn.fontType = fontTypes::SmallFont;
	topLeftLoc.y += height;
} 
 
 
bool PlotCtrl::wantsSustain( )
{
	return sustainButton.GetCheck( );
}


void PlotCtrl::rearrange( int width, int height, fontMap fonts )
{
	legButton.rearrange( width, height, fonts );
	sustainButton.rearrange( width, height, fonts );
	popDlgBttn.rearrange (width, height, fonts);
}

/*
Gives the limits of the x points of the raw data.
*/
void PlotCtrl::getDataXLims (std::vector<plotDataVec>& data )
{
	auto& minx = data_minmax.min_x;
	auto& maxx = data_minmax.max_x;
	minx = DBL_MAX;
	maxx = -DBL_MAX;
	for ( auto lineInc : range ( data.size ( ) - 1 ) )
	{
		auto line = data[ lineInc ];
		for ( auto elem : line )
		{
			minx = elem.x < minx ? elem.x : minx;
			maxx = elem.x > maxx ? elem.x : maxx;
		}
	}
	if ( style == plotStyle::HistPlot )
	{
		// then check thresholds as well.
		for ( auto t : thresholds )
		{
			minx = t < minx ? t : minx;
			maxx = t > maxx ? t : maxx;
		}
	}
}

/*
	these are the limits of the data itself. 
*/
void PlotCtrl::getDataYLims ( std::vector<plotDataVec>& data )
{
	auto& miny = data_minmax.min_y;
	auto& maxy = data_minmax.max_y;
	for (auto line : data)
	{
		for (auto elem : line)
		{
			if (elem.y < miny)
			{
				miny = elem.y;
			}
			if (elem.y > maxy)
			{
				maxy = elem.y;
			}
		}
	}
	//if ( style == plotStyle::OscilloscopePlot || style == plotStyle::HistPlot || style == plotStyle::DacPlot
	//	 || style == plotStyle::VertHist ) {}
	if ( style == plotStyle::VertHist || style == plotStyle::HistPlot )
	{
		miny = 0;
	}
}


bool PlotCtrl::handlePop (UINT id, CWnd* parent)
{
	if (id == popDlgBttn.GetDlgCtrlID ())
	{
		// start a new PlotDialog dialog
		PlotDialog* plot = new PlotDialog ( data, style, pens, textFont, brushes, 1000, { 0,0,0,0 }, 
											popDlgBttn.GetDlgCtrlID (), title );
		plot->Create (IDD_PLOT_DIALOG, parent);
		plot->ShowWindow (SW_SHOW);
		return true;
	}
	return false;
}


void PlotCtrl::convertDataToScreenCoords( std::vector<plotDataVec>& screenData, std::vector<long>& scaledThresholds )
{
	if ( screenData.size ( ) == 0 )
	{
		return;
	}
	getDataXLims ( screenData );
	getDataYLims ( screenData );
	getXLims ( );
	getYLims ( );
	double plotWidthPixels = widthScale2 * (plotAreaDims.right - plotAreaDims.left);
	double plotHeightPixels = heightScale2 * (plotAreaDims.bottom - plotAreaDims.top);

	// extra term is to make sure this is never zero, adding 1 if would be too close.
	double rangeX = view_minmax.max_x - view_minmax.min_x + ( fabs( view_minmax.max_x - view_minmax.min_x ) < 1e-12 ? 1 : 0 );
	// pixels per unit of the xpts
	double dataScaleX = plotWidthPixels / rangeX;
	// not positive why the 0.99 is necessary here
	boxWidthPixels = ceil ( boxWidth * dataScaleX * 0.99 );	
	/// handle flipped data
	if ( style == plotStyle::VertHist )
	{ 
		// flipped
		double dataScaleX = plotWidthPixels / ( view_minmax.max_y - view_minmax.min_y);
		double dataScaleY = plotHeightPixels / rangeX;
		boxWidthPixels = ceil ( boxWidth * dataScaleY * 0.99 );
		for ( auto& line : screenData )
		{
			for ( auto& point : line )
			{
				auto px = point.x;
				auto py = point.y;
				point.x = plotAreaDims.left * widthScale2 + ( py - view_minmax.min_y ) * dataScaleX + 10;
				point.y = plotAreaDims.bottom * heightScale2 - ( px - view_minmax.min_x ) * dataScaleY;
				point.err *= dataScaleY;
			}
		}
	}
	else
	{ /// scale normal data
		double dataScaleY = plotHeightPixels / ( view_minmax.max_y - view_minmax.min_y );

		for ( auto& line : screenData )
		{
			for ( auto& point : line )
			{
				point.x = plotAreaDims.left * widthScale2 + ( point.x - view_minmax.min_x ) * dataScaleX;
				point.y = plotAreaDims.bottom * heightScale2 - ( point.y - view_minmax.min_y ) * dataScaleY;
				point.err *= dataScaleY;
			}
		}
	}
	/// handle thresholds
	if ( style == plotStyle::HistPlot )
	{
		scaledThresholds.clear ( );
		for ( auto t : thresholds )
		{
			scaledThresholds.push_back ( plotAreaDims.left * widthScale2 + ( t - view_minmax.min_x ) * dataScaleX );
		}
	}
}

/*
the modified minx and maxx refer to the minimum and maximum data points which *could* appear on the plot.
*/
void PlotCtrl::getXLims ()
{
	auto& minx = view_minmax.min_x, &maxx = view_minmax.max_x;
	auto min_d_x = data_minmax.min_x, max_d_x = data_minmax.max_x;
	// in data's coordinates, not screen coordinates
	if ( style == plotStyle::HistPlot )
	{
		// resize things to take into acount the widths.
		maxx = max_d_x + boxWidth;
		minx = min_d_x - boxWidth;
	}
	else
	{
		minx = min_d_x;
		maxx = max_d_x;
	}
	// maximum points on the plot are slightly outside data
	minx -= ( max_d_x - min_d_x ) * 0.05;
	maxx += ( max_d_x - min_d_x ) * 0.05;
}

/*
the modified minx and maxx refer to the minimum and maximum data points which *could* appear on the plot.
*/
void PlotCtrl::getYLims ( )
{
	auto& miny = view_minmax.min_y, &maxy = view_minmax.max_y;
	auto min_d_y = data_minmax.min_y, max_d_y = data_minmax.max_y;
	// in data's coordinates, not screen coordinates
	if ( style == plotStyle::ErrorPlot )
	{
		// currently assuming 0-1 instead of auto-sizing.
		maxy = 1;
		miny = 0;
	}
	else if ( style == plotStyle::TtlPlot )
	{
		maxy = 1.5;
		// because of offset in plot
		miny = -0.5;
	}
	else if ( style == plotStyle::DacPlot )
	{
		// currently doing autoscaling here.
		maxy = max_d_y;
		miny = min_d_y;
	}
	else if ( style == plotStyle::OscilloscopePlot )
	{
		auto r = max_d_y - min_d_y;
		maxy = min_d_y + r * 1.1;
		miny = min_d_y - r*0.05;
	}
	else if ( style == plotStyle::HistPlot || style == plotStyle::VertHist )
	{
		maxy = max_d_y;
		miny = 0;
	}
	else
	{
		thrower ( "Bad value for plot style???  (A low level bug, this shouldn't happen)" );
	}
}


double PlotCtrl::getRelativeScreenLoc ( double dataVal, double limMin, double limMax )
{
	return ( dataVal - limMin ) / ( limMax - limMin );
}


void PlotCtrl::clear ( )
{
	pens.clear ( );
	brushes.clear ( );
}


void PlotCtrl::plotPoints ( memDC* d )
{
	if ( data.size ( ) == 0 )
	{
		return;
	}
	std::vector<plotDataVec> screenData;
	std::vector<plotDataVec> shiftedData;
	std::vector<long> scaledThresholds;
	{
		for ( auto lineCount : range ( data.size ( ) ) )
		{
			//std::lock_guard<std::mutex> lock ( dataMutexes[ lineCount ] );
			screenData.push_back ( *data[ lineCount ] );
		}
	}
	shiftedData = screenData;
	if ( style == plotStyle::TtlPlot )
	{
		shiftTtlData ( shiftedData );
	}
	convertDataToScreenCoords ( shiftedData, scaledThresholds );
	if ( style == plotStyle::HistPlot && shiftedData.size ( ) != thresholds.size ( )+1 )
	{
		if ( scaledThresholds.size ( ) == 1)
		{
			scaledThresholds = std::vector<long> ( shiftedData.size ( ), scaledThresholds[0] );
		}
		else
		{
			scaledThresholds = std::vector<long> ( shiftedData.size ( ), 0 );
			//thrower ( "Threshold number not compatible with data set number!" );
		}
	}
	std::vector<double> xRaw, xScaled;
	{
		//std::lock_guard<std::mutex> lock ( dataMutexes[ 0 ] );
		if ( style == plotStyle::HistPlot )
		{
			for ( auto lineInc : range ( shiftedData.size ( )-1 ))
			{
				for ( auto pointCount : range ( shiftedData[ lineInc ].size ( ) ) )
				{
					if ( std::find ( xScaled.begin ( ), xScaled.end ( ), shiftedData[ lineInc ][ pointCount ].x ) == xScaled.end ( ) )
					{
						xRaw.push_back ( data[ lineInc ]->at ( pointCount ).x );
						xScaled.push_back ( shiftedData[ lineInc ][ pointCount ].x );
					}
				}
			}
		}
		else
		{
			for ( auto pointCount : range ( shiftedData[ 0 ].size ( ) ) )
			{
				xRaw.push_back ( data[ 0 ]->at ( pointCount ).x );
				xScaled.push_back ( shiftedData[ 0 ][ pointCount ].x );
			}
		}
	}
	drawGridAndAxes( d, xRaw, xScaled);
	UINT penNum = 0;
	UINT lineNum = 0;
	for ( auto lineNum : range(shiftedData.size()) )
	{
		auto& line = shiftedData[ lineNum ];
		if ( style == plotStyle::HistPlot && lineNum == shiftedData.size ( ) - 1 )
		{
			break;
		}
		long thresh;
		if ( style == plotStyle::HistPlot )
		{
			thresh = scaledThresholds[ lineNum ];
		}
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
			//std::lock_guard<std::mutex> lock( dataMutexes[lineNum] );
			for ( auto point : line )
			{
				if ( data[lineNum]->at( pointCount ).y <= 1 && data[lineNum]->at( pointCount ).y >= 0 )
				{
					UINT circSize = point.err < 3 ? 2 * point.err : 6;
					circleMarker( d, { long( point.x ), long( point.y ) }, circSize, brush );
					errBars( d, { long( point.x ), long( point.y ) }, point.err, 4, pen );
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
		else if ( style == plotStyle::HistPlot )
		{
			makeBarPlot( d, line, brush );
			pen->SetWidth ( 3 );
			drawThresholds ( d, thresh, pen );
			pen->SetWidth ( 1 );
		}
		else if ( style == plotStyle::VertHist )
		{
			makeBarPlot ( d, line, brush );
		}
		lineNum++;
	}
	if ( legButton && legButton.GetCheck( ) )
	{
		drawLegend( d, shiftedData );
	}
	if (data.size () > 0 && data.back()->size() == 1 && style == plotStyle::ErrorPlot)
	{
		// if single data point, then print the avg on the plot.
		RECT r = { plotAreaDims.left * widthScale2, plotAreaDims.top * heightScale2,
				   plotAreaDims.right * widthScale2, plotAreaDims.bottom * heightScale2 };
		auto avgPt = data.back ()->at(0).y;
		d->DrawTextEx (const_cast<char*>( cstr (avgPt)), str (avgPt).size (), &r,
										  DT_CENTER | DT_SINGLELINE | DT_CENTER, NULL);
	}
}


void PlotCtrl::makeBarPlot( memDC* d, plotDataVec scaledLine, Gdiplus::SolidBrush* brush )
{
	if ( scaledLine.size ( ) == 0 )
	{
		return;
	}
	for ( auto ptNum : range(scaledLine.size()-1) )
	{
		auto& p = scaledLine[ ptNum ];
		auto& np = scaledLine[ ptNum + 1 ];
		Gdiplus::Rect r;
		if ( style == plotStyle::HistPlot )
		{
			auto w = ceil(np.x) - ceil(p.x);
			r = Gdiplus::Rect ( ceil(p.x), ceil(p.y), w,
								ceil(plotAreaDims.bottom * heightScale2 - ceil(p.y)) );
		}
		else if ( style == plotStyle::VertHist )
		{
			r = Gdiplus::Rect ( ceil( plotAreaDims.left * widthScale2 ), ceil(np.y),
								ceil( ceil(np.x) - plotAreaDims.left * widthScale2 ), ceil(p.y)-ceil(np.y) );
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


void PlotCtrl::drawGridAndAxes( memDC* d, std::vector<double> xAxisPts, std::vector<double> scaledX)
{
	// assumes that data has been converted to screen coords.
	RECT scaledArea = { plotAreaDims.left * widthScale2, plotAreaDims.top * heightScale2,
		plotAreaDims.right * widthScale2, plotAreaDims.bottom * heightScale2 };
	d->SelectObject( greyPen );
	d->SetBkMode( TRANSPARENT );
	d->SetTextColor( _myRGBs["Text"] );
	UINT count = 0;
	if ( scaledX.size ( ) < 15 )
	{
		// then label each point
		for ( auto x : scaledX )
		{
			// draw vertical lines for x points
			RECT r = { long ( x - 40 ), long ( scaledArea.bottom ), long ( x + 60 ),
				long ( controlDims.bottom * heightScale2 ) };
			std::string txt = str ( xAxisPts[ count ], 4 );
			drawLine( d, x, scaledArea.bottom + 5, x, scaledArea.top, greyGdiPen );
			d->SelectObject( textFont );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
			count++;
		}
	}
	else
	{
		double scaledWidth = scaledArea.right - scaledArea.left;
		double dataRange = view_minmax.max_x - view_minmax.min_x;
		int numLines = 8;
		for ( auto count : range( numLines ) )
		{
			// a box for the x tick text.
			RECT r = { long ( scaledArea.left + count * ( scaledWidth - 20 ) / ( numLines - 1 ) - 40 ), long ( scaledArea.bottom ),
					   long ( scaledArea.left + count * ( scaledWidth - 20 ) / ( numLines - 1 ) + 60 ),
					   long ( controlDims.bottom * heightScale2 ) };
			std::string txt = str ( view_minmax.min_x + count * dataRange / ( numLines - 1 ), 4 );
			drawLine ( d, scaledArea.left + count * ( scaledWidth) / ( numLines - 1 ), scaledArea.bottom + 5,
					   scaledArea.left + count * ( scaledWidth) / ( numLines - 1 ), scaledArea.top, greyGdiPen );
			d->SelectObject( textFont );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}
	}
	UINT numLines = 6;
	double vStep = (scaledArea.bottom - scaledArea.top) / (numLines - 1);
	double vRawStep = ( view_minmax.max_y - view_minmax.min_y ) / (numLines - 1);
	// min to max version
	if ( style == plotStyle::TtlPlot )
	{
		// no left to right gridlines.
	}
	else 
	{
		// Forces y-axis to be between 0 and 1.
		for ( auto gridline : range( numLines ) )
		{
			drawLine( d, scaledArea.left - 5, scaledArea.top + gridline * vStep, scaledArea.right,
					  scaledArea.top + gridline * vStep, greyGdiPen );
			RECT r = { long( controlDims.left * widthScale2 ), long( scaledArea.top + 10 + gridline * vStep ),
					   long( scaledArea.left ), long( scaledArea.top - 10 + gridline * vStep ) };
			std::string txt = str( view_minmax.max_y - ( view_minmax.max_y - view_minmax.min_y ) 
								   * double(gridline) / (numLines - 1), 5 );
			d->SelectObject( textFont );
			d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
		}
	}
	// axis labels
	RECT r = { scaledArea.left, scaledArea.bottom+10, scaledArea.right, controlDims.bottom * heightScale2 + 10 };
	std::string txt = "xlabel";
	d->SelectObject( textFont );
	d->DrawTextEx( LPSTR( cstr( txt ) ), txt.size( ), &r, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	r = { long( controlDims.left * widthScale2), scaledArea.top, scaledArea.left, scaledArea.bottom };
	if ( style == plotStyle::TtlPlot )	{ txt = "Ttl State"; }
	else if ( style == plotStyle::DacPlot )	{ txt = "Dac Volt"; }
	else if ( style == plotStyle::ErrorPlot ) { txt = "%"; }
	else if ( style == plotStyle::HistPlot ) { txt = "Occ"; }
	else { txt = "ylabel"; }
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


void PlotCtrl::drawThresholds ( memDC* d, long threshold, Gdiplus::Pen* pen ) 
{
	// expects that the threshold has already been converted to plot coordinates
	long top = plotAreaDims.top * heightScale2, bottom = plotAreaDims.bottom * heightScale2;
	drawLine ( d, { threshold, top }, { threshold, bottom}, pen );
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


