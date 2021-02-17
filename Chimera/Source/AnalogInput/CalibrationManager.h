#pragma once
// created by Mark O. Brown
#pragma once

#include <AnalogInput/calInfo.h>
#include "AnalogInput/AiSystem.h"
#include <AnalogInput/CalibrationThreadWorker.h>
#include "AnalogOutput/AoSystem.h"
#include "DigitalOutput/DoSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/Version.h"
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <Agilent/AgilentCore.h>
#include <Python/NewPythonHandler.h>
#include <type_traits>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>


class CalibrationManager : public IChimeraSystem {
	public:
		// THIS CLASS IS NOT COPYABLE.
		CalibrationManager& operator=(const CalibrationManager&) = delete;
		CalibrationManager (const CalibrationManager&) = delete;
		CalibrationManager (IChimeraQtWindow* parent);

		static void determineCalMinMax (calResult& res);
		void handleContextMenu (const QPoint& pos);
		static std::string calTtlConfigToString (std::vector<std::pair<DoRows::which, unsigned> > ttlConfig);
		static std::string calDacConfigToString (std::vector<std::pair<unsigned, double>> aoConfig);
		void initialize (QPoint& pos, IChimeraQtWindow* parent, AiSystem* ai, AoSystem* ao, DoSystem* ttls_in,
			std::vector<std::reference_wrapper<AgilentCore>> agilentsIn, NewPythonHandler* python_in);
		void runAllThreaded ();
		void calibrateThreaded (calSettings& cal, unsigned which);
		bool wantsExpAutoCal ();
		void handleSaveMasterConfig (std::stringstream& configStream);
		void handleSaveMasterConfigIndvResult (std::stringstream& configStream, calResult& cal);
		void handleOpenMasterConfigIndvResult (ConfigStream& configStream, calResult& result);
		void handleOpenMasterConfig (ConfigStream& configStream);
		std::vector<calResult> getCalibrationInfo ();
		void standardStartThread (std::vector<std::reference_wrapper<calSettings>> calibrations);
		void refreshListview ();
		static std::vector<double> calPtTextToVals (QString qtxt);
		static double calibrationFunction (double val, calResult calibration, IChimeraSystem* parent = nullptr);
	private:
		const std::string systemDelim = "CALIBRATION_MANAGER";
		QLabel* calsHeader;
		CQPushButton* calibrateAllButton;
		CQCheckBox* expAutoCalButton;
		QTableWidget* calibrationTable;
		QPushButton* cancelCalButton;
		QCustomPlotCtrl calibrationViewer;
		//PlotCtrl calibrationViewer;
		void updateCalibrationView (calSettings& cal);
		void handleSaveMasterConfigIndvCal (std::stringstream& configStream, calSettings& servo);
		calSettings handleOpenMasterConfigIndvCal (ConfigStream& configStream);
		std::vector<calSettings> calibrations;
		void addCalToListview (calSettings& cal);
		CalibrationThreadWorker* threadWorker;
		QThread* thread;
		AiSystem* ai;
		AoSystem* ao;
		DoSystem* ttls;
		std::vector<std::reference_wrapper<AgilentCore>> agilents;
		NewPythonHandler* pythonHandler;
};



