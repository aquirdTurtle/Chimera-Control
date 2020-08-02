// created by Mark O. Brown
#pragma once

#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "Plotting/PlotDialog.h"
#include "Plotting/PlottingInfo.h"
#include "Python/EmbeddedPythonHandler.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "atomGrid.h"
#include "Plotting/tinyPlotInfo.h"
#include "ParameterSystem/Expression.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include <deque>
#include <map>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtableWidget.h>
#include <qcombobox.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <RealTimeDataAnalysis/analysisSettings.h>
#include <GeneralObjects/IChimeraSystem.h>
struct realTimePlotterInput;
struct cameraPositions;

// variation data is to be organized
// variationData[datasetNumber][groupNumber][variationNumber];
typedef std::vector<std::vector<std::vector<double>>> variationData;
typedef std::vector<std::vector<double>> avgData;

class DataAnalysisControl : public IChimeraSystem {
	public:
		DataAnalysisControl(IChimeraQtWindow* parent );

		bool wantsThresholdAnalysis ( );
		void initialize( POINT& pos, IChimeraQtWindow* parent );
		unsigned getPlotFreq( );
		void handleOpenConfig(ConfigStream& file );
		static analysisSettings getAnalysisSettingsFromFile (ConfigStream& file);
		void handleSaveConfig(ConfigStream& file );
		static std::pair<bool, std::string> getBumpAnalysisOptions (ConfigStream& file);
		bool wantsBumpAnalysis ();
		void updateDataSetNumberEdit( int number );
		
		//void analyze( std::string date, long runNumber, long accumulations );
		void setGridCornerLocation (coordinate loc);
		std::vector<coordinate> getAnalysisLocs( );
		atomGrid getAtomGrid( unsigned which );
		std::vector<atomGrid> getGrids( );
		atomGrid getCurrentGrid( );
		unsigned getSelectedGridNumber( );
		void clearAtomLocations( );
		bool getLocationSettingStatus( );
		std::vector<std::string> getActivePlotList( );
		void reloadListView( );
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
			PlottingInfo plotInfo, unsigned repNum, vector<vector<std::pair<double, ULONG>> >& finData, 
			std::vector<std::vector<dataPoint>>& dataContainers, 
			unsigned variationNumber, vector<vector<bool>>& pscSatisfied, 
			int plotNumberCount, vector<vector<int> > atomPresent, unsigned plottingFrequency, unsigned groupNum, 
			unsigned picsPerVariation );
		static std::vector<std::vector<dataPoint>> handlePlotHist(
			PlottingInfo plotInfo, vector<vector<long>> countData,  
			vector<vector<std::deque<double>>>& finData, vector<vector<bool>>pscSatisfied, 
			vector<vector<std::map<int, std::pair<int, ULONG>>>>& histData,
			std::vector<std::vector<dataPoint>>& dataContainers, unsigned groupNum );
		static void determineWhichPscsSatisfied(
			PlottingInfo& info, unsigned groupSize, vector<vector<int>> atomPresentData, vector<vector<bool>>& pscSatisfied );
		bool getDrawGridOption ( );
		void handleContextMenu (const QPoint& pos);
		QString getBumpParamName ();
	private:
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

		std::vector<atomGrid> grids;
		unsigned selectedGrid = 0;
		CQPushButton* deleteGrid;

		std::vector<coordinate> atomLocations;
		bool threadNeedsCounts;
};


