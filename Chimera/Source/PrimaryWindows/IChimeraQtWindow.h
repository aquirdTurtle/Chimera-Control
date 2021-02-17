#pragma once

#include "GeneralObjects/commonTypes.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ConfigurationSystems/Version.h"
#include "ExperimentThread/DeviceList.h"
#include <array>
#include <QMainWindow>


class ColorBox;
class QtMainWindow;
class QtScriptWindow;
class QtAuxiliaryWindow;
class QtBaslerWindow;
class QtDeformableMirrorWindow;
class QtAndorWindow;

namespace Ui {
	class IChimeraQtWindow;
}

class IChimeraQtWindow : public QMainWindow{
	Q_OBJECT
	public:
		explicit IChimeraQtWindow (QWidget* parent = nullptr);
		~IChimeraQtWindow () {};
		// any chimera window should override these functions in order to handle
		virtual void windowOpenConfig (ConfigStream& configFile)=0;
		virtual void windowSaveConfig (ConfigStream& newFile)=0;
		// Basically every window has devices which should be programmed on it, 
		// so every window must have a function to add them.
		virtual void fillExpDeviceList (DeviceList& list) = 0;
		virtual void initializeWidgets () = 0;
		void initializeShortcuts ();
		void initializeMenu ();
		void loadFriends (QtMainWindow* mainWin_, QtScriptWindow* scriptWin_, QtAuxiliaryWindow* auxWin_,
						  QtBaslerWindow* basWin_, QtDeformableMirrorWindow* dmWindow_, QtAndorWindow* andorWin_);
		void reportErr (statusMsg msg);
		void reportWarning(statusMsg msg);
		void reportStatus (statusMsg msg);

		std::vector<IChimeraQtWindow*> winList();
		QtMainWindow* mainWin = nullptr;
		QtScriptWindow* scriptWin = nullptr;
		QtAndorWindow* andorWin = nullptr;
		QtAuxiliaryWindow* auxWin = nullptr;
		QtBaslerWindow* basWin = nullptr;
		QtDeformableMirrorWindow* dmWin = nullptr;
		static constexpr unsigned numWindows = 6;
		ColorBox* statBox;
		virtual void changeBoxColor (std::string sysDelim, std::string color);
	private:
		Ui::IChimeraQtWindow* ui;
	public Q_SLOTS:
		void configUpdated ();
};
