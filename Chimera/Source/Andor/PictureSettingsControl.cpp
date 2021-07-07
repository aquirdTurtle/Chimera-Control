// created by Mark O. Brown

#include "stdafx.h"
#include "PictureSettingsControl.h"

#include "Andor/AndorCameraCore.h"
#include "Andor/AndorCameraSettingsControl.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "ConfigurationSystems/ConfigSystem.h"

#include "Commctrl.h"
#include <boost/lexical_cast.hpp>

void PictureSettingsControl::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), &py = pos.ry ();
	// introducing things row by row
	/// Set Picture Options
	unsigned count = 0;
	auto handleChange = [this, parent]() {
		try {
			if (parent->andorWin) {
				parent->andorWin->handlePictureSettings ();
			}
		}
		catch (ChimeraError& err){
			parent->reportErr (err.qtrace());
		}
	};

	picScaleFactorLabel = new QLabel ("Pic. Scale Factor:", parent); 
	picScaleFactorLabel->setGeometry (px, py, 120, 20);

	picScaleFactorEdit  = new QLineEdit ("50", parent);
	picScaleFactorEdit->setGeometry (px + 120, py, 120, 20);
	parent->connect (picScaleFactorEdit, &QLineEdit::textChanged, handleChange);

	transformationModeCombo = new CQComboBox (parent);
	transformationModeCombo->setGeometry (px+240, py, 240, 20);
	transformationModeCombo->addItems ({ "Img Transformation: Fast", "Img Transformation: Smooth" });
	transformationModeCombo->setToolTip ( "This setting controls how Qt transforms the image as it places on the screen."
										  " Use \"Fast\" for larger images" );
	parent->connect (transformationModeCombo, qOverload<int> (&QComboBox::currentIndexChanged), 
		parent->andorWin, &QtAndorWindow::handleTransformationModeChange);

	/// Picture Numbers
	pictureLabel = new QLabel ("Picture #:", parent);
	pictureLabel->setGeometry (px, py += 20, 100, 20);
	for ( auto picInc : range(4) ) {
		pictureNumbers[picInc] = new QLabel (cstr (picInc + 1), parent);
		pictureNumbers[picInc]->setGeometry (px + 100 + 95 * picInc, py, 95, 20);
	}
	py += 20;
	/// Total picture number
	totalPicNumberLabel = new QLabel ("Total Pic. #", parent);
	totalPicNumberLabel->setGeometry (px, py, 100, 20);
	for (auto picInc : range (4)) {
		totalNumberChoice[picInc] = new CQRadioButton ("", parent);
		totalNumberChoice[picInc]->setGeometry (px + 100 + 95 * picInc, py, 95, 20);
		totalNumberChoice[picInc]->setChecked (picInc == 0);
		parent->connect (totalNumberChoice[picInc], &QRadioButton::toggled, handleChange);
	}
	/// Exposure Times
	exposureLabel = new QLabel ("Exposure(ms):", parent);
	exposureLabel->setGeometry (px, py+=20, 100, 20);
	for ( auto picInc : range(4) ) {
		exposureEdits[picInc] = new CQLineEdit (parent);
		exposureEdits[picInc]->setGeometry (px + 100 + 95 * picInc, py, 95, 20);
		parent->connect (exposureEdits[picInc], &QLineEdit::textChanged, handleChange);
	}
	setUnofficialExposures ( std::vector<float> ( 4, 10 / 1000.0f ) );

	/// Thresholds
	thresholdLabel = new QLabel ("Threshold:", parent);
	thresholdLabel->setGeometry (px, py += 20, 100, 20);
	for ( auto picInc : range(4) ) {
		thresholdEdits[picInc] = new CQLineEdit ("100", parent);
		thresholdEdits[picInc]->setGeometry (px + 100 + 95 * picInc, py, 95, 20);
		parent->connect (thresholdEdits[picInc], &QLineEdit::textChanged, handleChange);
		currentPicSettings.thresholds[ picInc ] = { 100 };
	}

	/// colormaps
	colormapLabel = new QLabel ("Colormap", parent);
	colormapLabel->setGeometry (px, py += 20,100, 25);
	for ( auto picInc : range(4) ){
		colormapCombos[picInc] = new CQComboBox (parent);
		colormapCombos[picInc]->setGeometry (px + 100 + 95 * picInc, py, 95, 25);
		colormapCombos[picInc]->addItems ({ "Dark Viridis","Inferno","Black & White", "Red-Black-Blue" });
		parent->connect (colormapCombos[picInc], qOverload<int>(&QComboBox::activated), handleChange);

		colormapCombos[picInc]->setCurrentIndex( 0 );
		currentPicSettings.colors[picInc] = 0;
	}
	/// display types
	displayTypeLabel = new QLabel ("Display-Type:", parent);
	displayTypeLabel->setGeometry (px, py += 25, 100, 25);
	for ( auto picInc : range ( 4 ) ) {
		displayTypeCombos[picInc] = new CQComboBox (parent);
		displayTypeCombos[picInc]->setGeometry (px + 100 + 95 * picInc, py, 95, 25);
		displayTypeCombos[picInc]->addItems ({"Normal","Dif: 1", "Dif: 2", "Dif: 3", "Dif: 4"});
		parent->connect (displayTypeCombos[picInc], qOverload<int> (&QComboBox::activated), handleChange);
		displayTypeCombos[ picInc ]->setCurrentIndex ( 0 );
		currentPicSettings.colors[ picInc ] = 0;
	}
	/// software accumulation mode
	softwareAccumulationLabel = new QLabel ("Soft. Accum:", parent);
	softwareAccumulationLabel->setToolTip ("Software Accumulation averages the images in real time. Underlying data in"
										   " the hdf5 file is unchanged.");
	softwareAccumulationLabel->setGeometry (px, py += 25, 100, 20);
	for ( auto picInc : range ( 4 ) ) {
		softwareAccumulateAll[picInc] = new CQCheckBox("All?", parent);
		softwareAccumulateAll[picInc]->setGeometry (px + 100 + 95 * picInc, py, 65, 20);
		softwareAccumulateAll[ picInc ]->setChecked( 0 );
		parent->connect (softwareAccumulateAll[picInc], &QCheckBox::stateChanged, handleChange);

		softwareAccumulateNum[picInc] = new CQLineEdit ("1", parent);
		softwareAccumulateNum[picInc]->setGeometry (px + 165 + 95 * picInc, py, 30, 20);
		parent->connect (softwareAccumulateNum[picInc], &QLineEdit::textChanged, handleChange);
	}
	py += 25;
	setPictureControlEnabled (0, true);
	setPictureControlEnabled (1, false);
	setPictureControlEnabled (2, false);
	setPictureControlEnabled (3, false);
}

std::array<displayTypeOption, 4> PictureSettingsControl::getDisplayTypeOptions( ){
	std::array<displayTypeOption, 4> options;
	unsigned counter = 0;
	for ( auto& combo : displayTypeCombos ){
		auto sel = combo->currentIndex( );
		if ( sel < 0 || sel > 4 ){
			thrower ( "Invalid selection in display type combo???" );
		}
		options[counter].isDiff = sel != 0;
		options[counter].whichPicForDif = sel;
		counter++;
	}
	return options;
}


std::array<std::string, 4> PictureSettingsControl::getThresholdStrings(){
	std::array<std::string, 4> res;
	// grab the thresholds
	for ( int thresholdInc = 0; thresholdInc < 4; thresholdInc++ ){
		auto& picThresholds = currentPicSettings.thresholds[ thresholdInc ];
		picThresholds.resize ( 1 );
		res[ thresholdInc ] = str(thresholdEdits[thresholdInc]->text ());
	}
	return res;
}

void PictureSettingsControl::handleSaveConfig(ConfigStream& saveFile){
	saveFile << "PICTURE_SETTINGS\n";
	saveFile << "/*Transformation Mode:*/ " << str (transformationModeCombo->currentIndex () == 0 ? "Fast" : "Smooth");
	saveFile << "\n/*Color Options:*/ ";
	for (auto color : currentPicSettings.colors){
		saveFile << color << " ";
	}
	saveFile << "\n/*Threshold Settings:*/ ";
	for (auto threshold : getThresholdStrings() ){
		saveFile << threshold << " ";
	}
	saveFile << "\n/*Software Accumulation (accum all / Number)*/ ";
	for ( auto saOpt : getSoftwareAccumulationOptions ( ) ){
		saveFile << saOpt.accumAll << " " << saOpt.accumNum << " ";
	}
	saveFile << "\n/*Pic Scale Factor:*/\t" << str(picScaleFactorEdit->text());
	saveFile << "\nEND_PICTURE_SETTINGS\n";
}

andorPicSettingsGroup PictureSettingsControl::getPictureSettingsFromConfig (ConfigStream& configFile ){
	andorPicSettingsGroup fileSettings;
	if (configFile.ver >= Version ("5.5")) {
		std::string transformationMode;
		configFile >> fileSettings.tMode;
	}
	else {
		fileSettings.tMode = "Fast";
	}
	if ( configFile.ver <= Version ( "4.7" ) ){
		int oldPicsPerRepTrash = 0;
		configFile >> oldPicsPerRepTrash;
	}
	for ( auto& color : fileSettings.colors ){
		configFile >> color;
	}
	if (configFile.ver <= Version ( "4.7" ) ){
		std::vector<float> oldExposureTimeTrash(4);
		for ( auto& exposure : oldExposureTimeTrash ){
			configFile >> exposure;
		}
	}
	for ( auto& threshold : fileSettings.thresholdStrs ){
		configFile >> threshold;
	}
	if (configFile.ver > Version ( "4.3" ) ){
		for ( auto& opt : fileSettings.saOpts ){
			configFile >> opt.accumAll >> opt.accumNum;
		}
	}
	if (configFile.ver >= Version ("5.12")) {
		configFile >> fileSettings.picScaleFactor;
	}
	return fileSettings;
}

void PictureSettingsControl::handleOpenConfig(ConfigStream& openFile, AndorCameraCore* andor){
	ConfigSystem::checkDelimiterLine(openFile, "PICTURE_SETTINGS");
	auto settings = getPictureSettingsFromConfig ( openFile );
	updateAllSettings ( settings );
	ConfigSystem::checkDelimiterLine(openFile, "END_PICTURE_SETTINGS");
}

void PictureSettingsControl::setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts ){
	for ( auto picInc : range ( 4 ) ) {
		softwareAccumulateAll[ picInc ]->setChecked ( opts[ picInc ].accumAll );
		softwareAccumulateNum[ picInc ]->setText ( cstr ( opts[ picInc ].accumNum ) );
	}
}

std::array<softwareAccumulationOption, 4> PictureSettingsControl::getSoftwareAccumulationOptions ( ){
	std::array<softwareAccumulationOption, 4> opts;
	if (softwareAccumulateAll[0] == nullptr) {
		return opts;
	}
	for ( auto picInc : range(4)){
		opts[ picInc ].accumAll = softwareAccumulateAll[ picInc ]->isChecked ( );
		
		try{
			opts[ picInc ].accumNum  = boost::lexical_cast<unsigned>( str(softwareAccumulateNum[picInc]->text ()) );
		}
		catch ( boost::bad_lexical_cast& ){
			thrower ( "Failed to convert software accumulation number to an unsigned integer!" );
		}
	}
	return opts;
}

void PictureSettingsControl::setPictureControlEnabled (int pic, bool enabled){
	if (pic > 3){
		return;
	}
	if (!exposureEdits[pic] || !thresholdEdits[pic] || !colormapCombos[pic] || !displayTypeCombos[pic] 
		|| !softwareAccumulateAll[pic] || !softwareAccumulateNum[pic]) {
		return;
	}
	exposureEdits[pic]->setEnabled(enabled);
	thresholdEdits[pic]->setEnabled (enabled);
	colormapCombos[pic]->setEnabled (enabled);
	displayTypeCombos[pic]->setEnabled (enabled);
	softwareAccumulateAll[pic]->setEnabled (enabled);
	softwareAccumulateNum[pic]->setEnabled (enabled);
}


unsigned PictureSettingsControl::getPicsPerRepetition(){
	unsigned which = 0, count=0;
	for ( auto& ctrl : totalNumberChoice ){
		count++;		
		which = ctrl->isChecked ( ) ? count : which;
	}
	if ( which == 0 ){
		thrower ( "ERROR: failed to get pics per repetition?!?" );
	}
	return which;
}


void PictureSettingsControl::setUnofficialPicsPerRep( unsigned picNum ){
	if ( picNum < 1 || picNum > 4 ){
		thrower ( "Tried to set bad number of pics per rep: " + str ( picNum ) );
	}
	unofficialPicsPerRep = picNum;
	unsigned count = 0;
	for (auto& totalNumRadio : totalNumberChoice){
		count++;
		totalNumRadio->setChecked (count == unofficialPicsPerRep);
		setPictureControlEnabled (count-1, count <= unofficialPicsPerRep);
	}
}


void PictureSettingsControl::handleOptionChange( ){
	for (auto radioInc : range(totalNumberChoice.size())){
		if (totalNumberChoice[radioInc]->isChecked()){
			setUnofficialPicsPerRep( radioInc + 1 );
		}
	}
}


std::array<float, 4> PictureSettingsControl::getExposureTimes ( ){
	std::array<float, 4> times;
	for ( auto ctrlNum : range(exposureEdits.size()) ){
		auto& ctrl = exposureEdits[ ctrlNum ];
		if (!ctrl) {
			return times;
		}
		try	{
			times[ctrlNum] = ctrl->text ().toDouble ()*1e-3;
			//times[ ctrlNum ] = boost::lexical_cast<double>( str(ctrl->text ()) ) * 1e-3;
		}
		catch ( boost::bad_lexical_cast& ){
			thrower ( "Failed to convert exposure time to a float!" );
		}
	}
	return times;
}

std::vector<float> PictureSettingsControl::getUsedExposureTimes() {
	updateSettings( );
	auto allTimes = getExposureTimes();
	std::vector<float> usedTimes(std::begin(allTimes), std::end(allTimes));
	usedTimes.resize ( getPicsPerRepetition ( ));
	return usedTimes;
}

void PictureSettingsControl::setThresholds( std::array<std::string, 4> newThresholds){
	for (unsigned thresholdInc = 0; thresholdInc < newThresholds.size(); thresholdInc++){
		thresholdEdits[thresholdInc]->setText(newThresholds[thresholdInc].c_str());
	}
}

std::array<int, 4> PictureSettingsControl::getPictureColors(){
	updateSettings ( );
	return currentPicSettings.colors;
}

void PictureSettingsControl::updateColormaps ( std::array<int, 4> colorIndexes ){
	currentPicSettings.colors = colorIndexes;
	for ( auto picInc : range(4) ){
		colormapCombos[ picInc ]->setCurrentIndex ( currentPicSettings.colors[ picInc ] );
	}
}

void PictureSettingsControl::setUnofficialExposures ( std::vector<float> times ){
	unsigned count = 0;
	for ( auto ti : times ){
		exposureEdits[ count++ ]->setText ( cstr ( ti*1e3,5) );
	}
}

void PictureSettingsControl::updateAllSettings ( andorPicSettingsGroup inputSettings ) {
	updateColormaps ( inputSettings.colors );
	setThresholds ( inputSettings.thresholdStrs );
	setSoftwareAccumulationOptions ( inputSettings.saOpts );
	if (inputSettings.tMode == "Fast") {
		transformationModeCombo->setCurrentIndex (0);
	}
	else {
		transformationModeCombo->setCurrentIndex (1);
	}
	picScaleFactorEdit->setText (qstr (inputSettings.picScaleFactor));
}

std::array<std::vector<int>, 4> PictureSettingsControl::getThresholds ( ){
	updateSettings ( );
	return currentPicSettings.thresholds;
}

void PictureSettingsControl::updateSettings( ){
	// grab the thresholds
	for (auto thresholdInc : range(4) ){
		if (!thresholdEdits[thresholdInc]) {
			return;
		}
		auto& picThresholds = currentPicSettings.thresholds[ thresholdInc ];
		picThresholds.resize ( 1 );
		int threshold;
		
		try{
			QString txt = thresholdEdits[thresholdInc]->text ();						
			threshold = txt.toInt ();
			picThresholds[0] = threshold;
		}
		catch ( boost::bad_lexical_cast& ){
			picThresholds.clear ( );
			// assume it's a file location.
			std::ifstream thresholdFile;
			thresholdFile.open ( str(thresholdEdits[thresholdInc]->text ()).c_str() );
			if ( !thresholdFile.is_open ( ) ){
				thrower  ( "ERROR: failed to convert threshold number " + str ( thresholdInc + 1 ) + " to an integer, "
						 "and it wasn't the address of a threshold-file." );  
			}
			while ( true ){
				double indv_file_threshold;
				thresholdFile >> indv_file_threshold;
				if ( thresholdFile.eof ( ) ) { break; }
				picThresholds.push_back ( indv_file_threshold );
			}
		}
	}
	for (auto colorInc : range (4)) {
		currentPicSettings.colors[colorInc] = colormapCombos[colorInc]->currentIndex();
	}
}

Qt::TransformationMode PictureSettingsControl::getTransformationMode (){
	if (transformationModeCombo->currentText () == "Fast") {
		return Qt::TransformationMode::FastTransformation;
	}
	else {
		return Qt::TransformationMode::SmoothTransformation;
	}
}

void PictureSettingsControl::setEnabledStatus (bool viewRunningSettings) {
	if (viewRunningSettings) {
		for (auto num : range (4)) {
			totalNumberChoice[num]->setEnabled (false);
			exposureEdits[num]->setEnabled (false);
			thresholdEdits[num]->setEnabled (false);
			colormapCombos[num]->setEnabled (false);
			displayTypeCombos[num]->setEnabled (false);
			softwareAccumulateAll[num]->setEnabled (false);
			softwareAccumulateNum[num]->setEnabled (false);
		}
	}
	else {
		for (auto num : range (4)) {
			totalNumberChoice[num]->setEnabled (true);
		}
		setUnofficialPicsPerRep (unofficialPicsPerRep);
	}
}

int PictureSettingsControl::getPicScaleFactor () {
	try {
		return boost::lexical_cast<int>(str (picScaleFactorEdit->text()));
	}
	catch (boost::bad_lexical_cast & err) {
		thrower ("Failed to convert picture scale factor to integer!");
	}
}

