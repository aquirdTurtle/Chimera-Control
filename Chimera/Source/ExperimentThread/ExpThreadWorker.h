#pragma once

#include <ExperimentThread/ExperimentThreadInput.h>
#include <qobject.h>
#include <string>

struct baslerSettings;

class ExpThreadWorker : public QObject {
    Q_OBJECT

    public:
        ExpThreadWorker (ExperimentThreadInput* input_);
        ~ExpThreadWorker ();

    public Q_SLOTS:
        void process ();
    Q_SIGNALS:
        void updateBoxColor (QString, QString);
        void notification (QString msg, unsigned debugLvl=0);
        void warn (QString msg, unsigned debugLvl=1);
        void debugInfo (QString msg);
        void repUpdate (unsigned int);
        void prepareAndor (void*);
        void prepareBasler (baslerSettings* settings);
        void plot_Xvals_determined (std::vector<double>);
        void doAoData (const std::vector<std::vector<plotDataVec>>& doData,
                       const std::vector<std::vector<plotDataVec>>& aoData);
        void normalExperimentFinish (QString, profileSettings);
		void calibrationFinish (QString, profileSettings);
        void errorExperimentFinish (QString, profileSettings);

        void mainProcessFinish (); 

    private:
        // add your variables here
        ExperimentThreadInput* inputr;
};

