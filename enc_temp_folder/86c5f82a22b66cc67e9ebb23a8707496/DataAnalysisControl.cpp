// created by Mark O. Brown
#include "stdafx.h"

#include "RealTimeDataAnalysis/DataAnalysisControl.h"
#include "Control.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "RealTimeDataAnalysis/PlotDesignerDialog.h"
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
	std::vector<std::string> names = ProfileSystem::searchForFiles (PLOT_FILES_SAVE_LOCATION, str ("*.") + PLOTTING_EXTENSION);
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
				+ PLOTTING_EXTENSION));
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
	plotListview->connect (detailsAction, &QAction::triggered, [this, item]() {
		try {
			// edit existing plot file using the plot designer.
			//PlotDesignerDialog dlg (fonts, PLOT_FILES_SAVE_LOCATION + "\\" + allTinyPlots[clRow].name + "."
			//						  + PLOTTING_EXTENSION);
			//dlg.DoModal ();
		}
		catch (ChimeraError & err) {
			errBox (err.trace ());
		}});
	menu.addAction (editAction);

	auto* newPerson = new QAction ("New Plot", plotListview);
	plotListview->connect (newPerson, &QAction::triggered, [this]() {
		});
	if (item) { menu.addAction (deleteAction); }
	menu.addAction (newPerson);
	menu.exec (plotListview->mapToGlobal (pos));
}

void DataAnalysisControl::initialize( POINT& pos, IChimeraQtWindow* parent ){
	header = new QLabel ("DATA ANALYSIS", parent);
	header->setGeometry (pos.x, pos.y, 480, 25);

	currentDataSetNumberText = new QLabel ("Data Set #:", parent);
	currentDataSetNumberText->setGeometry (pos.x, pos.y += 25, 350, 50);
	currentDataSetNumberDisp = new QLabel ("?", parent);
	currentDataSetNumberDisp->setGeometry (pos.x + 350, pos.y, 130, 50);
	currentDataSetNumberDisp->setStyleSheet("QLabel { font: bold 24pt; };");

	gridSelector = new CQComboBox (parent);
	gridSelector->setGeometry (pos.x, pos.y += 50, 50, 25);
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
	deleteGrid->setGeometry (pos.x + 50, pos.y, 50, 25);
	parent->connect (deleteGrid, &QPushButton::released, [this, parent]() {
			try{
				handleDeleteGrid ();
			}
			catch (ChimeraError& err){
				parent->reportErr (err.qtrace ());
			}
		});

	setGridCorner = new CQPushButton ("Set Grid T.L.", parent);
	setGridCorner->setGeometry (pos.x + 100, pos.y, 100, 25);
	parent->connect (setGridCorner, &QPushButton::released, 
		[parent]() {
			parent->andorWin->handleSetAnalysisPress ();
		});

	gridSpacingText = new QLabel ("Spacing", parent);
	gridSpacingText->setGeometry (pos.x+200, pos.y, 60, 25);

	gridSpacing = new CQLineEdit ("0", parent);
	gridSpacing->setGeometry (pos.x + 260, pos.y, 30, 25);

	gridWidthText = new QLabel ("Width", parent);
	gridWidthText->setGeometry (pos.x + 290, pos.y, 60, 25);

	gridWidth = new CQLineEdit ("0", parent);
	gridWidth->setGeometry (pos.x + 350, pos.y, 30, 25);

	gridHeightText = new QLabel ("Height", parent);
	gridHeightText->setGeometry (pos.x + 380, pos.y, 60, 25);

	gridHeight = new CQLineEdit ("0", parent);
	gridHeight->setGeometry (pos.x + 440, pos.y, 40, 25);
	// 
	displayGridBtn = new CQCheckBox ("Display Grid?", parent);
	displayGridBtn->setGeometry (pos.x, pos.y += 25, 240, 25);

	/// PLOTTING FREQUENCY CONTROLS
	updateFrequencyLabel1 = new QLabel ("Update plots every", parent);
	updateFrequencyLabel1->setGeometry (pos.x + 240, pos.y, 140, 25);

	updateFrequencyEdit = new CQLineEdit ("5", parent);
	updateFrequencyEdit->setGeometry (pos.x + 390, pos.y, 30, 25);
	updateFrequency = 5;
	
	updateFrequencyLabel2 = new QLabel (") reps.", parent);
	updateFrequencyLabel2->setGeometry (pos.x + 420, pos.y, 60, 25);

	plotTimerTxt = new QLabel ("Plot Update Timer (ms):", parent);
	plotTimerTxt->setGeometry (pos.x, pos.y += 25, 180, 25);
	
	plotTimerEdit = new CQLineEdit ("5000", parent);
	plotTimerEdit->setGeometry (pos.x + 180, pos.y, 60, 25);
	parent->connect (plotTimerEdit, &QLineEdit::textChanged, [this]() {updatePlotTime (); });

	autoThresholdAnalysisButton = new CQCheckBox ("Auto Threshold Analysis", parent);
	autoThresholdAnalysisButton->setGeometry (pos.x + 240, pos.y, 240, 25);
	autoThresholdAnalysisButton->setToolTip ("At the end of an experiment, run some python code which will fit the "
		"data and determine good thresholds which can be outputted to a file to "
		"keep the thresholds used by the real-time analysis up-to-date.");

	
	autoBumpAnalysis = new CQCheckBox ("Auto Bump Analysis", parent);
	autoBumpAnalysis->setGeometry (pos.x, pos.y += 25, 180, 25);
	autoBumpAnalysis->setToolTip ("At the end of the experiment, run some python code which will do standard data "
		"analysis on the resulting data set and fit a bump to it. The bump center value will be written to a file.");
	doBumpAnalysis = new CQPushButton ("Analyze Now", parent);
	doBumpAnalysis->setGeometry (pos.x + 180, pos.y, 180, 25);
	parent->connect (doBumpAnalysis, &QPushButton::released, [parent]() {
		parent->andorWin->handleBumpAnalysis (parent->mainWin->getProfileSettings());
		});
	bumpEditParam = new CQLineEdit (parent);
	bumpEditParam->setGeometry (pos.x + 360, pos.y, 120, 25);

	/// Initialize the listview
	plotListview = new QTableWidget (parent);
	plotListview->setGeometry (pos.x, pos.y+=25, 480, 150);
	pos.y += 150;
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
					allTinyPlots[row].whichGrid = boost::lexical_cast<unsigned>(cstr (item->text ()));
				}
				catch (ChimeraError&) {}
			}
		});
	parent->connect (plotListview, &QTableWidget::cellDoubleClicked, 
		[this, parent](int clRow, int clCol) {
			parent->configUpdated ();
			if (clCol == 2) {
				allTinyPlots[clRow].isActive = !allTinyPlots[clRow].isActive;
				auto* item = new QTableWidgetItem (allTinyPlots[clRow].isActive ? "YES" : "NO");
				item->setFlags (item->flags () & ~Qt::ItemIsEditable);
				plotListview->setItem (clRow, clCol, item);
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
		ProfileSystem::checkDelimiterLine (file, "BEGIN_ACTIVE_PLOTS");
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
		ProfileSystem::checkDelimiterLine (file, "END_ACTIVE_PLOTS");
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

void DataAnalysisControl::determineWhichPscsSatisfied( 
	PlottingInfo& info, unsigned groupSize, std::vector<std::vector<int>> atomPresentData, std::vector<std::vector<bool>>& pscSatisfied){
	// There's got to be a better way to iterate through these guys...
	for ( auto dataSetI : range( info.getDataSetNumber( ) ) ){
		for ( auto groupI : range( groupSize ) ){
			for ( auto conditionI : range( info.getConditionNumber( ) ) ){
				for ( auto pixelI : range( info.getPixelNumber( ) ) ){
					for ( auto picI : range( info.getPicNumber( ) ) ){
						// test if condition exists
						int condition = info.getPostSelectionCondition( dataSetI, conditionI, pixelI, picI );
						if ( condition == 0 ){
							continue;
						}
						if ( condition == 1 && atomPresentData[groupI][picI] != 1 )	{
							pscSatisfied[dataSetI][groupI] = false;
						}
						else if ( condition == -1 && atomPresentData[groupI][picI] != 0 ){
							pscSatisfied[dataSetI][groupI] = false;
						}
					}
				}
			}
		}
	}
}


std::vector<std::vector<dataPoint>> DataAnalysisControl::handlePlotAtoms( PlottingInfo plotInfo, unsigned pictureNumber,
											std::vector<std::vector<std::pair<double, unsigned long>> >& finData,
										    std::vector<std::vector<dataPoint>>& dataContainers,
										    unsigned variationNumber, std::vector<std::vector<bool>>& pscSatisfied,
										    int plotNumberCount, std::vector<std::vector<int> > atomPresent, unsigned plottingFrequency,
										    unsigned groupNum, unsigned picsPerVariation ){
	if (pictureNumber % picsPerVariation == plotInfo.getPicNumber()){
		// first pic of new variation, so need to update x vals.
		finData = std::vector<std::vector<std::pair<double, unsigned long>>>( plotInfo.getDataSetNumber( ),
															vector<std::pair<double, unsigned long>>( groupNum, { 0,0 } ) );
	}
	/// Check Data Conditions
	for (auto dataSetI : range(plotInfo.getDataSetNumber())){
		for (auto groupI :range(groupNum)){
			if (pscSatisfied[dataSetI][groupI] == false){
				// no new data.
				continue;
			}
			bool dataVal = true;
			for (auto pixelI : range(plotInfo.getPixelNumber())){
				for (auto picI : range(plotInfo.getPicNumber())){
					// check if there is a condition at all
					int truthCondition = plotInfo.getResultCondition(dataSetI, pixelI, picI);
					if (truthCondition == 0){
						continue;
					}
					int pixel = groupI;
					if (truthCondition == 1 && atomPresent[pixel][picI] != 1){
						dataVal = false;
					}
					// ?? This won't happen... see above continue...
					else if (truthCondition == 0 && atomPresent[groupI][picI] != 0)	{
						dataVal = false;
					}
				}
			}
			finData[dataSetI][groupI].first += dataVal;
			finData[dataSetI][groupI].second++;
		}
	}
	// Core data structures have been updated. return if not time for a plot update yet.
	if ( plotNumberCount % plottingFrequency != 0 ){
		return {};
	}
	if (dataContainers.size () == 0) {
		return {};
	}
	/// Calculate averages and standard devations for Data sets AND groups...
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( ))){
		unsigned avgId = dataContainers.size() - dataSetI - 1; 
		for ( auto groupI : range( groupNum)){
			// Will be function fo groupI and dataSetI; TBD			
			unsigned dataId = (dataSetI+1) * groupI;
			// calculate new data points
			double mean = finData[dataSetI][groupI].first / finData[dataSetI][groupI].second;
			double error = mean * ( 1 - mean ) / std::sqrt( finData[dataSetI][groupI].second );
			dataContainers[dataId][variationNumber].y = mean;
			dataContainers[dataId][variationNumber].err = error;
		}
		/// calculate averages
		double avgAvgVal = 0, avgErrsVal = 0;
		std::pair<double, unsigned long> allDataTempNew(0,0);
		for ( auto data : finData[dataSetI] ){
			allDataTempNew.first += data.first;
			allDataTempNew.second += data.second;
		}
		if (allDataTempNew.second == 0) {
			dataContainers[avgId][variationNumber].y = 0;
			dataContainers[avgId][variationNumber].err = 0;
		}
		else {
			double mean = allDataTempNew.first / allDataTempNew.second;
			double error = mean * (1 - mean) / std::sqrt (allDataTempNew.second);
			dataContainers[avgId][variationNumber].y = mean;
			dataContainers[avgId][variationNumber].err = error;
		}
	}
	return dataContainers;
}

std::vector<std::vector<dataPoint>> DataAnalysisControl::handlePlotHist( PlottingInfo plotInfo, vector<vector<long>> countData,
										  vector<vector<std::deque<double>>>& finData, vector<vector<bool>> pscSatisfied, 
										  vector<vector<std::map<int, std::pair<int, unsigned long>>>>& histData,
										  std::vector<std::vector<dataPoint>>& dataContainers,
										  unsigned groupNum ){
	/// options are fundamentally different for histograms.
	// load pixel counts
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) ){
		for ( auto groupI : range( groupNum ) )	{
			if ( pscSatisfied[dataSetI][groupI] == false ){
				// no new data.
				continue;
			}
			double binWidth = plotInfo.getDataSetHistBinWidth( dataSetI );
			for ( auto pixelI : range( plotInfo.getPixelNumber( ) ) ){
				for ( auto picI : range( plotInfo.getPicNumber( ) ) ){
					// check if there is a condition at all
					if ( plotInfo.getResultCondition( dataSetI, pixelI, picI ) ){
						int index = -int (plotInfo.getPicNumber ()) + int (picI);
						if (int(countData[groupI].size ()) + index < 0) {
							return {}; // not enough pictures yet
						}
						int binNum = std::round( double( countData[groupI][countData[groupI].size()+index] ) / binWidth );
						if ( histData[dataSetI][groupI].find( binNum ) == histData[dataSetI][groupI].end( ) ){
							// if bin doesn't exist
							histData[dataSetI][groupI][binNum] = { binNum * binWidth, 1 };
						}
						else{
							histData[dataSetI][groupI][binNum].second++;
						}
					}
				}
			}	
			// find the range of bins
			int min_bin = INT_MAX, max_bin = -INT_MAX;
			for ( auto p : histData[ dataSetI ][ groupI ] ){
				if ( p.first < min_bin ){
					min_bin = p.first;
				}
				if ( p.first > max_bin ){
					max_bin = p.first;
				}
			}
			/// check for empty data points and fill them with zeros.
			for ( auto bin_i : range ( max_bin-min_bin ) )	{
				auto binNum = bin_i + min_bin;
				if ( histData[ dataSetI ][ groupI ].find ( binNum ) == histData[ dataSetI ][ groupI ].end ( ) )	{
					// if bin doesn't exist
					histData[ dataSetI ][ groupI ][ binNum ] = { binNum * binWidth, 0 };
				}
			}
			// Will be function fo groupI and dataSetI; TBD			
			unsigned dataId = (dataSetI + 1) * groupI;
			// calculate new data points
			unsigned count = 0;
			if (dataContainers.size () <= dataId) {
				dataContainers.resize (dataId+1);
			}
			dataContainers[dataId].resize( histData[dataSetI][groupI].size( ) );
			for ( auto& bin : histData[dataSetI][groupI] ) {
				dataContainers[dataId][count].x = bin.second.first;
				dataContainers[dataId][count].y = bin.second.second;
				dataContainers[dataId][count].err = 0;
				count++;
			}
		}
	}
	return dataContainers;
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
	CString txt;
	try{
		currentSettings.grids[0].pixelSpacing = boost::lexical_cast<long>( str(gridSpacing->text()) );
		currentSettings.grids[0].height = boost::lexical_cast<long>( str( gridHeight->text() ) );
		currentSettings.grids[0].width = boost::lexical_cast<long>( str( gridWidth->text()) );
	}
	catch ( boost::bad_lexical_cast&){
		throwNested ( "ERROR: failed to convert grid parameters to longs while saving grid data!" );
	}
}

std::vector<std::string> DataAnalysisControl::getActivePlotList(){
	vector<std::string> list;
	for ( auto plot : allTinyPlots ){
		if ( plot.isActive ) {
			list.push_back( plot.name );
		}
	}
	return list;
}

bool DataAnalysisControl::getLocationSettingStatus(){
	return currentlySettingAnalysisLocations;
}

void DataAnalysisControl::updateDataSetNumberEdit( int number ){
	if ( number > 0 ){
		currentDataSetNumberDisp->setText( cstr( number ) );
	}
	else{
		currentDataSetNumberDisp->setText ( "None" );
	}
}

bool DataAnalysisControl::buttonClicked(){
	return (setGridCorner->isChecked());
}

void DataAnalysisControl::setGridCornerLocation(coordinate loc ){
	// else press it.
	currentSettings.grids[0].topLeftCorner = loc;
}

/*
atomGrid DataAnalysisControl::getCurrentGrid( ){
	return getAtomGrid( 0 );
}*/

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