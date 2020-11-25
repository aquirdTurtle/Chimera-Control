// created by Mark O. Brown
#include "stdafx.h"
#include "CalibrationManager.h"
#include "boost/lexical_cast.hpp"
#include "PhotodetectorCalibration.h"
#include <QHeaderView>
#include <QMenu>
#include "GeneralObjects/ChimeraStyleSheets.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <qapplication.h>

CalibrationManager::CalibrationManager (IChimeraQtWindow* parent) : IChimeraSystem (parent), 
calibrationViewer(1,plotStyle::GeneralErrorPlot,std::vector<int>(),"Calibration View",false,false) {}


void CalibrationManager::initialize (QPoint& pos, IChimeraQtWindow* parent, AiSystem* ai_in, AoSystem* ao_in,
									 DoSystem* ttls_in, std::vector<std::reference_wrapper<AgilentCore>> agilents_in,
									 NewPythonHandler* python_in) {
	auto& px = pos.rx (), & py = pos.ry ();
	px += 480;
	py -= 300;
	calibrationViewer.init (pos, 480, 300, parent);
	px += -480;
	calsHeader = new QLabel ("CALIBRATION MANAGER", parent);
	calsHeader->setGeometry (px, py, 280, 20);
	calibrateAllButton = new CQPushButton ("Calibrate All", parent);
	calibrateAllButton->setGeometry (px + 280, py, 175, 20);
	calibrateAllButton->setToolTip ("Force the recalibration.");
	parent->connect (calibrateAllButton, &QPushButton::released, [this, parent]() {
		if (!parent->mainWin->expIsRunning ()) {
			runAllThreaded ();
		}});

	expAutoCalButton = new CQCheckBox ("Exp. Auto-Cal?", parent);
	expAutoCalButton->setGeometry (px + 380 + 175, py, 175, 20);
	expAutoCalButton->setToolTip ("Automatically calibrate all calibrations before doing any experiment?");

	cancelCalButton = new QPushButton ("Cancel Calibration?", parent);
	cancelCalButton->setGeometry (px + 730, py, 150, 20);
	cancelCalButton->setToolTip ("Hold this button down to cancel a \"Run All\" Calibration.");
	py += 20;
	calibrationTable = new QTableWidget (parent);
	QStringList labels;
	labels << " Name " << " Ctrl Pts (V) " << " Res (V) " << " Ai " << " Ao " << "Agilent" << "Ag. Channel" 
		<< " DO-Config " << " AO-Config " << " Avgs " << "Cal. Vals" << "Include Sqrt" << "Polynomial Order";
	calibrationTable->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (calibrationTable, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {handleContextMenu (pos); });
	calibrationTable->setColumnCount (labels.size ());
	calibrationTable->setHorizontalHeaderLabels (labels);
	calibrationTable->horizontalHeader ()->setFixedHeight (25);
	calibrationTable->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
	calibrationTable->setGeometry (px, py, 960, 200);
	py += 200;
	calibrationTable->setToolTip ("");
	calibrationTable->verticalHeader ()->setSectionResizeMode (QHeaderView::Fixed);
	calibrationTable->verticalHeader ()->setDefaultSectionSize (25);
	calibrationTable->verticalHeader ()->setFixedWidth (50);
	calibrationTable->connect (calibrationTable, &QTableWidget::cellDoubleClicked, [this](int clRow, int clCol) {
		if (clCol == 10) {
			auto* item = new QTableWidgetItem (calibrationTable->item (clRow, clCol)->text () == "No" ? "Yes" : "No");
			item->setFlags (item->flags () & ~Qt::ItemIsEditable);
			calibrationTable->setItem (clRow, clCol, item);
		}
		else if (clCol == 11) {
			calibrations[clRow].includeSqrt = !calibrations[clRow].includeSqrt;
			auto* item = new QTableWidgetItem (calibrations[clRow].includeSqrt ? "Yes" : "No");
			item->setFlags (item->flags () & ~Qt::ItemIsEditable);
			calibrationTable->setItem (clRow, clCol, item);
		}});
	calibrationTable->connect (calibrationTable, &QTableWidget::currentCellChanged,
		[this](int row, int col) {if (unsigned(row) < calibrations.size() && calibrations[row].calibrated) {
		updateCalibrationView (calibrations[row]);
	} });
	calibrationTable->connect (
		calibrationTable, &QTableWidget::cellChanged, [this](int row, int col) {
			if (calibrations.size () <= row) {
				return;
			}
			auto& cal = calibrations[row];
			auto qtxt = calibrationTable->item (row, col)->text ();
			switch (col) {
				case 0:
					cal.result.calibrationName = str (qtxt);
					break;
				case 1: {
					cal.ctrlPtString = qtxt;
					break;
				}
				case 3:
					cal.aiInChan = boost::lexical_cast<unsigned>(str (qtxt));
					break;
				case 4:
					try {
						cal.aoControlChannel = boost::lexical_cast<unsigned>(str (qtxt));
					}
					catch (boost::bad_lexical_cast &) {
						errBox ("Error Trying to set analog output channel!");
					}
					break;
				case 5: {
					if (qtxt == "") {
						break;
					}
					try {
						cal.whichAg = AgilentEnum::fromStr (str (qtxt));
					}
					catch (ChimeraError &) {
						emit error ("Error In trying to read the ao info string!");
					}
					break;
				}
				case 6: {
					if (qtxt == "") {
						break;
					}
					try {
						cal.agChannel = boost::lexical_cast<unsigned> (str (qtxt));
					}
					catch (ChimeraError &) {
						emit error ("Error trying to set agilent channel!");
					}
					break;
				}
				case 7: {
					std::stringstream tmpStream (cstr (qtxt));
					std::string rowTxt;
					cal.ttlConfig.clear ();
					while (tmpStream >> rowTxt) {
						try {
							std::pair<DoRows::which, unsigned> ttl;
							ttl.first = DoRows::fromStr (rowTxt);
							tmpStream >> ttl.second;
							cal.ttlConfig.push_back (ttl);
						}
						catch (ChimeraError&) {
							errBox ("Error In trying to set the calibration ttl config!");
						}
					}
					break;
				}
				case 8: {
					std::stringstream tmpStream (cstr (qtxt));
					std::string dacIdTxt;
					cal.aoConfig.clear ();
					while (tmpStream >> dacIdTxt) {
						try {
							auto id = AoSystem::getBasicDacIdentifier (dacIdTxt);
							if (id == -1) {
								thrower ("Dac Identifier \"" + dacIdTxt + "\" failed to convert to a basic dac id!");
							}
							std::pair<unsigned, double> dacSetting;
							dacSetting.first = id;
							tmpStream >> dacSetting.second;
							cal.aoConfig.push_back (dacSetting);
						}
						catch (ChimeraError&) {
							errBox ("Error In trying to set the calibration dac config!");
						}
					}
					break;
				}
				case 12: {
					try {
						cal.polynomialOrder = boost::lexical_cast<unsigned>(str(qtxt));
					}
					catch (ChimeraError &) {
						errBox ("Error In trying to set the fit polynomial order!");
					}
					break;
				}
			}
		}
	);
	calibrationTable->resizeColumnsToContents ();
	ai = ai_in;
	ao = ao_in;
	agilents = agilents_in;
	ttls = ttls_in;
	pythonHandler = python_in;
}

void CalibrationManager::handleContextMenu (const QPoint& pos) {
	auto* newcal = new QAction ("New Item", calibrationTable);
	calibrationTable->connect (newcal, &QAction::triggered, [this]() {calibrations.push_back (calSettings ()); refreshListview (); });
	QTableWidgetItem* item = calibrationTable->itemAt (pos);
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	menu.addAction (newcal);
	if (!item) {
		menu.exec (calibrationTable->mapToGlobal (pos));
		return;
	}
	if (!calibrations[item->row ()].useAg) {
		auto* useag = new QAction ("Use Agilent", calibrationTable);
		calibrationTable->connect (useag, &QAction::triggered,
			[this, item]() {
				calibrations[item->row ()].useAg = true;
				refreshListview ();
			});
		menu.addAction (useag);
	}
	else {
		auto* useao = new QAction ("Use AO System", calibrationTable);
		calibrationTable->connect (useao, &QAction::triggered,
			[this, item]() {calibrations[item->row ()].useAg = false; refreshListview (); });
		menu.addAction (useao);
	}
	if (calibrations[item->row ()].active) {
		auto* deactivate = new QAction ("Deactivate", calibrationTable);
		calibrationTable->connect (deactivate, &QAction::triggered,
			[this, item]() {calibrations[item->row ()].active = false; refreshListview (); });
		menu.addAction (deactivate);
	}
	else {
		auto* activate = new QAction ("Activate", calibrationTable);
		calibrationTable->connect (activate, &QAction::triggered,
			[this, item]() {calibrations[item->row ()].active = true; refreshListview (); });
		menu.addAction (activate);
	}
	auto* deleteAction = new QAction ("Delete This Item", calibrationTable);
	calibrationTable->connect (deleteAction, &QAction::triggered,
		[this, item]() {calibrations.erase (calibrations.begin () + item->row ()); refreshListview (); });
	auto* calibrateThis = new QAction ("Calibrate This", calibrationTable);
	calibrationTable->connect (calibrateThis, &QAction::triggered, [this, item]() {
		try {
			calibrateThreaded (calibrations[item->row ()], item->row ());
			refreshListview ();
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
		}; });
	menu.addAction (deleteAction);
	menu.addAction (newcal);
	menu.addAction (calibrateThis);
	menu.exec (calibrationTable->mapToGlobal (pos));
}

std::vector<calSettings> CalibrationManager::getCalibrationInfo (){
	return calibrations;
}

void CalibrationManager::handleSaveMasterConfig (std::stringstream& configStream) {
	configStream << "\n/*CALIBRATION_MANAGER*/" 
		<< "\n/*Auto Cal Checked: */ " << expAutoCalButton->isChecked ()
		<< "\n/*Calibration Number: */ " << calibrations.size ();
	for (auto& cal : calibrations) {
		handleSaveMasterConfigIndvCal (configStream, cal);
	}
}

void CalibrationManager::handleSaveMasterConfigIndvCal(std::stringstream& configStream, calSettings& cal) {
	configStream << "\n/*Calibration Name: */ " << cal.result.calibrationName
		<< "\n/*Analog Input Chanel: */ " << cal.aiInChan
		<< "\n/*Analog Output Control Chanel: */ " << cal.aoControlChannel
		<< "\n/*Control Values: */ " << str(cal.ctrlPtString)
		<< "\n/*Calibration Active: */ " << cal.active
		<< "\n/*TTL Config Size: */ " << cal.ttlConfig.size () 
		<< "\n/*ttl config: */ " << calTtlConfigToString (cal.ttlConfig)
		<< "\n/*Analog Output Config Size: */ " << cal.aoConfig.size () << "\n/*Analog Output Config: */ ";
	for (auto& dac : cal.aoConfig) {
		configStream << dac.first << " " << dac.second << " ";
	}
	configStream << "\n/*Data Point Average Number: */ " << cal.avgNum
		<< "\n/*Use Agilent: */" << cal.useAg
		<< "\n/*Which Agilent: */" << AgilentEnum::toStr (cal.whichAg)
		<< "\n/*Which Agilent Channel: */" << cal.agChannel
		<< "\n/*Include Sqrt on Next Cal: */" << cal.includeSqrt
		<< "\n/*Number of Calibration Coefficients: */ " << cal.result.calibrationCoefficients.size ()
		<< "\n/*Calibration Coefficients: */ " << dblVecToString (cal.result.calibrationCoefficients)
		<< "\n/*Calibration Includes Sqrt: */ " << cal.result.includesSqrt
		<< "\n/*Polynomial Order: */ " << cal.polynomialOrder << "\n";
}

calSettings CalibrationManager::handleOpenMasterConfigIndvCal (ConfigStream& configStream) {
	calSettings tmpInfo;
	try {
		configStream >> tmpInfo.result.calibrationName;
		configStream >> tmpInfo.aiInChan >> tmpInfo.aoControlChannel;
		tmpInfo.ctrlPtString = qstr (configStream.getline ());
		configStream >> tmpInfo.active;
		unsigned numSettings;
		configStream >> numSettings;
		tmpInfo.ttlConfig.resize (numSettings);
		for (auto& ttl : tmpInfo.ttlConfig) {
			std::string rowStr;
			configStream >> rowStr >> ttl.second;
			ttl.first = DoRows::fromStr (rowStr);
		}
		configStream >> numSettings;
		tmpInfo.aoConfig.resize (numSettings);
		for (auto& ao : tmpInfo.aoConfig) {
			unsigned dacID;
			configStream >> dacID >> ao.second;
			ao.first = dacID;
		}
		configStream >> tmpInfo.avgNum;
		if (configStream.ver > Version ("2.10")) {
			std::string whichAgString;
			configStream >> tmpInfo.useAg >> whichAgString >> tmpInfo.agChannel;
			tmpInfo.whichAg = AgilentEnum::fromStr (whichAgString);
		}
		if (configStream.ver > Version ("2.11")) {
			unsigned numCoef;
			configStream >> tmpInfo.includeSqrt >> numCoef;
			if (numCoef > 50) {
				// catch weird bad values...
				thrower ("Suspicious Number of coefficients! Number was " + str (numCoef));
			}
			tmpInfo.result.calibrationCoefficients.resize (numCoef);
			for (auto& coef : tmpInfo.result.calibrationCoefficients) {
				configStream >> coef;
			}
			configStream >> tmpInfo.result.includesSqrt;
			configStream >> tmpInfo.polynomialOrder;
		}
	}
	catch (ChimeraError&) {
		throwNested ("Failed to load Calibration named " + tmpInfo.result.calibrationName + "!");
	}
	return tmpInfo;
}

void CalibrationManager::handleOpenMasterConfig (ConfigStream& configStream) {
	if (configStream.ver < Version ("2.10")) {
		// this was before the calibration manager.
		return;
	}
	bool expAutocal;
	configStream >> expAutocal;
	expAutoCalButton->setChecked (expAutocal);
	long numcalibrationsInFile;
	configStream >> numcalibrationsInFile;
	calibrations.clear ();
	for (auto calNum : range (numcalibrationsInFile)) {
		calibrations.push_back (handleOpenMasterConfigIndvCal (configStream));
	}
	for (auto& cal : calibrations) {
		if (cal.useAg) {
			auto& ag = agilents[(int)cal.whichAg].get ();
			ag.setCalibration (cal.result, cal.agChannel);
		}
	}
	refreshListview ();
}

void CalibrationManager::refreshListview () {
	calibrationTable->setRowCount (0);
	for (auto& cal : calibrations) {
		addCalToListview (cal);
	}
	//calibrationTable->resizeColumnsToContents ();
}

void CalibrationManager::addCalToListview (calSettings& cal) {
	int row = calibrationTable->rowCount ();
	int precision = 5;
	QColor textColor;
	textColor = cal.calibrated ? QColor (255, 255, 255) : QColor (255, 0, 0);
	auto setItemExtra = [row, this, cal, textColor](int item) {
		calibrationTable->item (row, item)->setFlags (!cal.active ? calibrationTable->item (row, item)->flags () & ~Qt::ItemIsEnabled
			: calibrationTable->item (row, item)->flags () | Qt::ItemIsEnabled);
		calibrationTable->item (row, item)->setForeground (textColor);
	};
	calibrationTable->insertRow (row);
	calibrationTable->setItem (row, 0, new QTableWidgetItem (cal.result.calibrationName.c_str ()));
	setItemExtra (0);
	calibrationTable->setItem (row, 1, new QTableWidgetItem (cal.ctrlPtString));
	setItemExtra (1);
	calibrationTable->setItem (row, 2, new QTableWidgetItem (qstr (dblVecToString (cal.resultValues), precision)));
	calibrationTable->item (row, 2)->setFlags (calibrationTable->item (row, 2)->flags () ^ Qt::ItemIsEnabled);
	calibrationTable->setItem (row, 3, new QTableWidgetItem (cstr (cal.aiInChan, precision)));
	setItemExtra (3);
	
	calibrationTable->setItem (row, 4, new QTableWidgetItem (qstr (cal.aoControlChannel)));
	//setItemExtra (4);
	calibrationTable->item (row, 4)->setFlags (cal.useAg || !cal.active ?
		calibrationTable->item (row, 4)->flags () & ~Qt::ItemIsEnabled
		: calibrationTable->item (row, 4)->flags () | Qt::ItemIsEnabled);
	
	calibrationTable->setItem (row, 5, new QTableWidgetItem (qstr(AgilentEnum::toStr(cal.whichAg))));
	//setItemExtra (5);
	calibrationTable->item (row, 5)->setFlags (!cal.useAg || !cal.active ?
		calibrationTable->item (row, 5)->flags () & ~Qt::ItemIsEnabled
		: calibrationTable->item (row, 5)->flags () | Qt::ItemIsEnabled);

	calibrationTable->setItem (row, 6, new QTableWidgetItem (qstr (cal.agChannel)));
	//setItemExtra (6);
	calibrationTable->item (row, 6)->setFlags (!cal.useAg || !cal.active ?
		calibrationTable->item (row, 6)->flags () & ~Qt::ItemIsEnabled
		: calibrationTable->item (row, 6)->flags () | Qt::ItemIsEnabled);
	calibrationTable->setItem (row, 7, new QTableWidgetItem (calTtlConfigToString (cal.ttlConfig).c_str ()));
	setItemExtra (7);
	calibrationTable->setItem (row, 8, new QTableWidgetItem (calDacConfigToString (cal.aoConfig).c_str ()));
	setItemExtra (8);
	calibrationTable->setItem (row, 9, new QTableWidgetItem (qstr (cal.avgNum, precision)));
	setItemExtra (9);
	calibrationTable->setItem (row, 10, new QTableWidgetItem (qstr(dblVecToString(cal.result.calibrationCoefficients))));
	setItemExtra (10);
	calibrationTable->item (row, 10)->setFlags (calibrationTable->item (row, 10)->flags () ^ Qt::ItemIsEnabled);
	calibrationTable->item (row, 10)->setToolTip (qstr (dblVecToString (cal.result.calibrationCoefficients)));
	calibrationTable->setItem (row, 11, new QTableWidgetItem (cal.includeSqrt ? "True" : "False"));
	setItemExtra (11);
	calibrationTable->setItem (row, 12, new QTableWidgetItem (qstr(cal.polynomialOrder)));
	setItemExtra (12);
}

std::string CalibrationManager::calDacConfigToString (std::vector<std::pair<unsigned, double>> aoConfig) {
	std::string aoString;
	for (auto ao : aoConfig) {
		aoString += "dac" + str (ao.first) + " " + str (ao.second, 4) + " ";
	}
	return aoString;
}

std::string CalibrationManager::calTtlConfigToString (std::vector<std::pair<DoRows::which, unsigned> > ttlConfig) {
	std::string digitalOutConfigString;
	for (auto val : ttlConfig) {
		digitalOutConfigString += DoRows::toStr (val.first) + " " + str (val.second) + " ";
	}
	return digitalOutConfigString;
}

std::string CalibrationManager::dblVecToString (std::vector<double> ctrls) {
	std::string ctrlString;
	for (auto val : ctrls) {
		ctrlString += str(val,4,true) +" ";
	}
	return ctrlString;
}


bool CalibrationManager::wantsExpAutoCal () {
	return expAutoCalButton->isChecked ();
}

void CalibrationManager::runAllThreaded () {
	emit notification ("Running All Calibrations.\n");
	std::vector<std::reference_wrapper<calSettings>> calInput;
	for (auto& cal : calibrations) {
		calInput.push_back (cal);
	}
	standardStartThread (calInput);
}

void CalibrationManager::standardStartThread (std::vector<std::reference_wrapper<calSettings>> calsToRun) {
	CalibrationThreadInput input;
	input.calibrations = calsToRun;
	input.agilents = agilents;
	input.ttls = ttls;
	input.ai = ai;
	input.ao = ao;
	input.calibrationViewer = &calibrationViewer;
	input.parentWin = parentWin;
	input.pythonHandler = pythonHandler;

	threadWorker = new CalibrationThreadWorker (input);
	thread = new QThread;

	threadWorker->moveToThread (thread);
	connect (threadWorker, &CalibrationThreadWorker::notification, this, &IChimeraSystem::notification);
	connect (threadWorker, &CalibrationThreadWorker::warn, this, &IChimeraSystem::error);
	connect (threadWorker, &CalibrationThreadWorker::calibrationChanged, this, [this]() { refreshListview (); });
	connect (threadWorker, &CalibrationThreadWorker::startingNewCalibration, this, [this](calSettings cal) {
			calibrationViewer.removeData ();
			calibrationViewer.initializeCalData (cal);

		});
	connect (threadWorker, &CalibrationThreadWorker::newCalibrationDataPoint, this, [this](QPointF pt) {
			auto chartData = calibrationViewer.getCalData ();
			*chartData << pt;
		});
	connect (threadWorker, &CalibrationThreadWorker::finishedCalibration, this, [this](calSettings cal) {
			updateCalibrationView (cal);
		});

	connect (thread, &QThread::started, threadWorker, &CalibrationThreadWorker::runAll);
	connect (thread, &QThread::finished, thread, &QObject::deleteLater);
	connect (thread, &QThread::finished, threadWorker, &QObject::deleteLater);
	thread->start ();
}

//void CalibrationManager::runAll () {
//	unsigned count = 0;
//	// made this asynchronous to facilitate updating gui while 
//	for (auto& cal : calibrations) {
//		if (cancelCalButton->isDown ()) {
//			break;
//		}
//		std::vector<QBrush> origColors;
//		for (auto col : range (calibrationTable->columnCount ())) {
//			if (calibrationTable->item (count, col) != nullptr) {
//				origColors.push_back (calibrationTable->item (count, col)->background ());
//				calibrationTable->item (count, col)->setBackground (QColor (0, 0, 50));
//			}
//		}
//		try {
//			CalibrationManager::calibrate (cal, count);
//		}
//		catch (ChimeraError & e) {
//			emit error (qstr (e.trace ()));
//			// but continue to try the other ones. 
//		}
//		for (auto col : range (calibrationTable->columnCount ())) {
//			calibrationTable->item (count, col)->setBackground (origColors[col]);
//		}
//		count++;
//		qApp->processEvents ();
//	}
//	refreshListview ();
//	ttls->zeroBoard ();
//	ao->zeroDacs (ttls->getCore (), { 0, ttls->getCurrentStatus () });
//}

void CalibrationManager::calibrateThreaded (calSettings& cal, unsigned which) {
	std::vector<std::reference_wrapper<calSettings>> calInput;
	calInput.push_back (cal);
	standardStartThread (calInput);
	//if (!cal.active) {
	//	return;
	//}
	//emit notification (qstr ("Running Calibration " + cal.result.calibrationName + ".\n"), 1);
	//cal.currentlyCalibrating = true;
	//ttls->zeroBoard ();
	//ao->zeroDacs (ttls->getCore (), { 0, ttls->getCurrentStatus () });
	//for (auto dac : cal.aoConfig) {
	//	ao->setSingleDac (dac.first, dac.second, ttls->getCore (), { 0, ttls->getCurrentStatus () });
	//}
	//for (auto ttl : cal.ttlConfig) {
	//	auto& outputs = ttls->getDigitalOutputs ();
	//	outputs (ttl.second, ttl.first).check->setChecked (true);
	//	outputs (ttl.second, ttl.first).set (1);
	//	ttls->getCore ().ftdi_ForceOutput (ttl.first, ttl.second, 1, ttls->getCurrentStatus ());
	//}
	//Sleep (200); // give some time for the lasers to settle..
	//cal.resultValues.clear ();
	//unsigned aiNum = cal.aiInChan;
	//unsigned aoNum = cal.aoControlChannel;
	//calibrationViewer.removeData ();
	//calibrationViewer.initializeCalData ();
	//auto calData = calibrationViewer.getCalData ();
	//for (auto calPoint : calPtTextToVals (cal.ctrlPtString)) {
	//	if (cal.useAg) {
	//		auto& ag = agilents[int(cal.whichAg)].get();
	//		dcInfo tempInfo;
	//		tempInfo.dcLevel = str(calPoint);
	//		tempInfo.dcLevel.internalEvaluate (std::vector<parameterType> (), 1);
	//		ag.setDC (cal.agChannel, tempInfo, 0);
	//	}
	//	else {
	//		ao->setSingleDac (aoNum, calPoint, ttls->getCore (), { 0, ttls->getCurrentStatus () });
	//	}
	//	cal.resultValues.push_back (ai->getSingleChannelValue (aiNum, cal.avgNum));
	//	*calData << QPointF (calPoint, cal.resultValues.back ());
	//	qApp->processEvents ();
	//	Sleep (20);
	//}
	//cal.currentlyCalibrating = false;
	//std::ofstream file ("C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CalibrationValuesFile.txt");
	//if (!file.is_open ()) {
	//	errBox ("Failed to Write Calibration Results!");
	//}
	//for (auto num : range (cal.resultValues.size())) {
	//	file << calPtTextToVals (cal.ctrlPtString)[num] << " " << cal.resultValues[num] << "\n";
	//}
	//file.close ();
	//cal.result.calibrationCoefficients = pythonHandler->runCalibrationFits (cal, parentWin);
	//cal.result.includesSqrt = cal.includeSqrt;
	//calibrationTable->repaint ();
	//cal.calibrated = true;
	//if (cal.useAg) {
	//	auto& ag = agilents[int (cal.whichAg)].get ();
	//	ag.setCalibration (cal.result, cal.agChannel);
	//}
	//updateCalibrationView (cal);
}

void CalibrationManager::updateCalibrationView (calSettings& cal) {
	std::vector<plotDataVec> plotData;
	plotData.resize (2);
	auto ctrlVec = calPtTextToVals (cal.ctrlPtString);
	if (cal.resultValues.size () != ctrlVec.size ()) {
		return;
	}
	auto maxy = -DBL_MAX;
	auto miny = DBL_MAX;
	for (auto num : range (calPtTextToVals (cal.ctrlPtString).size ())) {
		dataPoint dp;
		dp.x = ctrlVec[num];
		dp.y = cal.resultValues[num];
		plotData[0].push_back (dp);
		miny = (dp.y < miny ? dp.y : miny);
		maxy = (dp.y > maxy ? dp.y : maxy);
	}
	// create fit data. the input to the fit function should be the voltage wanted, and the function should return the
	// control value which gives that voltage, so this is reversed from the control configuration, where you give
	// a control and get a result. 
	int numfitpts = 30;
	plotData[1].resize (numfitpts);
	double runningVal=miny;
	cal.result.minval = miny;
	cal.result.maxval = maxy;
	for (auto pnum : range (plotData[1].size ())) {
		plotData[1][pnum].y = runningVal;
		plotData[1][pnum].x = calibrationFunction (plotData[1][pnum].y, cal.result);
		runningVal += (maxy - miny) / (numfitpts - 1);
	}
	calibrationViewer.resetChart ();
	calibrationViewer.setTitle ("Calibration View: " + cal.result.calibrationName);
	calibrationViewer.setData (plotData);
}

double CalibrationManager::calibrationFunction (double val, calResult calibration) {
	if (val < calibration.minval-1e-6 || val > calibration.maxval+1e-6) {
		thrower ("Tried to use calibration outside of calibration range! Not Allowed!");
	}
	double ctrl = 0;
	auto& cc = calibration.calibrationCoefficients;
	if (calibration.includesSqrt) {
		ctrl += cc[0] * std::pow (val + cc[1], 0.5);
		if (cc.size () > 2) {
			for (auto coefnum : range (cc.size () - 2)) {
				ctrl += cc[coefnum + 2] * std::pow (val, coefnum);
			}
		}
	}
	else {
		for (auto coefnum : range (cc.size ())) {
			ctrl += cc[coefnum] * std::pow (val, coefnum);
		}
	}
	return ctrl;
}

std::vector<double> CalibrationManager::calPtTextToVals (QString qtxt) {
	std::vector<double> vals;
	if (qtxt == "") {
		return vals;
	}
	std::stringstream tmpStream (cstr (qtxt));
	std::string ctrlTxt;
	tmpStream >> ctrlTxt;
	if (ctrlTxt == "(") {
		double leftBound, rightBound, inc;
		tmpStream >> leftBound >> rightBound >> inc;
		if (!tmpStream) {
			errBox ("Error In trying to set the calibration control values! Make sure text is all doubles.");
			return vals;
		}
		auto val = leftBound;
		while (val <= rightBound) {
			vals.push_back (val);
			val += inc;
		}
	}
	else {
		do {
			try {
				vals.push_back (boost::lexical_cast<double>(ctrlTxt));
			}
			catch (boost::bad_lexical_cast&) {
				errBox ("Error In trying to set the calibration control values! Make sure text is all doubles.");
			}
		} while (tmpStream >> ctrlTxt);
	}
	return vals;
}


