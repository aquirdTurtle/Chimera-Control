// created by Mark O. Brown
#include "stdafx.h"
#include "QCustomPlotCtrl.h"
#include <numeric>
#include <qgraphicslayout.h>
#include <qdebug.h>
#include <qmenu.h>
#include <AnalogInput/CalibrationManager.h>

QCustomPlotCtrl::QCustomPlotCtrl(unsigned numTraces, plotStyle inStyle, std::vector<int> thresholds_in,
	bool narrowOpt, bool plotHistOption) :
	style(inStyle), narrow(narrowOpt) {
}

QCustomPlotCtrl::~QCustomPlotCtrl() {
}

void QCustomPlotCtrl::handleContextMenu(const QPoint& pos) {
	QMenu menu;
	menu.setStyleSheet(chimeraStyleSheets::stdStyleSheet());
	auto* clear = new QAction("Clear Plot", plot);
	plot->connect(clear, &QAction::triggered,
		[this]() {
			plot->clearGraphs();
		});
	auto* leg = new QAction("Toggle Legend", plot);
	plot->connect(leg, &QAction::triggered,
		[this]() {
			showLegend = !showLegend;
			resetChart();
		});


	menu.addAction(clear);
	menu.addAction(leg);
	menu.exec(plot->mapToGlobal(pos));
}

dataPoint QCustomPlotCtrl::getMainAnalysisResult() {
	// get the average data. If not only a single data point, as this is currently ment to be used, then I'm not 
	// positive what value this is grabbing... maybe the last point of the average?
	return dataPoint();
}

QCPGraph* QCustomPlotCtrl::getCalData() {
	return plot->graph();
}

void QCustomPlotCtrl::removeData() {
	plot->clearGraphs();
}

void QCustomPlotCtrl::initializeCalData(calSettings cal) {
	plot->clearGraphs();
	//view->chart()->removeAllSeries();
	//calibrationData = new QtCharts::QScatterSeries(view->chart());
	//calibrationData->setBorderColor(Qt::black);
	//calibrationData->setMarkerSize(8.0);
	//calibrationData->setColor(QColor(255, 255, 255));
	//calibrationData->setName("Calibration Data");
	//view->chart()->addSeries(calibrationData);
	//double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX;
	//// set the range based on the previous calibration
	//for (auto xpt : cal.result.ctrlVals) {
	//	xmin = xpt < xmin ? xpt : xmin;
	//	xmax = xpt > xmax ? xpt : xmax;
	//}
	//for (auto ypt : cal.result.resVals) {
	//	ymin = ypt < ymin ? ypt : ymin;
	//	ymax = ypt > ymax ? ypt : ymax;
	//}
	//resetChart();
	//if (!view->chart()->axes(Qt::Horizontal)[0]) {
	//	return;
	//}
	//if (xmax == xmin) {
	//	view->chart()->axes(Qt::Horizontal)[0]->setRange(xmin - 1, xmin + 1);
	//}
	//else {
	//	view->chart()->axes(Qt::Horizontal)[0]->setRange(xmin - (xmax - xmin) / 20, xmax + (xmax - xmin) / 20);
	//}
	//if (ymax == ymin) {
	//	view->chart()->axes(Qt::Vertical)[0]->setRange(ymin - 1, ymin + 1);
	//}
	//else {
	//	view->chart()->axes(Qt::Vertical)[0]->setRange(ymin - (ymax - ymin) / 20, ymax + (ymax - ymin) / 20);
	//}
}

void QCustomPlotCtrl::setData(std::vector<plotDataVec> newData) {
	removeData();
	if (style == plotStyle::CalibrationPlot) {
		// need to rename...
		if (newData.size() != 3 || !plot) {
			// should always have the calibration data, historical fit, and recent fit.
			return;
		}
		// first data set is scatter
		auto& newLineData = newData[0];
		initializeCalData(calSettings());
		QVector<double> calXdata, calYdata;
		for (auto count : range(newLineData.size())) {
			calXdata.append(newLineData[count].x);
			calYdata.append(newLineData[count].y);
		}
		plot->addGraph();
		plot->graph()->setName("Data");
		plot->graph()->setPen(QColor(Qt::white));
		plot->graph()->setScatterStyle(QCPScatterStyle::ssCircle);
		plot->graph()->setData(calXdata, calYdata);
		// second line
		auto& newLineData2 = newData[1];
		QVector<double> fitXdata, fitYdata;
		for (auto count : range(newLineData2.size())) {
			fitXdata.append(newLineData2[count].x);
			fitYdata.append(newLineData2[count].y);
		}
		plot->addGraph();
		plot->graph()->setPen(QColor(Qt::red));
		plot->graph()->addData(fitXdata, fitYdata);
		plot->graph()->setName("Fit");
		// third line
		auto& newLineData3 = newData[2];
		QVector<double> histCalXdata, histCalYdata;
		for (auto count : range(newLineData3.size())) {
			histCalXdata.append(newLineData3[count].x);
			histCalYdata.append(newLineData3[count].y);
		}
		plot->addGraph();
		plot->graph()->setName("Historical Fit");
		plot->graph()->setPen(QColor("orange"));
		plot->graph()->addData(fitXdata, fitYdata);
	}
	else if (style == plotStyle::BinomialDataPlot) {
		if (newData.size() == 0 || !plot) {
			return;
		}
		unsigned lineCount = 0;
		for (auto traceNum : range(newData.size())) {
			auto& newLine = newData[traceNum];
			QVector<double> newXdata, newYdata;

			for (auto count : range(newLine.size())) {
				newXdata.append(newLine[count].x);
				newYdata.append(newLine[count].y);
			}

			plot->addGraph();
			plot->graph()->setName("DSet " + qstr(traceNum + 1));
			if (lineCount == newData.size() - 1) {
				plot->graph()->setPen(QColor(Qt::white));
			}
			else {
				auto gcolor = GIST_RAINBOW_RGB[lineCount * GIST_RAINBOW_RGB.size() / newData.size()];
				plot->graph()->setPen(QColor(gcolor[0], gcolor[1], gcolor[2], 100));
			}
			plot->graph()->setName("Historical Fit");
			plot->graph()->addData(newXdata, newYdata);
		}
	}
	else if (style == plotStyle::HistPlot) {
		if (newData.size() == 0 || !plot) {
			return;
		}
		unsigned lineCount = 0;
		for (auto traceNum : range(newData.size())) {
			auto& newLine = newData[traceNum];
			if (newLine.size() > 1000) {
				continue; // something very wrong...
			}
			auto color = GIST_RAINBOW_RGB[(lineCount++) * GIST_RAINBOW_RGB.size() / newData.size()];
			
			QVector<double> newXdata, newYdata;
			for (auto count : range(newLine.size())) {
				newXdata.append(newLine[count].x);
				newYdata.append(newLine[count].y);
			}
			plot->addGraph();
			plot->graph()->setPen(QColor(color[0], color[1], color[2], 150));
			plot->graph()->setName("DSet " + qstr(traceNum + 1));
			plot->graph()->addData(newXdata, newYdata);
		}
		lineCount = 0;
		for (auto thresholdNum : range(thresholds.size())) {
			QCPItemStraightLine* infLine = new QCPItemStraightLine(plot);
			infLine->point1->setCoords(thresholds[thresholdNum], 0);  
			infLine->point2->setCoords(thresholds[thresholdNum], 1);  
			auto color = GIST_RAINBOW_RGB[(lineCount++) * GIST_RAINBOW_RGB.size() / newData.size()];
			infLine->setPen(QColor(color[0], color[1], color[2], 150));
		}
	}
	else { // line plot... e.g. for dio and ao data
		if (newData.size() == 0 || !plot) {
			return;
		}
		unsigned lineCount = 0;
		for (auto traceNum : range(newData.size())) {
			auto& newLine = newData[traceNum];
			auto color = GIST_RAINBOW_RGB[lineCount * GIST_RAINBOW_RGB.size() / newData.size()];
			QVector<double> newXdata, newYdata;
			for (auto count : range(newLine.size())) {
				newXdata.append(newLine[count].x);
				newYdata.append(newLine[count].y);
			}
			lineCount++;
			plot->addGraph();
			plot->graph()->setPen(QColor(color[0],color[1],color[2]));
			plot->graph()->addData(newXdata, newYdata);
		}
	}
	plot->rescaleAxes();
	plot->replot();
	resetChart();
}

void QCustomPlotCtrl::setTitle(std::string newTitle) {
	title->setText(qstr(newTitle));
}

void QCustomPlotCtrl::setThresholds(std::vector<int> newThresholds) {
	thresholds = newThresholds;
}

void QCustomPlotCtrl::setStyle(plotStyle newStyle) {
	style = newStyle;
}

void QCustomPlotCtrl::resetChart() {
	if (!showLegend) {
		plot->legend->setVisible(false);
	}
	else {
		plot->legend->setVisible(true);
	}

	auto defs = chimeraStyleSheets::getDefs();
	auto neutralColor = QColor(defs["@NeutralTextColor"]);
	auto generalPen = QPen(neutralColor);

	plot->xAxis->setBasePen(generalPen);
	plot->xAxis->setLabelColor(neutralColor);
	plot->yAxis->setBasePen(generalPen);
	plot->yAxis->setLabelColor(neutralColor);

	plot->xAxis->setTickPen(generalPen);
	plot->yAxis->setTickPen(generalPen);
	plot->xAxis->setSubTickPen(generalPen);
	plot->yAxis->setSubTickPen(generalPen);
	plot->xAxis->setTickLabelColor(neutralColor);
	plot->yAxis->setTickLabelColor(neutralColor);
	plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);
	auto legendColor = QColor(defs["@StaticBackground"]);
	legendColor.setAlpha(150);
	plot->legend->setBrush(legendColor);
	plot->legend->setTextColor(neutralColor);
	plot->setBackground(QColor(defs["@StaticBackground"]));
	title->setTextColor(defs["@NeutralTextColor"]);
}

void QCustomPlotCtrl::init(QPoint& pos, long width, long height, IChimeraQtWindow* parent, QString titleIn) {
	
	auto& px = pos.rx(), & py = pos.ry();
	plot = new QCustomPlot(parent);
	plot->setContextMenuPolicy(Qt::CustomContextMenu);
	parent->connect(plot, &QtCharts::QChartView::customContextMenuRequested,
		[this, parent](const QPoint& pos) {
			try {
				handleContextMenu(pos);
			}
			catch (ChimeraError& err) {
				parent->reportErr(err.qtrace());
			}
		});
	title = new QCPTextElement(plot, titleIn);
	plot->plotLayout()->insertRow(0);
	plot->plotLayout()->addElement(0, 0, title);
	resetChart();
	plot->setGeometry(px, py, width, height);
	py += height;
}
