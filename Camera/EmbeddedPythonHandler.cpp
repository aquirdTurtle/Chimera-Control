#include "stdafx.h"
// main header.
#include "EmbeddedPythonHandler.h"
// for personInfo structure.
#include "SMSTextingControl.h"

// constructor is important.
EmbeddedPythonHandler::EmbeddedPythonHandler()
{
	Py_SetPythonHome(PYTHON_HOME);
	Py_Initialize();
	std::string stdOutErr = "import sys\n"
							"class CatchOutErr:\n"
							"\tdef __init__(self):\n"
							"\t\tself.value = ''\n"
							"\tdef write(self, txt):\n"
							"\t\tself.value += txt\n"
							"catchOutErr = CatchOutErr()\n"
							"sys.stderr = catchOutErr\n";
	//create main module
	this->mainModule = PyImport_AddModule("__main__"); 
	//invoke code to redirect
	PyRun_SimpleString(stdOutErr.c_str()); 
	//get our catchOutErr object (of type CatchOutErr) created above
	this->errorCatcher = PyObject_GetAttrString(mainModule, "catchOutErr"); 
	// start using the run function.
	this->run("from astropy.io import fits");
	this->run("import numpy");
	ERR_POP(run("import numpy as np"));
	ERR_POP(run("import matplotlib.pyplot as plt"));
	ERR_POP(run("import smtplib"));
	ERR_POP(run("from email.mime.text import MIMEText"));
	// not sure if these things are needed...
	ERR_POP(run("numpy.set_printoptions(threshold = numpy.nan)"));
	ERR_POP(run("from matplotlib.pyplot import figure, hist, plot, title, xlabel, ylabel, subplots, errorbar, show, draw"));
	ERR_POP(run("from matplotlib.cm import get_cmap"));
	//ERR_POP(run("from dataAnalysisFunctions import normalizeData, binData, guessGaussianPeaks, doubleGaussian, fitDoubleGaussian,"
	//	"calculateAtomThreshold, getAnalyzedSurvivalData"));
	// Make sure that python can find my module.
	ERR_POP(run("import sys"));
	ERR_POP(run("sys.path.append(\"" + ANALYSIS_CODE_LOCATION + "\")"));
	ERR_POP(run("import AutoanalysisFunctions"));
	PyObject* pythonModuleName = PyUnicode_DecodeFSDefault("AutoanalysisFunctions");
	if (pythonModuleName == NULL)
	{
		errBox("ERROR: failed to interpret \"AutoanalysisFunctions\" as python string?!?!?!?!?");
		return;
	}
	this->autoAnalysisModule = PyImport_Import(pythonModuleName);
	//Py_XDECREF(pythonModuleName);
	if (autoAnalysisModule == NULL)
	{
		errBox("ERROR: Failed to load python module for automatic data analysis!");
		return;
	}
	this->atomAnalysisFunction = PyObject_GetAttrString(autoAnalysisModule, "atomAnalysis");
	if (this->atomAnalysisFunction == NULL)
	{
		errBox("Failed to load python function \"singlePointAnalysis\"");
	}
	return;
}

bool EmbeddedPythonHandler::flush()
{
	//this->run("sys.stderr.flush()");
	return true;
}
// for full data analysis set.
bool EmbeddedPythonHandler::runDataAnalysis(std::string date, long runNumber, long accumulations, 
											std::vector<std::pair<int, int>> atomLocations)
{
	this->flush();
	if (this->autoAnalysisModule == NULL)
	{
		errBox("autoAnalysisModule is no longer available! This shouldn't happen... Continuing...");
	}
	// interpret the text here to get the actual function name.
	if (this->atomAnalysisFunction == NULL)
	{
		errBox("ERROR: Atom analysis function is null! The program can no longer call this function for some"
			"reason. Auto-Analysis will not occur.");
		return true;
	}
	if (!PyCallable_Check(this->atomAnalysisFunction))
	{
		errBox("ERROR: Python is telling me that it cannot call the Atom analysis function. I don't know why"
			", since the function pointer is not null. Auto-Analysis will not occur.");
		return true;
	}
		
	// I'm going to use comments before relevant commands to keep track of which python objects have references that I 
	// own, starting below (not counting the module and function references)

	// pythonFunctionArguments
	PyObject* pythonFunctionArguments = PyTuple_New(5);
	if (pythonFunctionArguments == NULL)
	{
		errBox("ERROR: creating tuple for python function arguments failed!?!?!?!? Auto-Analysis will terminate.");
		return true;
	}
	// pythonFunctionArguments, pythonDate
	PyObject* pythonDate = Py_BuildValue("s", date.c_str());
	// check success
	if (pythonDate == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		errBox("ERROR: Cannot Convert date! Auto-Analysis will terminate.");
		return true;
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);
	// pythonFunctionArguments, pythonRunNumber
	PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
	if (pythonRunNumber == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		errBox("Cannot Convert run number?!?!?!?!?! Auto-Analysis terminating...\r\n");
		return true;
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);

	// create the numpy array of atom locations. this is a 1D array, the other code assumes two numbers per atom, and 
	// figures out based on the function call whether it should look at pairs of atoms or not. If pairs, it assumes 
	// 1a, 1b, 2a, 2b, etc. formatting.
	// pythonFunctionArguments, pythonAtomLocationsArray
	PyObject* pythonAtomLocationsArray = PyTuple_New(atomLocations.size() * 2);
	for (int atomInc = 0; atomInc < atomLocations.size(); atomInc++)
	{
		// order is flipped. Dunno why...
		// PyTuple immediately steals the reference from PyLong_FromLong, so I don't need to handle any of these. 
		PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc, PyLong_FromLong(atomLocations[atomInc].second));
		PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc + 1, PyLong_FromLong(atomLocations[atomInc].first));
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);

	// format of function arguments:
	// def analyzeSingleLocation(date, runNumber, atomLocationRow, atomLocationColumn, picturesPerExperiment, accumulations, fileName) :
	// hard-coded for now to 2. (might change or remove later...)
	// pythonFunctionArguments, pythonPicturesPerExperiment
	PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2);
	if (pythonPicturesPerExperiment == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		errBox("Cannot Convert Pictures per experiment?!?!?!?!? Auto-Analysis terminating...");
		return true;
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);
	// pythonFunctionArguments, pythonAccumulations
	PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
	if (pythonAccumulations == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		errBox("Cannot Convert Accumulations?!?!?!?!?!?!?!?!?!? Auto-Analysis terminating...");
		return true;
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);
	PyObject* pythonReturnValue = PyObject_CallObject(this->atomAnalysisFunction, pythonFunctionArguments);
	if (pythonReturnValue == NULL)
	{
		errBox("Python function call returned NULL!");
		PyErr_Print();
		PyObject *output = PyObject_GetAttrString(errorCatcher, "value");
		errBox(PyBytes_AS_STRING(PyUnicode_AsEncodedString(output, "ASCII", "strict")));
		return true;
	}
	Py_DECREF(pythonReturnValue);
	// finished successfully.
	return false;
}

// for texting.
bool EmbeddedPythonHandler::sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail, 
									 std::string password)
{
	this->flush();
	if (baseEmail == "" || password == "")
	{
		errBox("ERROR: Please set an email and password for sending texts with!");
		return true;
	}

	ERR_POP_RETURN(run("email = MIMEText('" + msg + "', 'plain')"));
	ERR_POP_RETURN(run("email['Subject'] = '" + subject + "'"));
	ERR_POP_RETURN(run("email['From'] = '" + baseEmail + "'"));
	std::string recipient = person.number;
	if (person.provider == "verizon")
	{
		// for verizon: [number]@vzwpix.com
		recipient += "@vzwpix.com";
	}
	else if (person.provider == "tmobile")
	{
		// for tmobile: [number]@tmomail.net
		recipient += "@tmomail.net";
	}
	ERR_POP_RETURN(run("email['To'] = '" + recipient + "'"));
	ERR_POP_RETURN(run("mail = smtplib.SMTP('smtp.gmail.com', 587)"));
	ERR_POP_RETURN(run("mail.ehlo()"));
	ERR_POP_RETURN(run("mail.starttls()"));
	ERR_POP_RETURN(run("mail.login('" + baseEmail + "', '" + password + "')"));
	ERR_POP_RETURN(run("mail.sendmail(email['From'], email['To'], email.as_string())"));

	return false;
}

// for a single python command. Returns python's output of said command.
std::string EmbeddedPythonHandler::run(std::string cmd)
{
	this->flush();
	PyRun_SimpleString(cmd.c_str());
	// get the stdout and stderr from our catchOutErr object
	PyObject *output = PyObject_GetAttrString(errorCatcher, "value");
	return PyBytes_AS_STRING(PyUnicode_AsEncodedString(output, "ASCII", "strict"));
}
