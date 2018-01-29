#include "stdafx.h"
#include "OscilloscopeViewer.h"


ScopeViewer::ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn ) :
	visa( safemode, usbAddress ),
	numTraces( traceNumIn )
{
	try
	{
		visa.open( );
		visa.write( "header off\n" );
		visa.query( "WFMpre:YOFF?\n", yoffset );
		visa.query( "WFMpre:YMULT?\n", ymult );
		visa.close( );
	}
	catch ( Error& err )
	{
		errBox( "Error detected while initializing scope viewer! " + err.whatStr( ) );
	}
}


std::string ScopeViewer::getScopeInfo( )
{
	return visa.identityQuery( );
}


void ScopeViewer::refreshPlot(CDC* d, UINT width, UINT height, CBrush* backgroundBrush, CBrush* plotAreaBrush )
{
	viewPlot->setCurrentDims( width, height );
	memDC dacDC( d, &viewPlot->GetPlotRect( ) );
	viewPlot->drawBackground( dacDC, backgroundBrush, plotAreaBrush );
	viewPlot->drawTitle( dacDC );
	viewPlot->drawBorder( dacDC );
	viewPlot->plotPoints( &dacDC );
}


void ScopeViewer::rearrange( int width, int height, fontMap fonts )
{
	viewPlot->rearrange( width, height, fonts );
}


void ScopeViewer::initialize( POINT& topLeftLoc, UINT width, UINT height, CWnd* parent, std::vector<Gdiplus::Pen*> plotPens,
							  CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::string title )
{
	scopeData.resize( numTraces );
	for ( auto& data : scopeData )
	{
		data = pPlotDataVec( new plotDataVec( 100, { 0,0,0 } ) );
	}
	viewPlot = new PlotCtrl( scopeData, OscilloscopePlot, plotPens, font, plotBrushes, title );
	viewPlot->init( topLeftLoc, width, height, parent );
	topLeftLoc.y += height;
	//refreshData( );
}


void ScopeViewer::refreshData( )
{
	visa.open( );
	for ( auto line : range( numTraces ) )
	{
		std::string data;
		try
		{
			visa.write( "DATa:SOUrce CH" + str( line + 1 ) );
		}
		catch ( Error& err)
		{
			//errBox( err.what( ) );
		}
		try
		{
			visa.query( "Curve?\n", data );
		}
		catch ( Error& err )
		{
		}

		double count = 0;
		std::lock_guard<std::mutex> lock( viewPlot->dataMutexes[line] );
		scopeData[line]->clear( );
		for ( auto& c : data.substr(0, data.size() - 1) )
		{
			if ( count++ < 6 )
			{
				continue;
			}
			scopeData[line]->push_back( { count-7, ((((double)c) - yoffset) * ymult), 0 } );
		}
	}
	visa.close( );
}

