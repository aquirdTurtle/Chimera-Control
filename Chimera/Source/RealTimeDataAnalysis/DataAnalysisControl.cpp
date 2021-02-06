// created by Mark O. Brown
#include "stdafx.h"

#include "RealTimeDataAnalysis/DataAnalysisControl.h"
#include "Control.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "RealTimeDataAnalysis/QtPlotDesignerDlg.h"
#include "RealTimeDataAnalysis/realTimePlotterInput.h"
#include "GeneralUtilityFunctions/range.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <numeric>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <qheaderview.h>
#include <qmenu.h>

DataAnalysisControl::DataAnalysisControl (IChimeraQtWindow* parent) : IChimeraSystem(parent) {
	std::vector<std::string> names = ConfigSystem::searchForFiles (PLOT_FILES_SAVE_LOCATION, str ("*.") + PLOTTING_EXTENSION);
	for (auto name : names) {
		PlottingInfo totalInfo (PLOT_FILES_SAVE_LOCATION + "\\" + name + "." + PLOTTING_EXTENSION); 
		tinyPlotInfo info;
		info.name = name;
		info.isHist = (totalInfo.getPlotType () == "Pixel Count Histograms");
		info.numPics = totalInfo.getPicNumber ();
		allTinyPlots.push_back (info);
	}
	currentSettings.grids.resize( 1 );
}

void DataAnalysisControl::handleContextMenu (const QPoint& pos) {
	QTableWidgetItem* item = plotListview->itemAt (pos);
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete Plot", plotListview);
	plotListview->connect (deleteAction, &QAction::triggered, [this, item]() {
		auto name = plotListview->item (item->row (), 0)->text();
		tinyPlotInfo selectedInfo;
		unsigned selectedIndex = 0;
		for (auto& pltInfo : allTinyPlots) {
			if (pltInfo.name == str(name)) {
				selectedInfo = pltInfo;
				break;
			}
			selectedIndex++;
		}
		auto answer = QMessageBox::question ( plotListview, qstr ("Delete Plot?"),
											  qstr("Delete Plot " + selectedInfo.name + "?"));
		if (answer == QMessageBox::Yes){
			int result = DeleteFile (cstr (PLOT_FILES_SAVE_LOCATION + "\\" + selectedInfo.name + "." 
										   + PLOTTING_EXTENSION ));
			if (!result){
				errBox ("Failed to delete script file! Error code: " + str (GetLastError ()));
				return;
			}
			allTinyPlots.erase (allTinyPlots.begin () + selectedIndex);
			reloadListView ();
		}});
	auto* detailsAction = new QAction ("View Plot Details", plotListview);
	plotListview->connect (detailsAction, &QAction::triggered, [this, item]() {
		try {
			auto name = plotListview->item (item->row (), 0)->text ();
			tinyPlotInfo selectedInfo;
			unsigned selectedIndex = 0;
			for (auto& pltInfo : allTinyPlots) {
				if (pltInfo.name == str (name)) {
					selectedInfo = pltInfo;
					break;
				}
				selectedIndex++;
			}
			infoBox (PlottingInfo::getAllSettingsStringFromFile (
				PLOT_FILES_SAVE_LOCATION + "\\" + selectedInfo.name + "." + PLOTTING_EXTENSION));
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
		}});
	menu.addAction (detailsAction);
	auto* editAction = new QAction ("Edit Plot", plotListview);
	plotListview->connect (editAction, &QAction::triggered, [this, item]() {
		try {
			// edit existing plot file using the plot designer.
			QtPlotDesignerDlg* dialog = new QtPlotDesignerDlg (PLOT_FILES_SAVE_LOCATION + "\\" + str (item->text ()) + "." + PLOTTING_EXTENSION);
			dialog->setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
			dialog->exec ();
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
		}});
	menu.addAction (editAction);

	auto* newPerson = new QAction ("New Plot", plotListview);
	plotListview->connect (newPerson, &QAction::triggered, [this]() {
		QtPlotDesignerDlg* dialog = new QtPlotDesignerDlg (unofficialPicsPerRep);
		dialog->setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
		dialog->exec ();
		//allTinyPlots.push_back (newPlot);
		reloadListView ();
		});
	if (item) { menu.addAction (deleteAction); }
	menu.addAction (newPerson);
	menu.exec (plotListview->mapToGlobal (pos));
}

void DataAnalysisControl::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();

	header = new QLabel ("DATA ANALYSIS", parent);
	header->setGeometry (px, py, 480, 25);

	currentDataSetNumberText = new QLabel ("Data Set #:", parent);
	currentDataSetNumberText->setGeometry (px, py += 25, 350, 50);
	currentDataSetNumberDisp = new QLabel ("?", parent);
	currentDataSetNumberDisp->setGeometry (px + 350, py, 130, 50);
	currentDataSetNumberDisp->setStyleSheet("QLabel { font: bold 24pt; };");

	gridSelector = new CQComboBox (parent);
	gridSelector->setGeometry (px, py += 50, 50, 25);
	parent->connect (gridSelector, qOverload<int>(&QComboBox::currentIndexChanged), 
		[this, parent]() {
			try{
				handleAtomGridCombo ();
			}
			catch (ChimeraError& err){
				parent->reportErr (err.qtrace ());
			}
		});
	gridSelector->addItem ("0");
	gridSelector->addItem ("New");
	gridSelector->setCurrentIndex( 0 );	

	deleteGrid = new CQPushButton ("Del", parent);
	deleteGrid->setGeometry (px + 50, py, 50, 25);
	parent->connect (deleteGrid, &QPushButton::released, [this, parent]() {
			try{
				handleDeleteGrid ();
			}
			catch (ChimeraError& err){
				parent->reportErr (err.qtrace ());
			}
		});
	tlRowLabel = new QLabel("Row:", parent);
	tlRowLabel->setToolTip ("The row of the top-left pixel in the atom grid. (0 indexed)");
	tlRowLabel->setGeometry (px + 100, py, 50, 25);
	tlRowEdit = new CQLineEdit("0", parent);
	tlRowEdit->setGeometry (px + 150, py, 50, 25);
	tlColLabel = new QLabel ("Col:", parent);
	tlColLabel->setToolTip ("The Column of the top-left pixel in the atom grid. (0 indexed)");
	tlColLabel->setGeometry (px + 200, py, 50, 25);
	tlColEdit = new CQLineEdit ("0", parent);
	tlColEdit->setGeometry (px + 250, py, 50, 25);

	gridSpacingText = new QLabel ("Spacing", parent);
	gridSpacingText->setGeometry (px+300, py, 60, 25);

	gridSpacing = new CQLineEdit ("0", parent);
	gridSpacing->setGeometry (px + 360, py, 30, 25);

	gridWidthText = new QLabel ("Width", parent);
	gridWidthText->setGeometry (px + 390, py, 60, 25);

	gridWidth = new CQLineEdit ("0", parent);
	gridWidth->setGeometry (px + 450, py, 30, 25);

	gridHeightText = new QLabel ("Height", parent);
	gridHeightText->setGeometry (px, py+=25, 80, 25);

	gridHeight = new CQLineEdit ("0", parent);
	gridHeight->setGeometry (px + 80, py, 40, 25);
	// 
	displayGridBtn = new CQCheckBox ("Display Grid?", parent);
	displayGridBtn->setGeometry (px+120, py, 120, 25);

	/// PLOTTING FREQUENCY CONTROLS
	updateFrequencyLabel1 = new QLabel ("Update plots every", parent);
	updateFrequencyLabel1->setGeometry (px + 240, py, 140, 25);

	updateFrequencyEdit = new CQLineEdit ("5", parent);
	updateFrequencyEdit->setGeometry (px + 390, py, 30, 25);
	updateFrequency = 5;
	
	updateFrequencyLabel2 = new QLabel (") reps.", parent);
	updateFrequencyLabel2->setGeometry (px + 420, py, 60, 25);

	plotTimerTxt = new QLabel ("Plot Update Timer (ms):", parent);
	plotTimerTxt->setGeometry (px, py += 25, 180, 25);
	
	plotTimerEdit = new CQLineEdit ("5000", parent);
	plotTimerEdit->setGeometry (px + 180, py, 60, 25);
	parent->connect (plotTimerEdit, &QLineEdit::textChanged, [this]() { updatePlotTime (); });

	autoThresholdAnalysisButton = new CQCheckBox ("Auto Threshold Analysis", parent);
	autoThresholdAnalysisButton->setGeometry (px + 240, py, 240, 25);
	autoThresholdAnalysisButton->setToolTip ("At the end of an experiment, run some python code which will fit the "
		"data and determine good thresholds which can be outputted to a file to "
		"keep the thresholds used by the real-time analysis up-to-date.");

	
	autoBumpAnalysis = new CQCheckBox ("Auto Bump Analysis", parent);
	autoBumpAnalysis->setGeometry (px, py += 25, 180, 25);
	autoBumpAnalysis->setToolTip ("At the end of the experiment, run some python code which will do standard data "
		"analysis on the resulting data set and fit a bump to it. The bump center value will be written to a file.");
	doBumpAnalysis = new CQPushButton ("Analyze Now", parent);
	doBumpAnalysis->setGeometry (px + 180, py, 180, 25);
	parent->connect (doBumpAnalysis, &QPushButton::released, [parent]() {
		parent->andorWin->handleBumpAnalysis (parent->mainWin->getProfileSettings());
		});
	bumpEditParam = new CQLineEdit (parent);
	bumpEditParam->setGeometry (px + 360, py, 120, 25);

	/// Initialize the listview
	plotListview = new QTableWidget (parent);
	plotListview->setGeometry (px, py+=25, 480, 120);
	py += 120;
	QStringList labels;
	labels << " Name " << " Grid # " << " Active ";
	plotListview->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (plotListview, &QTableWidget::customContextMenuRequested,
					 [this](const QPoint& pos) { handleContextMenu (pos); });
	plotListview->setColumnCount (labels.size ());
	plotListview->setHorizontalHeaderLabels (labels);
	plotListview->horizontalHeader ()->setFixedHeight (25);
	plotListview->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
	plotListview->resizeColumnsToContents ();
	plotListview->verticalHeader ()->setSectionResizeMode (QHeaderView::Fixed);
	plotListview->verticalHeader ()->setDefaultSectionSize (22);
	plotListview->verticalHeader ()->setFixedWidth (40);
	parent->connect (plotListview, &QTableWidget::cellChanged, [this, parent](int row, int col) {
			if (col == 1) {
				auto* item = plotListview->item (row, col);
				try {
					allTinyPlots[row].whichGrid = boost::lexical_cast<unsigned>( cstr (item->text ()));
				}
				catch (ChimeraError&) {}
			}
		});
	parent->connect (plotListview, &QTableWidget::cellDoubleClicked, 
		[this, parent](int clRow, int clCol) {
			parent->configUpdated ();
			if (clCol == 2) {
				auto* item = plotListview->item (clRow, 0);
				unsigned counter = 0;
				for (auto& pltInfo : allTinyPlots) {
					if (pltInfo.name == str(item->text ())) {
						break;
					}
					counter++;
				}
				tinyPlotInfo& info = allTinyPlots[counter];
				info.isActive = !info.isActive;
				auto* newItem = new QTableWidgetItem (info.isActive ? "YES" : "NO");
				newItem->setFlags (newItem->flags () & ~Qt::ItemIsEditable);
				plotListview->setItem (clRow, clCol, newItem);
			}
		});
	reloadListView();
}

void DataAnalysisControl::updatePlotTime ( ){
	try	{
		plotTime = boost::lexical_cast<unsigned long>(str(plotTimerEdit->text()));
	}
	catch ( boost::bad_lexical_cast& ){
		//throwNested ( "ERROR: plot time failed to convert to an unsigned integer!" );
	}
}

std::atomic<unsigned>& DataAnalysisControl::getPlotTime( ){
	return plotTime;
}

void DataAnalysisControl::handleDeleteGrid( ){
	if (currentSettings.grids.size() == 1 ){
		thrower ( "ERROR: You are not allowed to delete the last grid for data analysis!" );
	}
	currentSettings.grids.erase(currentSettings.grids.begin( ) + 0 );
	gridSelector->clear ();
	unsigned count = 0;
	for ( auto grid : currentSettings.grids ){
		std::string txt( str( count++ ) );
		gridSelector->addItem( cstr( txt ) );
	}
	gridSelector->addItem( "New" );
	gridSelector->setCurrentIndex( 0 );
	loadGridParams(currentSettings.grids[0] );
}

unsigned DataAnalysisControl::getPlotFreq( ){
	try	{
		updateFrequency = boost::lexical_cast<long>( str(updateFrequencyEdit->text()) );
	}
	catch ( boost::bad_lexical_cast& ){
		throwNested ( "ERROR: Failed to convert plotting update frequency to an integer! text was: " 
			+ str(updateFrequencyEdit->text ()) );
	}
	return updateFrequency;
}

void DataAnalysisControl::updateDisplays (analysisSettings settings) {
	autoThresholdAnalysisButton->setChecked (settings.autoThresholdAnalysisOption);
	displayGridBtn->setChecked (settings.displayGridOption);
	autoBumpAnalysis->setChecked (settings.autoBumpOption);
	bumpEditParam->setText (qstr (settings.bumpParam));
	unsigned counter = 0;
	for (auto& pltInfo : allTinyPlots) {
		unsigned activeCounter = -1;
		pltInfo.isActive = false;
		for (auto activePlt : settings.activePlotNames) {
			activeCounter++;
			if (activePlt == pltInfo.name) {
				pltInfo.isActive = true;
				pltInfo.whichGrid = settings.whichGrids[activeCounter];
				break;
			}
		}
		counter++;
	}
	// load the grid parameters for that selection.
	loadGridParams (currentSettings.grids[0]);
	reloadGridCombo (currentSettings.grids.size ());
	gridSelector->setCurrentIndex (0);
	reloadListView ();
}

void DataAnalysisControl::setAnalysisSettings (analysisSettings settings) {
	currentSettings = settings;
	updateDisplays (currentSettings);
}

void DataAnalysisControl::handleOpenConfig( ConfigStream& file ){
	analysisSettings settings = getAnalysisSettingsFromFile (file);
	setAnalysisSettings (settings);
}

analysisSettings DataAnalysisControl::getAnalysisSettingsFromFile (ConfigStream& file) {
	analysisSettings settings;
	unsigned numGrids=1;
	if (file.ver > Version ("4.0")) {
		file >> settings.autoThresholdAnalysisOption;
	}
	if (file.ver > Version ("3.0")) {
		file >> numGrids;
	}
	if (numGrids <= 0) {
		numGrids = 1;
	}
	settings.grids.resize (numGrids);
	for (auto& grid : settings.grids) {
		file >> grid.topLeftCorner.row >> grid.topLeftCorner.column >> grid.width >> grid.height >> grid.pixelSpacing;
	}
	// load the grid parameters for that selection.
	if (file.ver > Version ("2.7")) {
		ConfigSystem::checkDelimiterLine (file, "BEGIN_ACTIVE_PLOTS");
		unsigned numPlots = 0;
		file >> numPlots;
		file.get ();
		for (auto pltInc : range (numPlots)) {
			std::string tmp = file.getline ();
			settings.activePlotNames.push_back (tmp);
			if (file.ver > Version ("3.0")) {
				unsigned which;
				file >> which;
				file.get ();
				settings.whichGrids.push_back (which);
			}
			else {
				settings.whichGrids.push_back (0);
			}
		}
		ConfigSystem::checkDelimiterLine (file, "END_ACTIVE_PLOTS");
	}
	if (file.ver >= Version ("4.7")) {
		file.get ();
		file >> settings.displayGridOption;
	}
	if (file.ver >= Version ("5.4")) {
		file >> settings.autoBumpOption;
		file >> settings.bumpParam;
	}
	return settings;
}

std::pair<bool, std::string> DataAnalysisControl::getBumpAnalysisOptions (ConfigStream& file) {
	auto settings = getAnalysisSettingsFromFile (file);
	return { settings.autoBumpOption, settings.bumpParam };
}

void DataAnalysisControl::updateSettings () {
	saveGridParams ();
	currentSettings.autoThresholdAnalysisOption = autoThresholdAnalysisButton->isChecked ();
	currentSettings.displayGridOption = displayGridBtn->isChecked ();
	currentSettings.autoBumpOption = autoBumpAnalysis->isChecked ();
	currentSettings.bumpParam = str (bumpEditParam->text ());
	currentSettings.activePlotNames = getActivePlotList ();
}

void DataAnalysisControl::handleSaveConfig( ConfigStream& file ){
	updateSettings ();
	file << "DATA_ANALYSIS\n";
	file << "/*Auto-Threshold Analysis?*/\t" << currentSettings.autoThresholdAnalysisOption;
	file << "\n/*Number of Analysis Grids: */\t" << currentSettings.grids.size ();
	unsigned count = 0;
	for ( auto grid : currentSettings.grids ){
		file << "\n/*Grid #" + str (++count) << ":*/ "
			<< "\n/*Top-Left Corner Row:*/\t\t" << grid.topLeftCorner.row
			<< "\n/*Top-Left Corner Column:*/\t\t" << grid.topLeftCorner.column
			<< "\n/*Grid Width:*/\t\t\t\t\t" << grid.width
			<< "\n/*Grid Height:*/\t\t\t\t" << grid.height
			<< "\n/*Pixel Spacing:*/\t\t\t\t" << grid.pixelSpacing;
	}
	file << "\nBEGIN_ACTIVE_PLOTS\n";
	unsigned activeCount = 0;
	for ( auto miniPlot : allTinyPlots ){
		if ( miniPlot.isActive ){
			activeCount++;
		}
	}
	file << "/*Number of Active Plots:*/ " << activeCount;
	count = 0;
	for ( auto miniPlot : allTinyPlots ){
		if ( miniPlot.isActive ){
			file << "\n/*Active Plot #" + str (++count) + "*/";
			file << "\n/*Plot Name:*/ " << miniPlot.name;
			file << "\n/*Which Grid:*/ " << miniPlot.whichGrid;
		}
	}
	file << "\nEND_ACTIVE_PLOTS\n";
	file << "/*Display Grid?*/ " << currentSettings.displayGridOption << "\n";
	file << "/*Auto Bump Analysis?*/ " << currentSettings.autoBumpOption << "\n";
	auto bumpParamStr = (currentSettings.bumpParam =="" ? ConfigStream::emptyStringTxt : currentSettings.bumpParam);
	file << "/*Bump Param?*/ " << bumpParamStr << "\n";
	file << "END_DATA_ANALYSIS\n"; 
}



void DataAnalysisControl::fillPlotThreadInput(realTimePlotterInput* input){
	std::vector<tinyPlotInfo> usedPlots;
	input->plotInfo.clear();
	for (auto activeName : currentlyRunningSettings.activePlotNames) {
		for (auto plt : allTinyPlots ){
			if (plt.name == activeName) {
				input->plotInfo.push_back (plt);
				break;
			}
		}
	}
	input->grids = currentlyRunningSettings.grids;
	input->plottingFrequency = updateFrequency;
	// as I fill the input, also check this, which is necessary info for plotting.
	input->needsCounts = false;
	for (auto plt : input->plotInfo){
		PlottingInfo info(PLOT_FILES_SAVE_LOCATION + "\\" + plt.name + "." + PLOTTING_EXTENSION);
		if (info.getPlotType() != "Atoms"){
			input->needsCounts = true;
		}
	}
}

void DataAnalysisControl::handleAtomGridCombo( ){
	saveGridParams( );
	int sel = gridSelector->currentIndex( );
	if ( sel == -1 ){
		return;
	}
	else if ( sel == currentSettings.grids.size() ){
		reloadGridCombo( sel + 1 );
	}
	else if (sel > currentSettings.grids.size()){
		thrower ( "ERROR: Bad value for atom grid combobox selection???  (A low level bug, this shouldn't happen)" );
	}
	gridSelector->setCurrentIndex( sel );
	// load the grid parameters for that selection.
	loadGridParams(currentSettings.grids[sel] );
}

void DataAnalysisControl::reloadGridCombo( unsigned num ){
	currentSettings.grids.resize( num );
	gridSelector->clear( );
	unsigned count = 0;
	for ( auto grid : currentSettings.grids ){
		std::string txt( str( count++ ) );
		gridSelector->addItem( cstr( txt ) );
	}
	gridSelector->addItem( "New" );
}

void DataAnalysisControl::loadGridParams( atomGrid grid ){
	if (!gridSpacing || !gridHeight || !gridWidth) {
		return;
	}
	tlColEdit->setText (qstr (grid.topLeftCorner.column));
	tlRowEdit->setText (qstr (grid.topLeftCorner.row));
	std::string txt = str( grid.pixelSpacing );
	gridSpacing->setText(cstr(txt));
	txt = str( grid.width );
	gridWidth->setText(cstr(txt));
	txt = str( grid.height );
	gridHeight->setText ( cstr( txt ) );
}

void DataAnalysisControl::saveGridParams( ){
	if (!gridSpacing || !gridHeight || !gridWidth) {
		return;
	}
	unsigned row = 0, col = 0;
	try{
		row = boost::lexical_cast<unsigned> (str (tlRowEdit->text ()));
		col = boost::lexical_cast<unsigned> (str (tlColEdit->text ()));
		currentSettings.grids[0].topLeftCorner = { row, col };
		currentSettings.grids[0].pixelSpacing = boost::lexical_cast<long>( str(gridSpacing->text()) );
		currentSettings.grids[0].height = boost::lexical_cast<long>( str( gridHeight->text() ) );
		currentSettings.grids[0].width = boost::lexical_cast<long>( str( gridWidth->text()) );
	}
	catch ( boost::bad_lexical_cast&){
		throwNested ( "ERROR: failed to convert grid parameters to longs while saving grid data!" );
	}
}

std::vector<std::string> DataAnalysisControl::getActivePlotList(){
	std::vector<std::string> list;
	for ( auto plot : allTinyPlots ){
		if ( plot.isActive ) {
			list.push_back( plot.name );
		}
	}
	return list;
}

void DataAnalysisControl::updateDataSetNumberEdit( int number ){
	if ( number > 0 ){
		currentDataSetNumberDisp->setText( cstr( number ) );
	}
	else{
		currentDataSetNumberDisp->setText ( "None" );
	}
}

void DataAnalysisControl::reloadListView(){
	plotListview->setRowCount (0);
	for (auto item : allTinyPlots){
		if (item.numPics != unofficialPicsPerRep) {
			continue;
		}
		int row = plotListview->rowCount ();
		plotListview->insertRow (row);
		plotListview->setItem (row, 0, new QTableWidgetItem (item.name.c_str()));
		plotListview->setItem (row, 1, new QTableWidgetItem (cstr(item.whichGrid)));
		auto item3 = new QTableWidgetItem (item.isActive ? "YES" : "NO");
		item3->setFlags (item3->flags () & ~Qt::ItemIsEditable);
		plotListview->setItem (row, 2, item3);
	}
}

void DataAnalysisControl::updateUnofficialPicsPerRep (unsigned ppr) {
	unofficialPicsPerRep = ppr;
	reloadListView ();
}

analysisSettings DataAnalysisControl::getConfigSettings () {
	updateSettings ();
	return currentSettings;
}

analysisSettings DataAnalysisControl::getRunningSettings() {
	return currentlyRunningSettings;
}

void DataAnalysisControl::setRunningSettings (analysisSettings options) {
	currentlyRunningSettings = options;
}