#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif

#include <string>
#include "coordinate.h"

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
		std::string run(std::string cmd, bool flush = true);
		void flush();
		void runPlotDacs( );
		void runPlotTtls( );
	private:
		PyObject* autoAnalysisModule;
		PyObject* atomAnalysisFunction;
		PyObject* mainModule;
		PyObject* errorCatcher;
};