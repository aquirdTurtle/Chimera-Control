#pragma once
#include <qobject.h>
#include <GeneralObjects/Matrix.h>
 
class BaslerGrabThreadWorker : public QObject {
	Q_OBJECT;
	 
	Q_SIGNALS:
		void newBaslerImage (Matrix<long> imageMatrix); 
};
