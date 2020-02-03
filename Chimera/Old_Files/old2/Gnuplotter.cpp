#include "stdafx.h"
#include "gnuplotter.h"

// very simple at the moment.
Gnuplotter::Gnuplotter(std::string exeLoc) : plotter(exeLoc) {}

void Gnuplotter::send(std::string cmd)
{
	plotter << (cmd + "\n").c_str();
}

// could prob make the following template function overloads instead of having double, but this works fine for now.
void Gnuplotter::sendData(std::vector<double> vals)
{
	plotter.send1d(vals);
}


void Gnuplotter::sendData( std::deque<double> vals )
{
	plotter.send1d( vals );
}



void Gnuplotter::sendData(std::vector<long> vals)
{
	plotter.send1d(vals);
}


void Gnuplotter::sendData( std::vector<int> xvals, std::vector<ULONG> yvals )
{
	plotter.send1d( boost::make_tuple( xvals, yvals ) );
}


void Gnuplotter::sendData(std::vector<double> xvals, std::vector<double> yvals)
{
	plotter.send1d(boost::make_tuple(xvals, yvals));
}

void Gnuplotter::sendData(std::vector<double> xvals, std::vector<long> yvals)
{
	plotter.send1d(boost::make_tuple(xvals, yvals));
}


void Gnuplotter::sendData(std::vector<double> xvals, std::vector<double> yvals, std::vector<double> errs)
{
	plotter.send1d(boost::make_tuple(xvals, yvals, errs));
}

