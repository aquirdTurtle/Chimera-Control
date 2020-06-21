// created by Mark O. Brown
#pragma once
#include "GeneralFlumes/VisaFlume.h"
#include "Plotting/PlotCtrl.h"
#include <qobject.h>
#include <QChart>
#include <QChartView>
#include "QChartView.h"
#include <QtCharts/QChartView>
#include <PrimaryWindows/IChimeraWindowWidget.h>

/*
 * This is a wrapper around a PlotCtrl object and a VisaFlume object that facilitates autmatically retrieving data
 * from a tektronics oscilloscope and plotting it. Currently this just allows users to see the scope without moving 
 * to where the scope is, but could also easily facilitate loggin of the scope data as well.
 */
class ScopeViewer : public QObject{
	Q_OBJECT

	public:
		ScopeViewer( std::string usbAddress, bool safemode, UINT traceNumIn, std::string name );
		void initialize( POINT& topLeftLoc, UINT width, UINT height, IChimeraWindowWidget* parent, 
						 std::string title="Scope!");
		
		void refreshData( );
		std::string getScopeInfo( );
		const UINT numTraces;
	private:
		const std::string usbAddress;
		const std::string scopeName;
		bool initializationFailed=false;
		const bool safemode;
		float yoffset, ymult;
		VisaFlume visa;
		QtCharts::QChartView* viewPlot;
		std::vector<QtCharts::QLineSeries*> data_t;
		QtCharts::QChart* chart;		
	public Q_SLOTS:
		void updateData (const QVector<double>& xdata, double xmin, double xmax, 
						 const QVector<double>& ydata, double ymin, double ymax, 
						 int traceNum);
};
