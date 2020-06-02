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
        void repUpdate (unsigned int);
    private:
        // add your variables here
        ExperimentThreadInput* inputr;
};

