#pragma once

#include "NiawgCore.h"
#include "Scripts/Script.h"
#include "Rearrangement/rerngGuiControl.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "CustomMfcControlWrappers/myButton.h""

class NiawgSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		NiawgSystem& operator=(const NiawgSystem&) = delete;
		NiawgSystem (const NiawgSystem&) = delete;
		NiawgSystem (DoRows::which trigRow, UINT trigNumber, bool safemode);

		void initialize (int& id, POINT& loc, CWnd* parent, cToolTips& tooltips);
		void rearrange (UINT width, UINT height, fontMap fonts);
		void handleSaveConfig (ConfigStream& saveFile);
		void handleOpenConfig (ConfigStream& openfile, Version ver);
		static bool getSettingsFromConfig (ConfigStream& openfile, Version ver);
		void updateWindowEnabled ();

		Script niawgScript;
		NiawgCore core;
		rerngGuiControl rearrangeCtrl;		
		Control<CStatic> niawgHeader;
		Control<CleanCheck> controlNiawg;
};
