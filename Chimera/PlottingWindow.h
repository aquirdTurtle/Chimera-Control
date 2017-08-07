#pragma once

#include "windows.h"
#include "commonTypes.h"

class PlottingWindow : public CDialog
{
	public:
		BOOL OnInitDialog() override;
		void saveAnalysisPixelLocations(bool clearEdits);
		void savePositiveConditions(bool clear);
		void savePostSelectionConditions(bool clear);
		void saveDataSet(bool clear);
		void enableAndDisable();
		std::string returnAllInfo();
		
	private:
		// all the controls...	
		
};