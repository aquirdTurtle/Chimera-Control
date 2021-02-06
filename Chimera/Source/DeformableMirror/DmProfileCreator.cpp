//Created by Max Kolanz, Mark Brown, and Chris Kheil
#include "stdafx.h"
#include "DmProfileCreator.h"
#include "GeneralUtilityFunctions/Thrower.h"
#include <math.h>
#include <fstream>
#include <boost/lexical_cast.hpp>

#define M_PI 3.14159265358979323846

template<typename T>
std::vector<double> linspace(T start_in, T end_in, int num_in){
	std::vector<double> linspaced;

	double start = static_cast<double>(start_in);
	double end = static_cast<double>(end_in);
	double num = static_cast<double>(num_in);

	if (num == 0) { return linspaced; }
	if (num == 1){
		linspaced.push_back(start);
		return linspaced;
	}

	double delta = (end - start) / (num - 1);

	for (int i = 0; i < num - 1; ++i){
		linspaced.push_back(start + delta * i);
	}
	linspaced.push_back(end); // I want to ensure that start and end
							  // are exactly the same as the input
	return linspaced;
}

DmProfileCreator::DmProfileCreator() /*: Mirror("0", true)*/ {
	currentAmps = linspace(0, 0, 45);
}

double DmProfileCreator::getKnoll(int n, int m) {
	return ((n * (n + 2) + m) / 2);
}

void DmProfileCreator::fromNoll(int j, int& n, int& m) {
	    n = 0;
		m = 0;
		while (j > n) {
			n += 1;
			j -= n;
		}
		m = -n + (2 * j);
		/*for(int j = 0; j <200; j++){
			int check1 = 0;
			int check2 = 0;
			fromNoll(j, check1, check2);
			if (j != getKnoll(check1, check2)) {
				thrower("Nolls are not inverses");
			}
		
		}*/
}

void DmProfileCreator::toPolar(double& r, double& phi, double x, double y) {
	r = sqrt((x*x) + (y*y));
	phi = atan2(y, x);
}

void DmProfileCreator::toCartesian(double r, double theta, double& x, double& y) {
	x = r * cos(theta);
	y = r * sin(theta);

}

double DmProfileCreator::zernikeTheta(int m, double theta) {
	if (m >= 0) {
		return cos(m * theta);
	}
	else {
		return sin(-m * theta);
	}
}

double DmProfileCreator::factorial(unsigned int n){
	double ret = 1;
	for (double i = 1; i <= n; ++i)
		ret *= i;
	return ret;
}

double DmProfileCreator::zernikeRadial(int n, int m, double r) {
	m = abs(m);
	if (m > n) {
		thrower("Error: n must be greater than or equal to m!");		
	}
	if ((int(n - m) % 2) > 0) {
		thrower("Error: (n - m) must be even!");
	}
	double end = ((n - m) / 2);
	int num = ((n - m) / 2 + 1);
	std::vector<double> kArray = linspace(0.0, end, num);
	double rArray = 0;
	for (auto& k : kArray) {
		double factk = factorial(k);
		double factNext = factorial((n + m) / 2 - k);
		double temp = factk * factNext;
		temp *= factorial((n - m) / 2 - k);
		rArray = rArray + (pow(r, n - 2 * k)) * pow(-1, k) * factorial(n - k) / temp;
	}
	return rArray;
}

double DmProfileCreator::normalization(int n, int m) {
	double a;
	if (m == 0) {
		a = 2 * M_PI;
	}
	else {
		a = 1 * M_PI;
	}
	double N = n;
	return sqrt(((2 * N) + 2) / a);
}

double DmProfileCreator::zernikePolar(int n, int m, double r, double theta) {
	double Product;
	Product = zernikeTheta(m, theta);
	Product *= zernikeRadial(n, m, r);
	Product *= normalization(n, m);
	return Product;
}

double DmProfileCreator::zernike(int n, int m, double x, double y) {
	double r;
	double phi;
	toPolar(r, phi, x, y);
	return zernikePolar(n, m, r, phi);
}

double DmProfileCreator::zernikeFit(QPoint X, std::vector<double> param) {
	double x = X.x();
	double y = X.y();
	int n = 0;
	int m = 0;
	fromNoll(0, n, m);
	double height = param[0] * zernike(n, m, x, y);
	for (int k = 1; k < 41; k++) {
		fromNoll(k, n, m);
		height += param[k] * zernike(n, m, x, y);
	}
	return height;
}

void DmProfileCreator::myDmLookup(double &x_coordinate, double &y_coordinate, int pixNum, double factor) {
	if (pixNum < 0 || pixNum > 136) {
		thrower("Invalid Pixel Number!");
	}
	int rowOff[] = { 4, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 4 };
	std::vector<int> numInRow;
	std::vector<double> centerOffsets;
	double yInit = 6.0;
	for (auto& rowOffset : rowOff) {
		numInRow.push_back((7 - (rowOffset + 1)) * 2 + 1);
	}
	for (int i = 0; i < 13; i++) {
		centerOffsets.push_back(-(numInRow[i] - 1) / 2);
	}

	//int pn = pixNum;
	double rowNum = 0;
	double pn = pixNum;
	for (auto& num : numInRow) {
		pn -= num;
		if (pn < 0) {
			pn += num;
			x_coordinate = (factor * (centerOffsets[rowNum] + pn));
			y_coordinate = (factor * (yInit - rowNum));
			return;
		}
		rowNum++;
	}
	thrower("ERROR: Somehow made it to the end of the loop depsite passing the initial value range check...");
}

void DmProfileCreator::readZernikeFile(std::string file) {
	std::ifstream read_file(file.c_str());
	if (!(read_file.is_open())){
		thrower("could not read the baseline file when creating profile.");
	}
	double voltage;
	std::string value;
	int counter = 0;
	while (counter < 137){
		std::getline(read_file, value);
		try {
			voltage = boost::lexical_cast<double>(value);
		}
		catch (boost::bad_lexical_cast) {
			voltage = 0.0;
		}
		valArray[counter] = voltage;
		counter++;
	}
}

void DmProfileCreator::writeZernikeFile(std::string out_file) {
	std::ofstream outWrite(DM_PROFILES_LOCATION + "//" + out_file);
	for (auto& element : writeArray) {
		outWrite << element << std::endl;
	}
}

void DmProfileCreator::makeIm() {
	std::vector<double> alv = valArray;
	int rowOff[] = { 4, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 4 };
	int tc = 0;
	int numInRow;
	int rowO;
	for (int i = 0; i < 13; i++) {
		rowO = rowOff[i];
		numInRow = (7 - (rowO + 1)) * 2 + 1;
		for (int j = 0; j < numInRow; j++) {
			temp[i][rowO + j] = alv[tc];
			tc++;
		}
	}
}

std::vector<double> DmProfileCreator::checkVals(std::vector<double> val) {
	int valNum = 0;
	for (auto& voltage : val) {
		if (voltage >= 1) {
			val[valNum] = 1;
			//thrower("ChimeraError: voltage on piston " + str(i) + " is out of range");
		}
		else if(voltage < 0){
			val[valNum] = 0;
		}
		valNum++;
	}
	return val;
}

void DmProfileCreator::addComa(double comaMag, double comaAngle) {
	//Adds at a particular angle and magnitude to a list of zernike amplituders.
	double x;
	double y;
	toCartesian(comaMag, comaAngle, x, y);
	currentAmps[7] = x;
	currentAmps[8] = y;
}

void DmProfileCreator::addAstigmatism( double astigMag, double astigAngle) {
	double x;
	double y;
	toCartesian(astigMag, astigAngle, x, y);
	currentAmps[3] = x;
	currentAmps[5] = y;
}

void DmProfileCreator::addTrefoil( double trefMag, double trefAngle) {
	double x;
	double y;
	toCartesian(trefMag, trefAngle, x, y);
	currentAmps[6] = x;
	currentAmps[9] = y;
}

void DmProfileCreator::addSpherical(double sphereMag) {
	currentAmps[12] = sphereMag;
}

std::vector<double> DmProfileCreator::createZernikeArray(std::vector<double> amplitudes, std::string baselineFile, bool quiet) {

		double inc = 1 / 3.0;
		std::vector<double> flatVoltage = valArray;
		std::vector<double> vals;
		
		for (int i = 0; i < 137; i++) {
			double zernSum = 0;
			int weightFactor = 1;
			double factor = 1/3.0;
			double x,y;
			double amp;
			myDmLookup(x, y, i, factor);

			if ((pow(x, 2) + pow(y, 2)) <= 1) {
				int j = 0;

				for(int c = 0; c < 45; c++) {
					int n, m;
					fromNoll(j, n, m);
					zernSum += amplitudes[c] * zernike(n, m, x, y);
					j++;
				}
			}
			
			else if ((pow(x, 2) + pow(y, 2)) < 1.5) {
				int count = 0;
				for (int d = 0; d < 45; d++) {
					int n, m;
					fromNoll(count, n, m);
					double r, t;
					toPolar(r, t, x, y);
					//use circle's edge (i.e. r=1) value at the angle of the pixel
					double x_, y_;
					toCartesian(1, t, x_, y_);
					double Zern;
					
					Zern = zernike(n, m, x_, y_);
					zernSum += (amplitudes[d] * Zern);
					count++;
				};
			}
			double volt = zernSum <= 0 ? -1 : 1;
			volt *= pow(zernSum, 2) * weightFactor;
			volt += flatVoltage[i];
			vals.push_back(volt);
		}
		if (!quiet) {
			vals = checkVals(vals);
		}
		return vals;
}

void DmProfileCreator::generateProfile() {
	std::string location = DM_FLAT_PROFILE;
	readZernikeFile(location);
	//double mag = 0.2;
	//std::vector<double> amp = std::vector<double>(0.0, 45);
	std::vector<double> vals_base = createZernikeArray(currentAmps, location, false);
	addComa(0.2, M_PI / 6);
	//addAstigmatism(currentAmps, 0.1, 3 * M_PI / 2);
	//addTrefoil(currentAmps, 0.1, M_PI);
	std::vector<double> vals_new;
	vals_new = createZernikeArray(currentAmps, location, false);
	writeArray = vals_new; 
	writeZernikeFile("checktheprofilemaker.txt");
}

std::vector<double> DmProfileCreator::getCurrAmps() {
	return currentAmps;
}
