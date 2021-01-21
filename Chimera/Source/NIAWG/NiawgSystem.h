#pragma once

#include "NiawgCore.h"
#include "Scripts/Script.h"
#include "Rearrangement/rerngGuiControl.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qcheckbox.h>

class NiawgSystem : public IChimeraSystem {
	public:
		// THIS CLASS IS NOT COPYABLE.
		NiawgSystem& operator=(const NiawgSystem&) = delete;
		NiawgSystem (const NiawgSystem&) = delete;
		NiawgSystem (DoRows::which trigRow, unsigned trigNumber, bool safemode, IChimeraQtWindow* parent);

		void initialize (QPoint& loc, IChimeraQtWindow* qtp);
		void handleSaveConfig (ConfigStream& saveFile);
		void handleOpenConfig (ConfigStream& openfile);
		//static bool getSettingsFromConfig (ConfigStream& openfile);
		void updateWindowEnabled ();

		Script niawgScript;
		NiawgCore core;
		rerngGuiControl rearrangeCtrl;		
		QLabel* niawgHeader;
		QCheckBox* controlNiawg;
		QLabel* debugLvlLabel;
		QLineEdit* debugLvlEdit;
};
