#pragma once
#include "PiezoCore.h"
#include "Control.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "ConfigurationSystems/Version.h"
#include <fstream>
#include "CustomMfcControlWrappers/myButton.h"
#include "Piezo/PiezoType.h"
#include <QWinWidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <QCheckBox.h>
#include <PrimaryWindows/IChimeraWindowWidget.h>

class PiezoController
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		PiezoController& operator=(const PiezoController&) = delete;
		PiezoController (const PiezoController&) = delete;

		PiezoController (piezoSetupInfo info);
		void initialize ( POINT& pos, IChimeraWindowWidget* parent, LONG width, piezoChan<std::string> names );
		std::string getDeviceInfo ( );
		void updateCurrentValues ( );
		void handleOpenConfig (ConfigStream& configFile);
		void handleSaveConfig (ConfigStream& configFile );
		PiezoCore& getCore ( );
		void updateCtrl ( );
		std::string getPiezoDeviceList ( );
		void handleProgramNowPress ( );
		void rearrange ( UINT width, UINT height, fontMap fonts);
		std::string getConfigDelim ( );

	private:
		PiezoCore core;
		QPushButton* programNowButton;
		QCheckBox* ctrlButton;
		piezoChan<QLabel*> labels;
		piezoChan<QLineEdit*> edits;
		piezoChan<QLabel*> currentVals;		
};