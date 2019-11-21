#pragma once
#include "stdafx.h"
#include "DmCore.h"

class DmProfileCreator {
	public:
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
		void readZernikeFile(std::string file);
		void writeZernikeFile(std::string out_file);
		void makeIm();
		void checkVals(std::vector<double> val);
		void addComa(std::vector<double> &zAmps, double comaMag, double comaAngle);
		void addAstigmatism(std::vector<double>& zAmps, double astigMag, double astigAngle);
		void addTrefoil(std::vector<double>& zAmps, double trefMag, double trefAngle);
		void addSpherical(std::vector<double>& zAmps, double sphereMag);
		std::vector<double> createZernikeArray(std::vector<double> amplitudes, std::string baselineFile, bool quiet = false);
		void generateProfile();
	private:
		std::vector<double> valArray = std::vector<double>(137);
		std::vector<double> writeArray = std::vector<double>(137);
		std::vector<std::vector<double>> temp = std::vector <std::vector<double>>(0.5, std::vector<double>(0.5,13));
		DmCore Mirror;
};