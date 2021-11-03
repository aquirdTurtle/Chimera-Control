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
calibrationViewer(1, plotStyle::CalibrationPlot,std::vector<int>(),false,false) {}

void CalibrationManager::initialize (QPoint& pos, IChimeraQtWindow* parent, AiSystem* ai_in, AoSystem* ao_in,
									 DoSystem* ttls_in, std::vector<std::reference_wrapper<AgilentCore>> agilents_in,
									 NewPythonHandler* python_in) {
	auto& px = pos.rx (), & py = pos.ry ();
	px += 480;
	py -= 300;
	calibrationViewer.init (pos, 480, 300, parent, "Calibration View");
	calibrationViewer.plot->xAxis->setLabel("Control Voltage (V)");
	calibrationViewer.plot->yAxis->setLabel("Photodetector Result Voltage (V)");
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
	calibrationTable->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (calibrationTable, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {handleContextMenu (pos); });
	QStringList labels;
	labels << " Name " << " Ctrl Pts (V) " << " Ai " << " Ao " << "Agilent" << "Ag. Channel" 
		<< " DO-Config " << " AO-Config " << " Avgs " << "Use Sqrt" << "Poly Order";
	calibrationTable->setColumnCount (labels.size ());
	calibrationTable->setHorizontalHeaderLabels (labels);
	calibrationTable->horizontalHeader ()->setFixedHeight (25);
	calibrationTable->horizontalHeader ()->setSectionResizeMode (QHeaderView::Interactive);
	calibrationTable->setGeometry (px, py, 960, 350);
	py += 350;
	calibrationTable->setToolTip ("");
	calibrationTable->verticalHeader ()->setSectionResizeMode (QHeaderView::Fixed);
	calibrationTable->verticalHeader ()->setDefaultSectionSize (25);
	calibrationTable->verticalHeader ()->setFixedWidth (50);
	calibrationTable->connect (calibrationTable, &QTableWidget::cellDoubleClicked, [this](int clRow, int clCol) {
		if (clCol == 9) {
			auto* item = new QTableWidgetItem (calibrationTable->item (clRow, clCol)->text () == "No" ? "Yes" : "No");
			item->setFlags (item->flags () & ~Qt::ItemIsEditable);
			calibrationTable->setItem (clRow, clCol, item);
		}});
	calibrationTable->connect (calibrationTable, &QTableWidget::currentCellChanged,
		[this, parent](int row, int col) {if (unsigned(row) < calibrations.size()) {
		try {
			updateCalibrationView (calibrations[row]);
		}
		catch (ChimeraError & err) {
			parent->mainWin->reportErr (err.qtrace ());
		}
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
				case 2:
					cal.aiInChan = boost::lexical_cast<unsigned>(str (qtxt));
					break;
				case 3:
					if (cal.useAg) {
						break;
					}
					try {
						cal.aoControlChannel = boost::lexical_cast<unsigned>(str (qtxt));
					}
					catch (boost::bad_lexical_cast &) {
						emit error ("Error Trying to set analog output channel!");
					}
					break;
				case 4: {
					if (qtxt == "" || !cal.useAg) {
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
				case 5: {
					if (qtxt == "" || !cal.useAg) {
						break;
					}
					try {
						cal.agChannel = boost::lexical_cast<unsigned> (str (qtxt));
					}
					catch (boost::bad_lexical_cast&) {
						emit error ("Error trying to set agilent channel!");
					}
					break;
				}
				case 6: {
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
				case 7: {
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
				case 10: {
					try {
						cal.result.polynomialOrder = boost::lexical_cast<unsigned>(str(qtxt));
					}
					catch (boost::bad_lexical_cast&) {
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
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
		}; });

	auto* setHistorical = new QAction ("Set Historical Fit", calibrationTable);
	calibrationTable->connect (setHistorical, &QAction::triggered, [this, item]() {
		try {
			auto answer = QMessageBox::question (nullptr, qstr ("Are You Sure?"), qstr ("Are You Sure?"), QMessageBox::Yes
				| QMessageBox::No);
			if (answer == QMessageBox::Yes) {
				calibrations[item->row ()].historicalResult = calibrations[item->row ()].result;
			}
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
		}; });


	menu.addAction (setHistorical);
	menu.addAction (deleteAction);
	menu.addAction (newcal);
	menu.addAction (calibrateThis);
	menu.exec (calibrationTable->mapToGlobal (pos));
}

std::vector<calResult> CalibrationManager::getCalibrationInfo (){
	std::vector<calResult> results;
	for (auto& cal : calibrations) {
		results.push_back (cal.result);
	}
	return results;
}

void CalibrationManager::handleSaveMasterConfig (std::stringstream& configStream) {
	configStream << "\n" << systemDelim
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
		<< "\n/*Include Sqrt on Next Cal: */" << cal.includeSqrt;
	configStream << "\n/*Recent Calibration Result:*/"; 
	handleSaveMasterConfigIndvResult (configStream, cal.result);
	configStream << "\n/*Historical Calibration Result:*/";
	handleSaveMasterConfigIndvResult (configStream, cal.historicalResult);
}
void CalibrationManager::handleSaveMasterConfigIndvResult (std::stringstream& configStream, calResult& result) {
	configStream << "\n/*Number of Calibration Coefficients: */ " << result.calibrationCoefficients.size ()
		<< "\n/*Calibration Coefficients: */ " << calBase::dblVecToString (result.calibrationCoefficients)
		<< "\n/*Calibration Includes Sqrt: */ " << result.includesSqrt
		<< "\n/*Polynomial Order: */ " << result.polynomialOrder
		<< "\n/*Number of Control Vals: */" << result.ctrlVals.size ()
		<< "\n/*Control Vals: */" << calBase::dblVecToString (result.ctrlVals)
		<< "\n/*Number of Result Vals: */" << result.resVals.size ()
		<< "\n/*Result Vals: */" << calBase::dblVecToString (result.resVals);
}

void CalibrationManager::handleOpenMasterConfigIndvResult (ConfigStream& configStream, calResult& result) {
	if (configStream.ver > Version ("2.11")) {
		unsigned numCoef;
		configStream >> numCoef;
		if (numCoef > 50) {
			// catch weird bad values...
			thrower ("Suspicious Number of coefficients! Number was " + str (numCoef));
		}
		result.calibrationCoefficients.resize (numCoef);
		for (auto& coef : result.calibrationCoefficients) {
			configStream >> coef;
		}
		configStream >> result.includesSqrt;
		configStream >> result.polynomialOrder;
	}
	if (configStream.ver > Version ("2.12")) {
		unsigned numCtrlVals;
		configStream >> numCtrlVals;
		if (numCtrlVals > 500) {
			// catch weird bad values...
			thrower ("Suspicious Number of control vals! Number was " + str (numCtrlVals));
		}
		result.ctrlVals.resize (numCtrlVals);
		for (auto& val : result.ctrlVals) {
			configStream >> val;
		}

		unsigned numResVals;
		configStream >> numResVals;
		if (numResVals > 500) {
			// catch weird bad values...
			thrower ("Suspicious Number of result vals! Number was " + str (numResVals));
		}
		result.resVals.resize (numResVals);
		for (auto& val : result.resVals) {
			configStream >> val;
		}
	}
	determineCalMinMax (result);
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
			configStream >> tmpInfo.includeSqrt;
		}
		handleOpenMasterConfigIndvResult (configStream, tmpInfo.result);
		if (configStream.ver > Version ("3.0")) {
			handleOpenMasterConfigIndvResult (configStream, tmpInfo.historicalResult);
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
	ConfigSystem::jumpToDelimiter (configStream, "CALIBRATION_MANAGER");
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
			ag.setAgCalibration (cal.result, cal.agChannel);
		}
	}
	refreshListview ();
}

void CalibrationManager::refreshListview () {
	calibrationTable->setRowCount (0);
	for (auto& cal : calibrations) {
		addCalToListview (cal);
	}
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
		calibrationTable->item (row, item)->setToolTip (qstr(cal.result.stringRepr ()));
	};
	calibrationTable->insertRow (row);
	calibrationTable->setItem (row, 0, new QTableWidgetItem (cal.result.calibrationName.c_str ()));
	setItemExtra (0);
	calibrationTable->setItem (row, 1, new QTableWidgetItem (cal.ctrlPtString));
	setItemExtra (1);
	calibrationTable->setItem (row, 2, new QTableWidgetItem (cstr (cal.aiInChan, precision)));
	setItemExtra (2);
	
	calibrationTable->setItem (row, 3, new QTableWidgetItem (cal.useAg ? "---" : qstr (cal.aoControlChannel)));
	calibrationTable->item (row, 3)->setFlags (cal.useAg || !cal.active ?
		calibrationTable->item (row, 3)->flags () & ~Qt::ItemIsEnabled
		: calibrationTable->item (row, 3)->flags () | Qt::ItemIsEnabled);
	
	calibrationTable->setItem (row, 4, new QTableWidgetItem (cal.useAg ? qstr(AgilentEnum::toStr(cal.whichAg)) : "---"));
	calibrationTable->item (row, 4)->setFlags (!cal.useAg || !cal.active ?
		calibrationTable->item (row, 4)->flags () & ~Qt::ItemIsEnabled
		: calibrationTable->item (row, 4)->flags () | Qt::ItemIsEnabled);

	calibrationTable->setItem (row, 5, new QTableWidgetItem (cal.useAg ? qstr (cal.agChannel) : "---"));
	calibrationTable->item (row, 5)->setFlags (!cal.useAg || !cal.active ?
		calibrationTable->item (row, 5)->flags () & ~Qt::ItemIsEnabled
		: calibrationTable->item (row, 5)->flags () | Qt::ItemIsEnabled);

	calibrationTable->setItem (row, 6, new QTableWidgetItem (calTtlConfigToString (cal.ttlConfig).c_str ()));
	setItemExtra (6);
	calibrationTable->setItem (row, 7, new QTableWidgetItem (calDacConfigToString (cal.aoConfig).c_str ()));
	setItemExtra (7);
	calibrationTable->setItem (row, 8, new QTableWidgetItem (qstr (cal.avgNum, precision)));
	setItemExtra (8);
	calibrationTable->setItem (row, 9, new QTableWidgetItem (cal.includeSqrt ? "True" : "False"));
	setItemExtra (9);
	calibrationTable->setItem (row, 10, new QTableWidgetItem (qstr(cal.result.polynomialOrder)));
	setItemExtra (10);
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

bool CalibrationManager::wantsExpAutoCal () {
	return expAutoCalButton->isChecked ();
}

void CalibrationManager::runAllThreaded () {
	emit notification({ str("Running All Calibrations.\n"),0,systemDelim });
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
	connect (threadWorker, &CalibrationThreadWorker::warn, this, &IChimeraSystem::warning);
	connect (threadWorker, &CalibrationThreadWorker::error, this, &IChimeraSystem::error);
	connect (threadWorker, &CalibrationThreadWorker::calibrationChanged, this, [this]() { refreshListview (); });
	connect (threadWorker, &CalibrationThreadWorker::startingNewCalibration, this, [this](calSettings cal) {
		try {
			updateCalibrationView(cal);
			calibrationViewer.initializeCalData(cal);
		}
		catch (ChimeraError & err) {
			this->parentWin->reportErr(err.qtrace());
		}
		});
	connect (threadWorker, &CalibrationThreadWorker::newCalibrationDataPoint, this, [this](QPointF pt) {
			auto* chartData = calibrationViewer.getCalData ();
			chartData->addData(pt.x(), pt.y());
			calibrationViewer.plot->replot();
			//*chartData << pt;
		});
	connect (threadWorker, &CalibrationThreadWorker::finishedCalibration, this, [this](calSettings cal) {
		try {
			updateCalibrationView(cal);
			refreshListview();
			}
		catch (ChimeraError & err) {
			this->parentWin->reportErr(err.qtrace());
		}
		});

	connect (thread, &QThread::started, threadWorker, &CalibrationThreadWorker::runAll);
	connect (thread, &QThread::finished, thread, &QObject::deleteLater);
	connect (thread, &QThread::finished, threadWorker, &QObject::deleteLater);
	thread->start ();
}

void CalibrationManager::calibrateThreaded (calSettings& cal, unsigned which) {
	std::vector<std::reference_wrapper<calSettings>> calInput;
	calInput.push_back (cal);
	standardStartThread (calInput);
}

void CalibrationManager::determineCalMinMax (calResult& res) {
	auto maxy = -DBL_MAX;
	auto miny = DBL_MAX;
	for (auto yp : res.resVals) {
		miny = (yp < miny ? yp : miny);
		maxy = (yp > maxy ? yp : maxy);
	}
	// create fit data. the input to the fit function should be the voltage wanted, and the function should return the
	// control value which gives that voltage, so this is reversed from the control configuration, where you give
	// a control and get a result. 
	res.calmin = miny;
	res.calmax = maxy;
}

void CalibrationManager::updateCalibrationView (calSettings& cal) {
	std::vector<plotDataVec> plotData;
	plotData.resize (3);
	cal.result.ctrlVals = calPtTextToVals (cal.ctrlPtString);
	if (cal.result.resVals.size () != cal.result.ctrlVals.size ()) {
		thrower ("Result vector doesn't match control vector size?!");
		return;
	}
	determineCalMinMax (cal.result);
	for (auto num : range (cal.result.ctrlVals.size ())) {
		dataPoint dp{ cal.result.ctrlVals[num] , cal.result.resVals[num] };
		plotData[0].push_back (dp);
	}
	int numfitpts = 30;
	plotData[1].resize (numfitpts);

	double runningVal= cal.result.calmin;
	for (auto pnum : range (plotData[1].size ())) {
		plotData[1][pnum].y = runningVal;
		plotData[1][pnum].x = calibrationFunction (plotData[1][pnum].y, cal.result, this);
		runningVal += (cal.result.calmax - cal.result.calmin) / (numfitpts - 1);
	}

	numfitpts = 30;
	plotData[2].resize (numfitpts);
	determineCalMinMax (cal.historicalResult);
	runningVal = cal.historicalResult.calmin;
	for (auto pnum : range (plotData[2].size ())) {
		plotData[2][pnum].y = runningVal;
		plotData[2][pnum].x = calibrationFunction (plotData[2][pnum].y, cal.historicalResult, this);
		runningVal += (cal.historicalResult.calmax - cal.historicalResult.calmin) / (numfitpts - 1);
	}
	
	calibrationViewer.resetChart ();
	calibrationViewer.setTitle ("Calibration View: " + cal.result.calibrationName);
	calibrationViewer.setData (plotData);
}

double CalibrationManager::calibrationFunction (double val, calResult res, IChimeraSystem* parent) {
	if (val < res.calmin - 1e-6) {
		if (parent != nullptr) {
			emit parent->warning (qstr("Warning: Tried to set calibrated value below calibration range. Assuming that you want the "
								  "minimum value the calibration can provide."));
		}
		val = res.calmin;
	}
	if (val > res.calmax+1e-6) {
		thrower ("Tried to use calibration \""+ res.calibrationName + "\"above calibration range! Not Allowed! Value "
				 "was " + str(val) + ", Max is " + str(res.calmax));
	}
	double ctrl = 0;
	auto& cc = res.calibrationCoefficients;
	if (cc.size () != res.polynomialOrder + 2 * res.includesSqrt) {
		thrower ("calibration constants size doesn't match fit polynomial order!");
	}
	if (res.includesSqrt) {
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
	// format is " [ initVal finVal ) numVals" where "[" and ")" can be either brackets or parenthesis for inclusive
	// and exclusive endpoints, or it's just a list of values for arbitrary values. 
	std::vector<double> vals;
	if (qtxt == "") {
		return vals;
	}
	std::stringstream tmpStream (cstr (qtxt));
	std::string ctrlTxt;
	tmpStream >> ctrlTxt;
	if (ctrlTxt == "(" || ctrlTxt == "[") {
		bool lIncl = bool(ctrlTxt == "[");
		double leftBound, rightBound;
		unsigned numPts;
		std::string rightInclusive;
		tmpStream >> leftBound >> rightBound >> rightInclusive >> numPts;
		if (rightInclusive != ")" && rightInclusive != "]") {
			thrower ("ERROR: bad right inclusive text! Expected \")\" or \"]\"!");
		}
		bool rIncl = bool (rightInclusive == "]");
		if (!tmpStream) {
			errBox ("Error In trying to set the calibration control values! Make sure text is all doubles.");
			return vals;
		}
		int spacings = numPts + (!lIncl && !rIncl) - (lIncl && rIncl);
		double valueRange = (rightBound - leftBound);
		double initVal = (lIncl ? leftBound : leftBound + valueRange / spacings);
		unsigned valNum = 0;
		for (auto valNum : range (numPts)) {
			vals.push_back(valueRange * valNum / spacings + initVal);
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


