// This is a small console application that can be used as a test-bed for implementing new libraries, 
// testing small code tricks, etc.

#include "stdafx.h"
#include <iostream>
#include "gnuplot-iostream.h"
#include "boost\tuple\tuple.hpp"

int main()
{
	Gnuplot plotter( "\"C:\\Program Files (x86)\\gnuplot\\bin\\gnuplot.exe\"" );
	plotter << "set terminal wxt 0 title \"Loading\" noraise background rgb 'black'\n";
	plotter << "set format y \"%.1f\"\n";
	plotter << "set xrange[0:1]\n";
	plotter << "set grid ytics lc rgb \"#bbbbbb\" lw 1 lt 0\n";
	plotter << "set grid xtics lc rgb \"#bbbbbb\" lw 1 lt 0\n";
	plotter << "set yrange[0:1]\n";
	plotter << "set title \"Loading\" tc rgb 'white'\n";
	plotter << "set xlabel \"Key Value\" tc rgb 'white'\n";
	plotter << "set ylabel \"Loading %\" tc rgb 'white'\n";
	plotter << "set border lc rgb 'white'\n";
	plotter << "set key tc rgb 'white'\n";
	plotter << "plot '-' using 1:2:3 with yerrorbars title \"G1 pixel\" lt rgb \"white\" pt 5 pointsize 0.5,\n";
	plotter.send1d( boost::make_tuple(std::vector<int>{0}, std::vector<int>{1} , std::vector<int>{0} ) );


	std::cout << "Hi!";
	std::cin.get();
	return 0;
}

