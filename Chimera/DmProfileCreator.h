#pragma once
#include "stdafx.h"
#include "DmCore.h"

class DmProfileCreator {
	public:
		DmProfileCreator::DmProfileCreator();
		double getKnoll(int n, int m);
		void fromNoll(int j, int& n, int& m);
		void toPolar(double& r, double& phi, double x, double y);
		void toCartesian(double r, double theta, double& x, double& y);
		double zernikeTheta(int m, double theta);
		double factorial(unsigned int n);
		double zernikeRadial(int n, int m, double r);
		double normalization(int n, int m);
		double zernikePolar(int n, int m, double r, double theta);
		double zernike(int n, int m, double x, double y);
		double zernikeFit(POINT X, std::vector<double> param);
		void myDmLookup(double& x_coordinate, double& y_coordinate, int pixNum, double factor);
		void readZernikeFile(std::string file);
		void writeZernikeFile(std::string out_file);
		void makeIm();
		void checkVals(std::vector<double> val);
		void addComa(double comaMag, double comaAngle);
		void addAstigmatism(std::vector<double>& zAmps, double astigMag, double astigAngle);
		void addTrefoil(std::vector<double>& zAmps, double trefMag, double trefAngle);
		void addSpherical(std::vector<double>& zAmps, double sphereMag);
		std::vector<double> createZernikeArray(std::vector<double> amplitudes, std::string baselineFile, bool quiet = false);
		void generateProfile();
		std::vector<double> getCurrAmps();
	private:
		std::vector<double> valArray = std::vector<double>(137);
		std::vector<double> writeArray = std::vector<double>(137);
		std::vector<double> currentAmps;
		std::vector<std::vector<double>> temp;
		DmCore Mirror;
};