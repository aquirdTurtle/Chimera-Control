// TestApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <process.h>
#include "stdlib.h"
#include <vector>
#include <string>
#include "Windows.h"
#include "stdio.h"
#include <iostream>
#define WIDTH 50
#define HEIGHT 50
#define MYVECSIZE WIDTH*HEIGHT

int main()
{
	std::cout << "Starting" << std::endl;
	std::vector<int> myVec, myVec2;
	long long realInit = GetTickCount64();
	for (int picInc = 0; picInc < 1000; picInc++)
	{
		myVec.clear();
		myVec.resize(MYVECSIZE);
		myVec2.resize(MYVECSIZE);
		//long long init = GetTickCount64();
		for (int randInc = 0; randInc < MYVECSIZE; randInc++)
		{
			myVec[randInc] = rand() % 10;
		}
		for (int imageVecInc = 0; imageVecInc < MYVECSIZE; imageVecInc++)
		{
			int number = ((imageVecInc % (WIDTH)) + 1) * (HEIGHT) - imageVecInc / (WIDTH) - 1;
			myVec2[imageVecInc] = myVec[number];
		}
		//long long fin = GetTickCount64();
		//std::cout << fin - init;
	}
	long long realFin = GetTickCount64();
	std::cout << "final output:" << realFin - realInit;
	std::cin.get();
	//MessageBox(0, std::to_string(()).c_str(), 0, 0);
    return 0;
}

