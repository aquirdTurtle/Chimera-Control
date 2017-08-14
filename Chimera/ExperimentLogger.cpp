#include "stdafx.h"
#include "ExperimentLogger.h"
#include <iomanip>
#include <ctime>
#include <iostream>
#include <fstream>
#include "VariableSystem.h"
#include <boost/algorithm/string/replace.hpp>

void ExperimentLogger::generateNiawgLog( MasterThreadInput* input, niawgPair<std::vector<std::fstream>>& niawgScripts,
										 std::vector<std::fstream > &intensityScripts, UINT repetitions )
{
	///					Logging
	// This report goes to a folder I create on the Andor. NEW: Always log.
	input->comm->sendStatus( "Logging Script and Experiment Parameters...\r\n" );
	niawgPair<std::string> niawgScriptLogPaths;
	niawgPair<std::ofstream> niawgScriptLogs;
	std::string intensityLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Intensity Script.txt";
	std::string parametersFileLogPath = EXPERIMENT_LOGGING_FILES_PATH + "\\Parameters.txt";

	bool andorConnected = false;
	/// log the niawg scritps
	for (auto axis : AXES)
	{
		/// Log the vertical script
		do
		{
			niawgScriptLogPaths[axis] = EXPERIMENT_LOGGING_FILES_PATH + "\\" + AXES_NAMES[axis] + " Script.txt";
			niawgScriptLogs[axis].open( niawgScriptLogPaths[axis] );
			std::string scriptText = "\n\n====================\n" + AXES_NAMES[axis] + " Script Being Used:\n====================\n";
			if (!niawgScriptLogs[axis].is_open() )
			{
				int andorDisconnectedOption = MessageBox( NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
														  "current script output sequence and will keep the default waveform running."
														  " (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
														  "without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE );
				switch (andorDisconnectedOption)
				{
					case IDABORT:
					{
						thrower( "ERROR: Andor is disconected and the user aborted.\r\n" );
					}
					case IDRETRY:
					{
						break;
					}
					case IDIGNORE:
					{
						// break out without writing file.
						andorConnected = true;
						break;
					}
				}
			}

			for (int sequenceInc = 0; sequenceInc < input->profile.sequenceConfigNames.size(); sequenceInc++)
			{
				niawgScripts[axis][sequenceInc].clear();
				niawgScripts[axis][sequenceInc].seekg( 0, std::ios::beg );
				scriptText += ("***Configuration [" + input->profile.sequenceConfigNames[sequenceInc]
								+ "] " + AXES_NAMES[axis] + " NIAWG Script***\n");
				std::stringstream tempStream;
				tempStream << niawgScripts[axis][sequenceInc].rdbuf();
				scriptText += tempStream.str();
				// reset the file so that it can be read again later.
				niawgScripts[axis][sequenceInc].clear();
				niawgScripts[axis][sequenceInc].seekg( 0, std::ios::beg );
			}
			scriptText += "\n";
			// make sure all line endings are \r\signal.
			boost::replace_all( scriptText, "\r", "" );
			boost::replace_all( scriptText, "\n", "\r\n" );
			if (andorConnected && !NIAWG_SAFEMODE)
			{
				niawgScriptLogs[axis] << scriptText;
			}
			if (input->debugOptions.outputNiawgHumanScript)
			{
				input->comm->sendDebug( scriptText );
			}
			andorConnected = true;
			niawgScriptLogs[axis].close();
		} while (!andorConnected);
		andorConnected = false;
	}
	andorConnected = false;
	/// Log the intensity script
	do
	{
		std::ofstream intensityScriptLog( intensityLogPath );
		std::string intensityScriptText = "\n\n====================\nIntensity Script Being Used:\n====================\n";
		if (!intensityScriptLog.is_open())
		{
			int andorDisconnectedOption = MessageBox( NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
													  "current script output sequence and will keep the default waveform running."
													  " (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
													  "without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE );
			switch (andorDisconnectedOption)
			{
				case IDABORT:
				{
					thrower( "ERROR: Andor Disconected. User Aborted.\r\n" );
				}
				case IDRETRY:
				{
					continue;
				}
				case IDIGNORE:
				{
					// break out without writing file.
					andorConnected = true;
					break;
				}
			}
		}


		for (int sequenceInc = 0; sequenceInc < input->profile.sequenceConfigNames.size(); sequenceInc++)
		{
			intensityScripts[sequenceInc].clear();
			intensityScripts[sequenceInc].seekg( 0, std::ios::beg );
			intensityScriptText += "***Configuration [" + input->profile.sequenceConfigNames[sequenceInc] + "] Intensity NIAWG Script***\n";
			std::stringstream tempStream;
			tempStream << intensityScripts[sequenceInc].rdbuf();
			intensityScriptText += tempStream.str();
			intensityScripts[sequenceInc].clear();
			intensityScripts[sequenceInc].seekg( 0, std::ios::beg );
		}
		intensityScriptText += "\n";
		// make sure all line endings are \r\n.
		boost::replace_all( intensityScriptText, "\r", "" );
		boost::replace_all( intensityScriptText, "\n", "\r\n" );
		if (andorConnected && !NIAWG_SAFEMODE)
		{
			intensityScriptLog << intensityScriptText;
		}
		if (input->debugOptions.outputAgilentScript)
		{
			input->comm->sendDebug( intensityScriptText );
		}
		andorConnected = true;
	} while (!andorConnected);

	andorConnected = false;
	/// Log other parameters
	do
	{
		std::ofstream parametersFileLog( parametersFileLogPath );
		if (!parametersFileLog.is_open())
		{
			int disconnectOption = MessageBox( NULL, "This computer can't currently open logging files on the andor.\nAbort will quit the "
											   "current script output sequence and will keep the default waveform running."
											   " (no output has started).\nRetry will re-attempt to connect to the Andor.\nIgnore will continue "
											   "without saving the current file.", "Andor Disconnected", MB_ABORTRETRYIGNORE );
			switch (disconnectOption)
			{
				case IDABORT:
				{
					thrower( "ERROR: Andor Disconected. User Aborted.\r\n" );
				}
				case IDRETRY:
				{
					break;
				}
				case IDIGNORE:
				{
					// break out without writing file.
					andorConnected = true;
					break;
				}
			}
		}
		else
		{
			andorConnected = true;
			// prepare the parameters list for the log
			std::string paramtersString = "\nDont Actually Generate = " + str( input->dontActuallyGenerate )
//				+ "\nConnect To Master = " + str( input->settings.connectToMaster )
//				+ "\nGet Variable Files From Master = " + str( input->settings.getVariables )
				+ "\nRepetitions = " + str( repetitions )
				+ "\nDon't Actually Generate = " + str( input->dontActuallyGenerate )
				+ "\nProgramming Intensity = " + str( input->programIntensity )
				+ "\nSequence File Names = \n";

			for (UINT seqInc = 0; seqInc < input->profile.sequenceConfigNames.size(); seqInc++)
			{
				paramtersString += "\t" + input->profile.sequenceConfigNames[seqInc] + "\n";
			}
			parametersFileLog << paramtersString;
		}
	} while (!andorConnected);
}


void ExperimentLogger::exportLog()
{
	// put this on the andor.
	std::fstream exportFile;
	exportFile.open( EXPERIMENT_LOGGING_FILES_PATH, std::ios::out);
	if (!exportFile.is_open())
	{
		thrower( "ERROR: logging file failed to open!" );
		return;
	}
	// export...
	exportFile << logText.str();
	exportFile.close();
}

std::string ExperimentLogger::getLog()
{
	return logText.str();
}
