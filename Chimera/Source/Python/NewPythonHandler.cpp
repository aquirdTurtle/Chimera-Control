#include "stdafx.h"
#include <Python/NewPythonHandler.h>
#include <RealTimeDataAnalysis/atomGrid.h>
#include <qdebug.h>
#include <qprocess.h>
#include <qfile.h>


void NewPythonHandler::runDataAnalysis (std::string date, long runNumber, long accumulations,
	std::vector<coordinate> atomLocations) {
	QString path = "C:/Users/Regal-Lab/Code/Data-Analysis-Code";
	QString  command ("python");
	QStringList params = QStringList () << "script.py";

	QProcess* process = new QProcess ();
	process->startDetached (command, params, path);
	process->waitForFinished ();
	process->close ();
}


double NewPythonHandler::runCarrierAnalysis (std::string date, long runNumber, atomGrid gridInfo, QWidget* parent) {
	QString command ("python");
	auto params = QStringList ();
	params << "C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CarrierAnalysis.py";
	params << qstr(date);
	params << qstr(runNumber);
	params << "[" + qstr (gridInfo.topLeftCorner.row) + "," + qstr (gridInfo.topLeftCorner.column) + ","
		+ qstr (gridInfo.pixelSpacing) + "," + qstr (gridInfo.width) + "," + qstr (gridInfo.height) + "]";

	QProcess* process = new QProcess (parent);
	process->start(command, params);
	process->waitForFinished ();
	process->waitForReadyRead ();
	QByteArray out = process->readAllStandardOutput ();
	QByteArray oute = process->readAllStandardError ();
	process->close ();
	qDebug() << "carrier analysis info: " << out << "\n" << oute;
	std::string filename = "C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CarrierResultFile.txt";
	double resval;
	std::ifstream stdfile (filename);
	stdfile >> resval;
	return resval;
}

std::vector<double> NewPythonHandler::runCalibrationFits (calSettings cal, QWidget* parent) {
	QString command ("python");
	auto params = QStringList ();
	params << "C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CalibrationAnalysis.py" << qstr(cal.includeSqrt) 
		<< qstr(cal.result.polynomialOrder);

	QProcess* process = new QProcess (parent);
	process->start (command, params);
	process->waitForFinished ();
	process->waitForReadyRead ();
	QByteArray out = process->readAllStandardOutput ();
	QByteArray oute = process->readAllStandardError ();
	process->close ();
	qDebug () << out << oute;
	std::string resname = "C:\\Users\\Regal-Lab\\Code\\Data-Analysis-Code\\CalibrationResultFile.txt";
	double resval;
	std::ifstream stdfile (resname);
	// the coefficients of the polynomial fit I'm using.
	std::vector<double> calibrationCoef;
	while (stdfile >> resval) {
		calibrationCoef.push_back (resval);
	};
	return calibrationCoef;
}
