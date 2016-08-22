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
							"sys.stdout = catchOutErr\n"
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
	ERR_POP(run(("sys.path.append(\"" + ANALYSIS_CODE_LOCATION + "\")").c_str()));
	PyObject* pythonModuleName = PyUnicode_DecodeFSDefault("AutoanalysisFunctions");
	this->autoAnalysisModule = PyImport_Import(pythonModuleName);

}

EmbeddedPythonHandler::~EmbeddedPythonHandler()
{

}
// for full data analysis set.
bool EmbeddedPythonHandler::runDataAnalysis(std::string analysisType, std::string date, long runNumber,
											long accumulations, std::string completeName, std::vector<std::pair<int, int>> atomLocations)
{
	std::string analysisFunctionName;
	// interpret the text here to get the actual function name.
	if (analysisType == "Single Point Analysis")
	{
		analysisFunctionName = "singlePointAnalysis";
	}
	else if (analysisType == "Pair Analysis")
	{
		analysisFunctionName = "pairAnalysis";
	}
	else
	{
		errBox("ERROR: unrecognized analysis type while trying to figure out the analysis function name! Ask Mark about bugs.");
		return true;
	}
	if (this->autoAnalysisModule != NULL)
	{
		PyObject* pythonFunction = PyObject_GetAttrString(autoAnalysisModule, analysisFunctionName.c_str());
		// function calls for these functions are the same because they are so simple.
		if (analysisFunctionName == "singlePointAnalysis" || analysisFunctionName == "pairAnalysis")
		{
			// make sure this function is okay.
			if (pythonFunction && PyCallable_Check(pythonFunction))
			{
				PyObject* pythonFunctionArguments = PyTuple_New(6);

				PyObject* pythonDate = Py_BuildValue("s", date.c_str());
				// check success
				if (!pythonDate)
				{
					Py_DECREF(pythonFunctionArguments);
					Py_DECREF(autoAnalysisModule);
					errBox("ERROR: Cannot Convert date!");
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);

				PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
				if (!pythonRunNumber)
				{
					Py_DECREF(pythonFunctionArguments);
					Py_DECREF(autoAnalysisModule);
					errBox("Cannot Convert run number\r\n");
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);


				// create the numpy array of atom locations. this is a 1D array, the other code assumes two numbers per picture.
				PyObject* pythonAtomLocationsArray = PyTuple_New(atomLocations.size() * 2);
				for (int atomInc = 0; atomInc < atomLocations.size(); atomInc++)
				{
					// order is flipped.
					PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc, PyLong_FromLong(atomLocations[atomInc].second));
					PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc + 1, PyLong_FromLong(atomLocations[atomInc].first));
				}
				PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);
				// format of function arguments:
				// def analyzeSingleLocation(date, runNumber, atomLocationRow, atomLocationColumn, picturesPerExperiment, accumulations, fileName) :
				// hard-coded for now (might change or remove later...)
				PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2);
				if (!pythonPicturesPerExperiment)
				{
					Py_DECREF(pythonFunctionArguments);
					errBox("Cannot Convert Pictures per experiment\r\n");
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);

				PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
				if (!pythonAccumulations)
				{
					Py_DECREF(pythonFunctionArguments);
					errBox("Cannot Convert Accumulations\r\n");
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);

				PyObject* pythonOutputName = Py_BuildValue("s", completeName.c_str());
				if (!pythonOutputName)
				{
					Py_DECREF(pythonFunctionArguments);
					errBox("Cannot Convert Run Number!\r\n");
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 5, pythonOutputName);

				PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
				// I think not including this line below is a memory leak but I also think that it might be the cause of some annoying memory issues... not sure which, maybe both.
				//Py_DECREF(pythonFunctionArguments);
				if (pythonReturnValue != NULL)
				{
					//MessageBox(0, "About to output.", 0, 0);
					std::string result = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")));
					Py_DECREF(pythonReturnValue);
				}
				else
				{
					// get the error details1
					PyObject *pExcType, *pExcValue, *pExcTraceback;
					std::string execType, execValue, execTraceback = "";
					PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
					PyErr_NormalizeException(&pExcType, &pExcValue, &pExcTraceback);
					if (pExcType != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcType);
						execType = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_XDECREF(pRepr);
						Py_XDECREF(pExcType);
					}
					if (pExcValue != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcValue);
						execValue = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_XDECREF(pRepr);
						Py_XDECREF(pExcValue);
					}
					if (pExcTraceback != NULL)
					{
						/*
						PyObject* pRepr = PyObject_Repr(pExcTraceback);
						execTraceback = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_XDECREF(pRepr);
						Py_XDECREF(pExcTraceback);
						*/
					}
					//std::string fileName = std::string(PyBytes_AS_STRING((traceback->tb_frame->f_code->co_filename), "ASCII", "string"));
					errBox("Python Call Failed: " + execType + "; " + execValue + "; " + execTraceback + "\r\n");
					return true;
				}
			}
			else
			{
				errBox("Failed to load function\r\n");
				return true;
			}
			Py_XDECREF(pythonFunction);
		}
	}
	else
	{
		errBox("Failed to load module\r\n");
		return true;
	}
	return false;
}

// for texting.
bool EmbeddedPythonHandler::sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail, 
									 std::string password)
{
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
	PyRun_SimpleString(cmd.c_str());
	//make python print any errors
	PyErr_Print();
	//get the stdout and stderr from our catchOutErr object
	PyObject *output = PyObject_GetAttrString(errorCatcher, "value");
	return PyBytes_AS_STRING(PyUnicode_AsEncodedString(output, "ASCII", "strict"));
}
