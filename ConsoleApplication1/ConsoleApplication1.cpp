// ConsoleApplication1.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "TRandom1.h"


int main()
{
	TRandom1* myrand = new TRandom1();
	/*
	// The values and the errors on the Y axis
	const int n_points = 10;
	double x_vals[n_points] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	double y_vals[n_points] = {6, 12, 14, 20, 22, 24, 35, 45, 44, 53};
	double y_errs[n_points] = {5, 5, 4.7, 4.5, 4.2, 5.1, 2.9, 4.1, 4.8, 5.43};
	
	// Instance of the graph
	TGraphErrors graph(n_points, x_vals, y_vals, nullptr, y_errs);
	graph.SetTitle("Measurement XYZ;lenght [cm];Arb.Units");
	
	// Make the plot estetically better
	graph.SetMarkerStyle(kOpenCircle);
	graph.SetMarkerColor(kBlue);
	graph.SetLineColor(kBlue);
	
	// The canvas on which we'll draw the graph
	auto mycanvas = new TCanvas();
	// Draw the graph !
	graph.DrawClone("APE");
	// Define a linear function
	TF1 f("Linear law", "[0]+x*[1]", .5, 10.5);
	// Let's make the funcion line nicer
	f.SetLineColor(kRed); f.SetLineStyle(2);
	// Fit it to the graph and draw it
	graph.Fit(&f);
	f.DrawClone("Same");
	// Build and Draw a legend
	TLegend leg(.1, .7, .3, .9, "Lab. Lesson 1");
	leg.SetFillColor(0);
	*/
	return 0;
}
