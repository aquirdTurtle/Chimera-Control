#pragma once
#include <memory>

#include <qobject.h>
#include <QWidget.h>
#include <qstring.h>
#include <AnalogInput/calInfo.h>
#include <Plotting/QCustomPlotCtrl.h>

class CalibrationManager; 
class AiSystem;
class AoSystem;
class DoSystem;
class AgilentCore;
class NewPythonHandler;
class PlotCtrl;

struct CalibrationThreadInput {
	CalibrationThreadInput () {}
	AiSystem* ai;
	AoSystem* ao;
	DoSystem* ttls;
	std::vector<std::reference_wrapper<AgilentCore>> agilents;
	NewPythonHandler* pythonHandler;
	QCustomPlotCtrl* calibrationViewer;
	QWidget* parentWin;
	std::vector<std::reference_wrapper<calSettings>> calibrations;
};

class CalibrationThreadWorker : public QObject {
	Q_OBJECT
	public:
		CalibrationThreadWorker (CalibrationThreadInput input_);
		~CalibrationThreadWorker ();
	public Q_SLOTS:
		void runAll ();
		void calibrate (calSettings& cal, unsigned which);
Q_SIGNALS:
		void notification(statusMsg);
		void warn(statusMsg);
		void error(statusMsg);
		/*void notification (QString msg, unsigned baseLvl = 0, int debugLvl=0);
		void warn (QString msg, unsigned debugLvl = 1);
		void error (QString msg, unsigned debugLvl = 0);*/
		void calibrationChanged ();
		void startingNewCalibration (calSettings cal);
		void newCalibrationDataPoint (QPointF point);
		void finishedCalibration(calSettings cal);
	private:
		CalibrationThreadInput input;
};

