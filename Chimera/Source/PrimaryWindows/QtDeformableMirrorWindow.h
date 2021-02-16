#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Agilent/Agilent.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"
#include "DeformableMirror/DmControl.h"

namespace Ui {
    class QtDeformableMirrorWindow;
}

class QtDeformableMirrorWindow : public IChimeraQtWindow {
    Q_OBJECT
		 
    public:
        explicit QtDeformableMirrorWindow (QWidget* parent=nullptr);
        ~QtDeformableMirrorWindow ();

        void initializeWidgets ();

        void windowOpenConfig (ConfigStream& configFile);
        void windowSaveConfig (ConfigStream& newFile);
        void fillExpDeviceList (DeviceList& list);

        void handleAddAbberations ();
        
    private:
        Ui::QtDeformableMirrorWindow* ui;
        DmControl dm;
};

