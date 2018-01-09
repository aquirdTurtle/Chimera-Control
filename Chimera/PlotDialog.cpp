#include "stdafx.h"
#include "PlotDialog.h"
#include "limits.h"
#include <algorithm>
#include "memdc.h"

PlotDialog::PlotDialog( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn) : plot(dataHolder, styleIn )
{
	backgroundBrush.CreateSolidBrush( RGB( 0, 30, 38 ) );
}


IMPLEMENT_DYNAMIC( PlotDialog, CDialog )

BEGIN_MESSAGE_MAP( PlotDialog, CDialog )
	ON_WM_CTLCOLOR( )
	ON_WM_PAINT( )
	ON_WM_SIZE( )
	ON_WM_TIMER()
END_MESSAGE_MAP()


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


HBRUSH PlotDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	return backgroundBrush;
}


BOOL PlotDialog::OnInitDialog( )
{
	// should fill this dialog.
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
		plot.drawBackground( dc, &backgroundBrush );
		plot.drawBorder( dc );
		plot.plotPoints( &dc );
		CDialog::OnPaint( );
	}
	ReleaseDC( cdc );
}
