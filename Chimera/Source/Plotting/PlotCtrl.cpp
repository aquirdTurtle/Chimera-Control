// created by Mark O. Brown
#include "stdafx.h"
#include "PlotCtrl.h"
#include "PlotDialog.h"
#include <numeric>
#include <qgraphicslayout.h>


PlotCtrl::PlotCtrl( unsigned numTraces, plotStyle inStyle, std::vector<int> thresholds_in,
					std::string titleIn, bool narrowOpt, bool plotHistOption ) :
	qtData( numTraces ), style( inStyle ), narrow(narrowOpt ), title(titleIn){
}


PlotCtrl::~PlotCtrl ( ){
}


dataPoint PlotCtrl::getMainAnalysisResult ( ){
	// get the average data. If not only a single data point, as this is currently ment to be used, then I'm not 
	// positive what value this is grabbing... maybe the last point of the average?
	return dataPoint();
}


std::vector<pPlotDataVec> PlotCtrl::getCurrentData ( ){
	return std::vector<pPlotDataVec>();
}


void PlotCtrl::setData (std::vector<plotDataVec> newData){
	assert (newData.size () == qtData.size ());
	double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX;
	double ;
	for (auto traceNum : range(newData.size())){
		auto* line = qtData[traceNum];
		view->chart ()->removeSeries (line);
		line->clear ();
		auto& newLine = newData[traceNum];
		for (auto count : range (newLine.size ())) {
			xmin = newLine[count].x < xmin ? newLine[count].x : xmin;
			xmax = newLine[count].x > xmax ? newLine[count].x : xmax;
			ymin = newLine[count].y < ymin ? newLine[count].y : ymin;
			ymax = newLine[count].y > ymax ? newLine[count].y : ymax;
			*line << QPointF (newLine[count].x, newLine[count].y);
		}
		view->chart ()->addSeries (line);
	}
	view->chart ()->axisX ()->setRange (xmin - (xmax-xmin)/20, xmax + (xmax - xmin) / 20);
	view->chart ()->axisY ()->setRange (ymin - (ymax - ymin) / 20, ymax + (ymax - ymin) / 20);
	//viewPlot->chart ()->axisX ()->setMin (xmin);
	//viewPlot->chart ()->axisX ()->setMax (xmax);
	//viewPlot->chart ()->axisY ()->setMin (ymin);
	//viewPlot->chart ()->axisY ()->setMax (ymax);
}

void PlotCtrl::setTitle (std::string newTitle){
	title = newTitle;
}

void PlotCtrl::setThresholds (std::vector<int> newThresholds){
	thresholds = newThresholds;
}

void PlotCtrl::setStyle (plotStyle newStyle){
	style = newStyle;
}

void PlotCtrl::init( POINT& pos, LONG width, LONG height, IChimeraWindowWidget* parent ){ 
	chart = new QtCharts::QChart ();
	chart->legend ()->hide ();
	for (auto datanum : range (qtData.size())){
		qtData[datanum] = new QtCharts::QLineSeries (chart);
		chart->addSeries (qtData[datanum]);
	}
	chart->createDefaultAxes ();
	chart->setTitle (title.c_str ());
	chart->layout ()->setContentsMargins (0, 0, 0, 0);
	chart->setBackgroundRoundness (0);
	chart->setBackgroundBrush (QBrush (QColor (20, 20, 20)));
	chart->setMargins (QMargins (0, 0, 0, 0));
	if (chart->axisX ()){
		auto pen = chart->axisX ()->gridLinePen ();
		pen.setColor (QColor (255, 255, 255, 50));
		chart->axisX ()->setGridLinePen (pen);
		chart->axisY ()->setGridLinePen (pen);
	}
	view= new QtCharts::QChartView (chart, parent);
	view->setRenderHint (QPainter::Antialiasing);
	view->setGeometry (pos.x, pos.y, width, height);
	pos.y += height;
}

void PlotCtrl::refreshData ()
{
	/*
	chart->removeAllSeries ();
	qtData.clear ();
	for (auto dataInc : range(qtData.size()))
	{
		qtData.push_back (new QtCharts::QLineSeries ());
		std::unique_lock<std::mutex> lock (dataMutexes[dataInc]);
		for (auto val : *data[dataInc].get ())
		{
			*qtData[dataInc] << QPoint(val.x, val.y);
		}
		chart->addSeries (qtData.back ());
	}
	*/
}


