#pragma once

#include "Control.h"

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
		void initialize(cameraPositions& pos, int& id, CWnd* parent, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips,
						int isTriggerModeSensitive);
		void handleDoubleClick();
		void handleRClick();
		void rearrange(std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts);
		//
		void updateDataSetNumberEdit( int number );
		void analyze( std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler,
					  Communicator* comm );
		void onButtonPushed();
		void setAtomLocation( std::pair<int, int> location );
		std::vector<std::pair<int, int>> getAtomLocations();
		void clearAtomLocations();
		bool getLocationSettingStatus();
		std::vector<std::string> getActivePlotList();
	private:
		// real time plotting
		int updateFrequency;
		Control<CStatic> updateFrequencyLabel1;
		Control<CStatic> updateFrequencyLabel2;
		Control<CEdit> updateFrequencyEdit;
		Control<CStatic> header;
		Control<CListCtrl> plotListview;
		std::vector<tinyPlotInfo> allPlots;
		CDialog plotDesigner;
		// other data analysis
		bool currentlySettingAnalysisLocations;
		Control<CStatic> currentDataSetNumberText;
		Control<CStatic> currentDataSetNumberEdit;
		//Control<CButton> autoAnalyzeCheckBox;
		Control<CButton> setAnalysisLocationsButton;
		//Control<CButton> analyzeMostRecentButton;
		std::vector<std::pair<int, int>> atomLocations;
};