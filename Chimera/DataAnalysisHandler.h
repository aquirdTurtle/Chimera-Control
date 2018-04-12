#pragma once

#include "Control.h"
#include "myButton.h"
#include "PlotDialog.h"
#include "PlottingInfo.h"
#include "EmbeddedPythonHandler.h"
#include "Communicator.h"

#include "atomGrid.h"
#include "tinyPlotInfo.h"
#include "Expression.h"
#include <deque>
#include <map>

struct realTimePlotterInput;
struct cameraPositions;

// variation data is to be organized
// variationData[datasetNumber][groupNumber][variationNumber];
typedef std::vector<std::vector<std::vector<double>>> variationData;
typedef std::vector<std::vector<double>> avgData;

class DataAnalysisControl
{
	public:
		DataAnalysisControl( );
		void initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips,
						 int isTriggerModeSensitive, rgbMap rgbs );
		ULONG getPlotFreq( );
		void handleOpenConfig( std::ifstream& file, int versionMajor, int versionMinor );
		void handleNewConfig( std::ofstream& file );
		void handleSaveConfig(std::ofstream& file );
		void handleDoubleClick( fontMap* fonts, UINT currentPicsPerRepetition );
		void handleRClick( );
		void rearrange( std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts );
		void updateDataSetNumberEdit( int number );
		void analyze( std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler,
					  Communicator* comm );
		void onManualButtonPushed( );
		void onCornerButtonPushed( );
		void handlePictureClick( coordinate location );
		std::vector<coordinate> getAnalysisLocs( );
		atomGrid getAtomGrid( UINT which );
		std::vector<atomGrid> getGrids( );
		atomGrid getCurrentGrid( );
		UINT getSelectedGridNumber( );
		void clearAtomLocations( );
		bool getLocationSettingStatus( );
		std::vector<std::string> getActivePlotList( );
		void reloadListView( );
		bool buttonClicked( );
		void handleAtomGridCombo( );
		void reloadGridCombo( UINT num );
		void fillPlotThreadInput( realTimePlotterInput* input );
		void loadGridParams( atomGrid grid );
		static unsigned __stdcall plotterProcedure( void* voidInput );
		void saveGridParams( );
		void handleDeleteGrid( );
		UINT getPlotTime( );
		// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
		// readable. I very rarely use things like this.
		template<class T> using vector = std::vector<T>;
		// subroutine for handling atom & count plots
		static void handlePlotAtoms( 
			PlottingInfo plotInfo, UINT repNum, vector<vector<std::pair<double, ULONG>> >& finData, 
			std::vector<std::shared_ptr<std::vector<dataPoint>>> dataContainers, 
			UINT variationNumber, vector<vector<bool>>& pscSatisfied, 
			int plotNumberCount, vector<vector<int> > atomPresent, UINT plottingFrequency, UINT groupNum, 
			UINT picsPerVariation );
		static void handlePlotHist( 
			PlottingInfo plotInfo, vector<vector<long>> countData,  
			vector<vector<std::deque<double>>>& finData, vector<vector<bool>>pscSatisfied, 
			vector<vector<std::map<int, std::pair<int, ULONG>>>>& histData,
			std::vector<std::shared_ptr<std::vector<dataPoint>>> dataArrays, UINT groupNum );
		static void determineWhichPscsSatisfied(
			PlottingInfo& info, UINT groupSize, vector<vector<int>> atomPresentData, vector<vector<bool>>& pscSatisfied );
	private:
		// real time plotting
		ULONG updateFrequency;
		Control<CStatic> updateFrequencyLabel1;
		Control<CStatic> updateFrequencyLabel2;
		Control<CEdit> updateFrequencyEdit;
		Control<CStatic> header;
		Control<CListCtrl> plotListview;
		std::vector<tinyPlotInfo> allTinyPlots;
		// other data analysis
		bool currentlySettingGridCorner;
		bool currentlySettingAnalysisLocations;
		Control<CStatic> currentDataSetNumberText;
		Control<CStatic> currentDataSetNumberDisp;
		Control<CleanButton> manualSetAnalysisLocsButton;

		Control<CComboBox> gridSelector;
		Control<CStatic> gridHeader;
		Control<CleanButton> setGridCorner;
		Control<CStatic> gridSpacingText;
		Control<CEdit> gridSpacing;
		Control<CStatic> gridWidthText;
		Control<CEdit> gridWidth;
		Control<CStatic> gridHeightText;
		Control<CEdit> gridHeight;

		Control<CStatic> plotTimerTxt;
		Control<CEdit> plotTimerEdit;

		std::vector<atomGrid> grids;
		UINT selectedGrid = 0;
		Control<CleanButton> deleteGrid;

		std::vector<coordinate> atomLocations;
		bool threadNeedsCounts;
};


