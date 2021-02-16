#pragma once
#include "PiezoCore.h"
#include "Control.h"
#include "ConfigurationSystems/Version.h"
#include <fstream>
#include "Piezo/PiezoType.h"
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <QCheckBox.h>
#include <GeneralObjects/IChimeraSystem.h>
#include <PrimaryWindows/IChimeraQtWindow.h>

class PiezoController : public IChimeraSystem {
	public:
		// THIS CLASS IS NOT COPYABLE.
		PiezoController& operator=(const PiezoController&) = delete;
		PiezoController (const PiezoController&) = delete;

		PiezoController (IChimeraQtWindow* parent, piezoSetupInfo info);
		void initialize ( QPoint& pos, IChimeraQtWindow* parent, long width, piezoChan<std::string> names );
		std::string getDeviceInfo ( );
		void updateCurrentValues ( );
		void handleOpenConfig (ConfigStream& configFile);
		void handleSaveConfig (ConfigStream& configFile );
		PiezoCore& getCore ( );
		void updateCtrl ( );
		std::string getPiezoDeviceList ( );
		void handleProgramNowPress ( );
		std::string getConfigDelim ( );
	private:
		bool expActive;
		const bool isReflective;
		PiezoCore core;
		QPushButton* programNowButton;
		QCheckBox* ctrlButton;
		piezoChan<QLabel*> labels;
		piezoChan<QLineEdit*> edits;
		piezoChan<QLabel*> currentVals;		
};