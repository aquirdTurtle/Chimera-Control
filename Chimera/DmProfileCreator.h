#pragma once
#include "stdafx.h"

class DmProfileCreator {
	private:
	double getKnoll(int n, int m);
	void fromNoll(int j, int& n, int& m);
	void toPolar(double& r, double& phi, double x, double y);
	void toCartesian(double r, double theta, double& x, double& y);
	double zernikeTheta(int m, int theta);
	unsigned int factorial(unsigned int n);
	double zernikeRadial(int n, int m, double r);
	double normalization(int n, int m);
	double zernikePolar(int n, int m, double r, double theta);
	double zernike(int n, int m, double x, double y);
	double zernikeFit(POINT X, std::vector<double> param);
	POINT myDmLookup(int pixNum, double factor = 1 / 3.0);
	public:
};