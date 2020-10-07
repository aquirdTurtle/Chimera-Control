// created by Mark O. Brown
#include "stdafx.h"
#include "ServoManager.h"
#include "boost/lexical_cast.hpp"
#include "PhotodetectorCalibration.h"
#include <QHeaderView>
#include <QMenu>
#include "GeneralObjects/ChimeraStyleSheets.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <qapplication.h>

ServoManager::ServoManager (IChimeraQtWindow* parent) : IChimeraSystem(parent) {

}

void ServoManager::handleContextMenu (const QPoint& pos){
	QTableWidgetItem* item = servoList->itemAt (pos);
	if (!item) { return; }
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	if (servos[item->row ()].active) {
		auto* deactivate = new QAction ("Deactivate", servoList);
		servoList->connect (deactivate, &QAction::triggered,
			[this, item]() {servos[item->row ()].active = false; refreshListview (); });
		menu.addAction (deactivate);
	}
	else{
		auto* activate = new QAction ("Activate", servoList);
		servoList->connect (activate, &QAction::triggered,
			[this, item]() {servos[item->row ()].active = true; refreshListview (); });
		menu.addAction (activate);
	}
	auto* deleteAction = new QAction ("Delete This Item", servoList);
	servoList->connect (deleteAction, &QAction::triggered, 
		[this, item]() {servos.erase (servos.begin () + item->row ()); refreshListview (); });
	auto* newServo = new QAction ("New Item", servoList);
	servoList->connect ( newServo, &QAction::triggered, [this]() {servos.push_back (servoInfo ()); refreshListview (); });
	auto* calibrateThisServo = new QAction ("Calibrate This Servo", servoList);
	servoList->connect (calibrateThisServo, &QAction::triggered, [this, item]() {
		try	{
			calibrate (servos[item->row ()], item->row ());
			refreshListview ();
		}
		catch (ChimeraError& err){
			errBox (err.trace ());
		}; });
	menu.addAction (deleteAction);
	menu.addAction (newServo);
	menu.addAction (calibrateThisServo);
	menu.exec (servoList->mapToGlobal (pos));
}

void ServoManager::initialize( QPoint& pos, IChimeraQtWindow* parent, AiSystem* ai_in, AoSystem* ao_in, 
							   DoSystem* ttls_in, ParameterSystem* globals_in){
	auto& px = pos.rx (), & py = pos.ry ();
	servosHeader = new QLabel ("SERVO MANAGER", parent);
	servosHeader->setGeometry (px, py, 280, 20);
	servoButton = new CQPushButton ("Servo All", parent);
	servoButton->setGeometry (px + 280, py, 175, 20);
	servoButton->setToolTip ("Force the servo to calibrate.");
	parent->connect (servoButton, &QPushButton::released, [this, parent]() {
		if (!parent->mainWin->expIsRunning ()) {
			runAll ();
		}});
	unitsCombo = new CQComboBox (parent);
	unitsCombo->setGeometry (px + 280 + 175, py, 100, 20);
	for (auto unitsOpt : AiUnits::allOpts) {
		unitsCombo->addItem (AiUnits::toStr (unitsOpt).c_str ());
	}
	parent->connect (unitsCombo, qOverload<int> (&QComboBox::currentIndexChanged), [this](int) {refreshListview (); });

	expAutoServoButton = new CQCheckBox ("Exp. Auto-Servo?", parent);
	expAutoServoButton->setGeometry (px + 380 + 175, py, 175, 20);
	expAutoServoButton->setToolTip ("Automatically calibrate all servos before doing any experiment?");

	cancelServo = new QPushButton ("Cancel Servo?", parent);
	cancelServo->setGeometry (px + 730, py, 150, 20);
	cancelServo->setToolTip ("Hold this button down to cancel a \"Run All\" servoing.");

	py += 20;
	servoList = new QTableWidget (parent);
	QStringList labels;
	labels << "Name" << " Set (V) " << " Ctrl (V) " << " dCtrl (%) " << " Res (V) " << "Ai" << "Ao" << " DO-Config " << " Tolerance "
		   << " Gain " << " Monitor? " << " AO-Config " << "Avgs";
	servoList->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (servoList, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {handleContextMenu (pos); });
	servoList->setColumnCount (labels.size());
	servoList->setHorizontalHeaderLabels (labels);
	servoList->horizontalHeader ()->setFixedHeight (25);
	servoList->setGeometry (px, py, 960, 450);
	servoList->setToolTip ( "Name: The name of the servo, gets incorperated into the name of the servo_variable.\n"
						   "Active: Whether the servo will calibrate when you auto-servoing or after servo-once\n"
						   "Set-Point: The servo\'s set point, in volts.\n"
						   "Control-Value: The current control value (in volts) which was found to make the input match the set-point\n"
						   "Ai: The analog input that the servo will watch & match to the set-point\n"
						   "Ao: The analog output the servo will use to try to change the value of the analog input\n"
						   "DO-Config: The digital output configuration the sevo will set before servoing. If a ttl is "
								"not listed here, it will be zero\'d.\n"
						   "Tolerance: The tolerance of the servo. The servo will stop servoing when value falls within tol of the set point\n"
						   "Gain: The gain of the servo loop.\n"
						   "Monitor: If monitoring, the servo only checks that the value is within the tolerance of the set point and raises a warning if not.\n"
						   "AO-Config: The digital output configuration the sevo will set before servoing. If a ao is "
								"not listed here, it will be zero\'d.\n");
	servoList->resizeColumnsToContents ();
	servoList->verticalHeader()->setSectionResizeMode (QHeaderView::Fixed);
	servoList->verticalHeader ()->setDefaultSectionSize (22);
	servoList->verticalHeader ()->setFixedWidth (40);
	servoList->connect (servoList, &QTableWidget::cellDoubleClicked, [this](int clRow, int clCol) {
		if (clCol == 10) {
			auto* item = new QTableWidgetItem (servoList->item (clRow, clCol)->text () == "No" ? "Yes" : "No");
			item->setFlags (item->flags () & ~Qt::ItemIsEditable);
			servoList->setItem (clRow, clCol, item);
		}});
	servoList->connect (
		servoList, &QTableWidget::cellChanged, [this](int row, int col) {
			if (servos.size () <= row) {
				return;
			}
			auto qtxt = servoList->item (row, col)->text ();
			switch (col) {
				case 0:
					servos[row].servoName = str (qtxt);
					break;
				case 1:
					servos[row].setPoint = boost::lexical_cast<double>(str (qtxt));
					break;
				case 5:
					servos[row].aiInChan = boost::lexical_cast<unsigned>(str (qtxt));
					break;
				case 6:
					servos[row].aoControlChannel = boost::lexical_cast<unsigned>(str (qtxt));
					break;
				case 7: {
					std::stringstream tmpStream (cstr (qtxt));
					std::string rowTxt;
					servos[row].ttlConfig.clear ();
					while (tmpStream >> rowTxt) {
						try {
							std::pair<DoRows::which, unsigned> ttl;
							ttl.first = DoRows::fromStr (rowTxt);
							tmpStream >> ttl.second;
							servos[row].ttlConfig.push_back (ttl);
						}
						catch (ChimeraError&) {
							errBox ("Error In trying to set the servo ttl config!");
						}
					}
					break;
				}
				case 11: {
					std::stringstream tmpStream (cstr (qtxt));
					std::string dacIdTxt;
					servos[row].aoConfig.clear ();
					while (tmpStream >> dacIdTxt) {
						try {
							auto id = AoSystem::getBasicDacIdentifier (dacIdTxt);
							if (id == -1) {
								thrower ("Dac Identifier \"" + dacIdTxt + "\" failed to convert to a basic dac id!");
							}
							std::pair<unsigned, double> dacSetting;
							dacSetting.first = id;
							tmpStream >> dacSetting.second;
							servos[row].aoConfig.push_back (dacSetting);
						}
						catch (ChimeraError&) {
							errBox ("Error In trying to set the servo dac config!");
						}
					}
					break;
				}
			}
		}
	);

	ai = ai_in;
	ao = ao_in;
	ttls = ttls_in;
	globals = globals_in;
}

std::vector<servoInfo> ServoManager::getServoInfo ( ){
	return servos;
}

AiUnits::which ServoManager::getUnitsOption(){
	return AiUnits::fromStr (str(unitsCombo->currentText ()));
}

void ServoManager::handleSaveMasterConfig( std::stringstream& configStream ){
	configStream << 0 << "\n" << expAutoServoButton->isChecked() << "\n"
		<< AiUnits::toStr(getUnitsOption()) << "\n" << servos.size ( ) << "\n";
	for ( auto& servo : servos ){
		handleSaveMasterConfigIndvServo ( configStream, servo );
	}
}

void ServoManager::handleOpenMasterConfig( ConfigStream& configStream ){
	if ( configStream.ver < Version( "2.1" ) ){
		// this was before the servo manager.
		return;
	}
	if (configStream.ver< Version ( "2.5" ) ){
		double tolerance;
		configStream >> tolerance;
	}
	bool calAutoServo, expAutoServo;
	configStream >> calAutoServo;
	if (configStream.ver >= Version ("2.9")){
		configStream >> expAutoServo;
		expAutoServoButton->setChecked (expAutoServo);
		std::string units_txt;
		configStream.get ();
		getline (configStream, units_txt);
		int index = unitsCombo->findData (QString(units_txt.c_str()));
		if (index != -1) { // -1 for not found
			unitsCombo->setCurrentIndex (index);
		}
	}
	LONG numServosInFile;
	configStream >> numServosInFile;
	servos.clear ( );
	for ( auto servoNum : range ( numServosInFile ) ){
		servos.push_back ( handleOpenMasterConfigIndvServo ( configStream ) );
	}
	refreshListview ( );
}

servoInfo ServoManager::handleOpenMasterConfigIndvServo ( ConfigStream& configStream ){
	servoInfo tmpInfo;
	configStream >> tmpInfo.servoName;
	if ( configStream.ver > Version ( "2.3" ) ){
		configStream >> tmpInfo.aiInChan >> tmpInfo.aoControlChannel;
	}
	else{
		tmpInfo.aiInChan = tmpInfo.aoControlChannel = 0;
	}
	configStream >> tmpInfo.active >> tmpInfo.setPoint;
	if (configStream.ver > Version ( "2.3" ) ){
		unsigned numSettings;
		configStream >> numSettings;
		tmpInfo.ttlConfig.resize ( numSettings );
		for ( auto& ttl : tmpInfo.ttlConfig ){
			std::string rowStr;
			configStream >> rowStr >> ttl.second;
			ttl.first = DoRows::fromStr ( rowStr );
		}
	}
	if (configStream.ver > Version ("2.6")){
		unsigned numSettings;
		configStream >> numSettings;
		tmpInfo.aoConfig.resize (numSettings);
		for (auto& ao : tmpInfo.aoConfig){
			unsigned dacID;
			configStream >> dacID >> ao.second;
			ao.first = dacID;
		}
	}
	if (configStream.ver > Version ( "2.4" ) ){
		configStream >> tmpInfo.tolerance >> tmpInfo.gain;
	}
	if (configStream.ver > Version ( "2.5" ) ){
		configStream >> tmpInfo.monitorOnly;	
	}
	if (configStream.ver > Version ("2.7")){
		configStream >> tmpInfo.avgNum;
	}
	return tmpInfo;
}

void ServoManager::refreshListview ( ){
	servoList->setRowCount (0);
	for ( auto& servo : servos ){
		addServoToListview ( servo );
	}
	servoList->resizeColumnsToContents ();
}

void ServoManager::addServoToListview ( servoInfo& si ){
	int row = servoList->rowCount ();
	auto ctp = [this, &si](double volt) {return convertToPower (volt, si); };
	int precision = 5;
	QColor textColor;
	if (si.servoed) {
		textColor = QColor (255, 255, 255);
	}
	else {
		textColor = QColor (255, 0, 0);
	}
	auto setItemExtra = [row, this, si, textColor](int item) {
		servoList->item (row, item)->setFlags (!si.active ? servoList->item (row, item)->flags () & ~Qt::ItemIsEnabled
															: servoList->item (row, item)->flags () | Qt::ItemIsEnabled);
		servoList->item (row, item)->setForeground (textColor);
	};
	servoList->insertRow (row);
	servoList->setItem (row, 0, new QTableWidgetItem ( si.servoName.c_str ()));
	setItemExtra (0);
	servoList->setItem (row, 1, new QTableWidgetItem ( cstr(ctp(si.setPoint), precision)));
	setItemExtra (1);
	servoList->setItem (row, 2, new QTableWidgetItem (si.monitorOnly ? "--" : str (ctp(si.controlValue), precision).c_str()));
	servoList->item (row, 2)->setFlags (servoList->item (row, 2)->flags () ^ Qt::ItemIsEnabled);
	setChangeVal (row, si.changeInCtrl);
	servoList->setItem (row, 4, new QTableWidgetItem (cstr (ctp(si.mostRecentResult), precision)));
	servoList->item (row, 4)->setFlags (servoList->item (row, 4)->flags () ^ Qt::ItemIsEnabled);
	servoList->setItem (row, 5, new QTableWidgetItem (cstr (si.aiInChan, precision)));
	setItemExtra (5);
	servoList->setItem (row, 6, new QTableWidgetItem ((si.monitorOnly ? "--" : str (si.aoControlChannel )).c_str()));
	setItemExtra (6);
	servoList->setItem (row, 7, new QTableWidgetItem (servoTtlConfigToString (si.ttlConfig).c_str()));
	setItemExtra (7);
	servoList->setItem (row, 8, new QTableWidgetItem (cstr (si.tolerance, precision)));
	setItemExtra (8);
	servoList->setItem (row, 9, new QTableWidgetItem ((si.monitorOnly ? "--" : str (si.gain )).c_str()));
	setItemExtra (9);
	servoList->setItem (row, 10, new QTableWidgetItem (cstr(si.monitorOnly ? "Yes" : "No") ));
	setItemExtra (10);
	servoList->setItem (row, 11, new QTableWidgetItem (servoDacConfigToString(si.aoConfig).c_str()));
	setItemExtra (11);
	servoList->setItem (row, 12, new QTableWidgetItem (cstr (si.avgNum, precision)));
	setItemExtra (12);
}

std::string ServoManager::servoDacConfigToString (std::vector<std::pair<unsigned, double>> aoConfig){
	std::string aoString;
	for (auto ao : aoConfig){
		aoString += "dac" + str (ao.first) + " " + str (ao.second, 4) + " ";
	}
	return aoString;
}

std::string ServoManager::servoTtlConfigToString (std::vector<std::pair<DoRows::which, unsigned> > ttlConfig){
	std::string digitalOutConfigString;
	for (auto val : ttlConfig){
		digitalOutConfigString += DoRows::toStr (val.first) + " " + str (val.second) + " ";
	}
	return digitalOutConfigString;
}

void ServoManager::handleSaveMasterConfigIndvServo ( std::stringstream& configStream, servoInfo& servo ){
	configStream << servo.servoName << " " << servo.aiInChan << " " << servo.aoControlChannel << " "
		<< servo.active << " " << servo.setPoint << " " << servo.ttlConfig.size ( ) << " " 
		<< servoTtlConfigToString (servo.ttlConfig);
	configStream << servo.aoConfig.size () << " ";
	for (auto& dac : servo.aoConfig){
		configStream << dac.first << " " << dac.second << " ";
	}
	configStream << servo.tolerance << " " << servo.gain << " " << servo.monitorOnly << " " << servo.avgNum << "\n";
}


bool ServoManager::wantsExpAutoServo (){
	return expAutoServoButton->isChecked ();
}

void ServoManager::runAll() {
	emit notification ("Running All Servos.\n");
	unsigned count = 0;
	// made this asynchronous to facilitate updating gui while 
	for ( auto& servo : servos ){
		if (cancelServo->isDown ()) {
			break;
		}
		auto origColor = servoList->item (count, 0)->background ();
		servoList->item (count, 0)->setBackground (Qt::red);
		try{
			ServoManager::calibrate (servo, count);
		}
		catch (ChimeraError & e) {
			emit error (qstr(e.trace ()));
			//comm.sendError (e.trace ());
			// but continue to try the other ones. 
		}
		servoList->item (count, 0)->setBackground(origColor);
		count++;
	}
	refreshListview ();
	ttls->zeroBoard ( );
	ao->zeroDacs (ttls->getCore (), { 0, ttls->getCurrentStatus () });
}

/**
 * expects the inputted power to be in -MILI-WATTS!
 * returns set point in VOLTS
 */
double ServoManager::convertToPower (double volt, servoInfo& si){
	double power = 0;
	// build the polynomial calibration.
	unsigned polyPower = 0;
	auto opt = getUnitsOption ();
	auto cc =  (opt == AiUnits::which::pdVolts ? std::vector<double> ({ 0,1 }) :
				opt == AiUnits::which::atomsPower ? AI_SYSTEM_CAL[si.aiInChan].atAtomsCalCoeff 
												  : AI_SYSTEM_CAL[si.aiInChan].atPdCalCoeff );
	for (auto coeff : cc){
		power += coeff * std::pow (volt, polyPower++);
	}
	return power;
} 

void ServoManager::calibrate( servoInfo& s, unsigned which ){
	if ( !s.active ){
		return;
	}
	emit notification (qstr("Running Servo "+s.servoName+".\n"),1);
	double sp = s.setPoint;
	s.currentlyServoing = true;
	ttls->zeroBoard ( );
	ao->zeroDacs (ttls->getCore (), { 0, ttls->getCurrentStatus () });
	for (auto dac : s.aoConfig){
		ao->setSingleDac (dac.first, dac.second, ttls->getCore (), { 0, ttls->getCurrentStatus () });
	}
	for ( auto ttl : s.ttlConfig ){
		auto& outputs = ttls->getDigitalOutputs ();
		outputs (ttl.second, ttl.first).check->setChecked (true);
		outputs(ttl.second, ttl.first).set (1);
		ttls->getCore ().ftdi_ForceOutput (ttl.first, ttl.second, 1, ttls->getCurrentStatus());
	}
	Sleep (200); // give some time for the lasers to settle..
	unsigned attemptLimit = 100;
	unsigned count = 0;
	unsigned aiNum = s.aiInChan;
	unsigned aoNum = s.aoControlChannel;
	if ( s.monitorOnly ){	// handle "servos" which are only monitoring values, not trying to change them. 
		double avgVal = ai->getSingleChannelValue ( aiNum, s.avgNum );
		s.mostRecentResult = avgVal;
		double percentDif = ( sp - avgVal) / sp;
		if ( fabs ( percentDif )  < s.tolerance ) { /* Value looks good, nothing to report. */ }
		else{
			errBox ( s.servoName + " Monitor: Value has drifted out of tolerance!" );
		}
		// And the rest of the function is handling the servo part. 
		s.currentlyServoing = false;
		return;
	}
	s.controlValue = globals->getVariableValue (str (s.servoName + servoSuffix, 13, false, true));
	// start the dac where it was last.
	auto oldVal = s.controlValue;
	ao->setSingleDac (aoNum, s.controlValue, ttls->getCore (), { 0, ttls->getCurrentStatus () });
	unsigned requiredConsecutiveMatches = 3;
	unsigned numConsecutiveMatches = 0;
	while ( count++ < attemptLimit ){
		double avgVal = ai->getSingleChannelValue(aiNum, s.avgNum);
		s.mostRecentResult = avgVal;
		double percentDif = (sp - avgVal) / fabs(sp);
		if ( fabs(percentDif)  < s.tolerance ){
			numConsecutiveMatches++;
			if (numConsecutiveMatches >= requiredConsecutiveMatches) {
				// found a good value.
				break;
			}
		}
		else{
			numConsecutiveMatches = 0;
			// modify dac value.
			s.controlValue = ao->getDacValue( aoNum );
			double diff = s.gain * percentDif > 0.05 ? 0.05 : s.gain * percentDif;
			s.controlValue += diff;
			try{
				ao->setSingleDac( aoNum, s.controlValue, ttls->getCore (), { 0, ttls->getCurrentStatus () });
			}
			catch ( ChimeraError& ){
				// happens if servo value gives result out of range of dacs.
				auto r = ao->getDacRange ( aoNum );
				try	{
					if ( s.controlValue < r.first ){
						ao->setSingleDac ( aoNum, r.first, ttls->getCore (), { 0, ttls->getCurrentStatus () });
					}
					else if ( s.controlValue > r.second ){
						ao->setSingleDac ( aoNum, r.second, ttls->getCore(), { 0, ttls->getCurrentStatus () });
					}
				}
				catch ( ChimeraError& ){
					// something went wrong...
					count = attemptLimit;
					break;
				}
			}
			// There's a little break built in here in order to let the laser power settle a little. 
			// Not sure how necessary this is.
			Sleep( 20 );
			s.changeInCtrl = (s.controlValue - oldVal) / oldVal;
			setControlDisplay ( which, ao->getDacValue( aoNum ) );
			setResDisplay (which, convertToPower(s.mostRecentResult, s));
			setChangeVal (which, convertToPower(s.changeInCtrl, s));
			qApp->processEvents ();
		}
	}
	auto dacVal = ao->getDacValue ( aoNum );
	s.changeInCtrl = (s.controlValue - oldVal) / oldVal; 
	setControlDisplay ( which, dacVal );
	setResDisplay (which, convertToPower(s.mostRecentResult,s));
	setChangeVal (which, convertToPower(s.changeInCtrl,s));
	
	s.servoed = (count < attemptLimit);
	s.currentlyServoing = false;
	if ( !s.servoed ){
		thrower( "" + s.servoName + " servo failed to servo!" );
		// and don't adjust the variable value with what is probably a bad value. 
	}
	else{
		globals->adjustVariableValue( str(s.servoName + servoSuffix, 13, false, true), dacVal );
	}
	servoList->repaint ();
}

void ServoManager::setChangeVal(unsigned which, double change){
	servoList->setItem (which, 3, new QTableWidgetItem(((change < 0 ? "" : "+") + str (change*100,5)).c_str()));
	servoList->item(which, 3)->setFlags(servoList->item (which, 3)->flags () ^ Qt::ItemIsEnabled); 
	if (abs (change) < 0.02) {
		servoList->item (which, 3)->setForeground (QBrush (QColor (0, 255, 0)));
	}
	else if (abs (change) < 0.05) {
		servoList->item (which, 3)->setForeground (QBrush (QColor (255, 165, 0)));
	}
	else {
		servoList->item (which, 3)->setForeground (QBrush (QColor (255, 0, 0)));
	}
}

void ServoManager::setControlDisplay (unsigned which, double value ){
	servoList->item (which, 2)->setText (cstr (value,5));
}

void ServoManager::setResDisplay (unsigned which, double value){
	servoList->item (which,4)->setText (cstr (value,5));
}

