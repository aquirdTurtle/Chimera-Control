// created by Mark O. Brown
#include "stdafx.h"
#include "OscilloscopeViewer.h"

ScopeViewer::ScopeViewer ( std::string usbAddress, bool safemode, UINT traceNumIn, std::string name ) :
	visa ( safemode, usbAddress ), numTraces ( traceNumIn ), safemode ( safemode ), scopeName ( name )
{
	try
	{
		visa.open( );
		visa.query( "WFMpre:YOFF?\n", yoffset );
		visa.query( "WFMpre:YMULT?\n", ymult );
		if (safemode) { ymult = 1.0; }
		visa.close( );
	}
	catch ( Error& err )
	{
		errBox( "Error detected while initializing " + scopeName + "scope viewer! " + err.trace( ) );
		initializationFailed = true;
	}
}


bool ScopeViewer::handlePlotPop (UINT id, CWnd* parent)
{
	return viewPlot->handlePop (id, parent);
}


std::string ScopeViewer::getScopeInfo( )
{
	return visa.identityQuery( );
}


void ScopeViewer::refreshPlot(CDC* cdc, UINT width, UINT height, CBrush* backgroundBrush, CBrush* plotAreaBrush )
{
	viewPlot->setCurrentDims( width, height );
	viewPlot->drawPlot ( cdc, backgroundBrush, plotAreaBrush );
}


void ScopeViewer::rearrange( int width, int height, fontMap fonts )
{
	if ( viewPlot )	{ viewPlot->rearrange ( width, height, fonts );	}
}


void ScopeViewer::initialize( POINT& topLeftLoc, UINT width, UINT height, CWnd* parent, std::vector<Gdiplus::Pen*> plotPens,
							  CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, int pltPopId, std::string title)
{
	scopeData.resize( numTraces );
	for ( auto& data : scopeData )
	{
		data = pPlotDataVec( new plotDataVec( 100, { 0,0,0 } ) );
	}
	if ( safemode )
	{
		title += " (SAFEMODE)";
	}
	if ( initializationFailed )
	{
		title += " (Initialization Failed)";
	}
	viewPlot = new PlotCtrl( scopeData, plotStyle::OscilloscopePlot, plotPens, font, plotBrushes, std::vector<int>(), title );
	viewPlot->init( topLeftLoc, width, height, parent, pltPopId);
	//topLeftLoc.y += height;
}


void ScopeViewer::refreshData( )
{
	if ( safemode )
	{
		Sleep( 5000 );
		for (auto line : range (numTraces))
		{
			scopeData[line]->clear ();
			double count = 0;
			for (auto count : range (1000))
			{
				auto val = double(rand () % 1000) / 1e3;
				scopeData[line]->push_back ({ double(count - 7), ((double(val) - yoffset) * ymult), 0 });
			}
		}
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
			// skip first 6 characters
			if ( count++ < 6 )
			{
				continue;
			}
			scopeData[line]->push_back( { count-7, ((((double)c) - yoffset) * ymult), 0 } );
		}
	}
	visa.close( );
}

