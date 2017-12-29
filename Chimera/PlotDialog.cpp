#include "stdafx.h"
#include "PlotDialog.h"
#include "limits.h"
#include <algorithm>
#include "memdc.h"

PlotDialog::PlotDialog( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn) : plot(dataHolder, styleIn )
{
	backgroundBrush.CreateSolidBrush( RGB( 0, 30, 38 ) );
}


PlotDialog::~PlotDialog( )
{

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
	SetTimer( 1, 1000, NULL );
	plot.init( { 0, 0 }, 1920, 997, this);
	return TRUE;
}


void PlotDialog::OnPaint( )
{
	CRect size;
	GetClientRect( &size );
	memDC dc( GetDC() );
	plot.drawBackground( dc, size.right - size.left, size.bottom - size.top, &backgroundBrush );
	plot.drawBorder( dc, size.right - size.left, size.bottom - size.top );
	plot.plotPoints( &dc, size.right - size.left, size.bottom - size.top );
	CDialog::OnPaint( );
}
