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
#include "IChimeraWindow.h"

class MainWindow;
class AuxiliaryWindow;
class BaslerWindow;
class AndorWindow;
class ScriptingWindow;
 
// The Device window houses most of the controls for seeting individual devices, other than the camera which gets its 
// own control. It also houses a couple auxiliary things like variables and the SMS texting control.
class DeformableMirrorWindow : public IChimeraWindow
{
	DECLARE_DYNAMIC(DeformableMirrorWindow);
	public:
		DeformableMirrorWindow();
		BOOL handleAccelerators(HACCEL m_haccel, LPMSG lpMsg);
		void handleProgramNow();
		void handlePistonChange(UINT id);
		void handleNewProfile();
		void handleAbberations();
		void windowOpenConfig(ConfigStream& configFile, Version ver);
		void windowSaveConfig(ConfigStream& newFile);
		BOOL OnInitDialog();
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnSize(UINT nType, int cx, int cy);
		void OnPaint();
		DmCore &GetCore();
		cToolTips toolTips;	
	private:
		DECLARE_MESSAGE_MAP();

		CMenu menu;
		ColorBox statusBox;
		DmControl dm;
		PlotCtrl *Mirror;
};
