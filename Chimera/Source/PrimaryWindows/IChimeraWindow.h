#pragma once

#include "afxwin.h"
#include "GeneralObjects/commonTypes.h"
#include <array>
#include "ConfigurationSystems/ConfigStream.h"
#include "ConfigurationSystems/Version.h"

class MainWindow;
class ScriptingWindow;
class AuxiliaryWindow;
class BaslerWindow;
class DeformableMirrorWindow;
class AndorWindow;

/*
* This isn't a pure abstract class.
*/
class IChimeraWindow : public CDialog
{
	DECLARE_DYNAMIC (IChimeraWindow)
	public:
		IChimeraWindow ();
		using CDialog::CDialog;
		virtual BOOL PreTranslateMessage (MSG* pMsg);
		// any Chimera window should override OnSize to rearrange any controls on the window.
		virtual void OnSize (UINT nType, int cx, int cy) = 0;
		// any chimera window should override these functions in order to 
		virtual void windowOpenConfig (ConfigStream& configFile, Version ver)=0;
		virtual void windowSaveConfig (ConfigStream& newFile)=0;

		virtual HBRUSH OnCtlColor (CDC* pDC, CWnd* pWnd, UINT nCtlColor);

		virtual void OnCancel ();
		virtual void OnEnter ();

		void passCommonCommand (UINT id);
		void loadFriends (MainWindow* mainWin_, ScriptingWindow* scriptWin_, AuxiliaryWindow* auxWin_,
						  BaslerWindow* basWin_, DeformableMirrorWindow* dmWindow_, AndorWindow* andorWin_);
		
		void reportErr (std::string errStr);
		void reportStatus (std::string statusStr);

		MainWindow* mainWin = NULL;
		ScriptingWindow* scriptWin = NULL;
		AndorWindow* andorWin = NULL;
		AuxiliaryWindow* auxWin = NULL;
		BaslerWindow* basWin = NULL;
		DeformableMirrorWindow* dmWin = NULL;
		static constexpr UINT numWindows = 6;
		cToolTips toolTips;
	private:
		DECLARE_MESSAGE_MAP ();
};
