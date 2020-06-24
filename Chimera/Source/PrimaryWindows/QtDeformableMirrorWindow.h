#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/Communicator.h"
#include "Agilent/Agilent.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraWindowWidget.h"
#include "DeformableMirror/DmControl.h"

namespace Ui {
    class QtDeformableMirrorWindow;
}

class QtDeformableMirrorWindow : public IChimeraWindowWidget {
    Q_OBJECT
		 
    public:
        explicit QtDeformableMirrorWindow (QWidget* parent=NULL);
        ~QtDeformableMirrorWindow ();

        void initializeWidgets ();

        void windowOpenConfig (ConfigStream& configFile);
        void windowSaveConfig (ConfigStream& newFile);
        void fillExpDeviceList (DeviceList& list);

        void handleProgramDmNow ();
        void handlePistonChange (UINT id);
        void handleNewDmProfile ();
        void handleAddAbberations ();
        
    private:
        Ui::QtDeformableMirrorWindow* ui;
        DmControl dm;
};

