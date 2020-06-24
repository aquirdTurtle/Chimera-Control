#pragma once

#include <qobject.h>
#include <string>
#include <GeneralObjects/Queues.h>
#include <RealTimeDataAnalysis/atomCruncherInput.h>

class CruncherThreadWorker : public QObject {
    Q_OBJECT

    public:
        CruncherThreadWorker (atomCruncherInput* input_);
        ~CruncherThreadWorker ();

    public Q_SLOTS:
        void init ();
        void handleImage (NormalImage);
		 
    Q_SIGNALS:
        void error (QString errstr);
        void atomArray (atomQueue aqueue);
        void pixArray (PixListQueue pixlist);
    private:
        atomCruncherInput* input;
        unsigned imageCount;
        std::vector<std::vector<long>> monitoredPixelIndecies;
};

