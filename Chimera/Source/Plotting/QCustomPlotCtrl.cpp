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
	if (plot->graphCount() == 0) {
		plot->addGraph();
	}
	return plot->graph(0);
}

void QCustomPlotCtrl::removeData() {
	//plot->clearGraphs();
	plot->clearPlottables();
}

void QCustomPlotCtrl::initializeCalData(calSettings cal) {
	if (plot->graphCount() == 0) {
		plot->addGraph();
	}
	plot->graph(0)->setPen(QColor(Qt::white));
	plot->graph(0)->setLineStyle(QCPGraph::lsNone);
	plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
	plot->graph(0)->setData({}, {});
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
		plot->graph(0)->setName("Data");
		plot->graph(0)->setLineStyle(QCPGraph::lsNone);
		plot->graph(0)->setPen(QColor(Qt::white));
		plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
		plot->graph(0)->setData(calXdata, calYdata);
		// second line
		auto& newLineData2 = newData[1];
		QVector<double> fitXdata, fitYdata;
		for (auto count : range(newLineData2.size())) {
			fitXdata.append(newLineData2[count].x);
			fitYdata.append(newLineData2[count].y);
		}
		plot->addGraph();
		plot->graph(1)->setPen(QColor(Qt::red));
		plot->graph(1)->setData(fitXdata, fitYdata);
		plot->graph(1)->setName("Fit");
		// third line
		auto& newLineData3 = newData[2];
		QVector<double> histCalXdata, histCalYdata;
		for (auto count : range(newLineData3.size())) {
			histCalXdata.append(newLineData3[count].x);
			histCalYdata.append(newLineData3[count].y);
		}
		plot->addGraph();
		plot->graph(2)->setName("Historical Fit");
		plot->graph(2)->setPen(QColor("orange"));
		plot->graph(2)->setData(histCalXdata, histCalYdata);
	}
	else if (style == plotStyle::BinomialDataPlot) {
		if (newData.size() == 0 || !plot) {
			return;
		}
		for (auto traceNum : range(newData.size())) {
			auto& newLine = newData[traceNum];
			QVector<double> newXdata, newYdata, yerrData;
			for (auto count : range(newLine.size())) {
				newXdata.append(newLine[count].x);
				newYdata.append(newLine[count].y);
				yerrData.append(newLine[count].err);
			}
			plot->addGraph();
			QCPErrorBars* errorBars = new QCPErrorBars(plot->xAxis, plot->yAxis);
			errorBars->removeFromLegend();
			errorBars->setAntialiased(false);
			errorBars->setDataPlottable(plot->graph());
			if (traceNum == newData.size() - 1) {
				plot->graph()->setName("Avg.");
				plot->graph()->setPen(QColor(Qt::white));
				errorBars->setPen(QColor(Qt::white));
				plot->graph()->setScatterStyle(QCPScatterStyle::ssDisc);
			}
			else {
				plot->graph()->setName("DSet " + qstr(traceNum + 1));
				auto gcolor = GIST_RAINBOW_RGB[traceNum * GIST_RAINBOW_RGB.size() / newData.size()];
				auto qtColor = QColor(gcolor[0], gcolor[1], gcolor[2], 100);
				plot->graph()->setPen(qtColor);
				errorBars->setPen(qtColor);
				plot->graph()->setScatterStyle(QCPScatterStyle::ssCircle);
			}			
			plot->graph()->setLineStyle(QCPGraph::lsNone);
			plot->graph()->setData(newXdata, newYdata);
			errorBars->setData(yerrData);
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
	else if (style == plotStyle::PicturePlot) {
		if (newData.size() == 0 || !plot) {
			return;
		}
		plot->plotLayout()->setAutoMargins(QCP::MarginSide::msNone);
		plot->xAxis->setTickLabels(false);
		plot->yAxis->setTickLabels(false);
		plot->plotLayout()->setMargins({ 0,0,0,0 });
		plot->xAxis->grid()->setPen(Qt::NoPen);
		plot->yAxis->grid()->setPen(Qt::NoPen);
		unsigned lineCount = 0;
		for (auto traceNum : range(newData.size())) {
			QCPCurve* newCurve = new QCPCurve(plot->xAxis, plot->yAxis);
			auto& newLine = newData[traceNum];
			auto color = GIST_RAINBOW_RGB[lineCount * GIST_RAINBOW_RGB.size() / newData.size()];
			QVector<double> newXdata, newYdata;
			for (auto count : range(newLine.size())) {
				newXdata.append(newLine[count].x);
				newYdata.append(newLine[count].y);
			}
			newCurve->setData(newXdata, newYdata);
			newCurve->setPen(QColor(color[0], color[1], color[2]));
			lineCount++;
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
	resetChart();
	plot->replot();
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
	
	if (style == plotStyle::BinomialDataPlot) {
		plot->yAxis->setRange({ 0,1 });
		plot->xAxis->rescale();
	}
	else if (style == plotStyle::HistPlot) {
		plot->yAxis->rescale();
		plot->yAxis->setRangeLower(0);
		plot->xAxis->rescale();
	}
	else {
		plot->rescaleAxes();
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
		[this, parent](const QPoint& pos2) {
			try {
				handleContextMenu(pos2);
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

void QCustomPlotCtrl::setControlLocation(QRect loc) {
	plot->setGeometry(loc);
}