#include "stdafx.h"
#include "DmProfileCreator.h"
#include "Thrower.h"
#include "math.h"
#include <fstream>
#include <boost/lexical_cast.hpp>
#define M_PI 3.14159265358979323846

double DmProfileCreator::getKnoll(int n, int m) {
	return ((n * (n + 2) + m) / 2);
}

void DmProfileCreator::fromNoll(int j, int& n, int& m) {
	    n = 0;
		m = 0;
		while (j > n) {
			n += 1
				j -= n
				m = -n + 2 * j
		}
		for(int j = 0; j <200; j++){
			int check1 = 0;
			int check2 = 0;
			fromNoll(j, check1, check2);
			if (j != getKnoll(check1, check2)) {
				thrower("Nolls are not inverses");
			}
		
		}
}

void DmProfileCreator::toPolar(double& r, double& phi, double x, double y) {
	r = sqrt((x*x) + (y*y));
	phi = atan2(y, x);
}

void DmProfileCreator::toCartesian(double r, double theta, double& x, double& y) {
	x = r * cos(theta);
	y = r * sin(theta);

}

double DmProfileCreator::zernikeTheta(int m, int theta) {
	if (m >= 0) {
		return cos(m * theta);
	}
	else {
		return sin(-m * theta);
	}
}

template<typename T>
std::vector<double> linspace(T start_in, T end_in, int num_in)
{

	std::vector<double> linspaced;

	double start = static_cast<double>(start_in);
	double end = static_cast<double>(end_in);
	double num = static_cast<double>(num_in);

	if (num == 0) { return linspaced; }
	if (num == 1)
	{
		linspaced.push_back(start);
		return linspaced;
	}

	double delta = (end - start) / (num - 1);

	for (int i = 0; i < num - 1; ++i)
	{
		linspaced.push_back(start + delta * i);
	}
	linspaced.push_back(end); // I want to ensure that start and end
							  // are exactly the same as the input
	return linspaced;
}

unsigned int DmProfileCreator::factorial(unsigned int n)
{
	unsigned int ret = 1;
	for (unsigned int i = 1; i <= n; ++i)
		ret *= i;
	return ret;
}

double DmProfileCreator::zernikeRadial(int n, int m, double r) {
	m = abs(m);
	if (m > n) {
		thrower("Error: n must be greater than or equal to n!");
	}
	if ((int(n - m) % 2) > 0) {
		thrower("Error: (n - m) must be even!");
	}
	double end = ((n - m) / 2);
	int num = ((n - m) / 2 + 1);
	std::vector<double> kArray = linspace(0.0, end, num);
	double rArray = 0;
	for (auto& k : kArray) {
		rArray = rArray + (pow(r, n - 2 * k)) * pow(-1, k) * factorial(n - k) / (factorial(k) * factorial((n + m) / 2 - k) * factorial((n - m) / 2 - k));
	}
	return rArray;
}

double DmProfileCreator::normalization(int n, int m) {
	double a;
	if (m == 0) {
		double a = 2 * M_PI;
	}
	else {
		a = 1 * M_PI;
	}
	return sqrt((2 * n + 2) / a);
}

double DmProfileCreator::zernikePolar(int n, int m, double r, double theta) {
	return zernikeTheta(m, theta) * zernikeRadial(n, m, r) * normalization(n, m);
}

double DmProfileCreator::zernike(int n, int m, double x, double y) {
	double r;
	double phi;
	toPolar(r, phi, x, y);
	return zernikePolar(n, m, r, phi);
}

double DmProfileCreator::zernikeFit(POINT X, std::vector<double> param) {
	double x = X.x;
	double y = X.y;
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

POINT DmProfileCreator::myDmLookup(int pixNum, double factor = 1 / 3.0) {
	if (pixNum < 0 || pixNum > 136) {
		thrower("Invalid Pixel Number!");
	}
	int rowOff[] = { 4, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 4 };
	std::vector<int> numInRow;
	std::vector<int> centerOffsets;
	int yInit = 6;
	for (auto& rowOffset : rowOff) {
		numInRow.push_back((7 - (rowOffset + 1)) * 2 + 1);
	}
	for (int i = 0; i < 13; i++) {
		centerOffsets.push_back(-(numInRow[i] - 1) / 2);
	}

	int pn = pixNum;
	int counter = 0;
	POINT PIXEL;
	for (auto& rowNum : numInRow) {
		pn -= counter;
		if (pn < 0) {
			pn += counter;
			PIXEL.x = factor * (centerOffsets[rowNum] + pn);
			PIXEL.y = factor * (yInit - rowNum);
			return PIXEL;
		}
		counter++;
	}
	thrower("ERROR: Somehow made it to the end of the loop depsite passing the initial value range check...");

}

void DmProfileCreator::readZernikeFile(std::string file) {

	std::ifstream read_file(file);
	double voltage;
	int counter = 0;
	while (counter < 137){
		std::getline(read_file, voltage);
		try {
			voltage = boost::lexical_cast<double>(voltage);
		}
		catch (boost::bad_lexical_cast) {
			voltage = 0.0;
		}
		valArray[counter] = voltage;
		counter++;
	}
}

void DmProfileCreator::writeZernikeFile(std::string out_file) {
	std::ofstream outWrite(out_file);
	for (auto& element : valArray) {
		outWrite << element << std::endl;
	}
}

void DmProfileCreator::makeIm() {
	std::vector<double> alv = Mirror.getActuatorValues();
	int rowOff[] = { 4, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 4 };
	int tc = 0;
	int numInRow;
	for (int i = 0; i < 13; i++) {
		int rowO = rowOff[i];
		numInRow = (7 - (rowO + 1)) * 2 + 1;
		for (int j = 0; j < numInRow; j++) {
			temp[i][rowO + j] = alv[tc];
			tc++;
		}
	}
}

void DmProfileCreator::checkVals() {
	int i = 0;
	for (auto& voltage : valArray) {
		if (voltage >= 1 || voltage < 0) {
			thrower("Error: voltage on piston " + str(i) + " is out of range");
		}
		i++;
	}
}

void DmProfileCreator::addComa(std::vector<double>& zAmps, double comaMag, double comaAngle) {
		//Adds at a particular angle and magnitude to a list of zernike amplituders.
	double x;
	double y;
	toCartesian(comaMag, comaAngle, x, y);
	zAmps[7] += x;
	zAmps[8] += y;
}

void DmProfileCreator::addAstigmatism(std::vector<double>& zAmps, double astigMag, double astigAngle) {
	double x;
	double y;
	toCartesian(astigMag, astigAngle, x, y);
	zAmps[3] += x;
	zAmps[5] += y;
}

void DmProfileCreator::addTrefoil(std::vector<double>& zAmps, double trefMag, double trefAngle) {
	double x;
	double y;
	toCartesian(trefMag, trefAngle, x, y);
	zAmps[6] += x;
	zAmps[9] += y;
}

void DmProfileCreator::addSpherical(std::vector<double>& zAmps, double sphereMag) {
	zAmps[12] += sphereMag;
}