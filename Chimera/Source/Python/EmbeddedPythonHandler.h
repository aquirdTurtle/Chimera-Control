// created by Mark O. Brown
#pragma once

#ifdef slots
#undef slots
#endif
#ifdef _DEBUG
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif

#include "GeneralObjects/coordinate.h"
#include <vector>
#include <string>

struct personInfo;

class EmbeddedPythonHandler
{
	public:
		// constructor is important.
		EmbeddedPythonHandler::EmbeddedPythonHandler();
		// for full data analysis set.
		void runDataAnalysis( std::string date, long runNumber, long accumulations, 
							  std::vector<coordinate> atomLocations );
		// for texting.
		void sendText( personInfo person, std::string msg, std::string subject, std::string baseEmail,
					   std::string password );
		// for a single python command.
		void run(std::string cmd, bool quiet = false, bool flush = true);
		void thresholdAnalysis ( std::string dateString, int fid, std::string analysisLocsString, int picsPerRep );
		void flush();
	private:
		PyObject* autoAnalysisModule;
		PyObject* atomAnalysisFunction;
		PyObject* mainModule;
		PyObject* errorCatcher;
};