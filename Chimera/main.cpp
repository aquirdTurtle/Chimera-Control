#include "stdafx.h"
#include <qapplication.h>
#include <ExcessDialogs/splashDialog.h>
#include <PrimaryWindows/QtMainWindow.h>

int main (int argc, char** argv) {
	initMyColors ();
	if (NIAWG_GAIN > MAX_GAIN) {
		errBox ("FATAL ERROR: NIAWG_GAIN SET TOO HIGH. Driving too much power into the AOMs could severaly damage the "
			"experiment!\r\n");
		return -10000;
	}
	qRegisterMetaType<Matrix<long>> ();
	qRegisterMetaType<QVector<double>> ();
	qRegisterMetaType<std::vector<double>> ();
	qRegisterMetaType<std::vector<std::vector<plotDataVec>>> ();
	qRegisterMetaType<NormalImage> ();
	qRegisterMetaType<atomQueue> ();
	qRegisterMetaType<std::vector<std::vector<dataPoint> >> ();
	qRegisterMetaType<PlottingInfo> ();
	qRegisterMetaType<PixListQueue> ();
	QApplication app (argc, argv);
	QtMainWindow* mainWinQt = new QtMainWindow ();
	mainWinQt->show ();
	// end of program.
	return app.exec ();
}