#pragma once

#include "NiawgCore.h"
#include "Scripts/Script.h"
#include "Rearrangement/rerngGuiControl.h"

class NiawgSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		NiawgSystem& operator=(const NiawgSystem&) = delete;
		NiawgSystem (const NiawgSystem&) = delete;
		NiawgSystem (DoRows::which trigRow, UINT trigNumber, bool safemode);

		Script niawgScript;
		NiawgCore core;
		rerngGuiControl rearrangeCtrl;		
};
