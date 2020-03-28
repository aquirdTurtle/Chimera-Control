// created by Mark O. Brown
#pragma once

#include "ConfigurationSystems/Version.h"
#include "Control.h"
#include "GeneralObjects/commonTypes.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "CustomMfcControlWrappers/myButton.h"


struct mainOptions
{
	bool dontActuallyGenerate=false;
	bool randomizeVariations=false;
	bool randomizeReps=false;
	UINT atomThresholdForSkip=UINT_MAX;
};

// this got whittled down recently, but keeping so that I can put more stuff in later.
class MainOptionsControl
{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		static mainOptions getSettingsFromConfig(ConfigStream& openFile, Version ver );
		void setOptions ( mainOptions opts );
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