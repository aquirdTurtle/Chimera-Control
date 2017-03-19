#include "stdafx.h"
#include "ExperimentLogger.h"
#include <iomanip>
//#include "MasterWindow.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include "VariableSystem.h"
#include <boost/algorithm/string/replace.hpp>


void ExperimentLogger::generateNiawgLog( experimentThreadInputStructure* input, niawgPair<std::vector<std::fstream>>& niawgScripts,
										 std::vector<std::fstream > &intensityScripts )
{
	/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///					Logging
	///
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
			if (niawgScriptLogs[axis].is_open() == false)
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

			for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
			{
				niawgScripts[axis][sequenceInc].clear();
				niawgScripts[axis][sequenceInc].seekg( 0, std::ios::beg );
				scriptText += "***Configuration [" + (*input).sequenceFileNames[sequenceInc]
					+ "] " + AXES_NAMES[axis] + " NIAWG Script***\n";
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
			if (andorConnected && !TWEEZER_COMPUTER_SAFEMODE)
			{
				niawgScriptLogs[axis] << scriptText;
			}
			if (input->debugInfo.outputNiawgHumanScript)
			{
				input->comm->sendDebug( scriptText );
			}
			andorConnected = true;
			niawgScriptLogs[axis].close();
		} while (andorConnected == false);
		andorConnected = false;
	}
	andorConnected = false;
	/// Log the intensity script
	do
	{
		std::ofstream intensityScriptLog( intensityLogPath );
		std::string intensityScriptText = "\n\n====================\nIntensity Script Being Used:\n====================\n";
		if (intensityScriptLog.is_open() == false)
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


		for (int sequenceInc = 0; sequenceInc < (*input).sequenceFileNames.size(); sequenceInc++)
		{
			intensityScripts[sequenceInc].clear();
			intensityScripts[sequenceInc].seekg( 0, std::ios::beg );
			intensityScriptText += "***Configuration [" + (*input).sequenceFileNames[sequenceInc] + "] Intensity NIAWG Script***\n";
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
		if (andorConnected && !TWEEZER_COMPUTER_SAFEMODE)
		{
			intensityScriptLog << intensityScriptText;
		}
		if (input->debugInfo.outputAgilentScript)
		{
			input->comm->sendDebug( intensityScriptText );
		}
		andorConnected = true;
	} while (andorConnected == false);

	andorConnected = false;
	/// Log other parameters
	do
	{
		std::ofstream parametersFileLog( parametersFileLogPath );
		if (parametersFileLog.is_open() == false)
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
			std::string paramtersString = "\nDont Actually Generate = " + std::to_string( input->dontActuallyGenerate )
				+ "\nConnect To Master = " + std::to_string( input->settings.connectToMaster )
				+ "\nGet Variable Files From Master = " + std::to_string( input->settings.getVariables )
				+ "\nRepetitions = " + std::to_string( input->repetitions )
				+ "\nDon't Actually Generate = " + std::to_string( input->dontActuallyGenerate )
				+ "\nProgramming Intensity = " + std::to_string( input->settings.programIntensity )
				+ "\nSequence File Names = \n";

			for (unsigned int seqInc = 0; seqInc < (*input).sequenceFileNames.size(); seqInc++)
			{
				paramtersString += "\t" + (*input).sequenceFileNames[seqInc] + "\n";
			}
			parametersFileLog << paramtersString;
		}
	} while (andorConnected == false);
}

/*
This is from the master computer.
void ExperimentLogger::generateLog(MasterWindow* master)
{
	this->logText.clear();
	// brief description.
	time_t timeObj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &timeObj);
	std::string timeString = std::to_string(currentTime.tm_year + 1900);
	timeString += "-" + std::to_string(currentTime.tm_mon + 1) + "-";
	timeString += std::to_string(currentTime.tm_mday) + ", " + std::to_string(currentTime.tm_hour);
	timeString += ":" + std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
	this->logText << "An experiment ran at (y-m-d h:m:s)" + timeString + " with the following parameters:\n";
	this->logText << "==========================================================================================\n";
	// log ttls
	// set width for table entries.
	logText << std::setw(10);
	logText << "TTL Settings:\n";
	// top left corner is blank.
	logText << "**VALUES**";
	for (int ttlNumberInc = 0; ttlNumberInc < master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
	{
		logText << std::setw(10) << ttlNumberInc;
	}
	logText << "\n";
	for (int ttlRowInc = 0; ttlRowInc < master->ttlBoard.getNumberOfTTLRows(); ttlRowInc++)
	{
		// first output row name:
		switch (ttlRowInc)
		{
			case 0:
				logText << std::setw(10) << "A:";
				break;
			case 1: 
				logText << std::setw(10) << "B:";
				break;
			case 2:
				logText << std::setw(10) << "C:";
				break;
			case 3:
				logText << std::setw(10) << "D:";
		}
		// now output all truth values.
		for (int ttlNumberInc = 0; ttlNumberInc < master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			if (master->ttlBoard.getTTL_Status(ttlRowInc, ttlNumberInc) == true)
			{
				logText << std::setw(10) << "ON";
			}
			else
			{
				logText << std::setw(10) << "OFF";
			}
		}
		logText << "\n";
	}
	logText << std::setw(10) << "**NAMES**";
	for (int ttlNumberInc = 0; ttlNumberInc < master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
	{
		logText << std::setw(10) << ttlNumberInc;
	}
	logText << "\n";
	for (int ttlRowInc = 0; ttlRowInc < master->ttlBoard.getNumberOfTTLRows(); ttlRowInc++)
	{
		// first output row name:
		switch (ttlRowInc)
		{
			case 0:
				logText << std::setw(10) << "A:";
				break;
			case 1:
				logText << std::setw(10) << "B:";
				break;
			case 2:
				logText << std::setw(10) << "C:";
				break;
			case 3:
				logText << std::setw(10) << "D:";
		}
		// now output all truth values.
		for (int ttlNumberInc = 0; ttlNumberInc < master->ttlBoard.getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			logText << std::setw(10) << master->ttlBoard.getName(ttlRowInc, ttlNumberInc);
		}
		logText << std::setw(10) << "\n";
	}


	// log dacs
	logText << "\n\nDAC Settings:\n";
	logText << std::setw(10) << "VALUES";
	for (int dacInc = 0; dacInc < master->dacBoards.getNumberOfDACs(); dacInc++)
	{
		if (dacInc % 8 == 0)
		{
			logText << "\n";
		}
		logText << std::setw(3) << std::to_string(dacInc+1) + ":" << std::setw(10) << std::setprecision(8) << std::left << std::to_string(master->dacBoards.getDAC_Value(dacInc));
	}
	logText << std::setw(10) << "\nNAMES";
	for (int dacInc = 0; dacInc < master->dacBoards.getNumberOfDACs(); dacInc++)
	{
		if (dacInc % 8 == 0)
		{
			logText << "\n";
		}
		logText << std::setw(3) << std::to_string(dacInc+1) + ":" << std::setw(10) << std::left << master->dacBoards.getName(dacInc);
	}
	// log the literal master script
	logText << "\n\nMaster Script:\n";
	logText << master->masterScript.getScriptText();
	// log the repetitions
	logText << "\n\nRepetitions: " << master->repetitionControl.getRepetitionNumber();
	logText << "\n\nVariables: ";
	std::vector<variable> varCopy = master->configVariables.getEverything();
	for (int varInc = 0; varInc < varCopy.size(); varInc++)
	{
		// output all variable information.
		logText << std::setw(10) << varCopy[varInc].name;
		if (varCopy[varInc].singleton)
		{
			logText << std::setw(10) << "Singleton";
			if (varCopy[varInc].timelike)
			{
				logText << std::setw(15) << "Timelike";
			}
			else
			{
				logText << std::setw(15) << "Not Timelike";
			}
			logText << std::setw(15) << "Value:" + std::to_string(varCopy[varInc].ranges[0].initialValue);
		}
		else
		{
			logText << std::setw(10) << "Variable";
			if (varCopy[varInc].timelike)
			{
				logText << std::setw(15) << "Timelike";
			}
			else
			{
				logText << std::setw(15) << "Not Timelike";
			}
			logText << std::setw(15) << "(init:fin:#)";
			for (int rangeInc = 0; rangeInc < varCopy[varInc].ranges.size(); rangeInc++)
			{
				logText << std::setw(8) << std::setprecision(8) << varCopy[varInc].ranges[rangeInc].initialValue << ":";
				logText << std::setw(8) << std::setprecision(8) << varCopy[varInc].ranges[rangeInc].finalValue << ":";
				logText << std::setw(8) << std::setprecision(8) << varCopy[varInc].ranges[rangeInc].variations << "\t";
			}
			logText << "\n";
		}		
	}
	// log the error status
	logText << "\n\nError Status:\n" << master->errorStatus.getText();
	// log the general status
	logText << "\n\nGeneral Status:\n" << master->generalStatus.getText();
	return;
}
*/

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
