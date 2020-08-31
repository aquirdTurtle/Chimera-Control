// created by Mark O. Brown
#include "stdafx.h"
// main header.
// conflict between qt slots and python slots. 
#include "Python/EmbeddedPythonHandler.h"
// for personInfo structure.
#include "Python/SMSTextingControl.h"

#ifdef _DEBUG
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif

// python constructor is important.
EmbeddedPythonHandler::EmbeddedPythonHandler( )
{
	if ( PYTHON_SAFEMODE ) {
		return;
	}
	Py_SetPythonHome( (wchar_t*)PYTHON_HOME );
	Py_Initialize( );
	std::string stdOutErr = 
		"import sys\n"
		"class CatchOutErr:\n"
			"\tdef __init__(self):\n"
			"\t\tself.value = ''\n"
			"\tdef write(self, txt):\n"
			"\t\tself.value += txt\n"
		"errCatcher = CatchOutErr()\n"
		"sys.stderr = errCatcher\n";
	// create main module
	mainModule = PyImport_AddModule( "__main__" );
	// invoke code to redirect
	PyRun_SimpleString( stdOutErr.c_str( ) );
	// get our errCatcher object (of type CatchOutErr) created above
	errorCatcher = PyObject_GetAttrString( mainModule, "errCatcher" );
	// start using the run function.
	try
	{
		run ( "import smtplib" );
	}
	catch ( ChimeraError& err )
	{
		errBox ( err.what ( ) );
	}
	try
	{
		run ( "from email.mime.text import MIMEText" );
	}
	catch ( ChimeraError& err )
	{
		errBox ( err.what ( ) );
	}
	try
	{
		run ( "sys.path.append('" + PYTHON_CODE_LOCATION + "')" );
	}
	catch ( ChimeraError& err )
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
	// are instead of taking inputs. So the code needs to writebtn the file and then let the python analysis use it
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
	thresholdInfoFile.close ( );
	auto res = system ( (str("python ") + DATA_ANALYSIS_CODE_LOCATION + "ThresholdAnalysis.py").c_str() );
	if ( res != 0 )
	{
		errBox ( "threshold analysis appears to have failed. system call result: " + str ( res ) );
	}
}


void EmbeddedPythonHandler::flush()
{
	if (PYTHON_SAFEMODE)
	{
		return;
	}
	// this resets the value of the class object, meaning that it resets the error text inside it.
	std::string flushMsg = "errCatcher.__init__()";
	run(flushMsg, true, false);
}


// for texting.
void EmbeddedPythonHandler::sendText(personInfo person, std::string msg, std::string subject, std::string baseEmail, 
									 std::string password){
	try	{
		flush ( );
		if ( baseEmail == "" || password == "" )		{
			thrower ( "ERROR: Please set an email and password for sending texts with!" );
		}
		run ( "email = MIMEText('" + msg + "', 'plain')" );
		run ( "email['Subject'] = '" + subject + "'" );
		run ( "email['From'] = '" + baseEmail + "'" );
		std::string recipient = person.number;
		if ( person.provider == "verizon" )	{
			// for verizon: [number]@vzwpix.com
			recipient += "@vzwpix.com";
		}
		else if ( person.provider == "tmobile" ){
			// for tmobile: [number]@tmomail.net
			recipient += "@tmomail.net";
		}
		else if ( person.provider == "at&t" ){
			recipient += "@txt.att.net";
		}
		else if ( person.provider == "googlefi" ){
			recipient += "@msg.fi.google.com";
		}

		run ( "email['To'] = '" + recipient + "'" );
		run ( "mail = smtplib.SMTP('smtp.gmail.com', 587)" );
		run ( "mail.ehlo()" );
		run ( "mail.starttls()" );
		run ( "mail.login('" + baseEmail + "', '" + password + "')" );
		run ( "mail.sendmail(email['From'], email['To'], email.as_string())" );
	}
	catch ( ChimeraError& err )	{
		errBox ( err.what ( ) );
	}
}

// for a single python command. Returns python's output of said command.
void EmbeddedPythonHandler::run(std::string cmd, bool quiet /*=false*/, bool flush /*=true*/){
	if (flush){
		this->flush();
	}
	if (PYTHON_SAFEMODE){
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
