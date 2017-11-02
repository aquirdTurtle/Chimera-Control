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

// constructor is important.
EmbeddedPythonHandler::EmbeddedPythonHandler( )
{
	if ( PYTHON_SAFEMODE )
	{
		return;
	}
	Py_SetPythonHome( PYTHON_HOME );
	Py_Initialize( );
	std::string stdOutErr = "import sys\n"
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
	ERR_POP( run( "import smtplib" ) );
	// for some reason the default qt based backed doesn't work
	ERR_POP( run( "from email.mime.text import MIMEText" ) );
	ERR_POP( run( "import sys" ) );
	ERR_POP( run( "sys.path.append('" + PYTHON_CODE_LOCATION + "')" ) )
	ERR_POP( run( "import plotDacs" ) );
	ERR_POP( run( "import plotTtls" ) );
}


void EmbeddedPythonHandler::flush()
{
	if (PYTHON_SAFEMODE)
	{
		return;
	}
	// this resets the value of the class object, meaning that it resets the error text inside it.
	std::string flushMsg = "catchOutErr.__init__()";
	run(flushMsg, false);
}


void EmbeddedPythonHandler::runPlotDacs( )
{
	ERR_POP( run( 
		"plotDacs.plotDacs( '" + PYTHON_INPUT_LOCATION + "DAC-Sequence.txt', time = True )"
	) );	
}

void EmbeddedPythonHandler::runPlotTtls( )
{
	ERR_POP(run(
		"plotTtls.plotTtls('" + PYTHON_INPUT_LOCATION + "TTL-Sequence.txt', time = True )"
		));
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
		thrower( "ERROR: Atom analysis function is null! The program can no longer call this function for some"
				 "reason. Auto-Analysis will not occur." );
	}
	if (!PyCallable_Check(this->atomAnalysisFunction))
	{
		thrower( "ERROR: Python is telling me that it cannot call the Atom analysis function. I don't know why"
				 ", since the function pointer is not null. Auto-Analysis will not occur." );
	}
		
	// I'm going to use comments before relevant commands to keep track of which python objects have references that I 
	// own, starting below (not counting the module and function references)

	// pythonFunctionArguments
	PyObject* pythonFunctionArguments = PyTuple_New(5);
	if (pythonFunctionArguments == NULL)
	{
		thrower("ERROR: creating tuple for python function arguments failed!?!?!?!? Auto-Analysis will terminate.");
	}
	// pythonFunctionArguments, pythonDate
	PyObject* pythonDate = Py_BuildValue("s", cstr(date));
	// check success
	if (pythonDate == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower("ERROR: Cannot Convert date! Auto-Analysis will terminate.");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 0, pythonDate);
	// pythonFunctionArguments, pythonRunNumber
	PyObject* pythonRunNumber = PyLong_FromLong(runNumber);
	if (pythonRunNumber == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower("Cannot Convert run number?!?!?!?!?! Auto-Analysis terminating...\r\n");
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
		thrower("Cannot Convert Pictures per experiment?!?!?!?!? Auto-Analysis terminating...");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 3, pythonPicturesPerExperiment);
	// pythonFunctionArguments, pythonAccumulations
	PyObject* pythonAccumulations = PyLong_FromLong(accumulations);
	if (pythonAccumulations == NULL)
	{
		Py_DECREF(pythonFunctionArguments);
		thrower("Cannot Convert Accumulations?!?!?!?!?!?!?!?!?!? Auto-Analysis terminating...");
	}
	// pythonFunctionArguments
	PyTuple_SetItem(pythonFunctionArguments, 4, pythonAccumulations);
	PyObject* pythonReturnValue = PyObject_CallObject(this->atomAnalysisFunction, pythonFunctionArguments);
	if (pythonReturnValue == NULL)
	{
		PyErr_Print();
		PyObject *output = PyObject_GetAttrString( errorCatcher, "value" );
		errBox( PyBytes_AS_STRING( PyUnicode_AsEncodedString( output, "ASCII", "strict" ) ) );
		thrower("Python function call returned NULL!");
	}
	Py_DECREF(pythonReturnValue);
	// finished successfully.
}


// for texting.
void EmbeddedPythonHandler::sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail, 
									 std::string password)
{
	flush();
	if (baseEmail == "" || password == "")
	{
		thrower("ERROR: Please set an email and password for sending texts with!");
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
	else if (person.provider == "at&t")
	{
		recipient += "@txt.att.net";
	}
	else if (person.provider == "googlefi")
	{
		recipient += "@msg.fi.google.com";
	}

	ERR_POP_RETURN(run("email['To'] = '" + recipient + "'"));
	ERR_POP_RETURN(run("mail = smtplib.SMTP('smtp.gmail.com', 587)"));
	ERR_POP_RETURN(run("mail.ehlo()"));
	ERR_POP_RETURN(run("mail.starttls()"));
	ERR_POP_RETURN(run("mail.login('" + baseEmail + "', '" + password + "')"));
	ERR_POP_RETURN(run("mail.sendmail(email['From'], email['To'], email.as_string())"));
}

// for a single python command. Returns python's output of said command.
std::string EmbeddedPythonHandler::run(std::string cmd, bool flush /*=true*/)
{
	if (flush)
	{
		this->flush();
	}
	if (PYTHON_SAFEMODE)
	{
		return "";
	}
	PyRun_SimpleString(cmd.c_str());
	// get the stdout and stderr from our catchOutErr object
	PyObject *output = PyObject_GetAttrString(errorCatcher, "value");
	std::string txt = PyBytes_AS_STRING( PyUnicode_AsEncodedString( output, "ASCII", "strict" ) );
	return txt;
}
