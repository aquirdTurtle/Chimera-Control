// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralImaging/imageParameters.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

struct cameraPositions;
class AndorWindow;
class MainWindow;


class ImageDimsControl{
	public:
		ImageDimsControl(std::string whichCam);
		void initialize( QPoint& pos, IChimeraQtWindow* parentWindow, int numRows, int width);
		imageParameters readImageParameters();
		void setImageParametersFromInput( imageParameters param );
		bool checkReady();
		void handleSave(ConfigStream& saveFile );
		void handleOpen(ConfigStream& openFile );
		static imageParameters getImageDimSettingsFromConfig (ConfigStream& configFile );
		const std::string camType;
		void updateEnabledStatus (bool viewRunning);
		void saveParams (ConfigStream& saveFile, imageParameters params);
	private:
		QLabel* leftText;
		QLabel* rightText;
		QLabel* horBinningText;
		QLabel* bottomLabel;
		QLabel* topLabel;
		QLabel* vertBinningText;
		CQLineEdit* leftEdit;
		CQLineEdit* rightEdit;
		CQLineEdit* horBinningEdit;
		CQLineEdit* bottomEdit;
		CQLineEdit* topEdit;
		CQLineEdit* vertBinningEdit;
		bool isReady;
		imageParameters currentImageParameters;
};
