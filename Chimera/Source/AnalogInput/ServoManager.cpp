// created by Mark O. Brown
#include "stdafx.h"
#include "ServoManager.h"
#include "ExcessDialogs/TextPromptDialog.h"
#include "boost/lexical_cast.hpp"
#include "ExperimentThread/Communicator.h"
#include "PhotodetectorCalibration.h"
#include <QHeaderView>
#include <QMenu>
#include "GeneralObjects/ChimeraStyleSheets.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <qapplication.h>

void ServoManager::handleContextMenu (const QPoint& pos)
{
	QTableWidgetItem* item = servoList->itemAt (pos);
	if (!item) { return; }
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete This Item", servoList);
	servoList->connect (deleteAction, &QAction::triggered, 
		[this, item]() {servos.erase (servos.begin () + item->row ()); refreshListview (); });
	auto* newServo = new QAction ("New Item", servoList);
	servoList->connect ( newServo, &QAction::triggered, [this]() {servos.push_back (servoInfo ()); refreshListview (); });
	auto* calibrateThisServo = new QAction ("Calibrate This Servo", servoList);
	servoList->connect (calibrateThisServo, &QAction::triggered, [this, item]() {
		try	{
			calibrate (servos[item->row ()], item->row ());
		}
		catch (Error& err){
			errBox (err.trace ());
		}; });

	menu.addAction (deleteAction);
	menu.addAction (newServo);
	menu.addAction (calibrateThisServo);
	menu.exec (servoList->mapToGlobal (pos));
}

void ServoManager::handleDraw (NMHDR* pNMHDR, LRESULT* pResult){}

void ServoManager::initialize( POINT& pos, IChimeraWindowWidget* parent, AiSystem* ai_in, AoSystem* ao_in, DoSystem* ttls_in, 
							   ParameterSystem* globals_in)
{
	servosHeader = new QLabel ("SERVO MANAGER", parent);
	servosHeader->setGeometry (pos.x, pos.y, 280, 20);
	servoButton = new CQPushButton ("Servo Once", parent);
	servoButton->setGeometry (pos.x + 280, pos.y, 175, 20);
	servoButton->setToolTip ("Force the servo to calibrate.");
	parent->connect (servoButton, &QPushButton::released, [this, parent]() {runAll (*parent->mainWin->getComm()); });
	unitsCombo = new CQComboBox (parent);
	unitsCombo->setGeometry (pos.x + 280 + 175, pos.y, 100, 20);
	for (auto unitsOpt : AiUnits::allOpts) {
		unitsCombo->addItem (AiUnits::toStr (unitsOpt).c_str ());
	}
	parent->connect (unitsCombo, qOverload<int> (&QComboBox::currentIndexChanged), [this](int) {refreshListview (); });

	expAutoServoButton = new CQCheckBox ("Exp. Auto-Servo?", parent);
	expAutoServoButton->setGeometry (pos.x + 380 + 175, pos.y, 175, 20);
	expAutoServoButton->setToolTip ("Automatically calibrate all servos before doing any experiment?");

	calAutoServoButton = new CQCheckBox ("Exp. Auto-Servo?", parent);
	calAutoServoButton->setGeometry (pos.x + 380 + 350, pos.y, 175, 20);
	calAutoServoButton->setToolTip ("Automatically calibrate all servos before doing standard calibration runs?");
	pos.y += 20;
	servoList = new QTableWidget (parent);
	QStringList labels;
	labels << "Name" << " Active? " << " Set (V) " << " Ctrl (V) " << " dCtrl (%) " << " Res (V) " << "Ai" << "Ao" << " DO-Config " << " Tolerance "
		   << " Gain " << " Monitor? " << " AO-Config " << "Avgs";
	servoList->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (servoList, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {this->handleContextMenu (pos); });
	servoList->setColumnCount (labels.size());
	servoList->setHorizontalHeaderLabels (labels);
	servoList->horizontalHeader ()->setFixedHeight (25);
	servoList->setGeometry (pos.x, pos.y, 960, 450);
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
		if (clCol == 1 || clCol == 11) {
			auto* item = new QTableWidgetItem (servoList->item (clRow, clCol)->text () == "No" ? "Yes" : "No");
			item->setFlags (item->flags () & ~Qt::ItemIsEditable);
			servoList->setItem (clRow, clCol, item);
		}});

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


void ServoManager::handleSaveMasterConfig( std::stringstream& configStream )
{

	configStream << calAutoServoButton->isChecked () << "\n" << expAutoServoButton->isChecked() << "\n"
		<< AiUnits::toStr(getUnitsOption()) << "\n" << servos.size ( ) << "\n";
	for ( auto& servo : servos )
	{
		handleSaveMasterConfigIndvServo ( configStream, servo );
	}
}

void ServoManager::handleOpenMasterConfig( ConfigStream& configStream )
{
	if ( configStream.ver < Version( "2.1" ) )
	{
		// this was before the servo manager.
		return;
	}
	if (configStream.ver< Version ( "2.5" ) )
	{
		double tolerance;
		configStream >> tolerance;
	}
	bool calAutoServo, expAutoServo;
	configStream >> calAutoServo;
	calAutoServoButton->setChecked( calAutoServo );
	if (configStream.ver >= Version ("2.9"))
	{
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
	for ( auto servoNum : range ( numServosInFile ) )
	{
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
		UINT numSettings;
		configStream >> numSettings;
		tmpInfo.ttlConfig.resize ( numSettings );
		for ( auto& ttl : tmpInfo.ttlConfig ){
			std::string rowStr;
			configStream >> rowStr >> ttl.second;
			ttl.first = DoRows::fromStr ( rowStr );
		}
	}
	if (configStream.ver > Version ("2.6")){
		UINT numSettings;
		configStream >> numSettings;
		tmpInfo.aoConfig.resize (numSettings);
		for (auto& ao : tmpInfo.aoConfig){
			UINT dacID;
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
	servoList->insertRow (row);
	servoList->setItem (row, 0, new QTableWidgetItem ( si.servoName.c_str ()));
	servoList->setItem (row, 1, new QTableWidgetItem ( si.active? "Yes" : "No"));
	servoList->setItem (row, 2, new QTableWidgetItem ( cstr(ctp(si.setPoint), precision)));
	servoList->setItem (row, 3, new QTableWidgetItem (si.monitorOnly ? "--" : str (ctp(si.controlValue), precision).c_str()));
	//servoList->item (row, 3)->setBackground (Qt::black);
	servoList->item (row, 3)->setFlags (servoList->item (row, 3)->flags () ^ Qt::ItemIsEnabled);
	servoList->setItem (row, 4, new QTableWidgetItem (((si.changeInCtrl < 0 ? "" : "+") + str (si.changeInCtrl*100, precision)).c_str ()));
	//servoList->item(row, 4)->setBackground (Qt::black);
	servoList->item (row, 4)->setFlags (servoList->item (row, 4)->flags () ^ Qt::ItemIsEnabled);
	servoList->setItem (row, 5, new QTableWidgetItem (cstr (ctp(si.mostRecentResult), precision)));
	//servoList->item (row, 5)->setBackground (Qt::black);
	servoList->item (row, 5)->setFlags (servoList->item (row, 5)->flags () ^ Qt::ItemIsEnabled);
	//servoList->item (row, 5)->setFlags (servoList->item (row, 5)->flags () ^ Qt::ItemIsEditable);
	servoList->setItem (row, 6, new QTableWidgetItem (cstr (si.aiInChan, precision)));
	servoList->setItem (row, 7, new QTableWidgetItem ((si.monitorOnly ? "--" : str (si.aoControlChannel )).c_str()));
	servoList->setItem (row, 8, new QTableWidgetItem (servoTtlConfigToString (si.ttlConfig).c_str()));
	servoList->setItem (row, 9, new QTableWidgetItem (cstr (si.tolerance, precision)));
	servoList->setItem (row, 10, new QTableWidgetItem ((si.monitorOnly ? "--" : str (si.gain )).c_str()));
	servoList->setItem (row, 11, new QTableWidgetItem (cstr(si.monitorOnly ? "Yes" : "No") ));
	servoList->setItem (row, 12, new QTableWidgetItem (servoDacConfigToString(si.aoConfig).c_str()));
	servoList->setItem (row, 13, new QTableWidgetItem (cstr (si.avgNum, precision)));
	//servoList->setStyleSheet (servoList->styleSheet ());
}

std::string ServoManager::servoDacConfigToString (std::vector<std::pair<UINT, double>> aoConfig){
	std::string aoString;
	for (auto ao : aoConfig){
		aoString += "dac" + str (ao.first) + " " + str (ao.second, 4) + " ";
	}
	return aoString;
}


std::string ServoManager::servoTtlConfigToString (std::vector<std::pair<DoRows::which, UINT> > ttlConfig){
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

bool ServoManager::wantsCalAutoServo( ){
	return calAutoServoButton->isChecked( );
}

bool ServoManager::wantsExpAutoServo (){
	return expAutoServoButton->isChecked ();
}


void ServoManager::runAll( Communicator& comm){
	UINT count = 0;
	// made this asynchronous to facilitate updating gui while 
	for ( auto& servo : servos ){
		auto origColor = servoList->item (count, 0)->background ();
		servoList->item (count, 0)->setBackground (Qt::red);
		try{
			ServoManager::calibrate (servo, count);
		}
		catch (Error & e) {
			comm.sendError (e.trace ());
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
	UINT polyPower = 0;
	auto opt = getUnitsOption ();
	auto cc =  (opt == AiUnits::which::pdVolts ? std::vector<double> ({ 0,1 }) :
				opt == AiUnits::which::atomsPower ? AI_SYSTEM_CAL[si.aiInChan].atAtomsCalCoeff 
												  : AI_SYSTEM_CAL[si.aiInChan].atPdCalCoeff );
	for (auto coeff : cc){
		power += coeff * std::pow (volt, polyPower++);
	}
	return power;
}

void ServoManager::calibrate( servoInfo& s, UINT which ){
	if ( !s.active ){
		return;
	}
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
	Sleep (20); // give some time for the lasers to settle..
	UINT attemptLimit = 100;
	UINT count = 0;
	UINT aiNum = s.aiInChan;
	UINT aoNum = s.aoControlChannel;
	if ( s.monitorOnly ){	// handle "servos" which are only monitoring values, not trying to change them. 
		double avgVal = ai->getSingleChannelValue ( aiNum, s.avgNum );
		s.mostRecentResult = avgVal;
		double percentDif = ( sp - avgVal) / sp;
		if ( fabs ( percentDif )  < s.tolerance ) { /* Value looks good, nothing to report. */ }
		else
		{
			errBox ( s.servoName + " Monitor: Value has drifted out of tolerance!" );
		}
		// And the rest of the function is handling the servo part. 
		s.currentlyServoing = false;
		return;
	}
	s.controlValue = globals->getVariableValue (str (s.servoName + "__servo_value", 13, false, true));
	// start the dac where it was last.
	auto oldVal = s.controlValue;
	ao->setSingleDac (aoNum, s.controlValue, ttls->getCore (), { 0, ttls->getCurrentStatus () });
	while ( count++ < attemptLimit ){
		double avgVal = ai->getSingleChannelValue(aiNum, s.avgNum);
		s.mostRecentResult = avgVal;
		double percentDif = (sp - avgVal) / sp;
		if ( fabs(percentDif)  < s.tolerance ){
			// found a good value.
			break;
		}
		else{
			// modify dac value.
			s.controlValue = ao->getDacValue( aoNum );
			double diff = s.gain * percentDif > 0.05 ? 0.05 : s.gain * percentDif;
			s.controlValue += diff;
			try{
				ao->setSingleDac( aoNum, s.controlValue, ttls->getCore (), { 0, ttls->getCurrentStatus () });
			}
			catch ( Error& ){
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
				catch ( Error& ){
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
		globals->adjustVariableValue( str(s.servoName + "__servo_value",13, false, true), dacVal );
	}
	servoList->repaint ();
}

void ServoManager::setChangeVal(UINT which, double change){
	servoList->setItem (which, 4, new QTableWidgetItem(((change < 0 ? "" : "+") + str (change*100,5)).c_str()));
	servoList->item(which, 4)->setFlags(servoList->item (which, 4)->flags () ^ Qt::ItemIsEnabled);
}

void ServoManager::setControlDisplay (UINT which, double value ){
	servoList->item (which, 3)->setText (cstr (value,5));
}

void ServoManager::setResDisplay (UINT which, double value){
	servoList->item (which,5)->setText (cstr (value,5));
}


