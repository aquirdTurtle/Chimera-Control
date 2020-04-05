#pragma once

#include "GeneralObjects/commonTypes.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ConfigurationSystems/Version.h"
#include "ExperimentMonitoringAndStatus/colorbox.h"
#include "ExperimentThread/DeviceList.h"
#include "afxwin.h"
#include <array>

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
		virtual void windowOpenConfig (ConfigStream& configFile)=0;
		virtual void windowSaveConfig (ConfigStream& newFile)=0;
		// Basically every window has devices which should be programmed on it, 
		// so every window must have a function to add them.
		virtual void fillExpDeviceList (DeviceList& list) = 0;

		BOOL OnInitDialog ();

		virtual HBRUSH OnCtlColor (CDC* pDC, CWnd* pWnd, UINT nCtlColor);

		virtual void OnCancel ();
		virtual void OnEnter ();
		void setMenuCheck (UINT menuItem, UINT itemState);

		

		void passCommonCommand (UINT id);
		void loadFriends (MainWindow* mainWin_, ScriptingWindow* scriptWin_, AuxiliaryWindow* auxWin_,
						  BaslerWindow* basWin_, DeformableMirrorWindow* dmWindow_, AndorWindow* andorWin_);
		
		void reportErr (std::string errStr);
		void reportStatus (std::string statusStr);
		std::vector<IChimeraWindow*> winList();
		MainWindow* mainWin = NULL;
		ScriptingWindow* scriptWin = NULL;
		AndorWindow* andorWin = NULL;
		AuxiliaryWindow* auxWin = NULL;
		BaslerWindow* basWin = NULL;
		DeformableMirrorWindow* dmWin = NULL;
		static constexpr UINT numWindows = 6;
		cToolTips toolTips;
		ColorBox statBox;
		virtual void changeBoxColor (std::string sysDelim, char color);
	private:
		CMenu menu;
		DECLARE_MESSAGE_MAP ();
};
