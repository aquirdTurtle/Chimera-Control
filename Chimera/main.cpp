#include "stdafx.h"
#include <qapplication.h>
#include <ExcessDialogs/splashDialog.h>
#include <PrimaryWindows/QtMainWindow.h>
#include <qsplashscreen.h>
#include <qscreen.h>
int main (int argc, char** argv) {
	QApplication app (argc, argv);
	QPixmap pixmap ("C:\\Users\\Regal-Lab\\Code\\Chimera-Control\\Chimera\\Source\\Shades_Of_Infrared.bmp");
	QSplashScreen splash (pixmap.scaledToHeight (qApp->screens ()[0]->geometry ().height ()));
	splash.showFullScreen ();

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
	qRegisterMetaType<profileSettings> ();

	QtMainWindow* mainWinQt = new QtMainWindow ();
	mainWinQt->show ();
	splash.finish (mainWinQt);
	return app.exec ();
}