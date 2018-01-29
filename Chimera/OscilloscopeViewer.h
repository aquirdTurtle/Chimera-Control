#pragma once
#include "VisaFlume.h"
#include "PlotCtrl.h"

class ScopeViewer
{
	public:
		ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn );
		void initialize( POINT& topLeftLoc, UINT width, UINT height, CWnd* parent, std::vector<Gdiplus::Pen*> plotPens, 
						 CFont* font, std::vector<Gdiplus::SolidBrush*> plotBrushes, std::string title="Scope!" );
		void refreshData( );
		void refreshPlot( CDC* d, UINT width, UINT height, CBrush* backgroundBrush, CBrush* plotAreaBrush );
		void rearrange( int width, int height, fontMap fonts );
		std::string getScopeInfo( );
	private:
		const std::string usbAddress;
		const UINT numTraces;
		float yoffset, ymult;
		VisaFlume visa;
		PlotCtrl* viewPlot;
		std::vector<pPlotDataVec> scopeData;
};