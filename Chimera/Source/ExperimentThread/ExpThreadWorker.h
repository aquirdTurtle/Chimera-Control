#pragma once

#include <ExperimentThread/ExperimentThreadInput.h>
#include <qobject.h>
#include <string>

class ExpThreadWorker : public QObject {
    Q_OBJECT

    public:
        ExpThreadWorker (ExperimentThreadInput* input_);
        ~ExpThreadWorker ();

    public Q_SLOTS:
        void process ();
    Q_SIGNALS:
        //void finished ();
        //void error (QString err);
        void updateBoxColor (QString, QString);
        void notification (QString msg);
        void warn (QString msg);
        void debugInfo (QString msg);
        void repUpdate (unsigned int);
        void prepareAndor (void*);
        void prepareBasler ();
        void plot_Xvals_determined (std::vector<double>);
        void doAoData (const std::vector<std::vector<plotDataVec>>& doData,
                       const std::vector<std::vector<plotDataVec>>& aoData);
        void normalExperimentFinish ();
        void errorExperimentFinish ();
        void mainProcessFinish ();
    private:
        // add your variables here
        ExperimentThreadInput* inputr;
};

