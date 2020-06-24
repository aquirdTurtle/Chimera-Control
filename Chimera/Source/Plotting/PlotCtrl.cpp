// created by Mark O. Brown
#include "stdafx.h"
#include "PlotCtrl.h"
#include "PlotDialog.h"
#include <numeric>
#include <qgraphicslayout.h>

PlotCtrl::PlotCtrl( unsigned numTraces, plotStyle inStyle, std::vector<int> thresholds_in,
					std::string titleIn, bool narrowOpt, bool plotHistOption ) :
	qtLineData( numTraces ), style( inStyle ), narrow(narrowOpt ), title(titleIn){
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
	view->chart ()->removeAllSeries ();
	double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX;
	if (style == plotStyle::ErrorPlot) {
		if (newData.size () == 0) {
			return;
		}
		if (!view->chart ()) {
			return;
		}		
		for (auto* line : qtScatterData) {
			//line->clear ();
		}
		qtScatterData.clear ();
		qtScatterData.resize (newData.size ());
		for (auto& line : qtScatterData) {
			line = new QtCharts::QScatterSeries(view->chart ());
			line->setMarkerSize (8.0);
			line->setBorderColor (Qt::black);
		}
		for (auto traceNum : range (newData.size ())) {
			auto* line = qtScatterData[traceNum];
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
	}
	else if (style == plotStyle::HistPlot) {
		if (newData.size () == 0) {
			return;
		}
		if (!view->chart ()) {
			return;
		}
		qtLineData.clear ();
		qtLineData.resize (newData.size ());
		unsigned lineCount=0;
		for (auto& line : qtLineData) {
			line = new QtCharts::QLineSeries (view->chart ());
			auto color = GIST_RAINBOW_RGB[(lineCount++)* GIST_RAINBOW_RGB.size()/qtLineData.size()];
			line->setColor (QColor (color[0], color[1], color[2]));
		}
		for (auto traceNum : range (newData.size ())) {
			auto* line = qtLineData[traceNum];
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
	}
	else {
		if (newData.size () == 0) {
			return;
		}
		if (!view->chart ()) {
			return;
		}
		qtLineData.clear ();
		qtLineData.resize (newData.size ());
		for (auto& line : qtLineData) {
			line = new QtCharts::QLineSeries (view->chart ());
		}
		for (auto traceNum : range (newData.size ())) {
			auto* line = qtLineData[traceNum];
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
	}
	resetChart ();
	if (!view->chart ()->axes (Qt::Horizontal)[0]) {
		return;
	}
	if (xmax == xmin) {
		view->chart ()->axes (Qt::Horizontal)[0]->setRange (xmin - 1, xmin + 1);
	}
	else {
		view->chart ()->axes (Qt::Horizontal)[0]->setRange (xmin - (xmax - xmin) / 20, xmax + (xmax - xmin) / 20);
	}
	if (style == plotStyle::ErrorPlot) {
		view->chart ()->axes (Qt::Vertical)[0]->setRange (0, 1);
	}
	else {
		if (ymax == ymin) {
			view->chart ()->axes (Qt::Vertical)[0]->setRange (ymin - 1, ymin + 1);
		}
		else {
			view->chart ()->axes (Qt::Vertical)[0]->setRange (ymin - (ymax - ymin) / 20, ymax + (ymax - ymin) / 20);
		}
	}
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

void PlotCtrl::resetChart () {
	view->chart ()->legend ()->hide ();
	view->chart ()->legend ()->hide ();
	view->chart ()->createDefaultAxes ();
	view->chart ()->setTitle (title.c_str ());
	view->chart ()->layout ()->setContentsMargins (0, 0, 0, 0);
	view->chart ()->setBackgroundRoundness (0);
	view->chart ()->setBackgroundBrush (QBrush (QColor (20, 20, 20)));
	view->chart ()->setMargins (QMargins (0, 0, 0, 0));
	if (view->chart ()->axes (Qt::Horizontal)[0]) {
		auto pen = view->chart ()->axes (Qt::Horizontal)[0]->gridLinePen ();
		pen.setColor (QColor (255, 255, 255, 50));
		view->chart ()->axes (Qt::Horizontal)[0]->setGridLinePen (pen);
		view->chart ()->axes (Qt::Vertical)[0]->setGridLinePen (pen);
	}
}

void PlotCtrl::init( POINT& pos, LONG width, LONG height, IChimeraWindowWidget* parent ){ 
	chart = new QtCharts::QChart ();
	view = new QtCharts::QChartView (chart, parent);
	for (auto datanum : range (qtLineData.size ())) {
		qtLineData[datanum] = new QtCharts::QLineSeries (view->chart ());
		view->chart()->addSeries (qtLineData[datanum]);
	}
	resetChart ();
	view->setRenderHint (QPainter::Antialiasing);
	view->setGeometry (pos.x, pos.y, width, height);
	pos.y += height;
}
