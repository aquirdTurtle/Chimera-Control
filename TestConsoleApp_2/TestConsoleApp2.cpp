// TestConsoleApp2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <string>
#include <vector>
#include "afxwin.h"

struct optParamSettings
{
	std::string name;
	double currentValue;
	// search limits
	double lowerLim;
	double upperLim;
	// important: this is the index within the MachineOptimizer class member vector of params, not within the original 
	// parametersystem vector of variables.
	UINT index = 0;
	double limitSizeRange ( )
	{
		return upperLim - lowerLim;
	}
	std::vector<double> valueHist;
	std::vector<double> resultHist;
};


int main()
{
	std::shared_ptr<optParamSettings> test;
	test->valueHist.push_back ( 10 );
    return 0;
}

