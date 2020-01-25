// created by Mark O. Brown
#pragma once
#include "VisaFlume.h"
#include "PlotCtrl.h"

/*
 * This is a wrapper around a PlotCtrl object and a VisaFlume object that facilitates autmatically retrieving data
 * from a tektronics oscilloscope and plotting it. Currently this just allows users to see the scope without moving 
 * to where the scope is, but could also easily facilitate loggin of the scope data as well.
 */
class ScopeViewer
{
	public:
		ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn, std::string name );
		void initialize( POINT& topLeftLoc, UINT width, UINT height, CWnd* parent, std::vector<Gdiplus::Pen*> plotPens, 
						 CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, int pltPopId,
						 std::string title="Scope!");
		void refreshData( );
		void refreshPlot( CDC* d, UINT width, UINT height, CBrush* backgroundBrush, CBrush* plotAreaBrush );
		void rearrange( int width, int height, fontMap fonts );
		std::string getScopeInfo( );
		bool handlePlotPop (UINT id, CWnd* parent);
	private:
		const std::string usbAddress;
		const std::string scopeName;
		const UINT numTraces;
		bool initializationFailed=false;
		const bool safemode;
		float yoffset, ymult;
		VisaFlume visa;
		PlotCtrl* viewPlot;
		std::vector<pPlotDataVec> scopeData;
};
