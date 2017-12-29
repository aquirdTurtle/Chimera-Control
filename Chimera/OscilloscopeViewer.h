#pragma once
#include "VisaFlume.h"
#include "PlotCtrl.h"

class ScopeViewer
{
	public:
		ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn );
		void initialize( POINT& topLeftLoc, UINT width, UINT height, CWnd* parent );
		void refreshData( );
		void refreshPlot( CDC* d, UINT width, UINT height, CBrush* backgroundBrush );
		void rearrange( int width, int height, fontMap fonts );
	private:
		const std::string usbAddress;
		const UINT numTraces;
		float yoffset, ymult;
		VisaFlume visa;
		PlotCtrl* viewPlot;
		std::vector<pPlotDataVec> scopeData;
};