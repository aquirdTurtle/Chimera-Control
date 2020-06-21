// created by Mark O. Brown
#include "stdafx.h"
#include "ScopeViewer.h"
#include <PrimaryWindows/IChimeraWindowWidget.h>
#include <QLineSeries>
#include <QGraphicsLayout>

ScopeViewer::ScopeViewer ( std::string usbAddress, bool safemode, UINT traceNumIn, std::string name ) :
	visa ( safemode, usbAddress ), numTraces ( traceNumIn ), safemode ( safemode ), scopeName ( name )
{
	try
	{
		visa.open( );
		visa.query( "WFMpre:YOFF?\n", yoffset );
		visa.query( "WFMpre:YMULT?\n", ymult );
		if (safemode) { ymult = 1.0; }
		visa.close( );
	}
	catch ( Error& err )
	{
		errBox( "Error detected while initializing " + scopeName + "scope viewer! " + err.trace( ) );
		initializationFailed = true;
	}
}


std::string ScopeViewer::getScopeInfo( )
{
	return visa.identityQuery( );
}

 
void ScopeViewer::initialize( POINT& pos, UINT width, UINT height, IChimeraWindowWidget* parent, std::string title)
{
	chart = new QtCharts::QChart ();
	chart->legend ()->hide ();
	for (auto datanum : range (numTraces)) {
		data_t.push_back (new QtCharts::QLineSeries (chart));
		chart->addSeries (data_t.back());
		auto pen = data_t.back ()->pen ();
		pen.setWidth (1);
		data_t.back ()->setPen (pen);
	}
	if ( safemode )	{
		title += " (SAFEMODE)";
	}
	if ( initializationFailed )	{
		title += " (Initialization Failed)";
	}
	chart->createDefaultAxes ();
	chart->setTitle (title.c_str());
	chart->layout ()->setContentsMargins (0, 0, 0, 0);
	chart->setBackgroundRoundness (0);
	chart->setBackgroundBrush (QBrush(QColor(20,20,20)));
	chart->setMargins (QMargins (0,0,0,0));
	auto pen = chart->axes (Qt::Horizontal)[0]->gridLinePen ();
	pen.setColor (QColor (255, 255, 255, 50));
	chart->axes (Qt::Horizontal)[0]->setGridLinePen (pen);
	chart->axes (Qt::Vertical)[0]->setGridLinePen (pen);
	viewPlot = new QtCharts::QChartView (chart, parent);
	viewPlot->setRenderHint (QPainter::Antialiasing);
	viewPlot->setGeometry (pos.x, pos.y, width, height);
	pos.y += height;
}

void ScopeViewer::updateData(const QVector<double>& xdata, double xmin, double xmax, 
							 const QVector<double>& ydata, double ymin, double ymax, 
							 int traceNum){
	if (safemode) {
		auto* line = data_t[traceNum];
		viewPlot->chart ()->removeSeries (line);
		line->clear ();
		for (auto count : range (xdata.size())) {
			*line << QPointF (xdata[count], ((ydata[count] - yoffset) * ymult));
		}
		viewPlot->chart ()->addSeries (line);
		viewPlot->chart ()->axes (Qt::Horizontal)[0]->setMin (xmin);
		viewPlot->chart ()->axes (Qt::Horizontal)[0]->setMax (xmax);
		viewPlot->chart ()->axes (Qt::Vertical)[0]->setMin (ymin);
		viewPlot->chart ()->axes (Qt::Vertical)[0]->setMax (ymax);
	}
}


void ScopeViewer::refreshData( )
{
	if ( safemode )	{
		Sleep( 50000 );
		double xminv=DBL_MAX, xmaxv=-DBL_MAX, yminv = DBL_MAX, ymaxv = -DBL_MAX;
		for (auto* line : data_t){
			line->clear ();
			double count = 0;
			for (auto count : range (250)){
				auto val = double(rand () % 1000) / 1e3;
				auto qval = QPointF (double (count - 7), ((double (val) - yoffset) * ymult));
				if (qval.x() > xmaxv){
					xmaxv = qval.x();
				}
				else if (qval.y() < xminv){
					xminv = qval.x();
				}
				if (qval.y() > ymaxv){
					ymaxv = qval.y();
				}
				else if (qval.y() < yminv){
					yminv = qval.y();
				}
				*line << qval;
			}
		}
		viewPlot->chart ()->axes (Qt::Horizontal)[0]->setMin (xminv);
		viewPlot->chart ()->axes (Qt::Horizontal)[0]->setMax (xmaxv);
		viewPlot->chart ()->axes (Qt::Vertical)[0]->setMin (yminv);
		viewPlot->chart ()->axes (Qt::Vertical)[0]->setMax (ymaxv);
		return;
	}
	visa.open( );
	for ( auto line : range( numTraces ) )
	{
		std::string data;
		try	{
			visa.write( "DATa:SOUrce CH" + str( line + 1 ) );
		}
		catch ( Error&)	{
			//errBox( err.what( ) );
		}
		try	{
			visa.query( "Curve?\n", data );
		}
		catch ( Error& ) {
			continue;
		}
		double count = 0;
		//std::lock_guard<std::mutex> lock( viewPlot->dataMutexes[line] );
		//scopeData[line]->clear( );
		UINT dataReadSize = data.size( );
		std::string temp( str( dataReadSize ) );
		for ( auto& c : data.substr(0, data.size() - 1) )
		{
			// skip first 6 characters
			if ( count++ < 6 )
			{
				continue;
			}
			//*data_t << QPoint (count - 7, ((((double)c) - yoffset) * ymult));
			//*scopeData[line].get () << QPoint (count - 7, ((((double)c) - yoffset) * ymult));
			//scopeData[line]->push_back( { count-7, ((((double)c) - yoffset) * ymult), 0 } );
		}
	}
	visa.close( );
}

