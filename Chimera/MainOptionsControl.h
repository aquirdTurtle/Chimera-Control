#pragma once

#include "Version.h"
#include "Control.h"
#include "commonTypes.h"
#include "myButton.h"


struct mainOptions
{
	bool dontActuallyGenerate;
	bool randomizeVariations;
	bool randomizeReps;
	UINT atomThresholdForSkip=UINT_MAX;
};

// this got whittled down recently, but keeping so that I can put more stuff in later.
class MainOptionsControl
{
	public:
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, Version ver );
		void initialize(int& idStart, POINT& loc, CWnd* parent, cToolTips& tooltips );
		mainOptions getOptions();
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CStatic> header;
		Control<CleanCheck> randomizeVariationsButton;
		Control<CleanCheck> randomizeRepsButton;
		Control<CStatic> atomThresholdForSkipText;
		Control<CEdit> atomThresholdForSkipEdit;
		mainOptions currentOptions;
};