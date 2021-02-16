// created by Mark O. Brown
#pragma once
#include "GeneralFlumes/VisaFlume.h"
#include "Plotting/QCustomPlotCtrl.h"
#include <qobject.h>
#include <QChart>
#include <QChartView>
#include "QChartView.h"
#include <QtCharts/QChartView>
#include <PrimaryWindows/IChimeraQtWindow.h>

/*
 * This is a wrapper around a PlotCtrl object and a VisaFlume object that facilitates autmatically retrieving data
 * from a tektronics oscilloscope and plotting it. Currently this just allows users to see the scope without moving 
 * to where the scope is, but could also easily facilitate loggin of the scope data as well.
 */
class ScopeViewer : public QObject{ 
	Q_OBJECT

	public:
		ScopeViewer( std::string usbAddress, bool safemode, unsigned traceNumIn, std::string name );
		~ScopeViewer ();
		void initialize( QPoint& topLeftLoc, unsigned width, unsigned height, IChimeraQtWindow* parent, 
						 std::string title="Scope!");
		
		QVector<double> getCurrentTraces (unsigned whichLine);
		std::string getScopeInfo( );
		const unsigned numTraces;
		const bool safemode;
	private:
		const std::string usbAddress;
		const std::string scopeName;
		bool initializationFailed=false;
		std::string name;
		float yoffset, ymult;
		VisaFlume visa;
		//PlotCtrl viewPlot;
		QCustomPlotCtrl viewPlot;
		std::vector<plotDataVec> data_pdv;
		std::vector<QtCharts::QLineSeries*> data_t;
		QPushButton* updateDataBtn;
	public Q_SLOTS:
		void updateData (const QVector<double>& xdata, double xmin, double xmax, 
						 const QVector<double>& ydata, double ymin, double ymax, 
						 int traceNum);
};
