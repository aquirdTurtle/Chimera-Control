// created by Mark O. Brown
#pragma once

#include "ConfigurationSystems/Version.h"
#include "Control.h"
#include "GeneralObjects/commonTypes.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "CustomMfcControlWrappers/myButton.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <PrimaryWindows/IChimeraQtWindow.h>

struct mainOptions{
	bool dontActuallyGenerate=false;
	bool randomizeVariations=false;
	bool randomizeReps=false;
	bool delayAutoCal = false;
	unsigned atomSkipThreshold=UINT_MAX;
};

// this got whittled down recently, but keeping so that I can put more stuff in later.
class MainOptionsControl{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		static mainOptions getSettingsFromConfig(ConfigStream& openFile );
		void setOptions ( mainOptions opts );
		void initialize(POINT& loc, IChimeraQtWindow* parent );
		mainOptions getOptions();

	private:
		QLabel* header;
		QCheckBox* randomizeVariationsButton;
		QCheckBox* delayAutoCal;
		QCheckBox* randomizeRepsButton;
		QLabel* atomThresholdForSkipText;
		QLineEdit* atomThresholdForSkipEdit;
		mainOptions currentOptions;
};