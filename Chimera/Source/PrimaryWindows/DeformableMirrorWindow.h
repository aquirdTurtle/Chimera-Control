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
	public:
		DeformableMirrorWindow();
		void handleProgramDmNow();
		void handlePistonChange(UINT id);
		void handleNewDmProfile();
		void handleAddAbberations();
		void windowOpenConfig(ConfigStream& configFile);
		void windowSaveConfig(ConfigStream& newFile);
		BOOL OnInitDialog();
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnSize(UINT nType, int cx, int cy);
		void OnPaint();
		DmCore &GetCore();
	private:
		DECLARE_DYNAMIC (DeformableMirrorWindow);
		DECLARE_MESSAGE_MAP();
		CMenu menu;
		DmControl dm;
		PlotCtrl *Mirror;
};
