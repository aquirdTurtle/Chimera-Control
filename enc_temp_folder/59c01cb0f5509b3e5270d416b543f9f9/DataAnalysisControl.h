// created by Mark O. Brown
#pragma once

#include "Plotting/PlotDialog.h"
#include "Plotting/PlottingInfo.h"
#include "Python/EmbeddedPythonHandler.h"
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
		DataAnalysisControl(IChimeraQtWindow* parent );

		void initialize( POINT& pos, IChimeraQtWindow* parent );
		void handleOpenConfig(ConfigStream& file );
		static analysisSettings getAnalysisSettingsFromFile (ConfigStream& file);
		void setAnalysisSettings (analysisSettings settings);
		void updateSettings ();
		void handleSaveConfig(ConfigStream& file );
		static std::pair<bool, std::string> getBumpAnalysisOptions (ConfigStream& file);
		void updateDataSetNumberEdit( int number );
		
		void setGridCornerLocation (coordinate loc);
		void reloadListView();
		bool buttonClicked( );
		void handleAtomGridCombo( );
		void reloadGridCombo( unsigned num );
		void fillPlotThreadInput( realTimePlotterInput* input );
		void loadGridParams( atomGrid grid );
		static unsigned __stdcall plotterProcedure( void* voidInput );
		void saveGridParams( );
		void handleDeleteGrid( );
		void updatePlotTime ( );
		std::atomic<unsigned>& getPlotTime( );
		// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
		// readable. I very rarely use things like this.
		template<class T> using vector = std::vector<T>;
		// subroutine for handling atom & count plots
		static std::vector<std::vector<dataPoint>> handlePlotAtoms(
			PlottingInfo plotInfo, unsigned repNum, vector<vector<std::pair<double, unsigned long>> >& finData, 
			std::vector<std::vector<dataPoint>>& dataContainers, 
			unsigned variationNumber, vector<vector<bool>>& pscSatisfied, 
			int plotNumberCount, vector<vector<int> > atomPresent, unsigned plottingFrequency, unsigned groupNum, 
			unsigned picsPerVariation );
		static std::vector<std::vector<dataPoint>> handlePlotHist(
			PlottingInfo plotInfo, vector<vector<long>> countData,  
			vector<vector<std::deque<double>>>& finData, vector<vector<bool>>pscSatisfied, 
			vector<vector<std::map<int, std::pair<int, unsigned long>>>>& histData,
			std::vector<std::vector<dataPoint>>& dataContainers, unsigned groupNum );
		static void determineWhichPscsSatisfied(
			PlottingInfo& info, unsigned groupSize, vector<vector<int>> atomPresentData, vector<vector<bool>>& pscSatisfied );
		void handleContextMenu (const QPoint& pos);

		bool getLocationSettingStatus ();
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
		bool currentlySettingGridCorner;
		bool currentlySettingAnalysisLocations;
		QLabel* currentDataSetNumberText;
		QLabel* currentDataSetNumberDisp;

		CQComboBox* gridSelector;
		CQPushButton* setGridCorner;
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