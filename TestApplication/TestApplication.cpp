#include "stdafx.h"
#include <iostream>
#include "Python.h"
#include <array>
#include <vector>

int main()
{
	PyObject *pName, *pFunc1, *pFunc, *pythonModule;
	PyObject *pArgs1, *pArgs, *pValue;
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"C:\\\\Users\\\\Mark\\\\Documents\\\\My Data Analysis\")");
	std::string analysisFunctionName = "singlePointAnalysis";
	std::string moduleName = "SingleAtomAnalysisFunction";
	std::string date = "160521";
	std::string outputName = "testtesttest";
	long runNumber = 57;
	std::vector<std::pair<int, int>> atomLocations;
	atomLocations.resize(1);
	atomLocations[0] = { 1,3 };
	long accumulations = 150;
	pName = PyUnicode_DecodeFSDefault("SingleAtomAnalysisFunction");
	if (pName == NULL)
	{
		return 2;
	}
	pythonModule = PyImport_Import(pName);
	if (pythonModule != NULL)
	{
		pFunc1 = PyObject_GetAttrString(pythonModule, "singlePointAnalysis");
		/* pFunc is a new reference */
		if (pFunc1 && PyCallable_Check(pFunc1)) 
		{
			PyObject* pythonFunctionArguments = PyTuple_New(6);
			PyObject* pythonDate = PyUnicode_DecodeFSDefault(date.c_str());
			// check success
			if (!pythonDate)
			{
				Py_DECREF(pythonFunctionArguments);
				Py_DECREF(pythonModule);
				std::cin.get();
				return 1;
			}
			PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);

			PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
			if (!pythonRunNumber)
			{
				Py_DECREF(pythonFunctionArguments);
				Py_DECREF(pythonModule);
				std::cin.get();
				return 1;
			}
			PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);

			PyObject* pythonAtomLocationsArray = PyTuple_New(6);
			PyTuple_SetItem(pythonAtomLocationsArray, 0, PyLong_FromLong(1));
			PyTuple_SetItem(pythonAtomLocationsArray, 1, PyLong_FromLong(3));
			PyTuple_SetItem(pythonAtomLocationsArray, 2, PyLong_FromLong(1));
			PyTuple_SetItem(pythonAtomLocationsArray, 3, PyLong_FromLong(5));
			PyTuple_SetItem(pythonAtomLocationsArray, 4, PyLong_FromLong(0));
			PyTuple_SetItem(pythonAtomLocationsArray, 5, PyLong_FromLong(0));
			PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);
			// hard-coded for now (might change or remove later...)
			PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2);
			if (!pythonPicturesPerExperiment)
			{
				Py_DECREF(pythonFunctionArguments);
				Py_DECREF(pythonModule);
				return 1;
			}
			PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);


			PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
			if (!pythonAccumulations)
			{
				Py_DECREF(pythonFunctionArguments);
				Py_DECREF(pythonModule);
				return 1;
			}
			PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);

			PyObject* pythonOutputName = PyUnicode_DecodeFSDefault(outputName.c_str());
			if (!pythonOutputName)
			{
				Py_DECREF(pythonFunctionArguments);
				Py_DECREF(pythonModule);
				return 1;
			}
			PyTuple_SetItem(pythonFunctionArguments, 5, pythonOutputName);

			PyObject* pythonReturnValue = PyObject_CallObject(pFunc1, pythonFunctionArguments);
			Py_DECREF(pythonFunctionArguments);
			Py_DECREF(pythonOutputName);
			Py_DECREF(pythonAccumulations);
			Py_DECREF(pythonPicturesPerExperiment);
			Py_DECREF(pythonAtomLocationsArray);
			Py_DECREF(pythonRunNumber);
			Py_DECREF(pythonDate);
			Py_DECREF(pythonAtomLocationsArray);
			//Py_DECREF(pArgs);
			if (pythonReturnValue != NULL)
			{
				printf("Result of call: %s\n", PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")));
				Py_DECREF(pythonReturnValue);
			}
			else 
			{
				Py_DECREF(pFunc1);
				Py_DECREF(pythonModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
				std::cin.get();
				return 1;
			}
		}
		else 
		{
			if (PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \n");
		}
		Py_XDECREF(pFunc1);
		Py_DECREF(pythonModule);
	}
	else 
	{
		PyErr_Print();
		fprintf(stderr, "Failed to load Module.\n");
		std::cin.get();
		return 1;
	}
	std::cin.get();
	//Py_Finalize();
	std::cout << "DONE";
	std::cin.get();
	return 0;
}
