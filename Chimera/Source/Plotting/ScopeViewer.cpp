// created by Mark O. Brown
#include "stdafx.h"
#include "ScopeViewer.h"
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <QLineSeries>
#include <QGraphicsLayout>

ScopeViewer::ScopeViewer ( std::string usbAddress, bool safemode, unsigned traceNumIn, std::string name ) :
	visa ( safemode, usbAddress ), numTraces ( traceNumIn ), safemode ( safemode ), scopeName ( name ), viewPlot(traceNumIn,
		plotStyle::OscilloscopePlot, std::vector<int>(), name)
{
	try{
		if (safemode) { ymult = 1.0; }
		data_pdv.resize (numTraces);
		visa.open( );
		visa.query( "WFMpre:YOFF?\n", yoffset );
		visa.query( "WFMpre:YMULT?\n", ymult );
		//visa.close( );
	}
	catch ( ChimeraError& err ){
		auto answer = QMessageBox::question (NULL, "Scope Init Failed!", qstr("Failed to initialize " + scopeName 
			+ ". " + err.trace() + ". Try again?"), QMessageBox::Yes | QMessageBox::No);
		while (answer == QMessageBox::Yes) {
			try {
				visa.open ();
				visa.query ("WFMpre:YOFF?\n", yoffset);
				visa.query ("WFMpre:YMULT?\n", ymult);
				break;
			}
			catch (ChimeraError & err) {
				answer = QMessageBox::question (NULL, "Scope Init Failed!", qstr ("Failed to initialize " + scopeName
					+ ". " + err.trace () + ". Try again?"), QMessageBox::Yes | QMessageBox::No);
			}
		}		
		initializationFailed = true;
	}
}

ScopeViewer::~ScopeViewer () {
	visa.close ();
}

std::string ScopeViewer::getScopeInfo( ){
	return visa.identityQuery( );
}
 
void ScopeViewer::initialize( POINT& pos, unsigned width, unsigned height, IChimeraQtWindow* parent, std::string title){
	if ( safemode )	{
		title += " (SAFEMODE)";
	}
	if ( initializationFailed )	{
		title += " (Initialization Failed)";
	}
	viewPlot.init (pos, width, height, parent);
}

void ScopeViewer::updateData(const QVector<double>& xdata, double xmin, double xmax, 
							 const QVector<double>& ydata, double ymin, double ymax, 
							 int traceNum){
	//auto* line = data_t[traceNum];
	
	auto& line = data_pdv[traceNum];
	line.resize (xdata.size ());
	for (auto count : range (xdata.size())) {
		line[count].x = xdata[count];
		line[count].y = ydata[count];
	}
	viewPlot.setData (data_pdv);
}

QVector<double> ScopeViewer::getCurrentTraces(unsigned whichNum){
	QVector<double> newData;
	if (initializationFailed) {
		return newData;
	}
	std::string dataStr;
	try	{
		visa.write( "DATa:SOUrce CH" + str( whichNum + 1 ) );
	}
	catch ( ChimeraError&)	{
		return newData;
	}
	try	{
		visa.query( "Curve?\n", dataStr);
	}
	catch ( ChimeraError& ) {
		return newData;
	}
	double count = 0;
	unsigned dataReadSize = dataStr.size( );
	std::string temp( str( dataReadSize ) );
	for ( auto& c : dataStr.substr(0, dataStr.size() - 1) ) {
		// skip first 6 characters
		if ( count++ < 6 ) {
			continue;
		}
		newData.push_back( (((double)c) - yoffset) * ymult);
	}
	//visa.close( );
	return newData;
}

