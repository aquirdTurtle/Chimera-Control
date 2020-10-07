// created by Mark O. Brown
#pragma once

#include "Plotting/PlottingInfo.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "atomGrid.h"
#include "Plotting/tinyPlotInfo.h"
#include "ParameterSystem/Expression.h"

#include <CustomQtControls/AutoNotifyCtrls.h>
#include <RealTimeDataAnalysis/analysisSettings.h>
#include <GeneralObjects/IChimeraSystem.h>

#include <deque>
#include <map>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtableWidget.h>
#include <qcombobox.h>

struct realTimePlotterInput;
struct cameraPositions;

// variation data is to be organized
// variationData[datasetNumber][groupNumber][variationNumber];
typedef std::vector<std::vector<std::vector<double>>> variationData;
typedef std::vector<std::vector<double>> avgData;

class DataAnalysisControl : public IChimeraSystem {
	public:
		static constexpr auto PLOTTING_EXTENSION = "plot";

		DataAnalysisControl(IChimeraQtWindow* parent );

		void initialize( QPoint& pos, IChimeraQtWindow* parent );
		void handleOpenConfig(ConfigStream& file );
		static analysisSettings getAnalysisSettingsFromFile (ConfigStream& file);
		void setAnalysisSettings (analysisSettings settings);
		void updateSettings ();
		void handleSaveConfig(ConfigStream& file );
		static std::pair<bool, std::string> getBumpAnalysisOptions (ConfigStream& file);
		void updateDataSetNumberEdit( int number );
		
		void reloadListView();
		void handleAtomGridCombo( );
		void reloadGridCombo( unsigned num );
		void fillPlotThreadInput( realTimePlotterInput* input );
		void loadGridParams( atomGrid grid );
		static unsigned __stdcall plotterProcedure( void* voidInput );
		void saveGridParams( );
		void handleDeleteGrid( );
		void updatePlotTime ( );
		std::atomic<unsigned>& getPlotTime( );

		void handleContextMenu (const QPoint& pos);

		unsigned getPlotFreq ();
		std::vector<std::string> getActivePlotList ();

		// used to determine what plots to show in the listview. should be updated from the official number when the 
		// official changes. 
		void updateUnofficialPicsPerRep (unsigned ppr);
		void updateDisplays (analysisSettings settings);
		analysisSettings getConfigSettings ();
		void setRunningSettings (analysisSettings options);
		analysisSettings getRunningSettings ();
	private:
		analysisSettings currentSettings, currentlyRunningSettings;

		unsigned unofficialPicsPerRep = 1;
		// real time plotting
		unsigned long updateFrequency;
		QLabel* updateFrequencyLabel1;
		QLabel* updateFrequencyLabel2;
		CQLineEdit* updateFrequencyEdit;

		QLabel* header;
		QTableWidget* plotListview;
		std::vector<tinyPlotInfo> allTinyPlots;
		
		// other data analysis
		QLabel* currentDataSetNumberText;
		QLabel* currentDataSetNumberDisp;

		CQComboBox* gridSelector;

		QLabel* tlRowLabel;
		CQLineEdit* tlRowEdit;
		QLabel* tlColLabel;
		CQLineEdit* tlColEdit;
		QLabel* gridSpacingText;
		CQLineEdit* gridSpacing;
		QLabel* gridWidthText;
		CQLineEdit* gridWidth;
		QLabel* gridHeightText;
		CQLineEdit* gridHeight;

		CQPushButton* doBumpAnalysis;
		CQCheckBox* autoBumpAnalysis;
		CQLineEdit* bumpEditParam;

		CQCheckBox* autoThresholdAnalysisButton;
		CQCheckBox* displayGridBtn;

		QLabel* plotTimerTxt;
		CQLineEdit* plotTimerEdit;
		std::atomic<unsigned> plotTime=5000;

		CQPushButton* deleteGrid;

};

/*
struct analysisSettings {
	bool autoThresholdAnalysisOption = false;
	bool displayGridOption;
	std::vector<atomGrid> grids;
	std::string bumpParam = "";
	bool autoBumpOption = false;
	std::vector<std::string> activePlotNames;
	std::vector<unsigned> whichGrids;
};*/