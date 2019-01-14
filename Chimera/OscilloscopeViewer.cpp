#include "stdafx.h"
#include "OscilloscopeViewer.h"
#include "Thrower.h"


ScopeViewer::ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn ) :
	visa( safemode, usbAddress ),
	numTraces( traceNumIn ), safemode(safemode)
{
	try
	{
		visa.open( );
		//visa.write( "header off\n" );
		visa.query( "WFMpre:YOFF?\n", yoffset );
		visa.query( "WFMpre:YMULT?\n", ymult );
		visa.close( );
	}
	catch ( Error& err )
	{
		errBox( "Error detected while initializing scope viewer! " + err.trace( ) );
	}
}


std::string ScopeViewer::getScopeInfo( )
{
	return visa.identityQuery( );
}


void ScopeViewer::refreshPlot(CDC* d, UINT width, UINT height, CBrush* backgroundBrush, CBrush* plotAreaBrush )
{
	viewPlot->setCurrentDims( width, height );
	viewPlot->drawPlot ( d, backgroundBrush, plotAreaBrush );
}


void ScopeViewer::rearrange( int width, int height, fontMap fonts )
{
	if ( viewPlot )
	{
		viewPlot->rearrange ( width, height, fonts );
	}
}


void ScopeViewer::initialize( POINT& topLeftLoc, UINT width, UINT height, CWnd* parent, std::vector<Gdiplus::Pen*> plotPens,
							  CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::string title )
{
	scopeData.resize( numTraces );
	for ( auto& data : scopeData )
	{
		data = pPlotDataVec( new plotDataVec( 100, { 0,0,0 } ) );
	}
	viewPlot = new PlotCtrl( scopeData, plotStyle::OscilloscopePlot, plotPens, font, plotBrushes, title );
	viewPlot->init( topLeftLoc, width, height, parent );
	topLeftLoc.y += height;
	//refreshData( );
}


void ScopeViewer::refreshData( )
{
	if ( safemode )
	{
		Sleep( 5000 );
		return;
	}
	visa.open( );
	for ( auto line : range( numTraces ) )
	{
		std::string data;
		try
		{
			visa.write( "DATa:SOUrce CH" + str( line + 1 ) );
		}
		catch ( Error&)
		{
			//errBox( err.what( ) );
		}
		try
		{
			visa.query( "Curve?\n", data );
		}
		catch ( Error& )
		{
			continue;
		}
		     
		double count = 0;
		std::lock_guard<std::mutex> lock( viewPlot->dataMutexes[line] );
		scopeData[line]->clear( );
		UINT dataReadSize = data.size( );
		std::string temp( str( dataReadSize ) );
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

