// created by Mark O. Brown
#include "stdafx.h"
// main header.
#ifdef _DEBUG
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif
#include "EmbeddedPythonHandler.h"
// for personInfo structure.
#include "SMSTextingControl.h"
#include "Thrower.h"

// constructor is important.
EmbeddedPythonHandler::EmbeddedPythonHandler( )
{
	if ( PYTHON_SAFEMODE )
	{
		return;
	}
	Py_SetPythonHome( PYTHON_HOME );
	Py_Initialize( );
	std::string stdOutErr = 
		"import sys\n"
		"class CatchOutErr:\n"
			"\tdef __init__(self):\n"
			"\t\tself.value = ''\n"
			"\tdef write(self, txt):\n"
			"\t\tself.value += txt\n"
		"catchOutErr = CatchOutErr()\n"
		"sys.stderr = catchOutErr\n";
	//create main module
	mainModule = PyImport_AddModule( "__main__" );
	//invoke code to redirect
	PyRun_SimpleString( stdOutErr.c_str( ) );
	//get our catchOutErr object (of type CatchOutErr) created above
	errorCatcher = PyObject_GetAttrString( mainModule, "catchOutErr" );
	// start using the run function.
	try
	{
		run ( "import smtplib" );
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}
	try
	{
		run ( "from email.mime.text import MIMEText" );
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}
	try
	{
		run ( "sys.path.append('" + PYTHON_CODE_LOCATION + "')" );
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}

}


void EmbeddedPythonHandler::thresholdAnalysis (std::string dateString, int fid, std::string analysisLocsString, int picsPerRep )
{
	// this is a bit round-about at the moment. I had trouble getting the embedded python handler to properly import 
	// numpy and H5Py (seems related to precompiled libraries that numpy and H5Py use). So instead I'm brute-forcing 
	// it at the momnet. This probably isn't the fastest way to accomplish this, hopefully it's fast enough for this 
	// purpose. The analyis function reads a file to figure out what the settings of the current experiment 
	// are instead of taking inputs. So the code needs to write the file and then let the python analysis use it
	// to figure out how to analyze the recent data.

	std::ofstream thresholdInfoFile ( DATA_ANALYSIS_CODE_LOCATION + "ThresholdAnalysisInfo.txt", std::ios::out | std::ios::trunc );
	if ( !thresholdInfoFile.is_open ( ) )
	{
		thrower ( "Failed to open Threshold Analysis File!" );
	}
	thresholdInfoFile << "Date tuple (day, month, year)\nfileNumber\nAtom Locations\nPicsPerRep\n\n";
	thresholdInfoFile << dateString << "\n";
	thresholdInfoFile << fid << "\n";
	thresholdInfoFile << analysisLocsString << "\n";
	thresholdInfoFile << picsPerRep << "\n";

	auto res = system ( (str("python ") + DATA_ANALYSIS_CODE_LOCATION + "ThresholdAnalysis.py").c_str() );
	errBox ( "threshold analysis result: " + str(res) );
}


void EmbeddedPythonHandler::flush()
{
	if (PYTHON_SAFEMODE)
	{
		return;
	}
	// this resets the value of the class object, meaning that it resets the error text inside it.
	std::string flushMsg = "catchOutErr.__init__()";
	run(flushMsg, true, false);
}


void EmbeddedPythonHandler::runDataAnalysis( std::string date, long runNumber, long accumulations, 
											 std::vector<coordinate> atomLocations)
{
	/* I don't use this anymore, but keeping it around in case I want to do anything like this again. */
	// for full data analysis set.
	flush();
	if (autoAnalysisModule == NULL)
	{
		errBox("autoAnalysisModule is no longer available! This shouldn't happen... Continuing...");
	}
	// interpret the text here to get the actual function name.
	if (atomAnalysisFunction == NULL)
	{
		thrower ( "ERROR: Atom analysis function is null! The program can no longer call this function for some"
				 "reason. Auto-Analysis will not occur." );
	}
	if (!PyCallable_Check(this->atomAnalysisFunction))
	{
		thrower ( "ERROR: Python is telling me that it cannot call the Atom analysis function. I don't know why"
				 ", since the function pointer is not null. Auto-Analysis will not occur." );
	}
		
	// I'm going to use comments before relevant commands to keep track of which python objects have references that I 
	// own, starting below (not counting the module and function references)

	// pythonFunctionArguments
	PyObject* pythonFunctionArguments = PyTuple_New(5);
	if (pythonFunctionArguments == NULL)
	{
		thrower ("ERROR: creating tuple for python function arguments failed!?!?!?!? Auto-Analysis will terminate.");
	}
	// pythonFunctionArguments, pythonDate
	PyObject* pythonDate = Py_BuildValue("s", cstr(date));
	// check success
	if (pythonDate == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower ("ERROR: Cannot Convert date! Auto-Analysis will terminate.");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);
	// pythonFunctionArguments, pythonRunNumber
	PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
	if (pythonRunNumber == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower ("Cannot Convert run number?!?!?!?!?! Auto-Analysis terminating...\r\n");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 1, pythonRunNumber);

	// create the numpy array of atom locations. this is a 1D array, the other code assumes two numbers per atom, and 
	// figures out based on the function call whether it should look at pairs of atoms or not. If pairs, it assumes 
	// 1a, 1b, 2a, 2b, etc. formatting.
	// pythonFunctionArguments, pythonAtomLocationsArray
	PyObject* pythonAtomLocationsArray = PyTuple_New(atomLocations.size() * 2);
	for (UINT atomInc = 0; atomInc < atomLocations.size(); atomInc++)
	{
		// order is flipped. Dunno why...
		// PyTuple immediately steals the reference from PyLong_FromLong, so I don't need to handle any of these. 
		PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc, PyLong_FromLong(atomLocations[atomInc].column));
		PyTuple_SetItem(pythonAtomLocationsArray, 2 * atomInc + 1, PyLong_FromLong(atomLocations[atomInc].row));
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 2, pythonAtomLocationsArray);

	// format of function arguments:
	// def analyzeSingleLocation(date, runNumber, atomLocationRow, atomLocationColumn, picturesPerExperiment, accumulations, fileName) :
	// hard-coded for now to 2. (might change or remove later...)
	// pythonFunctionArguments, pythonPicturesPerExperiment
	PyObject* pythonPicturesPerExperiment = PyLong_FromLong(2/*ePicturesPerRepetition*/);
	if (pythonPicturesPerExperiment == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower ("Cannot Convert Pictures per experiment?!?!?!?!? Auto-Analysis terminating...");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);
	// pythonFunctionArguments, pythonAccumulations
	PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
	if (pythonAccumulations == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower ("Cannot Convert Accumulations?!?!?!?!?!?!?!?!?!? Auto-Analysis terminating...");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);
	PyObject* pythonReturnValue = PyObject_CallObject(this->atomAnalysisFunction, pythonFunctionArguments);
	if (pythonReturnValue == NULL)
	{
		PyErr_Print();
		PyObject *output = PyObject_GetAttrString( errorCatcher, "value" );
		errBox( PyBytes_AS_STRING( PyUnicode_AsEncodedString( output, "ASCII", "strict" ) ) );
		thrower ("Python function call returned NULL!");
	}
	Py_DECREF(pythonReturnValue);
	// finished successfully.
}


// for texting.
void EmbeddedPythonHandler::sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail, 
									 std::string password)
{
	try
	{
		flush ( );
		if ( baseEmail == "" || password == "" )
		{
			thrower ( "ERROR: Please set an email and password for sending texts with!" );
		}
		run ( "email = MIMEText('" + msg + "', 'plain')" );
		run ( "email['Subject'] = '" + subject + "'" );
		run ( "email['From'] = '" + baseEmail + "'" );
		std::string recipient = person.number;
		if ( person.provider == "verizon" )
		{
			// for verizon: [number]@vzwpix.com
			recipient += "@vzwpix.com";
		}
		else if ( person.provider == "tmobile" )
		{
			// for tmobile: [number]@tmomail.net
			recipient += "@tmomail.net";
		}
		else if ( person.provider == "at&t" )
		{
			recipient += "@txt.att.net";
		}
		else if ( person.provider == "googlefi" )
		{
			recipient += "@msg.fi.google.com";
		}

		run ( "email['To'] = '" + recipient + "'" );
		run ( "mail = smtplib.SMTP('smtp.gmail.com', 587)" );
		run ( "mail.ehlo()" );
		run ( "mail.starttls()" );
		run ( "mail.login('" + baseEmail + "', '" + password + "')" );
		run ( "mail.sendmail(email['From'], email['To'], email.as_string())" );
	}
	catch ( Error& err )
	{
		errBox ( err.what ( ) );
	}
}


// for a single python command. Returns python's output of said command.
void EmbeddedPythonHandler::run(std::string cmd, bool quiet /*=false*/, bool flush /*=true*/)
{
	if (flush)
	{
		this->flush();
	}
	if (PYTHON_SAFEMODE)
	{
		return;
	}
	PyRun_SimpleString(cmd.c_str());
	// get the stdout and stderr from our catchOutErr object
	PyObject *output = PyObject_GetAttrString(errorCatcher, "value");
	if ( PyUnicode_Check ( output ) )
	{
		//auto res1 = PyUnicode_FromString ( output );
		auto res = PyUnicode_AsEncodedString ( output, "ASCII", "ignore" );
		std::string txt = PyBytes_AS_STRING ( res );
		if ( !quiet && txt != "" )
		{
			std::string runMsg = "Error seen while running python command \"" + cmd + "\"\n\n";
			thrower ( runMsg + txt );
		}
	}
}



