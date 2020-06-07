// created by Mark O. Brown
#include "stdafx.h"
#include "PlotDialog.h"
#include "limits.h"
#include <algorithm>
#include "GeneralImaging/memdc.h"
#include "GeneralObjects/SmartDC.h"

PlotDialog::PlotDialog( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<Gdiplus::Pen*> inPens,
						CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::atomic<UINT>& timerTime, 
						std::vector<int> thresholds, int pltIds, std::string title ) :
	plot( 1, styleIn, thresholds, title ), dynamicTimerLength( timerTime ), 
	dynamicTimer(true), staticTimer(0), plotPopId (pltIds)
{
	backgroundBrush.CreateSolidBrush ( _myRGBs[ "Main-Bkgd" ] );
	plotAreaBrush.CreateSolidBrush( _myRGBs[ "Interactable-Bkgd" ] );
}


PlotDialog::PlotDialog ( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<Gdiplus::Pen*> inPens,
						 CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, UINT timerTime,
						 std::vector<int> thresholds, int pltIds, std::string title ) :
	plot ( 1, styleIn, thresholds, title ),
	dynamicTimerLength ( std::atomic<UINT> ( 0 ) ), dynamicTimer ( false ), staticTimer ( timerTime ), plotPopId (pltIds)
{
	backgroundBrush.CreateSolidBrush ( _myRGBs[ "Main-Bkgd" ] );
	plotAreaBrush.CreateSolidBrush ( _myRGBs[ "Interactable-Bkgd" ] );
}

PlotDialog::~PlotDialog ( )
{
	errBox ( "Deleting plotdlg!" );
}


IMPLEMENT_DYNAMIC( PlotDialog, CDialog )

BEGIN_MESSAGE_MAP( PlotDialog, CDialog )
	ON_COMMAND_RANGE(ID_PLOT_POP_IDS_BEGIN, ID_PLOT_POP_IDS_END, PlotDialog::handlePopCommand)
	ON_WM_CTLCOLOR( )
	ON_WM_PAINT( )
	ON_WM_SIZE( )
	ON_WM_TIMER()
END_MESSAGE_MAP()

void PlotDialog::handlePopCommand (UINT id)
{

}

dataPoint PlotDialog::getMainAnalysisResult ( )
{
	return plot.getMainAnalysisResult ( );
}


bool PlotDialog::removeQuery( PlotDialog* plt )
{
	if ( plt->m_hWnd == NULL )
	{
		return true;
	}
	if ( !plt->wantsSustain( ) )
	{
		plt->OnCancel( );
		return true;
	}
	return false;
}


bool PlotDialog::wantsSustain( )
{
	return false;// plot.wantsSustain ();
}


void PlotDialog::OnTimer( UINT_PTR id )
{
	// in principle could just invalidate rect here.
	OnPaint( );
	if ( dynamicTimer )
	{
		// restart the timer in case the timer length has changed.
		KillTimer ( id );
		SetTimer ( 1, dynamicTimerLength, NULL );
	}
}


void PlotDialog::OnSize( UINT s, int cx, int cy)
{
	CDialog::OnSize( s, cx, cy );
	OnPaint( );
}


void PlotDialog::OnCancel( )
{
	DestroyWindow( );
}

void PlotDialog::PostNcDestroy( )
{
	CDialog::PostNcDestroy( );
}



HBRUSH PlotDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	return backgroundBrush;
}


BOOL PlotDialog::OnInitDialog( )
{
	POINT pt = { 0,0 };
	//plot.init( pt, 1920, 997, this, plotPopId);
	if ( dynamicTimer )
	{
		SetTimer ( 1, dynamicTimerLength, NULL );
	}
	else
	{
		SetTimer ( 1, staticTimer, NULL );
	}
	return TRUE;
}


void PlotDialog::OnPaint( )
{
	{
		CRect size;
		GetClientRect( &size );
		SmartDC sdc (this);
		memDC dc (sdc.get ());
		CDialog::OnPaint( );
	}
}
