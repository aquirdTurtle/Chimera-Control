#pragma once
#include <qobject.h>

class BaslerGrabThreadWorker : public QObject {
	Q_OBJECT;

	Q_SIGNALS:
		void newBaslerImage ();
};