#pragma once

#include <qobject.h>
#include <string>

class ScopeViewer;

class ScopeThreadWorker : public QObject {
    Q_OBJECT
    public:
        ScopeThreadWorker (ScopeViewer* input_);
        ~ScopeThreadWorker () {};
    private:
        ScopeViewer* input;
    public Q_SLOTS:
        void process (); 
    Q_SIGNALS:
        void notify (QString);
        void newData (const QVector<double>& xdata, double xmin, double xmax, const QVector<double>& ydata, double ymin, double ymax, int traceNum);
};

