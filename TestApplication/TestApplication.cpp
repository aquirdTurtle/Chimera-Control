#include "stdafx.h"
#include <iostream>
#include "Python.h"
#include <array>
#include <vector>
#include "Windows.h"
void test1();
void test2();
void test3();
int main()
{
	test1();
	std::cin.get();
	return 0;
}

void test2()
{
	PyObject *pName, *pFunc1, *pFunc;
	PyObject *pArgs1, *pArgs, *pValue;
	std::string date = "160521";
	std::string outputName = "testtesttest";
	long runNumber = 57;
	std::vector<std::pair<int, int>> atomLocations;
	atomLocations.resize(1);
	atomLocations[0] = { 1,3 };
	long accumulations = 150;
	Py_SetPythonHome(L"C:\\Users\\Regal Lab\\Anaconda3");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"C:\\\\Users\\\\Regal Lab\\\\Documents\\\\My Data Analysis\")");
	pName = PyUnicode_DecodeFSDefault("SingleAtomAnalysisFunction");
	// Make sure that python can find my module.
	std::string moduleName = "SingleAtomAnalysisFunction";
	{
		PyObject* pythonModuleName = PyUnicode_DecodeFSDefault(moduleName.c_str());
		PyObject* pythonModule = PyImport_Import(pythonModuleName);
		PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, "hiFunc");
		PyObject* pythonFunctionArguments = PyTuple_New(3);
		PyObject* pythonDate = PyUnicode_DecodeFSDefault(date.c_str());
		PyTuple_SetItem(pythonFunctionArguments, 0, Py_BuildValue("s", "stuff"));
		PyTuple_SetItem(pythonFunctionArguments, 1, PyLong_FromLong(0));
		PyTuple_SetItem(pythonFunctionArguments, 2, PyLong_FromLong(0));
		PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
		std::cout << ("Result of call: " + std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")))
			+ "\r\n").c_str();
		PyErr_Print();
		Py_DECREF(pythonModuleName);
		Py_DECREF(pythonModule);
		Py_DECREF(pythonFunction);
		Py_DECREF(pythonFunctionArguments);
		Py_DECREF(pythonDate);
	} 
	{
		PyObject* pythonModuleName = PyUnicode_DecodeFSDefault(moduleName.c_str());
		PyObject* pythonModule = PyImport_Import(pythonModuleName);
		PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, "hiFunc");
		PyObject* pythonFunctionArguments = PyTuple_New(3);
		PyObject* pythonDate = PyUnicode_DecodeFSDefault(date.c_str());
		PyTuple_SetItem(pythonFunctionArguments, 0, Py_BuildValue("s", "stuff"));
		PyTuple_SetItem(pythonFunctionArguments, 1, PyLong_FromLong(0));
		PyTuple_SetItem(pythonFunctionArguments, 2, PyLong_FromLong(0));
		PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
		std::cout << ("Result of call: " + std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")))
			+ "\r\n").c_str();
		PyErr_Print();
		Py_DECREF(pythonModuleName);
		Py_DECREF(pythonModule);
		Py_DECREF(pythonFunction);
		Py_DECREF(pythonFunctionArguments);
		Py_DECREF(pythonDate);
	}
}

void test1()
{
	PyObject *pName;
	Py_SetPythonHome(L"C:\\Users\\Regal Lab\\Anaconda3");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"C:\\\\Users\\\\Regal Lab\\\\Documents\\\\My Data Analysis\")");
	{
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
		// Make sure that python can find my module.
		//PyObject* pythonModuleName = PyUnicode_DecodeFSDefault(moduleName.c_str());
		if (pName == NULL)
		{
			std::cout << "CAnnot convert module name T.T\r\n";
			return;
		}
		PyObject* pythonModule = PyImport_Import(pName);
		//Py_DECREF(pName);
		if (pythonModule != NULL)
		{
			PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, analysisFunctionName.c_str());
			if (analysisFunctionName == "singlePointAnalysis")
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
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert date\r\n";
						std::cin.get();
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);

					PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
					if (!pythonRunNumber)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert run number\r\n";
						std::cin.get();
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);


					// create the numpy array of atom locations. this is a 1D array, the other code assumes two numbers per picture.
					PyObject* pythonAtomLocationsArray = PyTuple_New(atomLocations.size() * 2);
					for (int atomInc = 0; atomInc < atomLocations.size(); atomInc++)
					{
						PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc, PyLong_FromLong(atomLocations[atomInc].first));
						PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc + 1, PyLong_FromLong(atomLocations[atomInc].second));
					}
					PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);
					// format of function arguments:
					// def analyzeSingleLocation(date, runNumber, atomLocationRow, atomLocationColumn, picturesPerExperiment, accumulations, fileName) :

					// hard-coded for now (might change or remove later...)
					PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2);
					if (!pythonPicturesPerExperiment)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert Pictures per experiment\r\n";
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);

					PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
					if (!pythonAccumulations)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert Accumulations\r\n";
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);

					PyObject* pythonOutputName = PyBytes_FromString(outputName.c_str());
					if (!pythonOutputName)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert Run Number!\r\n";
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 5, Py_BuildValue("s", "stuff"));

					PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
					Py_DECREF(pythonFunctionArguments);
					Py_DECREF(pythonOutputName);
					Py_DECREF(pythonAccumulations);
					Py_DECREF(pythonPicturesPerExperiment);
					Py_DECREF(pythonAtomLocationsArray);
					Py_DECREF(pythonRunNumber);
					Py_DECREF(pythonDate);
					if (pythonReturnValue != NULL)
					{
						std::cout << ("Result of call: " + std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")))
							+ "\r\n").c_str();
						Py_DECREF(pythonReturnValue);
					}
					else
					{
						// get the error details
						PyObject *pExcType, *pExcValue, *pExcTraceback;
						std::string execType, execValue, execTraceback;
						PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
						if (pExcType != NULL)
						{
							PyObject* pRepr = PyObject_Repr(pExcType);
							execType = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
							Py_DecRef(pRepr);
							Py_DecRef(pExcType);
						}
						if (pExcValue != NULL)
						{
							PyObject* pRepr = PyObject_Repr(pExcValue);
							execValue = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
							Py_DecRef(pRepr);
							Py_DecRef(pExcValue);
						}
						if (pExcTraceback != NULL)
						{
							PyObject* pRepr = PyObject_Repr(pExcTraceback);
							execTraceback = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
							Py_DecRef(pRepr);
							Py_DecRef(pExcTraceback);
						}
						std::cout << ("Python Call Failed: " + execType + "; " + execValue + "; " + execTraceback + "\r\n").c_str();
						std::cout << "Failed.\r\n";
						Py_DECREF(pythonFunction);
						Py_DECREF(pythonModule);
						return;
					}
				}
				else
				{
					if (PyErr_Occurred())
					{
						PyErr_Print();
					}
					std::cout << "Failed to load function\r\n";
				}
				Py_XDECREF(pythonFunction);
				//Py_DECREF(pythonModule);
			}
		}
		else
		{
			PyErr_Print();
			std::cout << "Failed to load module\r\n";
			std::cin.get();
			return;
		}
	}
	{
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
		// Make sure that python can find my module.
		//PyObject* pythonModuleName = PyUnicode_DecodeFSDefault(moduleName.c_str());
		if (pName == NULL)
		{
			std::cout << "CAnnot convert module name T.T\r\n";
			return;
		}
		PyObject* pythonModule = PyImport_Import(pName);
		Py_DECREF(pName);
		if (pythonModule != NULL)
		{
			PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, analysisFunctionName.c_str());
			if (analysisFunctionName == "singlePointAnalysis")
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
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert date\r\n";
						std::cin.get();
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);

					PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
					if (!pythonRunNumber)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert run number\r\n";
						std::cin.get();
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);


					// create the numpy array of atom locations. this is a 1D array, the other code assumes two numbers per picture.
					PyObject* pythonAtomLocationsArray = PyTuple_New(atomLocations.size() * 2);
					for (int atomInc = 0; atomInc < atomLocations.size(); atomInc++)
					{
						PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc, PyLong_FromLong(atomLocations[atomInc].first));
						PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc + 1, PyLong_FromLong(atomLocations[atomInc].second));
					}
					PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);
					// format of function arguments:
					// def analyzeSingleLocation(date, runNumber, atomLocationRow, atomLocationColumn, picturesPerExperiment, accumulations, fileName) :

					// hard-coded for now (might change or remove later...)
					PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2);
					if (!pythonPicturesPerExperiment)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert Pictures per experiment\r\n";
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);

					PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
					if (!pythonAccumulations)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert Accumulations\r\n";
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);

					PyObject* pythonOutputName = PyBytes_FromString(outputName.c_str());
					if (!pythonOutputName)
					{
						Py_DECREF(pythonFunctionArguments);
						Py_DECREF(pythonModule);
						std::cout << "Cannot Convert Run Number!\r\n";
						return;
					}
					PyTuple_SetItem(pythonFunctionArguments, 5, Py_BuildValue("s", "stuff"));

					PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
					Py_DECREF(pythonFunctionArguments);
					Py_DECREF(pythonOutputName);
					Py_DECREF(pythonAccumulations);
					Py_DECREF(pythonPicturesPerExperiment);
					Py_DECREF(pythonAtomLocationsArray);
					Py_DECREF(pythonRunNumber);
					Py_DECREF(pythonDate);
					if (pythonReturnValue != NULL)
					{
						std::cout << ("Result of call: " + std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")))
							+ "\r\n").c_str();
						Py_DECREF(pythonReturnValue);
					}
					else
					{
						// get the error details
						PyObject *pExcType, *pExcValue, *pExcTraceback;
						std::string execType, execValue, execTraceback;
						PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
						if (pExcType != NULL)
						{
							PyObject* pRepr = PyObject_Repr(pExcType);
							execType = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
							Py_DecRef(pRepr);
							Py_DecRef(pExcType);
						}
						if (pExcValue != NULL)
						{
							PyObject* pRepr = PyObject_Repr(pExcValue);
							execValue = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
							Py_DecRef(pRepr);
							Py_DecRef(pExcValue);
						}
						if (pExcTraceback != NULL)
						{
							PyObject* pRepr = PyObject_Repr(pExcTraceback);
							execTraceback = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
							Py_DecRef(pRepr);
							Py_DecRef(pExcTraceback);
						}
						std::cout << ("Python Call Failed: " + execType + "; " + execValue + "; " + execTraceback + "\r\n").c_str();
						std::cout << "Failed.\r\n";
						Py_DECREF(pythonFunction);
						Py_DECREF(pythonModule);
						return;
					}
				}
				else
				{
					if (PyErr_Occurred())
					{
						PyErr_Print();
					}
					std::cout << "Failed to load function\r\n";
				}
				Py_XDECREF(pythonFunction);
				//Py_DECREF(pythonModule);
			}
		}
		else
		{
			PyErr_Print();
			std::cout << "Failed to load module\r\n";
			std::cin.get();
			return;
		}
	}
}


/// /////////////////////
 /////////////////////////
/// 
 /////////////////////////
/// /////////////////////

void test3()
{
	PyObject *pName, *pFunc1, *pFunc;
	PyObject *pArgs1, *pArgs, *pValue;
	Py_SetPythonHome(L"C:\\Users\\Regal Lab\\Anaconda3");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"C:\\\\Users\\\\Regal Lab\\\\Documents\\\\My Data Analysis\")");
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
	// Make sure that python can find my module.
	PyObject* pythonModuleName = PyUnicode_DecodeFSDefault(moduleName.c_str());
	if (pythonModuleName == NULL)
	{
		std::cout << "CAnnot convert module name T.T\r\n";
		return;
	}
	PyObject* pythonModule = PyImport_Import(pythonModuleName);
	//Py_DECREF(pythonModuleName);
	if (pythonModule != NULL)
	{
		PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, analysisFunctionName.c_str());
		if (analysisFunctionName == "singlePointAnalysis")
		{
			// make sure this function is okay.
			if (pythonFunction && PyCallable_Check(pythonFunction))
			{
				PyObject* pythonFunctionArguments = PyTuple_New(1);
				PyObject* pythonDate = Py_BuildValue("s", "stuff");
				PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);
				PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
				if (pythonReturnValue != NULL)
				{
					std::cout << ("Result of call: " + std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")))
						+ "\r\n").c_str();
					Py_DECREF(pythonReturnValue);
				}
				else
				{
					// get the error details
					PyObject *pExcType, *pExcValue, *pExcTraceback;
					std::string execType, execValue, execTraceback;
					PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
					if (pExcType != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcType);
						execType = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_DecRef(pRepr);
						Py_DecRef(pExcType);
					}
					if (pExcValue != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcValue);
						execValue = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_DecRef(pRepr);
						Py_DecRef(pExcValue);
					}
					if (pExcTraceback != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcTraceback);
						execTraceback = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_DecRef(pRepr);
						Py_DecRef(pExcTraceback);
					}
					std::cout << ("Python Call Failed: " + execType + "; " + execValue + "; " + execTraceback + "\r\n").c_str();
					std::cout << "Failed.\r\n";
					Py_DECREF(pythonFunction);
					Py_DECREF(pythonModule);
					return;
				}
			}
			else
			{
				if (PyErr_Occurred())
				{
					PyErr_Print();
				}
				std::cout << "Failed to load function\r\n";
			}
			Py_XDECREF(pythonFunction);
			//Py_DECREF(pythonModule);
		}
	}
	else
	{
		PyErr_Print();
		std::cout << "Failed to load module\r\n";
		std::cin.get();
		return;
	}

	// Make sure that python can find my module.
	pythonModuleName = PyUnicode_DecodeFSDefault(moduleName.c_str());
	if (pythonModuleName == NULL)
	{
		std::cout << "Cannot convert module name T.T\r\n";
		return;
	}
	//pythonModule = PyImport_Import(pythonModuleName);
	Py_DECREF(pythonModuleName);
	if (pythonModule != NULL)
	{
		PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, analysisFunctionName.c_str());
		if (analysisFunctionName == "singlePointAnalysis")
		{
			// make sure this function is okay.
			if (pythonFunction && PyCallable_Check(pythonFunction))
			{
				PyObject* pythonFunctionArguments = PyTuple_New(1);
				PyObject* pythonDate = Py_BuildValue("s", "stuff");
				PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);
				PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
				if (pythonReturnValue != NULL)
				{
					std::cout << ("Result of call: " + std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")))
						+ "\r\n").c_str();
					Py_DECREF(pythonReturnValue);
				}
				else
				{
					// get the error details
					PyObject *pExcType, *pExcValue, *pExcTraceback;
					std::string execType, execValue, execTraceback;
					PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
					if (pExcType != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcType);
						execType = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_DecRef(pRepr);
						Py_DecRef(pExcType);
					}
					if (pExcValue != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcValue);
						execValue = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_DecRef(pRepr);
						Py_DecRef(pExcValue);
					}
					if (pExcTraceback != NULL)
					{
						PyObject* pRepr = PyObject_Repr(pExcTraceback);
						execTraceback = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pRepr, "ASCII", "strict")));
						Py_DecRef(pRepr);
						Py_DecRef(pExcTraceback);
					}
					std::cout << ("Python Call Failed: " + execType + "; " + execValue + "; " + execTraceback + "\r\n").c_str();
					std::cout << "Failed.\r\n";
					Py_DECREF(pythonFunction);
					Py_DECREF(pythonModule);
					return;
				}
			}
			else
			{
				if (PyErr_Occurred())
				{
					PyErr_Print();
				}
				std::cout << "Failed to load function\r\n";
			}
			Py_XDECREF(pythonFunction);
			Py_DECREF(pythonModule);
		}
	}
	else
	{
		PyErr_Print();
		std::cout << "Failed to load module\r\n";
		std::cin.get();
	}


	std::cin.get();

}