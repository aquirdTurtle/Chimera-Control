#pragma once
#include "VisaFlume.h"
#include "PlotCtrl.h"

class ScopeViewer
{
	public:
		ScopeViewer( std::string usbAddress, bool safemode );
		void initialize( POINT topLeftLoc, UINT width, UINT height, CWnd* parent );
		void refreshData( );
		void refreshPlot( CDC* d, UINT width, UINT height, CBrush* backgroundBrush );
		void rearrange( int width, int height, fontMap fonts );
	private:
		const std::string usbAddress;
		float yoffset, ymult;
		VisaFlume visa;
		PlotCtrl* viewPlot;
		std::vector<pPlotDataVec> scopeData;
};