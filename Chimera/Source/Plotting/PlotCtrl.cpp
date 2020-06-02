// created by Mark O. Brown
#include "stdafx.h"
#include "PlotCtrl.h"
#include "PlotDialog.h"
#include <numeric>
#include <qgraphicslayout.h>


PlotCtrl::PlotCtrl( std::vector<pPlotDataVec> dataHolder, plotStyle inStyle, std::vector<int> thresholds_in,
					std::string titleIn, bool narrowOpt, bool plotHistOption ) :

	data( dataHolder ), style( inStyle ), dataMutexes( dataHolder.size( ) ), narrow(narrowOpt )
{
}


PlotCtrl::~PlotCtrl ( )
{
}


dataPoint PlotCtrl::getMainAnalysisResult ( )
{
	// get the average data. If not only a single data point, as this is currently ment to be used, then I'm not 
	// positive what value this is grabbing... maybe the last point of the average?
	return data.back ( )->back();
}


std::vector<pPlotDataVec> PlotCtrl::getCurrentData ( )
{
	return data;
}


void PlotCtrl::setData (std::vector<pPlotDataVec> newData)
{
	for (auto& dvec : data)
	{
		dvec.reset ();
	}
	data.clear ();
	data = newData;
}

void PlotCtrl::setTitle (std::string newTitle)
{
	title = newTitle;
}

void PlotCtrl::setThresholds (std::vector<int> newThresholds)
{
	thresholds = newThresholds;
}

void PlotCtrl::setStyle (plotStyle newStyle)
{
	style = newStyle;
}

void PlotCtrl::init( POINT& pos, LONG width, LONG height, IChimeraWindowWidget* parent )
{ 
	chart = new QtCharts::QChart ();
	chart->legend ()->hide ();
	for (auto datanum : range (data.size()))
	{
		qtData.push_back (new QtCharts::QLineSeries (chart));
		chart->addSeries (qtData.back ());
		auto pen = qtData.back ()->pen ();
		pen.setWidth (1);
		qtData.back ()->setPen (pen);
	}
	chart->createDefaultAxes ();
	chart->setTitle (title.c_str ());
	chart->layout ()->setContentsMargins (0, 0, 0, 0);
	chart->setBackgroundRoundness (0);
	chart->setBackgroundBrush (QBrush (QColor (20, 20, 20)));
	chart->setMargins (QMargins (0, 0, 0, 0));
	if (chart->axisX ())
	{
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
 
void PlotCtrl::rearrange( int width, int height, fontMap fonts )
{
}


void PlotCtrl::refreshData ()
{
	chart->removeAllSeries ();
	qtData.clear ();
	for (auto dataInc : range(data.size()))
	{
		qtData.push_back (new QtCharts::QLineSeries ());
		std::unique_lock<std::mutex> lock (dataMutexes[dataInc]);
		for (auto val : *data[dataInc].get ())
		{
			*qtData[dataInc] << QPoint(val.x, val.y);
		}
		chart->addSeries (qtData.back ());
	}
}


