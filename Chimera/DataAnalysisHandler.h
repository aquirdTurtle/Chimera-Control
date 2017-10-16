#pragma once

#include "Control.h"
#include "PlottingInfo.h"
#include "atomGrid.h"
#include "tinyPlotInfo.h"
#include "Expression.h"

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
		// subroutine for handling atom & count plots
		static void handlePlotAtomsOrCounts( realTimePlotterInput* input, PlottingInfo plotInfo, UINT repNum,
											 std::vector<std::vector<std::vector<long> > >& finData,
											 variationData& finAvgs, variationData& finErrs, variationData& finX,
											 avgData& avgAvgs, avgData& avgErrs, avgData& avgX,
											 std::vector<std::vector<bool> >& needNewData,
											 std::vector<std::vector<bool>>& pscSatisfied, int plotNumber,
											 std::vector<std::vector<long>>& countData, int plotNumberCount,
											 std::vector<std::vector<int> > atomPresent );

		static void handlePlotHist( realTimePlotterInput* input, PlottingInfo plotInfo, UINT plotNumber,
									std::vector<std::vector<long>> countData,
									std::vector<std::vector<std::vector<long>>>& finData,
									std::vector<std::vector<bool>>pscSatisfied, int plotNumberCount );
		static void determineWhichPscsSatisfied( PlottingInfo& info, UINT groupSize, 
												 std::vector<std::vector<int>> atomPresentData,
												 std::vector<std::vector<bool>>& pscSatisfied );
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


class pictureData
{

};

class repetitionData
{

};

class groupData
{

};
