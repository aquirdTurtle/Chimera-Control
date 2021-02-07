#include <stdafx.h>
#include <RealTimeDataAnalysis/QtPlotDesignerDlg.h>
#include <boost/lexical_cast.hpp>

QtPlotDesignerDlg::QtPlotDesignerDlg (unsigned pictureNumber) : picNumber (pictureNumber), currentPlotInfo (picNumber) {
	initializeWidgets ();
}

QtPlotDesignerDlg::QtPlotDesignerDlg (std::string fileName) : picNumber (PlottingInfo::getPicNumberFromFile (fileName)),
	currentPlotInfo (fileName) {
	initializeWidgets ();
}

void QtPlotDesignerDlg::initializeWidgets () {
	
	QPoint pos = { 0,0 };
	auto& px = pos.rx (), & py = pos.ry ();

	plotPropertiesText = new QLabel ("Plot Properties", this);
	plotPropertiesText->setGeometry (px, py, 480, 25);

	plotTitleText = new QLabel ("Plot Title", this);
	plotTitleText->setGeometry (px, py += 25, 240, 25);
	plotTitleEdit = new QLineEdit (qstr (currentPlotInfo.getTitle ()), this);
	plotTitleEdit->setGeometry (px+240, py, 240, 25);
	
	yLabelText = new QLabel ("Y-Axis Label", this);
	yLabelText->setGeometry (px, py += 25, 240, 25);
	yLabelEdit = new QLineEdit (qstr (currentPlotInfo.getYLabel ()), this);
	yLabelEdit->setGeometry (px + 240, py, 240, 25);

	plotFilenameText = new QLabel ("Plot Filename", this);
	plotFilenameText->setGeometry (px, py += 25, 240, 25);
	plotFilenameEdit = new QLineEdit (qstr (currentPlotInfo.getFileName ()), this);
	plotFilenameEdit->setGeometry (px + 240, py, 240, 25);

	generalPlotTypeText = new QLabel ("Plot Type", this);
	generalPlotTypeText->setGeometry (px, py += 25, 480, 25);
	generalPlotTypeCombo = new QComboBox (this);
	generalPlotTypeCombo->setGeometry (px, py += 25, 480, 25);
	generalPlotTypeCombo->addItems ({ "Pixel Count Histograms", "Pixel Counts", "Atoms" });
	generalPlotTypeCombo->setCurrentIndex (0);

	dataSetNumberText = new QLabel ("Data Set #", this);
	dataSetNumberText->setGeometry (px, py+=25, 240, 25);
	dataSetNumCombo = new QComboBox (this);
	dataSetNumCombo->setGeometry (px + 240, py, 240, 25);
	dataSetNumCombo->addItems ({ "Data Set #1", "Add New Data Set", "Remove Data Set" });
	connect ( dataSetNumCombo, qOverload<int> (&QComboBox::currentIndexChanged), 
			  [this]() {handleDataSetComboChange (); });

	plotThisDataBox = new QCheckBox ("Plot This Data", this);
	plotThisDataBox->setGeometry (px, py+=25, 480, 25);

	// Positive Result Conditions
	positiveResultConditionText = new QLabel ("Positive Result Condition", this);
	positiveResultConditionText->setGeometry (px, py += 25, 480, 25);

	prcPictureNumberText = new QLabel ("Picture Number", this);
	prcPictureNumberText->setGeometry (px, py += 25, 240, 25);

	prcPicNumCombo = new QComboBox (this);
	prcPicNumCombo->setGeometry (px + 240, py, 240, 25);
	for (auto num : range (picNumber)){
		prcPicNumCombo->addItem(qstr ("Picture #" + str (num + 1)));
	}
	connect (prcPicNumCombo, qOverload<int> (&QComboBox::currentIndexChanged), [this]() {handlePrcPictureNumberChange (); });

	prcPixelNumberText = new QLabel ("Pixel Number");
	prcPixelNumberText->setGeometry (px, py += 25, 240, 25);
	prcPixelNumCombo = new QComboBox (this);
	prcPixelNumCombo->setGeometry (px + 240, py, 240, 25);
	prcPixelNumCombo->addItem ("Pixel 1");
	connect (prcPixelNumCombo, qOverload<int> (&QComboBox::currentIndexChanged), [this]() {handlePrcPixelNumberChange (); });

	prcAtomBox = new QCheckBox ("Atom", this);
	prcAtomBox->setGeometry (px, py += 25, 240, 25);

	prcNoAtomBox = new QCheckBox ("No Atom", this);
	prcNoAtomBox->setGeometry (px + 240, py, 240, 25);
	prcShowAllButton = new QPushButton ("Show All", this);
	prcShowAllButton->setGeometry (px, py += 25, 480, 25);
	connect (prcShowAllButton, &QPushButton::pressed, [this]() { handlePrcShowAll (); });
	
	fitsText = new QLabel ("Fits", this);
	fitsText->setGeometry (px, py += 25, 480, 25);

	gaussianFit = new QCheckBox ("Gaussian", this);
	gaussianFit->setGeometry (px, py, 240, 25);

	lorentzianFit = new QCheckBox ("Lorentzian", this);
	lorentzianFit->setGeometry (px, py += 25, 240, 25);
	decayingSineFit = new QCheckBox ("Decaying Sine", this);
	decayingSineFit->setGeometry (px, py += 25, 240, 25);
	py -= 75;
	noFit = new QCheckBox ("None", this);
	noFit->setGeometry (px+240, py, 240, 25);
	noFit->setChecked (true);
	realTimeFit = new QCheckBox ("Real Time Fit", this);
	realTimeFit->setGeometry (px + 240, py += 25, 240, 25);

	atFinishFit = new QCheckBox ("At Finish", this);
	atFinishFit->setGeometry (px + 240, py += 25, 240, 25);
	setFitRadios ();
	px = 480;
	py = 0;

	analysisLocationsText = new QLabel ("Analysis Locations", this);
	analysisLocationsText->setGeometry (px, py, 480, 25);

	pixelsPerAnalysisGroupText = new QLabel ("Pixels Per Analysis Group", this);
	pixelsPerAnalysisGroupText->setGeometry (px, py += 25, 240, 25);
	pixelsPerAnalysisGroupEdit = new QLineEdit ("1", this);
	pixelsPerAnalysisGroupEdit->setGeometry (px + 240, py, 240, 25);
	connect (pixelsPerAnalysisGroupEdit, &QLineEdit::textChanged, [this]() {handlePixelEditChange (); });

	xAxisText = new QLabel ("X-Axis", this);
	xAxisText->setGeometry (px, py += 25, 480, 25);

	averageEachVariation = new QCheckBox ("Average Each Variation", this);
	averageEachVariation->setGeometry (px, py += 25, 480, 25);
	averageEachVariation->setChecked (1);

	runningAverage = new QCheckBox ("Running Average (Continuous Mode Only)", this);
	runningAverage->setGeometry (px, py += 25, 480, 25);

	aestheticsText = new QLabel ("Aesthetics", this);
	aestheticsText->setGeometry (px, py += 25, 480, 25);

	legendText = new QLabel ("Legend Text", this);
	legendText->setGeometry (px, py += 25, 240, 25);

	legendEdit = new QLineEdit (this);
	legendEdit->setGeometry (px + 240, py, 240, 25);

	binWidthText = new QLabel ("Hist Bin Width:", this);
	binWidthText->setGeometry (px, py += 25, 240, 25);
	binWidthEdit = new QLineEdit ("10", this);
	binWidthEdit->setGeometry (px + 240, py, 240, 25);

	// Post selection conditions
	postSelectionConditionText = new QLabel ("Post-Selection Conditions", this);
	postSelectionConditionText->setGeometry (px, py += 25, 480, 25);
	pscConditionNumberText = new QLabel ("Condition Number", this);
	pscConditionNumberText->setGeometry (px, py += 25, 240, 25);
	pscConditionNumCombo = new QComboBox (this);
	pscConditionNumCombo->setGeometry (px+240, py, 240, 25);
	pscConditionNumCombo->addItems ({ "Condition #1", "Add New Condition", "Remove Condition" });
	connect (pscConditionNumCombo, qOverload<int> (&QComboBox::currentIndexChanged), 
			 [this]() {handlePscConditionNumberChange (); });

	pscPictureNumberText = new QLabel ("Picture Number", this);
	pscPictureNumberText->setGeometry (px, py += 25, 240, 25);
	pscPicNumCombo = new QComboBox (this);
	pscPicNumCombo->setGeometry (px+240, py, 240, 25);
	for (auto num : range (picNumber)){
		pscPicNumCombo->addItem (qstr ("Picture #" + str (num + 1)));
	}
	connect (pscPicNumCombo, qOverload<int> (&QComboBox::currentIndexChanged),
		[this]() {handlePscPictureNumberChange (); });
	

	pscPixelNumberText = new QLabel ("Pixel Number", this);
	pscPixelNumberText->setGeometry (px, py += 25, 240, 25);
	pscPixelNumCombo = new QComboBox (this);
	pscPixelNumCombo->setGeometry (px+240, py, 240, 25);
	pscPixelNumCombo->addItem ("Pixel #1");
	connect (pscPixelNumCombo, qOverload<int> (&QComboBox::currentIndexChanged),
		[this]() { handlePscPixelNumberChange (); });

	

	pscAtomBox = new QCheckBox ("Atom", this);
	pscAtomBox->setGeometry (px, py += 25, 240, 25);
	pscNoAtomBox = new QCheckBox ("No Atom",this);
	pscNoAtomBox->setGeometry (px+240, py, 240, 25);
	pscShowAllButton = new QPushButton ("Show All", this);
	pscShowAllButton->setGeometry (px, py += 25, 480, 25);
	connect ( pscShowAllButton, &QPushButton::pressed, [this](){handlePscShowAll ();});

	saveButton = new QPushButton ("SAVE", this);
	saveButton->setGeometry (px, py += 25, 240, 25);
	connect (saveButton, &QPushButton::pressed, [this]() {handleSave (); });

	cancelButton = new QPushButton ("CANCEL", this);
	cancelButton->setGeometry (px+240, py, 240, 25);
	connect (cancelButton, &QPushButton::pressed, [this]() {handleCancel (); });
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePixelEditChange (){
	int pixelNum;
	auto txt = pixelsPerAnalysisGroupEdit->text ();
	try{
		pixelNum = boost::lexical_cast<long>(str (txt));
	}
	catch (boost::bad_lexical_cast&){
		errBox ("ERROR: pixels per analysis group text failed to convert to an integer!");
	}
	// change the pixel number in both of the edits.
	prcPixelNumCombo->clear ();
	pscPixelNumCombo->clear ();
	for (auto num : range (pixelNum)){
		std::string text ("Pixel #" + str (num + 1));
		prcPixelNumCombo->addItem (qstr (text));
		pscPixelNumCombo->addItem (qstr (text));
	}
	currentPlotInfo.resetPixelNumber (pixelNum);
}

void QtPlotDesignerDlg::handleSave () {
	/// Save Everything
	// General Parameters: ////////
	auto text = plotTitleEdit->text ();
	currentPlotInfo.changeTitle (str (text));
	text = yLabelEdit->text ();
	currentPlotInfo.changeYLabel (str (text));
	text = plotFilenameEdit->text ();
	currentPlotInfo.changeFileName (str (text));
	int runningAverageCheck = runningAverage->isChecked ();
	int variationAverageCheck = averageEachVariation->isChecked ();
	if (variationAverageCheck == BST_CHECKED) {
		if (runningAverageCheck == BST_CHECKED) {
			errBox ("Please select only one x-axis option.");
			return;
		}
		currentPlotInfo.changeXAxis ("Variation Average");
	}
	else if (runningAverageCheck == BST_CHECKED) {
		if (variationAverageCheck == BST_CHECKED) {
			errBox ("Please select only one x-axis option.");
			return;
		}
		currentPlotInfo.changeXAxis ("Running Average");
	}
	else {
		errBox ("Please select an x-axis option.");
		return;
	}
	/// get the (current) analysis pixel locations
	try {
		saveDataSet (false);
	}
	catch (ChimeraError & err) {
		errBox (err.trace ());
		return;
	}
	auto result = QMessageBox::question (nullptr, "All settings", qstr (currentPlotInfo.getAllSettingsString ()));
	if (result == QMessageBox::Yes) {
		try {
			currentPlotInfo.savePlotInfo ();
		}
		catch (ChimeraError & err) {
			errBox ("ERROR while saving plot info: " + err.trace ());
		}
		result = QMessageBox::question (nullptr, "Close?", "Close plot creator?");
		if (result == QMessageBox::Yes) {
			close ();
		}
	}
}

void QtPlotDesignerDlg::handleCancel (){
	close ();
}

void QtPlotDesignerDlg::setFitRadios () {
	int itemIndex = dataSetNumCombo->currentIndex ();
	if (itemIndex == -1 || itemIndex == 0) {
		return;
	}
	int fitCase = currentPlotInfo.getFitOption (unsigned (itemIndex - 1));
	gaussianFit->setChecked (fitCase == unsigned(FitType::GAUSSIAN_FIT));
	lorentzianFit->setChecked (fitCase == unsigned (FitType::LORENTZIAN_FIT));
	decayingSineFit->setChecked (fitCase == unsigned (FitType::SINE_FIT));
	int whenCase = currentPlotInfo.whenToFit (dataSetNumCombo->currentIndex ());
	noFit->setChecked (whenCase == unsigned (FitOption::NO_FIT));
	realTimeFit->setChecked (whenCase == unsigned (FitOption::REAL_TIME_FIT));
	atFinishFit->setChecked (whenCase == unsigned (FitOption::FIT_AT_END));
}

void QtPlotDesignerDlg::loadPositiveResultSettings (){
	if (prcPicNumCombo->currentIndex () >= 0 && prcPixelNumCombo->currentIndex () >= 0 && dataSetNumCombo->currentIndex () >= 0){
		// load current things.
		int currentValue = currentPlotInfo.getResultCondition (dataSetNumCombo->currentIndex (), prcPixelNumCombo->currentIndex (),
			prcPicNumCombo->currentIndex ());
		prcAtomBox->setChecked (currentValue == 1);
		prcNoAtomBox->setChecked (currentValue == -1);
	}
}

void QtPlotDesignerDlg::loadPostSelectionConditions (){
	if (pscConditionNumCombo->currentIndex () >= 0 && pscPicNumCombo->currentIndex () >= 0
		&& pscPixelNumCombo->currentIndex () >= 0){
		int currentValue = currentPlotInfo.getPostSelectionCondition (dataSetNumCombo->currentIndex (),
			pscConditionNumCombo->currentIndex (),
			pscPixelNumCombo->currentIndex (),
			pscPicNumCombo->currentIndex ());
		pscAtomBox->setChecked (currentValue == 1);
		pscNoAtomBox->setChecked (currentValue == -1);
	}
}

void QtPlotDesignerDlg::handleDataSetComboChange (){
	int itemIndex = dataSetNumCombo->currentIndex ();
	if (itemIndex == -1){
		return;
	}
	try	{
		saveDataSet (true);
	}
	catch (ChimeraError & err){
		errBox (err.trace ());
		return;
	}
	auto txt = dataSetNumCombo->itemText (itemIndex);
	std::string dataSetString (str(txt));
	if (dataSetString == "Add New Data Set"){
		dataSetNumCombo->setCurrentIndex (-1);
		int numberOfItems = dataSetNumCombo->count ();
		currentPlotInfo.addDataSet ();
		dataSetNumCombo->clear ();
		for (auto dsetInc : range (numberOfItems - 1)){
			dataSetNumCombo->addItem (qstr ("Data Set #" + str (dsetInc + 1)));
		}
		dataSetNumCombo->addItem("Add New Data Set");
		dataSetNumCombo->addItem ("Remove Data Set");
	}
	else if (dataSetString == "Remove Data Set"){
		int numberOfItems = dataSetNumCombo->count();
		if (numberOfItems < 4){
			errBox ("Can't delete last data set.");
			return;
		}
		// make data set struct smaller.
		currentPlotInfo.removeDataSet ();
		dataSetNumCombo->removeItem (numberOfItems - 3);
		currentDataSet = -1;
	}
	else {
		int numberOfItems = dataSetNumCombo->count();
		try {
			plotThisDataBox->setChecked (currentPlotInfo.getPlotThisDataValue (unsigned (itemIndex)));
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
			return;
		}
		if (currentPlotInfo.getPlotType () == "Atoms") {
			loadPositiveResultSettings ();
			loadPostSelectionConditions ();
		}
		else {
			unsigned pixel, picture;
			try	{
				currentPlotInfo.getDataCountsLocation (dataSetNumCombo->currentIndex (), pixel, picture);
				prcPicNumCombo->setCurrentIndex (picture);
				prcPixelNumCombo->setCurrentIndex (pixel);
				currentPrcPixel = pixel;
				currentPrcPicture = picture;
				prcAtomBox->setChecked (currentPlotInfo.getResultCondition (dataSetNumCombo->currentIndex (), pixel,
					picture) == 1);
				prcNoAtomBox->setChecked (0);
			}
			catch (ChimeraError&) {
				prcPicNumCombo->setCurrentIndex (-1);
				prcPixelNumCombo->setCurrentIndex (-1);
				currentPrcPixel = -1;
				currentPrcPicture = -1;
				prcAtomBox->setChecked (0);
				prcNoAtomBox->setChecked (0);
			}
			unsigned width = currentPlotInfo.getDataSetHistBinWidth (dataSetNumCombo->currentIndex ());
			binWidthEdit->setText(cstr (width));
		}
		currentDataSet = dataSetNumCombo->currentIndex ();
		if (dataSetNumCombo->currentIndex () != -1) {
			legendEdit->setText (cstr (currentPlotInfo.getLegendText (dataSetNumCombo->currentIndex ())));
		}
	}
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePrcPictureNumberChange (){
	if (prcPicNumCombo->currentIndex () == -1){
		return;
	}
	try	{
		// save before changing the number so that you save in the old config.
		savePositiveConditions (true);
		currentPrcPicture = prcPicNumCombo->currentIndex ();
		loadPositiveResultSettings ();
	}
	catch (ChimeraError & err){
		errBox (err.trace ());
	}
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePrcPixelNumberChange (){
	if (prcPixelNumCombo->currentIndex () == -1){
		return;
	}
	try	{
		savePositiveConditions (true);
		currentPrcPixel = prcPixelNumCombo->currentIndex ();
		loadPositiveResultSettings ();
	}
	catch (ChimeraError & err) {
		errBox (err.trace ());
	}
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePscConditionNumberChange (){
	if (pscConditionNumCombo->currentIndex () == -1){
		return;
	}
	try{
		savePostSelectionConditions (true);
		auto txt = pscConditionNumCombo->itemText (pscConditionNumCombo->currentIndex ());
		if (txt == "Add New Condition")	{
			pscConditionNumCombo->clear ();
			currentPscCondition = -1;
			unsigned currentConditionNumber = pscConditionNumCombo->count () - 2;
			// +1 for new condition
			for (auto num : range (currentConditionNumber + 1))	{
				pscConditionNumCombo->addItem (cstr ("Condition #" + str (num + 1)));
			}
			pscConditionNumCombo->addItem ("Add New Condition");
			pscConditionNumCombo->addItem ("Add Remove Condition");
			currentPlotInfo.addPostSelectionCondition ();
		}
		else if (txt == "Remove Condition")	{
			pscConditionNumCombo->clear ();
			currentPscCondition = -1;
			unsigned currentConditionNumber = pscConditionNumCombo->count () - 2;
			if (currentConditionNumber == 0){
				errBox ("No Condition to remove!");
			}
			// -1 for removed condition
			for (auto num : range (currentConditionNumber - 1)){
				pscConditionNumCombo->addItem (cstr ("Condition #" + str (num + 1)));
			}
			pscConditionNumCombo->addItem ("Add New Condition");
			pscConditionNumCombo->addItem ("Add Remove Condition");
			currentPlotInfo.removePostSelectionCondition ();
		}
		else{
			currentPscCondition = pscConditionNumCombo->currentIndex ();
			loadPostSelectionConditions ();
		}
	}
	catch (ChimeraError & err){
		errBox (err.trace ());
	}
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePscPictureNumberChange (){
	if (pscPicNumCombo->currentIndex () == -1){
		return;
	}
	try{
		savePostSelectionConditions (true);
		currentPscPicture = pscPicNumCombo->currentIndex ();
		loadPostSelectionConditions ();
	}
	catch (ChimeraError & err){
		errBox (err.trace ());
	}
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePscPixelNumberChange (){
	if (pscPixelNumCombo->currentIndex () == -1){
		return;
	}
	try{
		savePostSelectionConditions (true);
		currentPscPixel = pscPixelNumCombo->currentIndex ();
		loadPostSelectionConditions ();
	}
	catch (ChimeraError & err){
		errBox (err.trace ());
	}
	enableAndDisable ();
}

void QtPlotDesignerDlg::handlePrcShowAll (){
	savePositiveConditions (false);
	infoBox (currentPlotInfo.getPrcSettingsString ());
}

void QtPlotDesignerDlg::handlePscShowAll (){
	savePostSelectionConditions (false);
	infoBox (currentPlotInfo.getPscSettingsString ());
}

void QtPlotDesignerDlg::handleGeneralPlotTypeChange (){
	int itemIndex = generalPlotTypeCombo->currentIndex ();
	if (itemIndex == -1){
		return;
	}
	if (currentPlotInfo.getPlotType () == "Atoms"){
		saveDataSet (false);
	}
	auto txt = generalPlotTypeCombo->itemText(itemIndex);
	currentPlotInfo.changeGeneralPlotType (str(txt));
	enableAndDisable ();
}

void QtPlotDesignerDlg::savePositiveConditions (bool clear){
	// make sure that pictures and pixels are selected.
	if (currentPrcPicture >= 0 && currentPrcPixel >= 0 && dataSetNumCombo->currentIndex () >= 0){
		currentPlotInfo.setResultCondition (dataSetNumCombo->currentIndex (), currentPrcPixel, currentPrcPicture,
			prcAtomBox->isChecked () - prcNoAtomBox->isChecked ());
	}
	if (clear){
		prcAtomBox->setChecked (0);
		prcNoAtomBox->setChecked (0);
	}
}

void QtPlotDesignerDlg::savePostSelectionConditions (bool clear){
	if (dataSetNumCombo->currentIndex () >= 0 && currentPscCondition >= 0 && currentPscPixel >= 0 && currentPscPicture >= 0){
		currentPlotInfo.setPostSelCondition (dataSetNumCombo->currentIndex (), currentPscCondition, currentPscPixel,
			currentPscPicture, pscAtomBox->isChecked () - pscNoAtomBox->isChecked ());
	}
	if (clear){
		pscAtomBox->setChecked (0);
		pscNoAtomBox->setChecked (0);
	}
}

void QtPlotDesignerDlg::saveDataSet (bool clear){
	savePositiveConditions (clear);
	savePostSelectionConditions (clear);
	if (currentDataSet != -1){
		// legend
		auto txt = legendEdit->text ();
		currentPlotInfo.changeLegendText (currentDataSet, str (txt));
		txt = binWidthEdit->text ();
		unsigned width;
		try{
			width = boost::lexical_cast<double>(str (txt));
		}
		catch (boost::bad_lexical_cast&){
			throwNested ("ERROR: Failed to convert histogram bin width to an unsigned integer! width text was: " + str (txt));
		}
		currentPlotInfo.setDataSetHistBinWidth (currentDataSet, width);
		// fit options
		if (gaussianFit->isChecked ()){
			currentPlotInfo.setFitOption (currentDataSet, unsigned(FitType::GAUSSIAN_FIT));
		}
		else if (lorentzianFit->isChecked ()){
			currentPlotInfo.setFitOption (currentDataSet, unsigned (FitType::LORENTZIAN_FIT));
		}
		else if (decayingSineFit->isChecked ()){
			currentPlotInfo.setFitOption (currentDataSet, unsigned (FitType::SINE_FIT));
		}
		if (noFit->isChecked ()){
			currentPlotInfo.setWhenToFit (currentDataSet, unsigned(FitOption::NO_FIT));
		}
		else if (atFinishFit->isChecked ()){
			currentPlotInfo.setWhenToFit (currentDataSet, unsigned (FitOption::FIT_AT_END));
		}
		else if (realTimeFit->isChecked ()){
			currentPlotInfo.setWhenToFit (currentDataSet, unsigned (FitOption::REAL_TIME_FIT));
		}
	}
}

void QtPlotDesignerDlg::enableAndDisable (){
	dataSetNumCombo->setEnabled (true);
	pscShowAllButton->setEnabled (true);
	prcShowAllButton->setEnabled (true);
	saveButton->setEnabled (true);
	cancelButton->setEnabled (true);
	// each of the bools here determines whether a combo has a selection or not. 
	// There are simply a set of combos that must have a valid selection for a control it to be active.		
	bool dataSetSel = (dataSetNumCombo->currentIndex() != -1);
	gaussianFit->setEnabled (dataSetSel);
	lorentzianFit->setEnabled (dataSetSel);
	decayingSineFit->setEnabled (dataSetSel);
	noFit->setEnabled (dataSetSel);
	realTimeFit->setEnabled (dataSetSel);
	atFinishFit->setEnabled (dataSetSel);
	legendEdit->setEnabled (dataSetSel);
	prcPicNumCombo->setEnabled (dataSetSel);
	pscConditionNumCombo->setEnabled (dataSetSel);
	plotThisDataBox->setEnabled (dataSetSel);
	bool prcPictureNumSel = (prcPicNumCombo->currentIndex () != -1);
	prcPixelNumCombo->setEnabled (dataSetSel && prcPictureNumSel);
	bool pscConditionNumSel = (pscConditionNumCombo->currentIndex () != -1);
	pscPicNumCombo->setEnabled (dataSetSel && pscConditionNumSel);
	bool pscPictureNumSel = (pscPicNumCombo->currentIndex () != -1);
	pscPixelNumCombo->setEnabled (dataSetSel && pscConditionNumSel && pscPictureNumSel);
	bool pscPixelNumSel = (pscPixelNumCombo->currentIndex () != -1);
	pscAtomBox->setEnabled (dataSetSel && pscConditionNumSel && pscPictureNumSel && pscPixelNumSel);
	pscNoAtomBox->setEnabled (dataSetSel && pscConditionNumSel && pscPictureNumSel && pscPixelNumSel);
	bool prcPixelNumSel = (prcPixelNumCombo->currentIndex () != -1);
	prcAtomBox->setEnabled (dataSetSel && prcPictureNumSel && prcPixelNumSel); // && currentPlotInfo.getPlotType( ) == "Atoms" );
	prcNoAtomBox->setEnabled (dataSetSel && prcPictureNumSel && prcPixelNumSel); // && currentPlotInfo.getPlotType( ) == "Atoms" );
	binWidthEdit->setEnabled (currentPlotInfo.getPlotType () == "Pixel Count Histograms");
}
