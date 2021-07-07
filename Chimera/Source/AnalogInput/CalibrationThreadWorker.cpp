#include <stdafx.h>
#include <AnalogInput/CalibrationManager.h>
#include <AnalogInput/CalibrationThreadWorker.h>
#include <AnalogInput/calInfo.h>
#include "AnalogInput/AiSystem.h"
#include <AnalogInput/CalibrationThreadWorker.h>
#include "AnalogOutput/AoSystem.h"
#include "DigitalOutput/DoSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/Version.h"
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <Agilent/AgilentCore.h>
#include <Python/NewPythonHandler.h>


CalibrationThreadWorker::CalibrationThreadWorker (CalibrationThreadInput input_) {
	input = input_;
}

CalibrationThreadWorker::~CalibrationThreadWorker () {
}

void CalibrationThreadWorker::runAll () {
	unsigned count = 0;
	// made this asynchronous to facilitate updating gui while 
	for (auto& cal : input.calibrations) {
		try {
			calibrate (cal, count);
		}
		catch (ChimeraError & err) {
			emit error (qstr (err.trace ()));
			// but continue to try the other ones. 
		}
		Sleep (200);
		count++;
	}
	try	{
		input.ttls->zeroBoard();
		input.ao->zeroDacs(input.ttls->getCore(), { 0, input.ttls->getCurrentStatus() });
	}
	catch (ChimeraError & err) {
		emit error(qstr(err.trace()));
		// but continue to try the other ones. 
	}
}

void CalibrationThreadWorker::calibrate (calSettings& cal, unsigned which) {
	if (!cal.active) {
		return;
	}
	auto& result = cal.result;
	emit startingNewCalibration (cal);
	emit notification({ qstr("Running Calibration " + result.calibrationName + ".\n"), 1, qstr("CALIBRATION_THREAD_WORKER")});
	cal.currentlyCalibrating = true;
	input.ttls->zeroBoard ();
	input.ao->zeroDacs (input.ttls->getCore (), { 0, input.ttls->getCurrentStatus () });
	for (auto dac : cal.aoConfig) {
		input.ao->setSingleDac (dac.first, dac.second, input.ttls->getCore (), { 0, input.ttls->getCurrentStatus () });
	}
	for (auto ttl : cal.ttlConfig) {
		auto& outputs = input.ttls->getDigitalOutputs ();
		outputs (ttl.second, ttl.first).check->setChecked (true);
		outputs (ttl.second, ttl.first).set (1);
		input.ttls->getCore ().ftdi_ForceOutput (ttl.first, ttl.second, 1, input.ttls->getCurrentStatus ());
	}
	Sleep (100); // give some time for the lasers to settle..
	result.resVals.clear ();
	unsigned aiNum = cal.aiInChan;
	unsigned aoNum = cal.aoControlChannel;
	result.ctrlVals = CalibrationManager::calPtTextToVals (cal.ctrlPtString);
	for (auto calPoint : result.ctrlVals) {
		try {
			if (cal.useAg) {
				auto& ag = input.agilents[int (cal.whichAg)].get ();
				dcInfo tempInfo;
				tempInfo.dcLevel = str (calPoint);
				tempInfo.dcLevel.internalEvaluate (std::vector<parameterType> (), 1);
				ag.setDC (cal.agChannel, tempInfo, 0);
			}
			else {
				input.ao->setSingleDac (aoNum, calPoint, input.ttls->getCore (), { 0, input.ttls->getCurrentStatus () });
			}
		}
		catch (ChimeraError & err) {
			emit error (err.qtrace ());
		}
		result.resVals.push_back (input.ai->getSingleChannelValue (aiNum, cal.avgNum));
		emit newCalibrationDataPoint (QPointF (calPoint, result.resVals.back ()));
		Sleep (20);
	}
	CalibrationManager::determineCalMinMax (cal.result);

	cal.currentlyCalibrating = false;
	std::ofstream file ("C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CalibrationValuesFile.txt");
	if (!file.is_open ()) {
		errBox ("Failed to Write Calibration Results!");
	}
	for (auto num : range (result.resVals.size ())) {
		file << result.ctrlVals[num] << " " << result.resVals[num] << "\n";
	}
	file.close ();
	result.calibrationCoefficients = input.pythonHandler->runCalibrationFits (cal, input.parentWin);
	result.includesSqrt = cal.includeSqrt;
	//calibrationTable->repaint ();
	cal.calibrated = true;
	if (cal.useAg) {
		auto& ag = input.agilents[int (cal.whichAg)].get ();
		ag.setAgCalibration (result, cal.agChannel);
	}
	emit calibrationChanged ();
	emit finishedCalibration (cal);
}
