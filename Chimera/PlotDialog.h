#pragma once
#include "windows.h"
#include <vector>
#include "PlotCtrl.h"

class PlotDialog : public CDialog
{
	/*
	 * PlotDialog is a dialog with a single PlotCtrl plot in it. It's only purpose is to act as a 
	 */
	public:
		DECLARE_DYNAMIC( PlotDialog );
		PlotDialog( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<CPen*> inPens, 
					CFont* font, std::string title="Plot Dialog Plot" );
		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog( ) override;
		void OnPaint( );
		void OnTimer( UINT_PTR id );
		void OnSize( UINT s, int cx, int cy );
	private:
		DECLARE_MESSAGE_MAP( );
		PlotCtrl plot;
		CBrush backgroundBrush;
};

