#pragma once
// created by Max Kolanz
#pragma once
#include <unordered_map>
#include <string>
#include "afxwin.h"
#include "ExperimentMonitoringAndStatus/ColorBox.h"
#include "DeformableMirror/DmControl.h"
#include "Plotting/PlotCtrl.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ExperimentThread/Communicator.h"

class MainWindow;
class AuxiliaryWindow;
class BaslerWindow;
class AndorWindow;
class ScriptingWindow;
 
// The Device window houses most of the controls for seeting individual devices, other than the camera which gets its 
// own control. It also houses a couple auxiliary things like variables and the SMS texting control.
class DeformableMirrorWindow : public CDialog
{
	DECLARE_DYNAMIC(DeformableMirrorWindow);
	public:
		DeformableMirrorWindow();
	
		BOOL handleAccelerators(HACCEL m_haccel, LPMSG lpMsg);
		void handleProgramNow();
		void handlePistonChange(UINT id);
		void handleNewProfile();
		void handleAbberations();
		void handleNewConfig(std::ofstream& newFile);
		void windowOpenConfig(ScriptStream& configFile, Version ver);
		void handleSaveConfig(std::ofstream& newFile);
	
		BOOL OnInitDialog();
		void loadFriends(MainWindow* mainWin_, ScriptingWindow* scriptWin_, AndorWindow* camWin_,
						 AuxiliaryWindow* auxWin_, BaslerWindow* basWin2_);
	
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnCancel();
		void OnSize(UINT nType, int cx, int cy);
		void OnPaint();
		void passCommonCommand(UINT id);
		DmCore &GetCore();

		cToolTips toolTips;
	
	private:
		DECLARE_MESSAGE_MAP();

		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		AndorWindow* camWin;
		BaslerWindow* basWin;
		AuxiliaryWindow* auxWin;

		CMenu menu;
		fontMap Aux2fonts;
		Communicator comm;

		ColorBox statusBox;
		DmControl dm;
		PlotCtrl *Mirror;
};
