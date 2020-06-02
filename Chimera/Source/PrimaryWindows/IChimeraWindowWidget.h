#pragma once

#include "GeneralObjects/commonTypes.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ConfigurationSystems/Version.h"
#include "ExperimentThread/DeviceList.h"
#include "afxwin.h"
#include <array>
#include "QWinWidget.h"
#include <QMainWindow>
#include <ExperimentMonitoringAndStatus/ColorBox.h>

class ColorBox;

class QtMainWindow;
class QtScriptWindow;
class QtAuxiliaryWindow;
class QtBaslerWindow;
class QtDeformableMirrorWindow;
class QtAndorWindow;

namespace Ui {
	class IChimeraWindowWidget;
}

class IChimeraWindowWidget : public QMainWindow
{
	Q_OBJECT
	public:
		explicit IChimeraWindowWidget (QWidget* parent = NULL);
		~IChimeraWindowWidget () {};
		// any chimera window should override these functions in order to 
		virtual void windowOpenConfig (ConfigStream& configFile)=0;
		virtual void windowSaveConfig (ConfigStream& newFile)=0;
		// Basically every window has devices which should be programmed on it, 
		// so every window must have a function to add them.
		virtual void fillExpDeviceList (DeviceList& list) = 0;
		virtual void initializeWidgets () = 0;
		void initializeShortcuts ();
		void initializeMenu ();
		void setMenuCheck (UINT menuItem, UINT itemState);

		void passCommonCommand (UINT id);
		void loadFriends (QtMainWindow* mainWin_, QtScriptWindow* scriptWin_, QtAuxiliaryWindow* auxWin_,
						  QtBaslerWindow* basWin_, QtDeformableMirrorWindow* dmWindow_, QtAndorWindow* andorWin_);
		void reportErr (std::string errStr);
		void reportStatus (std::string statusStr);
		void configUpdated ();
		std::vector<IChimeraWindowWidget*> winList();
		QtMainWindow* mainWin = NULL;
		QtScriptWindow* scriptWin = NULL;
		QtAndorWindow* andorWin = NULL;
		QtAuxiliaryWindow* auxWin = NULL;
		QtBaslerWindow* basWin = NULL;
		QtDeformableMirrorWindow* dmWin = NULL;
		static constexpr UINT numWindows = 6;
		ColorBox* statBox;
		virtual void changeBoxColor (std::string sysDelim, std::string color);
	private:
		Ui::IChimeraWindowWidget* ui;
};
