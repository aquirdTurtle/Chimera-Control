// created by Mark O. Brown
#pragma once
#include "GeneralFlumes/VisaFlume.h"
#include "Plotting/PlotCtrl.h"
#include <QChart>
#include <QChartView>
#include "QChartView.h"
#include <QtCharts/QChartView>
#include <PrimaryWindows/IChimeraWindow.h>
/*
 * This is a wrapper around a PlotCtrl object and a VisaFlume object that facilitates autmatically retrieving data
 * from a tektronics oscilloscope and plotting it. Currently this just allows users to see the scope without moving 
 * to where the scope is, but could also easily facilitate loggin of the scope data as well.
 */
class ScopeViewer
{
	public:
		ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn, std::string name );
		void initialize( POINT& topLeftLoc, UINT width, UINT height, IChimeraWindowWidget* parent, 
						 std::string title="Scope!");
		void refreshData( );
		std::string getScopeInfo( );
	private:
		const std::string usbAddress;
		const std::string scopeName;
		const UINT numTraces;
		bool initializationFailed=false;
		const bool safemode;
		float yoffset, ymult;
		VisaFlume visa;
		//PlotCtrl* viewPlot = NULL;
		QtCharts::QChartView* viewPlot;
		std::vector<QtCharts::QLineSeries*> data_t;
		QtCharts::QChart* chart;		
};
