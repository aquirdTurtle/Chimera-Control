#pragma once

#include "Control.h"
#include "PlottingInfo.h"

struct plotThreadInput;
struct cameraPositions;

// "Tiny" because it only contains a little info right now.
struct tinyPlotInfo
{
	bool isActive;
	std::string name;
};

class DataAnalysisControl
{	
	public:
		void initialize( cameraPositions& pos, int& id, CWnd* parent, fontMap fonts, cToolTips& tooltips,
						 int isTriggerModeSensitive );
		void handleDoubleClick( fontMap* fonts, UINT currentPicsPerRepetition );
		void handleRClick( );
		void rearrange(std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts);
		//
		void updateDataSetNumberEdit( int number );
		void analyze( std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler,
					  Communicator* comm );
		void onButtonPushed();
		void setAtomLocation( std::pair<int, int> location );
		std::vector<std::pair<int, int>> getAnalysisLocations();
		void clearAtomLocations();
		bool getLocationSettingStatus();
		std::vector<std::string> getActivePlotList();
		void reloadListView();
		bool buttonClicked();

		void fillPlotThreadInput(plotThreadInput* input);
		static unsigned __stdcall plotterProcedure(void* voidInput);
		static void handlePlotAtomsOrCounts(plotThreadInput* input, PlottingInfo plotInfo, UINT repNum,
									 std::vector<std::vector<std::vector<long> > >& finData,
									 std::vector<std::vector<std::vector<double> > >& finAvgs,
									 std::vector<std::vector<std::vector<double> > >& finErrs,
									 std::vector<std::vector<std::vector<double> > >& finX,
									 std::vector<std::vector<bool> >& needNewData, 
									 std::vector<std::vector<bool>>& pscSatisfied, bool plotNumber,
									 std::vector<std::vector<long>>& countData, int plotNumberCount,
									 std::vector<std::vector<int> > atomPresent);

		static void handlePlotHist(plotThreadInput* input, PlottingInfo plotInfo, UINT plotNumber,
									std::vector<std::vector<long>> countData,
									std::vector<std::vector<std::vector<long>>>& finData,
									std::vector<std::vector<bool>>pscSatisfied);

	private:
		// real time plotting
		int updateFrequency;
		Control<CStatic> updateFrequencyLabel1;
		Control<CStatic> updateFrequencyLabel2;
		Control<CEdit> updateFrequencyEdit;
		Control<CStatic> header;
		Control<CListCtrl> plotListview;
		std::vector<tinyPlotInfo> allPlots;
		//CDialog plotDesigner;
		// other data analysis
		bool currentlySettingAnalysisLocations;
		Control<CStatic> currentDataSetNumberText;
		Control<CStatic> currentDataSetNumberEdit;
		//Control<CButton> autoAnalyzeCheckBox;
		Control<CButton> setAnalysisLocationsButton;
		//Control<CButton> analyzeMostRecentButton;
		std::vector<std::pair<int, int>> atomLocations;
		bool threadNeedsCounts;
};
