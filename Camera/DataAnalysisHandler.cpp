#include "stdafx.h"
#include "DataAnalysisHandler.h"
#include "constants.h"
#include "reorganizeControl.h"
#include "dialogProcedures.h"
#include "Python.h"
#include <process.h>
#include "appendText.h"
#include "myAndor.h"
#include "frameobject.h"

DataAnalysisHandler::DataAnalysisHandler()
{
	Py_SetPythonHome(L"C:\\Users\\Regal Lab\\Anaconda3\\");
	Py_Initialize();
	PyRun_SimpleString("from astropy.io import fits");
	PyRun_SimpleString("import numpy");
	PyRun_SimpleString("numpy.set_printoptions(threshold = numpy.nan)");
	PyRun_SimpleString("from matplotlib.pyplot import figure, hist, plot, title, xlabel, ylabel, subplots, errorbar, show, draw");
	PyRun_SimpleString("from matplotlib.cm import get_cmap");
	PyRun_SimpleString("from dataAnalysisFunctions import normalizeData, binData, guessGaussianPeaks, doubleGaussian, fitDoubleGaussian,"
					   "calculateAtomThreshold, getAnalyzedSurvivalData");
	// Make sure that python can find my module.
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(("sys.path.append(\"" + ANALYSIS_CODE_LOCATION + "\")").c_str());
	PyObject* pythonModuleName = PyUnicode_DecodeFSDefault("AutoanalysisFunctions");
	pythonModule = PyImport_Import(pythonModuleName);
}

DataAnalysisHandler::~DataAnalysisHandler()
{
	// this seems unncecessary since this class is a global singleton, and it tends to cause the code to crash upon exit for some reason. This might be fixed now.
	//Py_Finalize();
}

bool DataAnalysisHandler::reorganizeControls(RECT parentRectangle, std::string cameraMode)
{
	reorganizeControl(titleBox, cameraMode, parentRectangle);
	reorganizeControl(currentDataSetNumberText, cameraMode, parentRectangle);
	reorganizeControl(currentDataSetNumberEdit, cameraMode, parentRectangle);
	reorganizeControl(dataOutputNameText, cameraMode, parentRectangle);
	reorganizeControl(dataOutputNameCombo, cameraMode, parentRectangle);
	reorganizeControl(autoAnalyzeCheckBox, cameraMode, parentRectangle);
	reorganizeControl(autoAnalysisText, cameraMode, parentRectangle);
	reorganizeControl(autoAnalysisTypeCombo, cameraMode, parentRectangle);
	reorganizeControl(setAnalysisLocationsButton, cameraMode, parentRectangle);
	reorganizeControl(analyzeMostRecentButton, cameraMode, parentRectangle);
	return false;
}

bool DataAnalysisHandler::initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
	HWND parentWindow, bool isTriggerModeSensitive)
{
	// Title for the whole control
	titleBox.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	titleBox.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	titleBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	RECT initPos = titleBox.kineticSeriesModePos;
	titleBox.hwnd = CreateWindowEx(0, "STATIC", "Data Analysis", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	titleBox.fontType = "Heading";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	//
	currentDataSetNumberText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 400, topLeftPositionKinetic.y + 25 };
	currentDataSetNumberText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 400, topLeftPositionAccumulate.y + 25 };
	currentDataSetNumberText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberText.kineticSeriesModePos;
	currentDataSetNumberText.hwnd = CreateWindowEx(0, "STATIC", "Most Recent Data Set #:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	currentDataSetNumberText.fontType = "Normal";
	//
	currentDataSetNumberEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 400, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	currentDataSetNumberEdit.accumulateModePos = { topLeftPositionAccumulate.x + 400, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	currentDataSetNumberEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberEdit.kineticSeriesModePos;
	currentDataSetNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "?", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_MOST_RECENT_DATA_SET_NUMBER, eHInst, NULL);
	currentDataSetNumberEdit.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;

	autoAnalyzeCheckBox.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 20 };
	autoAnalyzeCheckBox.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 20 };
	autoAnalyzeCheckBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalyzeCheckBox.kineticSeriesModePos;
	autoAnalyzeCheckBox.hwnd = CreateWindowEx(0, "BUTTON", "Automatically Analyze Data at Finish?", WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_AUTOANALYZE_CHECKBOX, eHInst, NULL);
	autoAnalyzeCheckBox.fontType = "Normal";
	topLeftPositionKinetic.y += 20;
	topLeftPositionAccumulate.y += 20;

	dataOutputNameText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 100, topLeftPositionKinetic.y + 25 };
	dataOutputNameText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 100, topLeftPositionAccumulate.y + 25 };
	dataOutputNameText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameText.kineticSeriesModePos;
	dataOutputNameText.hwnd = CreateWindowEx(0, "EDIT", "Filename:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	dataOutputNameText.fontType = "Normal";

	dataOutputNameCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 100, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 380, topLeftPositionKinetic.y + 800 };
	dataOutputNameCombo.accumulateModePos = { topLeftPositionAccumulate.x + 100, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 380, topLeftPositionAccumulate.y + 800 };
	dataOutputNameCombo.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameCombo.kineticSeriesModePos;
	dataOutputNameCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_OUTPUT_NAME_COMBO, eHInst, NULL);
	dataOutputNameCombo.fontType = "Normal";
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Carrier_Calibration");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Sideband_Spectrum");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Microwave_Frequency_Spectrum");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Microwave_Rabi");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Microwave_Ramsey");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Microwave_Ramsey_Echo");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Piezo_Scan");
	// TODO: add more.
	dataOutputNameDetailCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 380, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 800 };
	dataOutputNameDetailCombo.accumulateModePos = { topLeftPositionAccumulate.x + 380, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 800 };
	dataOutputNameDetailCombo.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameDetailCombo.kineticSeriesModePos;
	dataOutputNameDetailCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_OUTPUT_NAME_DETAILS_COMBO, eHInst, NULL);
	dataOutputNameDetailCombo.fontType = "Normal";
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"");
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Top");
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Bottom");
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Axial");
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Radial");
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Vertical");
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Horizontal");

	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;

	autoAnalysisText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 150, topLeftPositionKinetic.y + 25 };
	autoAnalysisText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 150, topLeftPositionAccumulate.y + 25 };
	autoAnalysisText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalysisText.kineticSeriesModePos;
	autoAnalysisText.hwnd = CreateWindowEx(0, "EDIT", "Autoanalysis Type:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	autoAnalysisText.fontType = "Normal";

	autoAnalysisTypeCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 150, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 800 };
	autoAnalysisTypeCombo.accumulateModePos = { topLeftPositionAccumulate.x + 150, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 800};
	autoAnalysisTypeCombo.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalysisTypeCombo.kineticSeriesModePos;
	autoAnalysisTypeCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_AUTOANALYSIS_COMBO, eHInst, NULL);
	autoAnalysisTypeCombo.fontType = "Normal";
	SendMessage(autoAnalysisTypeCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Single Point Analysis");
	SendMessage(autoAnalysisTypeCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Pair Analysis");
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	// 
	setAnalysisLocationsButton.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	setAnalysisLocationsButton.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	setAnalysisLocationsButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = setAnalysisLocationsButton.kineticSeriesModePos;
	setAnalysisLocationsButton.hwnd = CreateWindowEx(0, "BUTTON", "Set AutoAnalysis Points", WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_SET_ANALYSIS_LOCATION, eHInst, NULL);
	setAnalysisLocationsButton.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;

	//
	analyzeMostRecentButton.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	analyzeMostRecentButton.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	analyzeMostRecentButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = analyzeMostRecentButton.kineticSeriesModePos;
	analyzeMostRecentButton.hwnd = CreateWindowEx(0, "BUTTON", "Analyze Most Recent Data", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_ANALYZE_MOST_RECENT, eHInst, NULL);
	analyzeMostRecentButton.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	return false;
}

bool DataAnalysisHandler::updateDataSetNumberEdit(int number)
{
	if (number > 0)
	{
		SendMessage(currentDataSetNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(number).c_str());
	}
	else
	{
		SendMessage(currentDataSetNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)"None");
	}
	return false;
}

// adds an output name to the output filename combo.
bool DataAnalysisHandler::addNameToCombo()
{
	std::string newOutputName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)"Please enter a new name to call the auto-analyzed data files.\r\nThis name will be temporary (removed upon program restart), please ask Mark to make any permantent changes.");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)newOutputName.c_str());
	return false;
}

bool DataAnalysisHandler::analyze(std::string date, long runNumber, long accumulations)
{
	std::string analysisFunctionName, analysisType;
	std::string outputName, details;
	// get analysis type
	int selectedNumber = SendMessage(autoAnalysisTypeCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR text[256];
	SendMessage(autoAnalysisTypeCombo.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	analysisType = std::string(text);
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
		MessageBox(0, "ERROR: unrecognized analysis type while trying to figure out the analysis function name! Ask Mark about bugs.", 0, 0);
		return true;
	}
	// get output name
	selectedNumber = SendMessage(dataOutputNameCombo.hwnd, CB_GETCURSEL, 0, 0);
	SendMessage(dataOutputNameCombo.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	outputName = std::string(text);
	selectedNumber = SendMessage(dataOutputNameDetailCombo.hwnd, CB_GETCURSEL, 0, 0);
	SendMessage(dataOutputNameDetailCombo.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	details = std::string(text);
	// python is initialized in the constructor for the data handler object. 
	appendText("Beginning Data Analysis... ", IDC_STATUS_EDIT);
	// Get information to send to the python script from inputParam
	std::string moduleName = "SingleAtomAnalysisFunction";

	if (pythonModule != NULL)
	{
		PyObject* pythonFunction = PyObject_GetAttrString(pythonModule, analysisFunctionName.c_str());
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
					Py_DECREF(pythonModule);
					appendText("Cannot Convert date\r\n", IDC_ERROR_EDIT);
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);

				PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
				if (!pythonRunNumber)
				{
					Py_DECREF(pythonFunctionArguments);
					Py_DECREF(pythonModule);
					appendText("Cannot Convert run number\r\n", IDC_ERROR_EDIT);
					std::cin.get();
					return 1;
				}
				PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);


				// create the numpy array of atom locations. this is a 1D array, the other code assumes two numbers per picture.
				PyObject* pythonAtomLocationsArray = PyTuple_New(atomLocations.size() * 2);
				for (int atomInc = 0; atomInc < atomLocations.size(); atomInc++)
				{
					// order is flipped.
					PyTuple_SetItem(pythonAtomLocationsArray, 2*atomInc, PyLong_FromLong(atomLocations[atomInc].second));
					PyTuple_SetItem(pythonAtomLocationsArray, 2*atomInc + 1, PyLong_FromLong(atomLocations[atomInc].first));
				}
				PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);
				// format of function arguments:
				// def analyzeSingleLocation(date, runNumber, atomLocationRow, atomLocationColumn, picturesPerExperiment, accumulations, fileName) :
				// hard-coded for now (might change or remove later...)
				PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2);
				if (!pythonPicturesPerExperiment)
				{
					Py_DECREF(pythonFunctionArguments);
					appendText("Cannot Convert Pictures per experiment\r\n", IDC_ERROR_EDIT);
					return true;
				}
				PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);

				PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
				if (!pythonAccumulations)
				{
					Py_DECREF(pythonFunctionArguments);
					appendText("Cannot Convert Accumulations\r\n", IDC_ERROR_EDIT);
					return 1;
				}
				PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);
				// new:
				PyObject* pythonOutputName = Py_BuildValue("s", (outputName + "_" + details).c_str());
				if (!pythonOutputName)
				{
					Py_DECREF(pythonFunctionArguments);
					appendText("Cannot Convert Run Number!\r\n", IDC_ERROR_EDIT);
					return 1;
				}
				PyTuple_SetItem(pythonFunctionArguments, 5, pythonOutputName);

				PyObject* pythonReturnValue = PyObject_CallObject(pythonFunction, pythonFunctionArguments);
				// I think not including this line below is a memory leak but I also think that it might be the cause of some annoying memory issues... not sure which, maybe both.
				//Py_DECREF(pythonFunctionArguments);
				if (pythonReturnValue != NULL)
				{
					//MessageBox(0, "About to output.", 0, 0);
					std::string result = std::string(PyBytes_AS_STRING(PyUnicode_AsEncodedString(pythonReturnValue, "ASCII", "strict")));
					appendText("Result of call: " + result + "\r\n", IDC_ERROR_EDIT);
					Py_DECREF(pythonReturnValue);
				}
				else
				{
					// get the error details
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
					appendText("Python Call Failed: " + execType + "; " + execValue + "; " + execTraceback + "\r\n", IDC_ERROR_EDIT);
					appendText("Failed.\r\n", IDC_STATUS_EDIT);
					return 1;
				}
			}
			else
			{
				if (PyErr_Occurred())
				{
					PyErr_Print();
				}
				appendText("Failed to load function\r\n", IDC_ERROR_EDIT);
			}
			Py_XDECREF(pythonFunction);
		}
	}
	else
	{
		appendText("Failed to load module\r\n", IDC_ERROR_EDIT);
		return 1;
	}
	appendText("Finished!\r\n", IDC_STATUS_EDIT);
	return false;
}

// handles the pressing of the analysis points button.
// TODO: handle different cases where single locations or pairs of locations are being analyzed. 
bool DataAnalysisHandler::onButtonPushed()
{
	BOOL checked = SendMessage(setAnalysisLocationsButton.hwnd, BM_GETCHECK, 0, 0);
	if (checked)
	{
		SendMessage(setAnalysisLocationsButton.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(setAnalysisLocationsButton.hwnd, WM_SETTEXT, 0, (LPARAM)"Set Analysis Points");
		eSettingAnalysisLocations = false;
	}
	else
	{
		atomLocations.clear();
		SendMessage(setAnalysisLocationsButton.hwnd, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(setAnalysisLocationsButton.hwnd, WM_SETTEXT, 0, (LPARAM)"Right-Click Relevant Points and Reclick");
		eSettingAnalysisLocations = true;
	}			
	int experimentPictureNumber;
	if (eRealTimePictures)
	{
		experimentPictureNumber = 0;
	}
	else
	{
		experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition);
	}
	if ((experimentPictureNumber == ePicturesPerRepetition - 1 || eRealTimePictures) && eDataExists)
	{
		myAndor::drawDataWindow();
	}
	return false;
}

bool DataAnalysisHandler::setAtomLocation(std::pair<int, int> location)
{
	atomLocations.push_back(location);
	return false;
}
std::vector<std::pair<int, int>> DataAnalysisHandler::getAtomLocations()
{
	return atomLocations;
}
bool DataAnalysisHandler::clearAtomLocations()
{
	atomLocations.clear();
	return false;
}

bool DataAnalysisHandler::combosAreEmpty()
{
	if (SendMessage(autoAnalysisTypeCombo.hwnd, CB_GETCURSEL, 0, 0) == -1 || SendMessage(dataOutputNameCombo.hwnd, CB_GETCURSEL, 0, 0) == -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}