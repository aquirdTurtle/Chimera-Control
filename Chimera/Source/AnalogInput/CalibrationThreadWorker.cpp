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
		std::vector<QBrush> origColors;
		/*for (auto col : range (calibrationTable->columnCount ())) {
			if (calibrationTable->item (count, col) != nullptr) {
				origColors.push_back (calibrationTable->item (count, col)->background ());
				calibrationTable->item (count, col)->setBackground (QColor (0, 0, 50));
			}
		}*/
		try {
			calibrate (cal, count);
		}
		catch (ChimeraError & e) {
			emit error (qstr (e.trace ()));
			// but continue to try the other ones. 
		}
		Sleep (200);
		//for (auto col : range (calibrationTable->columnCount ())) {
		//	calibrationTable->item (count, col)->setBackground (origColors[col]);
		//}
		count++;
		//qApp->processEvents ();
	}
	//refreshListview ();
	input.ttls->zeroBoard ();
	input.ao->zeroDacs (input.ttls->getCore (), { 0, input.ttls->getCurrentStatus () });
}

void CalibrationThreadWorker::calibrate (calSettings& cal, unsigned which) {
	if (!cal.active) {
		return;
	}
	emit startingNewCalibration (cal);
	emit notification (qstr ("Running Calibration " + cal.result.calibrationName + ".\n"), 1);
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
	Sleep (200); // give some time for the lasers to settle..
	cal.resultValues.clear ();
	unsigned aiNum = cal.aiInChan;
	unsigned aoNum = cal.aoControlChannel;
	for (auto calPoint : CalibrationManager::calPtTextToVals (cal.ctrlPtString)) {
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
		cal.resultValues.push_back (input.ai->getSingleChannelValue (aiNum, cal.avgNum));
		emit newCalibrationDataPoint (QPointF (calPoint, cal.resultValues.back ()));
		Sleep (20);
	}
	cal.currentlyCalibrating = false;
	std::ofstream file ("C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CalibrationValuesFile.txt");
	if (!file.is_open ()) {
		errBox ("Failed to Write Calibration Results!");
	}
	for (auto num : range (cal.resultValues.size ())) {
		file << CalibrationManager::calPtTextToVals (cal.ctrlPtString)[num] << " " << cal.resultValues[num] << "\n";
	}
	file.close ();
	cal.result.calibrationCoefficients = input.pythonHandler->runCalibrationFits (cal, input.parentWin);
	cal.result.includesSqrt = cal.includeSqrt;
	//calibrationTable->repaint ();
	cal.calibrated = true;
	if (cal.useAg) {
		auto& ag = input.agilents[int (cal.whichAg)].get ();
		ag.setCalibration (cal.result, cal.agChannel);
	}
	emit calibrationChanged ();
	emit finishedCalibration (cal);
}
