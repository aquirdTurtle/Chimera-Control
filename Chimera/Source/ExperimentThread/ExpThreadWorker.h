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
        void notify (QString);
        void warn (QString);
        void debugInfo (QString);
        void repUpdate (unsigned int);
        void prepareAndor (void*);
        void prepareBasler ();
        void doAoData (const std::vector<std::vector<plotDataVec>>& doData,
                       const std::vector<std::vector<plotDataVec>>& aoData);
    private:
        // add your variables here
        ExperimentThreadInput* inputr;
};

