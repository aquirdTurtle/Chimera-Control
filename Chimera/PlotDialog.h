#pragma once
#include "windows.h"
#include <vector>
#include "PlotCtrl.h"


class PlotDialog : public CDialog
{
	/*
	 * PlotDialog is a wrapper around a CDialog and a PlotCtrl. That is, it's a dialog with a PlotCtrl plot in it. 
	 * It's purpose is to act as a movable window for extra plots that don't belong in the main window.
	 */
	public:
		DECLARE_DYNAMIC( PlotDialog );
		PlotDialog( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<Gdiplus::Pen*> inPens,
					CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, UINT timerTime,
					std::string title="Plot Dialog Plot" );
		static bool removeQuery( PlotDialog* plt );
		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog( ) override;
		void OnCancel( ) override;
		void PostNcDestroy( ) override;
		void OnPaint( );
		void OnTimer( UINT_PTR id );
		void OnSize( UINT s, int cx, int cy );
		bool wantsSustain( );
	private:
		DECLARE_MESSAGE_MAP( );
		PlotCtrl plot;
		CBrush backgroundBrush, plotAreaBrush;
		// in ms
		const UINT timerLength;
};

