// created by Mark O. Brown
#include "stdafx.h"
#include "PlotCtrl.h"
#include <numeric>
#include <qgraphicslayout.h>
#include <qdebug.h>
#include <qmenu.h>
#include <AnalogInput/CalibrationManager.h>

PlotCtrl::PlotCtrl( unsigned numTraces, plotStyle inStyle, std::vector<int> thresholds_in,
					std::string titleIn, bool narrowOpt, bool plotHistOption ) :
	qtLineData( numTraces ), style( inStyle ), narrow(narrowOpt ), title(titleIn){
}

PlotCtrl::~PlotCtrl ( ){
}

void PlotCtrl::handleContextMenu (const QPoint& pos) {
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* clear = new QAction ("Clear Plot", view);
	view->connect (clear, &QAction::triggered,
		[this]() {
			view->chart ()->removeAllSeries ();
		});
	menu.addAction (clear);
	menu.exec (view->mapToGlobal (pos));
}

dataPoint PlotCtrl::getMainAnalysisResult ( ){
	// get the average data. If not only a single data point, as this is currently ment to be used, then I'm not 
	// positive what value this is grabbing... maybe the last point of the average?
	return dataPoint();
}

QtCharts::QScatterSeries* PlotCtrl::getCalData () {
	return calibrationData;
}

void PlotCtrl::removeData () {
	view->chart ()->removeAllSeries ();
}

void PlotCtrl::initializeCalData (calSettings cal) {
	view->chart ()->removeAllSeries ();
	calibrationData = new QtCharts::QScatterSeries (view->chart ());
	calibrationData->setBorderColor (Qt::black);
	calibrationData->setMarkerSize (8.0);
	calibrationData->setColor (QColor (255, 255, 255));
	calibrationData->setName ("Calibration Data");
	view->chart ()->addSeries (calibrationData);
	double xmin=DBL_MAX, xmax=-DBL_MAX, ymin=DBL_MAX, ymax=-DBL_MAX;
	// set the range based on the previous calibration
	for (auto xpt : CalibrationManager::calPtTextToVals (cal.ctrlPtString)) {
		xmin = xpt < xmin ? xpt : xmin;
		xmax = xpt > xmax ? xpt : xmax;
	}
	for (auto ypt : cal.resultValues) {
		ymin = ypt < ymin ? ypt : ymin;
		ymax = ypt > ymax ? ypt : ymax;
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
	if (ymax == ymin) {
		view->chart ()->axes (Qt::Vertical)[0]->setRange (ymin - 1, ymin + 1);
	}
	else {
		view->chart ()->axes (Qt::Vertical)[0]->setRange (ymin - (ymax - ymin) / 20, ymax + (ymax - ymin) / 20);
	}
}



void PlotCtrl::setData (std::vector<plotDataVec> newData){
	removeData ();
	double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX;
	if (style == plotStyle::GeneralErrorPlot) {
		// need to rename...
		if (newData.size () != 2 || !view->chart ()) {
			// should always have the calibration data and fit.
			return;
		}
		// first data set is scatter
		auto& newLineData = newData[0];
		initializeCalData (calSettings());
		for (auto count : range (newLineData.size ())) {
			xmin = newLineData[count].x < xmin ? newLineData[count].x : xmin;
			xmax = newLineData[count].x > xmax ? newLineData[count].x : xmax;
			ymin = newLineData[count].y < ymin ? newLineData[count].y : ymin;
			ymax = newLineData[count].y > ymax ? newLineData[count].y : ymax;
			*calibrationData << QPointF (newLineData[count].x, newLineData[count].y);
		}
		view->chart ()->addSeries (calibrationData);
		// second line
		auto& newLineData2 = newData[1];
		fitData = new QtCharts::QLineSeries (view->chart ());
		auto color = GIST_RAINBOW_RGB[1 * GIST_RAINBOW_RGB.size () / 2];
		QPen pen = fitData->pen ();
		pen.setWidth (2);
		pen.setColor ("red");
		fitData->setPen (pen);
		for (auto count : range (newLineData2.size ())) {
			xmin = newLineData2[count].x < xmin ? newLineData2[count].x : xmin;
			xmax = newLineData2[count].x > xmax ? newLineData2[count].x : xmax;
			ymin = newLineData2[count].y < ymin ? newLineData2[count].y : ymin;
			ymax = newLineData2[count].y > ymax ? newLineData2[count].y : ymax;
			*fitData << QPointF (newLineData2[count].x, newLineData2[count].y);
		}
		fitData->setName ("Fit");
		view->chart ()->addSeries (fitData);
	}
	else if (style == plotStyle::BinomialDataPlot) {
		if (newData.size () == 0 || !view->chart ()) {
			return;
		}
		qtScatterData.clear ();
		qtScatterData.resize (newData.size ());
		unsigned lineCount = 0;
		for (auto traceNum : range (newData.size ())) {
			auto* line = qtScatterData[traceNum];
			auto& newLine = newData[traceNum];
			line = new QtCharts::QScatterSeries (view->chart ());
			line->setBorderColor (Qt::black);
			if (lineCount == qtScatterData.size ()-1) {
				line->setMarkerSize (8.0);
				line->setColor( QColor (255, 255, 255));
			}
			else {
				auto gcolor = GIST_RAINBOW_RGB[lineCount * GIST_RAINBOW_RGB.size () / qtScatterData.size ()];
				line->setMarkerSize (6.0);
				line->setColor (QColor (gcolor[0], gcolor[1], gcolor[2], 100));
			}
			for (auto count : range (newLine.size ())) {
				xmin = newLine[count].x < xmin ? newLine[count].x : xmin;
				xmax = newLine[count].x > xmax ? newLine[count].x : xmax;
				ymin = newLine[count].y < ymin ? newLine[count].y : ymin;
				ymax = newLine[count].y > ymax ? newLine[count].y : ymax;
				*line << QPointF (newLine[count].x, newLine[count].y);
			}
			view->chart ()->addSeries (line);
			lineCount++;
			view->chart()->legend ()->setVisible (true);
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
		for (auto traceNum : range (newData.size ())) {
			auto* line = qtLineData[traceNum];
			auto& newLine = newData[traceNum];
			if (newLine.size () > 1000) {
				continue; // something very wrong...
			}
			line = new QtCharts::QLineSeries (view->chart ());
			auto color = GIST_RAINBOW_RGB[(lineCount++) * GIST_RAINBOW_RGB.size () / qtLineData.size ()];
			line->setColor (QColor (color[0], color[1], color[2], 150));
			for (auto count : range (newLine.size ())) {
				xmin = newLine[count].x < xmin ? newLine[count].x : xmin;
				xmax = newLine[count].x > xmax ? newLine[count].x : xmax;
				ymin = newLine[count].y < ymin ? newLine[count].y : ymin;
				ymax = newLine[count].y > ymax ? newLine[count].y : ymax;
				*line << QPointF (newLine[count].x, newLine[count].y);
			}
			view->chart ()->addSeries (line);
		}
		lineCount = 0;
		for (auto thresholdNum : range (thresholds.size ())) {
			QtCharts::QLineSeries* threshLine = new QtCharts::QLineSeries (view->chart ());
			*threshLine << QPointF (thresholds[thresholdNum], 0) << QPointF (thresholds[thresholdNum], ymax);
			auto color = GIST_RAINBOW_RGB[(lineCount++) * GIST_RAINBOW_RGB.size () / qtLineData.size ()];
			threshLine->setColor (QColor (color[0], color[1], color[2], 150));
			view->chart ()->addSeries (threshLine);
		}
		view->chart ()->legend ()->setVisible (true);
	}
	else { // line plot... e.g. for dio and ao data
		if (newData.size () == 0 || !view->chart ()) {
			return;
		}
		qtLineData.clear ();
		qtLineData.resize (newData.size ());
		for (auto& line : qtLineData) {
			line = new QtCharts::QLineSeries (view->chart ());
		}
		unsigned lineCount = 0;
		for (auto traceNum : range (newData.size ())) {
			auto* line = qtLineData[traceNum];
			auto& newLine = newData[traceNum];
			line = new QtCharts::QLineSeries (view->chart ());
			auto color = GIST_RAINBOW_RGB[lineCount * GIST_RAINBOW_RGB.size () / qtLineData.size ()];
			QPen pen = line->pen ();
			pen.setWidth (1);
			pen.setBrush (QColor (color[0], color[1], color[2], 50)); // or just pen.setColor("red");
			line->setPen (pen);
			//line->setColor ();
			
			for (auto count : range (newLine.size ())) {
				xmin = newLine[count].x < xmin ? newLine[count].x : xmin;
				xmax = newLine[count].x > xmax ? newLine[count].x : xmax;
				ymin = newLine[count].y < ymin ? newLine[count].y : ymin;
				ymax = newLine[count].y > ymax ? newLine[count].y : ymax;
				*line << QPointF (newLine[count].x, newLine[count].y);
			}
			lineCount++;
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
	if (style == plotStyle::BinomialDataPlot) {
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
	if (style != plotStyle::GeneralErrorPlot) {
		view->chart ()->legend ()->hide ();
	}
	view->chart ()->createDefaultAxes ();
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
	view->chart ()->setTitleBrush (QBrush (Qt::white));
	view->chart ()->setTitle (title.c_str ());
}

void PlotCtrl::init( QPoint& pos, LONG width, LONG height, IChimeraQtWindow* parent ){ 
	auto& px = pos.rx (), &py = pos.ry ();
	chart = new QtCharts::QChart ();
	view = new QtCharts::QChartView (chart, parent);
	view->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (view, &QtCharts::QChartView::customContextMenuRequested,
		[this](const QPoint& pos) { handleContextMenu (pos); });

	for (auto datanum : range (qtLineData.size ())) {
		qtLineData[datanum] = new QtCharts::QLineSeries (view->chart ());
		view->chart()->addSeries (qtLineData[datanum]);
	}
	resetChart ();
	view->setRenderHint (QPainter::Antialiasing);
	view->setGeometry (px, py, width, height);
	py += height;
}
