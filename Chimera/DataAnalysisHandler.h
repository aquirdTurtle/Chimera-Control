#pragma once

#include "Control.h"
#include "PlottingInfo.h"
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
		void initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips,
						 int isTriggerModeSensitive, rgbMap rgbs );
		ULONG getPlotFreq( );
		void handleOpenConfig( std::ifstream& file, int versionMajor, int versionMinor );
		void handleNewConfig( std::ofstream& file );
		void handleSaveConfig(std::ofstream& file );
		void handleDoubleClick( fontMap* fonts, UINT currentPicsPerRepetition );
		void handleRClick( );
		void rearrange( std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts );
		//
		void updateDataSetNumberEdit( int number );
		void analyze( std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler,
					  Communicator* comm );
		void onManualButtonPushed( );
		void onCornerButtonPushed( );
		void handlePictureClick( coordinate location );
		std::vector<coordinate> getAnalysisLocs( );
		atomGrid getAtomGrid( );
		void clearAtomLocations( );
		bool getLocationSettingStatus( );
		std::vector<std::string> getActivePlotList( );
		void reloadListView( );
		bool buttonClicked( );

		void fillPlotThreadInput( realTimePlotterInput* input );
		static unsigned __stdcall plotterProcedure( void* voidInput );
		
		// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
		// readable. I very rarely use things like this.
		template<class T> using vector = std::vector<T>;
		// subroutine for handling atom & count plots
		static void handlePlotAtoms( realTimePlotterInput* input, PlottingInfo plotInfo, UINT repNum,
									 vector<vector<std::pair<double, ULONG>> >& finData, variationData& finAvgs, 
									 variationData& finErrs, variationData& finX, avgData& avgAvgs, avgData& avgErrs, 
									 avgData& avgX, vector<vector<bool> >& needNewData, 
									 vector<vector<bool>>& pscSatisfied, int plotNumber, 
									 vector<vector<long>>& countData, int plotNumberCount, 
									 vector<vector<int> > atomPresent );

		static void handlePlotCounts( realTimePlotterInput* input, PlottingInfo plotInfo, UINT pictureNumber,
									  vector<vector<vector<long> > >& finData, variationData& finAvgs, 
									  variationData& finErrs, variationData& finX, avgData& avgAvgs, avgData& avgErrs,
									  avgData& avgX, vector<vector<bool> >& needNewData, 
									  vector<vector<bool>>& pscSatisfied, int plotNumber, 
									  vector<vector<long>>& countData, int plotNumberCount, 
									  vector<vector<int> > atomPresent );
		static void handlePlotHist( realTimePlotterInput* input, PlottingInfo plotInfo, UINT plotNumber,
									vector<vector<long>> countData,  vector<vector<std::deque<double>>>& finData,
									vector<vector<bool>>pscSatisfied, int plotNumberCount, 
									vector<vector<std::map<int, std::pair<int, ULONG>>>>& histData );
		static void determineWhichPscsSatisfied( PlottingInfo& info, UINT groupSize, 
												 vector<vector<int>> atomPresentData,
												 vector<vector<bool>>& pscSatisfied );
	private:
		// real time plotting
		ULONG updateFrequency;
		Control<CStatic> updateFrequencyLabel1;
		Control<CStatic> updateFrequencyLabel2;
		Control<CEdit> updateFrequencyEdit;
		Control<CStatic> header;
		Control<CListCtrl> plotListview;
		std::vector<tinyPlotInfo> allPlots;
		// other data analysis
		bool currentlySettingGridCorner;
		bool currentlySettingAnalysisLocations;
		Control<CStatic> currentDataSetNumberText;
		Control<CStatic> currentDataSetNumberDisp;
		Control<CButton> manualSetAnalysisLocsButton;

		Control<CStatic> gridHeader;
		Control<CButton> setGridCorner;
		Control<CStatic> gridSpacingText;
		Control<CEdit> gridSpacing;
		Control<CStatic> gridWidthText;
		Control<CEdit> gridWidth;
		Control<CStatic> gridHeightText;
		Control<CEdit> gridHeight;
		atomGrid currentGrid;
		std::vector<coordinate> atomLocations;
		bool threadNeedsCounts;
};


