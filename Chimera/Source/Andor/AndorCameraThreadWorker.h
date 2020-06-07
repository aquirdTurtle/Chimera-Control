#pragma once

#include <qobject.h>
#include <string>
#include <Andor/cameraThreadInput.h>

class AndorCameraThreadWorker : public QObject {
    Q_OBJECT

    public:
        AndorCameraThreadWorker (cameraThreadInput* input_);
        ~AndorCameraThreadWorker ();
    private:
        cameraThreadInput* input;

    public Q_SLOTS:
        void process ();
    Q_SIGNALS:
        void notify (QString);
        void pictureTaken (int);
        void acquisitionFinished ();
};

