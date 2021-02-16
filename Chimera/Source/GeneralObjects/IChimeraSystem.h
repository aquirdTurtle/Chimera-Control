#pragma once

#include <qwidget.h>
#include <qobject.h>

class IChimeraQtWindow;

class IChimeraSystem : public QObject {
	Q_OBJECT;
	public:
		IChimeraSystem(IChimeraQtWindow* parent_in);
		IChimeraQtWindow* parentWin;

	public Q_SLOTS:

	Q_SIGNALS:
		void notification (statusMsg msg);
		void warning (statusMsg msg);
		void error (statusMsg msg);
}; 

