// created by Mark O. Brown

#include "stdafx.h"
#include "PictureSettingsControl.h"

#include "Andor/AndorCameraCore.h"
#include "Andor/CameraSettingsControl.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "ConfigurationSystems/ProfileSystem.h"

#include "Commctrl.h"
#include <boost/lexical_cast.hpp>

void PictureSettingsControl::initialize( POINT& pos, IChimeraWindowWidget* parent )
{
	// introducing things row by row
	/// Set Picture Options
	UINT count = 0;
	auto handleChange = [this, parent]() {
		try {
			if (parent->andorWin) {
				parent->andorWin->handlePictureSettings ();
			}
		}
		catch (Error& err)
		{
			parent->reportErr (err.trace());
		}
	};
	/// Picture Numbers
	pictureLabel = new QLabel ("Picture #:", parent);
	pictureLabel->setGeometry (pos.x, pos.y, 100, 20);
	for ( auto picInc : range(4) ) {
		pictureNumbers[picInc] = new QLabel (cstr (picInc + 1), parent);
		pictureNumbers[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 95, 20);
	}
	pos.y += 20;
	/// Total picture number
	totalPicNumberLabel = new QLabel ("Total Picture #", parent);
	totalPicNumberLabel->setGeometry (pos.x, pos.y, 100, 20);
	for (auto picInc : range (4)) {
		totalNumberChoice[picInc] = new CQRadioButton ("", parent);
		totalNumberChoice[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 95, 20);
		totalNumberChoice[picInc]->setChecked (picInc == 0);
		parent->connect (totalNumberChoice[picInc], &QRadioButton::toggled, handleChange);
	}
	/// Exposure Times
	exposureLabel = new QLabel ("Exposure (ms):", parent);
	exposureLabel->setGeometry (pos.x, pos.y+=20, 100, 20);
	for ( auto picInc : range(4) ) {
		exposureEdits[picInc] = new CQLineEdit (parent);
		exposureEdits[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 95, 20);
		parent->connect (exposureEdits[picInc], &QLineEdit::textChanged, handleChange);
	}
	setUnofficialExposures ( std::vector<float> ( 4, 10 / 1000.0f ) );

	/// Thresholds
	thresholdLabel = new QLabel ("Threshold (cts)", parent);
	thresholdLabel->setGeometry (pos.x, pos.y += 20, 100, 20);
	for ( auto picInc : range(4) ) {
		thresholdEdits[picInc] = new CQLineEdit ("100", parent);
		thresholdEdits[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 95, 20);
		parent->connect (thresholdEdits[picInc], &QLineEdit::textChanged, handleChange);

		settings.thresholds[ picInc ] = { 100 };
	}
	/// colormaps
	colormapLabel = new QLabel ("Colormap", parent);
	colormapLabel->setGeometry (pos.x, pos.y += 20,100, 25);
	for ( auto picInc : range(4) )
	{
		colormapCombos[picInc] = new CQComboBox (parent);
		colormapCombos[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 95, 25);
		colormapCombos[picInc]->addItems ({ "Dark Viridis","Inferno","Black & White", "Red-Black-Blue" });
		parent->connect (colormapCombos[picInc], qOverload<int>(&QComboBox::currentIndexChanged), handleChange);

		colormapCombos[picInc]->setCurrentIndex( 0 );
		settings.colors[picInc] = 0;
	}
	/// display types
	displayTypeLabel = new QLabel ("Display-Type:", parent);
	displayTypeLabel->setGeometry (pos.x, pos.y += 25, 100, 25);
	for ( auto picInc : range ( 4 ) ) {
		displayTypeCombos[picInc] = new CQComboBox (parent);
		displayTypeCombos[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 95, 25);
		displayTypeCombos[picInc]->addItems ({"Normal","Dif: 1", "Dif: 2", "Dif: 3", "Dif: 4"});
		parent->connect (displayTypeCombos[picInc], qOverload<int> (&QComboBox::currentIndexChanged), handleChange);
		displayTypeCombos[ picInc ]->setCurrentIndex ( 0 );
		settings.colors[ picInc ] = 0;
	}
	/// software accumulation mode
	softwareAccumulationLabel = new QLabel ("Software Accum:", parent);
	softwareAccumulationLabel->setGeometry (pos.x, pos.y += 25, 100, 20);
	for ( auto picInc : range ( 4 ) ) {
		softwareAccumulateAll[picInc] = new CQCheckBox("All?", parent);
		softwareAccumulateAll[picInc]->setGeometry (pos.x + 100 + 95 * picInc, pos.y, 65, 20);
		softwareAccumulateAll[ picInc ]->setChecked( 0 );
		parent->connect (softwareAccumulateAll[picInc], &QCheckBox::stateChanged, handleChange);

		softwareAccumulateNum[picInc] = new CQLineEdit ("1", parent);
		softwareAccumulateNum[picInc]->setGeometry (pos.x + 165 + 95 * picInc, pos.y, 30, 20);
		parent->connect (softwareAccumulateNum[picInc], &QLineEdit::textChanged, handleChange);
	}
	//
	setPictureControlEnabled (0, true);
	setPictureControlEnabled (1, false);
	setPictureControlEnabled (2, false);
	setPictureControlEnabled (3, false);
}


std::array<displayTypeOption, 4> PictureSettingsControl::getDisplayTypeOptions( ){
	std::array<displayTypeOption, 4> options;
	UINT counter = 0;
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
		auto& picThresholds = settings.thresholds[ thresholdInc ];
		picThresholds.resize ( 1 );
		res[ thresholdInc ] = str(thresholdEdits[thresholdInc]->text ());
	}
	return res;
}

void PictureSettingsControl::handleSaveConfig(ConfigStream& saveFile){
	saveFile << "PICTURE_SETTINGS\n/*Color Options:*/ ";
	for (auto color : settings.colors){
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
	saveFile << "\nEND_PICTURE_SETTINGS\n";
}

andorPicSettingsGroup PictureSettingsControl::getPictureSettingsFromConfig (ConfigStream& configFile ){
	andorPicSettingsGroup fileSettings;
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
	return fileSettings;
}

void PictureSettingsControl::handleOpenConfig(ConfigStream& openFile, AndorCameraCore* andor){
	ProfileSystem::checkDelimiterLine(openFile, "PICTURE_SETTINGS");
	auto settings = getPictureSettingsFromConfig ( openFile );
	updateAllSettings ( settings );
	ProfileSystem::checkDelimiterLine(openFile, "END_PICTURE_SETTINGS");
}

void PictureSettingsControl::setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts ){
	for ( auto picInc : range ( 4 ) ) {
		softwareAccumulateAll[ picInc ]->setChecked ( opts[ picInc ].accumAll );
		softwareAccumulateNum[ picInc ]->setText ( cstr ( opts[ picInc ].accumNum ) );
	}
}

std::array<softwareAccumulationOption, 4> PictureSettingsControl::getSoftwareAccumulationOptions ( ){
	std::array<softwareAccumulationOption, 4> opts;
	for ( auto picInc : range(4)){
		opts[ picInc ].accumAll = softwareAccumulateAll[ picInc ]->isChecked ( );
		CString numTxt;
		try{
			opts[ picInc ].accumNum  = boost::lexical_cast<UINT>( str(softwareAccumulateNum[picInc]->text ()) );
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


UINT PictureSettingsControl::getPicsPerRepetition(){
	UINT which = 0, count=0;
	for ( auto& ctrl : totalNumberChoice ){
		count++;		
		which = ctrl->isChecked ( ) ? count : which;
	}
	if ( which == 0 ){
		thrower ( "ERROR: failed to get pics per repetition?!?" );
	}
	return which;
}


void PictureSettingsControl::setUnofficialPicsPerRep( UINT picNum ){
	if ( picNum < 1 || picNum > 4 ){
		thrower ( "Tried to set bad number of pics per rep: " + str ( picNum ) );
	}
	UINT count = 0;
	for (auto& totalNumRadio : totalNumberChoice){
		count++;
		totalNumRadio->setChecked (count == picNum);
		setPictureControlEnabled (count-1, count <= picNum);
	}
}


void PictureSettingsControl::handleOptionChange( ){
	for (auto radioInc : range(totalNumberChoice.size())){
		if (totalNumberChoice[radioInc]->isChecked()){
			setUnofficialPicsPerRep( radioInc + 1 );
		}
	}
}


std::array<float, 4> PictureSettingsControl::getExposureTimes ( )
{
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
		catch ( boost::bad_lexical_cast& )
		{
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
	for (UINT thresholdInc = 0; thresholdInc < newThresholds.size(); thresholdInc++){
		thresholdEdits[thresholdInc]->setText(newThresholds[thresholdInc].c_str());
	}
}


std::array<int, 4> PictureSettingsControl::getPictureColors(){
	updateSettings ( );
	return settings.colors;
}


void PictureSettingsControl::updateColormaps ( std::array<int, 4> colorIndexes )
{
	settings.colors = colorIndexes;
	for ( auto picInc : range(4) )	{
		colormapCombos[ picInc ]->setCurrentIndex ( settings.colors[ picInc ] );
	}
}


void PictureSettingsControl::setUnofficialExposures ( std::vector<float> times )
{
	UINT count = 0;
	for ( auto ti : times )
	{
		exposureEdits[ count++ ]->setText ( cstr ( ti*1e3 ) );
	}
}


void PictureSettingsControl::updateAllSettings ( andorPicSettingsGroup inputSettings ) {
	updateColormaps ( inputSettings.colors );
	setThresholds ( inputSettings.thresholdStrs );
	setSoftwareAccumulationOptions (inputSettings.saOpts);
}


/**/
std::array<std::vector<int>, 4> PictureSettingsControl::getThresholds ( )
{
	updateSettings ( );
	return settings.thresholds;
}

void PictureSettingsControl::updateSettings( )
{
	// grab the thresholds
	for (auto thresholdInc : range(4) ){
		if (!thresholdEdits[thresholdInc]) {
			return;
		}
		auto& picThresholds = settings.thresholds[ thresholdInc ];
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
		settings.colors[colorInc] = colormapCombos[colorInc]->currentIndex();
	}
}
