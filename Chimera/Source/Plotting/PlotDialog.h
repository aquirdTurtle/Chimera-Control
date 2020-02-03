﻿// created by Mark O. Brown
#pragma once
#include "windows.h"
#include "PlotCtrl.h"
#include <vector>
#include <atomic>

class PlotDialog : public CDialog
{
	/*
	 * PlotDialog is a wrapper around a CDialog and a PlotCtrl. That is, it's a dialog with a PlotCtrl plot in it. 
	 * It's purpose is to act as a movable window for extra plots that don't belong in the main window, mainly 
	 * real-time data analysis plots.
	 */
	public:
		DECLARE_DYNAMIC( PlotDialog );
		// the overload options here are for having a dynamic timer (first option) or a static timer (second option).
		PlotDialog (std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<Gdiplus::Pen*> inPens,
					CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::atomic<UINT>& timerTime,
					std::vector<int> thresholds, int pltIds,
					std::string title="Plot Dialog Plot" );
		PlotDialog ( std::vector<pPlotDataVec> dataHolder, plotStyle styleIn, std::vector<Gdiplus::Pen*> inPens,
					 CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, UINT timerTime,
					 std::vector<int> thresholds, int pltIds, std::string title);
		~PlotDialog ( );
		static bool removeQuery( PlotDialog* plt );
		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog( ) override;
		void OnCancel( ) override;
		void PostNcDestroy( ) override;
		void OnPaint( );
		void OnTimer( UINT_PTR id );
		void OnSize( UINT s, int cx, int cy );
		bool wantsSustain( );
		dataPoint getMainAnalysisResult ( );
		void handlePopCommand (UINT id);
	private:
		DECLARE_MESSAGE_MAP( );
		const int plotPopId;
		PlotCtrl plot;
		CBrush backgroundBrush, plotAreaBrush;
		// in ms
		std::atomic<UINT>& dynamicTimerLength;
		const bool dynamicTimer;
		const UINT staticTimer;
};
