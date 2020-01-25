#pragma once
#include "gnuplot-iostream.h"
#include <vector>
#include <string>
#include "windows.h"

class Gnuplotter
{
	public:
		Gnuplotter( std::string exeLoc );
		void send( std::string cmd );
		void sendData( std::vector<double> vals );
		void sendData( std::deque<double> vals );
		void sendData( std::vector<long> vals );
		void sendData( std::vector<double> xvals, std::vector<double> yvals );
		void sendData( std::vector<int> xvals, std::vector<ULONG> yvals );
		void sendData( std::vector<double> xvals, std::vector < long > yvals );
		void sendData( std::vector<double> xvals, std::vector<double> yvals, std::vector<double> errs );
	private:
		Gnuplot plotter;
};
