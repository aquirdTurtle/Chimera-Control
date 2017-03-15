#pragma once
#include "python.h"
#include <string>
//#include "SMSTextingControl.h"

struct personInfo;

class EmbeddedPythonHandler
{
	public:
		// constructor is important.
		EmbeddedPythonHandler();
		// for full data analysis set.
		void runDataAnalysis(std::string date, long runNumber, long accumulations,
			std::vector<std::pair<int, int>> atomLocations);
		// for texting.
		void sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail,
			std::string password);
		// for a single python command.
		std::string run(std::string cmd, bool flush = true);
		void flush();
	private:
		PyObject* autoAnalysisModule;
		PyObject* atomAnalysisFunction;
		PyObject* mainModule;
		PyObject* errorCatcher;
};