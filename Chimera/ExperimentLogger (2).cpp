#include "stdafx.h"
#include "ExperimentLogger.h"
#include <iomanip>
#include "MasterWindow.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include "VariableSystem.h"

void ExperimentLogger::generateLog(MasterWindow* master)
{
	logText.clear();
	// brief description.
	time_t timeObj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &timeObj);
	std::string timeString = std::to_string(currentTime.tm_year + 1900);
	timeString += "-" + std::to_string(currentTime.tm_mon + 1) + "-";
	timeString += std::to_string(currentTime.tm_mday) + ", " + std::to_string(currentTime.tm_hour);
	timeString += ":" + std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
	logText << "An experiment ran at (y-m-d h:m:s)" + timeString + " with the following parameters:\n";
	logText << "==========================================================================================\n";
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
	for (int dacInc = 0; dacInc < master->dacBoards.getNumberOfDacs(); dacInc++)
	{
		if (dacInc % 8 == 0)
		{
			logText << "\n";
		}
		logText << std::setw(3) << std::to_string(dacInc+1) + ":" << std::setw(10) << std::setprecision(8) << std::left << std::to_string(master->dacBoards.getDacValue(dacInc));
	}
	logText << std::setw(10) << "\nNAMES";
	for (int dacInc = 0; dacInc < master->dacBoards.getNumberOfDacs(); dacInc++)
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
	// log the repetitionNumber
	logText << "\n\nRepetitions: " << master->repetitionControl.getRepetitionNumber();
	logText << "\n\nVariables: ";
	std::vector<variable> varCopy = master->configVariables.getEverything();
	for (int varInc = 0; varInc < varCopy.size(); varInc++)
	{
		// output all variable information.
		logText << std::setw(10) << varCopy[varInc].name;
		if (varCopy[varInc].constant)
		{
			logText << std::setw(10) << "Constant";
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
}

void ExperimentLogger::exportLog()
{
	// put this on the andor.
	std::fstream exportFile;
	exportFile.open(LOGGING_FILE_ADDRESS, std::ios::out);
	if (!exportFile.is_open())
	{
		thrower("ERROR: logging file failed to open!");
	}
	// export...
	exportFile << logText.str();

	exportFile.close();
}

std::string ExperimentLogger::getLog()
{
	return this->logText.str();
}
