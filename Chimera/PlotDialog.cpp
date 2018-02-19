#include "stdafx.h"
#include "PlotDialog.h"
#include "limits.h"
#include <algorithm>
#include "memdc.h"

PlotDialog::PlotDialog( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<Gdiplus::Pen*> inPens,
						 CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::string title ) :
	plot(dataHolder, styleIn, inPens, font, plotBrushes, title )
{
	backgroundBrush.CreateSolidBrush( RGB( 0, 30, 38 ) );
	plotAreaBrush.CreateSolidBrush( RGB( 0, 0, 0 ) );
}


IMPLEMENT_DYNAMIC( PlotDialog, CDialog )

BEGIN_MESSAGE_MAP( PlotDialog, CDialog )
	ON_WM_CTLCOLOR( )
	ON_WM_PAINT( )
	ON_WM_SIZE( )
	ON_WM_TIMER()
END_MESSAGE_MAP()


bool PlotDialog::removeQuery( PlotDialog* plt )
{
	if ( !plt->wantsSustain( ) )
	{
		plt->OnCancel( );
		return true;
	}
	return false;
}


bool PlotDialog::wantsSustain( )
{
	return plot.wantsSustain();
}


void PlotDialog::OnTimer( UINT_PTR id )
{
	// in principle could just invalidate rect here.
	OnPaint( );
}


void PlotDialog::OnSize( UINT s, int cx, int cy)
{
	CDialog::OnSize( s, cx, cy );
	OnPaint( );
}


void PlotDialog::OnCancel( )
{
	plot.clear( );
	DestroyWindow( );
}

void PlotDialog::PostNcDestroy( )
{
	CDialog::PostNcDestroy( );
	// seems to cause problems...
	//delete this;
}



HBRUSH PlotDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	return backgroundBrush;
}


BOOL PlotDialog::OnInitDialog( )
{
	plot.init( { 0, 0 }, 1920, 997, this);
	SetTimer( 1, 10000, NULL );
	return TRUE;
}


void PlotDialog::OnPaint( )
{
	CDC* cdc = GetDC( );
	{
		CRect size;
		GetClientRect( &size );
		memDC dc( cdc );
		plot.setCurrentDims( size.right - size.left, size.bottom - size.top );
		plot.drawBackground( dc, &backgroundBrush, &plotAreaBrush );
		plot.drawTitle( dc );
		plot.drawBorder( dc );
		plot.plotPoints( &dc );
		CDialog::OnPaint( );
	}
	ReleaseDC( cdc );
}
