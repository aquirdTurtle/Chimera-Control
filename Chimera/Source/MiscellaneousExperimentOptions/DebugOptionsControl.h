// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/commonTypes.h"
#include "debugInfo.h"

class MainWindow;

class DebugOptionsControl
{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void initialize( int& idStart, POINT& loc, CWnd* parent, cToolTips& tooltips );
		void handleEvent(UINT id, MainWindow* comm);
		debugInfo getOptions();
		void setOptions(debugInfo options);
		void rearrange(int width, int height, fontMap fonts);

	private:
		Control<CStatic> header;
		Control<CleanCheck> readProgress;
		Control<CleanCheck> writeProgress;
		Control<CleanCheck> correctionTimes;
		Control<CleanCheck> niawgScript;
		Control<CleanCheck> outputAgilentScript;
		Control<CleanCheck> niawgMachineScript;
		Control<CleanCheck> excessInfo;
		Control<CleanCheck> showTtlsButton;
		Control<CleanCheck> showDacsButton;
		Control<CleanCheck> outputNiawgWavesToText;
		Control<CStatic> pauseText;
		Control<CEdit> pauseEdit;

		debugInfo currentOptions;
};