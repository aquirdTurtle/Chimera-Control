#pragma once

#include "NiawgCore.h"
#include "Scripts/Script.h"
#include "Rearrangement/rerngGuiControl.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qcheckbox.h>

class NiawgSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		NiawgSystem& operator=(const NiawgSystem&) = delete;
		NiawgSystem (const NiawgSystem&) = delete;
		NiawgSystem (DoRows::which trigRow, UINT trigNumber, bool safemode);

		void initialize (POINT& loc, IChimeraWindowWidget* qtp);
		void rearrange (UINT width, UINT height, fontMap fonts);
		void handleSaveConfig (ConfigStream& saveFile);
		void handleOpenConfig (ConfigStream& openfile);
		static bool getSettingsFromConfig (ConfigStream& openfile);
		void updateWindowEnabled ();

		Script niawgScript;
		NiawgCore core;
		rerngGuiControl rearrangeCtrl;		
		QLabel* niawgHeader;
		QCheckBox* controlNiawg;
};
